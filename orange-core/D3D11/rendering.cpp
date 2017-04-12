// =================================================================================
// Includes 
// =================================================================================
#include "stdafx.h"

bool ToRender = false;

void D3DHook::SetRender(bool state)
{
	ToRender = state;
}

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
	if (!ToRender) return;
	ImGui_ImplDX11_NewFrame();
	CGuiDispatcher::Get()();
	ImGui::Render();
}