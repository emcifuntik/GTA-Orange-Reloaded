#include "stdafx.h"

struct pass
{
	template<typename ...T> pass(T...) {}
};

int counter = 1;
unsigned long createGUID() { return counter++; }

int main(void)
{
	log << "Starting the server..." << std::endl;
	log << "Hostname: " << color::lred << CConfig::Get()->Hostname << std::endl;
	log << "Port: " << color::lred << CConfig::Get()->Port << std::endl;
	log << "Maximum players: " << color::lred << CConfig::Get()->MaxPlayers << std::endl;

	Plugin::LoadPlugins();	

	CHTTPServer::Get()->Start(88);
	CHTTPHandler h_a;
	CHTTPServer::g_server.addHandler("", h_a);
	
	auto netLoop = [=]()
	{
		CNetworkConnection::Get()->Start(CConfig::Get()->MaxPlayers, CConfig::Get()->Port);
		CRPCPlugin::Get();
		DWORD lastTick = 0;
		RakNet::RakNetStatistics stat;
		auto veh = new CNetworkVehicle(API::Get()->Hash("adder"), 77.2324f, -690.258f, 43.6847f, 339.335f);
		auto blip = new CNetwork3DText(0.f, 0.f, 71.f, 0xAAAAAAFF, 0x000000FF, "[ 0 ] Funtik");
		blip->AttachToVehicle(*veh);

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
				SetConsoleTitle(ss.str().c_str());
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
		if (!msg.compare("exit") || !msg.compare("kill"))
		{
			log << "Terminating server..." << std::endl;
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