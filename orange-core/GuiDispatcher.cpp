#include "stdafx.h"

CGuiDispatcher * CGuiDispatcher::instance = nullptr;


CGuiDispatcher::CGuiDispatcher()
{
}


CGuiDispatcher::~CGuiDispatcher()
{
}

CGuiDispatcher & CGuiDispatcher::Get()
{
	if (!instance)
		instance = new CGuiDispatcher();
	return *instance;
}

void CGuiDispatcher::Run()
{
	for each (std::function<void()> func in _functions)
		func();
}

void CGuiDispatcher::operator()()
{
	this->Run();
}

CGuiDispatcher & CGuiDispatcher::operator+=(std::function<void()> func)
{
	_functions.push_back(func);
	return *this;
}
