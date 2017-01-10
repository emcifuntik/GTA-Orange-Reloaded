#pragma once
class CNetworkPlayer
{
	// Static
	static std::vector<CNetworkPlayer*> _players;
	static void AddPlayer(CNetworkPlayer*);
	
	// Non-static
	unsigned int uiID;
	RakNet::RakNetGUID rnGUID;
	Hash hModel;
	bool bDead = false;
	bool bDucking = false;
	bool bBlipVisible = true;
	float fHeading;
	float fForwardPotential;
	float fStrafePotential;
	float fRotationPotential;
	DWORD dwMovementFlags;
	unsigned long ulWeapon;
	unsigned int uAmmo;
	unsigned short usHealth = 200;
	unsigned short usArmour = 0;
	bool bJumping = false;
	bool bAiming = false;
	bool bShooting = false;
	bool bEnteringVeh = false;
	CVector3 vecPosition;
	CVector3 vecRotation;
	CVector3 vecMoveSpeed;
	CVector3 vecAim;
	std::string sName;
	size_t uMoney;
	float fTagDrawDistance = 50.f;
	unsigned long ulLastVehUpdate = 0;
	color_t colColor;
public:
	CNetworkPlayer(RakNet::RakNetGUID GUID);
	~CNetworkPlayer();

	bool bInVehicle = false;
	RakNetGUID vehicle;
	short sSeat;

	static void Each(void(*func)(CNetworkPlayer*));
	static CNetworkPlayer *GetByGUID(RakNet::RakNetGUID GUID);
	static CNetworkPlayer *GetByID(UINT playerID);
	static void Tick();
	static UINT Count();
	static std::vector<CNetworkPlayer *> CNetworkPlayer::All();
	static void Remove(int playerid);

	unsigned int GetID() { return uiID; }
	RakNet::RakNetGUID GetGUID() { return rnGUID; }
	void SetOnFootData(const OnFootSyncData& data);
	void GetOnFootData(OnFootSyncData& data);
	void HideBlip() { bBlipVisible = false; }
	void ShowBlip() { bBlipVisible = true; }
	bool IsBlipVisible() { return bBlipVisible; }
	void SetName(std::string playername) { sName = playername; }
	std::string GetName() { return sName; }
	void SetMoney(size_t money) { uMoney = money; }
	void GiveMoney(size_t money) { uMoney += money; }
	size_t GetMoney() { return uMoney; }
	void SetPosition(const CVector3& position);
	void SetCoords(const CVector3 & position);
	void GetPosition(CVector3& position) { position = vecPosition; };
	CVector3 GetPosition() { return vecPosition; };
	void SetHeading(float heading) { fHeading = heading; }
	void GiveWeapon(unsigned int weaponHash, unsigned int ammo);
	void GiveAmmo(unsigned int weaponHash, unsigned int ammo);
	void SetModel(unsigned int model);
	void SetHealth(float health);
	void SetArmour(float armour);
	void SetColor(unsigned int color);
	color_t GetColor() { return colColor; }
	float GetHealth() { return (float)usHealth; }
	float GetArmour() { return (float)usArmour; }
	unsigned int GetModel() { return hModel; }
	float GetHeading() { return fHeading; }
	float GetTagDrawDistance() { return fTagDrawDistance; }
	void SendTextMessage(const char * message, unsigned int color);
};

