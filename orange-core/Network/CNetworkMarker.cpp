#include "stdafx.h"

std::vector<CNetworkMarker *> CNetworkMarker::MarkerPool;

CNetworkMarker::CNetworkMarker(RakNetGUID guid, float x, float y, float z, float height, float radius)
{
	m_GUID = guid;
	this->vecPos = CVector3(x, y, z);
	this->height = height;
	this->radius = radius;
	this->color = color;

	Handle = GRAPHICS::CREATE_CHECKPOINT(45, x, y, z - 1.1, 0, 0, 0, radius, 255, 216, 0, 115, 0);
	SetHeight(height);

	MarkerPool.push_back(this);
}

void CNetworkMarker::SetColor(int color)
{
	//UI::SET_Marker_COLOUR(Handle, color);
}

void CNetworkMarker::SetHeight(float height)
{
	this->height = height;
	GRAPHICS::SET_CHECKPOINT_CYLINDER_HEIGHT(Handle, height, height, radius);
}

CNetworkMarker::~CNetworkMarker()
{
	GRAPHICS::DELETE_CHECKPOINT(Handle);
}

std::vector<CNetworkMarker*> CNetworkMarker::All()
{
	return MarkerPool;
}

CNetworkMarker * CNetworkMarker::GetByGUID(RakNet::RakNetGUID GUID)
{
	for each (CNetworkMarker *_Marker in MarkerPool)
	{
		if (_Marker->m_GUID == GUID)
			return _Marker;
	}
	return nullptr;
}

void CNetworkMarker::DeleteByGUID(RakNet::RakNetGUID GUID)
{
	for each (CNetworkMarker *_Marker in MarkerPool)
	{
		if (_Marker->m_GUID == GUID)
			_Marker->~CNetworkMarker();
	}
}
