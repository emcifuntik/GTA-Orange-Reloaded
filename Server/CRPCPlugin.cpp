#include "stdafx.h"

CRPCPlugin* CRPCPlugin::singleInstance = nullptr;

CRPCPlugin::CRPCPlugin()
{
	CNetworkConnection::Get()->server->AttachPlugin(this);

	RegisterSlot("KeyEvent", Plugin::KeyEvent, 0);
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
