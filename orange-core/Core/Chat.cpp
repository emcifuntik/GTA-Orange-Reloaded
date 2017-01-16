#include "stdafx.h"

//TODO: Экранизация символов { и }

CChat *CChat::singleInstance = nullptr;

void CChat::AddLine(ChatLine line)
{
	access.lock();
	if (vChatLines.size() >= cuChatHistorySize)
		vChatLines.erase(vChatLines.begin());
	vChatLines.push_back(line);
	access.unlock();
	bScrollBottom = true;
}

CChat::CChat() :uiCarretPos(0)
{

}

CChat* CChat::Get()
{
	if (!singleInstance)
		singleInstance = new CChat();
	return singleInstance;
}


void CChat::RegisterCommandProcessor(int(*callback)(std::string))
{
	_commandProcess = callback;
}

CChat::~CChat()
{

}

void CChat::Render()
{
	if (!bEnabled)
		return;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
	ImGui::Begin("Chat", &bEnabled, ImVec2(400, 190), .0f, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

	ImGui::PushFont(CGlobals::Get().chatFont);
	access.lock();
	for each (auto chatLine in vChatLines)
	{
		struct chatFragment {
			color_t color;
			std::string str;
		};
		std::vector<chatFragment> substr;

		chatFragment frag;
		frag.color = chatLine.structColor;
		frag.str = "";

		unsigned currentFrag = 0;
		for (int i = 0; i < chatLine.sLineText.length(); ++i)
		{
			if (chatLine.sLineText[i] == '{' && ((i + 7) <= chatLine.sLineText.length() && chatLine.sLineText[i + 7] == '}'))
			{
				DWORD x = std::stoul(std::string("0x") + chatLine.sLineText.substr(i + 1, 6), nullptr, 16);
				color_t col;
				Utils::HexToRGB(x, col.red, col.green, col.blue);
				frag.color = col;
				frag.color.alpha = chatLine.structColor.alpha;
				frag.str = "";
				currentFrag++;
				chatLine.sLineText = chatLine.sLineText.substr(i + 8);
				i = -1;
				continue;
			}
			if (currentFrag == substr.size())
				substr.push_back(frag);
			substr[currentFrag].str += chatLine.sLineText[i];
		}

		for (unsigned i = 0; i < substr.size(); ++i)
		{
			ImGui::TextColored(ImVec4(float(substr[i].color.red / 255), float(substr[i].color.green / 255), float(substr[i].color.blue / 255), float(substr[i].color.alpha / 255)), (char*)substr[i].str.c_str());
			if(i != (substr.size() - 1))
				ImGui::SameLine(0.f, 0.f);
		}
	}
	access.unlock();


	if (bScrollBottom)
	{
		ImGui::SetScrollPosHere();
		bScrollBottom = false;
	}
	ImGui::PopFont();
	ImGui::End();
	ImGui::PopStyleVar(2);

	if (bOpened)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.f, 2.f));
		if (CChat::Get()->Opened())
			ShowCursor(TRUE);

		ImGui::SetNextWindowPos(ImVec2(0, 190), ImGuiSetCond_Always);
		ImGui::Begin("ChatInput", &bOpened, ImVec2(450, 50), .0f, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		char buffer[256] = "";
		ImGui::PushItemWidth(400);
		ImGui::PushFont(CGlobals::Get().chatFont);
		if (ImGui::InputText("", buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue/* | ImGuiInputTextFlags_CallbackHistory*/))
		{
			if (strlen(buffer))
			{
				if (buffer[0] == '/')
				{
					if (_commandProcess != nullptr)
					{
						if (_commandProcess(buffer) != 1)
						{
							RakNet::BitStream sendmessage;
							RakNet::RakString outStr(buffer);

							sendmessage.Write((MessageID)ID_COMMAND_MESSAGE);
							sendmessage.Write(outStr);
							CNetworkConnection::Get()->client->Send(&sendmessage,HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
						}
					}
				}
				else
				{
					RakNet::BitStream sendmessage;
					RakNet::RakString outStr(buffer);
					sendmessage.Write((MessageID)ID_CHAT_MESSAGE);
					sendmessage.Write(outStr);
					CNetworkConnection::Get()->client->Send(&sendmessage, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
				}
				bScrollBottom = true;
				strcpy_s(buffer, 256, "");
			}
			Close();
		}
		ImGui::PopFont();
		if (bJustOpened)
		{
			bJustOpened = false;
			ImGui::SetKeyboardFocusHere(0);
		}
		ImGui::PopItemWidth();
		ImGui::End();
		ImGui::PopStyleVar(5);
	}
}

void CChat::Input()
{
	if (bOpened)
	{
		UI::SET_PAUSE_MENU_ACTIVE(false);

		if (CONTROLS::_GET_LAST_INPUT_METHOD(2))
		for (int i = 0; i < 338; i++)
		{
			if (i >= 1 && i <= 6)
			{
				continue;
			}

			CONTROLS::DISABLE_CONTROL_ACTION(0, i, false);
		}
		//CONTROLS::DISABLE_ALL_CONTROL_ACTIONS(0);
	}
}

void CChat::AddChatMessage(std::string text, unsigned int rgba)
{
	ChatLine tempLine;
	tempLine.structColor = {
		((rgba >> 24) & 0xFF),
		((rgba >> 16) & 0xFF),
		((rgba >> 8) & 0xFF) ,
		((rgba) & 0xFF)
	};
	tempLine.sLineText = text;
	AddLine(tempLine);
}


void CChat::AddChatMessage(std::string text, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	ChatLine tempLine;
	tempLine.structColor = {
		red, green, blue, alpha
	};
	tempLine.sLineText = text;
	AddLine(tempLine);
}


void CChat::AddChatMessage(std::string text, color_t color)
{
	ChatLine tempLine;
	tempLine.structColor = color;
	tempLine.sLineText = text;
	AddLine(tempLine);
}

void CChat::Clear()
{
	vChatLines.clear();
	iOffset = 0;
}


void CChat::Toggle()
{
	if (bOpened)
		Close();
	else
		Open();
}


void CChat::Open()
{
	//PLAYER::SET_PLAYER_CONTROL(PLAYER::PLAYER_ID(), false, 0);
	/*GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION(scaleform, "SET_FOCUS");
	GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_INT(2);
	GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_INT(2);
	GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_STRING("ALL");
	GRAPHICS::_POP_SCALEFORM_MOVIE_FUNCTION_VOID();

	GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION(scaleform, "SET_FOCUS");
	GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_INT(1);
	GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_INT(2);
	GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_STRING("ALL");
	GRAPHICS::_POP_SCALEFORM_MOVIE_FUNCTION_VOID();*/

	(*CGlobals::Get().canLangChange) = true;
	bOpened = true;
	bJustOpened = true;
	ShowCursor(TRUE);
}


void CChat::Close()
{
	//PLAYER::SET_PLAYER_CONTROL(PLAYER::PLAYER_ID(), true, 0);
	(*CGlobals::Get().canLangChange) = false;
	bOpened = false;
	ShowCursor(FALSE);
}

void CChat::ScriptKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow)
{
	CChat *Chat = CChat::Get();

	
	if (isUpNow || wasDownBefore)
	{
		switch (key)
		{
		case VK_ESCAPE:
			if (Chat->bOpened)
				Chat->Close();
			if (!Chat->bOpened)
			{
				CGlobals::Get().mainmenushown = !CGlobals::Get().mainmenushown;
				if (CGlobals::Get().mainmenushown)
				{
					Chat->Close();
				}
			}
			break;
		case VK_F6:
			Chat->Toggle();
			break;
		case VK_F7:
			Chat->bEnabled = !Chat->bEnabled;
			break;
		case 0x47:
			if (!Chat->bOpened)
				CLocalPlayer::Get()->_togopassenger = true;
			break;
		case VK_F12:
			CGlobals::Get().displayServerBrowser = !CGlobals::Get().displayServerBrowser;
			break;
		case 0x54:
			if (!Chat->bOpened)
				Chat->Open();
			break;
		}
	}
}