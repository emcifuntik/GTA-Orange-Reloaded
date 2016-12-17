#pragma once

typedef struct tagChatLine
{
	std::string sLineText;
	color_t structColor;
} ChatLine;

enum ScrollRoute
{
	SCROLL_UP = 1,
	SCROLL_DOWN
};

class CChat
{
	void AddLine(ChatLine line);
	CChat();

	static CChat *singleInstance;
	std::vector<ChatLine> vChatLines;
	bool bOpened = false;
	bool bJustOpened = false;
	bool bEnabled = true;
	bool bScrollBottom = false;
	int iOffset = 0;
	std::wstring wsCurrentMessage;
	std::mutex access;

	unsigned int uiCarretPos = 0;

	const unsigned cuChatHistorySize = 100;
	int(*_commandProcess)(std::string command) = nullptr;


public:
	static CChat* Get();
	void RegisterCommandProcessor(int(*callback)(std::string));
	bool Opened() { return bOpened; }
	void Render();
	void Input();
	void AddChatMessage(std::string text, unsigned char red = 255, unsigned char green = 255, unsigned char blue = 255, unsigned char alpha = 255);
	void AddChatMessage(std::string text, unsigned int rgba);
	void AddChatMessage(std::string text, color_t color);
	void Clear();
	void Toggle();
	void Open();
	void Close();
	static void ScriptKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow);
	~CChat();
};