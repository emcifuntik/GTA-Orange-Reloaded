#pragma once

void __fastcall eventHook(GTA::CTask* task);

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
	Vehicle FutureVeh = 0;

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
	void GetAimPosition(CVector3 & aimPos);
	void Spawn();
};

