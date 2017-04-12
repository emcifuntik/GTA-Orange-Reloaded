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
	CClientScript script = { (unsigned char*)buffer, size, name };
	scripts.push_back(script);
}

void CClientScripting::SendGlobal(RakNet::Packet *packet)
{
	RakNet::BitStream bsOut;
	bsOut.Write((unsigned char)1);
	bsOut.Write(scripts.size());

	for (auto script : scripts)
	{
		unsigned int size = script.size;

		bsOut.Write(RakString(script.name.c_str()));
		bsOut.Write(size);
		bsOut.WriteAlignedBytes(script.buffer, script.size);

		//CRPCPlugin::Get()->Signal("LoadScript", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, packet->guid, false, false);
		//log << "Sending: " << script.name << ", size: " << script.size << ", offset: " << bsOut.GetWriteOffset() << std::endl;
	}
	CNetworkConnection::Get()->tcpserver->Send(reinterpret_cast<char*>(bsOut.GetData()), bsOut.GetNumberOfBytesUsed(), packet->systemAddress, false);
}

