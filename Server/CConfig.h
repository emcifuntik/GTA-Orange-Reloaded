#pragma once
class CConfig
{
	CConfig();

	static CConfig* singleInstance;
public:
	static CConfig* Get();
	std::string Path;
	std::string Hostname;
	std::vector<std::string> Resources;
	unsigned short Port;
	unsigned short HTTPPort;
	unsigned short MaxPlayers;
	~CConfig();
};

