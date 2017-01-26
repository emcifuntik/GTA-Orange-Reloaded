// dllmain.cpp: определяет точку входа для приложения DLL.
#include "stdafx.h"

std::string my_ostream::fname = "output.log";

const char * hwids[] = {
	"507afa0100e723cd60505ef1c02f05aa", //Metmonk
	"de8b7a030de6eed2d84f3c6df3e3257d", //TN1KS
	"21d847e027c78073c4eaf96502c93409", //BodyanGamer
	"4ae8e22684f5c2ae5fa2651e65e93d87", //RZX
	"2fbc5fbc89e6bc83e941334cdf289a67", //James_Braga
	"f289ef2d3bd4282dea3200d1a8643069", //SWF*
	"fb6ef2bd04c467f98d1610a3760618c9", //LambdaE
	"0738335dccb051518b1fc438a11cb403", //ZlamboV
	"4b8b6d024c9eaf1ff3ae8a34128c625c", //HarrWe
	"d6fb86f1e108590c33048a5f0e0262b6", //Scorpi
	"270e28492f8842197c055a57308ab1ad", //Doc
	"976dff169854464c09d44e1435d5d07f", //_Pokemon
	"37329c13ce9a73a4efd62c2e6dd67c6e", //__encoder
	"8428e015aff0780dcac5de8d9c874548", //Chesko
	"4d92907275e69028d4d55f19de7da7d2", //Tribunal
	"e0735fa46a46402ada23a2f412baa785", //Xenusoida
	"39de3a656067a9491e19f1474899f409", //themallard
	"343806704c614ca3ea7cdc998612f6d8", //Funtik
	"e506ab868dbde12cd1ec2539609eb13d", //frontface
	"75bb89ad01bb780b30b05f460e71f41d", //FunnyMan
	"867c8f56e502449b177d97c6285598cf", //Genius
	"20fc780ef02401f7e30431fa2e8464eb", //Xinerki
	"4174cedcd4eea146dbefb37034b1f954", //Theglobalfive
	"1872d951994962b6e40053aeae0b13f2", //Kiwi
	"d2fd5d9df2ae83066ea086c597cf8845", //Forces
	"8568e7ee13cad430d8c8a1100f041af7", //McNasty
	"b9563ce9442d0b5f1f25eaee02e1392d", //Soap
	"b1f7320cfcd2e29eeae89b2d3ac2edbc", //VadZz
	"6c865311de3409781693682ed860b327", //Xenusoida brother
	"852ea45965c13b4924387b566067325b" //Hexaflexagon
};

const char * hwidsWO[] = {
	"39c61f8d20e9723440b9768fac5b9200", //HarrWe
	"f6d2f4ba63931f24b9c04622a5c952f5", //Hexaflexagon
	"a4e6fe07e82d86e00be7e1350556950f", //KiritoLife
	"af380ad6e22d82678ac474c9515bf449", //FunnyMan
	"c259ba19d1f64c9ff22a1a44bae43f43", //Jesus
	"f6d2f4ba63931f24b9c04622a5c952f5" //McNasty
};

std::string GetModuleDir()
{
	HMODULE hModule;
	char    cPath[MAX_PATH] = { 0 };
	GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)GetModuleDir, &hModule);

	GetModuleFileNameA(hModule, cPath, MAX_PATH);
	std::string path = cPath;
	return path.substr(0, path.find_last_of("\\/"));
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		std::string myHwid = getHWID();
		bool found = false;
		for (int i = 0; i < sizeof(hwids)/8; ++i)
		{
			if (!myHwid.compare(hwids[i]))
				found = true;
		}
		if (!found)
		{

			std::string myHwidWO = getHWIDClear();
			for (int i = 0; i < sizeof(hwidsWO)/8; ++i)
			{
				if (!myHwidWO.compare(hwidsWO[i]))
					found = true;
			}
			if (!found)
			{
				CGlobals::Get().debugInfo << std::endl << myHwid << std::endl << myHwidWO;
				MessageBoxA(NULL, CGlobals::Get().debugInfo.str().c_str(), "Debug info", MB_OK);
				return false;
			}
		}


		CGlobals::Get().dllModule = hModule;
		CGlobals::Get().orangePath = GetModuleDir();

		std::fstream isDev(CGlobals::Get().orangePath + "/orange.developer");
		if (isDev.good())
			CGlobals::Get().isDeveloper = true;
		isDev.close();

		//std::stringstream path;
		//path << CGlobals::Get().orangePath << "\\bin;" << std::getenv("PATH");

		//_putenv_s("PATH", path.str().c_str());
		my_ostream::SetLogFile(CGlobals::Get().orangePath + "/client.log");
		PreLoadPatches();
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		FreeConsole();
		break;
	}
	return TRUE;
}

