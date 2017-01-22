#include "stdafx.h"

void BackScene()
{
	auto viewPortGame = GTA::CViewportGame::Get();
	ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiSetCond_Always);
	ImGui::Begin("Background", 0, ImVec2((float)viewPortGame->Width, (float)viewPortGame->Height), 0.f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
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

		const char* text = ss.str().c_str();
		float x = 0.23f * viewPortGame->Width;
		float y = 0.85f * viewPortGame->Height;
		ImColor color = ImColor(0x21, 0x96, 0xF3, 0xFF);

		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, 14.f, ImVec2(x - 1, y - 1), ImColor(0, 0, 0, 255), text);
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, 14.f, ImVec2(x + 1, y + 1), ImColor(0, 0, 0, 255), text);
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, 14.f, ImVec2(x + 1, y - 1), ImColor(0, 0, 0, 255), text);
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, 14.f, ImVec2(x - 1, y + 1), ImColor(0, 0, 0, 255), text);
		//ImGui::GetWindowDrawList()->AddText(CGlobals::Get().tagFont, font_size, ImVec2(tag.x - textSize.x / 2, tag.y), ImColor(0xFF, 0xFF, 0xFF, 0xFF), _name);

		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, 14.f, ImVec2(x, y), color, text);
	}

	if (CGlobals::Get().isDebug)
	{
		for (int i = 0, cnt = 0; i < ReplayInterfaces::Get()->ReplayInterfacePed->pool.Capacity(); ++i)
		{
			if (ReplayInterfaces::Get()->ReplayInterfacePed->pool.GetHandle(i) == -1)
				continue;
			if ((ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->vecPositionEntity - rage::CPedFactory::Get()->localPed->vecPositionEntity).Length() > 50.f)
				continue;
			cnt++;
			
			std::stringstream ss2;
			int primaryActive = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->pedIntelligence->pedTaskManager.Primary->iActiveTask;
			if (primaryActive > -1)
			{
				rage::CTask *primaryTask = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->pedIntelligence->pedTaskManager.Primary->GetTask();
				ss2 << "Primary: " << primaryTask->GetTree() << std::endl;
			}

			int secondaryActive = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->pedIntelligence->pedTaskManager.Secondary->iActiveTask;
			if (secondaryActive > -1)
			{
				rage::CTask *secondaryTask = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->pedIntelligence->pedTaskManager.Secondary->GetTask();
				ss2 << "Secondary: " << secondaryTask->GetTree() << std::endl;
			}

			int movementActive = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->pedIntelligence->pedTaskManager.Movement->iActiveTask;
			if (movementActive > -1)
			{
				rage::CTask *movementTask = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->pedIntelligence->pedTaskManager.Movement->GetTask();
				ss2 << "Movement: " << movementTask->GetTree() << std::endl;
			}

			int motionActive = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->pedIntelligence->pedTaskManager.Motion->iActiveTask;
			if (motionActive > -1)
			{
				rage::CTask *motionTask = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->pedIntelligence->pedTaskManager.Motion->GetTask();
				ss2 << "Motion: " << motionTask->GetTree() << std::endl;
			}


			/*ss << "Handle: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool.GetHandle(i) << std::endl <<
			"Position: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.ToString() << std::endl <<
			"Health: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Health << std::endl <<
			"Model: 0x" << std::hex << ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->PedModelInfo->ModelHash << std::endl <<
			ss2.str();*/

			ss2 << "RotationPotential: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->fRotationPotential << std::endl;
			ss2 << "ForwardPotential: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->fForwardPotential << std::endl;
			ss2 << "StrafePotential: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->fStrafePotential << std::endl;
			ss2 << "rage::CPed = 0x" << std::hex << ReplayInterfaces::Get()->ReplayInterfacePed->pool[i] << std::dec << std::endl;

			CVector3 screenPos;
			CGraphics::Get()->WorldToScreen(CVector3(
				ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->vecPositionEntity.fX,
				ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->vecPositionEntity.fY,
				ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->vecPositionEntity.fZ + 2.f),
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
			if (cnt >= ReplayInterfaces::Get()->ReplayInterfacePed->pool.Count())
				break;
		}

		for (int i = 0, cnt = 0; i < ReplayInterfaces::Get()->ReplayInterfaceVeh->pool.Capacity(); ++i)
		{
			if (ReplayInterfaces::Get()->ReplayInterfaceVeh->pool.GetHandle(i) == -1)
				continue;
			if ((ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->vecPositionEntity - rage::CPedFactory::Get()->localPed->vecPositionEntity).Length() > 50.f)
				continue;
			cnt++;

			int primaryActive = ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->intelligence->taskManager.primaryTasksPtr->iActiveTask;
			if (primaryActive > -1)
			{
				std::stringstream ss2;

				rage::CTask *primaryTask = ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->intelligence->taskManager.primaryTasksPtr->GetTask();
				ss2 << "Primary: " << primaryTask->GetTree() << std::endl;

				CVector3 screenPos;
				CGraphics::Get()->WorldToScreen(CVector3(
					ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->vecPositionEntity.fX,
					ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->vecPositionEntity.fY,
					ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->vecPositionEntity.fZ + 1.f),
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

			int secondaryActive = ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->intelligence->taskManager.primaryTasksPtr->iActiveTask;
			if (secondaryActive > -1)
			{
				std::stringstream ss2;
				rage::CTask *secondaryTask = ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->intelligence->taskManager.primaryTasksPtr->GetTask();
				ss2 << "Secondary: " << secondaryTask->GetTree() << std::endl;

				CVector3 screenPos;
				CGraphics::Get()->WorldToScreen(CVector3(
					ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->vecPositionEntity.fX,
					ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->vecPositionEntity.fY,
					ReplayInterfaces::Get()->ReplayInterfaceVeh->pool[i]->vecPositionEntity.fZ + 0.5f),
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
		
			if (cnt >= ReplayInterfaces::Get()->ReplayInterfaceVeh->pool.Count())
			break;
		}

		/*for (int i = 0, cnt = 0; i < ReplayInterfaces::Get()->ReplayInterfaceObject->pool.Capacity(); ++i)
		{
			if (ReplayInterfaces::Get()->ReplayInterfaceObject->pool.GetHandle(i) == -1)
			continue;
			if ((ReplayInterfaces::Get()->ReplayInterfaceObject->pool[i]->Position - CWorld::Get()->CPedPtr->Position).Length() > 50.f)
			continue;
			cnt++;

			
		}*/
	}

	CNetworkPlayer::Render();
	CNetwork3DText::Render();
	ImGui::PopStyleVar(7);
	ImGui::End();
}

GUI(BackScene);