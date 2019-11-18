

#include "stdafx.h"
#include "BitmapParser.h"

#include <windows.h>
#include <windowsx.h>           // For WM_COMMAND handling macros
#include <shlobj.h>             // For shell
#include <shlwapi.h>            // QISearch, easy way to implement QI
#include <commctrl.h>
#include "resource.h"

#pragma comment(lib, "shlwapi") // Default link libs do not include this.
#pragma comment(lib, "comctl32")
#pragma comment(lib, "WindowsCodecs")    // WIC

// Set up common controls v6 the easy way.
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")


HINSTANCE g_hinst = 0;

BitmapParser::BitmapParser()
{
}


BitmapParser::~BitmapParser()
{
}


void BitmapParser::InitBitmapInfo(__out_bcount(cbInfo) BITMAPINFO *pbmi, ULONG cbInfo, LONG cx, LONG cy, WORD bpp)
{
	ZeroMemory(pbmi, cbInfo);
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biCompression = BI_RGB;

	pbmi->bmiHeader.biWidth = cx;
	pbmi->bmiHeader.biHeight = cy;
	pbmi->bmiHeader.biBitCount = bpp;
}

HRESULT BitmapParser::Create32BitHBITMAP(HDC hdc, const SIZE *psize, __deref_opt_out void **ppvBits, __out HBITMAP* phBmp)
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

HRESULT BitmapParser::AddBitmapToMenuItem(HMENU hmenu, int iItem, BOOL fByPosition, HBITMAP hbmp)
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

HRESULT BitmapParser::AddIconToMenuItem(__in IWICImagingFactory *pFactory,
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