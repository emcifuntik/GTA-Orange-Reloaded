#include "stdafx.h"

std::vector<CNetworkBlip *> CNetworkBlip::AllBlips;

CNetworkBlip::CNetworkBlip(float x, float y, float z, float scale, int color, int sprite, int playerid):vecPos(x, y, z), scale(scale), color(color), sprite(sprite), playerid(playerid)
{
	RakNet::BitStream bsOut;

	rnGUID = RakNetGUID(createGUID());

	bsOut.Write(rnGUID);
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

void CNetworkBlip::Delete()
{
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

std::vector<CNetworkBlip *> CNetworkBlip::All()
{
	return AllBlips;
}

