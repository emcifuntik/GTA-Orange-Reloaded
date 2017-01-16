#include "stdafx.h"

std::vector<CNetwork3DText *> CNetwork3DText::TextPool;


CNetwork3DText::~CNetwork3DText()
{
}


CNetwork3DText::CNetwork3DText(RakNetGUID guid, float x, float y, float z, int color, int outColor, std::string text, TextAttachedTo attachmentType, RakNet::RakNetGUID attachedTo)
{
	m_GUID = guid;
	this->vecPos = CVector3(x, y, z);
	this->color = color;
	this->outColor = outColor;
	this->text = text;
	this->attachmentType = attachmentType;
	this->attachedTo = attachedTo;
	TextPool.push_back(this);
}

void CNetwork3DText::SetColor(int color)
{
	this->color = color;
}

void CNetwork3DText::SetOutColor(int outColor)
{
	this->outColor = outColor;
}


void CNetwork3DText::AttachToPlayer(RakNet::RakNetGUID GUID, float oX, float oY, float oZ)
{
	this->attachmentType = TextAttachedTo::PLAYER_ATTACHED;
	this->attachedTo = GUID;
	this->vecOffset = CVector3(oX, oY, oZ);
}

void CNetwork3DText::AttachToVehicle(RakNet::RakNetGUID GUID, float oX, float oY, float oZ)
{
	this->attachmentType = TextAttachedTo::VEHICLE_ATTACHED;
	this->attachedTo = GUID;
	this->vecOffset = CVector3(oX, oY, oZ);
}

void CNetwork3DText::SetText(std::string text)
{
	this->text = text;
}

void CNetwork3DText::SetFontSize(float size)
{
	rawFontSize = size;
}

void CNetwork3DText::Render()
{
	for each (CNetwork3DText *text in TextPool)
	{
		if (!text->IsVisible)
			continue;
		color_t outline;
		Utils::HexToRGBA(text->outColor, outline.red, outline.green, outline.blue, outline.alpha);
		color_t color;
		Utils::HexToRGBA(text->color, color.red, color.green, color.blue, color.alpha);
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, text->fontSize, ImVec2(text->scrPos.fX - 1, text->scrPos.fY - 1), ImColor(outline.red, outline.green, outline.blue, outline.alpha), text->text.c_str());
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, text->fontSize, ImVec2(text->scrPos.fX + 1, text->scrPos.fY + 1), ImColor(outline.red, outline.green, outline.blue, outline.alpha), text->text.c_str());
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, text->fontSize, ImVec2(text->scrPos.fX + 1, text->scrPos.fY - 1), ImColor(outline.red, outline.green, outline.blue, outline.alpha), text->text.c_str());
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, text->fontSize, ImVec2(text->scrPos.fX - 1, text->scrPos.fY + 1), ImColor(outline.red, outline.green, outline.blue, outline.alpha), text->text.c_str());
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().chatFont, text->fontSize, ImVec2(text->scrPos.fX, text->scrPos.fY), ImColor(color.red, color.green, color.blue, color.alpha), text->text.c_str());
	}
}

void CNetwork3DText::PreRender()
{
	CVector3 curPedPos = CLocalPlayer::Get()->GetPosition();
	for each (CNetwork3DText *text in TextPool)
	{
		switch (text->attachmentType)
		{
			case TextAttachedTo::PLAYER_ATTACHED:
			{
				auto pl = CNetworkPlayer::GetByGUID(text->attachedTo, false);
				if (pl)
					text->vecPos = (pl->GetPosition() + text->vecOffset);
				else
				{
					text->IsVisible = false;
					continue;
				}
			}
			case TextAttachedTo::VEHICLE_ATTACHED:
			{
				auto veh = CNetworkVehicle::GetByGUID(text->attachedTo);
				if (veh)
					text->vecPos = (veh->GetPosition() + text->vecOffset);
				else
				{
					text->IsVisible = false;
					continue;
				}
			}
		}

		float distance = (curPedPos - text->vecPos).Length();
		if (distance > 100.f)
		{
			text->IsVisible = false;
			continue;
		}
		text->IsVisible = true;

		float k = 1.3f - distance / 100;
		text->fontSize = text->rawFontSize * k;
		ImVec2 textSize = CGlobals::Get().chatFont->CalcTextSizeA(text->fontSize, 1000.f, 1000.f, text->text.c_str());
		CVector3 screenPos;
		CGraphics::Get()->WorldToScreen(CVector3(text->vecPos.fX, text->vecPos.fY, text->vecPos.fZ), screenPos);
		auto viewPortGame = GTA::CViewportGame::Get();
		text->scrPos.fX = (screenPos.fX * viewPortGame->Width) - textSize.x / 2;
		text->scrPos.fY = (screenPos.fY * viewPortGame->Height) - textSize.y / 2;
	}
}

void CNetwork3DText::DeleteByGUID(RakNet::RakNetGUID guid)
{
	for (int i = 0; i < TextPool.size(); ++i)
	{
		if (TextPool[i]->m_GUID == guid)
		{
			delete TextPool[i];
			TextPool.erase(TextPool.begin() + i, TextPool.begin() + i + 1);
		}
	}
}

CNetwork3DText * CNetwork3DText::GetByGUID(RakNet::RakNetGUID GUID)
{
	for each (CNetwork3DText *_Text in TextPool)
	{
		if (_Text->m_GUID == GUID)
			return _Text;
	}
	return nullptr;
}
