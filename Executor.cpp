#include "stdafx.h"
#include "Executor.h"
#include "Utils.h"
#include <string>
#include <vector>

Executor::Executor(std::wstring BG_SRV_CMD, int ENV, const std::vector<std::wstring> &filesArray)
{
	this->BG_SRV_CMD = BG_SRV_CMD;
	this->ENV = ENV;
	this->filesArray = filesArray;
}


Executor::~Executor()
{
}


HRESULT Executor::executeAction(const std::wstring &action) {
	return Utils::executeAction(BG_SRV_CMD, ENV, action, filesArray);
}
