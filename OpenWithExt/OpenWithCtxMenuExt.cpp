// OpenWithCtxMenuExt.cpp : Implementation of COpenWithCtxMenuExt

#include "stdafx.h"
#include "Shlwapi.h"

#include <algorithm>
#include <array>

#include <string.h>
#include <windows.h>
#include <winuser.h>
#include <stdio.h>
#include <tlhelp32.h>

#include "OpenWithExt.h"
#include "OpenWithCtxMenuExt.h"
#include "BitmapParser.h"
#include "Utils.h"
#include "Constants.h"
#include "Executor.h"

#pragma comment(lib,"shlwapi")

using namespace std;


// COpenWithCtxMenuExt
void COpenWithCtxMenuExt::clear() {
	GlobalUnlock(stg.hGlobal);
	ReleaseStgMedium(&stg);
}


HRESULT COpenWithCtxMenuExt::failAndClear() {
	clear();
	return E_INVALIDARG;
}


HICON COpenWithCtxMenuExt::LoadIcon(const wstring &name) {
	return (HICON)LoadImageW(
		NULL,
		(app_.iconsDir() + name).c_str(),
		IMAGE_ICON,
		16, 16,
		LR_LOADFROMFILE | LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS
	);
}



HRESULT COpenWithCtxMenuExt::Initialize ( LPCITEMIDLIST pidlFolder,
                                          LPDATAOBJECT pDataObj,
                                          HKEY hProgID )
{
FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
this->stg = { TYMED_HGLOBAL };
HDROP     hDrop;
	
	// Look for CF_HDROP data in the data object.
    if ( FAILED( pDataObj->GetData ( &fmt, &stg )))
		return failAndClear(); 

    // Get a pointer to the actual data.
    hDrop = (HDROP) GlobalLock ( stg.hGlobal );
    if ( NULL == hDrop )
		return failAndClear();

	// Validation: number of files selected
	UINT uNumFiles = DragQueryFile ( hDrop, 0xFFFFFFFF, NULL, 0 );

    if ( 0 == uNumFiles || 501 < uNumFiles)
		return failAndClear();

	// Validation: if directories are found and DCC is running
	if (FAILED(app_.setUp()))
		return failAndClear();

	// Validation: selected files are from the synced directory
	for (size_t i = 0; i < uNumFiles; i++) {
		wchar_t *m_szSelectedFile = new wchar_t[MAX_PATH];
		if (0 == DragQueryFileW(hDrop, i, m_szSelectedFile, MAX_PATH))
			return failAndClear();
		
		const wstring entry(m_szSelectedFile);
		if (FAILED(app_.parseFile(entry)))
			return failAndClear();

		delete m_szSelectedFile;
	}

	app_.setSelectionType();
	clear();
	return S_OK;
}


HRESULT COpenWithCtxMenuExt::QueryContextMenu ( HMENU hmenu, UINT  uMenuIndex, 
                                                UINT  uidFirstCmd, UINT  uidLastCmd,
                                                UINT  uFlags )
{
    // If the flags include CMF_DEFAULTONLY then we shouldn't do anything.
    if ( uFlags & CMF_DEFAULTONLY ) return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 0 );

	// Load icons
	HICON ICON_MAIN = LoadIcon(L"application.ico");
	HICON ICON_PDF_EXPORT = LoadIcon(L"pdf.ico");
	HICON ICON_DISTILL = LoadIcon(L"3d.ico");
	HICON ICON_PHOTOGRAM = LoadIcon(L"photo.ico");
	HICON ICON_LINK = LoadIcon(L"link.ico");
	HICON ICON_NAST = LoadIcon(L"nast.ico");
	HICON ICON_UPSAMPLE = LoadIcon(L"upsample.ico");

	HMENU hSubmenu = CreatePopupMenu();
	UINT uID = uidFirstCmd;

	if (Utils::isVWXType(app_.selectionType())) {
		InsertMenuW(hSubmenu, 0, MF_BYPOSITION, uID++, _T(L"Generate &PDF"));
		InsertMenuW(hSubmenu, 1, MF_BYPOSITION, uID++, _T(L"Generate 3D &model"));
		InsertMenuW(hSubmenu, 2, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
		InsertMenuW(hSubmenu, 3, MF_BYPOSITION, uID++, _T(L"Shareable &link"));

		BitmapParser::AddIconToMenuItem(hSubmenu, 0, true, ICON_PDF_EXPORT, true, NULL);
		BitmapParser::AddIconToMenuItem(hSubmenu, 1, true, ICON_DISTILL, true, NULL);
		BitmapParser::AddIconToMenuItem(hSubmenu, 3, true, ICON_LINK, true, NULL);
	}
	else if (Utils::isSingleFolderType(app_.selectionType())) {
		InsertMenuW(hSubmenu, 0, MF_BYPOSITION, uID++, _T(L"&Photos to 3D model"));
		InsertMenuW(hSubmenu, 1, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
		InsertMenuW(hSubmenu, 2, MF_BYPOSITION, uID++, _T(L"Shareable &link"));

		BitmapParser::AddIconToMenuItem(hSubmenu, 0, true, ICON_PHOTOGRAM, true, NULL);
		BitmapParser::AddIconToMenuItem(hSubmenu, 2, true, ICON_LINK, true, NULL);
	}
	else if (Utils::isPhotogramType(app_.selectionType())) {
		InsertMenuW(hSubmenu, 0, MF_BYPOSITION, uID++, _T(L"&Photos to 3D model"));
		InsertMenuW(hSubmenu, 1, MF_BYPOSITION, uID++, _T(L"&Stylize images"));
		InsertMenuW(hSubmenu, 2, MF_BYPOSITION, uID++, _T(L"&Upsample images"));
		InsertMenuW(hSubmenu, 3, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
		InsertMenuW(hSubmenu, 4, MF_BYPOSITION, uID++, _T(L"Shareable &link"));

		BitmapParser::AddIconToMenuItem(hSubmenu, 0, true, ICON_PHOTOGRAM, true, NULL);
		BitmapParser::AddIconToMenuItem(hSubmenu, 1, true, ICON_NAST, true, NULL);
		BitmapParser::AddIconToMenuItem(hSubmenu, 2, true, ICON_UPSAMPLE, true, NULL);
		BitmapParser::AddIconToMenuItem(hSubmenu, 4, true, ICON_LINK, true, NULL);
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

	Executor executor = Executor(app_);
	
	if (Utils::isVWXType(app_.selectionType())) {
		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { return executor.executeAction(L"PDF_EXPORT"); }
			case 1: { return executor.executeAction(L"DISTILL"); }
			case 2: { return executor.executeAction(L"LINK"); }
			// case 3: { Utils::executeAction("SHARE", filesArray_); return S_OK; }
			default: return E_INVALIDARG;
		}
	}
	else if (Utils::isSingleFolderType(app_.selectionType())) {
		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { return executor.executeAction(L"PHOTOGRAM"); }
			case 1: { return executor.executeAction(L"LINK"); }
			default: return E_INVALIDARG;
		}
	}
	else if (Utils::isPhotogramType(app_.selectionType())) {
		switch (LOWORD(pCmdInfo->lpVerb)) {
		case 0: { return executor.executeAction(L"PHOTOGRAM"); }
		case 1: { return executor.executeAction(L"STYLIZE"); }
		case 2: { return executor.executeAction(L"UPSAMPLE"); }
		case 3: { return executor.executeAction(L"LINK"); }
		default: return E_INVALIDARG;
		}
	}
	else {
		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { return executor.executeAction(L"LINK"); }
			default: return E_INVALIDARG;
		}
	}
}
