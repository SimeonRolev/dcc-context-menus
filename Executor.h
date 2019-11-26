#pragma once
#include <string>
#include <vector>


class Executor
{
public:
	Executor(std::wstring BG_SRV_CMD, int ENV, const std::vector<std::wstring> &filesArray);
	~Executor();

	HRESULT executeAction(const std::wstring &action);
private:
	int ENV;
	std::wstring BG_SRV_CMD;
	std::vector<std::wstring> filesArray;
};
