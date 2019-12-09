#pragma once
#include <string>
#include <vector>

using namespace std;

class Executor
{
public:
	Executor(const wstring &bgSrvCmd_, int env_, const vector<wstring> &filesArray_);
	~Executor();

	HRESULT executeAction(const wstring &action);
private:
	int env_;
	wstring bgSrvCmd_;
	vector<wstring> filesArray_;
};
