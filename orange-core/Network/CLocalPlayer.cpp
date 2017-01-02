#include "stdafx.h"

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

	rageGlobals::SetPlayerColor(0xFF, 0x8F, 0x00, 0xFF);

	typedef int(*ShowAbilityBar)(bool);
	((ShowAbilityBar)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1F26D4)())(false);

	PLAYER::SET_PLAYER_HEALTH_RECHARGE_MULTIPLIER(PLAYER::PLAYER_ID(), 0.f);
	PLAYER::SET_AUTO_GIVE_PARACHUTE_WHEN_ENTER_PLANE(PLAYER::PLAYER_ID(), false);
	PLAYER::ENABLE_SPECIAL_ABILITY(PLAYER::PLAYER_ID(), false);
	STATS::STAT_SET_INT(Utils::Hash("WHEELIE_ABILITY"), 100, 1);
	STATS::STAT_SET_INT(Utils::Hash("STAMINA"), 100, 1);
	STATS::STAT_SET_INT(Utils::Hash("STRENGTH"), 100, 1);
	STATS::STAT_SET_INT(Utils::Hash("LUNG_CAPACITY"), 100, 1);
	STATS::STAT_SET_INT(Utils::Hash("FLYING_ABILITY"), 100, 1);
	STATS::STAT_SET_INT(Utils::Hash("SHOOTING_ABILITY"), 100, 1);
	STATS::STAT_SET_INT(Utils::Hash("STEALTH_ABILITY"), 100, 1);
}


CLocalPlayer::~CLocalPlayer()
{
}

void CLocalPlayer::Spawn()
{
	Spawned = true;

	//CAM::DESTROY_CAM(CGlobals::Get().currentcam, true);
	CAM::RENDER_SCRIPT_CAMS(false, false, 0, false, false);
	UI::DISPLAY_HUD(true);
	UI::DISPLAY_RADAR(true);
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
	onfoot.vecAim = CWorld::Get()->CPedPtr->CPlayerInfoPtr->AimPosition;
	onfoot.bAiming = (CWorld::Get()->CPedPtr->CPlayerInfoPtr->AimState == 2);
	onfoot.bShooting = PED::IS_PED_SHOOTING(Handle) ? true : false;
	onfoot.bRagdoll = PED::IS_PED_RAGDOLL(Handle) ? true : false;

	if (PED::IS_PED_IN_ANY_VEHICLE(Handle, true)) {
		onfoot.bInVehicle = true;
		CNetworkVehicle *veh = CNetworkVehicle::GetByHandle(PED::GET_VEHICLE_PED_IS_TRYING_TO_ENTER(Handle));
		if (veh)
		{
			onfoot.vehicle = veh->m_GUID;
			short seat = PED::GET_SEAT_PED_IS_TRYING_TO_ENTER(Handle);
			onfoot.vehseat = seat == -3 ? -2 : seat;
		}
		else {
			veh = CNetworkVehicle::GetByHandle(PED::GET_VEHICLE_PED_IS_IN(Handle, false));
			if (veh) {
				onfoot.vehicle = veh->m_GUID;
				onfoot.vehseat = GetSeat();
			}
		}
	}
	else {
		onfoot.bInVehicle = false;
	}
}

void CLocalPlayer::GetVehicleSync(VehicleData& vehsync)
{
	CNetworkVehicle *veh = CNetworkVehicle::GetByHandle(PED::GET_VEHICLE_PED_IS_IN(Handle, false));
	if (!veh) {
		return;
	}
	vehsync.hasDriver = true;
	vehsync.GUID = veh->m_GUID;
	CVector3 pos = veh->GetPosition();
	pos.fZ -= 0.02f;
	vehsync.vecPos = pos;
	vehsync.vecRot = veh->GetRotation();
	vehsync.vecMoveSpeed = veh->GetMovementVelocity();

	vehsync.RPM = *CMemory(veh->GetAddress()).get<float>(0x7F4);
	vehsync.Burnout = VEHICLE::IS_VEHICLE_IN_BURNOUT(veh->GetHandle()) != 0;

	if (VEHICLE::IS_THIS_MODEL_A_CAR(veh->GetModel()) || VEHICLE::IS_THIS_MODEL_A_BIKE(veh->GetModel()) || VEHICLE::IS_THIS_MODEL_A_QUADBIKE(veh->GetModel()))
		vehsync.steering = (*CMemory(veh->GetAddress()).get<float>(0x8CC)) * (180.0f / PI);

	vehsync.usHealth = veh->GetHealth();
	vehsync.fEngineHealth = VEHICLE::GET_VEHICLE_ENGINE_HEALTH(veh->GetHandle());
	vehsync.fBodyHealth = VEHICLE::GET_VEHICLE_BODY_HEALTH(veh->GetHandle());
	vehsync.fTankHealth = VEHICLE::GET_VEHICLE_PETROL_TANK_HEALTH(veh->GetHandle());
	vehsync.bDrivable = VEHICLE::IS_VEHICLE_DRIVEABLE(veh->GetHandle(), 0) == 1;

	vehsync.bHorn = AUDIO::IS_HORN_ACTIVE(veh->GetHandle()) == 1;
	vehsync.bSirenState = VEHICLE::IS_VEHICLE_SIREN_ON(veh->GetHandle());

	//log << "Vehicle: 0x" << std::hex << veh->GetAddress() << std::endl;
	//log << "Horn: " << AUDIO::IS_HORN_ACTIVE(veh->GetHandle()) << std::endl;
	//log << "Engine health: " << VEHICLE::GET_VEHICLE_ENGINE_HEALTH(veh->GetHandle()) << std::endl;
	//log << "Body health: " << VEHICLE::GET_VEHICLE_BODY_HEALTH(veh->GetHandle()) << std::endl;
	//log << "PetrolTank health: " << VEHICLE::GET_VEHICLE_PETROL_TANK_HEALTH(veh->GetHandle()) << std::endl;
	//log << "Drivable: " << VEHICLE::IS_VEHICLE_DRIVEABLE(veh->GetHandle(), 0) << std::endl;
	//log << "Health: " << veh->GetHealth() << std::endl;
}

CLocalPlayer * CLocalPlayer::Get()
{
	if (!Instance)
		Instance = new CLocalPlayer();
	return Instance;
}

void CLocalPlayer::Tick()
{
	if (PED::IS_PED_DEAD_OR_DYING(Handle, 1))
	{
		//CVector3 pos = GetPosition();
		//ChangeModel(GetModel());
	}
	if (newModel != 0)
	{
		ChangeModel(newModel);
		newModel = 0;
	}
	if (!Spawned) AI::CLEAR_PED_TASKS_IMMEDIATELY(Handle);
	if (_togopassenger) CLocalPlayer::GoPassenger();
}

void CLocalPlayer::ChangeModel(Hash model)
{
	if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model))
	{
		STREAMING::REQUEST_MODEL(model);
		while (!STREAMING::HAS_MODEL_LOADED(model))
			scriptWait(0);
		PLAYER::SET_PLAYER_MODEL(PLAYER::PLAYER_ID(), model);
		PED::SET_PED_DEFAULT_COMPONENT_VARIATION(PLAYER::PLAYER_PED_ID());
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
		Handle = PLAYER::PLAYER_PED_ID();
		typedef int(*ShowAbilityBar)(bool);
		((ShowAbilityBar)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1F26D4)())(false);
		PLAYER::SET_PLAYER_HEALTH_RECHARGE_MULTIPLIER(PLAYER::PLAYER_ID(), 0.f);
		PLAYER::SET_AUTO_GIVE_PARACHUTE_WHEN_ENTER_PLANE(PLAYER::PLAYER_ID(), false);
		PLAYER::ENABLE_SPECIAL_ABILITY(PLAYER::PLAYER_ID(), false);
		STATS::STAT_SET_INT(Utils::Hash("WHEELIE_ABILITY"), 100, 1);
		STATS::STAT_SET_INT(Utils::Hash("STAMINA"), 100, 1);
		STATS::STAT_SET_INT(Utils::Hash("STRENGTH"), 100, 1);
		STATS::STAT_SET_INT(Utils::Hash("LUNG_CAPACITY"), 100, 1);
		STATS::STAT_SET_INT(Utils::Hash("FLYING_ABILITY"), 100, 1);
		STATS::STAT_SET_INT(Utils::Hash("SHOOTING_ABILITY"), 100, 1);
		STATS::STAT_SET_INT(Utils::Hash("STEALTH_ABILITY"), 100, 1);
	}
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
	bsOut.Write((MessageID)ID_SEND_PLAYER_DATA);
	OnFootSyncData data;
	GetOnFootSync(data);
	lastSendSeat = data.vehseat;
	bsOut.Write(data);

	CNetworkConnection::Get()->client->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);

	if (PED::IS_PED_IN_ANY_VEHICLE(Handle, false) && GetSeat() == -1) {
		RakNet::BitStream bsOut2;
		bsOut2.Write((MessageID)ID_SEND_VEHICLE_DATA);
		VehicleData data;
		GetVehicleSync(data);
		bsOut2.Write(data);

		CNetworkConnection::Get()->client->Send(&bsOut2, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	}

	//log << "Using: " << PED::GET_VEHICLE_PED_IS_USING(Handle) << std::endl;
	//log << "In(false): " << PED::GET_VEHICLE_PED_IS_IN(Handle, false) << std::endl;
	//log << "In(true): " << PED::GET_VEHICLE_PED_IS_IN(Handle, true) << std::endl;
	//log << "Trying: " << PED::GET_VEHICLE_PED_IS_TRYING_TO_ENTER(Handle) << std::endl;
}

short CLocalPlayer::GetSeat()
{
	short seat = -2;
	Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(Handle, false);
	for (int i = -1; i < VEHICLE::GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(veh); i++)
		if (!VEHICLE::IS_VEHICLE_SEAT_FREE(veh, i) && VEHICLE::GET_PED_IN_VEHICLE_SEAT(veh, i) == Handle) seat = i;
	return seat;
}

void CLocalPlayer::GoPassenger()
{
	Vehicle veh;
	float smallestDistance = 0;

	if (PED::IS_PED_IN_ANY_VEHICLE(Handle, true)) return;

	_togopassenger = false;

	for each(auto tveh in CNetworkVehicle::All())
	{
		if ((tveh->GetPosition() - GetPosition()).Length() <= smallestDistance || smallestDistance == 0)
		{
			smallestDistance = (tveh->GetPosition() - GetPosition()).Length();
			veh = tveh->GetHandle();
		}
	}

	if (smallestDistance > 10.f) return;

	AI::CLEAR_PED_TASKS(Handle);
	for (int seat = 0; seat < VEHICLE::GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(veh); seat++)
	{
		if (VEHICLE::IS_VEHICLE_SEAT_FREE(veh, seat))
		{
			AI::TASK_ENTER_VEHICLE(Handle, veh, -1, seat, 2.f, 0, 0);
			return;
		}
	}
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