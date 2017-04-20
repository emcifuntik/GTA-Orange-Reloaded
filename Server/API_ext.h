#pragma once
#include <string>
#include "API.h"

class API: public APIBase
{
	static API * instance;
public:
	void LoadClientScript(std::string name, char * buffer, size_t size);
	void ClientEvent(const char * name, std::vector<MValue> args, long playerid);
	void KickPlayer(long playerid);
	//Player
	bool SetPlayerPosition(long playerid, float x, float y, float z);
	CVector3 GetPlayerPosition(long playerid);
	bool IsPlayerInRange(long playerid, float x, float y, float z, float range);
	bool SetPlayerHeading(long playerid, float angle);
	float GetPlayerHeading(long playerid);
	bool RemovePlayerWeapons(long playerid);
	bool GivePlayerWeapon(long playerid, long weapon, long ammo);
	bool GivePlayerAmmo(long playerid, long weapon, long ammo);
	bool GivePlayerMoney(long playerid, long money);
	bool SetPlayerMoney(long playerid, long money);
	bool ResetPlayerMoney(long playerid);
	size_t GetPlayerMoney(long playerid);
	bool SetPlayerModel(long playerid, long model);
	long GetPlayerModel(long playerid);
	bool SetPlayerName(long playerid, const char * name);
	std::string GetPlayerName(long playerid);
	bool SetPlayerHealth(long playerid, float health);
	float GetPlayerHealth(long playerid);
	bool SetPlayerArmour(long playerid, float armour);
	float GetPlayerArmour(long playerid);
	bool SetPlayerColor(long playerid, unsigned int color);
	unsigned int GetPlayerColor(long playerid);
	void BroadcastClientMessage(const char * message, unsigned int color);
	bool SendClientMessage(long playerid, const char * message, unsigned int color);
	bool SetPlayerIntoVehicle(long playerid, unsigned long vehicle, char seat);
	void DisablePlayerHud(long playerid, bool toggle);
	unsigned long GetPlayerGUID(long playerid);

	//Vehicle
	unsigned long CreateVehicle(long hash, float x, float y, float z, float heading);
	bool SetVehiclePosition(unsigned long guid, float x, float y, float z);
	CVector3 GetVehiclePosition(unsigned long guid);
	bool SetVehicleColours(unsigned long guid, int pColor, int sColor);
	unsigned long GetVehicleDriver(unsigned long guid);
	bool DeleteVehicle(unsigned long guid);

	bool CreatePickup(int type, float x, float y, float z, float scale);

	unsigned long CreateBlipForAll(std::string name, float x, float y, float z, float scale, int color, int sprite);
	unsigned long CreateBlipForPlayer(long playerid, std::string name, float x, float y, float z, float scale, int color, int sprite);
	void DeleteBlip(unsigned long guid);
	void SetBlipColor(unsigned long guid, int color);
	void SetBlipScale(unsigned long guid, float scale);
	void SetBlipRoute(unsigned long guid, bool route);
	void SetBlipSprite(unsigned long guid, int sprite);
	void SetBlipName(unsigned long guid, std::string name);
	void SetBlipAsShortRange(unsigned long guid, bool _short);
	void AttachBlipToPlayer(unsigned long _guid, long player);
	void AttachBlipToVehicle(unsigned long _guid, unsigned long vehicle);

	unsigned long CreateMarkerForAll(float x, float y, float z, float height, float radius);
	unsigned long CreateMarkerForPlayer(long playerid, float x, float y, float z, float height, float radius);
	void DeleteMarker(unsigned long guid);

	unsigned long CreateObject(long model, float x, float y, float z, float pitch, float yaw, float roll);
	bool DeleteObject(unsigned long guid);

	bool SendNotification(long playerid, const char * msg);
	bool SetInfoMsg(long playerid, const char * msg);
	bool UnsetInfoMsg(long playerid);

	//3DTexts
	unsigned long Create3DText(const char * text, float x, float y, float z, int color, int outColor, float fontSize);
	unsigned long Create3DTextForPlayer(unsigned long player, const char * text, float x, float y, float z, int color, int outColor);
	bool Attach3DTextToVehicle(unsigned long textId, unsigned long vehicle, float oX, float oY, float oZ);
	bool Attach3DTextToPlayer(unsigned long textId, unsigned long player, float oX, float oY, float oZ);
	bool Set3DTextContent(unsigned long textId, const char * text);
	bool Delete3DText(unsigned long textId);

	//World
	void Print(const char * message);
	long Hash(const char * str);

	static API * Get()
	{
		if (!instance)
			instance = new API();
		return instance;
	}
};
