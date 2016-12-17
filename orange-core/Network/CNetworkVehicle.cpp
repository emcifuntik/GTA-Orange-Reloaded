#include "stdafx.h"

std::vector<CNetworkVehicle *> CNetworkVehicle::VehiclePool;
Hash __adder = Utils::Hash("adder");

CNetworkVehicle::CNetworkVehicle():CVehicle(__adder, 0, 0, 0, 0)
{
	log_debug << "Creating veh.." << std::endl;
	m_Model = __adder;
	m_futureModel = __adder;
	UpdateModel();
	VehiclePool.push_back(this);
}

void CNetworkVehicle::UpdateModel()
{
	//log << "Updating model: " << m_Model << " => " << m_futureModel << std::endl;

	m_Model = m_futureModel;
	CVector3 curPos = GetPosition();
	float curHead = GetHeading();

	VEHICLE::DELETE_VEHICLE(&Handle);

	if (STREAMING::IS_MODEL_IN_CDIMAGE(m_Model) && STREAMING::IS_MODEL_VALID(m_Model) && STREAMING::IS_MODEL_A_VEHICLE(m_Model))
	{
		STREAMING::REQUEST_MODEL(m_Model);
		while (!STREAMING::HAS_MODEL_LOADED(m_Model))
			scriptWait(0);
		Handle = VEHICLE::CREATE_VEHICLE(m_Model, curPos.fX, curPos.fY, curPos.fZ, curHead, false, true);
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(m_Model);
	}

	Blip blip = AddBlip();
	UI::SET_BLIP_AS_SHORT_RANGE(blip, false);
	UI::SET_BLIP_COLOUR(blip, 0);
	UI::SET_BLIP_SPRITE(blip, 225);
}

void CNetworkVehicle::UpdateTargetPosition()
{
	if (HasTargetPosition())
	{
		unsigned long ulCurrentTime = timeGetTime();
		float fAlpha = Math::UnlerpClamped(m_interp.pos.ulStartTime, ulCurrentTime, m_interp.pos.ulFinishTime);

		// Get the current error portion to compensate
		float fCurrentAlpha = (fAlpha - m_interp.pos.fLastAlpha);
		m_interp.pos.fLastAlpha = fAlpha;

		// Apply the error compensation
		CVector3 vecCompensation = Math::Lerp(CVector3(), fCurrentAlpha, m_interp.pos.vecError);

		// If we finished compensating the error, finish it for the next pulse
		if (fAlpha == 1.0f)
			m_interp.pos.ulFinishTime = 0;

		// Get our position
		CVector3 vecCurrentPosition = GetPosition();

		// Calculate the new position
		CVector3 vecNewPosition = (vecCurrentPosition + vecCompensation);

		// Check if the distance to interpolate is too far
		if ((vecCurrentPosition - m_interp.pos.vecTarget).Length() > 10)
		{
			// Abort all interpolation
			m_interp.pos.ulFinishTime = 0;
			vecNewPosition = m_interp.pos.vecTarget;
		}

		// Set our new position
		SetPosition(vecNewPosition); // , false);
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
		CVector3 vecCurrentRotation = GetPosition();

		// Calculate the new position
		CVector3 vecNewRotation = (vecCurrentRotation + vecCompensation);

		// Check if the distance to interpolate is too far
		if ((vecCurrentRotation - m_interp.rot.vecTarget).Length() > 5)
		{
			// Abort all interpolation
			m_interp.rot.ulFinishTime = 0;
			vecNewRotation = m_interp.rot.vecTarget;
		}

		// Set our new position
		SetRotation(vecNewRotation); // , false);
	}
}

void CNetworkVehicle::SetTargetPosition(const CVector3& vecPosition, unsigned long ulDelay)
{

	if (!VEHICLE::_IS_VEHICLE_ENGINE_ON(Handle))
	{
		VEHICLE::SET_VEHICLE_ENGINE_ON(Handle, true, true, true);
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

void CNetworkVehicle::SetTargetRotation(const CVector3& vecRotation, unsigned long ulDelay)
{
	// Update our target position
	UpdateTargetRotation();

	// Get our position
	CVector3 vecCurrentRotation = GetRotation();

	// Set the target position
	m_interp.rot.vecTarget = vecRotation;

	// Calculate the relative error
	m_interp.rot.vecError = (vecRotation - vecCurrentRotation);

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
	if (PED::GET_VEHICLE_PED_IS_IN(CLocalPlayer::Get()->GetHandle(), false) != Handle)
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
	if (m_MoveSpeed != .0f)
	{
		if (m_hasDriver)
		{
			//SetMoveToDirection(m_interp.pos.vecTarget, m_vecMove, m_MoveSpeed);
			ENTITY::SET_ENTITY_VELOCITY(Handle, m_vecMove.fX, m_vecMove.fY, m_vecMove.fZ);

			if (VEHICLE::IS_THIS_MODEL_A_CAR(m_Model) || VEHICLE::IS_THIS_MODEL_A_BIKE(m_Model))
			{
				CVector3 curPos = GetPosition();

					//if (m_hasDriver) AI::TASK_VEHICLE_DRIVE_TO_COORD(m_Driver, Handle, curPos.fX + m_vecMove.fX, curPos.fY + m_vecMove.fY, curPos.fZ + m_vecMove.fZ, 100, 0, m_Model, 16, 0.f, 0.f);
					//VEHICLE::SET_VEHICLE_FORWARD_SPEED(Handle, m_MoveSpeed);

				if (m_Burnout) {
					AI::TASK_VEHICLE_TEMP_ACTION(m_Driver, Handle, 23, 2000);
					//AI::TASK_VEHICLE_TEMP_ACTION(m_Driver, Handle, 30, 2000);
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
		ENTITY::SET_ENTITY_VELOCITY(Handle, 0, 0, 0);
		//if (m_hasDriver) AI::TASK_VEHICLE_TEMP_ACTION(m_Driver, Handle, 1, 2000);
	}
	*CMemory(GetAddress()).get<float>(0x8CC) = m_steering / 180 * PI;
	*CMemory(GetAddress()).get<float>(0x7F4) = m_RPM;
}

void CNetworkVehicle::SetVehicleData(VehicleData data, unsigned long ulDelay)
{
	m_hasDriver = data.hasDriver;
	if (m_hasDriver && data.driver != UNASSIGNED_RAKNET_GUID) {
		m_Driver = CNetworkPlayer::GetByGUID(data.driver)->GetHandle();
		if (PED::GET_VEHICLE_PED_IS_IN(m_Driver, false) != Handle) m_hasDriver = false;
		//log << "Driver...." << data.driver.ToString() << std::endl;
		//AI::TASK_OPEN_VEHICLE_DOOR(m_Driver, Handle, 10, -1, 2.0f);
		//AI::TASK_ENTER_VEHICLE(m_Driver, Handle, -1, -1, 2.0, 0, 0);
		//PED::SET_PED_INTO_VEHICLE(m_Driver, Handle, -1);
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
	//SetHealth(data.usHealth);
}


CNetworkVehicle::~CNetworkVehicle()
{
}

std::vector<CNetworkVehicle*> CNetworkVehicle::All()
{
	return VehiclePool;
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

void CNetworkVehicle::Tick()
{
	for each (CNetworkVehicle * veh in VehiclePool)
	{
		veh->Interpolate();
	}
}
