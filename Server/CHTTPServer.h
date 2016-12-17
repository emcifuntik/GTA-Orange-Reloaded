#pragma once

class CHTTPHandler : public CivetHandler
{
public:
	bool handleGet(CivetServer *server, struct mg_connection *conn)
	{
		const struct mg_request_info *req_info = mg_get_request_info(conn);

		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: "
			"text/html\r\nConnection: close\r\n\r\n");
		const char* res = Plugin::OnHTTPRequest("GET", req_info->request_uri, req_info->query_string ? req_info->query_string : "", "");
		if (res != NULL) mg_printf(conn, res);
		return true;
	}

	bool handlePost(CivetServer *server, struct mg_connection *conn)
	{
		const struct mg_request_info *req_info = mg_get_request_info(conn);
		std::string data;
		int readBytes;

		char buffer[1024];
		while ((readBytes = mg_read(conn, buffer, 1024)) > 0) {
			data.append(buffer, 0, (readBytes));
		}

		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: "
			"text/html\r\nConnection: close\r\n\r\n");
		const char* res = Plugin::OnHTTPRequest("POST", req_info->request_uri, req_info->query_string ? req_info->query_string : "", data);
		if(res != NULL) mg_printf(conn, res);
		return true;
	}
};

class CHTTPServer
{
public:
	static CivetServer g_server;
	static CHTTPServer * Get();
	CHTTPServer();
	bool Start(int port);
	static bool BindHandlers();
	~CHTTPServer();
private:
	static CHTTPServer *singleInstance;
};
