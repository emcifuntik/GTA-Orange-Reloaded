#include "stdafx.h"

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
	return true;
}

void CNetworkUI::DXRender()
{
	for each(auto menu in menus)
	{
		if(!menu->shown) continue;
		ImVec2 textSize = CGlobals::Get().tagFont->CalcTextSizeA(48, 1000.f, 1000.f, menu->name.c_str());

		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(menu->pos.fX, menu->pos.fY), ImVec2(menu->pos.fX + 200, menu->pos.fY + 50), ImColor(38, 38, 38, 220), 0.f, 15);
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().tagFont, 48, ImVec2(menu->pos.fX + 100 - textSize.x / 2, menu->pos.fY + 25 - textSize.y / 2), ImColor(0xFF, 0xFF, 0xFF, 0xFF), menu->name.c_str());

		unsigned char pos = 0;
		for each(auto child in menu->children)
		{
			ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(menu->pos.fX, menu->pos.fY + 50 + pos * 20), ImVec2(menu->pos.fX + 200, menu->pos.fY + 50 + pos * 20 + 20), pos == menu->active ? ImColor(100, 100, 100, 155) : ImColor(200, 200, 200, 155), 0.f, 15);
			ImGui::GetWindowDrawList()->AddText(CGlobals::Get().tagFont, 16, ImVec2(menu->pos.fX + 10, menu->pos.fY + 50 + pos * 20 + 2), ImColor(0xFF, 0xFF, 0xFF, 0xFF), child->name.c_str());
			pos++;
		}
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
	if (el->type == 1) el->cb();
	return true;
}

CNetworkUI::~CNetworkUI()
{

}

void CNetworkUI::ScriptKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow)
{
	//if (isUpNow && wasDownBefore) log << "down: " << std::hex << key << std::endl;
	//else if (!isUpNow && !wasDownBefore) log << "up: " << std::hex << key << std::endl;

	if (!CChat::Get()->Opened()) return;

	if (!isUpNow)
	{
		if (key == VK_DOWN)
			for each(auto menu in menus)
				if (menu->shown) {
					menu->active++;
					if (menu->active > menu->children.size() - 1) menu->active = 0;
				}

		if (key == VK_UP)
			for each(auto menu in menus)
				if (menu->shown) {
					if (menu->active == 0) menu->active = menu->children.size() - 1;
					else menu->active--;
				}

		if (key == VK_RETURN)
			for each(auto menu in menus)
				if (menu->shown) {
					CNetworkUI::Get()->Call(menu);
				}
	}

	if (isUpNow && wasDownBefore)
	{
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
