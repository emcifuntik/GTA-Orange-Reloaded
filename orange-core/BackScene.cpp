#include "stdafx.h"

void BackScene()
{
	auto viewPortGame = GTA::CViewportGame::Get();
	ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiSetCond_Always);
	ImGui::Begin("Background", 0, ImVec2((float)viewPortGame->Width, (float)viewPortGame->Height), 0.f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
	if (CGlobals::Get().currentGameState == GameStatePlaying && CGlobals::Get().isDebug)
	{
		std::stringstream ss;
		ss << "Peds pool: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool.Count() << " / " << ReplayInterfaces::Get()->ReplayInterfacePed->pool.Capacity() << std::endl <<
			"Vehicles pool: " << ReplayInterfaces::Get()->ReplayInterfaceVeh->pool.Count() << " / " << ReplayInterfaces::Get()->ReplayInterfaceVeh->pool.Capacity() << std::endl <<
			"Objects pool: " << ReplayInterfaces::Get()->ReplayInterfaceObject->pool.Count() << " / " << ReplayInterfaces::Get()->ReplayInterfaceObject->pool.Capacity() << std::endl <<
			"Ped pos: " << CLocalPlayer::Get()->GetPosition().ToString() << std::endl <<
			"Ped heading: " << CLocalPlayer::Get()->GetHeading() << std::endl <<
			"FPS: " << ImGui::GetIO().Framerate;
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, 14.f, ImVec2(0.23f * viewPortGame->Width, 0.85f * viewPortGame->Height),
			ImColor(0x21, 0x96, 0xF3, 0xFF), ss.str().c_str());
	}

	if (CGlobals::Get().isDebug)
	{
		for (int i = 0, cnt = 0; i < ReplayInterfaces::Get()->ReplayInterfacePed->pool.Capacity(); ++i)
		{
			if (ReplayInterfaces::Get()->ReplayInterfacePed->pool.GetHandle(i) == -1)
				continue;
			if ((ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position - CWorld::Get()->CPedPtr->Position).Length() > 50.f)
				continue;
			cnt++;
			
			int primaryActive = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->PrimaryTasks->ActiveTask;
			if (primaryActive > -1)
			{
				std::stringstream ss2;
				GTA::CTask *primaryTask = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->PrimaryTasks->TasksArray[primaryActive];
				ss2 << "Primary: " << primaryTask->GetTree() << std::endl;

				CVector3 screenPos;
				CGraphics::Get()->WorldToScreen(CVector3(
					ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fX,
					ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fY,
					ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fZ + 1.f),
					screenPos);
				auto viewPortGame = GTA::CViewportGame::Get();
				float x = screenPos.fX * viewPortGame->Width;
				float y = screenPos.fY * viewPortGame->Height;

				float font_size = 18.0f;
				ImVec2 textSize = CGlobals::Get().chatFont->CalcTextSizeA(font_size, 1000.f, 1000.f, ss2.str().c_str());

				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 - 1, y - textSize.y / 2 - 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 + 1, y - textSize.y / 2 + 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 + 1, y - textSize.y / 2 - 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 - 1, y - textSize.y / 2 + 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2, y - textSize.y / 2), ImColor(0xFF, 0xFF, 0xFF, 0xFF), ss2.str().c_str());
			}

			int secondaryActive = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->SecondaryTasks->ActiveTask;
			if (secondaryActive > -1)
			{
				std::stringstream ss2;
				GTA::CTask *secondaryTask = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->SecondaryTasks->TasksArray[secondaryActive];
				ss2 << "Secondary: " << secondaryTask->GetTree() << std::endl;

				CVector3 screenPos;
				CGraphics::Get()->WorldToScreen(CVector3(
					ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fX,
					ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fY,
					ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fZ + 0.5f),
					screenPos);
				auto viewPortGame = GTA::CViewportGame::Get();
				float x = screenPos.fX * viewPortGame->Width;
				float y = screenPos.fY * viewPortGame->Height;

				float font_size = 18.0f;
				ImVec2 textSize = CGlobals::Get().chatFont->CalcTextSizeA(font_size, 1000.f, 1000.f, ss2.str().c_str());

				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 - 1, y - textSize.y / 2 - 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 + 1, y - textSize.y / 2 + 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 + 1, y - textSize.y / 2 - 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 - 1, y - textSize.y / 2 + 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2, y - textSize.y / 2), ImColor(0xFF, 0xFF, 0xFF, 0xFF), ss2.str().c_str());
			}

			int movementActive = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->MovementTasks->ActiveTask;
			if (movementActive > -1)
			{
				std::stringstream ss2;
				GTA::CTask *movementTask = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->MovementTasks->TasksArray[movementActive];
				ss2 << "Movement: " << movementTask->GetTree() << std::endl;

				CVector3 screenPos;
				CGraphics::Get()->WorldToScreen(CVector3(
					ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fX,
					ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fY,
					ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fZ),
					screenPos);
				auto viewPortGame = GTA::CViewportGame::Get();
				float x = screenPos.fX * viewPortGame->Width;
				float y = screenPos.fY * viewPortGame->Height;

				float font_size = 18.0f;
				ImVec2 textSize = CGlobals::Get().chatFont->CalcTextSizeA(font_size, 1000.f, 1000.f, ss2.str().c_str());

				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 - 1, y - textSize.y / 2 - 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 + 1, y - textSize.y / 2 + 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 + 1, y - textSize.y / 2 - 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 - 1, y - textSize.y / 2 + 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2, y - textSize.y / 2), ImColor(0xFF, 0xFF, 0xFF, 0xFF), ss2.str().c_str());
			}

			int motionActive = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->MotionTasks->ActiveTask;
			if (motionActive > -1)
			{
				std::stringstream ss2;
				GTA::CTask *motionTask = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->MotionTasks->TasksArray[motionActive];
				ss2 << "Motion: " << motionTask->GetTree() << std::endl;

				CVector3 screenPos;
				CGraphics::Get()->WorldToScreen(CVector3(
					ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fX,
					ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fY,
					ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fZ - 0.5f),
					screenPos);
				auto viewPortGame = GTA::CViewportGame::Get();
				float x = screenPos.fX * viewPortGame->Width;
				float y = screenPos.fY * viewPortGame->Height;

				float font_size = 18.0f;
				ImVec2 textSize = CGlobals::Get().chatFont->CalcTextSizeA(font_size, 1000.f, 1000.f, ss2.str().c_str());

				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 - 1, y - textSize.y / 2 - 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 + 1, y - textSize.y / 2 + 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 + 1, y - textSize.y / 2 - 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2 - 1, y - textSize.y / 2 + 1), ImColor(0, 0, 0, 255), ss2.str().c_str());
				ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, font_size, ImVec2(x - textSize.x / 2, y - textSize.y / 2), ImColor(0xFF, 0xFF, 0xFF, 0xFF), ss2.str().c_str());
			}
			/*ss << "Handle: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool.GetHandle(i) << std::endl <<
			"Position: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.ToString() << std::endl <<
			"Health: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Health << std::endl <<
			"Model: 0x" << std::hex << ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->PedModelInfo->ModelHash << std::endl <<
			ss2.str();*/
			if (cnt >= ReplayInterfaces::Get()->ReplayInterfacePed->pool.Count())
				break;
		}

		/*for (int i = 0, cnt = 0; i < ReplayInterfaces::Get()->ReplayInterfaceVeh->pool.Capacity(); ++i)
		{
		if (ReplayInterfaces::Get()->ReplayInterfaceVeh->pool.GetHandle(i) == -1)
		continue;
		if ((ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->Position - CWorld::Get()->CPedPtr->Position).Length() > 50.f)
		continue;
		cnt++;
		ss = std::stringstream();
		ss << "Handle: " << ReplayInterfaces::Get()->ReplayInterfaceVeh->pool.GetHandle(i) << std::endl <<
		"Position: " << ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->Position.ToString() << std::endl <<
		"Model: 0x" << std::hex << ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->VehicleModelInfo->ModelHash;
		CGraphics::Get()->Draw3DText(ss.str(), 0.3f, ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->Position.fX,
		ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->Position.fY,
		ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->Position.fZ, { 255, 255, 255, 255 });
		if (cnt >= ReplayInterfaces::Get()->ReplayInterfaceVeh->pool.Count())
		break;
		}

		for (int i = 0, cnt = 0; i < ReplayInterfaces::Get()->ReplayInterfaceObject->pool.Capacity(); ++i)
		{
		if (ReplayInterfaces::Get()->ReplayInterfaceObject->pool.GetHandle(i) == -1)
		continue;
		if ((ReplayInterfaces::Get()->ReplayInterfaceObject->pool[i]->Position - CWorld::Get()->CPedPtr->Position).Length() > 50.f)
		continue;
		cnt++;
		ss = std::stringstream();
		ss << "Handle: " << ReplayInterfaces::Get()->ReplayInterfaceObject->pool.GetHandle(i) << std::endl <<
		"Position: " << ReplayInterfaces::Get()->ReplayInterfaceObject->pool[i]->Position.ToString() << std::endl <<
		"Model: 0x" << std::hex << ReplayInterfaces::Get()->ReplayInterfaceObject->pool[i]->ModelInfo->ModelHash;
		CGraphics::Get()->Draw3DText(ss.str(), 0.3f, ReplayInterfaces::Get()->ReplayInterfaceObject->pool[i]->Position.fX,
		ReplayInterfaces::Get()->ReplayInterfaceObject->pool[i]->Position.fY,
		ReplayInterfaces::Get()->ReplayInterfaceObject->pool[i]->Position.fZ, { 255, 255, 255, 255 });
		if (cnt >= ReplayInterfaces::Get()->ReplayInterfaceObject->pool.Count())
		break;
		}*/
	}

	CNetworkPlayer::Render();
	CNetwork3DText::Render();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::End();
}

GUI(BackScene);