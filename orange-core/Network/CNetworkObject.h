#pragma once

class CNetworkObject : public CEntity
{
	/*struct
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

	float				m_MoveSpeed;
	float				lastMoveSpeed;
	int					updateTick = 0;
	DWORD				lastTick = 0;
	DWORD				lastUpdate = 9999;

	unsigned short		m_Health;*/
	CVector3			m_vecPos;
	CVector3			m_vecRot;
	Hash				m_Model;
	Hash				m_futureModel;
	bool				m_bVisible = false;

	static std::vector<CNetworkObject *> ObjectPool;
public:
	RakNet::RakNetGUID	m_GUID;
	CNetworkObject(ObjectData data);

	static int Count;

	void UpdateModel();

	//bool HasTargetPosition() { return (m_interp.pos.ulFinishTime != 0); }
	//bool HasTargetRotation() { return (m_interp.rot.ulFinishTime != 0); }

	//void UpdateTargetPosition();
	//void UpdateTargetRotation();
	//void SetTargetPosition(const CVector3 & vecPosition, unsigned long ulDelay);
	//void SetTargetRotation(const CVector3 & vecRotation, unsigned long ulDelay);
	//void Interpolate();

	//void BuildTasksQueue();

	//void UpdateLastTickTime();
	//int GetTickTime();

	//void SetObjectData(ObjectData data, unsigned long ulDelay);

	static std::vector<CNetworkObject*> All();
	static void Clear();
	static CNetworkObject * GetByHandle(Object veh);
	static CNetworkObject * GetByGUID(RakNet::RakNetGUID GUID);

	static void Tick();

	static void Delete(RakNet::RakNetGUID GUID);

	~CNetworkObject();
};

