#include "stdafx.h"

API * API::instance = nullptr;

void API::LoadClientScript(std::string name, char* buffer, size_t size)
{
	CClientScripting::AddScript(name, buffer, size);
}

void API::ClientEvent(const char* name, std::vector<MValue> args, long playerid)
{
	BitStream bsOut;

	RakString rname(name);
	bsOut.Write(rname);

	int size = args.size();
	bsOut.Write(size);

	for (auto arg : args) {
		switch (arg.type)
		{
		case M_BOOL:
		{
			bsOut.Write((char)0);
			bsOut.Write(arg.getBool());
			break;
		}
		case M_DOUBLE:
		{
			bsOut.Write((char)1);
			bsOut.Write(arg.getDouble());
			break;
		}
		case M_STRING:
		{
			RakString str(arg.getString());
			bsOut.Write((char)2);
			bsOut.Write(str);
			break;
		}
		case M_ARRAY:
		{
			bsOut.Write((char)3);
			MArray arr = arg.getArray();
			bsOut.Write((unsigned int)(arr.ikeys.size() + arr.skeys.size()));
			for (auto arg1 : arr.ikeys)
			{
				log << arg1.first << std::endl;
				switch (arg1.second->type)
				{
				case M_BOOL:
				{
					bsOut.Write((char)(0 | 16));
					bsOut.Write(arg1.first);
					bsOut.Write(arg1.second->getBool());
					break;
				}
				case M_DOUBLE:
				{
					bsOut.Write((char)(1 | 16));
					bsOut.Write(arg1.first);
					bsOut.Write(arg1.second->getDouble());
					break;
				}
				case M_STRING:
				{
					std::string str(arg1.second->getString());
					bsOut.Write((char)(2 | 16));
					bsOut.Write(arg1.first);
					bsOut.Write(str);
					log << str << std::endl;
					break;
				}
				}
			}
			for (auto arg2 : arr.skeys)
			{
				switch (arg2.second->type)
				{
				case M_BOOL:
				{
					bsOut.Write((char)(0));
					bsOut.Write(arg2.first);
					bsOut.Write(arg2.second->getBool());
					break;
				}
				case M_DOUBLE:
				{
					bsOut.Write((char)(1));
					bsOut.Write(arg2.first);
					bsOut.Write(arg2.second->getDouble());
					break;
				}
				case M_STRING:
				{
					std::string str(arg2.second->getString());
					bsOut.Write((char)(2));
					bsOut.Write(arg2.first);
					bsOut.Write(str);
					log << str << std::endl;
					break;
				}
				}
			}
			break;
		}
		default:
			log << "You can only pass bools, numbers, strings and tables" << std::endl;
			break;
		}
	}
	if (playerid == -1)
	{
		CRPCPlugin::Get()->Signal("ClientEvent", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	}
	else
	{
		CNetworkPlayer *pl = CNetworkPlayer::GetByID(playerid);
		if(pl) CRPCPlugin::Get()->Signal("ClientEvent", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, pl->GetGUID(), false, false);
	}
}

void API::KickPlayer(long playerid)
{
	CNetworkConnection::Get()->server->CloseConnection(CNetworkPlayer::GetByID(playerid)->GetGUID(), true, 0);
}

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

float API::GetPlayerHeading(long playerid)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return 0.f;
	return player->GetHeading();
}

bool API::RemovePlayerWeapons(long playerid)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;
	player->RemoveAllWeapons();
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
		return "wtf?";
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
	CRPCPlugin::Get()->Signal("SendClientMessage", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, player->GetGUID(), false, false);
	return true;
}

bool API::SetPlayerIntoVehicle(long playerid, unsigned long vehicle, char seat)
{
	RakNet::BitStream bsOut;
	bsOut.Write(CNetworkPlayer::GetByID(playerid)->GetGUID());
	bsOut.Write(RakNetGUID(vehicle));
	bsOut.Write(seat);
	CRPCPlugin::Get()->Signal("SetPlayerIntoVehicle", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	return true;
}

void API::DisablePlayerHud(long playerid, bool toggle)
{
	BitStream bsOut;
	bsOut.Write(toggle);
	CRPCPlugin::Get()->Signal("DisableHud", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, CNetworkPlayer::GetByID(playerid)->GetGUID(), false, false);
}

unsigned long API::GetPlayerGUID(long playerid)
{
	auto pl = CNetworkPlayer::GetByID(playerid);
	if (pl) return RakNetGUID::ToUint32(pl->GetGUID());
	return 0;
}

unsigned long API::CreateVehicle(long hash, float x, float y, float z, float heading)
{
	CNetworkVehicle *veh = new CNetworkVehicle(hash, x, y, z, heading);
	return RakNetGUID::ToUint32(veh->GetGUID()); // (new CNetworkVehicle(hash, x, y, z, heading));
}

bool API::DeleteVehicle(unsigned long guid)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		delete veh;
	}
	BitStream bsOut;
	bsOut.Write(_guid);
	CRPCPlugin::Get()->Signal("DeleteVehicle", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	return true;
}

bool API::SetVehiclePosition(unsigned long guid, float x, float y, float z)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		CVector3 vecPos = CVector3(x, y, z);
		veh->SetPosition(vecPos);
		BitStream bsOut;
		bsOut.Write(_guid);
		bsOut.Write(vecPos);
		CRPCPlugin::Get()->Signal("SetVehiclePosition", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
		return true;
	}
	return false;
}

CVector3 API::GetVehiclePosition(unsigned long guid)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		return veh->GetPosition();
	}
	return CVector3(0, 0, 0);
}

bool API::SetVehicleRotation(unsigned long guid, float rx, float ry, float rz)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		CVector3 vecRot = CVector3(rx, ry, rz);
		veh->SetRotation(vecRot);
		BitStream bsOut;
		bsOut.Write(_guid);
		bsOut.Write(vecRot);
		CRPCPlugin::Get()->Signal("SetVehicleRotation", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
		return true;
	}
	return false;
}

CVector3 API::GetVehicleRotation(unsigned long guid)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		return veh->GetRotation();
	}
	return CVector3(0, 0, 0);
}

bool API::SetVehicleColours(unsigned long guid, int Color1, int Color2)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		veh->Color1 = Color1;
		veh->Color2 = Color2;
		BitStream bsOut;
		bsOut.Write(_guid);
		bsOut.Write(Color1);
		bsOut.Write(Color2);
		CRPCPlugin::Get()->Signal("SetVehicleColours", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
		return true;
	}
	return false;
}

bool API::GetVehicleColours(unsigned long guid, int *Color1, int *Color2)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		*Color1 = veh->Color1;
		*Color2 = veh->Color2;
		return true;
	}
	return false;
}

bool API::SetVehicleTyresBulletproof(unsigned long guid, bool bulletproof)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		veh->bTyresBulletproof = bulletproof;
		BitStream bsOut;
		bsOut.Write(_guid);
		bsOut.Write(bulletproof);
		CRPCPlugin::Get()->Signal("SetVehicleTyresBulletproof", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
		return true;
	}
	return false;
}

bool API::GetVehicleTyresBulletproof(unsigned long guid)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		return veh->bTyresBulletproof;
	}
	return false;
}

bool API::SetVehicleCustomPrimaryColor(unsigned long guid, int rColor, int gColor, int bColor)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		log << "SetVehicleCustomPrimaryColor not implemented" << std::endl;
		return true;
	}
	return false;
}

bool API::GetVehicleCustomPrimaryColor(unsigned long guid, int *rColor, int *gColor, int *bColor)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		log << "GetVehicleCustomPrimaryColor not implemented" << std::endl;
		return true;
	}
	return false;
}

bool API::SetVehicleCustomSecondaryColor(unsigned long guid, int rColor, int gColor, int bColor)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		log << "SetVehicleCustomSecondaryColor not implemented" << std::endl;
		return true;
	}
	return false;
}

bool API::GetVehicleCustomSecondaryColor(unsigned long guid, int *rColor, int *gColor, int *bColor)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		log << "GetVehicleCustomSecondaryColor not implemented" << std::endl;
		return true;
	}
	return false;
}

bool API::SetVehicleEngineStatus(unsigned long guid, bool status, bool locked)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		veh->bEngineStatus = status;
		veh->bEngineLocked = locked;
		BitStream bsOut;
		bsOut.Write(_guid);
		bsOut.Write(status);
		bsOut.Write(locked);
		CRPCPlugin::Get()->Signal("SetVehicleEngineStatus", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
		return true;
	}
	return false;
}

bool API::GetVehicleEngineStatus(unsigned long guid)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		return veh->bEngineStatus;
	}
	return false;
}

bool API::SetVehicleLocked(unsigned long guid, bool locked)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		if (veh->bLocked != locked)
		{
			veh->bLocked = locked;
			BitStream bsOut;
			bsOut.Write(_guid);
			bsOut.Write(locked);
			CRPCPlugin::Get()->Signal("SetVehicleLocked", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
		}
		return true;
	}
	return false;
}

bool API::IsVehicleLocked(unsigned long guid)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		return veh->bLocked;
	}
	return false;
}

bool API::SetVehicleBodyHealth(unsigned long guid, float health)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		veh->fBodyHealth = health;
		BitStream bsOut;
		bsOut.Write(_guid);
		bsOut.Write(health);
		CRPCPlugin::Get()->Signal("SetVehicleBodyHealth", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
		return true;
	}
	return false;
}

bool API::SetVehicleEngineHealth(unsigned long guid, float health)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		veh->fEngineHealth = health;
		BitStream bsOut;
		bsOut.Write(_guid);
		bsOut.Write(health);
		CRPCPlugin::Get()->Signal("SetVehicleEngineHealth", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
		return true;
	}
	return false;
}

bool API::SetVehicleTankHealth(unsigned long guid, float health)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		veh->fTankHealth = health;
		BitStream bsOut;
		bsOut.Write(_guid);
		bsOut.Write(health);
		CRPCPlugin::Get()->Signal("SetVehicleTankHealth", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
		return true;
	}
	return false;
}

bool API::GetVehicleHealth(unsigned long guid, float *body, float *engine, float *tank)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		*body = veh->fBodyHealth;
		*engine = veh->fEngineHealth;
		*tank = veh->fTankHealth;
		return true;
	}
	return false;
}

bool API::SetVehicleNumberPlate(unsigned long guid, const char *text)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		log << "SetVehicleNumberPlate not implemented" << std::endl;
		return true;
	}
	return false;
}

std::string API::GetVehicleNumberPlate(unsigned long guid)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		log << "GetVehicleNumberPlate not implemented" << std::endl;
		return "none";
	}
	return false;
}

bool API::SetVehicleNumberPlateStyle(unsigned long guid, int style)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		log << "SetVehicleNumberPlateStyle not implemented" << std::endl;
		return true;
	}
	return false;
}

int API::GetVehicleNumberPlateStyle(unsigned long guid)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		log << "GetVehicleNumberPlateStyle not implemented" << std::endl;
		return 0;
	}
	return -1;
}

bool API::SetVehicleSirenState(unsigned long guid, bool state)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		veh->bSirenState = state;
		BitStream bsOut;
		bsOut.Write(_guid);
		bsOut.Write(state);
		CRPCPlugin::Get()->Signal("SetVehicleSirenState", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
		return true;
	}
	return false;
}

bool API::GetVehicleSirenState(unsigned long guid)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		return veh->bSirenState;
	}
	return false;
}

bool API::SetVehicleWheelColor(unsigned long guid, int color)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		log << "SetVehicleWheelColor not implemented" << std::endl;
		return true;
	}
	return false;
}

int API::GetVehicleWheelColor(unsigned long guid)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		log << "GetVehicleWheelColor not implemented" << std::endl;
		return true;
	}
	return false;
}

bool API::SetVehicleWheelType(unsigned long guid, int type)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		log << "SetVehicleWheelType not implemented" << std::endl;
		return true;
	}
	return false;
}

int API::GetVehicleWheelType(unsigned long guid)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		log << "GetVehicleWheelType not implemented" << std::endl;
		return true;
	}
	return false;
}

int API::GetVehicleDriver(unsigned long guid)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	if (veh) {
		return veh->GetDriver();
	}
	return -1;
}

std::vector<unsigned int> API::GetVehiclePassengers(unsigned long guid)
{
	RakNetGUID _guid(guid);
	auto veh = CNetworkVehicle::GetByGUID(_guid);
	std::vector<unsigned int> value;
	if (veh) {
		value = veh->GetPassengers();
	}
	return value;
}

bool API::CreatePickup(int type, float x, float y, float z, float scale)
{
	log << "Not implemented" << std::endl;
	return true;
}

unsigned long API::CreateBlipForAll(std::string name, float x, float y, float z, float scale, int color, int sprite)
{
	CNetworkBlip * blip = new CNetworkBlip(name, x, y, z, scale, color, sprite, -1);
	return RakNetGUID::ToUint32(blip->rnGUID);
}

unsigned long API::CreateBlipForPlayer(long playerid, std::string name, float x, float y, float z, float scale, int color, int sprite)
{
	CNetworkBlip * blip = new CNetworkBlip(name, x, y, z, scale, color, sprite, playerid);
	return RakNetGUID::ToUint32(blip->rnGUID);
}

void API::DeleteBlip(unsigned long guid)
{
	delete CNetworkBlip::GetByGUID(RakNetGUID(guid));
}

void API::SetBlipColor(unsigned long _guid, int color)
{
	RakNet::BitStream bsOut;
	RakNetGUID guid = RakNetGUID(_guid);
	CNetworkBlip *blip = CNetworkBlip::GetByGUID(guid);
	blip->SetColor(color);

	bsOut.Write(guid);
	bsOut.Write(color);

	if (blip->GetPlayerID() == -1) CRPCPlugin::Get()->Signal("SetBlipColor", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	else CRPCPlugin::Get()->Signal("SetBlipColor", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, CNetworkPlayer::GetByID(blip->GetPlayerID())->GetGUID(), false, false);
}

void API::SetBlipScale(unsigned long _guid, float scale)
{
	RakNet::BitStream bsOut;
	RakNetGUID guid = RakNetGUID(_guid);
	CNetworkBlip *blip = CNetworkBlip::GetByGUID(guid);
	blip->SetScale(scale);

	bsOut.Write(guid);
	bsOut.Write(scale);

	if(blip->GetPlayerID() == -1) CRPCPlugin::Get()->Signal("SetBlipScale", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	else CRPCPlugin::Get()->Signal("SetBlipScale", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, CNetworkPlayer::GetByID(blip->GetPlayerID())->GetGUID(), false, false);
}

void API::SetBlipRoute(unsigned long _guid, bool route)
{
	RakNet::BitStream bsOut;
	RakNetGUID guid = RakNetGUID(_guid);
	CNetworkBlip *blip = CNetworkBlip::GetByGUID(guid);
	blip->SetRoute(route);

	bsOut.Write(guid);
	bsOut.Write(route);

	if (blip->GetPlayerID() == -1) CRPCPlugin::Get()->Signal("SetBlipRoute", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	else CRPCPlugin::Get()->Signal("SetBlipRoute", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, CNetworkPlayer::GetByID(blip->GetPlayerID())->GetGUID(), false, false);
}

void API::SetBlipSprite(unsigned long _guid, int sprite)
{
	RakNet::BitStream bsOut;
	RakNetGUID guid = RakNetGUID(_guid);
	CNetworkBlip *blip = CNetworkBlip::GetByGUID(guid);
	blip->SetSprite(sprite);

	bsOut.Write(guid);
	bsOut.Write(sprite);

	if (blip->GetPlayerID() == -1) CRPCPlugin::Get()->Signal("SetBlipSprite", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	else CRPCPlugin::Get()->Signal("SetBlipSprite", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, CNetworkPlayer::GetByID(blip->GetPlayerID())->GetGUID(), false, false);
}

void API::SetBlipName(unsigned long _guid, std::string name)
{
	RakNet::BitStream bsOut;
	RakNetGUID guid = RakNetGUID(_guid);
	CNetworkBlip *blip = CNetworkBlip::GetByGUID(guid);
	blip->SetName(name);

	bsOut.Write(guid);
	bsOut.Write(RakString(name.c_str()));

	if (blip->GetPlayerID() == -1) CRPCPlugin::Get()->Signal("SetBlipName", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	else CRPCPlugin::Get()->Signal("SetBlipName", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, CNetworkPlayer::GetByID(blip->GetPlayerID())->GetGUID(), false, false);
}

void API::SetBlipAsShortRange(unsigned long _guid, bool _short)
{
	RakNet::BitStream bsOut;
	RakNetGUID guid = RakNetGUID(_guid);
	CNetworkBlip *blip = CNetworkBlip::GetByGUID(guid);
	blip->SetAsShortRange(_short);

	bsOut.Write(guid);
	bsOut.Write(_short);

	if (blip->GetPlayerID() == -1) CRPCPlugin::Get()->Signal("SetBlipRange", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	else CRPCPlugin::Get()->Signal("SetBlipRange", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, CNetworkPlayer::GetByID(blip->GetPlayerID())->GetGUID(), false, false);
}

void API::AttachBlipToPlayer(unsigned long _guid, long player)
{
	RakNet::BitStream bsOut;
	RakNetGUID guid = RakNetGUID(_guid);
	CNetworkBlip *blip = CNetworkBlip::GetByGUID(guid);
	auto pl = CNetworkPlayer::GetByID(player);

	if (!blip || !pl)
	{
		log << "No player" << std::endl;
		return;
	}

	bsOut.Write(guid);
	bsOut.Write(pl->GetGUID());

	if (blip->GetPlayerID() == -1) CRPCPlugin::Get()->Signal("AttachBlipToPlayer", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	else CRPCPlugin::Get()->Signal("AttachBlipToPlayer", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, CNetworkPlayer::GetByID(blip->GetPlayerID())->GetGUID(), false, false);
}

void API::AttachBlipToVehicle(unsigned long _guid, unsigned long vehicle)
{
	RakNet::BitStream bsOut;
	RakNetGUID guid = RakNetGUID(_guid);
	RakNetGUID veh(vehicle);
	CNetworkBlip *blip = CNetworkBlip::GetByGUID(guid);

	if (!blip) return;

	bsOut.Write(guid);
	bsOut.Write(veh);

	if (blip->GetPlayerID() == -1) CRPCPlugin::Get()->Signal("AttachBlipToVehicle", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	else CRPCPlugin::Get()->Signal("AttachBlipToVehicle", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, CNetworkPlayer::GetByID(blip->GetPlayerID())->GetGUID(), false, false);
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

unsigned long API::CreateObject(long model, float x, float y, float z, float pitch, float yaw, float roll)
{
	CNetworkObject *obj = new CNetworkObject(model, x, y, z, pitch, yaw, roll);
	return RakNetGUID::ToUint32(obj->rnGUID);
}

bool API::DeleteObject(unsigned long guid)
{
	RakNetGUID _guid(guid);
	auto obj = CNetworkObject::GetByGUID(_guid);
	if (obj) {
		delete obj;
	}
	BitStream bsOut;
	bsOut.Write(_guid);
	CRPCPlugin::Get()->Signal("DeleteObject", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	return true;
}

bool API::SendNotification(long playerid, const char * msg)
{
	auto player = CNetworkPlayer::GetByID(playerid);
	if (!player)
		return false;

	RakNet::BitStream bsOut;
	bsOut.Write(RakNet::RakString(msg));

	CRPCPlugin::Get()->Signal("SendNotification", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, player->GetGUID(), false, false);
	return false;
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

unsigned long API::Create3DText(const char * text, float x, float y, float z, int color, int outColor, float fontSize)
{
	CNetwork3DText * blip = new CNetwork3DText(x, y, z, color, outColor, text, -1, 0.f, 0.f, 0.f, fontSize);
	return RakNetGUID::ToUint32(blip->rnGUID);
}

unsigned long API::Create3DTextForPlayer(unsigned long player, const char * text, float x, float y, float z, int color, int outColor)
{
	auto pl = CNetworkPlayer::GetByGUID(RakNetGUID(player));
	CNetwork3DText * blip = new CNetwork3DText(x, y, z, color, outColor, text, pl->GetID());
	return RakNetGUID::ToUint32(blip->rnGUID);
}

bool API::Attach3DTextToVehicle(unsigned long textId, unsigned long vehicle, float oX, float oY, float oZ)
{
	CNetwork3DText *text = CNetwork3DText::GetByGUID(RakNetGUID(textId));
	auto veh = CNetworkVehicle::GetByGUID(RakNetGUID(vehicle));
	if (veh)
	{
		text->AttachToVehicle(*veh, oX, oY, oZ);
		return true;
	}
	return false;
}

bool API::Attach3DTextToPlayer(unsigned long textId, unsigned long player, float oX, float oY, float oZ)
{
	CNetwork3DText *text = CNetwork3DText::GetByGUID(RakNetGUID(textId));
	auto pl = CNetworkPlayer::GetByGUID(RakNetGUID(player));
	if (pl)
	{
		text->AttachToPlayer(*pl, oX, oY, oZ);
		return true;
	}
	return false;
}

bool API::Set3DTextContent(unsigned long textId, const char * text)
{
	CNetwork3DText *textItem = CNetwork3DText::GetByGUID(RakNetGUID(textId));
	textItem->SetText(text);
	return true;
}

bool API::Delete3DText(unsigned long textId)
{
	CNetwork3DText *textItem = CNetwork3DText::GetByGUID(RakNetGUID(textId));
	delete[] textItem;
	return true;
}

void API::Print(const char * message)
{
	log << message << std::endl;
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
