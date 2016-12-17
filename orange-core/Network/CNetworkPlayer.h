#pragma once

class CNetworkPlayer: public CPedestrian
{
private:
	static std::vector<CNetworkPlayer *> PlayersPool;
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
	RakNet::RakNetGUID	m_GUID;
	std::string			m_Name;
	CVector3			m_vecMove;
	CVector3			m_vecAim;
	Hash				m_Model;
	bool				m_Spawned = false;
	bool				m_Ducking = false;
	bool				m_Jumping = false;
	bool				m_JustJumping = false;
	bool				m_TagVisible = true;
	bool				m_Aiming = false;
	bool				m_Shooting = false;
	bool				m_InVehicle = false;
	RakNetGUID			m_Vehicle;
	bool				m_Entering = false;

	bool				pedJustDead = false;
	float				m_MoveSpeed;
	float				lastMoveSpeed;
	int					updateTick = 0;
	int					lastTick = 0;
	int					tasksToIgnore = 0;
	DWORD				lastUpdate = 9999;
	unsigned short		m_Health = 200;
	std::queue<std::function<void()>> taskQueue;
	CNetworkPlayer();
public:
	CPed* pedHandler;
	static int ignoreTasks;
	static Hash hFutureModel;
	static std::vector<CNetworkPlayer*> All();
	static void DeleteByGUID(RakNet::RakNetGUID guid);
	static CNetworkPlayer * GetByGUID(RakNet::RakNetGUID GUID);
	static bool Exists(RakNet::RakNetGUID GUID);
	static CNetworkPlayer * GetByHandler(Entity handler);
	static void Clear();
	static void Tick();
	static void PreRender();
	static void Render();

	
	void UpdateLastTickTime();
	int GetTickTime();
	
	std::string GetName() { return m_Name; }
	void SetName(std::string Name) { m_Name = Name; }

	bool IsSpawned();
	void SetPosition(const CVector3 & vecPosition, bool bResetInterpolation);
	void SetRotation(const CVector3 & vecRotation, bool bResetInterpolation);
	void Spawn(const CVector3& vecPosition);

	void SetTargetPosition(const CVector3& vecPosition, unsigned long ulDelay);
	void SetTargetRotation(const CVector3& vecRotation, unsigned long ulDelay);
	void SetOnFootData(OnFootSyncData data, unsigned long ulDelay);

	bool HasTargetPosition() { return (m_interp.pos.ulFinishTime != 0); }
	bool HasTargetRotation() { return (m_interp.rot.ulFinishTime != 0); }

	void UpdateTargetPosition();
	void UpdateTargetRotation();

	void SetModel(Hash model);

	void RemoveTargetPosition();
	void RemoveTargetRotation();
	void ResetInterpolation();

	void SetMovementTask(RakNet::BitStream& bsIn);

	void Interpolate();

	void SetMoveToDirection(CVector3 vecPos, CVector3 vecMove, float iMoveSpeed);

	void SetMoveToDirectionAndAiming(CVector3 vecPos, CVector3 vecMove, CVector3 aimPos, float moveSpeed, bool shooting);

	void AssignTask(GTA::CTask * task);

	void BuildTasksQueue();

	void MakeTag();
	void DrawTag();

	~CNetworkPlayer()
	{
		PED::DELETE_PED(&Handle);
	}
};
