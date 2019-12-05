#include <Lmcons.h>

#include "stdafx.h"

#include <algorithm>
#include <fstream>
#include <vector>
#include <string>

#include "Utils.h"
#include "Constants.h"

#include <windows.h>
#include <tlhelp32.h>

#include <assert.h> 

#include <locale>
#include <codecvt>


using namespace std;


Utils::Utils()
{
}


Utils::~Utils()
{
}


map<wstring, wstring> Utils::parsePathsFromExecutable(const wstring &exePath) {
	wstring serverPath = exePath.substr(0, exePath.rfind(L"\\"));
	wstring resourcesPath = serverPath.substr(0, serverPath.rfind(L"\\"));
	wstring basePath = resourcesPath.substr(0, resourcesPath.rfind(L"\\"));
	wstring programsPath = basePath.substr(0, basePath.rfind(L"\\"));
	wstring localPath = programsPath.substr(0, programsPath.rfind(L"\\"));
	wstring installedApp = basePath.substr(programsPath.length() + 1, basePath.length());

	map<wstring, wstring> res;
	res.insert(pair<wstring, wstring>(L"locApp_", localPath + L"\\"));
	res.insert(pair<wstring, wstring>(L"locAppProgs_", programsPath + L"\\"));
	res.insert(pair<wstring, wstring>(L"installedApp_", installedApp));
	res.insert(pair<wstring, wstring>(L"baseDir_", basePath + L"\\"));
	res.insert(pair<wstring, wstring>(L"resourcesDir_", resourcesPath + L"\\"));
	res.insert(pair<wstring, wstring>(L"serverDir_", serverPath + L"\\"));
	res.insert(pair<wstring, wstring>(L"iconsDir_", resourcesPath + L"\\context_actions\\icons\\"));
	res.insert(pair<wstring, wstring>(L"bgSrvCmd_", Utils::wrapSpacesForCMD(serverPath + L"\\", L"\\") + L"\"Vectorworks Cloud Services Background Service\".exe"));

	return res;
}


HRESULT Utils::readJsonFile(const wstring &path, const char* key, wstring &out) {
	using namespace rapidjson;

	string p(path.begin(), path.end());
	FILE* fp;
	if (fp = fopen(p.c_str(), "rb")) {
		char readBuffer[8192];
		FileReadStream is(fp, readBuffer, sizeof(readBuffer));

		Document d;
		d.ParseStream(is);
		fclose(fp);

		if (d.HasMember(key) && d[key].IsString()) {
			wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
			wstring wide = converter.from_bytes(d[key].GetString());
			out = wide;
		}
		else out = L"";
		return S_OK;
	}

	out = L"";
	return E_INVALIDARG;
}


HRESULT Utils::processPathByName(const wstring &appName, wstring &out) {
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(PROCESSENTRY32W);

	const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	bool a = Process32FirstW(snapshot, &entry);

	do {
		if (!_wcsicmp(entry.szExeFile, appName.c_str())) {
			const auto snapmodule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, entry.th32ProcessID);

			MODULEENTRY32W mod;
			mod.dwSize = sizeof(MODULEENTRY32W);
			bool m = Module32FirstW(snapmodule, &mod);
			out = mod.szExePath;

			CloseHandle(snapshot);
			CloseHandle(snapmodule);
			return S_OK;
		}
	} while (Process32NextW(snapshot, &entry));

	CloseHandle(snapshot);
	return E_INVALIDARG;
}


int Utils::envFromAppName(const wstring &installedApp) {
	int env_ = -1;
	if (installedApp.compare(L"vectorworks-cloud-services") == 0) env_ = 0;
	else {
		for (int i = 1; i < ENV_ARRAY->size(); i++) {
			if (installedApp.rfind(ENV_ARRAY[i]) != wstring::npos) env_ = i;
		}
	}
	return env_;
}


bool Utils::isVWXType(wstring ext) {
	if (ext.compare(EXT_VWX) == 0) return true;
	return false;
}


bool Utils::isPhotogramType(wstring ext) {
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


bool Utils::isFolder(const wstring &path) {
	DWORD dwAttr = GetFileAttributesW(path.c_str());
	if (dwAttr != 0xffffffff && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
		return true;
	return false;
}


bool Utils::childNodeOf(const wstring &root, const wstring &entry) {
	if (
		root.empty() ||
		entry.compare(root) == 0 ||  // Should not be the root itself
		entry.size() < root.size() ||  // entry length should be larger than the root length
		entry.substr(0, root.size()).compare(root) != 0  // child check
		) return false;
	return true;
}


wstring Utils::toLower(wstring &str) {
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}


wstring Utils::getExtension(wstring str) {
	size_t startIndex = str.find_last_of(L".");
	if (startIndex == wstring::npos) return L"";

	wstring sl = str.substr(startIndex + 1, str.size() - startIndex - 1);
	Utils::toLower(sl);
	return sl;
}


wstring Utils::placeQuotes(const wstring &input) {
	return L"\"" + input + L"\"";
}


wstring Utils::wrapSpacesForCMD(const wstring &text, wchar_t* sep) {
	vector<wstring> tokens;
	size_t start = 0, end = 0;
	while ((end = text.find(sep, start)) != wstring::npos) {
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));

	wstring res(L"");
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i].find(L" ") != wstring::npos) {
			res.append(L"\"" + tokens[i] + L"\"");
		} else {
			res.append(tokens[i]);
		}
		if (i != tokens.size() - 1) {
			res.append(L"\\");
		}
	}

	return res;
}


wstring Utils::getActions(const vector <wstring> &filesArray_) {
	if (all_of(filesArray_.begin(), filesArray_.end(), [](wstring path) {
		wstring ext = Utils::getExtension(path);
		return ext.compare(EXT_VWX) == 0;
	})) return EXT_VWX;
	else if (
		(filesArray_.size() == 1 && filesArray_[0].compare(filesArray_[0].length() - 2, 1, L"\\") == 0) || // Single folder
		all_of(filesArray_.begin(), filesArray_.end(), [](wstring path) { // All files are images
			wstring ext = Utils::getExtension(path);
			return Utils::isPhotogramType(ext);
		}
	)) return EXT_JPEG;
	return L"NONE";
}
