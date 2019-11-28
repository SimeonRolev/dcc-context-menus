#include <Lmcons.h>

#include "stdafx.h"

#include <algorithm>
#include <fstream>
#include <vector>

#include "Utils.h"
#include "Constants.h"

#include <windows.h>
#include <tlhelp32.h>

#include <assert.h> 

#include <locale>
#include <codecvt>


Utils::Utils()
{
}


Utils::~Utils()
{
}


std::wstring Utils::getExtension(std::wstring str) {
	std::size_t startIndex = str.find_last_of(L".");
	std::wstring sl = str.substr(startIndex + 1, str.size() - startIndex - 2);
	std::transform(sl.begin(), sl.end(), sl.begin(), ::tolower);
	return sl;
}


bool Utils::isVWXType(std::wstring ext) {
	if (ext.compare(EXT_VWX) == 0) return true;
	return false;
}


bool Utils::isPhotogramType(std::wstring ext) {
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


bool Utils::isFolder(std::wstring path) {
	DWORD dwAttr = GetFileAttributesW(path.c_str());
	if (dwAttr != 0xffffffff && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
		return true;
	return false;
}


HRESULT Utils::readJsonFile(const std::wstring &path, std::wstring &out) {
	using namespace rapidjson;

	std::string p(path.begin(), path.end());
	FILE* fp;
	if (fp = fopen(p.c_str(), "rb")) {
		char readBuffer[8192];
		FileReadStream is(fp, readBuffer, sizeof(readBuffer));

		Document d;
		d.ParseStream(is);
		fclose(fp);

		if (d.HasMember("rootFolder") && d["rootFolder"].IsString()) {
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			std::wstring wide = converter.from_bytes(d["rootFolder"].GetString());
			out = wide;
			return S_OK;
		}
	}

	return E_INVALIDARG;
}


std::wstring Utils::wrapSpacesForCMD(const std::wstring &text, wchar_t* sep) {
	std::vector<std::wstring> tokens;
	std::size_t start = 0, end = 0;
	while ((end = text.find(sep, start)) != std::wstring::npos) {
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));

	std::wstring res(L"");
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i].find(L" ") != std::wstring::npos) {
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


HRESULT Utils::executeAction(std::wstring BG_SRV_CMD, int ENV, std::wstring action, const std::vector<std::wstring> &args) {
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	
	std::wstring result(L"cmd /C " + BG_SRV_CMD + L" --config=" + ENV_CONFIG_ARRAY[ENV] + L" ");
	for (int i = 0; i < args.size(); i++) {
		result.append(args[i]);
		result.append(L" ");
	}
	result.append(action);

	const wchar_t* _command = result.c_str();
	wchar_t* command = const_cast<wchar_t*>(_command);

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcessW(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		printf("CreateProcess failed (%d)\n", GetLastError());
		return E_INVALIDARG;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return S_OK;
}


std::wstring Utils::getActions(std::wstring &SELECTION_TYPE, const std::vector <std::wstring> &filesArray) {
	if (std::all_of(filesArray.begin(), filesArray.end(), [](std::wstring path) {
		std::wstring ext = Utils::getExtension(path);
		return ext.compare(EXT_VWX) == 0;
	})) {
		SELECTION_TYPE = EXT_VWX;
		return EXT_VWX;
	} else if (
		(filesArray.size() == 1 && filesArray[0].compare(filesArray[0].length() - 2, 1, L"\\") == 0) || // Single folder
		std::all_of(filesArray.begin(), filesArray.end(), [](std::wstring path) { // All files are images
			std::wstring ext = Utils::getExtension(path);
			return Utils::isPhotogramType(ext);
		}
	)) {
		SELECTION_TYPE = EXT_JPEG;
		return EXT_JPEG;
	} else return L"NONE";
}
