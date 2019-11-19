// OpenWithCtxMenuExt.cpp : Implementation of COpenWithCtxMenuExt

#include "stdafx.h"
#include <string>
#include <algorithm>
#include "OpenWithExt.h"
#include "OpenWithCtxMenuExt.h"
#include "BitmapParser.h"
#include "Utils.h"
#include <windows.h>
#include <array>
#include <winuser.h>
#include <stdio.h>
#include "Shlwapi.h"

#include <string.h>

#pragma comment(lib,"shlwapi")

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

    if ( 0 == uNumFiles || 501 < uNumFiles)
        {
        GlobalUnlock ( stg.hGlobal );
        ReleaseStgMedium ( &stg );
        return E_INVALIDARG;
        }

	HRESULT hr = S_OK;

	for (size_t i = 0; i < uNumFiles; i++) {
		TCHAR *m_szSelectedFile = new TCHAR[MAX_PATH + 2];

		if (0 == DragQueryFile(hDrop, i, m_szSelectedFile, MAX_PATH)) { hr = E_INVALIDARG; break; }
		else PathQuoteSpaces(m_szSelectedFile);

		filesArray.push_back(std::string(m_szSelectedFile));
		delete[] m_szSelectedFile;
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
    if ( uFlags & CMF_DEFAULTONLY ) return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 0 );

	HMENU hSubmenu = CreatePopupMenu();
	UINT uID = uidFirstCmd;
	
	std::string ext = Utils::getActions(filesArray);
	std::string EXT_VWX("vwx");
	
	if (ext.compare(EXT_VWX) == 0) {
		InsertMenu(hSubmenu, 0, MF_BYPOSITION, uID++, _T("Generate &PDF"));
		InsertMenu(hSubmenu, 1, MF_BYPOSITION , uID++, _T("Generate 3D &model"));
		InsertMenu(hSubmenu, 2, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
		InsertMenu(hSubmenu, 3, MF_BYPOSITION, uID++, _T("Shareable &link"));
		// InsertMenu(hSubmenu, 3, MF_BYPOSITION, uID++, _T("&Share"));

	}
	else if (Utils::isPhotogramType(ext)) {
		InsertMenu(hSubmenu, 0, MF_BYPOSITION, uID++, _T("&Photos to 3D model"));
		InsertMenu(hSubmenu, 1, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
		InsertMenu(hSubmenu, 2, MF_BYPOSITION, uID++, _T("Shareable &link"));
		// InsertMenu(hSubmenu, 2, MF_BYPOSITION, uID++, _T("&Share"));
	}
	else {
		InsertMenu(hSubmenu, 0, MF_BYPOSITION, uID++, _T("Shareable &link"));
		// InsertMenu(hSubmenu, 1, MF_BYPOSITION, uID++, _T("&Share"));
		// SetMenuItemBitmaps(hSubmenu, 0, MF_BYPOSITION, icoBitmap, icoBitmap);
		// Use BitmapParser::AddIconToMenuItem
	}
    
    
    // Insert the submenu into the ctx menu provided by Explorer.
	MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
    mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
    mii.wID = uID++;
    mii.hSubMenu = hSubmenu;
    mii.dwTypeData = _T("Vectorworks Cloud Services");

    InsertMenuItem ( hmenu, uMenuIndex, TRUE, &mii );
	// SetMenuItemBitmaps(hmenu, mii.wID, MF_BYCOMMAND, hbmp, hbmp);

	HBITMAP* hbmp = NULL;
	HICON hicon = (HICON)LoadImage(NULL, "C:\\Users\\Simeon Rolev\\AppData\\Local\\Programs\\vectorworks-cloud-services-devel\\resources\\context_actions\\icon.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE | LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);
	
	IWICImagingFactory *pFactory = NULL;
	BitmapParser::AddIconToMenuItem(hmenu, mii.wID, false, hicon, true, hbmp);

    return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, uID - uidFirstCmd );
}

HRESULT COpenWithCtxMenuExt::GetCommandString (UINT_PTR  idCmd,      UINT uFlags,
                                                UINT* pwReserved, LPSTR pszName,
                                                UINT  cchMax )
{
USES_CONVERSION;
    // Check idCmd, it must be 0 or 4 since we have max five menu items.
    if ( idCmd > 3 ) return E_INVALIDARG;

    if ( uFlags & GCS_HELPTEXT ) {
        LPCTSTR pszText = "";

        if ( uFlags & GCS_UNICODE ) lstrcpynW((LPWSTR)pszName, T2CW(pszText), cchMax);
        else lstrcpynA(pszName, T2CA(pszText), cchMax);

        return S_OK;
    }

    return E_INVALIDARG;
}


HRESULT COpenWithCtxMenuExt::InvokeCommand ( LPCMINVOKECOMMANDINFO pCmdInfo ) {
	std::string ext = Utils::getActions(filesArray);
	std::string EXT_VWX("vwx");
	if (ext.compare(EXT_VWX) == 0) {
		if (0 != HIWORD(pCmdInfo->lpVerb)) return E_INVALIDARG;

		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { Utils::executeAction("PDF_EXPORT", filesArray); return S_OK; }
			case 1: { Utils::executeAction("DISTILL", filesArray); return S_OK; }
			case 2: { Utils::executeAction("LINK", filesArray); return S_OK; }
			// case 3: { Utils::executeAction("SHARE", filesArray); return S_OK; }
			default: return E_INVALIDARG;
		}
	}

	else if (Utils::isPhotogramType(ext)) {
		if (0 != HIWORD(pCmdInfo->lpVerb)) return E_INVALIDARG;

		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { Utils::executeAction("PHOTOGRAM", filesArray); return S_OK; }
			case 1: { Utils::executeAction("LINK", filesArray); return S_OK; }
			// case 2: { Utils::executeAction("SHARE", filesArray); return S_OK; }
			default: return E_INVALIDARG;
		}
	} else {
		if (0 != HIWORD(pCmdInfo->lpVerb)) return E_INVALIDARG;

		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { Utils::executeAction("LINK", filesArray); return S_OK; }
			// case 1: { Utils::executeAction("SHARE", filesArray); return S_OK; }
			default: return E_INVALIDARG;
		}
	}

	return E_INVALIDARG;
}
