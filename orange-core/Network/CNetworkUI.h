#pragma once

struct screenInfo_
{
	bool shown = false;
	char* msg;
};

struct CMenuElement {
	std::string name;
	CVector2 pos;
	UCHAR type;
	std::string buffer;
	float min = 0, max = 1;
	float step = 0.05;
	double state = 0;
	std::function<void()> cb;
};

struct CMenu {
	std::string name;
	std::string subname;
	DWORD button;
	bool shown;
	unsigned char active;
	bool jactive = true;
	CVector2 pos;
	std::vector<CMenuElement*> children;
};

class CNetworkUI
{
	static CNetworkUI* Instance;
public:

	CNetworkUI();
	bool Render();
	void DXRender();
	bool SetScreenInfo(const char* msg);
	bool UnsetScreenInfo();
	bool SendNotification(char * msg);
	int AddMenu(CMenu * menu);
	bool Call(CMenu * menu);
	~CNetworkUI();
	static void ScriptKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow);
	static CNetworkUI* Get();
	static void Clear();

	static std::vector<CMenu*> menus;
};