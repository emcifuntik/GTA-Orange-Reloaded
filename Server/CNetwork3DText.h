#pragma once

enum TextAttachedTo
{
	NON_ATTACHED = 0,
	VEHICLE_ATTACHED,
	PLAYER_ATTACHED
};

class CNetwork3DText
{
	static std::vector<CNetwork3DText *> AllTexts;
	CVector3 vecPos;
	CVector3 vecOffset;
	int color;
	int outColor;
	float fontSize;
	std::string text;
	int playerid;
	TextAttachedTo attachmentType = TextAttachedTo::NON_ATTACHED;
	RakNet::RakNetGUID attachedTo;
public:
	RakNet::RakNetGUID rnGUID;
	CNetwork3DText(float x, float y, float z, int color, int outColor, std::string text, int playerid = -1, float oX = 0.f, float oY = 0.f, float oZ = 0.f, float fontSize = 16.f);
	void AttachToVehicle(CNetworkVehicle &vehicle, float oX = 0.f, float oY = 0.f, float oZ = 0.f);
	void AttachToPlayer(CNetworkPlayer &player, float oX = 0.f, float oY = 0.f, float oZ = 0.f);
	void SetText(std::string text);
	int GetPlayerID() { return playerid; };
	void Delete();
	~CNetwork3DText();
	static void SendGlobal(RakNet::Packet * packet);
	static CNetwork3DText * GetByGUID(RakNetGUID guid);
};

