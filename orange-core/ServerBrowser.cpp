#include "stdafx.h"

void ServerBrowser()
{
	if (CGlobals::Get().displayServerBrowser)
	{
		ShowCursor(TRUE);
		(*CGlobals::Get().canLangChange) = true;
		CConfig::Get();
		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiSetCond_Always);
		ImGui::SetNextWindowPosCenter(ImGuiSetCond_Always);
		ImGui::PushFont(CGlobals::Get().chatFont);
		ImGui::Begin("Server browser", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		ImGui::Text("Nickname");
		ImGui::InputText("  ", CGlobals::Get().nickName, 32);
		ImGui::Text("Direct connect");
		ImGui::InputText(":", CGlobals::Get().serverIP, 32);
		ImGui::SameLine();
		ImGui::InputInt("Port", &CGlobals::Get().serverPort, 1, 100);
		if (ImGui::Button("Connect"))
		{
			CNetworkPlayer::Clear();
			CNetworkVehicle::Clear();
			CNetworkObject::Clear();
			if (CNetworkConnection::Get()->IsConnected()) CNetworkConnection::Get()->Disconnect();
			std::stringstream ss;
			ss << "Connecting to " << CGlobals::Get().serverIP << ":" << CGlobals::Get().serverPort;
			CChat::Get()->AddChatMessage(ss.str());
			if (!CNetworkConnection::Get()->Connect(CGlobals::Get().serverIP, CGlobals::Get().serverPort))
				CChat::Get()->AddChatMessage("Can't connect to the server", { 255, 0, 0, 255 });
			CGlobals::Get().displayServerBrowser ^= 1;
			CGlobals::Get().showChat = true;
			CConfig::Get()->sNickName = std::string(CGlobals::Get().nickName);
			CConfig::Get()->sIP = std::string(CGlobals::Get().serverIP);
			CConfig::Get()->uiPort = CGlobals::Get().serverPort;
			CConfig::Get()->Save();
			ShowCursor(FALSE);
			(*CGlobals::Get().canLangChange) = false;
		}
		ImGui::End();
		ImGui::PopFont();
	}
}

GUI(ServerBrowser);