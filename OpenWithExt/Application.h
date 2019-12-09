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
	int env_ = -1;

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
	int env() { return env_; }

	const wstring localApp() { return paths_[L"LOCAL_APP"]; }
	const wstring localAppProgs() { return paths_[L"LOCAL_APP_PROGS"]; }
	const wstring baseDir() { return paths_[L"BASE"]; }
	const wstring resourcesDir() { return paths_[L"RESOURCES"]; }
	const wstring serverDir() { return paths_[L"SERVER"]; }
	const wstring iconsDir() { return paths_[L"ICONS"]; }

	const wstring installedApp() { return paths_[L"installedApp_"]; }
	const wstring bgSrvCmd() { return paths_[L"bgSrvCmd_"]; }

	const wstring label() { return ENV_ARRAY[env()]; }

	const wstring syncDir() { return syncDir_; }
	const wstring dropboxDir() { return dropboxDir_; }

	const wstring selectionType() { return selectionType_; }
	const vector<wstring> filesArray() { return filesArray_; }

	const map<wstring, wstring> paths() { return paths_; }

	// Setters
	void setEnv(const int &in) { this->env_ = in; };
	void setPaths(const map<wstring, wstring> &in) { this->paths_ = in; };
	void setSyncDir(const wstring &in) { this->syncDir_= in; };
	void setDropboxDir(const wstring &in) { this->dropboxDir_ = in; };
	void setSelectionType();
};
