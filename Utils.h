#pragma once
#include <string>
#include <string.h>

#include <vector>


class Utils
{
public:
	Utils();
	~Utils();
	static std::string getExtension(std::string charString);
	static bool isPhotogramType(std::string ext);
	static wchar_t* stringToWChar(std::string s);
	static std::string getUsername();
	static std::string _getBaseRoot();
	static int getEnv();
	static std::string _getBaseRootCMD();
	static int executeAction(std::string action, std::vector<std::string> args);
	static std::string Utils::getActions(std::vector <std::string> filesArray);
};
