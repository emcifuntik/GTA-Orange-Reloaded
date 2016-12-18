#pragma once
class CNetworkObject
{
	static std::vector<CNetworkObject *> Objects;

public:
	RakNet::RakNetGUID rnGUID;
	Hash hashModel;						// Object model
	CVector3 vecPos;					// Object position
	CVector3 vecRot;					// Object rotation
	//CVector3 vecMoveSpeed;				// Object move speed
	unsigned short int usHealth;		// Object health

	static std::vector<CNetworkObject *> All();
	static void SendGlobal(RakNet::Packet * packet);
	static int Count();
	static CNetworkObject *GetByGUID(RakNetGUID);

	CNetworkObject(Hash model, float x, float y, float z, float pitch, float roll, float yaw);

	RakNetGUID GetGUID();

	void SetPosition(CVector3 position);
	void SetRotation(CVector3 rotation);
	void SetHealth(unsigned short health);

	CVector3 GetPosition() { return vecPos; };

	void GetObjectData(ObjectData& data);

	~CNetworkObject();
};

