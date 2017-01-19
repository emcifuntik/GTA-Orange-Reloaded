#include "stdafx.h"
#define log std::cout

std::vector<CClientScript> CClientScripting::scripts;

CClientScripting::CClientScripting()
{

}

CClientScripting::~CClientScripting()
{

}

void CClientScripting::AddScript(std::string name, char* buffer, size_t size)
{
	CClientScript script = { (unsigned char*)buffer, size, name };
	scripts.push_back(script);
}

void CClientScripting::SendGlobal(RakNet::Packet *packet)
{
	for (auto script : scripts)
	{
		RakNet::BitStream bsOut;
		bsOut.Write(RakString(script.name.c_str()));
		bsOut.Write(script.size);
		bsOut.WriteAlignedBytes(script.buffer, script.size);
		CRPCPlugin::Get()->Signal("LoadScript", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, packet->guid, false, false);
	}
}

