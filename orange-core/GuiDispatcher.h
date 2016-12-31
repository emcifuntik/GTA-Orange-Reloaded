#pragma once
class CGuiDispatcher
{
	static CGuiDispatcher * instance;
	std::vector<std::function<void()>> _functions;
	CGuiDispatcher();
	~CGuiDispatcher();
public:
	static CGuiDispatcher& Get();
	void Run();
	void operator()();
	CGuiDispatcher& operator+=(std::function<void()> func);
};
