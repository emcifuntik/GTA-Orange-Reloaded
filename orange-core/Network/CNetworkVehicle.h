#pragma once

class CNetworkVehicle : public CVehicle
{
	struct
	{
		struct
		{
			CVector3      vecStart;
			CVector3      vecTarget;
			CVector3      vecError;
			float         fLastAlpha;
			unsigned long ulStartTime;
			unsigned long ulFinishTime;
		} pos;
		struct
		{
			float         fStart;
			float         fTarget;
			float         fError;
			float         fLastAlpha;
			unsigned long ulStartTime;
			unsigned long ulFinishTime;
		} heading;
		struct
		{
			CVector3      vecStart;
			CVector3      vecTarget;
			CVector3      vecError;
			float         fLastAlpha;
			unsigned long ulStartTime;
			unsigned long ulFinishTime;
		} rot;
	}					m_interp;

	CVector3			m_vecMove;
	Hash				m_Model;
	Hash				m_futureModel;
	bool				m_hasDriver;
	Ped					m_Driver;

	float				m_MoveSpeed;
	float				lastMoveSpeed;
	float				m_steering;
	int					updateTick = 0;
	DWORD				lastTick = 0;
	int					tasksToIgnore = 0;
	DWORD				lastUpdate = 9999;

	bool				m_Burnout;
	float				m_RPM;

	unsigned short		m_Health;

	static std::vector<CNetworkVehicle *> VehiclePool;
public:
	RakNet::RakNetGUID	m_GUID;
	CNetworkVehicle();

	void UpdateModel();

	bool HasTargetPosition() { return (m_interp.pos.ulFinishTime != 0); }
	bool HasTargetRotation() { return (m_interp.rot.ulFinishTime != 0); }

	void UpdateTargetPosition();
	void UpdateTargetRotation();
	void SetTargetPosition(const CVector3 & vecPosition, unsigned long ulDelay);
	void SetTargetRotation(const CVector3 & vecRotation, unsigned long ulDelay);
	void Interpolate();

	void BuildTasksQueue();

	void UpdateLastTickTime();
	int GetTickTime();

	void SetVehicleData(VehicleData data, unsigned long ulDelay);

	static std::vector<CNetworkVehicle*> All();
	static void Clear();
	static CNetworkVehicle * GetByHandle(Vehicle veh);
	static CNetworkVehicle * GetByGUID(RakNet::RakNetGUID GUID);

	static void Tick();

	~CNetworkVehicle();
};

