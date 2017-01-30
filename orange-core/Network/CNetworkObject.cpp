#include "stdafx.h"

std::vector<CNetworkObject *> CNetworkObject::ObjectPool;

CNetworkObject::CNetworkObject(ObjectData data) :CEntity(-1)
{
	m_Model = data.hashModel;
	m_futureModel = m_Model;
	//UpdateModel();

	scriptWait(5);
	if (STREAMING::IS_MODEL_IN_CDIMAGE(m_Model) && STREAMING::IS_MODEL_VALID(m_Model))
	{
		STREAMING::REQUEST_MODEL(m_Model);
		while (!STREAMING::HAS_MODEL_LOADED(m_Model))
			scriptWait(0);
		Handle = OBJECT::CREATE_OBJECT(m_Model, data.vecPos.fX, data.vecPos.fY, data.vecPos.fZ, false, true, false);
		ENTITY::FREEZE_ENTITY_POSITION(Handle, true);
	}

	SetRotation(data.vecRot);
	ObjectPool.push_back(this);
}

void CNetworkObject::UpdateModel()
{
	m_Model = m_futureModel;
	CVector3 curPos = GetPosition();
	float curHead = GetHeading();

	OBJECT::DELETE_OBJECT(&Handle);

	if (STREAMING::IS_MODEL_IN_CDIMAGE(m_Model) && STREAMING::IS_MODEL_VALID(m_Model))
	{
		STREAMING::REQUEST_MODEL(m_Model);
		while (!STREAMING::HAS_MODEL_LOADED(m_Model))
			scriptWait(0);
		Handle = OBJECT::CREATE_OBJECT(m_Model, curPos.fX, curPos.fY, curPos.fZ, false, true, false);
	}
}

/*void CNetworkObject::UpdateTargetPosition()
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
		if (fAlpha == 1.5f)
		{
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

void CNetworkObject::UpdateTargetRotation()
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

void CNetworkObject::SetTargetPosition(const CVector3& vecPosition, unsigned long ulDelay)
{

	if (!Object::_IS_Object_ENGINE_ON(Handle))
	{
		Object::SET_Object_ENGINE_ON(Handle, true, true, true);
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

void CNetworkObject::SetTargetRotation(const CVector3& vecRotation, unsigned long ulDelay)
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

void CNetworkObject::Interpolate()
{
	if (m_Model != m_futureModel) UpdateModel();
	if (PED::GET_Object_PED_IS_IN(CLocalPlayer::Get()->GetHandle(), false) != Handle)
	{
		UpdateTargetRotation();
		UpdateTargetPosition();
		BuildTasksQueue();
	}
}

void CNetworkObject::BuildTasksQueue()
{
	if (tasksToIgnore > 0)
	{
		tasksToIgnore--;
		return;
	}
	if (m_MoveSpeed != .0f)
	{
		ENTITY::SET_ENTITY_VELOCITY(Handle, m_vecMove.fX, m_vecMove.fY, m_vecMove.fZ);
		if (m_hasDriver)
		{
			//SetMoveToDirection(m_interp.pos.vecTarget, m_vecMove, m_MoveSpeed);

			if (Object::IS_THIS_MODEL_A_CAR(m_Model) || Object::IS_THIS_MODEL_A_BIKE(m_Model))
			{
				CVector3 curPos = GetPosition();

				//if (m_hasDriver) AI::TASK_Object_DRIVE_TO_COORD(m_Driver, Handle, curPos.fX + m_vecMove.fX, curPos.fY + m_vecMove.fY, curPos.fZ + m_vecMove.fZ, 100, 0, m_Model, 16, 0.f, 0.f);
				//Object::SET_Object_FORWARD_SPEED(Handle, m_MoveSpeed);

				if (m_Burnout) {
					AI::TASK_Object_TEMP_ACTION(m_Driver, Handle, 23, 200);
					//AI::TASK_Object_TEMP_ACTION(m_Driver, Handle, 30, 2000);
					//AI::TASK_Object_TEMP_ACTION(m_Driver, Handle, 6, 2000);
				} //else if (m_RPM > 0.9 && m_MoveSpeed < 0.025) AI::TASK_Object_TEMP_ACTION(m_Driver, Handle, 31, 2000);
			}
			if (Object::IS_THIS_MODEL_A_PLANE(m_Model))
			{
				Object::_0xB8FBC8B1330CA9B4(Handle, true);
			}
		}
	}
	else
	{
		ENTITY::SET_ENTITY_VELOCITY(Handle, 0, 0, 0);
		//if (m_hasDriver) AI::TASK_Object_TEMP_ACTION(m_Driver, Handle, 1, 2000);
	}
	SetHealth(m_Health);
	*CMemory(GetAddress()).get<float>(0x8CC) = m_steering / 180 * PI;
	*CMemory(GetAddress()).get<float>(0x7F4) = m_RPM;
}

void CNetworkObject::UpdateLastTickTime()
{
	updateTick = (timeGetTime() - lastTick);
	lastTick = timeGetTime();
}

int CNetworkObject::GetTickTime()
{
	return updateTick;
}

void CNetworkObject::SetObjectData(ObjectData data, unsigned long ulDelay)
{
	m_hasDriver = data.hasDriver;
	if (m_hasDriver && data.driver != UNASSIGNED_RAKNET_GUID) {
		CNetworkPlayer *pl = CNetworkPlayer::GetByGUID(data.driver);
		m_Driver = pl->GetHandle();
		if (PED::GET_Object_PED_IS_IN(m_Driver, false) != Handle) m_hasDriver = false;
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
}*/


CNetworkObject::~CNetworkObject()
{
	OBJECT::DELETE_OBJECT(&Handle);
}

std::vector<CNetworkObject*> CNetworkObject::All()
{
	return ObjectPool;
}

void CNetworkObject::Clear()
{
	for each(CNetworkObject* obj in ObjectPool)
	{
		obj->~CNetworkObject();
		delete obj;
	}
	ObjectPool.erase(ObjectPool.begin(), ObjectPool.end());
}

CNetworkObject * CNetworkObject::GetByHandle(Object veh)
{
	for each (CNetworkObject *_object in ObjectPool)
	{
		if (_object->Handle == veh)
			return _object;
	}
	return nullptr;
}

CNetworkObject * CNetworkObject::GetByGUID(RakNet::RakNetGUID GUID)
{
	for each (CNetworkObject *_Object in ObjectPool)
	{
		if (_Object->m_GUID == GUID)
			return _Object;
	}
	return nullptr;
}

/*void CNetworkObject::Tick()
{
	for each (CNetworkObject * veh in ObjectPool)
	{
		veh->Interpolate();
	}
}*/
