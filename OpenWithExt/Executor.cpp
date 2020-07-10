#include "stdafx.h"

#include "Application.h"
#include "Constants.h"
#include "Executor.h"
#include "Utils.h"

#include <string>
#include <vector>

using namespace std;

Executor::Executor(Application app)
{
	wstring a = app.bgSrvCmd();
	this->command = L"cmd /C cd /d " +
		Utils::wrapSpacesForCMD(app.serverDir(), L"\\") +
		L" && set ENV_FOR_DYNACONF=" + ENV_ARRAY[app.env()] + L" && " +
		L"\"Vectorworks Cloud Services Background Service\".exe" +
		L" --config=" + ENV_CONFIG_ARRAY[app.env()] + L" ";
	for (int i = 0; i < app.filesArray().size(); i++) {
		this->command.append(Utils::placeQuotes(app.filesArray()[i]));
		this->command.append(L" ");
	}
}


Executor::~Executor()
{
}


HRESULT Executor::executeAction(const wstring &action) {
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	wstring result = wstring(this->command);
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
