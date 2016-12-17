#include "stdafx.h"

CNetworkUI* CNetworkUI::Instance = nullptr;

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
	RakNet::BitStream bsOut;

	if (isUpNow || wasDownBefore) bsOut.Write(true);
	else bsOut.Write(false);

	bsOut.Write(key);

	CRPCPlugin::Get()->rpc.Signal("KeyEvent", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
}

CNetworkUI * CNetworkUI::Get()
{
	if (!Instance)
		Instance = new CNetworkUI();
	return Instance;
}
