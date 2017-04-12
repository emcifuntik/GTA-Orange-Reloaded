#pragma once

enum
{
	TASK_TYPE_PRIMARY = 0,
	TASK_TYPE_SECONDARY,
	TASK_TYPE_MOVEMENT,
	TASK_TYPE_MOTION
};

struct OnFootSyncData
{
public:
	bool bAiming : 1;
	bool bShooting : 1;
	bool bRagdoll : 1;
	bool bReloading : 1;
	bool bDuckState : 1;
	bool bJumping : 1;
	bool bInVehicle : 1;
	bool bAimAtPlayer : 1;

	char cSeat;

	unsigned short int usHealth : 16;
	unsigned short int usArmour : 16;

	unsigned int uAmmo;

	unsigned int ulWeapon;
	unsigned int hModel;

	float fMoveSpeed;
	float fHeading;

	CVector3 vecPos;
	CVector3 vecRot;
	CVector3 vecAim;
	CVector3 vecMoveSpeed;

	RakNetGUID rnVehicle;
	RakNetGUID rnAimAt;
};

struct VehicleData
{
	bool bTaxiLights : 1;
	bool bSirenState : 1;
	bool bDrivable : 1;
	bool bHorn : 1;
	bool hasDriver : 1;
	bool Burnout : 1;

	unsigned short int usHealth : 16;

	unsigned int hashModel;

	float iDirtLevel;
	float steering;
	float RPM;
	float fEngineHealth;
	float fBodyHealth;
	float fTankHealth;

	CVector3 vecPos;
	CVector3 vecRot;
	CVector3 vecMoveSpeed;

	RakNetGUID GUID;
	RakNetGUID driver;
};

struct ObjectData
{
public:
	unsigned int hashModel;
	CVector3 vecPos;
	CVector3 vecRot;
	unsigned short int usHealth : 16;


	RakNetGUID GUID;
};

class CSyncDataLogger
{
public:
	virtual ~CSyncDataLogger();
	virtual void overload1();
	virtual void overload2();
	virtual void overload3();
	virtual void overload4();
	virtual void overload5();
	virtual void overload6();
	virtual void printBool(bool value, int64_t unknownValue = 0);
};

class CSerialisedFSMTaskInfo
{
public:
	virtual ~CSerialisedFSMTaskInfo();
	virtual int64_t GetID();
	virtual void* Read(void* task);
	virtual void* Write(void* task);
	virtual void* Log(void* task);
	virtual int64_t Size();
	virtual int widen();
	virtual void sub_F65F30_2();
	virtual void* GetTask();
	virtual bool IsCloned();
	virtual void sub_F5D670();
	virtual void sub_13C5608();
	virtual void sub_6656C4();
	virtual int64_t anyGetter();
	virtual void anySetter(int64_t value);
	virtual void sub_F65F30_3();
	virtual int64_t anyGetter_2(int64_t ptr);
	virtual int64_t anyGetter_3(int64_t ptr);
	virtual int64_t anyGetter_4();
	virtual void sub_167A9B8_2();
	virtual void sub_167A9B8_3();
	virtual void sub_B89BE4();
	virtual void sub_12F2410();
	virtual void sub_C2E798();
	virtual int SetData(void *);
	virtual char* GetStatus();
};

class CClonedGoToPointAimingInfo:
	public CSerialisedFSMTaskInfo
{
public:
	char data[0x78];
};

class CClonedAimGunOnFootInfo :
	public CSerialisedFSMTaskInfo
{
public:
	char data[0x38];
};
