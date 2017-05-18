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
	"6c865311de3409781693682ed860b327", //Xenusoida brother
	"39de3a656067a9491e19f1474899f409", //themallard
	"343806704c614ca3ea7cdc998612f6d8", //Funtik
	"75bb89ad01bb780b30b05f460e71f41d", //FunnyMan
	"867c8f56e502449b177d97c6285598cf", //Genius
	"20fc780ef02401f7e30431fa2e8464eb", //Xinerki
	"4174cedcd4eea146dbefb37034b1f954", //Theglobalfive
	"1872d951994962b6e40053aeae0b13f2", //Kiwi
	"d2fd5d9df2ae83066ea086c597cf8845", //Forces
	"8568e7ee13cad430d8c8a1100f041af7", //McNasty
	"b9563ce9442d0b5f1f25eaee02e1392d", //Soap
	"b1f7320cfcd2e29eeae89b2d3ac2edbc", //VadZz
	"852ea45965c13b4924387b566067325b", //Hexaflexagon
	"ddbeb8b80ef352cb76bf3b5dc4fc2e19"  //0x452

};

const char * hwidsWO[] = { 
	"bfecea4018283f4b7657966b78450abf", //HarrWe
	"af380ad6e22d82678ac474c9515bf449", //FunnyMan
	"c259ba19d1f64c9ff22a1a44bae43f43", //Jesus
	"f6d2f4ba63931f24b9c04622a5c952f5", //McNasty
	"f6d2f4ba63931f24b9c04622a5c952f5", //Hexaflexagon
	"a4e6fe07e82d86e00be7e1350556950f", //KiritoLife
	"2d56b12455537c45d997ea8043cacae0", //value.H12
	"4657822e7dca0316148ec2b3e10b5321", //SWF*
	"67854c47df2fd66c96b124ab4410feb4", //RZX
	"f6d2f4ba63931f24b9c04622a5c952f5", //McNasty
	"5ab5d5e44bc5ff9132c397cf3c4690dd", //Tracer
	"964ef44881cda92d7a78a74e008d5bd8", //Dima_Runner
	"ddbeb8b80ef352cb76bf3b5dc4fc2e19", //0x452
	"780d05a3fe2998da790f583a92dd2057", //HarrWe
	"f6d2f4ba63931f24b9c04622a5c952f5", //Hexaflexagon (PC)
	"125cf7e9b4f41c1653f156691ee8c67b", //Hexaflexagon (Laptop)
	"a4e6fe07e82d86e00be7e1350556950f", //KiritoLife
	"370553d993e366678b3601d352dacdd4", //Xenusoid brother
	"836e4e6671721f6f307f1768e508253e", //S4MPLE
	"6b42fa1249dbe75a163833eb96d2939b", //DENO
	"e38da14a8180f22a56db9661d345e125", //frontface

	// Server-beta

	"480aae7e7d09a36acb9ced72a7a78e77", //ModernB
	"e92c9d537addd60ee0bf01adc70116ca", //Crimson
	"30062ff0dc4106c55941e136cd433655", //Nero
	"329771ca21768ac80ae1271727f316d9", //Laurynu
	"87c6e3467af57dcc828dd8d01b6dee79", //uPriZz
	"d0e61da8848fbefbab7f5e7118b98fd0", //Max053
	"a5c475b32369fde6e7d9a9e364cde402", //RacheT
	"f772bdddea07204fe82b55718d884c37", //kalle
	//"9f1ae8c2ee9f1866fd228373f6fab0b4", //Darkness.
	"d34fcb8e269e3f24707fdd5307e7a2f1", //Darkness.
	"c7aa35f139be7c6d700c41919492add5", //Aldona
	"389bbfdbf187719b4c83ad2d2dd15c20", //Nacho
	"274f9933c9ccaf3938b4e211b309e6fd", //kentas
	"2d89ee2f3e308dc7b889450a180af534", //Suppeer
	"70d3c8de0a7875dd48149266f9e57518", //LastSurvivorYT
	"cb0cbf2a6293de9c5ee0e4bd03cb9b19", //LastSurvivorYT (PC)
	"c7144590032300c9bd5afbaee9b37857", //Derk
	"ed896cabc875aca414c66bbbdaf50f8b", //Vektor
	"f76ae3a70603bc0082baf9177559242f", //Jonniboy
	"557614ad028176a27c4dd5038b8b1eac", //SirRamzi
	"85da2868ec755558e28459d2f38d6968", //Scott
	"d7eed02e9023f12afab3d1ce5abdfae8", //xiadosw
	"a000afd0f8e3b5d222d06d9fe3935b06", //VFRZ
	"ff0413fb80e1ab9f392071e2bad7f29d", //Appi
	"0561034664916301aff422693153b675", //Goomba
	"e62fc667d7bb75e4972d0eb28df496b1", //Scheffi
	"1be5c5ce6eb1bf812a73dadfd8cd8444", //TheBloodyScreen
	"b9290a8467b586b9cbf91f8f588a5cfe", //Mars
	//"aaf9a453dc5b5d125001b84c6dc9de9e", //TREY
	"cca72dcb2dd9dabebcbe7454f3ebf3d7", //Derk friend
	"8d880899d9d2a48689b0513f5261fe85", //Mafiatoss
	"e18b236a388264eb0d9aeb78c19c30d2", //Metmonk
	"e331c9ef2788a931c2abfd0f2cef564c", //ChristianWood
	"742514290db11a0fef8206211a1092f0", //vaffanculo
	"f946a5a452a79f52b323371a9be5b09e", //ddoommbbaass
	"062a3a68664204673cfe7c15cb6c207d", //GAMELASTER
	"b9476e462d3502468bb1fa1d8ede63eb", //TReY
	"7f209a5692c4476408904a0a17b87a27", //TReY second
	"f3bbcbc509ca9dff1f68f99dcdc67948", //Evgeny
	"5408d1b001d806ea472d87d0b6202fa8"  //Gazreyn
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
				std::ofstream dfile(GetModuleDir() + "\\debug.log");
				dfile << myHwidWO;
				dfile.close();
				//MessageBoxA(NULL, CGlobals::Get().debugInfo.str().c_str(), "Debug info", MB_OK);
				TerminateProcess(NULL, 0);
				return false;
			}
			//return false;
		}

		CGlobals::Get().dllModule = hModule;
		CGlobals::Get().orangePath = GetModuleDir();

		std::fstream isDev(CGlobals::Get().orangePath + "/orange.developer");
		if (isDev.good())
			CGlobals::Get().isDeveloper = true;
		isDev.close();

		if (CGlobals::Get().isDeveloper)
		{
			strcpy_s(CGlobals::Get().serverIP, CConfig::Get()->sIP.c_str());
			CGlobals::Get().serverPort = CConfig::Get()->uiPort;
		}

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

