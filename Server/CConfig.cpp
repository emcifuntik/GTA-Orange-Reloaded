#include "stdafx.h"

CConfig* CConfig::singleInstance = nullptr;


CConfig::CConfig()
{
	std::ifstream fin("config.yml");
	YAML::Parser parser(fin);

	YAML::Node doc;
	if (!parser.GetNextDocument(doc)) {
		log << "Can`t load config.yml" << std::endl;
		Hostname.append("Unnamed server");
		Port = 7788;
		MaxPlayers = 128;
	}
	else {
		if(!doc.FindValue("name")) Hostname.append("Unnamed server");
		else doc["name"] >> Hostname;
		if (!doc.FindValue("port")) Port = 7788;
		else doc["port"] >> Port;
		if (!doc.FindValue("players")) MaxPlayers = 128;
		else doc["players"] >> MaxPlayers;

		const YAML::Node& resources = doc["resources"];

		for (unsigned i = 0; i < resources.size(); i++) {
			std::string res;;
			resources[i] >> res;
			Resources.push_back(res);
		}
	}
	if (MaxPlayers > 256) {
		log << "Only 256 players supported, set players to 256" << std::endl;
		MaxPlayers = 256;
	}
}

CConfig* CConfig::Get()
{
	if (!singleInstance)
		singleInstance = new CConfig();
	return singleInstance;
}


CConfig::~CConfig()
{
}
