#pragma once

enum TextAttachedTo
{
	NON_ATTACHED = 0,
	VEHICLE_ATTACHED,
	PLAYER_ATTACHED
};

class CNetwork3DText
{
	static std::vector<CNetwork3DText *> TextPool;
	CVector3 vecPos;
	CVector3 vecOffset;
	int color;
	int outColor;
	std::string text;
	TextAttachedTo attachmentType = TextAttachedTo::NON_ATTACHED;
	RakNet::RakNetGUID attachedTo;

	CVector3 scrPos;
	bool IsVisible = false;
	float fontSize = 20.f;
	float rawFontSize = 20.f;
public:
	RakNet::RakNetGUID	m_GUID;
	CNetwork3DText(RakNetGUID guid, float x, float y, float z, int color, int outColor, std::string text, TextAttachedTo attachmentType, RakNet::RakNetGUID attachedTo);

	void SetColor(int color);
	void SetOutColor(int outColor);
	void SetText(std::string text);
	void SetFontSize(float size);
	void AttachToPlayer(RakNet::RakNetGUID GUID, float oX, float oY, float oZ);
	void AttachToVehicle(RakNet::RakNetGUID GUID, float oX, float oY, float oZ);

	static void Render();
	static void PreRender();
	static CNetwork3DText * GetByGUID(RakNet::RakNetGUID GUID);
	static void DeleteByGUID(RakNet::RakNetGUID guid);

	~CNetwork3DText();
	static void Clear();
};

