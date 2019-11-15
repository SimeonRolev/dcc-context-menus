// OpenWithCtxMenuExt.cpp : Implementation of COpenWithCtxMenuExt

#include "stdafx.h"
#include <string>
#include <algorithm>
#include "OpenWithExt.h"
#include "OpenWithCtxMenuExt.h"
#include <windows.h>
#include <winuser.h>
#include <stdio.h>
#include "Shlwapi.h"
#include <Lmcons.h>
#include <string.h>

#pragma comment(lib,"shlwapi")

/////////////////////////////////////////////////////////////////////////////


#define STRICT_TYPED_ITEMIDS    // In case you use IDList, you want this on for better type safety.
#include <windows.h>
#include <windowsx.h>           // For WM_COMMAND handling macros
#include <shlobj.h>             // For shell
#include <shlwapi.h>            // QISearch, easy way to implement QI
#include <commctrl.h>
#include <wincodec.h>           // WIC
#include "resource.h"

#pragma comment(lib, "shlwapi") // Default link libs do not include this.
#pragma comment(lib, "comctl32")
#pragma comment(lib, "WindowsCodecs")    // WIC

// Set up common controls v6 the easy way.
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

HINSTANCE g_hinst = 0;

// AddIconToMenuItem and its supporting functions.
// Note: BufferedPaintInit/BufferedPaintUnInit should be called to
// improve performance.
// In this sample they are called in _OnInitDlg/_OnDestroyDlg.
// In a full application you would call these during WM_NCCREATE/WM_NCDESTROY.

typedef DWORD ARGB;

void InitBitmapInfo(__out_bcount(cbInfo) BITMAPINFO *pbmi, ULONG cbInfo, LONG cx, LONG cy, WORD bpp)
{
	ZeroMemory(pbmi, cbInfo);
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biCompression = BI_RGB;

	pbmi->bmiHeader.biWidth = cx;
	pbmi->bmiHeader.biHeight = cy;
	pbmi->bmiHeader.biBitCount = bpp;
}

HRESULT Create32BitHBITMAP(HDC hdc, const SIZE *psize, __deref_opt_out void **ppvBits, __out HBITMAP* phBmp)
{
	*phBmp = NULL;

	BITMAPINFO bmi;
	InitBitmapInfo(&bmi, sizeof(bmi), psize->cx, psize->cy, 32);

	HDC hdcUsed = hdc ? hdc : GetDC(NULL);
	if (hdcUsed)
	{
		*phBmp = CreateDIBSection(hdcUsed, &bmi, DIB_RGB_COLORS, ppvBits, NULL, 0);
		if (hdc != hdcUsed)
		{
			ReleaseDC(NULL, hdcUsed);
		}
	}
	return (NULL == *phBmp) ? E_OUTOFMEMORY : S_OK;
}

HRESULT AddBitmapToMenuItem(HMENU hmenu, int iItem, BOOL fByPosition, HBITMAP hbmp)
{
	HRESULT hr = E_FAIL;

	MENUITEMINFO mii = { sizeof(mii) };
	mii.fMask = MIIM_BITMAP;
	mii.hbmpItem = hbmp;
	if (SetMenuItemInfo(hmenu, iItem, fByPosition, &mii))
	{
		hr = S_OK;
	}

	return hr;
}

HRESULT AddIconToMenuItem(__in IWICImagingFactory *pFactory,
	HMENU hmenu, int iMenuItem, BOOL fByPosition, HICON hicon, BOOL fAutoDestroy, __out_opt HBITMAP *phbmp)
{
	HBITMAP hbmp = NULL;

	IWICBitmap *pBitmap;
	HRESULT hr = pFactory->CreateBitmapFromHICON(hicon, &pBitmap);
	if (SUCCEEDED(hr))
	{
		IWICFormatConverter *pConverter;
		hr = pFactory->CreateFormatConverter(&pConverter);
		if (SUCCEEDED(hr))
		{
			hr = pConverter->Initialize(pBitmap, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
			if (SUCCEEDED(hr))
			{
				UINT cx, cy;
				hr = pConverter->GetSize(&cx, &cy);
				if (SUCCEEDED(hr))
				{
					const SIZE sizIcon = { (int)cx, -(int)cy };
					BYTE *pbBuffer;
					hr = Create32BitHBITMAP(NULL, &sizIcon, reinterpret_cast<void **>(&pbBuffer), &hbmp);
					if (SUCCEEDED(hr))
					{
						const UINT cbStride = cx * sizeof(ARGB);
						const UINT cbBuffer = cy * cbStride;
						hr = pConverter->CopyPixels(NULL, cbStride, cbBuffer, pbBuffer);
					}
				}
			}

			pConverter->Release();
		}

		pBitmap->Release();
	}

	if (SUCCEEDED(hr))
	{
		hr = AddBitmapToMenuItem(hmenu, iMenuItem, fByPosition, hbmp);
	}

	if (FAILED(hr))
	{
		DeleteObject(hbmp);
		hbmp = NULL;
	}

	if (fAutoDestroy)
	{
		DestroyIcon(hicon);
	}

	if (phbmp)
	{
		*phbmp = hbmp;
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////////
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
    // First, create and populate a submenu
HMENU hSubmenu = CreatePopupMenu();
UINT uID = uidFirstCmd;

	HBITMAP icoBitmap = (HBITMAP)LoadImage(NULL, "C:\\Users\\Simeon Rolev\\AppData\\Local\\Programs\\vectorworks-cloud-services-devel\\resources\\context_actions\\BLK.BMP", IMAGE_BITMAP, 16, 16, LR_LOADFROMFILE);
	HBITMAP pdfExportBitmap = (HBITMAP)LoadImage(NULL, "C:\\Users\\Simeon Rolev\\AppData\\Local\\Programs\\vectorworks-cloud-services-devel\\resources\\context_actions\\mono.bmp", IMAGE_BITMAP, 16, 16, LR_LOADTRANSPARENT | LR_LOADFROMFILE);
	HBITMAP dccBitmap = (HBITMAP)LoadImage(NULL, "C:\\Users\\Simeon Rolev\\AppData\\Local\\Programs\\vectorworks-cloud-services-devel\\resources\\context_actions\\DCC-16x16-alpha.bmp", IMAGE_BITMAP, 16, 16, LR_LOADTRANSPARENT | LR_LOADFROMFILE);
	// HICON hicon = (HICON)LoadImage(NULL, "C:\\Users\\Simeon Rolev\\AppData\\Local\\Programs\\vectorworks-cloud-services-devel\\resources\\context_actions\\icon.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
	HICON hicon = (HICON)LoadImage(NULL, "C:\\Users\\Simeon Rolev\\AppData\\Local\\Programs\\vectorworks-cloud-services-devel\\resources\\context_actions\\check_16x16.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE | LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);

// HBITMAP LoadBitmapW(NULL, );

	//////////////////////////////////
	HBITMAP hbmp = NULL;

	IWICImagingFactory *pFactory;
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
	if (SUCCEEDED(hr))
	{
		IWICBitmap *pBitmap;
		hr = pFactory->CreateBitmapFromHICON(hicon, &pBitmap);
		if (SUCCEEDED(hr))
		{
			UINT cx, cy;
			hr = pBitmap->GetSize(&cx, &cy);
			if (SUCCEEDED(hr))
			{
				const SIZE sizIcon = { (int)cx, -(int)cy };
				BYTE *pbBuffer;
				hr = Create32BitHBITMAP(NULL, &sizIcon,
					reinterpret_cast<void
					**>(&pbBuffer), &hbmp);
				if (SUCCEEDED(hr))
				{
					const UINT cbStride = cx * sizeof(ARGB);
					const UINT cbBuffer = cy * cbStride;
					hr = pBitmap->CopyPixels(NULL, cbStride,
						cbBuffer, pbBuffer);
				}
			}
			pBitmap->Release();
		}
		pFactory->Release();
	}


	//////////////////////////////
	
	std::string ext = getExtension(m_szSelectedFile);
	
	if (ext.compare(EXT_VWX) == 0) {
		InsertMenu(hSubmenu, 0, MF_BYPOSITION, uID++, _T("Generate &PDF"));
		InsertMenu(hSubmenu, 1, MF_BYPOSITION, uID++, _T("Generate 3D &model"));
		InsertMenu(hSubmenu, 2, MF_BYPOSITION, uID++, _T("&Share"));
		InsertMenu(hSubmenu, 3, MF_BYPOSITION, uID++, _T("Shareable &link"));

		SetMenuItemBitmaps(hSubmenu, 0, MF_BYPOSITION, pdfExportBitmap, pdfExportBitmap);
		SetMenuItemBitmaps(hSubmenu, 1, MF_BYPOSITION, dccBitmap, dccBitmap);
		SetMenuItemBitmaps(hSubmenu, 2, MF_BYPOSITION, hbmp, hbmp);
		SetMenuItemBitmaps(hSubmenu, 3, MF_BYPOSITION, icoBitmap, icoBitmap);
	}
	else if (isPhotogramType(ext)) {
		InsertMenu(hSubmenu, 0, MF_BYPOSITION, uID++, _T("&Photos to 3D model"));
		InsertMenu(hSubmenu, 1, MF_BYPOSITION, uID++, _T("&Share"));
		InsertMenu(hSubmenu, 2, MF_BYPOSITION, uID++, _T("Shareable &link"));

		SetMenuItemBitmaps(hSubmenu, 0, MF_BYPOSITION, icoBitmap, icoBitmap);
		SetMenuItemBitmaps(hSubmenu, 1, MF_BYPOSITION, icoBitmap, icoBitmap);
		SetMenuItemBitmaps(hSubmenu, 2, MF_BYPOSITION, icoBitmap, icoBitmap);
	}
	else {
		InsertMenu(hSubmenu, 0, MF_BYPOSITION, uID++, _T("&Share"));
		InsertMenu(hSubmenu, 1, MF_BYPOSITION, uID++, _T("Shareable &link"));

		SetMenuItemBitmaps(hSubmenu, 0, MF_BYPOSITION, icoBitmap, icoBitmap);
		SetMenuItemBitmaps(hSubmenu, 1, MF_BYPOSITION, icoBitmap, icoBitmap);
	}
    
	// SetMenuItemBitmaps(hmenu, 0, MF_BYPOSITION, icoBitmap, icoBitmap); // Sets to the lower menu item
	// SetMenuItemBitmaps(hmenu, 2, MF_BYCOMMAND, icoBitmap, icoBitmap);
    
    // Insert the submenu into the ctx menu provided by Explorer.
MENUITEMINFO mii = { sizeof(MENUITEMINFO) };

    mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
    mii.wID = uID++;
    mii.hSubMenu = hSubmenu;
    mii.dwTypeData = _T("Vectorworks Cloud Services");

    InsertMenuItem ( hmenu, uMenuIndex, TRUE, &mii );
	SetMenuItemBitmaps(hmenu, mii.wID, MF_BYCOMMAND, icoBitmap, icoBitmap);

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

wchar_t* stringToWChar(std::string s) {
	std::wstring wide_string = std::wstring(s.begin(), s.end());
	const wchar_t* _res = wide_string.c_str();
	return const_cast<wchar_t*>(_res);
}

std::string getUsername() {
	char _username[UNLEN + 1];
	DWORD size = UNLEN + 1;
	bool u = GetUserName((TCHAR*)_username, &size);

	return std::string(_username);
}

std::string _getBaseRoot() {
	std::string BASE_ROOT("C:\\Users\\");
	BASE_ROOT.append(getUsername());
	BASE_ROOT.append("\\AppData\\Local\\Programs\\vectorworks-cloud-services-");
	return BASE_ROOT;
}

std::string getEnv() {
	const std::string BASE_ROOT(_getBaseRoot());
	
	DWORD dwAttr;
	std::string appDir;

	for (int i = 0; i < ENV_ARRAY->size(); i++) {
		appDir = BASE_ROOT + ENV_ARRAY[i];
		dwAttr = GetFileAttributes(appDir.c_str());

		if (dwAttr != 0xffffffff && (dwAttr & FILE_ATTRIBUTE_DIRECTORY)) return ENV_ARRAY[i];
	}

	return ENV_PROD;
}


std::string _getBaseRootCMD() {
	std::string BASE_DIR("C:\\Users\\\"");
	BASE_DIR.append(getUsername());
	BASE_DIR.append("\"\\AppData\\Local\\Programs\\vectorworks-cloud-services-");
	return BASE_DIR;
}

const std::string ENV(getEnv());
const std::string BASE_DIR(_getBaseRootCMD() + ENV);
const std::string CONTEXT_ACTIONS_DIR(BASE_DIR + std::string("\\resources\\context_actions\\"));


int executeAction(std::string action, std::string args)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	std::string result("");

	result.append("cmd /K ");
	result.append(CONTEXT_ACTIONS_DIR);
	result.append(action);
	result.append(".bat ");
	result.append(args);

	std::wstring wide_string = std::wstring(result.begin(), result.end());
	const wchar_t* _command = wide_string.c_str();
	wchar_t* command = const_cast<wchar_t*>(_command);

	// wchar_t* command = stringToWChar(result);

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
