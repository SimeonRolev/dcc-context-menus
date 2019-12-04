#pragma once
#include <string.h>

#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"


class Utils
{
public:
	Utils();
	~Utils();
	
	static HRESULT readJsonFile(const std::wstring &path, const char* key, std::wstring &out);
	static std::wstring getExtension(std::wstring charString);
	
	static bool isVWXType(std::wstring ext);
	static bool isPhotogramType(std::wstring ext);
	static bool isFolder(const std::wstring &path);

	// CMD command requires proper quotes wrapping
	static std::wstring wrapSpacesForCMD(const std::wstring &text, wchar_t* sep);
	
	// getActions is used twice, so we allow it to modify the SELECTION_TYPE and reuse that
	static std::wstring getActions(std::wstring &SELECTION_TYPE, const std::vector <std::wstring> &filesArray);
	
	// exectuteAction should not modify anything, so all params are passed by value
	static HRESULT executeAction(std::wstring BG_SRV_CMD, int ENV, std::wstring action, const std::vector<std::wstring> &args);
};
