#include "stdafx.h"

char* ipls[] = {
	"facelobby",
	"shr_int",
	"FIBlobby",
	"post_hiest_unload",
	"refit_unload",
	"CS1_02_cf_onmission1",
	"CS1_02_cf_onmission2",
	"CS1_02_cf_onmission3",
	"CS1_02_cf_onmission4",
	"RC12B_Default",
	"SP1_10_real_interior",
	"burnt_switch_off",
	"id2_14_during1",
	"id2_14_during_door",
	"farm",
	"farmint",
	"des_farmhouse",
	"Coroner_Int_on",
	"FINBANK",
	"ch1_02_open",
	"hei_yacht_heist",
	"hei_yacht_heist_enginrm",
	"hei_yacht_heist_lounge",
	"hei_yacht_heist_bridge",
	"hei_yacht_heist_bar",
	"hei_yacht_heist_bedrm",
	"hei_carrier",
	"hei_Carrier_int1",
	"hei_Carrier_int2",
	"hei_Carrier_int3",
	"hei_Carrier_int4",
	"hei_Carrier_int5",
	"hei_Carrier_int6",
	"hei_yacht_heist_distantlights",
	"hei_yacht_heist_lodlights",
	"hei_carrier_DistantLights",
	"hei_carrier_LODLights",
	"lr_cs6_08_grave_closed",
	"bkr_bi_hw1_13_int",
	"hei_bi_hw1_13_door",
	"bkr_bi_id1_23_door",
	"FINBANK",
	"v_tunnel_hole",
	"DT1_03_Shutter",
	"DT1_03_Gr_Closed",
	"canyonriver01",
	"railing_start",
	"farm",
	"farmint",
	"farm_props",
	"CS3_05_water_grp1",
	"CS3_05_water_grp2"
};

void Action()
{
	bool teleported = false;

	while (true)
	{
		if (!teleported)
		{
			int map = GRAPHICS::REQUEST_SCALEFORM_MOVIE("minimap");
			GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION(map, "MULTIPLAYER_IS_ACTIVE");
			GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_BOOL(true);
			GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_BOOL(false);
			GRAPHICS::_POP_SCALEFORM_MOVIE_FUNCTION_VOID();

			SCRIPT::_REQUEST_STREAMED_SCRIPT(Utils::Hash("standard_global_init"));

			DLC2::_LOAD_MP_DLC_MAPS();
			GAMEPLAY::_ENABLE_MP_DLC_MAPS(true);

			for (int i = 0; i < sizeof(ipls) / sizeof(ipls[0]); i++)
				STREAMING::REQUEST_IPL(ipls[i]);

			Entity glass = OBJECT::CREATE_OBJECT_NO_OFFSET(Utils::Hash("prop_showroom_glass_1b"), -59.77f, -1098.77f, 27.2f, 1, true, true);
			ENTITY::SET_ENTITY_ROTATION(glass, 0.f, 0.f, 121.5f, 2, 1);
			ENTITY::FREEZE_ENTITY_POSITION(glass, true);

			ENTITY::SET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 363.871f, 621.555f, 78.44f, true, false, false, false);
			CGlobals::Get().currentcam = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", false);
			CAM::SET_CAM_COORD(CGlobals::Get().currentcam, 301.194f, 485.823f, 209.966f);
			CAM::POINT_CAM_AT_COORD(CGlobals::Get().currentcam, 340.f, 180.f, 200.f);
			CAM::SET_CAM_ACTIVE(CGlobals::Get().currentcam, true);
			CAM::RENDER_SCRIPT_CAMS(true, false, 0, true, false);
			UI::DISPLAY_HUD(false);
			UI::DISPLAY_RADAR(false);
			CLocalPlayer::Get()->ChangeModel(Utils::Hash("mp_m_freemode_01"));
			CScriptEngine::Get()->Init();
			CGlobals::Get().displayServerBrowser = true;
			teleported = true;
			std::stringstream ss;
			ss << "{E30022}" << u8"\ueffb" << "{FFFFFF} Grand Theft Auto: {FF8F00}Orange {FFFFFF}loaded";
			CChat::Get()->AddChatMessage(ss.str());
		}

		if(CNetworkConnection::Get()->bClear) {
			CScriptEngine::Close();
			CScriptEngine::Get()->Init();

			CNetworkPlayer::Clear();
			CNetworkVehicle::Clear();
			CNetworkObject::Clear();

			CNetwork3DText::Clear();
			CNetworkMarker::Clear();
			CNetworkBlip::Clear();

			CNetworkConnection::Get()->bClear = false;
		}

		ImVec2 pos = ImGui::GetMousePos();

		CefMouseEvent event;
		event.x = pos.x;
		event.y = pos.y;

		event.modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
		event.modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
		event.modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;

		// Is that even needed?
		//if (UI::IsCtrlPressed) event.modifiers |= EVENTFLAG_CONTROL_DOWN;

		for (auto cefView : CEFCore::Get()->views)
		{
			cefView->m_pWebView->GetHost()->SendMouseMoveEvent(event, false);
		}

		//Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true);
		//GRAPHICS::DRAW_MARKER(2, pos.x, pos.y, pos.z, 0.0f, 0.0f, 0.0f, 180.0f, 0.0f, 0.0f, 0.75f, 0.75f, 0.75f, 204, 204, 0, 100, false, true, 2, false, false, false, false);

		scriptWait(0);
	}
}

SCRIPT(Action);
