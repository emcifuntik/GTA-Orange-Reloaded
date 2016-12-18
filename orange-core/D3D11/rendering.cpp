// =================================================================================
// Includes 
// =================================================================================
#include "stdafx.h"

void CreateRenderTarget()
{
	DXGI_SWAP_CHAIN_DESC sd;
	CGlobals::Get().d3dSwapChain->GetDesc(&sd);
	// Create the render target
	
	ID3D11Texture2D* pBackBuffer;
	D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc;
	memset(&render_target_view_desc, 0, sizeof(render_target_view_desc));
	render_target_view_desc.Format = sd.BufferDesc.Format;
	render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	CGlobals::Get().d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	CGlobals::Get().d3dDevice->CreateRenderTargetView(pBackBuffer, &render_target_view_desc, &CGlobals::Get().d3dRenderTargetView);
	CGlobals::Get().d3dDeviceContext->OMSetRenderTargets(1, &CGlobals::Get().d3dRenderTargetView, NULL);
	pBackBuffer->Release();
}

void D3DHook::Render()
{
	if (UI::IS_PAUSE_MENU_ACTIVE() || UI::_0xE18B138FABC53103())
		return;
	ImGui_ImplDX11_NewFrame();
	if(CGlobals::Get().showChat)
		CChat::Get()->Render();

	if (CGlobals::Get().displayServerBrowser)
	{
		ShowCursor(TRUE);
		CConfig::Get();
		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiSetCond_Always);
		ImGui::SetNextWindowPosCenter(ImGuiSetCond_Always);
		ImGui::PushFont(CGlobals::Get().chatFont);
		ImGui::Begin("Server browser", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		ImGui::Text("Nickname");
		ImGui::InputText("  ", CGlobals::Get().nickName, 32);
		ImGui::Text("Direct connect");
		ImGui::InputText(":", CGlobals::Get().serverIP, 32);
		ImGui::SameLine();
		ImGui::InputInt("Port", &CGlobals::Get().serverPort, 1, 100);
		if (ImGui::Button("Connect"))
		{
			CNetworkPlayer::Clear();
			CNetworkVehicle::Clear();
			std::stringstream ss;
			ss << "Connecting to " << CGlobals::Get().serverIP << ":" << CGlobals::Get().serverPort;
			CChat::Get()->AddChatMessage(ss.str());
			if (!CNetworkConnection::Get()->Connect(CGlobals::Get().serverIP, CGlobals::Get().serverPort))
				CChat::Get()->AddChatMessage("Can't connect to the server", { 255, 0, 0, 255 });
			CGlobals::Get().displayServerBrowser ^= 1;
			CGlobals::Get().showChat = true;
			CConfig::Get()->sNickName = std::string(CGlobals::Get().nickName);
			CConfig::Get()->sIP = std::string(CGlobals::Get().serverIP);
			CConfig::Get()->uiPort = CGlobals::Get().serverPort;
			CConfig::Get()->Save();
			ShowCursor(FALSE);
		}
		ImGui::End();
		ImGui::PopFont();
	}

	auto viewPortGame = GTA::CViewportGame::Get();
	ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiSetCond_Always);
	ImGui::Begin("Background", 0, ImVec2((float)viewPortGame->Width, (float)viewPortGame->Height), 0.f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
	if (CGlobals::Get().currentGameState == GameStatePlaying)
	{
		std::stringstream ss;
		ss << "Peds pool: " << ReplayInterfaces::Get()->ReplayInterfacePed->pool.Count() << " / " << ReplayInterfaces::Get()->ReplayInterfacePed->pool.Capacity() << std::endl <<
			"Vehicles pool: " << ReplayInterfaces::Get()->ReplayInterfaceVeh->pool.Count() << " / " << ReplayInterfaces::Get()->ReplayInterfaceVeh->pool.Capacity() << std::endl <<
			"Objects pool: " << ReplayInterfaces::Get()->ReplayInterfaceObject->pool.Count() << " / " << ReplayInterfaces::Get()->ReplayInterfaceObject->pool.Capacity() << std::endl <<
			"Ped pos: " << CLocalPlayer::Get()->GetPosition().ToString() << std::endl <<
			"FPS: " << ImGui::GetIO().Framerate;
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, 14.f, ImVec2(0.23f * viewPortGame->Width, 0.85f * viewPortGame->Height),
			ImColor(0x21, 0x96, 0xF3, 0xFF), ss.str().c_str());
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
	ImGui::End();
	ImGui::Render();
}