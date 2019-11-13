// OpenWithCtxMenuExt.cpp : Implementation of COpenWithCtxMenuExt

#include "stdafx.h"
#include <string>
#include <algorithm>
#include "OpenWithExt.h"
#include "OpenWithCtxMenuExt.h"
#include <windows.h>
#include <stdio.h>

#pragma comment(lib,"shlwapi")

/////////////////////////////////////////////////////////////////////////////
// COpenWithCtxMenuExt

std::string getExtension(const char* charString) {
	std::string str(charString);
	std::size_t startIndex = str.find_last_of(".");
	std::string sl = str.substr(startIndex + 1, str.size() - startIndex - 2);
	std::transform(sl.begin(), sl.end(), sl.begin(), ::tolower);
	return sl;
}

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

// VWX extensions
std::string EXT_VWX("vwx");

// PHOTOGRAM extensions
std::string EXT_TIFF("tiff");
std::string EXT_TIF("tif");
std::string EXT_SVG("svg");
std::string EXT_PNG("png");
std::string EXT_JPG("jpg");
std::string EXT_JPEG("jpeg");
std::string EXT_ICO("ico");
std::string EXT_GIF("gif");
std::string EXT_BMP("bmp");

bool isPhotogramType(std::string ext) {
	if (
		ext.compare(EXT_TIFF) == 0 ||
		ext.compare(EXT_TIF) == 0 ||
		ext.compare(EXT_SVG) == 0 ||
		ext.compare(EXT_PNG) == 0 ||
		ext.compare(EXT_JPG) == 0 ||
		ext.compare(EXT_JPEG) == 0 ||
		ext.compare(EXT_ICO) == 0 ||
		ext.compare(EXT_GIF) == 0 ||
		ext.compare(EXT_BMP) == 0
		) return true;
	return false;
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

	std::string ext = getExtension(m_szSelectedFile);

	
	if (ext.compare(EXT_VWX) == 0) {
		InsertMenu(hSubmenu, 0, MF_BYPOSITION, uID++, _T("Generate &PDF"));
		InsertMenu(hSubmenu, 1, MF_BYPOSITION, uID++, _T("Generate 3D &model"));
	}

	if (isPhotogramType(ext)) {
		InsertMenu(hSubmenu, 2, MF_BYPOSITION, uID++, _T("&Photos to 3D model"));
	}
	
    InsertMenu ( hSubmenu, 3, MF_BYPOSITION, uID++, _T("&Share") );
    InsertMenu ( hSubmenu, 4, MF_BYPOSITION, uID++, _T("Shareable &link") );
    
    // Insert the submenu into the ctx menu provided by Explorer.
MENUITEMINFO mii = { sizeof(MENUITEMINFO) };

    mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
    mii.wID = uID++;
    mii.hSubMenu = hSubmenu;
    mii.dwTypeData = _T("Vectorworks Cloud Services");

    InsertMenuItem ( hmenu, uMenuIndex, TRUE, &mii );

    return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, uID - uidFirstCmd );
}

HRESULT COpenWithCtxMenuExt::GetCommandString (UINT_PTR  idCmd,      UINT uFlags,
                                                UINT* pwReserved, LPSTR pszName,
                                                UINT  cchMax )
{
USES_CONVERSION;
    // Check idCmd, it must be 0 or 4 since we have max five menu items.
    if ( idCmd > 4 ) return E_INVALIDARG;

    if ( uFlags & GCS_HELPTEXT ) {
        LPCTSTR pszText = "";

        if ( uFlags & GCS_UNICODE ) lstrcpynW((LPWSTR)pszName, T2CW(pszText), cchMax);
        else lstrcpynA(pszName, T2CA(pszText), cchMax);

        return S_OK;
    }

    return E_INVALIDARG;
}

std::string ENV_PROD("prod");
std::string ENV_BETA("beta");
std::string ENV_QA("qa");
std::string ENV_DEVEL("devel");

const std::string ENV_ARRAY[] = { ENV_PROD, ENV_BETA, ENV_QA, ENV_DEVEL };

int executeAction(std::string action, std::string args)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	std::string base("cmd /K C:\\Users\\\"%username%\"\\AppData\\Local\\Programs\\vectorworks-cloud-services-");
	std::string result("");

	for (int i = 2; i < ENV_ARRAY->size(); i++) {
		result.append(base);
		result.append(ENV_ARRAY[i]);
		result.append(std::string("\\resources\\context_actions\\"));
		result.append(action);
		result.append(std::string(".bat "));
		result.append(args);

		if (i != ENV_ARRAY->size() - 1) {
			result.append(std::string(" || "));
		}
	}

	std::wstring wide_string = std::wstring(result.begin(), result.end());
	const wchar_t* _command = wide_string.c_str();
	wchar_t* command = const_cast<wchar_t*>(_command);

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcessW(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		printf("CreateProcess failed (%d)\n", GetLastError());
		return FALSE;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 0;
}

HRESULT COpenWithCtxMenuExt::InvokeCommand ( LPCMINVOKECOMMANDINFO pCmdInfo ) {
	std::string args(m_szSelectedFile);
	std::string ext = getExtension(m_szSelectedFile);

	if (ext.compare(EXT_VWX) == 0) {
		if (0 != HIWORD(pCmdInfo->lpVerb)) return E_INVALIDARG;

		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { executeAction("pdf_export", args); return S_OK; }
			case 1: { executeAction("distill", args); return S_OK; }
			case 2: { executeAction("share", args); return S_OK; }
			case 3: { executeAction("link", args); return S_OK; }
			default: return E_INVALIDARG;
		}
	}

	else if (isPhotogramType(ext)) {
		if (0 != HIWORD(pCmdInfo->lpVerb)) return E_INVALIDARG;

		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { executeAction("photogram", args); return S_OK; }
			case 1: { executeAction("share", args); return S_OK; }
			case 2: { executeAction("link", args); return S_OK; }
			default: return E_INVALIDARG;
		}
	} else {
		if (0 != HIWORD(pCmdInfo->lpVerb)) return E_INVALIDARG;

		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { executeAction("share", args); return S_OK; }
			case 1: { executeAction("link", args); return S_OK; }
			default: return E_INVALIDARG;
		}
	}

	return E_INVALIDARG;
}
