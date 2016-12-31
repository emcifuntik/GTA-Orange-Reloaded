#include "stdafx.h"

void FreemodeCustomize()
{
	if (CGlobals::Get().displayCustomizeWindow)
	{
		ShowCursor(TRUE);
		CConfig::Get();
		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiSetCond_Always);
		ImGui::SetNextWindowPosCenter(ImGuiSetCond_Always);
		ImGui::PushFont(CGlobals::Get().chatFont);
		ImGui::Begin("Character customizer", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		if (ImGui::CollapsingHeader("Head overlay")) {
			ImGui::Checkbox("Off 01", &CGlobals::Get().custom.BlemishesOff); ImGui::SameLine(); ImGui::DragInt("Blemishes", &CGlobals::Get().custom.Blemishes, 0.1f, 0, PED::_GET_NUM_HEAD_OVERLAY_VALUES(0) - 1);
			ImGui::Checkbox("Off 02", &CGlobals::Get().custom.Facial_HairOff); ImGui::SameLine(); ImGui::DragInt("Facial Hair", &CGlobals::Get().custom.Facial_Hair, 0.1f, 0, PED::_GET_NUM_HEAD_OVERLAY_VALUES(1) - 1);
			ImGui::Checkbox("Off 03", &CGlobals::Get().custom.EyebrowsOff); ImGui::SameLine(); ImGui::DragInt("Eyebrows", &CGlobals::Get().custom.Eyebrows, 0.1f, 0, PED::_GET_NUM_HEAD_OVERLAY_VALUES(2) - 1);
			ImGui::Checkbox("Off 04", &CGlobals::Get().custom.AgeingOff); ImGui::SameLine(); ImGui::DragInt("Ageing", &CGlobals::Get().custom.Ageing, 0.1f, 0, PED::_GET_NUM_HEAD_OVERLAY_VALUES(3) - 1);
			ImGui::Checkbox("Off 05", &CGlobals::Get().custom.MakeupOff); ImGui::SameLine(); ImGui::DragInt("Makeup", &CGlobals::Get().custom.Makeup, 0.1f, 0, PED::_GET_NUM_HEAD_OVERLAY_VALUES(4) - 1);
			ImGui::Checkbox("Off 06", &CGlobals::Get().custom.BlushOff); ImGui::SameLine(); ImGui::DragInt("Blush", &CGlobals::Get().custom.Blush, 0.1f, 0, PED::_GET_NUM_HEAD_OVERLAY_VALUES(5) - 1);
			ImGui::Checkbox("Off 07", &CGlobals::Get().custom.ComplexionOff); ImGui::SameLine(); ImGui::DragInt("Complexion", &CGlobals::Get().custom.Complexion, 0.1f, 0, PED::_GET_NUM_HEAD_OVERLAY_VALUES(6) - 1);
			ImGui::Checkbox("Off 08", &CGlobals::Get().custom.Sun_DamageOff); ImGui::SameLine(); ImGui::DragInt("Sun Damage", &CGlobals::Get().custom.Sun_Damage, 0.1f, 0, PED::_GET_NUM_HEAD_OVERLAY_VALUES(7) - 1);
			ImGui::Checkbox("Off 09", &CGlobals::Get().custom.LipstickOff); ImGui::SameLine(); ImGui::DragInt("Lipstick", &CGlobals::Get().custom.Lipstick, 0.1f, 0, PED::_GET_NUM_HEAD_OVERLAY_VALUES(8) - 1);
			ImGui::Checkbox("Off 10", &CGlobals::Get().custom.Moles_FrecklesOff); ImGui::SameLine(); ImGui::DragInt("Moles/Freckles", &CGlobals::Get().custom.Moles_Freckles, 0.1f, 0, PED::_GET_NUM_HEAD_OVERLAY_VALUES(9) - 1);
			ImGui::Checkbox("Off 11", &CGlobals::Get().custom.Chest_HairOff); ImGui::SameLine(); ImGui::DragInt("Chest Hair", &CGlobals::Get().custom.Chest_Hair, 0.1f, 0, PED::_GET_NUM_HEAD_OVERLAY_VALUES(10) - 1);
			ImGui::Checkbox("Off 12", &CGlobals::Get().custom.Body_BlemishesOff); ImGui::SameLine(); ImGui::DragInt("Body Blemishes", &CGlobals::Get().custom.Body_Blemishes, 0.1f, 0, PED::_GET_NUM_HEAD_OVERLAY_VALUES(11) - 1);
			ImGui::Checkbox("Off 13", &CGlobals::Get().custom.Add_Body_BlemishesOff); ImGui::SameLine(); ImGui::DragInt("Add Body Blemishes", &CGlobals::Get().custom.Add_Body_Blemishes, 0.1f, 0, PED::_GET_NUM_HEAD_OVERLAY_VALUES(12) - 1);
		}
		if (CGlobals::Get().custom.Blemishes != CGlobals::Get().lastCustom.Blemishes)
		{
			CGlobals::Get().lastCustom.Blemishes = CGlobals::Get().custom.Blemishes;
			CScriptInvoker::Get().Push([=]() {
				PED::SET_PED_HEAD_OVERLAY(PLAYER::PLAYER_PED_ID(), 0, CGlobals::Get().custom.Blemishes, 1.0f);
			});
		}
		if (CGlobals::Get().custom.Facial_Hair != CGlobals::Get().lastCustom.Facial_Hair)
		{
			CGlobals::Get().lastCustom.Facial_Hair = CGlobals::Get().custom.Facial_Hair;
			CScriptInvoker::Get().Push([=]() {
				PED::SET_PED_HEAD_OVERLAY(PLAYER::PLAYER_PED_ID(), 1, CGlobals::Get().custom.Facial_Hair, 1.0f);
			});
		}
		if (CGlobals::Get().custom.Eyebrows != CGlobals::Get().lastCustom.Eyebrows)
		{
			CGlobals::Get().lastCustom.Eyebrows = CGlobals::Get().custom.Eyebrows;
			CScriptInvoker::Get().Push([=]() {
				PED::SET_PED_HEAD_OVERLAY(PLAYER::PLAYER_PED_ID(), 2, CGlobals::Get().custom.Eyebrows, 1.0f);
			});
		}
		if (CGlobals::Get().custom.Ageing != CGlobals::Get().lastCustom.Ageing)
		{
			CGlobals::Get().lastCustom.Ageing = CGlobals::Get().custom.Ageing;
			CScriptInvoker::Get().Push([=]() {
				PED::SET_PED_HEAD_OVERLAY(PLAYER::PLAYER_PED_ID(), 3, CGlobals::Get().custom.Ageing, 1.0f);
			});
		}
		if (CGlobals::Get().custom.Makeup != CGlobals::Get().lastCustom.Makeup)
		{
			CGlobals::Get().lastCustom.Makeup = CGlobals::Get().custom.Makeup;
			CScriptInvoker::Get().Push([=]() {
				PED::SET_PED_HEAD_OVERLAY(PLAYER::PLAYER_PED_ID(), 4, CGlobals::Get().custom.Makeup, 1.0f);
			});
		}
		if (CGlobals::Get().custom.Blush != CGlobals::Get().lastCustom.Blush)
		{
			CGlobals::Get().lastCustom.Blush = CGlobals::Get().custom.Blush;
			CScriptInvoker::Get().Push([=]() {
				PED::SET_PED_HEAD_OVERLAY(PLAYER::PLAYER_PED_ID(), 5, CGlobals::Get().custom.Blush, 1.0f);
			});
		}
		if (CGlobals::Get().custom.Complexion != CGlobals::Get().lastCustom.Complexion)
		{
			CGlobals::Get().lastCustom.Complexion = CGlobals::Get().custom.Complexion;
			CScriptInvoker::Get().Push([=]() {
				PED::SET_PED_HEAD_OVERLAY(PLAYER::PLAYER_PED_ID(), 6, CGlobals::Get().custom.Complexion, 1.0f);
			});
		}
		if (CGlobals::Get().custom.Sun_Damage != CGlobals::Get().lastCustom.Sun_Damage)
		{
			CGlobals::Get().lastCustom.Sun_Damage = CGlobals::Get().custom.Sun_Damage;
			CScriptInvoker::Get().Push([=]() {
				PED::SET_PED_HEAD_OVERLAY(PLAYER::PLAYER_PED_ID(), 7, CGlobals::Get().custom.Sun_Damage, 1.0f);
			});
		}
		if (CGlobals::Get().custom.Lipstick != CGlobals::Get().lastCustom.Lipstick)
		{
			CGlobals::Get().lastCustom.Lipstick = CGlobals::Get().custom.Lipstick;
			CScriptInvoker::Get().Push([=]() {
				PED::SET_PED_HEAD_OVERLAY(PLAYER::PLAYER_PED_ID(), 8, CGlobals::Get().custom.Lipstick, 1.0f);
			});
		}
		if (CGlobals::Get().custom.Moles_Freckles != CGlobals::Get().lastCustom.Moles_Freckles)
		{
			CGlobals::Get().lastCustom.Moles_Freckles = CGlobals::Get().custom.Moles_Freckles;
			CScriptInvoker::Get().Push([=]() {
				PED::SET_PED_HEAD_OVERLAY(PLAYER::PLAYER_PED_ID(), 9, CGlobals::Get().custom.Moles_Freckles, 1.0f);
			});
		}
		if (CGlobals::Get().custom.Chest_Hair != CGlobals::Get().lastCustom.Chest_Hair)
		{
			CGlobals::Get().lastCustom.Chest_Hair = CGlobals::Get().custom.Chest_Hair;
			CScriptInvoker::Get().Push([=]() {
				PED::SET_PED_HEAD_OVERLAY(PLAYER::PLAYER_PED_ID(), 10, CGlobals::Get().custom.Chest_Hair, 1.0f);
			});
		}
		if (CGlobals::Get().custom.Body_Blemishes != CGlobals::Get().lastCustom.Body_Blemishes)
		{
			CGlobals::Get().lastCustom.Body_Blemishes = CGlobals::Get().custom.Body_Blemishes;
			CScriptInvoker::Get().Push([=]() {
				PED::SET_PED_HEAD_OVERLAY(PLAYER::PLAYER_PED_ID(), 11, CGlobals::Get().custom.Body_Blemishes, 1.0f);
			});
		}
		if (CGlobals::Get().custom.Add_Body_Blemishes != CGlobals::Get().lastCustom.Add_Body_Blemishes)
		{
			CGlobals::Get().lastCustom.Add_Body_Blemishes = CGlobals::Get().custom.Add_Body_Blemishes;
			CScriptInvoker::Get().Push([=]() {
				PED::SET_PED_HEAD_OVERLAY(PLAYER::PLAYER_PED_ID(), 12, CGlobals::Get().custom.Add_Body_Blemishes, 1.0f);
			});
		}

		if (ImGui::Button("Finish"))
		{
			CGlobals::Get().displayCustomizeWindow = false;
			ShowCursor(FALSE);
		}
		ImGui::End();
		ImGui::PopFont();
	}
}

GUI(FreemodeCustomize);