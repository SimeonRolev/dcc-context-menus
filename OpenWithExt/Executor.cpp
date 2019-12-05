#include "stdafx.h"
#include "Constants.h"
#include "Executor.h"
#include "Utils.h"
#include <string>
#include <vector>



Executor::Executor(const std::wstring &bgSrvCmd_, int env_, const std::vector<std::wstring> &filesArray_)
{
	this->bgSrvCmd_ = bgSrvCmd_;
	this->env_ = env_;
	this->filesArray_ = filesArray_;
}


Executor::~Executor()
{
}


HRESULT Executor::executeAction(const std::wstring &action) {
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	std::wstring result(L"cmd /C " + bgSrvCmd_ + L" --config=" + ENV_CONFIG_ARRAY[env_] + L" ");
	for (int i = 0; i < filesArray_.size(); i++) {
		result.append(Utils::placeQuotes(filesArray_[i]));
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
