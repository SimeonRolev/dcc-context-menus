#pragma once
#include <string>
#include <string.h>

#include <vector>


class Utils
{
public:
	Utils();
	~Utils();
	static std::wstring getExtension(std::wstring charString);
	static bool isPhotogramType(std::wstring ext);
	static wchar_t* stringToWChar(std::wstring s);
	static std::wstring getDCCRoot();
	static int getEnv();
	static int executeAction(std::wstring action, std::vector<std::wstring> args);
	static std::wstring Utils::getActions(std::vector <std::wstring> filesArray);
};
