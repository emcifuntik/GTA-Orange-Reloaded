#include "stdafx.h"

void StartScript(const char* name)
{
	Hash hash = Utils::Hash(name);
	SCRIPT::_REQUEST_STREAMED_SCRIPT(hash);
	while (!SCRIPT::_HAS_STREAMED_SCRIPT_LOADED(hash)) {
		scriptWait(0);
		SCRIPT::_REQUEST_STREAMED_SCRIPT(hash);
	}
	SYSTEM::_START_NEW_STREAMED_SCRIPT(hash, 1424);
	SCRIPT::_SET_STREAMED_SCRIPT_AS_NO_LONGER_NEEDED(hash);
}

void Action()
{
	bool teleported = false;
	while (true)
	{
		if (!teleported)
		{
			StartScript("mp_registration");
			StartScript("title_update_registration");

			StartScript("standard_global_init");
			StartScript("standard_global_reg");

			scriptWait(0);

			MOBILE::DESTROY_MOBILE_PHONE();

			//CGlobals::Get().InitializeOnline();

			ENTITY::SET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 363.871f, 621.555f, 78.44f, true, false, false, false);
			CGlobals::Get().currentcam = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", false);
			CAM::SET_CAM_COORD(CGlobals::Get().currentcam, 301.194f, 485.823f, 209.966f);
			CAM::POINT_CAM_AT_COORD(CGlobals::Get().currentcam, 340.f, 180.f, 200.f);
			CAM::SET_CAM_ACTIVE(CGlobals::Get().currentcam, true);
			CAM::RENDER_SCRIPT_CAMS(true, false, 0, true, false);
			UI::DISPLAY_HUD(false);
			UI::DISPLAY_RADAR(false);
			CLocalPlayer::Get()->ChangeModel(Utils::Hash("mp_m_freemode_01"));
			CGlobals::Get().displayServerBrowser = true;
			teleported = true;
			CChat::Get()->AddChatMessage("Grand Theft Auto: Orange loaded");
		}
		scriptWait(0);
	}
}

SCRIPT(Action);
