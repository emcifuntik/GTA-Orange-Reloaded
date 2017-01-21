#include "stdafx.h"

std::vector<CNetwork3DText *> CNetwork3DText::AllTexts;

CNetwork3DText::CNetwork3DText(float x, float y, float z, int color, int outColor, std::string text, int playerid, float oX, float oY, float oZ, float fontSize)
{
	RakNet::BitStream bsOut;

	rnGUID = RakNetGUID(createGUID());

	bsOut.Write(rnGUID);
	bsOut.Write(x);
	bsOut.Write(y);
	bsOut.Write(z);
	bsOut.Write(color);
	bsOut.Write(outColor);
	bsOut.Write(fontSize);
	RakString content = RakString(text.c_str());
	bsOut.Write(content);
	bsOut.Write(attachmentType);
	bsOut.Write(attachedTo);
	bsOut.Write(oX);
	bsOut.Write(oY);
	bsOut.Write(oZ);

	vecPos = CVector3(x, y, z);
	vecOffset = CVector3(oX, oY, oZ);
	this->color = color;
	this->outColor = outColor;
	this->text = text;
	this->playerid = playerid;
	this->fontSize = fontSize;

	if (playerid == -1)
	{
		CRPCPlugin::Get()->Signal("Create3DText", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	}
	else {
		RakNetGUID guid = CNetworkPlayer::GetByID(playerid)->GetGUID();
		CRPCPlugin::Get()->Signal("Create3DText", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, guid, false, false);
	}
	AllTexts.push_back(this);
}

void CNetwork3DText::AttachToVehicle(CNetworkVehicle & vehicle, float oX, float oY, float oZ)
{
	attachmentType = TextAttachedTo::VEHICLE_ATTACHED;
	attachedTo = vehicle.GetGUID();
	vecOffset = CVector3(oX, oY, oZ);

	RakNet::BitStream bsOut;
	bsOut.Write(rnGUID);
	bsOut.Write(attachedTo);
	bsOut.Write(oX);
	bsOut.Write(oY);
	bsOut.Write(oZ);
	if (playerid == -1)
	{
		CRPCPlugin::Get()->Signal("Attach3DTextToVehicle", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	}
	else {
		RakNetGUID guid = CNetworkPlayer::GetByID(playerid)->GetGUID();
		CRPCPlugin::Get()->Signal("Attach3DTextToVehicle", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, guid, false, false);
	}
}

void CNetwork3DText::AttachToPlayer(CNetworkPlayer & player, float oX, float oY, float oZ)
{
	this->attachmentType = TextAttachedTo::PLAYER_ATTACHED;
	this->attachedTo = player.GetGUID();
	vecOffset = CVector3(oX, oY, oZ);

	RakNet::BitStream bsOut;
	bsOut.Write(rnGUID);
	bsOut.Write(attachedTo);
	bsOut.Write(oX);
	bsOut.Write(oY);
	bsOut.Write(oZ);
	if (playerid == -1)
	{
		CRPCPlugin::Get()->Signal("Attach3DTextToPlayer", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	}
	else {
		RakNetGUID guid = CNetworkPlayer::GetByID(playerid)->GetGUID();
		CRPCPlugin::Get()->Signal("Attach3DTextToPlayer", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, guid, false, false);
	}
}

void CNetwork3DText::SetText(std::string text)
{
	this->text = text;

	RakNet::BitStream bsOut;
	bsOut.Write(rnGUID);
	RakString content = RakString(text.c_str());
	bsOut.Write(content);
	if (playerid == -1)
	{
		CRPCPlugin::Get()->Signal("Change3DTextContent", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	}
	else {
		RakNetGUID guid = CNetworkPlayer::GetByID(playerid)->GetGUID();
		CRPCPlugin::Get()->Signal("Change3DTextContent", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, guid, false, false);
	}
}

void CNetwork3DText::Delete()
{
}


CNetwork3DText::~CNetwork3DText()
{
	RakNet::BitStream bsOut;

	bsOut.Write(rnGUID);

	if (playerid == -1) CRPCPlugin::Get()->Signal("Delete3DText", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, UNASSIGNED_RAKNET_GUID, true, false);
	else CRPCPlugin::Get()->Signal("Delete3DText", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, CNetworkPlayer::GetByID(playerid)->GetGUID(), false, false);
}

void CNetwork3DText::SendGlobal(RakNet::Packet *packet)
{
	for (auto *text : AllTexts)
	{
		if (text->playerid != -1)
			continue;
		RakNet::BitStream bsOut;

		bsOut.Write(text->rnGUID);
		bsOut.Write(text->vecPos.fX);
		bsOut.Write(text->vecPos.fY);
		bsOut.Write(text->vecPos.fZ);
		bsOut.Write(text->color);
		bsOut.Write(text->outColor);
		bsOut.Write(text->fontSize);
		RakString content = RakString(text->text.c_str());
		bsOut.Write(content);
		bsOut.Write(text->attachmentType);
		bsOut.Write(text->attachedTo);

		CRPCPlugin::Get()->Signal("Create3DText", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, packet->guid, false, false);
	}
}

CNetwork3DText * CNetwork3DText::GetByGUID(RakNetGUID guid)
{
	for (CNetwork3DText *text : AllTexts)
		if (text && text->rnGUID == guid)
			return text;
	return nullptr;
}

