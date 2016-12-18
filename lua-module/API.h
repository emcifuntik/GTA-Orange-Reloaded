#pragma once
#include <string>
#include "../../Shared/CVector3.h"

enum {
	M_STRING,
	M_INT,
	M_BOOL,
	M_DOUBLE,
	M_ULONG
};

class MValue
{
public:
	MValue(const char* val) {
		string_val = _strdup(val);
		type = M_STRING;
	};
	MValue(int val) {
		int_val = val;
		type = M_INT;
	};
	MValue(bool val) {
		bool_val = val;
		type = M_BOOL;
	}
	MValue(double val) {
		double_val = val;
		type = M_DOUBLE;
	};
	MValue(unsigned long val) {
		ulong_val = val;
		type = M_ULONG;
	};

	char* getString() { if (type == M_STRING) return string_val; return NULL; };
	int getInt() { if (type == M_INT) return int_val; return 0; };
	bool getBool() { if (type == M_BOOL) return bool_val; return false; };
	double getDouble() { if (type == M_DOUBLE) return double_val; return 0; };
	unsigned long getULong() { if (type == M_ULONG) return ulong_val; return 0; };

	bool isString() { return type == M_STRING; };
	bool isInt() { return type == M_INT; };
	bool isBool() { return type == M_BOOL; };
	bool isDouble() { return type == M_DOUBLE; };
	bool isULong() { return type == M_ULONG; };

	char type;
private:
	char* string_val;
	int int_val;
	bool bool_val;
	double double_val;
	unsigned long ulong_val;
};

class APIBase {
public:
	//Player
	virtual bool SetPlayerPosition(long playerid, float x, float y, float z) = 0;
	virtual CVector3 GetPlayerPosition(long playerid) = 0;
	virtual bool IsPlayerInRange(long playerid, float x, float y, float z, float range) = 0;
	virtual bool SetPlayerHeading(long playerid, float angle) = 0;
	virtual bool GivePlayerWeapon(long playerid, long weapon, long ammo) = 0;
	virtual bool GivePlayerAmmo(long playerid, long weapon, long ammo) = 0;
	virtual bool GivePlayerMoney(long playerid, long money) = 0;
	virtual bool SetPlayerMoney(long playerid, long money) = 0;
	virtual bool ResetPlayerMoney(long playerid) = 0;
	virtual size_t GetPlayerMoney(long playerid) = 0;
	virtual bool SetPlayerModel(long playerid, long model) = 0;
	virtual long GetPlayerModel(long playerid) = 0;
	virtual bool SetPlayerName(long playerid, const char * name) = 0;
	virtual std::string GetPlayerName(long playerid) = 0;
	virtual bool SetPlayerHealth(long playerid, float health) = 0;
	virtual float GetPlayerHealth(long playerid) = 0;
	virtual bool SetPlayerArmour(long playerid, float armour) = 0;
	virtual float GetPlayerArmour(long playerid) = 0;
	virtual bool SetPlayerColor(long playerid, unsigned int color) = 0;
	virtual unsigned int GetPlayerColor(long playerid) = 0;
	virtual void BroadcastClientMessage(const char * message, unsigned int color) = 0;
	virtual bool SendClientMessage(long playerid, const char * message, unsigned int color) = 0;
	//World
	virtual void Print(const char * message) = 0;
	virtual long Hash(const char * str) = 0;

	//TODO
	//virtual bool PlayerExists(long playerid) = 0;
	//virtual bool VehicleExists(long playerid) = 0;
	virtual unsigned long CreateVehicle(long hash, float x, float y, float z, float heading) = 0;
	virtual bool SetVehiclePosition(int vehicleid, float x, float y, float z) = 0;
	virtual CVector3 GetVehiclePosition(int vehicleid) = 0;

	virtual unsigned long CreateObject(long model, float x, float y, float z, float pitch, float yaw, float roll) = 0;

	virtual bool CreatePickup(int type, float x, float y, float z, float scale) = 0;
	virtual unsigned long CreateBlipForAll(float x, float y, float z, float scale, int color, int sprite) = 0;
	virtual unsigned long CreateBlipForPlayer(long playerid, float x, float y, float z, float scale, int color, int sprite) = 0;
	virtual void DeleteBlip(unsigned long guid) = 0;
	virtual void SetBlipColor(unsigned long guid, int color) = 0;
	virtual void SetBlipScale(unsigned long guid, float scale) = 0;
	virtual void SetBlipRoute(unsigned long guid, bool route) = 0;

	virtual unsigned long CreateMarkerForAll(float x, float y, float z, float height, float radius) = 0;
	virtual unsigned long CreateMarkerForPlayer(long playerid, float x, float y, float z, float height, float radius) = 0;
	virtual void DeleteMarker(unsigned long guid) = 0;

	virtual bool SetInfoMsg(long playerid, const char * msg) = 0;
	virtual bool UnsetInfoMsg(long playerid) = 0;

	virtual bool Print3DText(const char * text, float x, float y, float z, float scale) = 0;
};

class API:
	public APIBase 
{
public:
	static API * instance;
	static void Set(API * api){instance = api;}
	static API& Get(){return *instance;}
};
