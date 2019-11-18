#pragma once
#include <string>
#include <string.h>


class Utils
{
public:
	Utils();
	~Utils();
	static std::string getExtension(const char* charString);
	static bool isPhotogramType(std::string ext);
	static wchar_t* stringToWChar(std::string s);
	static std::string getUsername();
	static std::string _getBaseRoot();
	static std::string getEnv();
	static std::string _getBaseRootCMD();
	static int executeAction(std::string action, std::string args);
};
