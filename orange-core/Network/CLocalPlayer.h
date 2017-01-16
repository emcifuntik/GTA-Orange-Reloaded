#pragma once

class CPedestrian;

class CLocalPlayer: public CPedestrian
{
	static CLocalPlayer* Instance;
	CLocalPlayer();
public:
	bool Spawned = false;
	bool updateTasks = false;
	CVector3 *aimPosition = nullptr;
	Hash newModel = 0;
	bool _togopassenger = false;
	char FutureSeat = 0;
	CNetworkVehicle *FutureVeh = nullptr;

	short lastSendSeat;

	static CLocalPlayer *Get();

	void ChangeModel(Hash model);
	void Connect();
	void Tick();
	void GetOnFootSync(OnFootSyncData& onfoot);
	void GetVehicleSync(VehicleData & vehsync);
	void SendOnFootData();
	short GetSeat();
	void GoPassenger();
	void SendTasks();
	void SetMoney(int money);
	~CLocalPlayer();
	void Spawn();
};

