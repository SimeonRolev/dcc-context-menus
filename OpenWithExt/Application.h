#pragma once

#include "Constants.h"

#include <string>
#include <vector>
#include <map>

using namespace std;


class Application
{
public:
	Application();
	~Application();

private:
	wstring env_;

	map<wstring, wstring> paths_;

	// Synced dirs
	wstring syncDir_;
	wstring dropboxDir_;

	wstring selectionType_;
	vector<wstring> filesArray_;
	
	HRESULT _retrieveService();
	HRESULT _getSyncedFolders();

public:
	HRESULT setUp();
	HRESULT parseFile(const wstring &entry);

	// Getters
	wstring env() { return env_; }

	wstring localApp() { return paths_[L"LOCAL_APP"]; }
	wstring localAppProgs() { return paths_[L"LOCAL_APP_PROGS"]; }
	wstring baseDir() { return paths_[L"BASE"]; }
	wstring resourcesDir() { return paths_[L"RESOURCES"]; }
	wstring serverDir() { return paths_[L"SERVER"]; }
	wstring iconsDir() { return paths_[L"ICONS"]; }

	wstring installedApp() { return paths_[L"installedApp_"]; }
	wstring bgSrvCmd() { return paths_[L"bgSrvCmd_"]; }

	wstring syncDir() { return syncDir_; }
	wstring dropboxDir() { return dropboxDir_; }

	wstring selectionType() { return selectionType_; }
	vector<wstring> filesArray() { return filesArray_; }

	map<wstring, wstring> paths() { return paths_; }

	// Setters
	void setEnv(const int &in) { this->env_ = in; };
	void setPaths(const map<wstring, wstring> &in) { this->paths_ = in; };
	void setSyncDir(const wstring &in) { this->syncDir_= in; };
	void setDropboxDir(const wstring &in) { this->dropboxDir_ = in; };
	void setSelectionType();
};
