#include "stdafx.h"
#include "Utils.h"
#include <Lmcons.h>

#include <algorithm>



Utils::Utils()
{
}


Utils::~Utils()
{
}

std::string Utils::getExtension(const char* charString) {
	std::string str(charString);
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

std::string ENV_PROD("prod");
std::string ENV_BETA("beta");
std::string ENV_QA("qa");
std::string ENV_DEVEL("devel");

const std::string ENV_ARRAY[] = { ENV_PROD, ENV_BETA, ENV_QA, ENV_DEVEL };

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

std::string Utils::_getBaseRoot() {
	std::string BASE_ROOT("C:\\Users\\");
	BASE_ROOT.append(getUsername());
	BASE_ROOT.append("\\AppData\\Local\\Programs\\vectorworks-cloud-services-");
	return BASE_ROOT;
}

std::string Utils::getEnv() {
	const std::string BASE_ROOT(Utils::_getBaseRoot());

	DWORD dwAttr;
	std::string appDir;

	for (int i = 0; i < ENV_ARRAY->size(); i++) {
		appDir = BASE_ROOT + ENV_ARRAY[i];
		dwAttr = GetFileAttributes(appDir.c_str());

		if (dwAttr != 0xffffffff && (dwAttr & FILE_ATTRIBUTE_DIRECTORY)) return ENV_ARRAY[i];
	}

	return ENV_PROD;
}


std::string Utils::_getBaseRootCMD() {
	std::string BASE_DIR("C:\\Users\\\"");
	BASE_DIR.append(Utils::getUsername());
	BASE_DIR.append("\"\\AppData\\Local\\Programs\\vectorworks-cloud-services-");
	return BASE_DIR;
}

const std::string ENV(Utils::getEnv());
const std::string BASE_DIR(Utils::_getBaseRootCMD() + ENV);
const std::string CONTEXT_ACTIONS_DIR(BASE_DIR + std::string("\\resources\\context_actions\\"));


int Utils::executeAction(std::string action, std::string args)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	std::string result("");

	result.append("cmd /K ");
	result.append(CONTEXT_ACTIONS_DIR);
	result.append(action);
	result.append(".bat ");
	result.append(args);

	std::wstring wide_string = std::wstring(result.begin(), result.end());
	const wchar_t* _command = wide_string.c_str();
	wchar_t* command = const_cast<wchar_t*>(_command);

	// wchar_t* command = stringToWChar(result);

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcessW(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		printf("CreateProcess failed (%d)\n", GetLastError());
		return FALSE;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 0;
}