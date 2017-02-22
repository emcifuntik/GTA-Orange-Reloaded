#pragma once

class baseClass;
class CWorld;
class CPed;
class grcRenderTargetDX11;
class grcTextureDX11;
class Tasks;
class CPlayerInfo;
class CTaskTreePed;
class CTaskTree;
class CTaskTreeMotion;
class CEntity_;
class CPedInventory;
class CPedWeaponManager;
class CModelInfo;
class CPedModelInfo;
class CVehicleModelInfo;
class phInstGta;
class CPedDrawHandler;
class CTaskTreeMovement;
class CBuilding;
class CPedMethods;
class CPedWeaponManager_;
class CPedClothCollision;
class CPedTaskManager;
class CWeapon;
class CWeaponMethods_;
class CPedFactory;
class CVehicleFactory;
class CTaskTreeFunctions;
class CTaskTreeMovement_;
class VTasks;

#ifndef D3DMATRIX_DEFINED
typedef struct _D3DMATRIX {
#if(DIRECT3D_VERSION >= 0x0500)
#if (defined __cplusplus) && (defined D3D_OVERLOADS)
	union {
		struct {
#endif

#endif /* DIRECT3D_VERSION >= 0x0500 */
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;

#if(DIRECT3D_VERSION >= 0x0500)
#if (defined __cplusplus) && (defined D3D_OVERLOADS)
		};
		D3DVALUE m[4][4];
	};
	_D3DMATRIX() { }
	_D3DMATRIX(D3DVALUE _m00, D3DVALUE _m01, D3DVALUE _m02, D3DVALUE _m03,
		D3DVALUE _m10, D3DVALUE _m11, D3DVALUE _m12, D3DVALUE _m13,
		D3DVALUE _m20, D3DVALUE _m21, D3DVALUE _m22, D3DVALUE _m23,
		D3DVALUE _m30, D3DVALUE _m31, D3DVALUE _m32, D3DVALUE _m33
	)
	{
		m[0][0] = _m00; m[0][1] = _m01; m[0][2] = _m02; m[0][3] = _m03;
		m[1][0] = _m10; m[1][1] = _m11; m[1][2] = _m12; m[1][3] = _m13;
		m[2][0] = _m20; m[2][1] = _m21; m[2][2] = _m22; m[2][3] = _m23;
		m[3][0] = _m30; m[3][1] = _m31; m[3][2] = _m32; m[3][3] = _m33;
	}

	D3DVALUE& operator()(int iRow, int iColumn) { return m[iRow][iColumn]; }
	const D3DVALUE& operator()(int iRow, int iColumn) const { return m[iRow][iColumn]; }
#if(DIRECT3D_VERSION >= 0x0600)
	friend _D3DMATRIX operator* (const _D3DMATRIX&, const _D3DMATRIX&);
#endif /* DIRECT3D_VERSION >= 0x0600 */
#endif
#endif /* DIRECT3D_VERSION >= 0x0500 */
} D3DMATRIX;
#define D3DMATRIX_DEFINED
#endif
typedef struct _D3DMATRIX *LPD3DMATRIX;

#define PED_UNCONTROLLABLE 0x40000000
#define PED_ENEMY 0x40

namespace GTA
{

	class CVehicleIntelligence;

	class CVehicle;

	class CViewportGame
	{
	public:
		char pad_0x0000[0x24C]; //0x0000
		_D3DMATRIX ViewPort; //0x024C 
		char pad_0x028C[0x1C4]; //0x028C
		__int32 Width; //0x0450 
		__int32 Height; //0x0454 

		static CViewportGame *Get();
	}; //Size=0x028C
	class CTask;
};

namespace rageGlobals
{
	void SetPlayerColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
};

class baseClass
{
public:
	CWorld* CWorldPtr; //0x0000 
	grcRenderTargetDX11* grcRenderTargetDX11Ptr; //0x0008 
	char pad_0x0010[0x8]; //0x0010
	CPedModelInfo* CPedModelInfoPtr; //0x0018 
	char pad_0x0020[0x8]; //0x0020
	grcTextureDX11* grcTextureDX11Ptr; //0x0028 

}; //Size=0x0030

class CWorld
{
public:
	CPedFactory * CPedFactoryPtr;
	CPed * CPedPtr; //0x0008 

	static CWorld *Get();
}; //Size=0x0010

class CPed
{
public:
	virtual void* Convert(bool somebool);
	virtual void unknown_1();
	virtual void unknown_2();
	virtual void unknown_3();
	virtual void unknown_4();
	virtual void unknown_5();
	virtual void unknown_6();
	virtual void unknown_7();
	virtual void unknown_8();
	virtual void unknown_9();
	virtual void unknown_10();
	virtual void unknown_11();
	virtual void GetMovePed();
	char pad_0x0008[0x18]; //0x0008
	CPedModelInfo* PedModelInfo; //0x0020 
	char pad_0x0028[0x8]; //0x0028
	phInstGta* phInstGtaPtr; //0x0030 
	char pad_0x0038[0x10]; //0x0038
	CPedDrawHandler* CPedDrawHandlerPtr; //0x0048 
	char pad_0x0050[0x40]; //0x0050
	CVector3 Position; //0x0090 
	char pad_0x009C[0x74]; //0x009C
	CVector3 Position2; //0x0110 
	char pad_0x011C[0x4]; //0x011C
	CVector3 Position3; //0x0120 
	char pad_0x012C[0x4]; //0x012C
	float Height; //0x0130 
	char pad_0x0134[0x2C]; //0x0134
	CVector3 Position4; //0x0160 
	char pad_0x016C[0x114]; //0x016C
	float Health; //0x0280 
	char unk1[28];
	float MaxHealth; //0x0284 
	char pad_0x0284[0xE4]; //0x0284
	float float_1; //0x0368 
	float float_2; //0x036C 
	float float_3; //0x0370 
	float float_4; //0x0374 
	char pad_0x0378[0x1DC]; //0x0378
	float float_5; //0x0554 
	char pad_0x0558[0x20]; //0x0558
	float MoveSpeed; //0x0578 
	float MoveSpeed2; //0x0578 
	float MoveSpeed3; //0x0578 
	float MoveSpeed4; //0x0578 
	float WalkSpeed; //0x0588 
	char pad_0x058C[0x614]; //0x058C
	CBuilding* CBuildingPtr; //0x0BA0 
	char pad_0x0BA8[0x4D8]; //0x0BA8
	Tasks* TasksPtr; //0x1080 
	CPlayerInfo* CPlayerInfoPtr; //0x1088 
	CPedInventory* CPedInventoryPtr; //0x1090 
	CPedWeaponManager* CPedWeaponManagerPtr; //0x1098 
	char pad_0x10A0[0x2A0]; //0x10A0
	CPedClothCollision* CPedClothCollisionPtr; //0x1340 
	char pad_0x1348[0x78]; //0x1348
	int32_t Flags; //0x13C0 
	char pad_0x13C4[0xD4]; //0x13C8
	GTA::CVehicle* pedVehicle; //0x1498 
	char pad_0x14A0[0x450]; //0x14A0

	static CPed * GetFromScriptID(int Handle);

}; //Size=0x18E8

class grcRenderTargetDX11
{
public:
	char pad_0x0000[0x8]; //0x0000

}; //Size=0x0008

class grcTextureDX11
{
public:
	char pad_0x0000[0x8]; //0x0000

}; //Size=0x0008

class Tasks
{
public:
	char pad_0x0000[0x358]; //0x0000
	CPedTaskManager* CPedTaskManagerPtr; //0x0358 
	CTaskTree* PrimaryTasks; //0x0360 
	CTaskTree* SecondaryTasks; //0x0368 
	CTaskTree* MovementTasks; //0x0370 
	CTaskTree* MotionTasks; //0x0378 
	char pad_0x0380[0x100]; //0x0380

}; //Size=0x0480

class CPlayerInfo
{
public:
	virtual ~CPlayerInfo();
	char pad_0x0008[0x208]; //0x0000
	float TimeSinceAim; //0x0210 
	__int32 AimState; //0x0214 
	__int32 CurrentWeaponHash; //0x0218 
	char pad_0x021C[0x44]; //0x021C
	CVector3 AimPosition; //0x0260 
	float AimLength; //0x026C 
	char pad_0x0270[0xA0]; //0x0270
	CEntity_* TargetEntity; //0x0310
	DWORD dwLastAimTime; //0x0318 
	char pad_0x011C[0x34]; //0x031C
	int8_t bAimAtEntity; //0x0350 
	char pad_0x0151[0x2F]; //0x0351

}; //Size=0x05C8

class CTaskTreePed
{
public:
	char pad_0x0000[0x8]; //0x0000

}; //Size=0x0008


struct TaskPair
{
	CSerialisedFSMTaskInfo *task;
	unsigned short taskID;
};

class CTaskTree
{
public:
	virtual ~CTaskTree();
	virtual int Clear();
	virtual void AssignTask(GTA::CTask *task, int taskPriority);
	virtual int Play(bool reset, int taskPriority, float speed);
	virtual int AddTask(GTA::CTask *task, float some = 0.f);
	CPed *TaskOwner;
	int32_t ActiveTask;
	int32_t field_4;
	int32_t field_5;
	int32_t field_6;
	GTA::CTask *TasksArray[6];

	GTA::CTask* GetTaskByID(unsigned int taskID);

	GTA::CTask* GetTask()
	{
		if (ActiveTask < 0)
			return nullptr;
		else
			return TasksArray[ActiveTask];
	}

}; //Size=0x0008

class CTaskTreeMotion
{
public:
	char pad_0x0000[0x8]; //0x0000

}; //Size=0x0008

class CEntity_
{
public:
	char pad_0x0000[0x8]; //0x0000

}; //Size=0x0008

class CPedInventory
{
public:
	char pad_0x0000[0x48]; //0x0000

}; //Size=0x0048

class CPedWeaponManager
{
public:
	CPedWeaponManager_* CPedWeaponManagerMethods; //0x0000 
	char pad_0x0008[0x80]; //0x0008
	CWeapon* CWeaponPtr; //0x0088 
	char pad_0x0090[0x78]; //0x0090

}; //Size=0x0108

class CModelInfo
{
public:
	void* vtable;
	char pad_0x0008[0x10];
	int64_t ModelHash;
};

class CPedModelInfo : public CModelInfo
{
public:
	char pad_0x0020[0xA8]; //0x0020

}; //Size=0x0008

class CVehicleModelInfo : public CModelInfo
{
public:
	char pad_0x0020[0xA8]; //0x0020

}; //Size=0x0008

class phInstGta
{
public:
	char pad_0x0000[0x8]; //0x0000

}; //Size=0x0008

class CPedDrawHandler
{
public:
	char pad_0x0000[0x8]; //0x0000

}; //Size=0x0008

class CTaskTreeMovement
{
public:
	CTaskTreeMovement_* CTaskTreeMovementMethods; //0x0000 
}; //Size=0x0008

class CBuilding
{
public:
	char pad_0x0000[0x8]; //0x0000

}; //Size=0x0008

class CPedMethods
{
public:
	virtual __int64 *func_1(__int64 a1, char a2) = 0; //0x0000 
	virtual bool *func_2(__int64 a1, int a2) = 0; //0x0008 
	char pad_0x0010[0x18]; //0x0010

}; //Size=0x0028

class CPedWeaponManager_
{
public:
	__int64 (*func_1)(__int64 a1, char a2); //0x0000 
	int (*func_2)(__int64 a1, unsigned int a2); //0x0008 
	int (*func_3)(__int64 a1, int a2, __int64 a3); //0x0010 
	int (*func_4)(__int64 a1); //0x0018

}; //Size=0x0020

class CPedClothCollision
{
public:
	char pad_0x0000[0x8]; //0x0000

}; //Size=0x0008

class CPedTaskManager
{
public:
	__int64 * (*func_1)(__int64 *a1, char a2); //0x0000 
}; //Size=0x0010

class CWeapon
{
public:
	CWeaponMethods_* CWeaponMethods; //0x0000 
	char pad_0x0008[0x10]; //0x0008

}; //Size=0x0018

class CWeaponMethods_
{
public:
	__int64(*Equip)(__int64 a1, char a2); //0x0000 
	char(*Shoot)(__int64 a1, CPed *a2, __int64 a3); //0x0008 
}; //Size=0x0010

class CPedFactory
{
public:
	__int64(*func_1)(CPedFactory* pedFactory, bool a2); //0x0000 
	__int64(*Create)(CPedFactory* pedFactory, unsigned char *a2, int *a3, __int64 a4, bool a5, bool networkEntity, bool a7); //0x0008 
	__int64(*func_3)(CPedFactory* *pedFactory, __int64 a2, int *a3, __int64 a4, int a5, bool a6); //0x0010 
	__int64(*func_4)(CPedFactory* pedFactory, __int64 a2, __int64 a3, bool a4, bool a5); //0x0018 
	void(*func_5)(CPedFactory* pedFactory, __int64 a2, __int64 a3, bool a4); //0x0020 
	__int64(*func_6)(CPedFactory* pedFactory, unsigned char *a2, __int64 a3, __int64 a4, __int64 a5); //0x0028 
	int(*func_7)(CPedFactory* pedFactory, __int64 a2); //0x0030 

}; //Size=0x0038

class PedFactoryHook
{
	static PedFactoryHook* singleInstance;
public:
	PedFactoryHook() {}
	static PedFactoryHook* Get()
	{
		if(!singleInstance)
			singleInstance = new PedFactoryHook();
		return singleInstance;
	}
	__int64(*CreateHook)(CPedFactory* pedFactory, unsigned char *a2, int *a3, __int64 a4, bool a5, bool networkEntity, bool a7);
};

inline __int64 hookCreatePed(CPedFactory* pedFactory, unsigned char *a2, int *a3, __int64 a4, bool a5, bool networkEntity, bool a7)
{
	return (__int64)nullptr;
}

class CVehicleFactory
{
public:
	virtual ~CVehicleFactory();
	virtual int64_t CreateVehicle(__int64 a2, unsigned int a3, unsigned int a4, float a5, signed int *a6, char a7, char a8);
	virtual int64_t func_3(__int64 a2, __int64 a3, __int64 a4, float a5, unsigned int a6, unsigned int a7, bool a8); //0x0010 
	virtual int func_4(__int64 a2, unsigned int a3, unsigned int a4, unsigned int a5, __int64 *a6); //0x0018 
	virtual char func_5(__int64 a2, unsigned int a3, unsigned int a4, unsigned int a5, WORD *a6); //0x0020 
	static CVehicleFactory* Get();
}; //Size=0x0088

class VehicleFactoryHook
{
	static VehicleFactoryHook* singleInstance;
public:
	VehicleFactoryHook() {}
	static VehicleFactoryHook* Get()
	{
		if (!singleInstance)
			singleInstance = new VehicleFactoryHook();
		return singleInstance;
	}
	__int64(*CreateHook)(__int64 a1, __int64 a2, unsigned int a3, unsigned int a4, float a5, signed int *a6, char a7, char a8);
};

inline __int64 hookCreateVehicle(__int64 a1, __int64 a2, unsigned int a3, unsigned int a4, float a5, signed int *a6, char a7, char a8)
{
	return (__int64)nullptr;
}

namespace rage
{
	class aiTask
	{
		virtual ~aiTask();
		int64_t var1;
		int64_t var2;
		int64_t var3;
		int64_t var4;
		int64_t var5;
		int32_t var6;
		int32_t var7;
		int16_t var8;
		int64_t var9;
	};
}

namespace GTA
{
	class CVehicleIntelligence
	{
	public:
		char pad_0x0000[0x7B8]; //0x0000
		CTaskTree* PrimaryTasks; //0x07B8 
		CTaskTree* SecondaryTasks; //0x07C0 
	}; //Size=0x0808

	class CVehicle
	{
	public:
		virtual void Function0(); //
		virtual void Function1(); //
		virtual void Function2(); //
		virtual void Function3(); //
		virtual void Function4(); //
		virtual void Function5(); //
		virtual void Function6(); //
		virtual void Function7(); //
		virtual void Function8(); //
		virtual void Function9(); //

		char pad_0x0008[0x18]; //0x0008
		CVehicleModelInfo* VehicleModelInfo; //0x0020 
		char pad_0x0028[0x68]; //0x0028
		Vector3 Position; //0x0090 
		char pad_0x009C[0x9F4]; //0x009C
		CVehicleIntelligence* VehicleIntelligence; //0x0A90 
		char pad_0x0A98[0xC28]; //0x0A98

	};

	class CObject
	{
	public:
		char pad_0x0000[0x20];
		CModelInfo *ModelInfo;
		char pad_0x0028[0x68]; //0x0028
		CVector3 Position; //0x0090 
	};

	class CTask
	{
	public:
		virtual ~CTask();
		virtual int64_t GetID();
		virtual CTask* Clone();
		virtual bool SetSubTask_(int someInt, GTA::CTask* childTask);
		virtual bool IsSimple();
		virtual void unknown_5();
		virtual void unknown_6();
		virtual void unknown_7();
		virtual void unknown_8();
		virtual void unknown_9();
		virtual void unknown_10();
		virtual void unknown_11();
		virtual void unknown_12();
		virtual void unknown_13();
		virtual void unknown_14();
		virtual void unknown_15();
		virtual void unknown_16();
		virtual void unknown_17();
		virtual void unknown_18();
		virtual void unknown_19();
		virtual void unknown_20();
		virtual void unknown_21();
		virtual void unknown_22();
		virtual void unknown_23();
		virtual void unknown_24();
		virtual void unknown_25();
		virtual void unknown_26();
		virtual void unknown_27();
		virtual void unknown_28();
		virtual void unknown_29();
		virtual void unknown_30();
		virtual void unknown_31();
		virtual void unknown_32();
		virtual void unknown_33();
		virtual void unknown_34();
		virtual void unknown_35();
		virtual void unknown_36();
		virtual void unknown_37();
		virtual void unknown_38();
		virtual void unknown_39();
		virtual void unknown_40();
		virtual void unknown_41();
		virtual void unknown_42();
		virtual void unknown_43();
		virtual void unknown_44();
		virtual bool IsSerializable();
		virtual CSerialisedFSMTaskInfo* Serialize();
		virtual void unknown_47();
		virtual void unknown_48();
		virtual void unknown_49();
		virtual void unknown_50();
		virtual bool returnZero64_1();
		virtual bool returnZero64_2();
		virtual bool returnTrue_1();
		virtual bool returnZero64_3();
		virtual bool returnMinusOne_1();
		virtual bool returnFalse_1();
		virtual void unknown_57();
		virtual void unknown_58();
		virtual void unknown_59();
		virtual void unknown_60();
		virtual void unknown_61();
		virtual bool returnFalse_2();
		virtual bool returnFalse_3();
		virtual bool returnFalse_4();
		virtual bool returnFalse_5();
		virtual bool returnFalse_6();
		virtual bool returnTrue_2();
		virtual bool returnFalse_7();
		virtual bool returnTrue_3();
		virtual int Deserialize(void* clonedTask);

		void *somePtr;
		CPed *targetPed;
		CTask *Parent;
		CTask *SubTask;
		CTask* NextSubTask;
		int32_t field_2;
		int32_t field_3;
		float timeFromBegin;
		float totalTime;

		std::string GetTree(CTask *task = nullptr, int n = 0);
	};
};


typedef void CAutomobileSyncTree; 
typedef void CBikeSyncTree;
typedef void CBoatSyncTree;
typedef void CSubmarineSyncTree;
typedef void CHeliSyncTree;
typedef void CObjectSyncTree;
typedef void CPedSyncTree;
typedef void CPickupSyncTree;
typedef void CPickupPlacementSyncTree;
typedef void CPlayerSyncTree;
typedef void CTrainSyncTree;
typedef void CPlaneSyncTree;
typedef void CDoorSyncTree;

class rageBuffer
{
public:
	unsigned char *Buffer; //0x0000 
	DWORD Align; //0x0008 
	DWORD StartSize; //0x000C 
	DWORD FinalSize; //0x0010 
	DWORD Size2; //0x0014 
	DWORD Unknown; //0x0018 
	unsigned char Flags; //0x001C 

}; //Size=0x001D

typedef void*(*GetSyncTree_)(int64_t unused, int16_t id);
class SyncTree
{
private:
	static bool initialized;
	static GetSyncTree_ GetSyncTree;
public:
	static CAutomobileSyncTree* GetAutomobileSyncTree()
	{
		Init();
		return GetSyncTree(0, 0);
	}
	static CBikeSyncTree* GetBikeSyncTree()
	{
		Init();
		return GetSyncTree(0, 1);
	}
	static CBoatSyncTree* GetBoatSyncTree()
	{
		Init();
		return GetSyncTree(0, 2);
	}

	static CSubmarineSyncTree* GetSubmarineSyncTree()
	{
		Init();
		return GetSyncTree(0, 10);
	}
	static CHeliSyncTree* GetHeliSyncTree()
	{
		Init();
		return GetSyncTree(0, 4);
	}
	static CObjectSyncTree* GetObjectSyncTree()
	{
		Init();
		return GetSyncTree(0, 5);
	}
	static CPedSyncTree* GetPedSyncTree()
	{
		Init();
		return GetSyncTree(0, 6);
	}
	static CPickupSyncTree* GetPickupSyncTree()
	{
		Init();
		return GetSyncTree(0, 7);
	}
	static CPickupPlacementSyncTree* GetPickupPlacementSyncTree()
	{
		Init();
		return GetSyncTree(0, 8);
	}
	static CPlayerSyncTree* GetPlayerSyncTree()
	{
		Init();
		return GetSyncTree(0, 11);
	}
	static CTrainSyncTree* GetTrainSyncTree()
	{
		Init();
		return GetSyncTree(0, 13);
	}
	static CPlaneSyncTree* GetPlaneSyncTree()
	{
		Init();
		return GetSyncTree(0, 9);
	}
	static CDoorSyncTree* GetDoorSyncTree()
	{
		Init();
		return GetSyncTree(0, 3);
	}

	static void Init()
	{
		if (!initialized)
		{
			/*typedef void(*InitSyncTrees_)();
			InitSyncTrees_ initSyncTrees = (InitSyncTrees_)CMemory::Find("48 83 EC 28 E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 48 83 C4 28 E9 ? ? ? ?")();
			initSyncTrees();
			GetSyncTree = (GetSyncTree_)CMemory::Find("0F B7 CA 83 F9 07 7F 5E  74 54 85 C9 74 48")();*/
			initialized = true;
		}
	}
};

