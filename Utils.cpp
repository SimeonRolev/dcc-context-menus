#include "stdafx.h"
#include "Utils.h"
#include <Lmcons.h>

#include <algorithm>
#include <vector>

#include <windows.h>
#include <tlhelp32.h>


Utils::Utils()
{
}


Utils::~Utils()
{
}


std::wstring EXT_VWX(L"vwx");

// PHOTOGRAM extensions
std::wstring EXT_TIFF(L"tiff");
std::wstring EXT_TIF(L"tif");
std::wstring EXT_SVG(L"svg");
std::wstring EXT_PNG(L"png");
std::wstring EXT_JPG(L"jpg");
std::wstring EXT_JPEG(L"jpeg");
std::wstring EXT_ICO(L"ico");
std::wstring EXT_GIF(L"gif");
std::wstring EXT_BMP(L"bmp");

const std::wstring ENV_PROD(L"prod");
const std::wstring ENV_BETA(L"beta");
const std::wstring ENV_QA(L"qa");
const std::wstring ENV_DEVEL(L"devel");
const std::wstring ENV_ARRAY[] = { ENV_PROD, ENV_BETA, ENV_QA, ENV_DEVEL };

const std::wstring PROD_CONFIG(L"dcc.main.prod_settings");
const std::wstring BETA_CONFIG(L"dcc.main.beta_settings");
const std::wstring TEST_CONFIG(L"dcc.main.test_settings");
const std::wstring DEV_CONFIG(L"dcc.main.dev_settings");
const std::wstring ENV_CONFIG_ARRAY[] = { PROD_CONFIG, BETA_CONFIG, TEST_CONFIG, DEV_CONFIG };


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

HRESULT Utils::getDCCRoot(std::wstring &out) {
	PWSTR localAppDataPrograms = NULL;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_UserProgramFiles, 0, NULL, &localAppDataPrograms);
	
	if (SUCCEEDED(hr)) {
		std::wstring resdir = std::wstring(localAppDataPrograms) + L"\\vectorworks-cloud-services-";
		out = resdir;
	}
	CoTaskMemFree(localAppDataPrograms);
	return hr;
}

HRESULT Utils::getEnv(std::wstring &BASE_DIR, int &out) {
	DWORD dwAttr;
	std::wstring appDir;

	for (int i = 0; i < ENV_ARRAY->size(); i++) {
		appDir = BASE_DIR + ENV_ARRAY[i];
		dwAttr = GetFileAttributesW(appDir.c_str());

		if (dwAttr != 0xffffffff && (dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
			out = i;
			BASE_DIR = appDir + L"\\";
			return S_OK;
		};
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
	} else if (std::all_of(filesArray.begin(), filesArray.end(), [](std::wstring path) {
		std::wstring ext = Utils::getExtension(path);
		return Utils::isPhotogramType(ext);
	})) {
		SELECTION_TYPE = EXT_JPEG;
		return EXT_JPEG;
	} else return L"NONE";
}


// bool IsProcessRunning() 
bool Utils::serviceIsRunning(std::wstring sAppName) {
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(PROCESSENTRY32W);

	const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	bool a = Process32FirstW(snapshot, &entry);

	do {
		if (!_wcsicmp(entry.szExeFile, sAppName.c_str())) {
			CloseHandle(snapshot);
			return true;
		}
	} while (Process32NextW(snapshot, &entry));

	CloseHandle(snapshot);
	return false;
}
