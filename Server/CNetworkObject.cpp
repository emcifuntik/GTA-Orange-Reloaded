#include "stdafx.h"

std::vector<CNetworkObject *> CNetworkObject::Objects;

int CNetworkObject::Count()
{
	int _count = 0;
	for each (CNetworkObject *Object in Objects)
	{
		if (Object)
			_count++;
	}
	return _count;
}

CNetworkObject::CNetworkObject(Hash model, float x, float y, float z, float pitch, float yaw, float roll) :hashModel(model), vecPos(x, y, z), vecRot(pitch, yaw, roll), usHealth(100)
{
	rnGUID = RakNetGUID::RakNetGUID(createGUID());
	Objects.push_back(this);

	bool e = false;
	for (Hash _model : CNetworkConnection::Get()->UsedModels) if (_model == model) e = true;
	if (!e) CNetworkConnection::Get()->UsedModels.push_back(model);

	RakNet::BitStream bsOut;

	ObjectData data;
	GetObjectData(data);

	bsOut.Write(data);

	CRPCPlugin::Get()->Signal("CreateObject", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, UNASSIGNED_RAKNET_GUID, true, false);
}

RakNetGUID CNetworkObject::GetGUID()
{
	return rnGUID;
}

void CNetworkObject::SetPosition(CVector3 position)
{
	vecPos = position;
}

void CNetworkObject::SetRotation(CVector3 rotation)
{
	vecRot = rotation;
}

void CNetworkObject::SetHealth(unsigned short health)
{
	usHealth = health;
}

/*void CNetworkObject::SetObjectData(const ObjectData & data)
{
	rnGUID = data.GUID;
	vecPos = data.vecPos;
	vecRot = data.vecRot;
	usHealth = data.usHealth;
	vecMoveSpeed = data.vecMoveSpeed;
	hasDriver = data.hasDriver;
	driverGUID = data.driver;

	if (hasDriver)
	{
		CNetworkPlayer::GetByGUID(data.driver)->SetCoords(vecPos);
	}
}*/

void CNetworkObject::GetObjectData(ObjectData & data)
{
	data.GUID = rnGUID;
	data.vecPos = vecPos;
	data.vecRot = vecRot;
	data.usHealth = usHealth;
	data.hashModel = hashModel;
}


CNetworkObject::~CNetworkObject()
{

}

std::vector<CNetworkObject *> CNetworkObject::All()
{
	return Objects;
}

CNetworkObject * CNetworkObject::GetByGUID(RakNet::RakNetGUID GUID)
{
	for each (CNetworkObject *veh in Objects)
		if (veh->rnGUID == GUID)
			return veh;
	return nullptr;
}

void CNetworkObject::SendGlobal(RakNet::Packet *packet)
{
	for each(auto *obj in Objects)
	{
		RakNet::BitStream bsOut;

		ObjectData data;
		obj->GetObjectData(data);

		bsOut.Write(data);

		CRPCPlugin::Get()->Signal("CreateObject", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, packet->guid, false, false);
	}
}
