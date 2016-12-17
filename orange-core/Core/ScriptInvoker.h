#pragma once
#include <functional>
#include <queue>

typedef std::function<void()> InvokeFunction_;

class CScriptInvoker
{
	static CScriptInvoker * singleInstance;
	CScriptInvoker();

	std::queue<InvokeFunction_> InvokeFunctions;
public:
	static CScriptInvoker& Get()
	{
		if (!singleInstance)
			singleInstance = new CScriptInvoker();
		return *singleInstance;
	}

	bool Empty() {
		return InvokeFunctions.empty();
	}

	InvokeFunction_ Pop()
	{
		InvokeFunction_ tmp = InvokeFunctions.front();
		InvokeFunctions.pop();
		return tmp;
	}

	void Push(InvokeFunction_ func)
	{
		InvokeFunctions.push(func);
	}

	~CScriptInvoker();
};

