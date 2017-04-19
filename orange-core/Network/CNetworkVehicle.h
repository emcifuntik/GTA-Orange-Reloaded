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
	bool				m_ShouldHasDriver = false;
	Ped					m_Driver;
	RakNetGUID			m_DriverGUID;

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
	float				m_EngineHealth = 1000,
		m_BodyHealth = 100,
		m_TankHealth = 1000;

	bool				m_Drivable = false;
	bool				m_Exploded = false;

	bool				m_Horn = false;
	bool				m_Siren = false;

	bool				m_Inited = false;
	bool				m_bVisible = false;

	int					m_Color1 = -1;
	int					m_Color2 = -1;

	static std::vector<CNetworkVehicle *> VehiclePool;
public:
	RakNet::RakNetGUID	m_GUID;
	CNetworkVehicle();

	void Init();

	void UpdateModel();

	bool HasTargetPosition() { return (m_interp.pos.ulFinishTime != 0); }
	bool HasTargetRotation() { return (m_interp.rot.ulFinishTime != 0); }

	void UpdateTargetPosition();
	void UpdateTargetRotation();
	void SetTargetPosition(const CVector3 & vecPosition);
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
	static void SetColours(RakNet::RakNetGUID GUID, int Color1, int Color2);
	static void Delete(RakNet::RakNetGUID GUID);

	static void Tick();

	~CNetworkVehicle();
};

