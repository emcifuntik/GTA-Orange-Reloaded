#pragma once
class CNetworkMarker
{
	static std::vector<CNetworkMarker *> GlobalMarkers;
	static std::vector<CNetworkMarker *> PlayerMarkers;
	CVector3 vecPos;
	float height;
	float radius;
	int playerid;
public:
	RakNet::RakNetGUID rnGUID;
	static std::vector<CNetworkMarker *> AllGlobal();
	CNetworkMarker(float x, float y, float z, float height, float radius, int playerid);

	//void SetScale(float _scale);

	void Delete();

	~CNetworkMarker();
	static void SendGlobal(RakNet::Packet * packet);
	static CNetworkMarker * GetByGUID(RakNetGUID guid);
	static void Tick();
};
