#include "stdafx.h"

std::vector<CNetworkVehicle *> CNetworkVehicle::Vehicles;

int CNetworkVehicle::Count()
{
	int _count = 0;
	for (CNetworkVehicle *vehicle : Vehicles)
	{
		if (vehicle)
			_count++;
	}
	return _count;
}

CNetworkVehicle::CNetworkVehicle(Hash model, float x, float y, float z, float heading):hashModel(model),vecPos(x,y,z),vecRot(0, 0, heading)
{
	rnGUID = RakNetGUID(createGUID());
	Vehicles.push_back(this);

	bool e = false;
	for (Hash _model : CNetworkConnection::Get()->UsedModels) if (_model == model) e = true;
	if (!e) CNetworkConnection::Get()->UsedModels.push_back(model);

	RakNet::BitStream bsOut;

	VehicleData data;
	GetVehicleData(data);

	data.RPM = 0.2f;
	data.Burnout = false;
	data.steering = 0;
	data.hasDriver = false;
	data.bHorn = false;

	bsOut.Write(data);

	CRPCPlugin::Get()->Signal("CreateVehicle", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, UNASSIGNED_RAKNET_GUID, true, false);
}

RakNetGUID CNetworkVehicle::GetGUID()
{
	return rnGUID;
}

void CNetworkVehicle::SetPosition(CVector3 position)
{
	vecPos = position;
}

void CNetworkVehicle::SetDriver(CNetworkPlayer* driver)
{
	this->driver = driver;
}

void CNetworkVehicle::SetRotation(CVector3 rotation)
{
	vecRot = rotation;
}

void CNetworkVehicle::SetHealth(unsigned short health)
{
	usHealth = health;
}

void CNetworkVehicle::SetVehicleData(const VehicleData & data)
{
	rnGUID = data.GUID;
	vecPos = data.vecPos;
	vecRot = data.vecRot;
	usHealth = data.usHealth;
	vecMoveSpeed = data.vecMoveSpeed;
	hasDriver = data.hasDriver;
	driverGUID = data.driver;
	fEngineHealth = data.fEngineHealth;
	fBodyHealth = data.fBodyHealth;
	fTankHealth = data.fTankHealth;
	bDrivable = data.bDrivable;
	bEngineStatus = data.bEngineStatus;
	bSirenState = data.bSirenState;

	if (hasDriver)
	{
		CNetworkPlayer::GetByGUID(data.driver)->SetCoords(vecPos);
	}
}

void CNetworkVehicle::GetVehicleData(VehicleData & data)
{
	data.GUID = rnGUID;
	data.vecPos = vecPos;
	data.vecRot = vecRot;
	data.usHealth = usHealth;
	data.hashModel = hashModel;
	data.vecMoveSpeed = vecMoveSpeed;
	data.hasDriver = hasDriver;
	data.driver = driverGUID;
	data.fEngineHealth = fEngineHealth;
	data.fBodyHealth = fBodyHealth;
	data.fTankHealth = fTankHealth;
	data.bDrivable = bDrivable;
	data.bSirenState = bSirenState;
	data.bEngineStatus = bEngineStatus;
	data.bEngineLocked = bEngineLocked;
	data.bTyresBulletproof = bTyresBulletproof;
	data.Color1 = Color1;
	data.Color2 = Color2;
	data.bLocked = bLocked;
}


CNetworkVehicle::~CNetworkVehicle()
{
	for (int i = 0; i < Vehicles.size(); i++)
		if (Vehicles[i]->rnGUID == rnGUID) Vehicles.erase(Vehicles.begin() + i, Vehicles.begin() + i + 1);
}

std::vector<CNetworkVehicle *> CNetworkVehicle::All()
{
	return Vehicles;
}

CNetworkVehicle * CNetworkVehicle::GetByGUID(RakNet::RakNetGUID GUID)
{
	for (CNetworkVehicle *veh : Vehicles)
		if (veh->rnGUID == GUID)
			return veh;
	return nullptr;
}

void CNetworkVehicle::SendGlobal(RakNet::Packet *packet)
{
	for (auto *veh : Vehicles)
	{
		RakNet::BitStream bsOut;

		VehicleData data;
		veh->GetVehicleData(data);

		data.RPM = 0.2f;
		data.Burnout = false;
		data.steering = 0;
		data.hasDriver = false;
		data.bHorn = false;

		bsOut.Write(data);

		CRPCPlugin::Get()->Signal("CreateVehicle", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, packet->guid, false, false);
	}
}

std::vector<unsigned int> CNetworkVehicle::GetPassengers()
{
	std::vector<unsigned int> result;
	for (CNetworkPlayer *player : CNetworkPlayer::All())
	{
		if (player && player->bInVehicle && player->vehicle == rnGUID)
		{
			result.push_back(player->GetID());
		}
	}
	return result;
}

int CNetworkVehicle::GetDriver()
{
	if (hasDriver == true)
	{
		for (CNetworkPlayer *player : CNetworkPlayer::All())
		{
			if (player && player->bInVehicle && player->vehicle == rnGUID)
			{
				if (player->cSeat == -1)
					return player->GetID();
			}
		}
	}
	return -1;
}