#pragma once
class CNetworkVehicle
{
	static std::vector<CNetworkVehicle *> Vehicles;

public:
	RakNet::RakNetGUID rnGUID;
	CNetworkPlayer* driver;				// vehicle driver
	Hash hashModel;						// vehicle model
	CVector3 vecPos;					// vehicle position
	CVector3 vecRot;					// vehicle rotation
	CVector3 vecMoveSpeed;				// vehicle move speed
	unsigned short int usHealth;		// vehicle health 
	bool bTaxiLights = false;			// vehicle taxi lights
	bool bSirenState = false;			// vehicle siren state
	bool bEngineStatus = false;			// vehicle engine status
	bool bEngineLocked = false;			// vehicle engine locked
	bool bTyresBulletproof = false;		// vehicle tyres bulletproof
	bool bLocked = false;				// vehicle locked
	float iDirtLevel;					// vehicle dirt level

	unsigned char Color1;
	unsigned char Color2;

	float fEngineHealth = 1000, fBodyHealth = 1000, fTankHealth = 1000;
	bool bDrivable = true;

	RakNetGUID driverGUID;
	bool hasDriver;

	static std::vector<CNetworkVehicle *> All();
	static void SendGlobal(RakNet::Packet * packet);
	static int Count();
	static CNetworkVehicle *GetByGUID(RakNetGUID);

	CNetworkVehicle(Hash model, float x, float y, float z, float heading);

	RakNetGUID GetGUID();

	void SetPosition(CVector3 position);
	void SetDriver(CNetworkPlayer* driver);
	void SetRotation(CVector3 rotation);
	void SetHealth(unsigned short health);
	std::vector<unsigned int> GetPassengers();
	int GetDriver();

	CVector3 GetPosition() { return vecPos; };
	CVector3 GetRotation() { return vecRot; };

	void SetVehicleData(const VehicleData& data);
	void GetVehicleData(VehicleData& data);

	~CNetworkVehicle();
};

