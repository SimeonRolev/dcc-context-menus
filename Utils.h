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
	static bool isVWXType(std::wstring ext);
	static bool isPhotogramType(std::wstring ext);

	// These will be used for directly setting dirs and envs, so references are passed
	// They return HRESULT to check for possible errors
	static HRESULT getDCCRoot(std::wstring &out);
	static HRESULT getEnv(std::wstring &BASE_DIR, int &out);

	// CMD command requires proper quotes wrapping
	static std::wstring wrapSpacesForCMD(const std::wstring &text, wchar_t* sep);
	
	// getActions is used twice, so we allow it to modify the SELECTION_TYPE and reuse that
	static std::wstring getActions(std::wstring &SELECTION_TYPE, const std::vector <std::wstring> &filesArray);
	
	// exectuteAction should not modify anything, so all params are passed by value
	static HRESULT executeAction(std::wstring BG_SRV_CMD, int ENV, std::wstring action, const std::vector<std::wstring> &args);
};
