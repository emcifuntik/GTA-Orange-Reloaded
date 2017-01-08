#pragma once

class CReplayInterfacePed;
class CReplayInterfaceVeh;
class CReplayInterfaceCamera;
class CReplayInterfaceGame;
class CReplayInterfacePickup;
class CReplayInterfaceObject;

template<typename T>
class CPoolItem
{
public:
	T* object;
	int16_t objectID;
};

template<typename T>
class CPool
{
public:
	CPoolItem<T> *list;
	T* operator[](std::size_t number)
	{
		return list[number].object;
	}
	int64_t GetHandle(std::size_t number)
	{
		return list[number].objectID;
	}
	int64_t Count()
	{
		return count;
	}
	int64_t Capacity()
	{
		return capacity;
	}
private:
	int64_t capacity;
	int64_t count;
};

class ReplayInterfaces
{
public:
	CReplayInterfaceGame* ReplayInterfaceGame;
	CReplayInterfaceCamera* ReplayInterfaceCamera;
	CReplayInterfaceVeh* ReplayInterfaceVeh;
	CReplayInterfacePed* ReplayInterfacePed;
	CReplayInterfacePickup* ReplayInterfacePickup;
	CReplayInterfaceObject* ReplayInterfaceObject;
	
	static ReplayInterfaces* Get();
};

class CReplayInterfacePedVTable
{
public:
	int64_t(*Function1)(int64_t a1, char a2);
};

class CReplayInterfacePed
{
public:
	CReplayInterfacePedVTable *virtualFunctions;
	char pad_0x0008[0xF8]; //0x0000
	CPool<rage::CPed> pool;
	//char pad_0x0118[0x370]; //0x0118

}; //Size=0x0488

class CReplayInterfaceVeh
{
public:
	char pad_0x0000[0x180]; //0x0000
	CPool<rage::CVehicle> pool;
	char pad_0x0198[0xB0]; //0x0198

}; //Size=0x0248

class CReplayInterfaceCamera
{
public:
	char pad_0x0000[0x8]; //0x0000

}; //Size=0x0008

class CReplayInterfaceGame
{
public:
	char pad_0x0000[0x108]; //0x0000

}; //Size=0x0108

class CReplayInterfacePickup
{
public:
	char pad_0x0000[0x100]; //0x0000
	int64_t PoolContent; //0x0100 
	int64_t PoolCapacity; //0x0108 
	int64_t PoolSize; //0x0110 
	char pad_0x0118[0x70]; //0x0118

}; //Size=0x0188

class CReplayInterfaceObject
{
public:
	char pad_0x0000[0x158]; //0x0000
	CPool<GTA::CObject> pool;
	char pad_0x0170[0x58]; //0x0170

}; //Size=0x01C8
