// OpenWithCtxMenuExt.h : Declaration of the COpenWithCtxMenuExt

#ifndef __OPENWITHCTXMENUEXT_H_
#define __OPENWITHCTXMENUEXT_H_

#include "resource.h"       // main symbols
#include "Utils.h"
#include <string>
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// COpenWithCtxMenuExt

class ATL_NO_VTABLE COpenWithCtxMenuExt : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<COpenWithCtxMenuExt, &CLSID_OpenWithCtxMenuExt>,
    public IShellExtInit,
    public IContextMenu
{
public:
    COpenWithCtxMenuExt()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_OPENWITHCTXMENUEXT)
DECLARE_NOT_AGGREGATABLE(COpenWithCtxMenuExt)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(COpenWithCtxMenuExt)
    COM_INTERFACE_ENTRY(IShellExtInit)
    COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()

public:
    // IShellExtInit
    STDMETHOD(Initialize)(LPCITEMIDLIST, LPDATAOBJECT, HKEY);

    // IContextMenu
    STDMETHOD(GetCommandString)(UINT_PTR, UINT, UINT*, LPSTR, UINT);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO);
    STDMETHOD(QueryContextMenu)(HMENU, UINT, UINT, UINT, UINT);

protected:
    TCHAR m_szSelectedFile[MAX_PATH+2];
	std::vector<std::wstring> filesArray;

private:
	int ENV;
	std::wstring BASE_DIR;

	std::wstring RESOURCES_DIR;
	std::wstring SERVER_DIR;
	std::wstring ICONS_DIR;
	std::wstring BG_SRV_CMD;

	std::wstring SELECTION_TYPE;

public:
	HRESULT setDirs() {
		HRESULT hr_base = Utils::getDCCRoot(BASE_DIR);
		HRESULT hr_env = Utils::getEnv(BASE_DIR, ENV);

		if (FAILED(hr_base) || FAILED(hr_env)) {
			return E_INVALIDARG;
		}

		RESOURCES_DIR = BASE_DIR + L"resources\\";
		SERVER_DIR = RESOURCES_DIR + L"server\\";
		ICONS_DIR = RESOURCES_DIR + L"context_actions\\";

		BG_SRV_CMD = Utils::wrapSpacesForCMD(SERVER_DIR, L"\\") + L"\"Vectorworks Cloud Services Background Service\".exe";

		return S_OK;
	};
};

#endif //__OPENWITHCTXMENUEXT_H_
