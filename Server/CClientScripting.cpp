#include "stdafx.h"

std::vector<CClientScript> CClientScripting::scripts;
std::vector<std::string> CClientScripting::files;

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

void CClientScripting::AddFile(std::string path)
{
	files.push_back(path);
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

	bsOut.Write(files.size());

	for (std::string file : files)
	{
		std::ifstream ifile(file, std::ios::binary | std::ios::ate);
		size_t fsize = ifile.tellg();
		ifile.seekg(std::ios::beg);

		//log << file << "(" << fsize << "): " << std::endl;
		char* data = (char*)malloc(fsize);

		ifile.read(data, fsize);

		//for (int i = 0; i < fsize; i++)
		//	std::cout << std::hex << (int)data[i] << " ";

		//log << data << std::endl;
		RakString sfile = RakString(file.c_str());
		bsOut.Write(sfile);
		bsOut.Write(fsize);
		bsOut.WriteAlignedBytes((unsigned char*)data, fsize);

		free(data);
	}

	CNetworkConnection::Get()->tcpserver->Send(reinterpret_cast<char*>(bsOut.GetData()), bsOut.GetNumberOfBytesUsed(), packet->systemAddress, false);
}

