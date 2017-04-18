#include "stdafx.h"

void CNetworkManager::Tick()
{
	for (packet = server->Receive(); packet; server->DeallocatePacket(packet), packet = server->Receive())
	{
		unsigned char packetIdentifier = packet->data[0];
		RakNet::BitStream bsIn(packet->data, packet->length, false);
		bsIn.IgnoreBytes(sizeof(unsigned char));
		RakNet::BitStream bsOut;

		switch (packetIdentifier)
		{

		}
	}
}

CNetworkManager::CNetworkManager()
{
	server = RakNet::RakPeerInterface::GetInstance();
}


CNetworkManager::~CNetworkManager()
{
}
