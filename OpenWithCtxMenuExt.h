// OpenWithCtxMenuExt.h : Declaration of the COpenWithCtxMenuExt

#ifndef __OPENWITHCTXMENUEXT_H_
#define __OPENWITHCTXMENUEXT_H_

#include "resource.h"       // main symbols
#include <string>
#include <vector>

#include "Constants.h"


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

private:
	STGMEDIUM stg;

	int ENV = -1;

	std::wstring LOC_APP;
	std::wstring LOC_APP_RPOGS;

	std::wstring BASE_DIR;
	std::wstring SYNCED_DIR;
	std::wstring RESOURCES_DIR; // Others	
	std::wstring SERVER_DIR;
	std::wstring ICONS_DIR;
	std::wstring BG_SRV_CMD;

	std::wstring SELECTION_TYPE;
	std::vector<std::wstring> filesArray;

	HRESULT _setDirs();
	HRESULT _setEnv();
	HRESULT _getSyncedFolder();
public:
	HRESULT setUp();
	HRESULT failAndClear();

	HICON LoadIcon(const std::wstring &name);

	// Getters
	int env() { return ENV; }
	std::wstring localApp() { return LOC_APP; }
	std::wstring localAppProgs() { return LOC_APP_RPOGS; }
	std::wstring baseDir() { return BASE_DIR; }
	std::wstring iconsDir() { return ICONS_DIR; }
	std::wstring label() { return ENV_ARRAY[env()]; }
};

#endif //__OPENWITHCTXMENUEXT_H_
