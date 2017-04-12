#pragma once

typedef void(*function_)();

class CScript {
	static std::queue<std::pair<std::string, CScript*>> * scriptQueue;
protected:
	virtual void Run() = 0;
	CScript(std::string _name)
	{
		if (!scriptQueue)
			scriptQueue = new std::queue<std::pair<std::string, CScript*>>();
		scriptQueue->push(std::pair<std::string, CScript*>(_name, this));
	}
public:
	static void RunAll()
	{
		//log_debug << "Scripts in queue: " << scriptQueue->size() << std::endl;
		while (!scriptQueue->empty())
		{
			//log_debug << "Running new script: " << scriptQueue->front().first << std::endl;
			scriptQueue->front().second->Run();
			scriptQueue->pop();
		}
	}
};