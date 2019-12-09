#pragma once
#include <string>
#include <vector>

#include "Application.h"

using namespace std;

class Executor
{
public:
	Executor(Application app);
	~Executor();

	HRESULT executeAction(const wstring &action);
private:
	wstring command;
};
