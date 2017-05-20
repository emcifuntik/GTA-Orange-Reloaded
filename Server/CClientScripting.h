#pragma once

struct CClientScript {
	unsigned char* buffer;
	size_t size;
	std::string name;
};

class CClientScripting
{
public:
	static std::vector<CClientScript> scripts;
	static std::vector<std::string> files;
	CClientScripting();
	~CClientScripting();
	static void AddScript(std::string name, char * buffer, size_t size);
	static void AddFile(std::string path);
	static void SendGlobal(RakNet::Packet * packet);
};


