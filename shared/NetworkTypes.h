#pragma once

enum
{
	TASK_TYPE_PRIMARY = 0,
	TASK_TYPE_SECONDARY,
	TASK_TYPE_MOVEMENT,
	TASK_TYPE_MOTION
};

class OnFootSyncData
{
public:
	//CControlState controlState;		// control state
	Hash hModel;						// player model
	float fMoveSpeed;
	CVector3 vecPos;					// player position
	CVector3 vecRot;					// player rotation
	float fHeading;						// player heading
	CVector3 vecMoveSpeed;				// player move speed
	CVector3 vecTurnSpeed;				// player turn speed
	bool bDuckState : 1;				// ducking
	bool bJumping : 1;					// jumping
	unsigned short int usHealth : 16;	// player health 
	unsigned short int usArmour : 16;	// player armor
	unsigned long ulWeapon;				// player weapon and ammo
	unsigned int uAmmo;					// player weapon and ammo
	bool bAiming : 1;
	bool bShooting : 1;
	bool bReloading : 1;
	CVector3 vecAim;
	bool bInVehicle;
	RakNetGUID vehicle;

	std::string ToString()
	{
		std::stringstream ss;
		ss << "Position: X = " << vecPos.fX << ", Y = " << vecPos.fY << ", Z = " << vecPos.fZ << std::endl <<
			"Rotation: X = " << vecRot.fY << ", Y = " << vecRot.fY << ", Z = " << vecRot.fZ << std::endl <<
			"Heading: " << fHeading << std::endl <<
			"Velocity: X = " << vecMoveSpeed.fX << ", Y = " << vecMoveSpeed.fY << ", Z = " << vecMoveSpeed.fY << std::endl <<
			"Rotation velocity: X = " << vecTurnSpeed.fX << ", Y = " << vecTurnSpeed.fY << ", Z = " << vecTurnSpeed.fZ << std::endl <<
			"DuckState: " << (bDuckState ? "true" : "false") << std::endl <<
			"Health: " << usHealth << std::endl <<
			"Armour: " << usArmour << std::endl <<
			"Weapon: " << ulWeapon << std::endl <<
			"Ammo: " << uAmmo << std::endl <<
			"BlendRatio: " << fMoveSpeed << std::endl <<
			"Jumping: " << (bJumping ? "true" : "false") << std::endl <<
			"Aiming: " << (bAiming ? "true" : "false") << std::endl <<
			"Shooting: " << (bShooting ? "true" : "false") << std::endl <<
			"Aim pos: X = " << vecAim.fX << ", Y = " << vecAim.fY << ", Z = " << vecAim.fZ << std::endl <<
			"Player model: " << hModel << std::endl;
		return ss.str();
	}
};

class VehicleData
{
public:
	RakNet::RakNetGUID GUID;
	RakNet::RakNetGUID driver;			// vehicle driver
	Hash hashModel;						// vehicle model
	CVector3 vecPos;					// vehicle position
	CVector3 vecRot;					// vehicle rotation
	CVector3 vecMoveSpeed;				// vehicle move speed
	unsigned short int usHealth : 16;	// vehicle health 
	bool bTaxiLights : 1;				// vehicle taxi lights
	bool bSirenState : 1;				// vehicle siren state
	float iDirtLevel;					// vehicle dirt level
	bool hasDriver = false;
	float steering;
	bool Burnout;
	float RPM;

	std::string ToString()
	{
		std::stringstream ss;
		ss << "Position: X = " << vecPos.fX << ", Y = " << vecPos.fY << ", Z = " << vecPos.fZ << std::endl <<
			"Rotation: X = " << vecRot.fY << ", Y = " << vecRot.fY << ", Z = " << vecRot.fZ << std::endl <<
			"Velocity: X = " << vecMoveSpeed.fX << ", Y = " << vecMoveSpeed.fY << ", Z = " << vecMoveSpeed.fY << std::endl <<
			"Health: " << usHealth << std::endl <<
			"Model: 0x" << std::hex << hashModel << std::endl <<
			"Driver: " << driver.ToString() << std::endl <<
			"Taxi lights: " << (bTaxiLights ? "true" : "false") << std::endl <<
			"Siren state: " << (bSirenState ? "true" : "false") << std::endl <<
			"Dirt level: " << iDirtLevel << std::endl;
		return ss.str();
	}
};

class ObjectData
{
public:
	RakNet::RakNetGUID GUID;
	//RakNet::RakNetGUID driver;			// vehicle driver
	Hash hashModel;						// vehicle model
	CVector3 vecPos;					// vehicle position
	CVector3 vecRot;					// vehicle rotation
	//CVector3 vecMoveSpeed;				// vehicle move speed
	unsigned short int usHealth : 16;	// vehicle health 
	//bool bTaxiLights : 1;				// vehicle taxi lights
	//bool bSirenState : 1;				// vehicle siren state
	//float iDirtLevel;					// vehicle dirt level
	//bool hasDriver = false;
	//float steering;
	//bool Burnout;
	//float RPM;

	std::string ToString()
	{
		std::stringstream ss;
		ss << "Position: X = " << vecPos.fX << ", Y = " << vecPos.fY << ", Z = " << vecPos.fZ << std::endl <<
			"Rotation: X = " << vecRot.fY << ", Y = " << vecRot.fY << ", Z = " << vecRot.fZ << std::endl <<
			//"Velocity: X = " << vecMoveSpeed.fX << ", Y = " << vecMoveSpeed.fY << ", Z = " << vecMoveSpeed.fY << std::endl <<
			"Health: " << usHealth << std::endl <<
			"Model: 0x" << std::hex << hashModel << std::endl; // <<
			//"Driver: " << driver.ToString() << std::endl <<
			//"Taxi lights: " << (bTaxiLights ? "true" : "false") << std::endl <<
			//"Siren state: " << (bSirenState ? "true" : "false") << std::endl <<
			//"Dirt level: " << iDirtLevel << std::endl;
		return ss.str();
	}
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
