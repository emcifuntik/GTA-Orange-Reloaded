#include "stdafx.h"

std::vector<CNetworkBlip *> CNetworkBlip::BlipPool;

CNetworkBlip::CNetworkBlip(RakNetGUID guid, std::string name, float x, float y, float z, float scale, int color, int sprite):name(name), vecPos(x, y, z), scale(scale), color(color), sprite(sprite)
{
	m_GUID = guid;

	Handle = UI::ADD_BLIP_FOR_COORD(x, y, z);

	SetSprite(sprite);
	SetAsShortRange(true);

	BlipPool.push_back(this);
}

void CNetworkBlip::SetScale(float scale)
{
	this->scale = scale;
	UI::SET_BLIP_SCALE(Handle, scale);
}

void CNetworkBlip::SetColor(int color)
{
	this->color = color;
	UI::SET_BLIP_COLOUR(Handle, color);
}

void CNetworkBlip::SetAsShortRange(bool _short)
{
	UI::SET_BLIP_AS_SHORT_RANGE(Handle, _short);
}

void CNetworkBlip::SetSprite(int sprite)
{
	this->sprite = sprite;
	UI::SET_BLIP_SPRITE(Handle, sprite);

	SetScale(scale);
	SetColor(color);
	SetName(name);
}

void CNetworkBlip::SetRoute(bool route)
{
	UI::SET_BLIP_ROUTE(Handle, route);
}

void CNetworkBlip::SetName(std::string name)
{
	this->name = name;
	UI::BEGIN_TEXT_COMMAND_SET_BLIP_NAME("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(name.c_str());
	UI::END_TEXT_COMMAND_SET_BLIP_NAME(Handle);
}

void CNetworkBlip::AttachToPlayer(RakNet::RakNetGUID GUID)
{
	UI::REMOVE_BLIP(&Handle);
	auto pl = CNetworkPlayer::GetByGUID(GUID, false);
	if (pl)
	{
		Handle = pl->AddBlip();
		SetSprite(sprite);
	}
}

void CNetworkBlip::AttachToVehicle(RakNet::RakNetGUID GUID)
{
	UI::REMOVE_BLIP(&Handle);
	auto veh = CNetworkVehicle::GetByGUID(GUID);
	if (veh)
	{
		veh->AttachBlip(this);
		SetSprite(sprite);
	}
}

void CNetworkBlip::Update()
{
	SetSprite(sprite);
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
