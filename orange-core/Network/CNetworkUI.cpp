#include "stdafx.h"

CNetworkUI* CNetworkUI::Instance = nullptr;

screenInfo_ screenInfo;
bool keystate[0xA5];

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

CNetworkUI::~CNetworkUI()
{

}

void CNetworkUI::ScriptKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow)
{
	//if (isUpNow && wasDownBefore) log << "down: " << std::hex << key << std::endl;
	//else if (!isUpNow && !wasDownBefore) log << "up: " << std::hex << key << std::endl;

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
