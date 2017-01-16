#include "stdafx.h"

CHTTPServer *CHTTPServer::singleInstance = nullptr;
CivetServer CHTTPServer::g_server = NULL;

CHTTPServer *CHTTPServer::Get()
{
	if (!singleInstance)
		singleInstance = new CHTTPServer();
	return singleInstance;
}

CHTTPServer::CHTTPServer()
{
	
}

bool CHTTPServer::Start(int port)
{
	std::string portStr = std::to_string(port);
	const char *options[] = {
		"document_root", ".", "listening_ports", portStr.c_str(), 0 };

	std::vector<std::string> cpp_options;
	for (int i = 0; i<(sizeof(options) / sizeof(options[0]) - 1); i++) {
		cpp_options.push_back(options[i]);
	}
	CHTTPServer::g_server = *(new CivetServer(cpp_options));
	
	return true;
}

bool CHTTPServer::BindHandlers()
{
	return true;
}

CHTTPServer::~CHTTPServer()
{
}
