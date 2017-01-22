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
	std::function<void()> cb;
};

struct CMenu {
	std::string name;
	bool shown;
	unsigned char active;
	CVector2 pos;
	std::vector<CMenuElement*> children;
};

class CNetworkUI
{
	static CNetworkUI* Instance;
	static std::vector<CMenu*> menus;
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
};