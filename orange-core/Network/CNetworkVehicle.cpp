#include "stdafx.h"

std::vector<CNetworkVehicle *> CNetworkVehicle::VehiclePool;
Hash __adder = Utils::Hash("adder");

CNetworkVehicle::CNetworkVehicle():CVehicle(__adder, 0, 0, 0, 0, false)
{
	m_Model = 0;
	m_futureModel = 0;
	VehiclePool.push_back(this);
}

void CNetworkVehicle::Init()
{
	m_Inited = true;
	//log << " ShouldHaveDriver: " << m_ShouldHasDriver << std::endl;
	if (m_ShouldHasDriver) {
		//log << "DGUID: " << m_DriverGUID.ToString() << std::endl;
		CNetworkPlayer *pl = CNetworkPlayer::GetByGUID(m_DriverGUID, false);
		if (pl)
		{
			//log << "Handle: " << pl->GetHandle() << " Seat: " << pl->m_FutureSeat << " Player: 0x" << std::hex << pl << " GUID: " << m_DriverGUID.ToString() << std::endl;
			PED::SET_PED_INTO_VEHICLE(pl->GetHandle(), Handle, pl->m_FutureSeat);
			if (!PED::IS_PED_IN_ANY_VEHICLE(pl->GetHandle(), false)) {
				m_Inited = false;
				//log << "Not set" << std::endl;
			}
		}
		else m_Inited = false;
	}
}

void CNetworkVehicle::UpdateModel()
{
	m_Model = m_futureModel;
	CVector3 curPos = GetPosition();
	float curHead = GetHeading();

	VEHICLE::DELETE_VEHICLE(&Handle);

	if (STREAMING::IS_MODEL_IN_CDIMAGE(m_Model) && STREAMING::IS_MODEL_VALID(m_Model) && STREAMING::IS_MODEL_A_VEHICLE(m_Model))
	{
		STREAMING::REQUEST_MODEL(m_Model);
		while (!STREAMING::HAS_MODEL_LOADED(m_Model))
			scriptWait(0);
		Handle = VEHICLE::CREATE_VEHICLE(m_Model, curPos.fX, curPos.fY, curPos.fZ, curHead, true, true);
		VEHICLE::SET_VEHICLE_COLOURS(Handle, m_Color1, m_Color2);
		VEHICLE::SET_VEHICLE_SIREN(Handle, m_Siren);
		//VEHICLE::SET_VEHICLE_ENGINE_ON(Handle, m_EngineState, m_EngineLocked, true);
		VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(Handle, !m_TyresBulletproof);
		VEHICLE::SET_VEHICLE_EXPLODES_ON_HIGH_EXPLOSION_DAMAGE(Handle, false);
		VEHICLE::SET_VEHICLE_DOORS_LOCKED(Handle, m_Locked == false ? 0 : 2);
		if (blip)
		{
			blip->Handle = UI::ADD_BLIP_FOR_ENTITY(Handle);
			blip->Update();
		}
	}
}

void CNetworkVehicle::UpdateTargetPosition()
{
	if (HasTargetPosition())
	{
		unsigned long ulCurrentTime = timeGetTime();
		float fAlpha = Math::Unlerp(m_interp.pos.ulStartTime, ulCurrentTime, m_interp.pos.ulFinishTime);

		fAlpha = Math::Clamp(0.0f, fAlpha, 1.5f);

		// Get the current error portion to compensate
		float fCurrentAlpha = (fAlpha - m_interp.pos.fLastAlpha);
		m_interp.pos.fLastAlpha = fAlpha;

		// Apply the error compensation
		CVector3 vecCompensation = Math::Lerp(CVector3(), fCurrentAlpha, m_interp.pos.vecError);

		// If we finished compensating the error, finish it for the next pulse
		if (fAlpha > 10.f)
		{
			ENTITY::SET_ENTITY_VELOCITY(Handle, 0, 0, 0);
			m_RPM = 0.2;
			m_Horn = false;
			m_interp.pos.ulFinishTime = 0;
		}

		// Get our position
		CVector3 vecCurrentPosition = GetPosition();

		// Calculate the new position
		CVector3 vecNewPosition = (vecCurrentPosition + vecCompensation);

		// Check if the distance to interpolate is too far
		if ((vecCurrentPosition - m_interp.pos.vecTarget).Length() > 50)
		{
			// Abort all interpolation
			m_interp.pos.ulFinishTime = 0;
			vecNewPosition = m_interp.pos.vecTarget;
		}

		// Set our new position
		SetPosition(vecNewPosition);
	}
}

void CNetworkVehicle::UpdateTargetRotation()
{
	if (HasTargetRotation())
	{
		unsigned long ulCurrentTime = timeGetTime();
		float fAlpha = Math::UnlerpClamped(m_interp.rot.ulStartTime, ulCurrentTime, m_interp.rot.ulFinishTime);

		// Get the current error portion to compensate
		float fCurrentAlpha = (fAlpha - m_interp.rot.fLastAlpha);
		m_interp.rot.fLastAlpha = fAlpha;

		// Apply the error compensation
		CVector3 vecCompensation = Math::Lerp(CVector3(), fCurrentAlpha, m_interp.rot.vecError);

		// If we finished compensating the error, finish it for the next pulse
		if (fAlpha == 1.0f)
			m_interp.rot.ulFinishTime = 0;

		// Get our position
		CVector3 vecCurrentRotation = GetRotation();

		// Calculate the new position
		CVector3 vecNewRotation = (vecCurrentRotation + vecCompensation);

		// Set our new position
		SetRotation(vecNewRotation);
	}
}

void CNetworkVehicle::SetTargetPosition(const CVector3 & vecPosition)
{
	m_interp.pos.vecTarget = vecPosition;
}

void CNetworkVehicle::SetTargetPosition(const CVector3& vecPosition, unsigned long ulDelay)
{

	if (!VEHICLE::_IS_VEHICLE_ENGINE_ON(Handle) && m_EngineState)
	{
		//VEHICLE::SET_VEHICLE_ENGINE_ON(Handle, true, true, true);
	}

	// Update our target position
	UpdateTargetPosition();

	// Get our position
	CVector3 vecCurrentPosition = GetPosition();

	// Set the target position
	m_interp.pos.vecTarget = vecPosition;

	// Calculate the relative error
	m_interp.pos.vecError = (vecPosition - vecCurrentPosition);

	// Get the interpolation interval
	unsigned long ulTime = timeGetTime();
	m_interp.pos.ulStartTime = ulTime;
	m_interp.pos.ulFinishTime = (ulTime + ulDelay);

	// Initialize the interpolation
	m_interp.pos.fLastAlpha = 0.0f;
}

void CNetworkVehicle::SetTargetRotation(const CVector3& vecRotation)
{
	m_interp.rot.vecTarget = vecRotation;
}

void CNetworkVehicle::SetTargetRotation(const CVector3& vecRotation, unsigned long ulDelay)
{
	// Update our target position
	UpdateTargetRotation();

	// Get our position
	CVector3 vecCurrentRotation = GetRotation();

	// Set the target position
	m_interp.rot.vecTarget = vecRotation;

	// Calculate the relative error
	m_interp.rot.vecError = Math::GetOffsetDegrees(vecCurrentRotation, vecRotation);

	// Get the interpolation interval
	unsigned long ulTime = timeGetTime();
	m_interp.rot.ulStartTime = ulTime;
	m_interp.rot.ulFinishTime = (ulTime + ulDelay);

	// Initialize the interpolation
	m_interp.rot.fLastAlpha = 0.0f;
}

void CNetworkVehicle::Interpolate()
{
	if (m_Model != m_futureModel) UpdateModel();
	if (Handle == 0) return;
	if (PED::GET_VEHICLE_PED_IS_IN(CLocalPlayer::Get()->GetHandle(), false) != Handle || CLocalPlayer::Get()->GetSeat() != -1)
	{
		UpdateTargetRotation();
		UpdateTargetPosition();
		BuildTasksQueue();
	}
}

void CNetworkVehicle::BuildTasksQueue()
{
	if (tasksToIgnore > 0)
	{
		tasksToIgnore--;
		return;
	}
	if (Handle == 0) return;

	if (HasTargetPosition()) ENTITY::SET_ENTITY_VELOCITY(Handle, m_vecMove.fX, m_vecMove.fY, m_vecMove.fZ);

	if (m_MoveSpeed != .0f)
	{
		if (m_hasDriver)
		{
			if (VEHICLE::IS_THIS_MODEL_A_CAR(m_Model) || VEHICLE::IS_THIS_MODEL_A_BIKE(m_Model))
			{
				CVector3 curPos = GetPosition();

					//if (m_hasDriver) AI::TASK_VEHICLE_DRIVE_TO_COORD(m_Driver, Handle, curPos.fX + m_vecMove.fX, curPos.fY + m_vecMove.fY, curPos.fZ + m_vecMove.fZ, 100, 0, m_Model, 16, 0.f, 0.f);
					//VEHICLE::SET_VEHICLE_FORWARD_SPEED(Handle, m_MoveSpeed);

				if (m_Burnout) {
					//AI::TASK_VEHICLE_TEMP_ACTION(m_Driver, Handle, 23, 200);
					//AI::TASK_VEHICLE_TEMP_ACTION(m_Driver, Handle, 30, 200);
					//AI::TASK_VEHICLE_TEMP_ACTION(m_Driver, Handle, 6, 2000);
				} //else if (m_RPM > 0.9 && m_MoveSpeed < 0.025) AI::TASK_VEHICLE_TEMP_ACTION(m_Driver, Handle, 31, 2000);
			}
			if (VEHICLE::IS_THIS_MODEL_A_PLANE(m_Model))
			{
				VEHICLE::_0xB8FBC8B1330CA9B4(Handle, true);
			}
		}
	}
	else
	{
		//ENTITY::SET_ENTITY_VELOCITY(Handle, 0, 0, 0);
		//if (m_hasDriver) AI::TASK_VEHICLE_TEMP_ACTION(m_Driver, Handle, 1, 2000);
	}

	if(m_Horn) VEHICLE::START_VEHICLE_HORN(Handle, 40, 0x4f485502, 0);

	SetHealth(m_Health);
	VEHICLE::SET_VEHICLE_BODY_HEALTH(Handle, m_BodyHealth);
	VEHICLE::SET_VEHICLE_ENGINE_HEALTH(Handle, m_EngineHealth);
	VEHICLE::SET_VEHICLE_PETROL_TANK_HEALTH(Handle, m_TankHealth);

	if (!ENTITY::IS_AN_ENTITY(Handle) || !ENTITY::DOES_ENTITY_EXIST(Handle))
	{
		//log << "Veh: " << Handle << " " << ENTITY::DOES_ENTITY_EXIST(Handle) << " " << ENTITY::IS_AN_ENTITY(Handle) << std::endl;
		return;
	}
	
	//log << "Veh: " << Handle << " " << m_steering << std::endl;

	*CMemory(GetAddress()).get<float>(0x8CC) = m_steering / 180 * PI;
	*CMemory(GetAddress()).get<float>(0x7F4) = m_RPM;
}

void CNetworkVehicle::UpdateLastTickTime()
{
	updateTick = (timeGetTime() - lastTick);
	lastTick = timeGetTime();
}

int CNetworkVehicle::GetTickTime()
{
	return updateTick;
}

void CNetworkVehicle::SetVehicleData(VehicleData data, unsigned long ulDelay)
{
	m_ShouldHasDriver = data.hasDriver;
	m_hasDriver = m_ShouldHasDriver;
	if (m_hasDriver && data.driver != UNASSIGNED_RAKNET_GUID) {
		m_DriverGUID = data.driver;
		CNetworkPlayer *pl = CNetworkPlayer::GetByGUID(data.driver, false);
		if (pl)
		{
			m_Driver = pl->GetHandle();
			if (PED::GET_VEHICLE_PED_IS_IN(m_Driver, false) != Handle) m_hasDriver = false;
			else pl->m_FutureSeat = -1;
		}
	}
	else m_hasDriver = false;

	m_steering = data.steering;
	m_vecMove = data.vecMoveSpeed;
	m_MoveSpeed = m_vecMove.Length();
	m_futureModel = data.hashModel;
	m_GUID = data.GUID;

	m_Burnout = data.Burnout;
	m_RPM = data.RPM;
	
	SetTargetPosition(data.vecPos, ulDelay);
	SetTargetRotation(data.vecRot, ulDelay);

	m_Health = data.usHealth;
	m_EngineHealth = data.fEngineHealth;
	m_BodyHealth = data.fBodyHealth;
	m_TankHealth = data.fTankHealth;
	m_Drivable = data.bDrivable;

	m_TyresBulletproof = data.bTyresBulletproof;
	m_EngineState = data.bEngineStatus;
	m_Color1 = data.Color1;
	m_Color2 = data.Color2;

	m_Locked = data.bLocked;

	if (!m_Exploded && m_EngineHealth < 0 && m_TankHealth < 0 && (m_EngineHealth == -4000 || m_TankHealth == -1000))
	{
		NETWORK::NETWORK_EXPLODE_VEHICLE(Handle, 1, 0, 0);
		m_Exploded = true;
	}

	m_Horn = data.bHorn;

	if (m_Siren != data.bSirenState) {
		m_Siren = data.bSirenState;
		VEHICLE::SET_VEHICLE_SIREN(Handle, m_Siren);
	}
}


CNetworkVehicle::~CNetworkVehicle()
{
	VEHICLE::DELETE_VEHICLE(&Handle);
}

std::vector<CNetworkVehicle*> CNetworkVehicle::All()
{
	return VehiclePool;
}

void CNetworkVehicle::Clear()
{
	for each(CNetworkVehicle* veh in VehiclePool)
		delete veh;

	VehiclePool.erase(VehiclePool.begin(), VehiclePool.end());
}

CNetworkVehicle * CNetworkVehicle::GetByHandle(Vehicle veh)
{
	for each (CNetworkVehicle *_vehicle in VehiclePool)
	{
		if (_vehicle->Handle == veh)
			return _vehicle;
	}
	return nullptr;
}

CNetworkVehicle * CNetworkVehicle::GetByGUID(RakNet::RakNetGUID GUID)
{
	for each (CNetworkVehicle *_vehicle in VehiclePool)
	{
		if (_vehicle->m_GUID == GUID)
			return _vehicle;
	}
	return nullptr;
}

void CNetworkVehicle::Delete(RakNet::RakNetGUID GUID)
{
	for(int i = 0; i < VehiclePool.size(); ++i)
	{
		if (VehiclePool[i]->m_GUID == GUID)
		{
			delete VehiclePool[i];
			VehiclePool.erase(VehiclePool.begin() + i);
			break;
		}
	}
}

void CNetworkVehicle::Tick()
{
	//log << "Count: " << VehiclePool.size() << std::endl;
	for each (CNetworkVehicle * veh in VehiclePool)
	{
		//log << "Kek: " << veh->m_interp.pos.vecTarget.ToString() << std::endl << CLocalPlayer::Get()->GetPosition().ToString() << std::endl;
		if (PED::IS_PED_IN_ANY_VEHICLE(CLocalPlayer::Get()->GetHandle(), true) && PED::GET_VEHICLE_PED_IS_USING(CLocalPlayer::Get()->GetHandle()) == veh->Handle && CLocalPlayer::Get()->GetSeat() == -1) continue;
		if ((veh->m_interp.pos.vecTarget - CLocalPlayer::Get()->GetPosition()).Length() < 300)
		{
			//log << "Kek" << std::endl;
			if (!veh->m_bVisible)
			{
				//log << "Kek2" << std::endl;
				if (STREAMING::IS_MODEL_IN_CDIMAGE(veh->m_Model) && STREAMING::IS_MODEL_VALID(veh->m_Model) && STREAMING::IS_MODEL_A_VEHICLE(veh->m_Model))
				{
					STREAMING::REQUEST_MODEL(veh->m_Model);
					while (!STREAMING::HAS_MODEL_LOADED(veh->m_Model))
						scriptWait(0);
					veh->Handle = VEHICLE::CREATE_VEHICLE(veh->m_Model, veh->m_interp.pos.vecTarget.fX, veh->m_interp.pos.vecTarget.fY, veh->m_interp.pos.vecTarget.fZ, veh->m_interp.rot.vecTarget.fZ, true, true);
					VEHICLE::SET_VEHICLE_COLOURS(veh->Handle, veh->m_Color1, veh->m_Color2);
					VEHICLE::SET_VEHICLE_SIREN(veh->Handle, veh->m_Siren);
					//VEHICLE::SET_VEHICLE_ENGINE_ON(veh->Handle, veh->m_EngineState, veh->m_EngineLocked, true);
					VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(veh->Handle, !veh->m_TyresBulletproof);
					VEHICLE::SET_VEHICLE_DOORS_LOCKED(veh->Handle, veh->m_Locked == false ? 0 : 2);
					if (veh->blip)
					{
						veh->blip->Handle = UI::ADD_BLIP_FOR_ENTITY(veh->Handle);
						veh->blip->Update();
					}
				}
				veh->m_bVisible = true;
			}
		}
		else
		{
			if (veh->m_bVisible)
			{
				VEHICLE::DELETE_VEHICLE(&veh->Handle);
				veh->m_bVisible = false;
			}
		}
		if (!veh->m_bVisible) continue;
		if (!veh->m_Inited) veh->Init();
		veh->Interpolate();
	}
}

void CNetworkVehicle::SetTargetColours(int color1, int color2)
{
	m_Color1 = color1;
	m_Color2 = color2;
	VEHICLE::SET_VEHICLE_COLOURS(Handle, m_Color1, m_Color2);
}

void CNetworkVehicle::SetTargetTyresBulletproof(bool bulletproof)
{
	if (m_TyresBulletproof != bulletproof) {
		m_TyresBulletproof = bulletproof;
		VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(Handle, !m_TyresBulletproof);
	}
}

void CNetworkVehicle::SetTargetEngineStatus(bool state, bool locked)
{
	if (m_EngineState != state || m_EngineLocked != locked) {
		m_EngineState = state;
		m_EngineLocked = locked;
		VEHICLE::SET_VEHICLE_ENGINE_ON(Handle, m_EngineState, true, true);
	}
}

void CNetworkVehicle::SetTargetBodyHealth(float health)
{
	m_BodyHealth = health;
	VEHICLE::SET_VEHICLE_BODY_HEALTH(Handle, health);
}

void CNetworkVehicle::SetTargetEngineHealth(float health)
{
	m_EngineHealth = health;
	VEHICLE::SET_VEHICLE_ENGINE_HEALTH(Handle, m_EngineHealth);
}

void CNetworkVehicle::SetTargetTankHealth(float health)
{
	m_TankHealth = health;
	VEHICLE::SET_VEHICLE_PETROL_TANK_HEALTH(Handle, m_TankHealth);
}

void CNetworkVehicle::SetTargetSiren(bool state)
{
	if (m_Siren != state) {
		m_Siren = state;
		VEHICLE::SET_VEHICLE_SIREN(Handle, m_Siren);
	}
}

void CNetworkVehicle::SetTargetLocked(bool locked)
{
	if (m_Locked != locked) {
		m_Locked = locked;
		VEHICLE::SET_VEHICLE_DOORS_LOCKED(Handle, locked == false ? 0 : 2);
	}
}