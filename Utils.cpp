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

std::string Utils::getExtension(std::string str) {
	std::size_t startIndex = str.find_last_of(".");
	std::string sl = str.substr(startIndex + 1, str.size() - startIndex - 2);
	std::transform(sl.begin(), sl.end(), sl.begin(), ::tolower);
	return sl;
}


// PHOTOGRAM extensions
std::string EXT_TIFF("tiff");
std::string EXT_TIF("tif");
std::string EXT_SVG("svg");
std::string EXT_PNG("png");
std::string EXT_JPG("jpg");
std::string EXT_JPEG("jpeg");
std::string EXT_ICO("ico");
std::string EXT_GIF("gif");
std::string EXT_BMP("bmp");


bool Utils::isPhotogramType(std::string ext) {
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

const std::string ENV_PROD("prod");
const std::string ENV_BETA("beta");
const std::string ENV_QA("qa");
const std::string ENV_DEVEL("devel");
const std::string ENV_ARRAY[] = { ENV_PROD, ENV_BETA, ENV_QA, ENV_DEVEL };

const std::string PROD_CONFIG("dcc.main.prod_settings");
const std::string BETA_CONFIG("dcc.main.beta_settings");
const std::string TEST_CONFIG("dcc.main.test_settings");
const std::string DEV_CONFIG("dcc.main.dev_settings");
const std::string ENV_CONFIG_ARRAY[] = { PROD_CONFIG, BETA_CONFIG, TEST_CONFIG, DEV_CONFIG };


wchar_t* Utils::stringToWChar(std::string s) {
	std::wstring wide_string = std::wstring(s.begin(), s.end());
	const wchar_t* _res = wide_string.c_str();
	return const_cast<wchar_t*>(_res);
}

std::string Utils::getUsername() {
	char _username[UNLEN + 1];
	DWORD size = UNLEN + 1;
	bool u = GetUserName((TCHAR*)_username, &size);

	return std::string(_username);
}

// In order for this to work there must not be " when detecting the folder
std::string Utils::_getBaseRoot() {
	std::string BASE_ROOT("C:\\Users\\");
	BASE_ROOT.append(getUsername());
	BASE_ROOT.append("\\AppData\\Local\\Programs\\vectorworks-cloud-services-");
	return BASE_ROOT;
}

// Adds " to the username so that CMD can execute the command
std::string Utils::_getBaseRootCMD() {
	std::string BASE_DIR("C:\\Users\\\"");
	BASE_DIR.append(Utils::getUsername());
	BASE_DIR.append("\"\\AppData\\Local\\Programs\\vectorworks-cloud-services-");
	return BASE_DIR;
}

int Utils::getEnv() {
	const std::string BASE_ROOT(Utils::_getBaseRoot());

	DWORD dwAttr;
	std::string appDir;

	for (int i = 0; i < ENV_ARRAY->size(); i++) {
		appDir = BASE_ROOT + ENV_ARRAY[i];
		dwAttr = GetFileAttributes(appDir.c_str());

		if (dwAttr != 0xffffffff && (dwAttr & FILE_ATTRIBUTE_DIRECTORY)) return i;
	}

	return 3;
}

const int ENV(Utils::getEnv());
const std::string BACKGROUND_SERVICE(Utils::_getBaseRootCMD() + ENV_ARRAY[ENV] + std::string("\\resources\\server\\\"Vectorworks Cloud Services Background Service\".exe"));

int Utils::executeAction(std::string action, std::vector<std::string> args)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	std::string result("cmd /c " + BACKGROUND_SERVICE + " --config=" + ENV_CONFIG_ARRAY[ENV] + " ");
	for (int i = 0; i < args.size(); i++) {
		result.append(args[i]);
		result.append(" ");
	}
	result.append(action);

	std::wstring wide_string = std::wstring(result.begin(), result.end());
	const wchar_t* _command = wide_string.c_str();
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


std::string EXT_VWX("vwx");

std::string Utils::getActions(std::vector <std::string> filesArray){
	if (std::all_of(filesArray.begin(), filesArray.end(), [](std::string path) {
		std::string ext = Utils::getExtension(path);
		return ext.compare(EXT_VWX) == 0;
	})) {
		return EXT_VWX;
	} else if (std::all_of(filesArray.begin(), filesArray.end(), [](std::string path) {
		std::string ext = Utils::getExtension(path);
		return Utils::isPhotogramType(ext);
	})) {
		return EXT_JPEG;
	} else return "NONE";
}
