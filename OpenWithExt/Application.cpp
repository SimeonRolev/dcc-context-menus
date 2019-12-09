#include "stdafx.h"
#include "Application.h"

#include "Utils.h"
#include <map>


Application::Application()
{
}


Application::~Application()
{
}


HRESULT Application::_retrieveService() {
	const wstring exeName = L"Vectorworks Cloud Services Background Service.exe";
	wstring appPath;

	if (SUCCEEDED(Utils::processPathByName(exeName, appPath))) {
		this->setPaths(Utils::parsePathsFromExecutable(appPath));

		env_ = Utils::envFromAppName(installedApp());
		if (env_ < 0) return E_INVALIDARG;

		return S_OK;
	}

	return E_INVALIDARG;
};


// Requires that you have baseDir_ and env_ set
HRESULT Application::_getSyncedFolders() {
	if (localApp().empty()) return E_INVALIDARG;
	if (env() < 0) return E_INVALIDARG;

	wstring appName = L"Vectorworks Cloud Services";
	if (env() > 0) appName.append(L" ").append(label());

	wstring path = localApp() +
		+L"Nemetschek\\" + appName
		+ L"\\Cache\\" + ACTIVE_SESSION_FN;

	if (
		SUCCEEDED(Utils::readJsonFile(path, "rootFolder", syncDir_)) &&
		SUCCEEDED(Utils::readJsonFile(path, "dropboxFolder", dropboxDir_))
		) return S_OK;
	return E_INVALIDARG;
}


HRESULT Application::setUp() {
	if (FAILED(this->_retrieveService()) ||
		FAILED(this->_getSyncedFolders())
		) return E_INVALIDARG;
	return S_OK;
}


HRESULT Application::parseFile(const wstring &entry) {
	if (!Utils::childNodeOf(syncDir(), entry) && !Utils::childNodeOf(dropboxDir(), entry))
		return E_INVALIDARG;

	wstring res(entry);

	if (Utils::isFolder(entry)) res.append(L"\\\\");
	filesArray_.push_back(res);
	return S_OK;
}


void Application::setSelectionType() {
	this->selectionType_ = Utils::getActions(filesArray());
}
