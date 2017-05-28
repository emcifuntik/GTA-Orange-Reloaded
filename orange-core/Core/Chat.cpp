#include "stdafx.h"

//TODO: Экранизация символов { и }

CChat *CChat::singleInstance = nullptr;
ImVector<char*> CChat::History;
int CChat::HistoryPos;

void CChat::AddLine(ChatLine line)
{
	access.lock();
	if (vChatLines.size() >= cuChatHistorySize)
		vChatLines.erase(vChatLines.begin());
	vChatLines.push_back(line);
	ulLastUpdate = timeGetTime();
	access.unlock();
	bScrollBottom = true;
}

CChat::CChat() :uiCarretPos(0)
{
	HistoryPos = -1;
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
	for (int i = 0; i < History.Size; i++)
		free(History[i]);
}

void CChat::Render()
{
	if (!bEnabled)
		return;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(0.f, 0.f, 0.f, 0.5f));

	float fOpacity = 0.f;
	if (!bOpened)
	{
		unsigned long ulCurTime = timeGetTime();
		unsigned long timeSinceUpdate = ulCurTime - ulLastUpdate;
		if (timeSinceUpdate < 4500)
			fOpacity = 0.5f;
		else if (timeSinceUpdate >= 4500 && timeSinceUpdate <= 5000)
			fOpacity = 0.5f - ((float)(timeSinceUpdate - 4500) / 1000);
		else
			fOpacity = 0.f;
	}
	else
		fOpacity = 0.5f;

	ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImColor(0.f, 0.f, 0.f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImColor(0.7f, 0.7f, 0.7f, fOpacity));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImColor(0.8f, 0.8f, 0.8f, fOpacity));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImColor(0.75f, 0.75f, 0.75f, fOpacity));

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
	ImGui::Begin("Chat", &bEnabled, ImVec2(400, 190), fOpacity, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

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
				DWORD x;

				try {
					x = std::stoul(std::string("0x") + chatLine.sLineText.substr(i + 1, 6), nullptr, 16);
				}
				catch (std::invalid_argument)
				{
					x = 0xFFFFFF;
				}

				color_t col;
				Utils::HexToRGB(x, col.red, col.green, col.blue);
				frag.color = col;
				frag.color.alpha = chatLine.structColor.alpha;
				frag.str = "";
				if (i != 0)
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
			ImGui::TextColored(ImColor(substr[i].color.red, substr[i].color.green, substr[i].color.blue, BYTE(std::roundf(substr[i].color.alpha * (fOpacity * 2)))), (char*)substr[i].str.c_str());
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
	ImGui::PopStyleColor(4);

	if (bOpened)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.f, 2.f));
		//if (CChat::Get()->Opened())
			//ShowCursor(TRUE);

		ImGui::SetNextWindowPos(ImVec2(0, 190), ImGuiSetCond_Always);
		ImGui::Begin("ChatInput", &bOpened, ImVec2(450, 50), .0f, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		ImGui::PushItemWidth(400);
		ImGui::PushFont(CGlobals::Get().chatFont);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(0.f, 0.f, 0.f, 0.5f));
		auto callback = [](ImGuiTextEditCallbackData* data) {
			switch (data->EventFlag)
			{
				case ImGuiInputTextFlags_CallbackCompletion:
				{
					// Example of TEXT COMPLETION

					// Locate beginning of current word
					const char* word_end = data->Buf + data->CursorPos;
					const char* word_start = word_end;
					while (word_start > data->Buf)
					{
						const char c = word_start[-1];
						if (c == ' ' || c == '\t' || c == ',' || c == ';')
							break;
						word_start--;
					}

					// Build a list of candidates
					ImVector<const char*> candidates;
					auto players = CNetworkPlayer::All();
					for (int i = 0; i < players.size(); i++)
					{
						char *chr = _strdup(players[i]->GetName().c_str());
						if (strncmp(chr, word_start, (int)(word_end - word_start)) == 0)
							candidates.push_back(chr);
					}
						
					if (strncmp(CConfig::Get()->sNickName.c_str(), word_start, (int)(word_end - word_start)) == 0)
						candidates.push_back(_strdup(CConfig::Get()->sNickName.c_str()));

					if (candidates.Size == 0)
					{
					}
					else if (candidates.Size == 1)
					{
						// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
						data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
						data->InsertChars(data->CursorPos, candidates[0]);
						//data->InsertChars(data->CursorPos, " ");
					}
					else
					{
						// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
						int match_len = (int)(word_end - word_start);
						for (;;)
						{
							int c = 0;
							bool all_candidates_matches = true;
							for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
								if (i == 0)
									c = toupper(candidates[i][match_len]);
								else if (c == 0 || c != toupper(candidates[i][match_len]))
									all_candidates_matches = false;
							if (!all_candidates_matches)
								break;
							match_len++;
						}

						if (match_len > 0)
						{
							data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
							data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
						}
					}

					break;
				}
				case ImGuiInputTextFlags_CallbackHistory:
				{
					const int prev_history_pos = HistoryPos;
					if (data->EventKey == ImGuiKey_UpArrow)
					{
						if (HistoryPos == -1)
							HistoryPos = History.Size - 1;
						else if (HistoryPos > 0)
							HistoryPos--;
					}
					else if (data->EventKey == ImGuiKey_DownArrow)
					{
						if (HistoryPos != -1)
							if (++HistoryPos >= History.Size)
								HistoryPos = -1;
					}

					if (prev_history_pos != HistoryPos)
					{
						data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = (int)snprintf(data->Buf, (size_t)data->BufSize, "%s", (HistoryPos >= 0) ? History[HistoryPos] : "");
						data->BufDirty = true;
					}
					break;
				}
			}
			return 0;
		};
		if (ImGui::InputText("", CGlobals::Get().chatBuffer, 256, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackCompletion, callback))
		{
			if (strlen(CGlobals::Get().chatBuffer))
			{
				HistoryPos = -1;
				for (int i = History.Size - 1; i >= 0; i--)
				if (_stricmp(History[i], CGlobals::Get().chatBuffer) == 0)
				{
					free(History[i]);
					History.erase(History.begin() + i);
					break;
				}
				History.push_back(_strdup(CGlobals::Get().chatBuffer));

				if (CGlobals::Get().chatBuffer[0] == '/')
				{
					if (_commandProcess != nullptr)
					{
						if (_commandProcess(CGlobals::Get().chatBuffer) != 1)
						{
							RakNet::BitStream sendmessage;
							RakNet::RakString outStr(CGlobals::Get().chatBuffer);

							sendmessage.Write((MessageID)ID_COMMAND_MESSAGE);
							sendmessage.Write(outStr);
							CNetworkConnection::Get()->client->Send(&sendmessage,HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
						}
					}
				}
				else
				{
					RakNet::BitStream sendmessage;
					RakNet::RakString outStr(CGlobals::Get().chatBuffer);
					sendmessage.Write((MessageID)ID_CHAT_MESSAGE);
					sendmessage.Write(outStr);
					CNetworkConnection::Get()->client->Send(&sendmessage, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
				}
				bScrollBottom = true;
				strcpy_s(CGlobals::Get().chatBuffer, 256, "");
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
		ImGui::PopStyleColor(1);
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

	//(*CGlobals::Get().canLangChange) = true;
	bOpened = true;
	bJustOpened = true;
	//ShowCursor(TRUE);
}


void CChat::Close()
{
	//PLAYER::SET_PLAYER_CONTROL(PLAYER::PLAYER_ID(), true, 0);
	//(*CGlobals::Get().canLangChange) = false;
	bOpened = false;
	//ShowCursor(FALSE);
	ulLastUpdate = timeGetTime() - 4000;
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
		//case VK_F6:
			//Chat->Toggle();
			//break;
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
		/*case 0x54:
		{
			if (Chat->bOpened) break;
			bool _break = false;
			for (auto m : CNetworkUI::Get()->menus)
				if (m->shown)
				{
					_break = true;
					break;
				}
			if (_break) break;
			Chat->Open();
			break;
		}*/
		}
	}
}