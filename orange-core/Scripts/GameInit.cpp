#include "stdafx.h"

void Action()
{
	bool teleported = false;
	while (true)
	{
		if (!teleported)
		{
			SCRIPT::_REQUEST_STREAMED_SCRIPT(Utils::Hash("standard_global_init"));
			CGlobals::Get().InitializeOnline();
			ENTITY::SET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0.f, 0.f, 73.f, true, false, false, false);
			CGlobals::Get().currentcam = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", false);
			CAM::SET_CAM_COORD(CGlobals::Get().currentcam, 0.f, 0.f, 100.f);
			CAM::POINT_CAM_AT_COORD(CGlobals::Get().currentcam, 300.f, 300.f, 50.f);
			CAM::SET_CAM_ACTIVE(CGlobals::Get().currentcam, true);
			CAM::RENDER_SCRIPT_CAMS(true, false, 0, true, false);
			UI::DISPLAY_HUD(false);
			UI::DISPLAY_RADAR(false);
			CGlobals::Get().displayServerBrowser = true;
			teleported = true;
			CChat::Get()->AddChatMessage("Grand Theft Auto: Orange loaded");
		}
		scriptWait(0);
	}
}

SCRIPT("gameInit", Action, gameInit);
