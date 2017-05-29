#include "stdafx.h"

CivetServer *server;
//std::mutex smutex;

class HTTPHandler : public CivetHandler
{
public:
	bool handleGet(CivetServer *server, struct mg_connection *conn)
	{
		//std::cout << "[Request] New request from " << mg_get_request_info(conn)->remote_addr << std::endl;
		std::string responce;

		responce.append("[");
		//smutex.lock();

		std::vector<OrangeServer*> servers = OrangeServer::All();
		bool first = true;
		for (int i = 0; i < servers.size(); i++)
		{
			if (!first)
				responce.append(",");
			else
				first = false;

			auto oserver = servers[i];
			if (oserver->init)
			{
				char buffer[256];
				sprintf(buffer, "{\"ip\":\"%s\",\"port\":%d,\"name\":\"%s\",\"players\":%d,\"maxplayers\":%d,\"gamemode\":%s,\"isverified\":%s,\"haspassword\":%s}", oserver->IP.C_String(), oserver->Port, oserver->Hostname.C_String(), oserver->Players, oserver->MaxPlayers, oserver->Gamemode.C_String(), oserver->isVerified ? "true" : "false", oserver->hasPassword ? "true" : "false");
				responce.append(buffer);
			}
		}

		//smutex.unlock();
		responce.append("]");

		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: "
			"application/json\r\nConnection: close\r\n\r\n");

		mg_printf(conn, responce.c_str());

		return true;
	}
};

time_t lasttime = 0;

int main()
{
	const char *options[] = { "listening_ports", "7800", 0 };

	HTTPHandler h;
	server = new CivetServer(options);
	server->addHandler("/server-list/", h);

	CNetworkManager::Get()->Start();

	for (;;)
	{
		CNetworkManager::Get()->Tick();
	}

    return 0;
}

