#include "stdafx.h"

void __fastcall eventHook(GTA::CTask* task)
{
	log_debug << task->GetTree() << std::endl;
	CLocalPlayer::Get()->updateTasks = true;
}

CLocalPlayer* CLocalPlayer::Instance = nullptr;

CLocalPlayer::CLocalPlayer() :CPedestrian(PLAYER::PLAYER_PED_ID())
{
	for (int i = 0; i < 5; i++)
		GAMEPLAY::DISABLE_HOSPITAL_RESTART(i, true);

	for (int i = 0; i < 50; i++) {
		GAMEPLAY::DISABLE_STUNT_JUMP_SET(i);
		GAMEPLAY::DELETE_STUNT_JUMP(i);
	}

	CEntity::InitOffsetFunc();

	aimPosition = &CWorld::Get()->CPedPtr->CPlayerInfoPtr->AimPosition;

	//rageGlobals::SetPlayerColor(0x33, 0xFF, 0x33, 0xFF);

	/*auto addr = CMemory((uintptr_t)GetModuleHandle(NULL) + 0x4E1FA4);
	addr.nop(20);*/

	GiveWeapon(Utils::Hash("weapon_smg"), 9999);

	typedef int(*ShowAbilityBar)(bool);
	((ShowAbilityBar)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1F26D4)())(false);
}


CLocalPlayer::~CLocalPlayer()
{
}

void CLocalPlayer::GetOnFootSync(OnFootSyncData& onfoot)
{
	onfoot.hModel = GetModel();
	onfoot.bJumping = IsJumping();
	onfoot.fMoveSpeed = CWorld::Get()->CPedPtr->MoveSpeed;
	onfoot.vecPos = GetPosition();
	onfoot.vecRot = GetRotation();
	onfoot.fHeading = GetHeading();
	GetMoveSpeed(onfoot.vecMoveSpeed);
	onfoot.vecTurnSpeed = GetRotationVelocity();
	onfoot.bDuckState = IsDucking();
	onfoot.usHealth = GetHealth();
	onfoot.usArmour = GetArmour();
	onfoot.ulWeapon = GetCurrentWeapon();
	onfoot.uAmmo = GetCurrentWeaponAmmo();
	onfoot.vecAim = *aimPosition;
	onfoot.bAiming = (CWorld::Get()->CPedPtr->CPlayerInfoPtr->AimState == 2);
	onfoot.bShooting = PED::IS_PED_SHOOTING(Handle) ? true : false;
	if (PED::IS_PED_IN_ANY_VEHICLE(Handle, true)) {
		onfoot.bInVehicle = true;
		onfoot.vehicle = CNetworkVehicle::GetByHandle(PED::GET_VEHICLE_PED_IS_TRYING_TO_ENTER(Handle))->m_GUID;
	}
	else {
		onfoot.bInVehicle = false;
	}
}

void CLocalPlayer::GetVehicleSync(VehicleData& vehsync)
{
	CNetworkVehicle *veh = CNetworkVehicle::GetByHandle(PED::GET_VEHICLE_PED_IS_IN(Handle, false));
	//log << "Steering ang: " << (*CMemory((uintptr_t)_entityAddressFunc(veh->GetHandle())).get<float>(0x8CC)) * (180.0 / PI) << std::endl;
	if (!veh) {
		//log << "Cant get vehData.." << std::endl;
		return;
	}
	vehsync.hasDriver = true;
	vehsync.GUID = veh->m_GUID;
	CVector3 pos = veh->GetPosition();
	pos.fZ -= 0.02f;
	vehsync.vecPos = pos;
	vehsync.vecRot = veh->GetRotation();
	vehsync.vecMoveSpeed = veh->GetMovementVelocity();
	vehsync.usHealth = veh->GetHealth();

	vehsync.RPM = *CMemory(veh->GetAddress()).get<float>(0x7F4);
	vehsync.Burnout = VEHICLE::IS_VEHICLE_IN_BURNOUT(veh->GetHandle()) != 0;

	if (VEHICLE::IS_THIS_MODEL_A_CAR(veh->GetModel()) || VEHICLE::IS_THIS_MODEL_A_BIKE(veh->GetModel()) || VEHICLE::IS_THIS_MODEL_A_QUADBIKE(veh->GetModel()))
		vehsync.steering = (*CMemory(veh->GetAddress()).get<float>(0x8CC)) * (180.0f / PI);
}

CLocalPlayer * CLocalPlayer::Get()
{
	if (!Instance)
		Instance = new CLocalPlayer();
	return Instance;
}

void CLocalPlayer::Tick()
{
	if (newModel != 0)
	{
		ChangeModel(newModel);
		newModel = 0;
	}
	PLAYER::SET_PLAYER_HEALTH_RECHARGE_MULTIPLIER(PLAYER::PLAYER_ID(), 0.f);
	PLAYER::SET_AUTO_GIVE_PARACHUTE_WHEN_ENTER_PLANE(PLAYER::PLAYER_ID(), false);
	PLAYER::ENABLE_SPECIAL_ABILITY(PLAYER::PLAYER_ID(), false);
}

void CLocalPlayer::ChangeModel(Hash model)
{
	if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model))
		STREAMING::REQUEST_MODEL(model);
	while (!STREAMING::HAS_MODEL_LOADED(model))
		scriptWait(0);
	PLAYER::SET_PLAYER_MODEL(PLAYER::PLAYER_ID(), model);
	PED::SET_PED_DEFAULT_COMPONENT_VARIATION(PLAYER::PLAYER_PED_ID());
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
	Handle = PLAYER::PLAYER_PED_ID();
	typedef int(*ShowAbilityBar)(bool);
	((ShowAbilityBar)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1F26D4)())(false);
}

void CLocalPlayer::Connect()
{
	RakNet::BitStream requestid;
	RakString playerName(CConfig::Get()->sNickName.c_str());
	requestid.Write((MessageID)ID_CONNECT_TO_SERVER);
	requestid.Write(playerName);
	CNetworkConnection::Get()->client->Send(&requestid, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void CLocalPlayer::SendOnFootData()
{
	RakNet::BitStream bsOut;
	if (!PED::IS_PED_IN_ANY_VEHICLE(Handle, false))
	{
		bsOut.Write((MessageID)ID_SEND_PLAYER_DATA);
		OnFootSyncData data;
		GetOnFootSync(data);
		bsOut.Write(data);
	}
	else {
		bsOut.Write((MessageID)ID_SEND_VEHICLE_DATA);
		VehicleData data;
		GetVehicleSync(data);
		bsOut.Write(data);
	}
	CNetworkConnection::Get()->client->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void CLocalPlayer::SendTasks()
{
	if (PLAYER::IS_PLAYER_PLAYING(PLAYER::PLAYER_ID()))
	{
		RakNet::BitStream bsOut;
		bsOut.Write((unsigned char)ID_SEND_TASKS);
		bool foundPrimary = false;

		int tasks = 0;
		for (GTA::CTask *task = CWorld::Get()->CPedPtr->TasksPtr->PrimaryTasks->GetTask(); task; task = task->SubTask)
		{
			if (!task->IsSerializable())
				continue;
			tasks++;
		}
		bsOut.Write(tasks);

		for (GTA::CTask *task = CWorld::Get()->CPedPtr->TasksPtr->PrimaryTasks->GetTask(); task; task = task->SubTask)
		{
			if (!task->IsSerializable())
				continue;
			auto ser = task->Serialize();

			if (ser)
			{
				foundPrimary = true;
				unsigned short taskID = (unsigned short)task->GetID();
				bsOut.Write(taskID);
				unsigned int size = (unsigned int)ser->Size();
				bsOut.Write(size);

				rageBuffer data;
				unsigned char *buffer = new unsigned char[Utils::RoundToBytes(size)];
				memset(buffer, 0, Utils::RoundToBytes(size));

				typedef void(*InitWriteBuffer)(rageBuffer*, unsigned char*, int, int);
				((InitWriteBuffer)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x11E7920)())(&data, buffer, size, 0);

				void *reader = ser->Write(&data);
				bsOut.WriteBits(buffer, size);
				delete[] buffer;
			}
			rage::sysMemAllocator::Get()->free(ser, rage::HEAP_TASK_CLONE);
		}
		if (foundPrimary)
		{
			CNetworkConnection::Get()->client->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		}
	}
}

void CLocalPlayer::SetMoney(int money)
{
	for (int i = 0; i < 3; i++)
	{
		char statNameFull[32];
		sprintf_s(statNameFull, "SP%d_TOTAL_CASH", i);
		Hash hash = GAMEPLAY::GET_HASH_KEY(statNameFull);
		STATS::STAT_SET_INT(hash, money, 1);
	}
}