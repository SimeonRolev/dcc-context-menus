// OpenWithCtxMenuExt.h : Declaration of the COpenWithCtxMenuExt

#ifndef __OPENWITHCTXMENUEXT_H_
#define __OPENWITHCTXMENUEXT_H_

#include "resource.h"       // main symbols
#include <string>
#include <vector>

#include "Application.h"
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

	Application app_;
public:
	void clear();
	HRESULT failAndClear();
	HICON LoadIcon(const wstring &name);

	Application app () { return app_; };
};

#endif //__OPENWITHCTXMENUEXT_H_
