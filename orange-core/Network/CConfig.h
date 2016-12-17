#pragma once
class CConfig
{
	static CConfig *singleInstance;
	CConfig();
	void InitConfig();
public:
	static CConfig *Get()
	{
		if (!singleInstance)
			singleInstance = new CConfig();
		return singleInstance;
	}

	tinyxml2::XMLDocument doc;
	std::string sNickName = "Player";
	std::string sIP = "";
	unsigned int uiPort;

	void Save();
	~CConfig();
};

