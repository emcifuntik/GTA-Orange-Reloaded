#pragma once
#include <string>
#include "CVector3.h"
#include <memory>
#ifndef ULONG
#define ULONG unsigned long
#endif // ULONG

enum {
	M_STRING,
	M_INT,
	M_BOOL,
	M_DOUBLE,
	M_ULONG,
	M_ARRAY
};

class MValue;
struct MArray {
	std::map<int, MValue*> ikeys;
	std::map<std::string, MValue*> skeys;
};

class MValue
{
public:
	static MValue CreateMArray() {
		MValue val;
		val.type = M_ARRAY;
		val._val = new MArray;
		val.counter = new int(1);
		return val;
	};
	MValue() { type = -1; counter = new int(1); };
	MValue(const MValue &val)
	{
		std::stringstream ss;
		type = val.type;
		_val = val._val;
		counter = val.counter;
		(*counter)++;
	};
	MValue(const char* val) {
		_val = strdup(val);
		type = M_STRING;
		counter = new int(1);
	};
	MValue(int const &val) {
		_val = new int(val);
		type = M_INT;
		counter = new int(1);
	};
	MValue(bool const &val) {
		_val = new bool(val);
		type = M_BOOL;
		counter = new int(1);
	}
	MValue(double const &val) {
		_val = new double(val);
		type = M_DOUBLE;
		counter = new int(1);
	};
	MValue(unsigned long const &val) {
		_val = new unsigned long(val);
		type = M_ULONG;
		counter = new int(1);
	};
	~MValue()
	{
		(*counter)--;
		if ((*counter) == 0)
		{
			switch (type)
			{
			case M_STRING:
				//std::cout << "free: " << (char*)_val << std::endl;
				free(_val);
				break;
			case M_INT:
				delete (int*)_val;
				break;
			case M_BOOL:
				delete (bool*)_val;
				break;
			case M_DOUBLE:
				delete (double*)_val;
				break;
			case M_ULONG:
				delete (ULONG*)_val;
				break;
			case M_ARRAY:
			{
				MArray *val = (MArray*)_val;
				val->ikeys.erase(val->ikeys.begin(), val->ikeys.end());
				val->skeys.erase(val->skeys.begin(), val->skeys.end());
				delete (MArray*)_val;
				break;
			}
			default:
				break;
			}
		}
	}

	char* getString() { if (type == M_STRING) return (char*)_val; return NULL; };
	int getInt() { if (type == M_INT) return *(int*)_val; return 0; };
	bool getBool() { if (type == M_BOOL) return *(bool*)_val; return false; };
	double getDouble() { if (type == M_DOUBLE) return *(double*)_val; return 0; };
	unsigned long getULong() { if (type == M_ULONG) return *(unsigned long*)_val; return 0; };
	MArray getArray() { if (type == M_ARRAY) return *(MArray*)_val; };

	bool isString() { return type == M_STRING; };
	bool isInt() { return type == M_INT; };
	bool isBool() { return type == M_BOOL; };
	bool isDouble() { return type == M_DOUBLE; };
	bool isULong() { return type == M_ULONG; };
	bool isArray() { return type == M_ARRAY; };

	void push(MValue key, MValue val) {
		if (type == M_ARRAY)
		{
			MArray *arr = (MArray*)_val;
			switch (key.type)
			{
			case M_INT:
			{
				if (arr->ikeys[key.getInt()]) delete (arr->ikeys[key.getInt()]);
				arr->ikeys[key.getInt()] = new MValue(val);
				break;
				break;
			}
			case M_STRING:
			{
				if (arr->skeys[key.getString()]) delete (arr->skeys[key.getString()]);
				arr->skeys[key.getString()] = new MValue(val);
				break;
			}
			default:
				break;
			}
		}
	};
	MValue get(MValue key) {
		if (type == M_ARRAY)
		{
			MArray *arr = (MArray*)_val;
			switch (key.type)
			{
			case M_INT:
			{
				if (arr->ikeys[key.getInt()])
					return arr->ikeys[key.getInt()];
			}
			case M_STRING:
			{
				if (arr->skeys[key.getString()])
					return arr->skeys[key.getString()];
			}
			default:
				return MValue(0);
			}
		}
	};

	char type;
	void* _val;
	int* counter;
};

class APIBase {
public:
	virtual void LoadClientScript(std::string name, char* buffer, size_t size) = 0;
	virtual void ClientEvent(const char * name, std::vector<MValue> args, long playerid) = 0;
	//Player
	virtual void KickPlayer(long playerid) = 0;
	virtual bool SetPlayerPosition(long playerid, float x, float y, float z) = 0;
	virtual CVector3 GetPlayerPosition(long playerid) = 0;
	virtual bool IsPlayerInRange(long playerid, float x, float y, float z, float range) = 0;
	virtual bool SetPlayerHeading(long playerid, float angle) = 0;
	virtual float GetPlayerHeading(long playerid) = 0;
	virtual bool RemovePlayerWeapons(long playerid) = 0;
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
	virtual bool SetPlayerIntoVehicle(long playerid, unsigned long vehicle, char seat) = 0;
	virtual void DisablePlayerHud(long playerid, bool toggle) = 0;
	virtual unsigned long GetPlayerGUID(long playerid) = 0;

	//World
	virtual void Print(const char * message) = 0;
	virtual long Hash(const char * str) = 0;

	//TODO
	//virtual bool PlayerExists(long playerid) = 0;
	//virtual bool VehicleExists(long playerid) = 0;
	virtual unsigned long CreateVehicle(long hash, float x, float y, float z, float heading) = 0;
	virtual bool DeleteVehicle(unsigned long guid) = 0;
	virtual bool SetVehiclePosition(unsigned long guid, float x, float y, float z) = 0;
	virtual CVector3 GetVehiclePosition(unsigned long guid) = 0;
	virtual bool SetVehicleRotation(unsigned long guid, float rx, float ry, float rz) = 0;
	virtual CVector3 GetVehicleRotation(unsigned long guid) = 0;
	virtual bool SetVehicleColours(unsigned long guid, int Color1, int Color2) = 0;
	virtual bool GetVehicleColours(unsigned long guid, int *Color1, int *Color2) = 0;
	virtual bool SetVehicleTyresBulletproof(unsigned long guid, bool bulletproof) = 0;
	virtual bool GetVehicleTyresBulletproof(unsigned long guid) = 0;
	virtual bool SetVehicleCustomPrimaryColor(unsigned long guid, int rColor, int gColor, int bColor) = 0; //Not implemented
	virtual bool GetVehicleCustomPrimaryColor(unsigned long guid, int *rColor, int *gColor, int *bColor) = 0; //Not implemented
	virtual bool SetVehicleCustomSecondaryColor(unsigned long guid, int rColor, int gColor, int bColor) = 0; //Not implemented
	virtual bool GetVehicleCustomSecondaryColor(unsigned long guid, int *rColor, int *gColor, int *bColor) = 0; //Not implemented
	virtual bool SetVehicleEngineStatus(unsigned long guid, bool status, bool locked) = 0;
	virtual bool GetVehicleEngineStatus(unsigned long guid) = 0;
	virtual bool SetVehicleBodyHealth(unsigned long guid, float health) = 0; //Not implemented
	virtual bool SetVehicleEngineHealth(unsigned long guid, float health) = 0; //Not implemented
	virtual bool SetVehicleTankHealth(unsigned long guid, float health) = 0; //Not implemented
	virtual bool GetVehicleHealth(unsigned long guid, float *body, float *engine, float *tank) = 0;
	virtual bool SetVehicleNumberPlate(unsigned long guid, const char *text) = 0; //Not implemented
	virtual std::string GetVehicleNumberPlate(unsigned long guid) = 0; //Not implemented
	virtual bool SetVehicleNumberPlateStyle(unsigned long guid, int style) = 0; //Not implemented
	virtual int GetVehicleNumberPlateStyle(unsigned long guid) = 0; //Not implemented
	virtual bool SetVehicleSirenState(unsigned long guid, bool state) = 0;
	virtual bool GetVehicleSirenState(unsigned long guid) = 0;
	virtual bool SetVehicleWheelColor(unsigned long guid, int color) = 0; //Not implemented
	virtual int GetVehicleWheelColor(unsigned long guid) = 0; //Not implemented
	virtual bool SetVehicleWheelType(unsigned long guid, int type) = 0; //Not implemented
	virtual int GetVehicleWheelType(unsigned long guid) = 0; //Not implemented
	virtual unsigned long GetVehicleDriver(unsigned long guid) = 0;
	virtual std::vector<unsigned long> GetVehiclePassengers(unsigned long guid) = 0;

	virtual unsigned long CreateObject(long model, float x, float y, float z, float pitch, float yaw, float roll) = 0;
	virtual bool DeleteObject(unsigned long guid) = 0;

	virtual bool CreatePickup(int type, float x, float y, float z, float scale) = 0;

	virtual unsigned long CreateBlipForAll(std::string name, float x, float y, float z, float scale, int color, int sprite) = 0;
	virtual unsigned long CreateBlipForPlayer(long playerid, std::string name, float x, float y, float z, float scale, int color, int sprite) = 0;
	virtual void DeleteBlip(unsigned long guid) = 0;
	virtual void SetBlipColor(unsigned long guid, int color) = 0;
	virtual void SetBlipScale(unsigned long guid, float scale) = 0;
	virtual void SetBlipRoute(unsigned long guid, bool route) = 0;
	virtual void SetBlipSprite(unsigned long guid, int sprite) = 0;
	virtual void SetBlipName(unsigned long guid, std::string name) = 0;
	virtual void SetBlipAsShortRange(unsigned long guid, bool _short) = 0;
	virtual void AttachBlipToPlayer(unsigned long _guid, long player) = 0;
	virtual void AttachBlipToVehicle(unsigned long _guid, unsigned long vehicle) = 0;

	virtual unsigned long CreateMarkerForAll(float x, float y, float z, float height, float radius) = 0;
	virtual unsigned long CreateMarkerForPlayer(long playerid, float x, float y, float z, float height, float radius) = 0;
	virtual void DeleteMarker(unsigned long guid) = 0;

	virtual bool SendNotification(long playerid, const char * msg) = 0;
	virtual bool SetInfoMsg(long playerid, const char * msg) = 0;
	virtual bool UnsetInfoMsg(long playerid) = 0;

	virtual unsigned long Create3DText(const char * text, float x, float y, float z, int color, int outColor, float fontSize) = 0;
	virtual unsigned long Create3DTextForPlayer(unsigned long player, const char * text, float x, float y, float z, int color, int outColor) = 0;
	virtual bool Attach3DTextToVehicle(unsigned long textId, unsigned long vehicle, float oX, float oY, float oZ) = 0;
	virtual bool Attach3DTextToPlayer(unsigned long textId, unsigned long player, float oX, float oY, float oZ) = 0;
	virtual bool Set3DTextContent(unsigned long textId, const char * text) = 0;
	virtual bool Delete3DText(unsigned long textId) = 0;
};

#ifndef GTA_ORANGE_SERVER
class API : public APIBase
{
public:
	static API * instance;
	static void Set(API * api) { instance = api; }
	static API& Get() { return *instance; }
};
#endif
