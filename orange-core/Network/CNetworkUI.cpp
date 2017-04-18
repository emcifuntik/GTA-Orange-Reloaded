#include "stdafx.h"
#include "D3D11/imgui_internal.h"

CNetworkUI* CNetworkUI::Instance = nullptr;
std::vector<CMenu*> CNetworkUI::menus;

screenInfo_ screenInfo;

CNetworkUI::CNetworkUI()
{
}

bool CNetworkUI::Render()
{
	if (screenInfo.shown)
	{
		UI::_SET_TEXT_COMPONENT_FORMAT("STRING");
		UI::_ADD_TEXT_COMPONENT_STRING(screenInfo.msg);
		UI::_DISPLAY_HELP_TEXT_FROM_STRING_LABEL(0, 0, 1, -1);
	}

	for each(auto menu in menus)
	{
		if (menu->shown)
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
			return true;
		}
	}
	return true;
}

void CNetworkUI::DXRender()
{
	float k = GTA::CViewportGame::Get()->Width / 1920.f;

	for each(auto menu in menus)
	{
		if(!menu->shown) continue;
		ImVec2 textSize = CGlobals::Get().signpainterFont->CalcTextSizeA(72.f, 1000.f, 1000.f, menu->name.c_str());

		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(menu->pos.fX*k, menu->pos.fY*k), ImVec2((menu->pos.fX + 430)*k, (menu->pos.fY + 95)*k), ImColor(39, 39, 39, 200), 0.f, 15);
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().signpainterFont, 72.f * k, ImVec2((menu->pos.fX + 215 - textSize.x / 2)*k, (menu->pos.fY + 48 - textSize.y / 2)*k), ImColor(0xFF, 0xFF, 0xFF, 0xFF), menu->name.c_str());

		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(menu->pos.fX*k, (menu->pos.fY + 95)*k), ImVec2((menu->pos.fX + 430)*k, (menu->pos.fY + 95 + 37)*k), ImColor(52, 52, 52, 220), 0.f, 15);
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().londonFont, 21.f * k, ImVec2((menu->pos.fX + 15)*k, (menu->pos.fY + 95 + 10)*k), ImColor(255, 255, 255, 255), menu->subname.c_str());

		unsigned char pos = 0;
		for each(auto child in menu->children)
		{
			ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(menu->pos.fX * k, (menu->pos.fY + 132 + pos * 37)*k), ImVec2((menu->pos.fX + 430)*k, (menu->pos.fY + 132 + pos * 37 + 37)*k), pos == menu->active ? ImColor(255, 255, 255, 255) : ImColor(74, 74, 74, 200), 0.f, 15);
			ImGui::GetWindowDrawList()->AddText(CGlobals::Get().londonFont, 21.f * k, ImVec2((menu->pos.fX + 15)*k, (menu->pos.fY + 132 + pos * 37 + 10)*k), pos == menu->active ? ImColor(0, 0, 0, 255) : ImColor(255, 255, 255, 255), child->name.c_str());

			if (child->type == 2)
			{
				ImGui::GetWindowDrawList()->AddRectFilled(ImVec2((menu->pos.fX + 400)*k, (menu->pos.fY + 132 + pos * 37 + 7)*k), ImVec2((menu->pos.fX + 423)*k, (menu->pos.fY + 132 + pos * 37 + 30)*k), pos == menu->active ? ImColor(0, 0,  0, 255) : ImColor(255, 255, 255, 255), 0.f, 15);
				if(child->state) ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, 18.f * k, ImVec2((menu->pos.fX + 403)*k, (menu->pos.fY + 132 + pos * 37 + 10)*k), pos == menu->active ? ImColor(255, 255, 255, 255) : ImColor(0, 0, 0, 255), u8"\uf00c");
			}

			if (child->type == 3)
			{
				if (pos == menu->active)
				{
					auto io = ImGui::GetIO();

					int len = ImStrlenW(io.InputCharacters);

					if(len > 0)
					{
						char buf[17];
						ImTextStrToUtf8(buf, 17, io.InputCharacters, &io.InputCharacters[len]);
						bool call = true;

						if (buf[0] != '\0')
						{
							child->buffer += buf;
							for (int i = 0; i < child->buffer.size();)
							{
								if (child->buffer[i] == 8)
								{
									if (i > 0)
									{
										log << (int)child->buffer[i - 1] << std::endl;
										child->buffer.erase(i - 1, i + 1);
										i--;
									}
									else
									{
										child->buffer.erase(i, i + 1);
										call = false;
									}
								}
								else if (child->buffer[i] == 13)
								{
									child->buffer.erase(i, i + 1);
									call = false;
								}
								else if (child->buffer[i] < 0)
								{
									unsigned int c;
									child->buffer.erase(i, i + 2);
								}
								else
								{
									i++;
								}
								//log << (int)(child->buffer[i]) << std::endl;
							}
							if (call && child->cb)
							{
								CScriptInvoker::Get().Push(child->cb);
							}
	
						}
					}					
				}

				if (pos == menu->active && menu->jactive)
				{
					//ImGui::SetKeyboardFocusHere(0);
				}
				//else ImGui::CaptureKeyboardFromApp();

				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, 20.f * k, ImVec2((menu->pos.fX + 160)*k, (menu->pos.fY + 132 + pos * 37 + 6)*k), pos == menu->active ? ImColor(0, 0, 0, 255) : ImColor(255, 255, 255, 255), child->buffer.c_str());
			}

			if (child->type == 4)
			{
				ImGui::GetWindowDrawList()->AddRectFilled(ImVec2((menu->pos.fX + 160)*k, (menu->pos.fY + 132 + pos * 37 + 16)*k), ImVec2((menu->pos.fX + 420)*k, (menu->pos.fY + 132 + pos * 37 + 21)*k), pos == menu->active ? ImColor(0, 0, 0, 255) : ImColor(255, 255, 255, 255));
				ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2((menu->pos.fX + 160 + 260*child->state)*k, (menu->pos.fY + 132 + pos * 37 + 19)*k), 4, pos == menu->active ? ImColor(0, 0, 0, 255) : ImColor(255, 255, 255, 255));
			}

			pos++;
		}
		menu->jactive = false;
	}
}

bool CNetworkUI::SetScreenInfo(const char* msg)
{
	screenInfo.shown = true;
	screenInfo.msg = _strdup(msg);
	return true;
}

bool CNetworkUI::UnsetScreenInfo()
{
	screenInfo.shown = false;
	screenInfo.msg = "";
	return true;
}

bool CNetworkUI::SendNotification(char* msg)
{
	UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(msg);
	UI::_DRAW_NOTIFICATION(0, 1);
	return true;
}

int CNetworkUI::AddMenu(CMenu *menu)
{
	menus.push_back(menu);
	return menus.size()-1;
}

bool CNetworkUI::Call(CMenu * menu)
{
	auto el = menu->children[menu->active];
	
	if (el->type == 2)
		el->state = el->state > 0 ? 0 : 1;

	if (el->cb && (el->type == 1 || el->type == 2)) {
		CScriptInvoker::Get().Push(el->cb);
		return true;
	}

	return false;
}

CNetworkUI::~CNetworkUI()
{
	
}

void CNetworkUI::ScriptKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow)
{
	//if (isUpNow && wasDownBefore) log << "down: " << std::hex << key << std::endl;
	//else if (!isUpNow && !wasDownBefore) log << "up: " << std::hex << key << std::endl;

	if (CChat::Get()->Opened()) return;

	if (!isUpNow)
	{
		if (key == VK_DOWN)
			for each(auto menu in menus)
				if (menu->shown) {
					if (menu->children[menu->active]->type == 3) *(CGlobals::Get().canLangChange) = false;

					menu->active++;
					if (menu->active > menu->children.size() - 1) menu->active = 0;
					menu->jactive = true;

					if (menu->children[menu->active]->type == 3) *(CGlobals::Get().canLangChange) = true;
				}

		if (key == VK_UP)
			for each(auto menu in menus)
				if (menu->shown) {
					if (menu->children[menu->active]->type == 3) *(CGlobals::Get().canLangChange) = false;

					if (menu->active == 0) menu->active = menu->children.size() - 1;
					else menu->active--;
					menu->jactive = true;

					if (menu->children[menu->active]->type == 3) *(CGlobals::Get().canLangChange) = true;
				}

		if (key == VK_LEFT)
			for each(auto menu in menus)
				if (menu->shown) {
					auto c = menu->children[menu->active];
					if (c->type == 4 && c->state > 0)
					{
						auto c = menu->children[menu->active];
						c->state -= c->step;
						
						if (c->state < 0) c->state = 0;

						CScriptInvoker::Get().Push(c->cb);
					}
				}

		if (key == VK_RIGHT)
			for each(auto menu in menus)
				if (menu->shown) {
					auto c = menu->children[menu->active];
					if (c->type == 4 && c->state < 1)
					{
						c->state += c->step;

						if (c->state > 1) c->state = 1;

						CScriptInvoker::Get().Push(c->cb);
					}
				}

		if (key == VK_RETURN)
			for each(auto menu in menus)
				if (menu->shown) {
					if (CNetworkUI::Get()->Call(menu)) break;
				}
		
		/*if (key == VK_ESCAPE)
			for each(auto menu in menus)
				menu->shown = false;*/

		for each(auto menu in menus)
		{
			if (menu->shown && menu->children[menu->active]->type == 3) continue;
			if (key == menu->button) menu->shown ^= 1;
			//else log << "Pressed: 0x" << std::hex << key << " trigger: 0x" << std::hex << menu->button << std::endl;
		}
	}

	if (isUpNow && wasDownBefore)
	{
		for each(auto menu in menus)
		{
			if (menu->shown) return;
		}
		RakNet::BitStream bsOut;
		bsOut.Write(key);
		CRPCPlugin::Get()->rpc.Signal("KeyEvent", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	}
}

CNetworkUI * CNetworkUI::Get()
{
	if (!Instance)
		Instance = new CNetworkUI();
	return Instance;
}

void CNetworkUI::Clear()
{
	for each(auto menu in Get()->menus)
	{
		for each(auto child in menu->children)
		{
			delete child;
		}
		delete menu;
	}
	Get()->menus.erase(Get()->menus.begin(), Get()->menus.end());
}
