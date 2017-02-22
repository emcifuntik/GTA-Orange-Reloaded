#include "stdafx.h"

std::vector<CNetworkBlip *> CNetworkBlip::AllBlips;

CNetworkBlip::CNetworkBlip(std::string name, float x, float y, float z, float scale, int color, int sprite, int playerid):name(name), vecPos(x, y, z), scale(scale), color(color), sprite(sprite), playerid(playerid)
{
	RakNet::BitStream bsOut;

	rnGUID = RakNetGUID(createGUID());

	bsOut.Write(rnGUID);
	bsOut.Write(RakString(name.c_str()));
	bsOut.Write(x);
	bsOut.Write(y);
	bsOut.Write(z);
	bsOut.Write(scale);
	bsOut.Write(color);
	bsOut.Write(sprite);

	if (playerid == -1)
	{
		CRPCPlugin::Get()->Signal("CreateBlip", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	} else {
		RakNetGUID guid = CNetworkPlayer::GetByID(playerid)->GetGUID();
		CRPCPlugin::Get()->Signal("CreateBlip", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, guid, false, false);
	}
	AllBlips.push_back(this);
}

void CNetworkBlip::SetScale(float _scale)
{
	scale = _scale;
}

void CNetworkBlip::SetColor(int _color)
{
	color = _color;
}

void CNetworkBlip::SetSprite(int _sprite)
{
	sprite = _sprite;
}

void CNetworkBlip::SetRoute(bool _route)
{
	route = _route;
}

void CNetworkBlip::SetName(std::string _name)
{
	name = _name;
}

void CNetworkBlip::SetAsShortRange(bool _short)
{
	bnear = _short;
}

CNetworkBlip::~CNetworkBlip()
{
	RakNet::BitStream bsOut;

	bsOut.Write(rnGUID);

	if (playerid == -1) CRPCPlugin::Get()->Signal("DeleteBlip", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, UNASSIGNED_RAKNET_GUID, true, false);
	else CRPCPlugin::Get()->Signal("DeleteBlip", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, CNetworkPlayer::GetByID(playerid)->GetGUID(), false, false);
}

void CNetworkBlip::SendGlobal(RakNet::Packet *packet)
{
	for (auto *blip : AllBlips)
	{
		if (blip->playerid == -1)
		{
			RakNet::BitStream bsOut;

			bsOut.Write(blip->rnGUID);
			bsOut.Write(RakString(blip->name.c_str()));
			bsOut.Write(blip->vecPos.fX);
			bsOut.Write(blip->vecPos.fY);
			bsOut.Write(blip->vecPos.fZ);
			bsOut.Write(blip->scale);
			bsOut.Write(blip->color);
			bsOut.Write(blip->sprite);

			CRPCPlugin::Get()->Signal("CreateBlip", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, packet->guid, false, false);
		}
	}
}

CNetworkBlip * CNetworkBlip::GetByGUID(RakNetGUID guid)
{
	for (CNetworkBlip *blip : AllBlips)
		if (blip && blip->rnGUID == guid)
			return blip;

	return nullptr;
}

void CNetworkBlip::Delete(RakNetGUID guid)
{
	for (int i = 0; i < AllBlips.size(); i++)
	{
		CNetworkBlip *blip = AllBlips[i];
		if (blip && blip->rnGUID == guid)
		{
			delete blip;
			AllBlips[i] = NULL;
		}
	}
}

std::vector<CNetworkBlip *> CNetworkBlip::All()
{
	return AllBlips;
}

