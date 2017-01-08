#include "stdafx.h"

std::vector<CClientScript> CClientScripting::scripts;

CClientScripting::CClientScripting()
{

}

CClientScripting::~CClientScripting()
{
	
}

void CClientScripting::AddScript(std::string name, char* buffer, size_t size)
{
	log << buffer[100] << buffer[99];
	CClientScript script = { (unsigned char*)buffer, size, name };
	log << script.buffer[100] << script.buffer[99];
	scripts.push_back(script);
}

void CClientScripting::SendGlobal(RakNet::Packet *packet)
{
	for each(auto script in scripts)
	{
		RakNet::BitStream bsOut;
		bsOut.Write(RakString(script.name.c_str()));
		bsOut.Write(script.size);
		log << script.buffer[100] << script.buffer[99];
		bsOut.WriteAlignedBytes(script.buffer, script.size);
		log << "1";
		CRPCPlugin::Get()->Signal("LoadScript", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, packet->guid, false, false);
		log << "1";
	}
}

