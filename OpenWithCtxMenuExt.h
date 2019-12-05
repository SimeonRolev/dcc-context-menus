// OpenWithCtxMenuExt.h : Declaration of the COpenWithCtxMenuExt

#ifndef __OPENWITHCTXMENUEXT_H_
#define __OPENWITHCTXMENUEXT_H_

#include "resource.h"       // main symbols
#include <string>
#include <vector>

#include "Constants.h"

using namespace std;


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

	int env_ = -1;

	wstring locApp_;
	wstring locAppProgs_;

	wstring baseDir_;
	wstring resourcesDir_; // Others	
	wstring serverDir_;
	wstring iconsDir_;
	wstring bgSrvCmd_;

	// Synced dirs
	wstring syncDir_;
	wstring dropboxDir_;

	wstring selectionType_;
	vector<wstring> filesArray_;

	HRESULT _retrieveService();
	HRESULT _getSyncedFolders();
public:
	HRESULT setUp();
	void clear();
	HRESULT failAndClear();

	HICON LoadIcon(const wstring &name);

	// Getters
	int env() { return env_; }
	wstring localApp() { return locApp_; }
	wstring localAppProgs() { return locAppProgs_; }
	wstring baseDir() { return baseDir_; }
	wstring iconsDir() { return iconsDir_; }
	wstring label() { return ENV_ARRAY[env()]; }

	wstring syncDir() { return syncDir_; }
	wstring dropboxDir() { return dropboxDir_; }

	// Setters
	void setSelectionType(wstring input) { this->selectionType_ = input; }
};

#endif //__OPENWITHCTXMENUEXT_H_
