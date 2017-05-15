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

	float x1 = viewPortGame->Width - CGlobals::Get().tagFont->CalcTextSizeA(22.f, 1000.f, 1000.f, "GTA: Orange").x - 5;
	float y1 = 0;

	/*ImGui::GetWindowDrawList()->AddText(CGlobals::Get().tagFont, 22.f, ImVec2(x1 - 1, y1 - 1), ImColor(0, 0, 0, 100), "GTA: Orange");
	ImGui::GetWindowDrawList()->AddText(CGlobals::Get().tagFont, 22.f, ImVec2(x1 + 1, y1 + 1), ImColor(0, 0, 0, 100), "GTA: Orange");
	ImGui::GetWindowDrawList()->AddText(CGlobals::Get().tagFont, 22.f, ImVec2(x1 + 1, y1 - 1), ImColor(0, 0, 0, 100), "GTA: Orange");
	ImGui::GetWindowDrawList()->AddText(CGlobals::Get().tagFont, 22.f, ImVec2(x1 - 1, y1 + 1), ImColor(0, 0, 0, 100), "GTA: Orange");
	ImGui::GetWindowDrawList()->AddText(CGlobals::Get().tagFont, 22.f, ImVec2(x1, y1), ImColor(0xFF, 0x8F, 0x00, 150), "GTA: Orange");*/

	if (CGlobals::Get().m_pTextureView != nullptr)
	{
		if (CGlobals::Get().dirtybuffer)
		{
			// Map resource
			CGlobals::Get().cefmutex.lock();

			D3D11_MAPPED_SUBRESOURCE mapped;

			if (SUCCEEDED(CGlobals::Get().d3dDeviceContext->Map(CGlobals::Get().m_pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
			{
				// Set subresource data and copy texture row by row
				auto width2 = static_cast<unsigned int>(CGlobals::Get().cefsize.fX * 4);
				auto dstData = static_cast<byte*>(mapped.pData);
				auto srcData = static_cast<const byte*>(CGlobals::Get().cefbuffer);

				for (unsigned int y = 0; y < CGlobals::Get().cefsize.fY; ++y)
				{
					std::memcpy(&dstData[mapped.RowPitch * y], &srcData[width2 * y], width2); // TODO: Copy only box
				}
				// Unmap
				CGlobals::Get().d3dDeviceContext->Unmap(CGlobals::Get().m_pTexture, 0);
			}
			CGlobals::Get().dirtybuffer = false;

			CGlobals::Get().cefmutex.unlock();
		}

		ImGui::GetWindowDrawList()->AddImage(CGlobals::Get().m_pTextureView, ImVec2(0, 0), ImVec2((float)viewPortGame->Width, (float)viewPortGame->Height));
		//ImGui::SetCursorPos(ImVec2(0, 0));
		//ImGui::Image(CGlobals::Get().m_pTextureView, ImVec2((float)viewPortGame->Width, (float)viewPortGame->Height));
	}
	
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
#if _DEBUG

	if (CGlobals::Get().isDebug)
	{
		//for (int i = 0, cnt = 0; i < ReplayInterfaces::Get()->ReplayInterfacePed->pool.Capacity(); ++i)
		//{
		//	if (ReplayInterfaces::Get()->ReplayInterfacePed->pool.GetHandle(i) == -1)
		//		continue;
		//	if ((ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position - CWorld::Get()->CPedPtr->Position).Length() > 50.f)
		//		continue;
		//	cnt++;
		//	ss = std::stringstream();
		//	ss << "Handle: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool.GetHandle(i) << std::endl;
		//	CGraphics::Get()->Draw3DText(ss.str(), 0.3f, ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fX,
		//		ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fY,
		//		ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fZ + 1.2f, { 255, 255, 255, 255 });
		//	int primaryActive = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->PrimaryTasks->ActiveTask;
		//	if (primaryActive > -1)
		//	{
		//		std::stringstream ss2;
		//		GTA::CTask *primaryTask = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->PrimaryTasks->TasksArray[primaryActive];
		//		ss2 << "P: " << primaryTask->GetTree() << std::endl;
		//		CGraphics::Get()->Draw3DText(ss2.str(), 0.3f, ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fX,
		//			ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fY,
		//			ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fZ + 1.0f, { 255, 255, 255, 255 });
		//	}
		//	int secondaryActive = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->SecondaryTasks->ActiveTask;
		//	if (secondaryActive > -1)
		//	{
		//		std::stringstream ss2;
		//		GTA::CTask *secondaryTask = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->SecondaryTasks->TasksArray[secondaryActive];
		//		ss2 << "S: " << secondaryTask->GetTree() << std::endl;
		//		CGraphics::Get()->Draw3DText(ss2.str(), 0.3f, ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fX,
		//			ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fY,
		//			ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fZ + 0.5f, { 255, 255, 255, 255 });
		//	}
		//	int movementActive = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->MovementTasks->ActiveTask;
		//	if (movementActive > -1)
		//	{
		//		std::stringstream ss2;
		//		GTA::CTask *movementTask = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->MovementTasks->TasksArray[movementActive];
		//		ss2 << "M: " << movementTask->GetTree() << std::endl;
		//		CGraphics::Get()->Draw3DText(ss2.str(), 0.3f, ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fX,
		//			ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fY,
		//			ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fZ, { 255, 255, 255, 255 });
		//	}
		//	int motionActive = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->MotionTasks->ActiveTask;
		//	if (motionActive > -1)
		//	{
		//		std::stringstream ss2;
		//		GTA::CTask *motionTask = ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->TasksPtr->MotionTasks->TasksArray[motionActive];
		//		ss2 << "MN: " << motionTask->GetTree() << std::endl;
		//		CGraphics::Get()->Draw3DText(ss2.str(), 0.3f, ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fX,
		//			ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fY,
		//			ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.fZ - 0.5f, { 255, 255, 255, 255 });
		//	}
		//	/*ss << "Handle: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool.GetHandle(i) << std::endl <<
		//	"Position: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Position.ToString() << std::endl <<
		//	"Health: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->Health << std::endl <<
		//	"Model: 0x" << std::hex << ReplayInterfaces::Get()->ReplayInterfacePed->pool[i]->PedModelInfo->ModelHash << std::endl <<
		//	ss2.str();*/
		//	if (cnt >= ReplayInterfaces::Get()->ReplayInterfacePed->pool.Count())
		//		break;
		//}

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
#endif
	CNetworkPlayer::Render();
	CNetwork3DText::Render();
	ImGui::PopStyleVar(7);
	ImGui::End();
}

GUI(BackScene);