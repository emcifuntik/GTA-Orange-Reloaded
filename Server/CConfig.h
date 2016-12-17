#pragma once
class CConfig
{
	CConfig();

	static CConfig* singleInstance;
public:
	static CConfig* Get();
	std::string Hostname;
	std::vector<std::string> Resources;
	unsigned short Port;
	unsigned short MaxPlayers;
	~CConfig();
};

