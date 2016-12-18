#include "stdafx.h"

API * API::instance = nullptr;

bool API::SetPlayerPosition(long playerid, float x, float y, float z)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	player->SetPosition(CVector3(x, y, z));
	return true;
}

CVector3 API::GetPlayerPosition(long playerid)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return CVector3(0.f, 0.f, 0.f);
	CVector3 vecPos;
	player->GetPosition(vecPos);
	return vecPos;
}

bool API::IsPlayerInRange(long playerid, float x, float y, float z, float range)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	CVector3 vecPos, vecNextPos(x, y, z);
	player->GetPosition(vecPos);
	if ((vecNextPos - vecPos).Length() <= range)
		return true;
	return false;
}

bool API::SetPlayerHeading(long playerid, float angle)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	player->SetHeading(angle);
	return true;
}

bool API::GivePlayerWeapon(long playerid, long weapon, long ammo)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	player->GiveWeapon(weapon, ammo);
	return true;
}

bool API::GivePlayerAmmo(long playerid, long weapon, long ammo)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	player->GiveAmmo(weapon, ammo);
	return true;
}

bool API::GivePlayerMoney(long playerid, long money)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	player->GiveMoney(money);
	return true;
}

bool API::SetPlayerMoney(long playerid, long money)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	player->SetMoney(money);
	return true;
}

bool API::ResetPlayerMoney(long playerid)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	player->SetMoney(0);
	return true;
}

size_t API::GetPlayerMoney(long playerid)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return 0L;
	size_t money = player->GetMoney();
	return money;
}

bool API::SetPlayerModel(long playerid, long model)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	player->SetModel(model);
	return true;
}

long API::GetPlayerModel(long playerid)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	return player->GetModel();
}

bool API::SetPlayerName(long playerid, const char * name)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	player->SetName(name);
	return true;
}

std::string API::GetPlayerName(long playerid)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return nullptr;
	return player->GetName();
}

bool API::SetPlayerHealth(long playerid, float health)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	player->SetHealth(health);
	return true;
}

float API::GetPlayerHealth(long playerid)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return 0.f;
	return player->GetHealth();
}

bool API::SetPlayerArmour(long playerid, float armour)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	player->SetArmour(armour);
	return true;
}

float API::GetPlayerArmour(long playerid)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return 0.f;
	return player->GetArmour();
}

bool API::SetPlayerColor(long playerid, unsigned int color)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	player->SetColor(color);
	return true;
}

unsigned int API::GetPlayerColor(long playerid)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	color_t playerColor = player->GetColor();
	return ((playerColor.red & 0xff) << 24) + ((playerColor.green & 0xff) << 16) + ((playerColor.blue & 0xff) << 8) + (playerColor.alpha & 0xff);
}



void API::BroadcastClientMessage(const char * message, unsigned int color)
{
	RakNet::BitStream bsOut;
	RakNet::RakString msg(message);
	bsOut.Write(msg);
	color_t col;
	col.red = (BYTE)((color >> 24) & 0xFF);  // Extract the RR byte
	col.green = (BYTE)((color >> 16) & 0xFF);   // Extract the GG byte
	col.blue = (BYTE)((color >> 8) & 0xFF);   // Extract the GG byte
	col.alpha = (BYTE)((color) & 0xFF);        // Extract the BB byte
	bsOut.Write(col);
	CRPCPlugin::Get()->Signal("SendClientMessage", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
}

bool API::SendClientMessage(long playerid, const char * message, unsigned int color)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	RakNet::BitStream bsOut;
	RakNet::RakString msg(message);
	bsOut.Write(msg);
	color_t col;
	col.red = (BYTE)((color >> 24) & 0xFF);  // Extract the RR byte
	col.green = (BYTE)((color >> 16) & 0xFF);   // Extract the GG byte
	col.blue = (BYTE)((color >> 8) & 0xFF);   // Extract the GG byte
	col.alpha = (BYTE)((color) & 0xFF);        // Extract the BB byte
	bsOut.Write(col);
	CRPCPlugin::Get()->Signal("SendClientMessage", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, player->GetGUID(), true, false);
	return true;
}

int API::CreateVehicle(long hash, float x, float y, float z, float heading)
{
	CNetworkVehicle *veh = new CNetworkVehicle(hash, x, y, z, heading);
	return RakNetGUID::ToUint32(veh->GetGUID()); // (new CNetworkVehicle(hash, x, y, z, heading));
}

bool API::SetVehiclePosition(int vehicleid, float x, float y, float z)
{
	log << "Not implemented" << std::endl;
	return true;
}

CVector3 API::GetVehiclePosition(int vehicleid)
{
	log << "Not implemented" << std::endl;
	return CVector3(0, 0, 0);
}

bool API::CreatePickup(int type, float x, float y, float z, float scale)
{
	log << "Not implemented" << std::endl;
	return true;
}

unsigned long API::CreateBlipForAll(float x, float y, float z, float scale, int color, int sprite)
{
	CNetworkBlip * blip = new CNetworkBlip(x, y, z, scale, color, sprite, -1);
	return RakNetGUID::ToUint32(blip->rnGUID);
}

unsigned long API::CreateBlipForPlayer(long playerid, float x, float y, float z, float scale, int color, int sprite)
{
	CNetworkBlip * blip = new CNetworkBlip(x, y, z, scale, color, sprite, playerid);
	return RakNetGUID::ToUint32(blip->rnGUID);
}

void API::DeleteBlip(unsigned long guid)
{
	CNetworkBlip::GetByGUID(RakNetGUID(guid))->~CNetworkBlip();
}

void API::SetBlipColor(unsigned long _guid, int color)
{
	RakNet::BitStream bsOut;
	RakNetGUID guid = RakNetGUID(_guid);
	CNetworkBlip *blip = CNetworkBlip::GetByGUID(guid);
	//blip->SetColor(color);

	bsOut.Write(guid);
	bsOut.Write(color);
	CRPCPlugin::Get()->Signal("SetBlipColor", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
}

void API::SetBlipScale(unsigned long _guid, float scale)
{
	RakNet::BitStream bsOut;
	RakNetGUID guid = RakNetGUID(_guid);
	CNetworkBlip *blip = CNetworkBlip::GetByGUID(guid);
	blip->SetScale(scale);

	bsOut.Write(guid);
	bsOut.Write(scale);
	CRPCPlugin::Get()->Signal("SetBlipScale", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
}

void API::SetBlipRoute(unsigned long _guid, bool route)
{
	RakNet::BitStream bsOut;
	RakNetGUID guid = RakNetGUID(_guid);
	CNetworkBlip *blip = CNetworkBlip::GetByGUID(guid);
	//blip->SetRoute(color);

	bsOut.Write(guid);
	bsOut.Write(route);
	CRPCPlugin::Get()->Signal("SetBlipRoute", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
}

unsigned long API::CreateMarkerForAll(float x, float y, float z, float height, float radius)
{
	CNetworkMarker * marker = new CNetworkMarker(x, y, z, height, radius, -1);
	return RakNetGUID::ToUint32(marker->rnGUID);
}

unsigned long API::CreateMarkerForPlayer(long playerid, float x, float y, float z, float height, float radius)
{
	CNetworkMarker * marker = new CNetworkMarker(x, y, z, height, radius, playerid);
	return RakNetGUID::ToUint32(marker->rnGUID);
}

void API::DeleteMarker(unsigned long guid)
{
	CNetworkMarker::GetByGUID(RakNetGUID(guid))->~CNetworkMarker();
}

bool API::SetInfoMsg(long playerid, const char* msg)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;

	RakNet::BitStream bsOut;
	bsOut.Write(true);
	bsOut.Write(RakNet::RakString(msg));

	CRPCPlugin::Get()->Signal("SetInfoMsg", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, player->GetGUID(), false, false);
	return true;
}

bool API::UnsetInfoMsg(long playerid)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;

	RakNet::BitStream bsOut;
	bsOut.Write(false);

	CRPCPlugin::Get()->Signal("SetInfoMsg", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, player->GetGUID(), false, false);
	return true;
}

void API::Print(const char * message)
{
	log << message << std::endl;
}

bool API::Print3DText(const char* text, float x, float y, float z, float scale)
{
	log << "Not implemented" << std::endl;
	return true;
}

long API::Hash(const char * str)
{
	unsigned int value = 0, temp = 0;
	for (size_t i = 0; i<strlen(str); i++)
	{
		temp = tolower(str[i]) + value;
		value = temp << 10;
		temp += value;
		value = temp >> 6;
		value ^= temp;
	}
	temp = value << 3;
	temp += value;
	unsigned int temp2 = temp >> 11;
	temp = temp2 ^ temp;
	temp2 = temp << 15;
	value = temp2 + temp;
	if (value < 2) value += 2;
	return value;
}
