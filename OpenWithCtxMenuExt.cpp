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
#include "Constants.h"


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
	
    if ( FAILED( pDataObj->GetData ( &fmt, &stg ))) return E_INVALIDARG; // Look for CF_HDROP data in the data object.

    // Get a pointer to the actual data.
    hDrop = (HDROP) GlobalLock ( stg.hGlobal );
    if ( NULL == hDrop ) return E_INVALIDARG;

	if (!Utils::serviceIsRunning(L"Vectorworks Cloud Services Background Service.exe")) return E_INVALIDARG;

	// Get installation directories
	if (FAILED(this->setDirs())) return E_INVALIDARG;
	if (FAILED(Utils::getSyncedFolder(LOC_APP, ENV_ARRAY[ENV], SYNCED_DIR))) return E_INVALIDARG;
	

	// Check number of files selected
	UINT uNumFiles = DragQueryFile ( hDrop, 0xFFFFFFFF, NULL, 0 );

    if ( 0 == uNumFiles || 501 < uNumFiles) {
        GlobalUnlock ( stg.hGlobal );
        ReleaseStgMedium ( &stg );
        return E_INVALIDARG;
    }

	HRESULT hr = S_OK;

	// Add files array that holds the multiple selection
	for (size_t i = 0; i < uNumFiles; i++) {
		wchar_t *m_szSelectedFile = new wchar_t[MAX_PATH + 2];

		if (0 == DragQueryFileW(hDrop, i, m_szSelectedFile, MAX_PATH)) { hr = E_INVALIDARG; break; }
		else PathQuoteSpacesW(m_szSelectedFile);

		// Check if root folder
		std::wstring f(m_szSelectedFile);
		if (f.substr(1, SYNCED_DIR.size()).compare(SYNCED_DIR) != 0) return E_INVALIDARG;

		filesArray.push_back(std::wstring(m_szSelectedFile));
		delete[] m_szSelectedFile;
	}

	if (SUCCEEDED(hr)) {
		Utils::getActions(SELECTION_TYPE, filesArray);
	}

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

	// Load icons
	HICON ICON_MAIN       = (HICON)LoadImageW(NULL, (ICONS_DIR + L"dcc.ico").c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE | LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);
	HICON ICON_PDF_EXPORT = (HICON)LoadImageW(NULL, (ICONS_DIR + L"pdf.ico").c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE | LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);
	HICON ICON_DISTILL    = (HICON)LoadImageW(NULL, (ICONS_DIR + L"3d.ico").c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE | LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);
	HICON ICON_PHOTOGRAM  = (HICON)LoadImageW(NULL, (ICONS_DIR + L"photo.ico").c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE | LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);
	HICON ICON_LINK       = (HICON)LoadImageW(NULL, (ICONS_DIR + L"link.ico").c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE | LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);
	HICON ICON_NAST       = (HICON)LoadImageW(NULL, (ICONS_DIR + L"nast.ico").c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE | LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);
	
	HMENU hSubmenu = CreatePopupMenu();
	UINT uID = uidFirstCmd;

	if (Utils::isVWXType(SELECTION_TYPE)) {
		InsertMenuW(hSubmenu, 0, MF_BYPOSITION, uID++, _T(L"Generate &PDF"));
		InsertMenuW(hSubmenu, 1, MF_BYPOSITION, uID++, _T(L"Generate 3D &model"));
		InsertMenuW(hSubmenu, 2, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
		InsertMenuW(hSubmenu, 3, MF_BYPOSITION, uID++, _T(L"Shareable &link"));

		BitmapParser::AddIconToMenuItem(hSubmenu, 0, true, ICON_PDF_EXPORT, true, NULL);
		BitmapParser::AddIconToMenuItem(hSubmenu, 1, true, ICON_DISTILL, true, NULL);
		BitmapParser::AddIconToMenuItem(hSubmenu, 3, true, ICON_LINK, true, NULL);
	}
	else if (Utils::isPhotogramType(SELECTION_TYPE)) {
		InsertMenuW(hSubmenu, 0, MF_BYPOSITION, uID++, _T(L"&Photos to 3D model"));
		InsertMenuW(hSubmenu, 1, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
		InsertMenuW(hSubmenu, 2, MF_BYPOSITION, uID++, _T(L"Shareable &link"));

		BitmapParser::AddIconToMenuItem(hSubmenu, 0, true, ICON_PHOTOGRAM, true, NULL);
		BitmapParser::AddIconToMenuItem(hSubmenu, 2, true, ICON_LINK, true, NULL);
	}
	else {
		InsertMenuW(hSubmenu, 0, MF_BYPOSITION, uID++, _T(L"Shareable &link"));
		BitmapParser::AddIconToMenuItem(hSubmenu, 0, true, ICON_LINK, true, NULL);
	}
    
    // Insert the submenu into the ctx menu provided by Explorer.
	MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
    mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
    mii.wID = uID++;
    mii.hSubMenu = hSubmenu;
    mii.dwTypeData = _T("Vectorworks Cloud Services");

    InsertMenuItem ( hmenu, uMenuIndex, TRUE, &mii );
	BitmapParser::AddIconToMenuItem(hmenu, mii.wID, false, ICON_MAIN, true, NULL);

    return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, uID - uidFirstCmd );
}


HRESULT COpenWithCtxMenuExt::GetCommandString (UINT_PTR  idCmd,      UINT uFlags,
                                                UINT* pwReserved, LPSTR pszName,
                                                UINT  cchMax )
{
USES_CONVERSION;
    // Check idCmd, it must be 0 or 4 since we have max 4 menu items.
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
	if (0 != HIWORD(pCmdInfo->lpVerb)) return E_INVALIDARG;
	
	if (Utils::isVWXType(SELECTION_TYPE)) {
		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { return Utils::executeAction(BG_SRV_CMD, ENV, L"PDF_EXPORT", filesArray); }
			case 1: { return Utils::executeAction(BG_SRV_CMD, ENV, L"DISTILL", filesArray); }
			case 2: { return Utils::executeAction(BG_SRV_CMD, ENV, L"LINK", filesArray); }
			// case 3: { Utils::executeAction("SHARE", filesArray); return S_OK; }
			default: return E_INVALIDARG;
		}
	}
	else if (Utils::isPhotogramType(SELECTION_TYPE)) {
		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { return Utils::executeAction(BG_SRV_CMD, ENV, L"PHOTOGRAM", filesArray); }
			case 1: { return Utils::executeAction(BG_SRV_CMD, ENV, L"LINK", filesArray); }
			default: return E_INVALIDARG;
		}
	} else {
		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { return Utils::executeAction(BG_SRV_CMD, ENV, L"LINK", filesArray); }
			default: return E_INVALIDARG;
		}
	}
}
