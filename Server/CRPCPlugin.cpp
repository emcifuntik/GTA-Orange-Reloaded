#include "stdafx.h"

CRPCPlugin* CRPCPlugin::singleInstance = nullptr;

CRPCPlugin::CRPCPlugin()
{
	CNetworkConnection::Get()->server->AttachPlugin(this);

	RegisterSlot("KeyEvent", Plugin::KeyEvent, 0);
	RegisterSlot("PlayerDeath", Plugin::PlayerDeath, 0);
	RegisterSlot("PlayerSpawn", Plugin::PlayerSpawn, 0);
	RegisterSlot("ServerEvent", Plugin::ServerEvent, 0);
}

CRPCPlugin *CRPCPlugin::Get()
{
	if (!singleInstance)
		singleInstance = new CRPCPlugin();
	return singleInstance;
}


CRPCPlugin::~CRPCPlugin()
{
	RPC4::DestroyInstance(this);
}
