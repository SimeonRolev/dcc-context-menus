#include "stdafx.h"
#include "Utils.h"
#include <Lmcons.h>

#include <algorithm>
#include <vector>


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


wchar_t* Utils::stringToWChar(std::wstring s) {
	std::wstring wide_string = std::wstring(s.begin(), s.end());
	const wchar_t* _res = wide_string.c_str();
	return const_cast<wchar_t*>(_res);
}

std::wstring Utils::getDCCRoot() {
	wchar_t* localAppDataPrograms = 0;
	SHGetKnownFolderPath(FOLDERID_UserProgramFiles, 0, NULL, &localAppDataPrograms);
	CoTaskMemFree(static_cast<void*>(localAppDataPrograms));

	return std::wstring(localAppDataPrograms) + L"\\vectorworks-cloud-services-";
}

const std::wstring BASE_ROOT(Utils::getDCCRoot());

int Utils::getEnv() {
	DWORD dwAttr;
	std::wstring appDir;

	for (int i = 0; i < ENV_ARRAY->size(); i++) {
		appDir = BASE_ROOT + ENV_ARRAY[i];
		dwAttr = GetFileAttributesW(appDir.c_str());

		if (dwAttr != 0xffffffff && (dwAttr & FILE_ATTRIBUTE_DIRECTORY)) return i;
	}

	return 3;
}

const int ENV(Utils::getEnv());
const std::wstring BG_SRV(BASE_ROOT + ENV_ARRAY[ENV] + L"\\resources\\server\\");

std::wstring wrapSpaces(const std::wstring &text, wchar_t* sep) {
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

	return res + L"\"Vectorworks Cloud Services Background Service\".exe";
}


int Utils::executeAction(std::wstring action, std::vector<std::wstring> args)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	std::wstring BG_SRV_CMD = wrapSpaces(BG_SRV, L"\\");
	
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
		return FALSE;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 0;
}


std::wstring EXT_VWX(L"vwx");

std::wstring Utils::getActions(std::vector <std::wstring> filesArray){
	if (std::all_of(filesArray.begin(), filesArray.end(), [](std::wstring path) {
		std::wstring ext = Utils::getExtension(path);
		return ext.compare(EXT_VWX) == 0;
	})) {
		return EXT_VWX;
	} else if (std::all_of(filesArray.begin(), filesArray.end(), [](std::wstring path) {
		std::wstring ext = Utils::getExtension(path);
		return Utils::isPhotogramType(ext);
	})) {
		return EXT_JPEG;
	} else return L"NONE";
}
