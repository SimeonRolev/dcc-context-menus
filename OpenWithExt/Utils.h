#pragma once
#include <string.h>

#include <vector>
#include <map>

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

using namespace std;


class Utils
{
public:
	Utils();
	~Utils();

	static const map<wstring, wstring> parsePathsFromExecutable(const wstring &BG_SERVICE_PATH);

	static HRESULT readJsonFile(const wstring &path, const char* key, wstring &out);
	static HRESULT Utils::processPathByName(const wstring &appName, wstring &out);

	static int envFromAppName(const wstring &installedApp);

	static bool isVWXType(wstring ext);
	static bool isPhotogramType(wstring ext);
	static bool isFolder(const wstring &path);
	static bool childNodeOf(const wstring &root, const wstring &entry);

	static wstring toLower(wstring &charString);
	static wstring getExtension(wstring charString);
	static wstring placeQuotes(const wstring &input);
	static wstring wrapSpacesForCMD(const wstring &text, wchar_t* sep);
	static wstring getActions(const vector <wstring> &filesArray_);
};
