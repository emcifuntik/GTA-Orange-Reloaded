#include "stdafx.h"

std::vector<CNetworkPlayer*> CNetworkPlayer::_players;

void CNetworkPlayer::Each(void(*func)(CNetworkPlayer *))
{
	for each (CNetworkPlayer *player in _players)
		if (player)
			func(player);
}

CNetworkPlayer * CNetworkPlayer::GetByGUID(RakNet::RakNetGUID GUID)
{
	for each (CNetworkPlayer *player in _players)
		if (player && player->rnGUID == GUID)
			return player;
	CNetworkPlayer *player = new CNetworkPlayer(GUID);
	return player;
}

CNetworkPlayer * CNetworkPlayer::GetByID(UINT playerID)
{
	if (playerID < _players.size())
		return _players[playerID];
	return nullptr;
}

void CNetworkPlayer::AddPlayer(CNetworkPlayer *player)
{
	int playerID = -1;
	for (size_t i = 0; i < _players.size(); ++i)
	{
		if (!_players[i])
		{
			playerID = (int)i;
			break;
		}
	}
	if (playerID == -1)
	{
		player->uiID = (unsigned int)_players.size();
		_players.push_back(player);
	}
	else
	{
		player->uiID = (unsigned int)playerID;
		_players[playerID] = player;
	}
}

CNetworkPlayer::CNetworkPlayer(RakNet::RakNetGUID GUID):rnGUID(GUID)
{
	AddPlayer(this);
}

CNetworkPlayer::~CNetworkPlayer()
{
	_players[uiID] = nullptr;
}

void CNetworkPlayer::SetOnFootData(const OnFootSyncData& data)
{
	hModel = data.hModel;
	bJumping = data.bJumping;
//	fMoveSpeed = data.fMoveSpeed;

	fForwardPotential = data.fForwardPotential;
	fStrafePotential = data.fStrafePotential;
	fRotationPotential = data.fRotationPotential;
	dwMovementFlags = data.dwMovementFlags;

	vecPosition = data.vecPos;
	vecRotation = data.vecRot;
	fHeading = data.fHeading;
	ulWeapon = data.ulWeapon;
	uAmmo = data.uAmmo;
	usHealth = data.usHealth;
	usArmour = data.usArmour;
	bDucking = data.bDuckState;
	vecMoveSpeed = data.vecMoveSpeed;
	vecAim = data.vecAim;
	bAiming = data.bAiming;
	bShooting = data.bShooting;
	bInVehicle = data.bInVehicle;

	if (!bInVehicle && bEnteringVeh)
	{
		Plugin::Trigger("LeftVehicle", (unsigned long)GetID(), RakNetGUID::ToUint32(vehicle));
		bEnteringVeh = false;
	}
	else if (bInVehicle && !bEnteringVeh) {
		Plugin::Trigger("EnterVehicle", (unsigned long)GetID(), RakNetGUID::ToUint32(data.vehicle));
		bEnteringVeh = true;
	}

	vehicle = data.vehicle;
	sSeat = data.vehseat;
}

void CNetworkPlayer::GetOnFootData(OnFootSyncData& data)
{
	data.hModel = hModel;
	data.bJumping = bJumping;
//	data.fMoveSpeed = fMoveSpeed;

	data.fForwardPotential = fForwardPotential;
	data.fStrafePotential = fStrafePotential;
	data.fRotationPotential = fRotationPotential;
	data.dwMovementFlags = dwMovementFlags;

	data.vecPos = vecPosition;
	data.vecRot = vecRotation;
	data.fHeading = fHeading;
	data.ulWeapon = ulWeapon;
	data.uAmmo = uAmmo;
	data.usHealth = usHealth;
	data.usArmour = usArmour;
	data.bDuckState = bDucking;
	data.vecMoveSpeed = vecMoveSpeed;
	data.vecAim = vecAim;
	data.bAiming = bAiming;
	data.bShooting = bShooting;
	data.bInVehicle = bInVehicle;
	data.vehicle = vehicle;
	data.vehseat = sSeat;
}

void CNetworkPlayer::SetPosition(const CVector3 & position)
{
	RakNet::BitStream bsOut;
	bsOut.Write(position);
	CRPCPlugin::Get()->Signal("SetPlayerPos", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, rnGUID, false, false);
}

void CNetworkPlayer::SetCoords(const CVector3 & position)
{
	vecPosition = position;
}

void CNetworkPlayer::GiveWeapon(unsigned int weaponHash, unsigned int ammo)
{
	RakNet::BitStream bsOut;
	bsOut.Write(weaponHash);
	bsOut.Write(ammo);
	CRPCPlugin::Get()->Signal("GivePlayerWeapon", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, rnGUID, false, false);
}

void CNetworkPlayer::GiveAmmo(unsigned int weaponHash, unsigned int ammo)
{
	RakNet::BitStream bsOut;
	bsOut.Write(weaponHash);
	bsOut.Write(ammo);
	CRPCPlugin::Get()->Signal("GivePlayerAmmo", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, rnGUID, false, false);
}

void CNetworkPlayer::SetModel(unsigned int model)
{
	RakNet::BitStream bsOut;
	bsOut.Write(model);
	CRPCPlugin::Get()->Signal("SetPlayerModel", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, rnGUID, false, false);
}

void CNetworkPlayer::SetHealth(float health)
{
	RakNet::BitStream bsOut;
	bsOut.Write(health);
	CRPCPlugin::Get()->Signal("SetPlayerHealth", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, rnGUID, false, false);
}

void CNetworkPlayer::SetArmour(float armour)
{
	RakNet::BitStream bsOut;
	bsOut.Write(armour);
	CRPCPlugin::Get()->Signal("SetPlayerArmour", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, rnGUID, false, false);
}

void CNetworkPlayer::SetColor(unsigned int color)
{
	RakNet::BitStream bsOut;
	color_t col;
	col.red = (BYTE)(((color >> 24) & 0xFF) / 255.0);  // Extract the RR byte
	col.green = (BYTE)(((color >> 16) & 0xFF) / 255.0);   // Extract the GG byte
	col.blue = (BYTE)(((color >> 8) & 0xFF) / 255.0);   // Extract the GG byte
	col.alpha = (BYTE)(((color) & 0xFF) / 255.0);        // Extract the BB byte
	bsOut.Write(col);
	colColor = col;
	CRPCPlugin::Get()->Signal("SetPlayerColor", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, rnGUID, false, false);
}

//void CNetworkPlayer::SendTextMessage(const char *message, unsigned int color)
//{
//	RakNet::BitStream bsOut;
//	RakNet::RakString msg(message);
//	bsOut.Write(msg);
//	color_t col;
//	col.red = (BYTE)((color >> 24) & 0xFF);  // Extract the RR byte
//	col.green = (BYTE)((color >> 16) & 0xFF);   // Extract the GG byte
//	col.blue = (BYTE)((color >> 8) & 0xFF);   // Extract the GG byte
//	col.alpha = (BYTE)((color) & 0xFF);        // Extract the BB byte
//	bsOut.Write(col);
//	CRPCPlugin::Get()->Signal("SendClientMessage", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, rnGUID, false, false);
//}

void CNetworkPlayer::Tick()
{
	for each (CNetworkPlayer *player in _players)
	{
		if (!player)
			continue;
		if (player->usHealth <= 0 && !player->bDead)
		{
			// Call death callback
			player->bDead = true;
		}

		if (player->usHealth > 0 && player->bDead) {
			// Spawn player here
			player->bDead = false;
		}
	}
}

UINT CNetworkPlayer::Count()
{
	UINT count = 0;
	for each (CNetworkPlayer *player in _players)
	{
		if (player)
			count++;
	}
	return count;
}

std::vector<CNetworkPlayer *> CNetworkPlayer::All()
{
	return _players;
}

void CNetworkPlayer::Remove(int playerid)
{
	delete _players[playerid];
	_players[playerid] = nullptr;
}
