#include "stdafx.h"

std::vector<CNetworkMarker *> CNetworkMarker::GlobalMarkers;
std::vector<CNetworkMarker *> CNetworkMarker::PlayerMarkers;

std::map<CNetworkMarker *, std::vector<CNetworkPlayer *>> playermap;
std::map<CNetworkMarker *, std::vector<CNetworkVehicle *>> vehmap;

CNetworkMarker::CNetworkMarker(float x, float y, float z, float height, float radius, int playerid, int color)
{
	RakNet::BitStream bsOut;

	rnGUID = RakNetGUID::RakNetGUID(createGUID());

	bsOut.Write(rnGUID);
	bsOut.Write(x);
	bsOut.Write(y);
	bsOut.Write(z);

	bsOut.Write(height);
	bsOut.Write(radius);
	bsOut.Write(color);

	vecPos = CVector3(x, y, z);
	this->height = height;
	this->radius = radius;
	this->playerid = playerid;

	if (playerid == -1)
	{
		CRPCPlugin::Get()->Signal("CreateMarker", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
		GlobalMarkers.push_back(this);
		log << "gl m" << std::endl;
	}
	else {
		RakNetGUID guid = CNetworkPlayer::GetByID(playerid)->GetGUID();
		CRPCPlugin::Get()->Signal("CreateMarker", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, guid, false, false);
		PlayerMarkers.push_back(this);
		log << "loc m" << std::endl;
	}

}

void CNetworkMarker::Delete()
{
}


CNetworkMarker::~CNetworkMarker()
{
	RakNet::BitStream bsOut;

	bsOut.Write(rnGUID);

	if(playerid == -1) CRPCPlugin::Get()->Signal("DeleteMarker", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, UNASSIGNED_RAKNET_GUID, true, false);
	else CRPCPlugin::Get()->Signal("DeleteMarker", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, CNetworkPlayer::GetByID(playerid)->GetGUID(), false, false);
}

void CNetworkMarker::SendGlobal(RakNet::Packet *packet)
{
	for each(auto *Marker in GlobalMarkers)
	{
		RakNet::BitStream bsOut;

		bsOut.Write(Marker->rnGUID);
		bsOut.Write(Marker->vecPos.fX);
		bsOut.Write(Marker->vecPos.fY);
		bsOut.Write(Marker->vecPos.fZ);
		bsOut.Write(Marker->height);
		bsOut.Write(Marker->radius);
		bsOut.Write(Marker->color);

		CRPCPlugin::Get()->Signal("CreateMarker", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, packet->guid, false, false);
	}
}

CNetworkMarker * CNetworkMarker::GetByGUID(RakNetGUID guid)
{
	for each (CNetworkMarker *Marker in GlobalMarkers)
		if (Marker && Marker->rnGUID == guid)
			return Marker;

	for each (CNetworkMarker *Marker in PlayerMarkers)
		if (Marker && Marker->rnGUID == guid)
			return Marker;

	return nullptr;
}

void CNetworkMarker::Tick()
{
	for (auto m : GlobalMarkers)
	{
		auto map = &playermap[m];
		for (auto pl : CNetworkPlayer::All())
		{
			if (!pl) continue;
			CVector3 mPos = m->vecPos;
			CVector3 pPos = pl->GetPosition();

			if (Math::GetDistanceBetweenPoints2D(mPos.fX, mPos.fY, pPos.fX, pPos.fY) < m->radius && (pPos.fZ - mPos.fZ)*(pPos.fZ - mPos.fZ) < m->radius*m->radius)
			{
				bool was = false;
				for (auto _pl : *map) if (_pl == pl) was = true;
				if (!was) {
					map->push_back(pl);
					Plugin::Trigger("EnterMarker", (ULONG)pl->GetID(), RakNetGUID::ToUint32(m->rnGUID));
				}
			}
			else {
				for (int i = 0; i < map->size(); i++) {
					if (map->at(i) == pl) {
						map->erase(map->begin() + i, map->begin() + i + 1);
						Plugin::Trigger("LeftMarker", (ULONG)pl->GetID(), RakNetGUID::ToUint32(m->rnGUID));
					}
				}
			}
		}

		auto vmap = &vehmap[m];
		for (auto veh : CNetworkVehicle::All())
		{
			if (!veh) continue;
			CVector3 mPos = m->vecPos;
			CVector3 pPos = veh->GetPosition();

			if (Math::GetDistanceBetweenPoints2D(mPos.fX, mPos.fY, pPos.fX, pPos.fY) < m->radius && (pPos.fZ - mPos.fZ)*(pPos.fZ - mPos.fZ) < m->radius*m->radius)
			{
				bool was = false;
				for (auto _veh : *vmap) if (_veh == veh) was = true;
				if (!was) {
					vmap->push_back(veh);
					Plugin::Trigger("VehEnterMarker", (ULONG)RakNetGUID::ToUint32(veh->GetGUID()), RakNetGUID::ToUint32(m->rnGUID));
				}
			}
			else {
				for (int i = 0; i < vmap->size(); i++) {
					if (vmap->at(i) == veh) {
						vmap->erase(vmap->begin() + i, vmap->begin() + i + 1);
						Plugin::Trigger("VehLeftMarker", (ULONG)RakNetGUID::ToUint32(veh->GetGUID()), RakNetGUID::ToUint32(m->rnGUID));
					}
				}
			}
		}
	}
	for (auto m : PlayerMarkers)
	{
		auto map = &playermap[m];

		auto pl = CNetworkPlayer::GetByID(m->playerid);
		if (!pl) continue;

		if (!pl->bInVehicle)
		{
			CVector3 mPos = m->vecPos;
			CVector3 pPos = pl->GetPosition();

			if (Math::GetDistanceBetweenPoints2D(mPos.fX, mPos.fY, pPos.fX, pPos.fY) < m->radius && (pPos.fZ - mPos.fZ)*(pPos.fZ - mPos.fZ) < m->radius*m->radius)
			{
				bool was = false;
				for (auto _pl : *map) if (_pl == pl) was = true;
				if (!was) {
					map->push_back(pl);
					Plugin::Trigger("EnterMarker", (ULONG)pl->GetID(), RakNetGUID::ToUint32(m->rnGUID));
				}
			}
			else {
				for (int i = 0; i < map->size(); i++) {
					if (map->at(i) == pl) {
						map->erase(map->begin() + i, map->begin() + i + 1);
						Plugin::Trigger("LeftMarker", (ULONG)pl->GetID(), RakNetGUID::ToUint32(m->rnGUID));
					}
				}
			}
		}
		else {
			auto vmap = &vehmap[m];
			auto veh = CNetworkVehicle::GetByGUID(pl->vehicle);
			if (!veh) continue;
			CVector3 mPos = m->vecPos;
			CVector3 pPos = veh->GetPosition();

			if (Math::GetDistanceBetweenPoints2D(mPos.fX, mPos.fY, pPos.fX, pPos.fY) < m->radius && (pPos.fZ - mPos.fZ)*(pPos.fZ - mPos.fZ) < m->radius*m->radius)
			{
				bool was = false;
				for (auto _veh : *vmap) if (_veh == veh) was = true;
				if (!was) {
					vmap->push_back(veh);
					Plugin::Trigger("VehEnterMarker", (ULONG)RakNetGUID::ToUint32(veh->GetGUID()), RakNetGUID::ToUint32(m->rnGUID));
				}
			}
			else {
				for (int i = 0; i < vmap->size(); i++) {
					if (vmap->at(i) == veh) {
						vmap->erase(vmap->begin() + i, vmap->begin() + i + 1);
						Plugin::Trigger("VehLeftMarker", (ULONG)RakNetGUID::ToUint32(veh->GetGUID()), RakNetGUID::ToUint32(m->rnGUID));
					}
				}
			}
		}
	}
}

std::vector<CNetworkMarker *> CNetworkMarker::AllGlobal()
{
	return GlobalMarkers;
}

