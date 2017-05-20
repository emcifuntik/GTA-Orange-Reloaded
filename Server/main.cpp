#include "stdafx.h"

struct pass
{
	template<typename ...T> pass(T...) {}
};

int counter = 1;
unsigned long createGUID() { return counter++; }

#ifndef _WIN32
#include <sys/time.h>
unsigned long GetTickCount()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (tv.tv_sec*1000+tv.tv_usec/1000);
}
#endif

bool bExit = false;

#ifdef _WIN32

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	Plugin::Trigger("ServerUnload", dwCtrlType);
	bExit = true;
	return TRUE;
}

#else

void sig_handler(int s) {
	if(s == 2) Plugin::Trigger("ServerUnload");
	else printf("Caught signal %d\n", s);
	exit(1);
}

#endif

int main(void)
{
#ifdef _WIN32
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);
#else
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = sig_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);
#endif

	log << "Starting the server..." << std::endl;
	log << "Hostname: " << color::lred << CConfig::Get()->Hostname << std::endl;
	log << "Port: " << color::lred << CConfig::Get()->Port << std::endl;
	log << "Maximum players: " << color::lred << CConfig::Get()->MaxPlayers << std::endl;

	Plugin::LoadPlugins();

	/*CHTTPServer::Get()->Start(CConfig::Get()->HTTPPort);
	CHTTPHandler h_a;
	CHTTPServer::g_server->addHandler("", h_a);*/

	auto netLoop = [=]()
	{
		CNetworkConnection::Get()->Start(CConfig::Get()->MaxPlayers, CConfig::Get()->Port);
		CRPCPlugin::Get();
		DWORD lastTick = 0;
		RakNet::RakNetStatistics stat;

		for (;;)
		{
			RakSleep(5);
			CNetworkConnection::Get()->Tick();
			CNetworkPlayer::Tick();
			CNetworkMarker::Tick();
			Plugin::Tick();

			if ((GetTickCount() - lastTick) > 100)
			{
				CNetworkConnection::Get()->server->GetStatistics(0, &stat);
				std::stringstream ss;
				ss << CConfig::Get()->Hostname << ". Players online: " << CNetworkPlayer::Count() << ", "
					<< "Packet loss: " << std::setprecision(2) << std::fixed << stat.packetlossTotal * 100 << "%";
				//SetConsoleTitle(ss.str().c_str());
				lastTick = GetTickCount();
			}
		}
	};
	std::thread netThread(netLoop);
	netThread.detach();

	for (;;)
	{
		std::string msg;
		std::getline(std::cin, msg);
		if (bExit || !msg.compare("exit") || !msg.compare("kill"))
		{
			log << "Terminating server..." << std::endl;
#ifndef _WIN32
			Plugin::Trigger("ServerUnload");
#endif
			break;
		}
		else
		{
			Plugin::ServerCommand(msg);
		}
	}
	netThread.~thread();
	//delete Python::Get();
	return 0;
}
