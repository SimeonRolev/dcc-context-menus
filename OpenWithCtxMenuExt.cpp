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

// COpenWithCtxMenuExt


HRESULT COpenWithCtxMenuExt::_retrieveService() {
	std::wstring sAppName = L"Vectorworks Cloud Services Background Service.exe";

	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(PROCESSENTRY32W);

	const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	bool a = Process32FirstW(snapshot, &entry);

	do {
		if (!_wcsicmp(entry.szExeFile, sAppName.c_str())) {
			const auto snapmodule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, entry.th32ProcessID);

			MODULEENTRY32W mod;
			mod.dwSize = sizeof(MODULEENTRY32W);
			bool m = Module32FirstW(snapmodule, &mod);

			std::wstring exePath(mod.szExePath);
			std::wstring serverPath = exePath.substr(0, exePath.rfind(L"\\"));
			std::wstring resourcesPath = serverPath.substr(0, serverPath.rfind(L"\\"));
			std::wstring basePath = resourcesPath.substr(0, resourcesPath.rfind(L"\\"));
			std::wstring programsPath = basePath.substr(0, basePath.rfind(L"\\"));
			std::wstring localPath = programsPath.substr(0, programsPath.rfind(L"\\"));

			// Detected environment
			std::wstring installedApp = basePath.substr(programsPath.length() + 1, basePath.length());
			if (installedApp.compare(L"vectorworks-cloud-services") == 0) ENV = 0;
			else {
				for (int i = 1; i < ENV_ARRAY->size(); i++) {
					if (installedApp.rfind(ENV_ARRAY[i]) != std::wstring::npos) ENV = i;
				}
			}

			if (ENV < 0) return E_INVALIDARG;

			// Setters
			LOC_APP = localPath + L"\\";
			LOC_APP_RPOGS = programsPath + L"\\";
			BASE_DIR = basePath + L"\\";
			RESOURCES_DIR = resourcesPath + L"\\";
			SERVER_DIR = serverPath + L"\\";
			ICONS_DIR = resourcesPath + L"\\context_actions\\icons\\";
			BG_SRV_CMD = Utils::wrapSpacesForCMD(SERVER_DIR, L"\\") + L"\"Vectorworks Cloud Services Background Service\".exe";

			CloseHandle(snapshot);
			CloseHandle(snapmodule);
			return S_OK;
		}
	} while (Process32NextW(snapshot, &entry));

	CloseHandle(snapshot);
	return E_INVALIDARG;
};


// Requires that you have BASE_DIR and ENV set
HRESULT COpenWithCtxMenuExt::_getSyncedFolders() {
	if (localApp().empty()) return E_INVALIDARG;
	if (env() < 0) return E_INVALIDARG;

	std::wstring appName = L"Vectorworks Cloud Services";
	if (env() > 0) appName.append(L" ").append(label());

	std::wstring path = localApp() +
		+ L"Nemetschek\\" + appName
		+ L"\\Cache\\" + ACTIVE_SESSION_FN;

	if (
		SUCCEEDED(Utils::readJsonFile(path, "rootFolder", SYNCED_DIR)) &&
		SUCCEEDED(Utils::readJsonFile(path, "dropboxFolder", DROPBOX_DIR))
	) return S_OK;
	return E_INVALIDARG;
}


bool COpenWithCtxMenuExt::childNodeOf(const std::wstring &root, const std::wstring &entry) {
	if (
		root.empty() ||
		entry.compare(root) == 0 ||  // Should not be the root itself
		entry.size() < root.size() ||  // entry length should be larger than the root length
		entry.substr(0, root.size()).compare(root) != 0  // child check
	) return false;
	return true;
}


HRESULT COpenWithCtxMenuExt::setUp() {
	if (FAILED(this->_retrieveService()) ||
		FAILED(this->_getSyncedFolders())
	) return E_INVALIDARG;
	return S_OK;
}


void COpenWithCtxMenuExt::clear() {
	GlobalUnlock(stg.hGlobal);
	ReleaseStgMedium(&stg);
}


HRESULT COpenWithCtxMenuExt::failAndClear() {
	clear();
	return E_INVALIDARG;
}


HICON COpenWithCtxMenuExt::LoadIcon(const std::wstring &name) {
	return (HICON)LoadImageW(
		NULL,
		(iconsDir() + name).c_str(),
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

	// Validation: if directories are found and DCC is running
	if (FAILED(this->setUp()))
		return failAndClear();

	// Validation: number of files selected
	UINT uNumFiles = DragQueryFile ( hDrop, 0xFFFFFFFF, NULL, 0 );

    if ( 0 == uNumFiles || 501 < uNumFiles)
		return failAndClear();

	// Validation: selected files are from the synced directory
	for (size_t i = 0; i < uNumFiles; i++) {
		wchar_t *m_szSelectedFile = new wchar_t[MAX_PATH];
		if (0 == DragQueryFileW(hDrop, i, m_szSelectedFile, MAX_PATH))
			return failAndClear();
		
		std::wstring entry(m_szSelectedFile);

		if (!childNodeOf(SYNCED_DIR, entry) && !childNodeOf(DROPBOX_DIR, entry))
			return failAndClear();

		if (Utils::isFolder(entry)) entry.append(L"\\\\");
		std::wstring result = L"\"" + entry + L"\"";
		filesArray.push_back(result);
	}

	Utils::getActions(SELECTION_TYPE, filesArray);
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
	HICON ICON_MAIN = LoadIcon(L"dcc.ico");
	HICON ICON_PDF_EXPORT = LoadIcon(L"pdf.ico");
	HICON ICON_DISTILL = LoadIcon(L"3d.ico");
	HICON ICON_PHOTOGRAM = LoadIcon(L"photo.ico");
	HICON ICON_LINK = LoadIcon(L"link.ico");
	HICON ICON_NAST = LoadIcon(L"nast.ico");

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

	Executor executor = Executor(BG_SRV_CMD, ENV, filesArray);
	
	if (Utils::isVWXType(SELECTION_TYPE)) {
		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { return executor.executeAction(L"PDF_EXPORT"); }
			case 1: { return executor.executeAction(L"DISTILL"); }
			case 2: { return executor.executeAction(L"LINK"); }
			// case 3: { Utils::executeAction("SHARE", filesArray); return S_OK; }
			default: return E_INVALIDARG;
		}
	}
	else if (Utils::isPhotogramType(SELECTION_TYPE)) {
		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { return executor.executeAction(L"PHOTOGRAM"); }
			case 1: { return executor.executeAction(L"LINK"); }
			default: return E_INVALIDARG;
		}
	} else {
		switch (LOWORD(pCmdInfo->lpVerb)) {
			case 0: { return executor.executeAction(L"LINK"); }
			default: return E_INVALIDARG;
		}
	}
}
