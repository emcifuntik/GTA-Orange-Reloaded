#include "stdafx.h"

void ServerBrowser()
{
	//ShowCursor(TRUE);
	if (CGlobals::Get().displayServerBrowser)
	{
		//(*CGlobals::Get().canLangChange) = true;
		CConfig::Get();
		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiSetCond_Always);
		ImGui::SetNextWindowPosCenter(ImGuiSetCond_Always);
		ImGui::PushFont(CGlobals::Get().chatFont);
		ImGui::Begin("Server browser", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
		ImGui::Text("Nickname");
		ImGui::InputText("  ", CGlobals::Get().nickName, 32);
		if (CGlobals::Get().isDeveloper)
		{
			ImGui::Text("Direct connect");
			ImGui::InputText(":", CGlobals::Get().serverIP, 32);
			ImGui::SameLine();
			ImGui::InputInt("Port", &CGlobals::Get().serverPort, 1, 100);
		}
		if (ImGui::Button("Connect"))
		{
			int size = strlen(CGlobals::Get().nickName);
			for (int i = 0; i < size; ++i)
			{
				if (CGlobals::Get().nickName[i] == '{' || CGlobals::Get().nickName[i] == '}') {
					//log << "found: " << i << std::endl;
					CGlobals::Get().nickName[i] == '\u0020';
				}
			}
			CNetworkUI::Clear();

			if (CNetworkConnection::Get()->IsConnected())
			{
				CNetworkConnection::Get()->bConnected = false;
				CNetworkConnection::Get()->bEstablished = false;
				CNetworkConnection::Get()->cEstablished = 0;
				CNetworkConnection::Get()->bClear = true;
				CNetworkConnection::Get()->Disconnect();
			}

			std::stringstream ss;
			ss << "Connecting to beta-test server"; //<< CGlobals::Get().serverIP << ":" << CGlobals::Get().serverPort;
			CChat::Get()->AddChatMessage(ss.str());

			if (!CNetworkConnection::Get()->Connect(CGlobals::Get().serverIP, CGlobals::Get().serverPort))
				CChat::Get()->AddChatMessage("Can't connect to the server", { 
					255, 0, 0, 255 });

			CGlobals::Get().displayServerBrowser ^= 1;
			CGlobals::Get().showChat = true;
			CGlobals::Get().name = CGlobals::Get().nickName;
			CConfig::Get()->sNickName = std::string(CGlobals::Get().nickName);
			CConfig::Get()->sIP = std::string(CGlobals::Get().serverIP);
			CConfig::Get()->uiPort = CGlobals::Get().serverPort;
			CConfig::Get()->Save();
			//ShowCursor(FALSE);
			//(*CGlobals::Get().canLangChange) = false;
		}
		ImGui::End();
		ImGui::PopFont();
	}
}

GUI(ServerBrowser);