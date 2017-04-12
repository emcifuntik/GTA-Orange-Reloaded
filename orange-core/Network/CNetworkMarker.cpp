#include "stdafx.h"

std::vector<CNetworkMarker *> CNetworkMarker::MarkerPool;

CNetworkMarker::CNetworkMarker(RakNetGUID guid, float x, float y, float z, float height, float radius, color_t color)
{
	m_GUID = guid;
	this->vecPos = CVector3(x, y, z);
	this->radius = radius;

	Handle = GRAPHICS::CREATE_CHECKPOINT(45, x, y, z - 1.1f, 0, 0, 0, radius, 255, 216, 0, 115, 0);
	SetHeight(height);
	SetColor(color);
	MarkerPool.push_back(this);
}

void CNetworkMarker::SetColor(color_t color)
{
	this->color = color;
	GRAPHICS::SET_CHECKPOINT_RGBA(Handle, color.red, color.green, color.blue, color.alpha);
	GRAPHICS::_SET_CHECKPOINT_ICON_RGBA(Handle, color.red, color.green, color.blue, color.alpha);
}

void CNetworkMarker::SetHeight(float height)
{
	this->height = height;
	GRAPHICS::SET_CHECKPOINT_CYLINDER_HEIGHT(Handle, height, height, radius);
}

void CNetworkMarker::SetScale(float scale)
{
	GRAPHICS::_0x4B5B4DA5D79F1943(Handle, scale);
}

CNetworkMarker::~CNetworkMarker()
{
	GRAPHICS::DELETE_CHECKPOINT(Handle);
}

void CNetworkMarker::Clear()
{
	for each(CNetworkMarker* m in MarkerPool)
		delete m;

	MarkerPool.erase(MarkerPool.begin(), MarkerPool.end());
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
