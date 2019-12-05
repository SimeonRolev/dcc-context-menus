#pragma once
#include <string>
#include <vector>


class Executor
{
public:
	Executor(const std::wstring &bgSrvCmd_, int env_, const std::vector<std::wstring> &filesArray_);
	~Executor();

	HRESULT executeAction(const std::wstring &action);
private:
	int env_;
	std::wstring bgSrvCmd_;
	std::vector<std::wstring> filesArray_;
};
