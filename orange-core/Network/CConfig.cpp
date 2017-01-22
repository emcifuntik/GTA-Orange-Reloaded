#include "stdafx.h"

CConfig *CConfig::singleInstance = nullptr;

CConfig::CConfig()
{
	try {
		doc.LoadFile((CGlobals::Get().orangePath + "/config.xml").c_str());
		if (doc.Error())
		{
			InitConfig();
			return;
		}
		tinyxml2::XMLElement * root = doc.FirstChildElement("config");
		if (!root)
		{
			InitConfig();
			return;
		}
		tinyxml2::XMLElement * serverNode = root->FirstChildElement("server");
		if (!serverNode)
		{
			InitConfig();
			return;
		}
		uiPort = serverNode->IntAttribute("port");
		sIP = std::string(serverNode->GetText());
		sNickName = std::string(root->FirstChildElement("player")->GetText());
		CGlobals::Get().serverPort = uiPort;
		strcpy_s(CGlobals::Get().serverIP, 32, sIP.c_str());
		strcpy_s(CGlobals::Get().nickName, 32, sNickName.c_str());
	}
	catch (...)
	{
		InitConfig();
	}
}

void CConfig::InitConfig()
{
	uiPort = 7788;
	sIP = "127.0.0.1";
	sNickName = std::string("Player");
	CGlobals::Get().serverPort = uiPort;
	strcpy_s(CGlobals::Get().serverIP, 32, sIP.c_str());
	strcpy_s(CGlobals::Get().nickName, 32, sNickName.c_str());
	Save();
}

void CConfig::Save()
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLNode * pRoot = doc.NewElement("config");
	doc.InsertFirstChild(pRoot);

	tinyxml2::XMLElement * pServer = doc.NewElement("server");
	pServer->SetText(sIP.c_str());
	pServer->SetAttribute("port", (int64_t)uiPort);
	pRoot->InsertEndChild(pServer);

	tinyxml2::XMLElement * pPlayer = doc.NewElement("player");
	pPlayer->SetText(sNickName.c_str());
	pRoot->InsertEndChild(pPlayer);

	tinyxml2::XMLError eResult = doc.SaveFile((CGlobals::Get().orangePath + "/config.xml").c_str());
	if (eResult != tinyxml2::XMLError::XML_SUCCESS)
		throw std::exception("TinyXML Error");
}

CConfig::~CConfig()
{
}
