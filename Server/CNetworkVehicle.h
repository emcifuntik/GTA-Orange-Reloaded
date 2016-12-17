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
	bool bTaxiLights;					// vehicle taxi lights
	bool bSirenState;					// vehicle siren state
	float iDirtLevel;					// vehicle dirt level

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

	CVector3 GetPosition() { return vecPos; };

	void SetVehicleData(const VehicleData& data);
	void GetVehicleData(VehicleData& data);

	~CNetworkVehicle();
};

