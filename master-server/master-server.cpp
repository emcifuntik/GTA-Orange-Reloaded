// master-server.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

struct OrangeServer
{
	std::string name;
	std::string ip;
	DWORD dietime = 0;
	unsigned short port;
	unsigned short players;
	unsigned short max_players;
};

std::vector<OrangeServer*> servers;
CivetServer *server;
std::mutex smutex;

class HTTPHandler : public CivetHandler
{
public:
	bool handleGet(CivetServer *server, struct mg_connection *conn)
	{
		std::cout << "[Request] New request from " << mg_get_request_info(conn)->remote_addr << std::endl;
		std::string responce;

		responce.append("[");
		smutex.lock();

		for (int i = 0; i < servers.size(); i++)
		{
			if (i > 0) responce.append(",");
			auto oserver = servers[i];
			char buffer[256];
			sprintf(buffer, "{\"ip\":\"%s\",\"port\":%d,\"name\":\"%s\",\"players\":%d,\"max-players\":%d}", oserver->ip.c_str(), oserver->port, oserver->name.c_str(), oserver->players, oserver->max_players);
			responce.append(buffer);
		}

		smutex.unlock();
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

	OrangeServer *s = new OrangeServer;

	s->name = "Orange TEST";
	s->ip = "192.168.0.2";
	s->port = 7788;
	s->players = 100;
	s->max_players = 256;

	servers.push_back(s);
	servers.push_back(s);
	servers.push_back(s);

	for (;;)
	{
		smutex.lock();
		time_t t = time(0);

		for (int i = 0; i < servers.size();)
		{
			auto srv = servers[i];
			if(srv->dietime < t) servers.erase(servers.begin() + i);
			else i++;
		}

		lasttime = t;
		smutex.unlock();
	}

    return 0;
}

