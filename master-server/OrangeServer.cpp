#include "stdafx.h"

std::vector<OrangeServer *> OrangeServer::ServerList;

int OrangeServer::Count()
{
	int _count = 0;
	for (OrangeServer *Server : ServerList)
	{
		if (Server)
			_count++;
	}
	return _count;
}

OrangeServer::OrangeServer(RakNet::RakNetGUID GUID)
{
	this->GUID = GUID;
	ServerList.push_back(this);
}

OrangeServer::~OrangeServer()
{
	for (int i = 0; i < ServerList.size(); i++)
		if (ServerList[i]->GUID == GUID) ServerList.erase(ServerList.begin() + i, ServerList.begin() + i + 1);
}


std::vector<OrangeServer *> OrangeServer::All()
{
	return ServerList;
}

OrangeServer * OrangeServer::GetByGUID(RakNet::RakNetGUID GUID)
{
	for (OrangeServer *Server : ServerList)
		if (Server->GUID == GUID)
			return Server;
	return nullptr;
}