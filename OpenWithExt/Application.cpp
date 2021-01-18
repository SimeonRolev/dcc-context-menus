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
		Utils::log(L"PROCESS :: Identified process by name: " + exeName);
		Utils::log(L"PROCESS :: Process running from: " + appPath);

		map<wstring, wstring> parsedPaths = Utils::parsePathsFromExecutable(appPath);
		this->setPaths(parsedPaths);

		map<wstring, wstring>::iterator it;
		for (it = parsedPaths.begin(); it != parsedPaths.end(); it++)
		{
			Utils::log(it->first + L": " + it->second);
		}

		env_ = Utils::envFromAppName(installedApp());
		Utils::log(L"Environment: " + env_);
		return S_OK;
	}
	else {
		Utils::log(L"ERROR :: Failed to find process by name: " + exeName);
	}

	return E_INVALIDARG;
};


// Requires that you have baseDir_ and env_ set
HRESULT Application::_getSyncedFolders() {
	if (localApp().empty()) return E_INVALIDARG;

	wstring appName = L"Vectorworks Cloud Services";
	if (!env().empty()) appName.append(L" ").append(env());

	wstring path = localApp() +
		+L"Nemetschek\\" + appName
		+ L"\\Cache\\" + ACTIVE_SESSION_FN;


	if (
		SUCCEEDED(Utils::readJsonFile(path, "rootFolder", syncDir_)) &&
		SUCCEEDED(Utils::readJsonFile(path, "dropboxFolder", dropboxDir_))
		) {
		Utils::log(L"ROOT FOLDER :: Home detected at: " + syncDir_);
		Utils::log(L"ROOT FOLDER :: Dropbox detected at: " + dropboxDir_);
		return S_OK;
	}
	
	Utils::log(L"ERROR :: ROOT FOLDER :: Failed to read active_json at: " + path);

	return E_INVALIDARG;
}


HRESULT Application::setUp() {
	Utils::deleteLogFile();

	if (FAILED(this->_retrieveService()) ||
		FAILED(this->_getSyncedFolders())
		)
	{
		Utils::log(L"ERROR :: Setup failed");
		return E_INVALIDARG;
	}
	return S_OK;
}


HRESULT Application::parseFile(const wstring &entry) {
	if (!Utils::childNodeOf(syncDir(), entry) && !Utils::childNodeOf(dropboxDir(), entry)) {
		Utils::log(L"ERROR :: FILE :: Entry was not recognized as child of root folder: " + entry);
		return E_INVALIDARG;
	}

	wstring res(entry);

	if (Utils::isFolder(entry)) res.append(L"\\\\");
	filesArray_.push_back(res);
	return S_OK;
}


void Application::setSelectionType() {
	this->selectionType_ = Utils::getActions(filesArray());
}
