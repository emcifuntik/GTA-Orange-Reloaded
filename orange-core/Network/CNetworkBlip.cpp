#include "stdafx.h"

std::vector<CNetworkBlip *> CNetworkBlip::BlipPool;

CNetworkBlip::CNetworkBlip(RakNetGUID guid, float x, float y, float z, float scale, int color, int sprite)
{
	m_GUID = guid;
	this->vecPos = CVector3(x, y, z);
	this->scale = scale;
	this->color = color;
	this->sprite = sprite;

	Handle = UI::ADD_BLIP_FOR_COORD(x, y, z);

	SetScale(scale);
	SetSprite(sprite);
	SetColor(color);
	SetAsShortRange(false);

	BlipPool.push_back(this);
}

void CNetworkBlip::SetScale(float scale)
{
	UI::SET_BLIP_SCALE(Handle, scale);
}

void CNetworkBlip::SetColor(int color)
{
	UI::SET_BLIP_COLOUR(Handle, color);
}

void CNetworkBlip::SetAsShortRange(bool _short)
{
	UI::SET_BLIP_AS_SHORT_RANGE(Handle, _short);
}

void CNetworkBlip::SetSprite(int sprite)
{
	UI::SET_BLIP_SPRITE(Handle, sprite);
}

void CNetworkBlip::SetRoute(bool route)
{
	UI::SET_BLIP_ROUTE(Handle, route);
}

void CNetworkBlip::AttachToPlayer(RakNet::RakNetGUID GUID)
{
	if (CNetworkPlayer::Exists(GUID))
	{
		auto pl = CNetworkPlayer::GetByGUID(GUID);
		if (pl)
		{
			UI::REMOVE_BLIP(&Handle);
			Handle = pl->AddBlip();
			SetScale(scale);
			SetColor(color);
			SetSprite(sprite);
			SetAsShortRange(false);
		}
	}
}

CNetworkBlip::~CNetworkBlip()
{
	UI::REMOVE_BLIP(&Handle);
}

std::vector<CNetworkBlip*> CNetworkBlip::All()
{
	return BlipPool;
}

CNetworkBlip * CNetworkBlip::GetByGUID(RakNet::RakNetGUID GUID)
{
	for each (CNetworkBlip *_Blip in BlipPool)
	{
		if (_Blip->m_GUID == GUID)
			return _Blip;
	}
	return nullptr;
}
