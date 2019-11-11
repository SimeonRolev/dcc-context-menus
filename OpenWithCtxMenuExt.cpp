// OpenWithCtxMenuExt.cpp : Implementation of COpenWithCtxMenuExt

#include "stdafx.h"
#include "OpenWithExt.h"
#include "OpenWithCtxMenuExt.h"

#pragma comment(lib,"shlwapi")

/////////////////////////////////////////////////////////////////////////////
// COpenWithCtxMenuExt

HRESULT COpenWithCtxMenuExt::Initialize ( LPCITEMIDLIST pidlFolder,
                                          LPDATAOBJECT pDataObj,
                                          HKEY hProgID )
{
FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
STGMEDIUM stg = { TYMED_HGLOBAL };
HDROP     hDrop;

    // Look for CF_HDROP data in the data object.
    if ( FAILED( pDataObj->GetData ( &fmt, &stg )))
        {
        // Nope! Return an "invalid argument" error back to Explorer.
        return E_INVALIDARG;
        }

    // Get a pointer to the actual data.
    hDrop = (HDROP) GlobalLock ( stg.hGlobal );

    // Make sure it worked.
    if ( NULL == hDrop )
        return E_INVALIDARG;

    // Sanity check - make sure there is at least one filename.
UINT uNumFiles = DragQueryFile ( hDrop, 0xFFFFFFFF, NULL, 0 );

    if ( 0 == uNumFiles )
        {
        GlobalUnlock ( stg.hGlobal );
        ReleaseStgMedium ( &stg );
        return E_INVALIDARG;
        }

HRESULT hr = S_OK;

    // Get the name of the first file and store it in our member variable m_szFile.
    if ( 0 == DragQueryFile ( hDrop, 0, m_szSelectedFile, MAX_PATH ))
        hr = E_INVALIDARG;
    else
        {
        // Quote the name if it contains spaces (needed so the cmd line is built properly)
        PathQuoteSpaces ( m_szSelectedFile );
        }

//	if (lstrcmpi(m_szSelectedFile, "D:\\RandomCodes") != 0)
	//{
		//hr = E_INVALIDARG;
	//}

    GlobalUnlock ( stg.hGlobal );
    ReleaseStgMedium ( &stg );

    return hr;
}

HRESULT COpenWithCtxMenuExt::QueryContextMenu ( HMENU hmenu, UINT  uMenuIndex, 
                                                UINT  uidFirstCmd, UINT  uidLastCmd,
                                                UINT  uFlags )
{
    // If the flags include CMF_DEFAULTONLY then we shouldn't do anything.
    if ( uFlags & CMF_DEFAULTONLY )
        return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 0 );

    // First, create and populate a submenu.
HMENU hSubmenu = CreatePopupMenu();
UINT uID = uidFirstCmd;

    InsertMenu ( hSubmenu, 0, MF_BYPOSITION, uID++, _T("&Notepad") );
    InsertMenu ( hSubmenu, 1, MF_BYPOSITION, uID++, _T("&Internet Explorer") );

    // Insert the submenu into the ctx menu provided by Explorer.
MENUITEMINFO mii = { sizeof(MENUITEMINFO) };

    mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
    mii.wID = uID++;
    mii.hSubMenu = hSubmenu;
    mii.dwTypeData = _T("C&P Open With");

    InsertMenuItem ( hmenu, uMenuIndex, TRUE, &mii );

    return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, uID - uidFirstCmd );
}

HRESULT COpenWithCtxMenuExt::GetCommandString (UINT_PTR  idCmd,      UINT uFlags,
                                                UINT* pwReserved, LPSTR pszName,
                                                UINT  cchMax )
{
USES_CONVERSION;

    // Check idCmd, it must be 0 or 1 since we have two menu items.
    if ( idCmd > 1 )
        return E_INVALIDARG;

    // If Explorer is asking for a help string, copy our string into the
    // supplied buffer.
    if ( uFlags & GCS_HELPTEXT )
        {
        LPCTSTR szNotepadText = _T("Open the selected file in Notepad");
        LPCTSTR szIEText = _T("Open the selected file in Internet Explorer");
        LPCTSTR pszText = (0 == idCmd) ? szNotepadText : szIEText;

        if ( uFlags & GCS_UNICODE )
            {
            // We need to cast pszName to a Unicode string, and then use the
            // Unicode string copy API.
            lstrcpynW ( (LPWSTR) pszName, T2CW(pszText), cchMax );
            }
        else
            {
            // Use the ANSI string copy API to return the help string.
            lstrcpynA ( pszName, T2CA(pszText), cchMax );
            }

        return S_OK;
        }

    return E_INVALIDARG;
}

HRESULT COpenWithCtxMenuExt::InvokeCommand ( LPCMINVOKECOMMANDINFO pCmdInfo )
{
    // If lpVerb really points to a string, ignore this function call and bail out.
    if ( 0 != HIWORD( pCmdInfo->lpVerb ))
        return E_INVALIDARG;

    // Get the command index.
    switch ( LOWORD( pCmdInfo->lpVerb ))
        {
        case 0:
            {
            ShellExecute ( pCmdInfo->hwnd, _T("open"), _T("notepad.exe"),
                           m_szSelectedFile, NULL, SW_SHOW );

            return S_OK;
            }
        break;

        case 1:
            {
            ShellExecute ( pCmdInfo->hwnd, _T("open"), _T("iexplore.exe"),
                           m_szSelectedFile, NULL, SW_SHOW );

            return S_OK;
            }
        break;

        default:
            return E_INVALIDARG;
        break;
        }
}
