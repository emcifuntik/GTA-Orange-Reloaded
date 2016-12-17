#pragma once

void __fastcall eventHook(GTA::CTask* task);

class CPedestrian;

class CLocalPlayer: public CPedestrian
{
	static CLocalPlayer* Instance;
	CLocalPlayer();
public:
	bool updateTasks = false;
	CVector3 *aimPosition = nullptr;
	Hash newModel = 0;

	static CLocalPlayer *Get();

	void ChangeModel(Hash model);
	void Connect();
	void Tick();
	void GetOnFootSync(OnFootSyncData& onfoot);
	void GetVehicleSync(VehicleData & vehsync);
	void SendOnFootData();
	void SendTasks();
	void SetMoney(int money);
	~CLocalPlayer();
};

