#include "stdafx.h"

std::vector<CNetworkPlayer *> CNetworkPlayer::PlayersPool;
Hash CNetworkPlayer::hFutureModel = 0;
int CNetworkPlayer::ignoreTasks = 0;

CNetworkPlayer::CNetworkPlayer() :CPedestrian(0)
{
	PlayersPool.push_back(this);
	m_Model = hFutureModel;
	Spawn({ 0.f, 0.f, 0.f });
}

CNetworkPlayer* CNetworkPlayer::GetByGUID(RakNet::RakNetGUID GUID, bool create)
{
	for each (CNetworkPlayer *_player in PlayersPool)
	{
		if (_player->m_GUID == GUID)
			return _player;
	}
	if (create)
	{
		CNetworkPlayer *_newPlayer = new CNetworkPlayer();
		_newPlayer->m_GUID = GUID;
		return _newPlayer;
	}
	return nullptr;
}

CNetworkPlayer * CNetworkPlayer::GetByPedPtr(CPed * ped)
{
	for each (CNetworkPlayer *_player in PlayersPool)
		for (int i = 0, cnt = 0; i < ReplayInterfaces::Get()->ReplayInterfacePed->pool.Capacity(); ++i)
			if (ReplayInterfaces::Get()->ReplayInterfacePed->pool.GetHandle(i) == _player->GetHandle())
				return _player;
	return nullptr;
}

bool CNetworkPlayer::Exists(RakNet::RakNetGUID GUID)
{
	for each (CNetworkPlayer *_player in PlayersPool)
	{
		if (_player->m_GUID == GUID)
			return true;
	}
	return false;
}

CNetworkPlayer* CNetworkPlayer::GetByHandler(Entity handler)
{
	for each (CNetworkPlayer *_player in PlayersPool)
	{
		if (_player->Handle == handler)
			return _player;
	}
	return nullptr;
}

void CNetworkPlayer::Clear()
{
	for each(CNetworkPlayer* player in PlayersPool)
	{
		PED::DELETE_PED(&(player->Handle));
		delete player;
	}
	PlayersPool.erase(PlayersPool.begin(), PlayersPool.end());
}

void CNetworkPlayer::Tick()
{
	for each (CNetworkPlayer * player in PlayersPool)
	{
		if (player->IsSpawned())
		{
			player->Interpolate();
		}
	}
}

void CNetworkPlayer::PreRender()
{
	for each (CNetworkPlayer * player in PlayersPool)
	{
		if (player->IsSpawned())
		{
			player->MakeTag();
		}
	}
}

void CNetworkPlayer::Render()
{
	for each (CNetworkPlayer * player in PlayersPool)
	{
		if (player->IsSpawned())
		{
			player->DrawTag();
		}
	}
}

void CNetworkPlayer::UpdateLastTickTime()
{
	updateTick = (timeGetTime() - lastTick);
	lastTick = timeGetTime();
}

int CNetworkPlayer::GetTickTime()
{
	return updateTick;
}

std::vector<CNetworkPlayer*> CNetworkPlayer::All()
{
	return PlayersPool;
}

void CNetworkPlayer::DeleteByGUID(RakNet::RakNetGUID guid)
{
	for (int i = 0; i < PlayersPool.size(); ++i)
	{
		if (PlayersPool[i]->m_GUID == guid)
		{
			PED::DELETE_PED(&PlayersPool[i]->Handle);
			delete PlayersPool[i];
			PlayersPool.erase(PlayersPool.begin() + i, PlayersPool.begin() + i + 1);
		}
	}
}

void CNetworkPlayer::Spawn(const CVector3& vecPosition)
{
	m_Spawned = true;
	if (STREAMING::IS_MODEL_IN_CDIMAGE(m_Model) && STREAMING::IS_MODEL_VALID(m_Model))
	{
		STREAMING::REQUEST_MODEL(m_Model);
	while (!STREAMING::HAS_MODEL_LOADED(m_Model))
		scriptWait(0);
	Handle = PED::CREATE_PED(1, m_Model, vecPosition.fX, vecPosition.fY, vecPosition.fZ, .0f, true, true);
	pedHandler = (rage::CPed*)CPed::GetFromScriptID(Handle);
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(m_Model);
	AI::TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(Handle, true);
	PED::SET_PED_CAN_RAGDOLL_FROM_PLAYER_IMPACT(Handle, false);
	PED::SET_PED_FLEE_ATTRIBUTES(Handle, 0, 0);
	PED::SET_PED_COMBAT_ATTRIBUTES(Handle, 17, 1);
	PED::SET_PED_CAN_RAGDOLL(Handle, false);
	//PED::_SET_PED_RAGDOLL_FLAG(Handle, 1 | 2 | 4);
#if _DEBUG
	//pedHandler->Flags |= 1 << 30;
#endif
	//pedHandler->Flags |= 1 << 6;
	ENTITY::SET_ENTITY_PROOFS(Handle, true, true, true, true, true, true, true, true);

		Blip blip = AddBlip();
		UI::SET_BLIP_AS_SHORT_RANGE(blip, false);
	}
}

void CNetworkPlayer::SetTargetPosition(const CVector3& vecPosition, unsigned long ulDelay)
{
	// Are we spawned?
	if (IsSpawned())
	{
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
}

void CNetworkPlayer::SetTargetRotation(const CVector3& vecRotation, unsigned long ulDelay)
{
	// Are we spawned?
	if (IsSpawned())
	{
		// Update our target position
		//UpdateTargetRotation();

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
}

void CNetworkPlayer::SetOnFootData(OnFootSyncData data, unsigned long ulDelay)
{
	SetHealth(data.usHealth);
	m_Health = data.usHealth;
	if (m_Health <= 100.f)
		return;
	else if(pedJustDead)
	{
		if (ENTITY::IS_ENTITY_A_PED(Handle))
			PED::DELETE_PED(&Handle);
		Spawn(data.vecPos);
		pedJustDead = false;
	}

	if (m_InVehicle && !data.bInVehicle)
	{
		//AI::CLEAR_PED_TASKS(Handle);
		//AI::CLEAR_PED_SECONDARY_TASK(Handle);
		//AI::CLEAR_PED_TASKS_IMMEDIATELY(Handle);
		AI::TASK_LEAVE_VEHICLE(Handle, PED::GET_VEHICLE_PED_IS_IN(Handle, false), (CLocalPlayer::Get()->GetPosition() - GetPosition()).Length() > 50 ? 16 : 0);
		m_Entering = false;
		m_Lefting = true;
	}

	m_InVehicle = data.bInVehicle;

	if (data.bRagdoll != m_Ragdoll) {
		m_Ragdoll = data.bRagdoll;
		PED::SET_PED_CAN_RAGDOLL(Handle, m_Ragdoll);
	}

	if (!m_InVehicle && !m_Lefting)
	{
		if (data.hModel != m_Model)
			SetModel(data.hModel);
		//m_MoveSpeed = data.fForwardPotential;
		m_vecMove = data.vecMoveSpeed;
		m_Jumping = data.bJumping;
		m_Aiming = data.bAiming;
		m_Shooting = data.bShooting;
		m_vecAim = data.vecAim;
		SetTargetPosition(data.vecPos, ulDelay);
		//if (!m_Aiming && !m_Shooting)
		//	SetTargetRotation(data.vecRot, ulDelay);
		SetArmour(data.usArmour);
		if (GetCurrentWeapon() != data.ulWeapon)
			SetCurrentWeapon(data.ulWeapon, true);
		//SetAmmo(data.ulWeapon, 9999);
		//SetDucking(data.bDuckState);
		m_Ducking = data.bDuckState;
		//SetMovementVelocity(data.vecMoveSpeed);

		pedHandler->fForwardPotential = data.fForwardPotential;
		pedHandler->fStrafePotential = data.fStrafePotential;
		pedHandler->fRotationPotential = data.fRotationPotential;
		pedHandler->dwMovementFlags = data.dwMovementFlags;


		m_Entering = false;
	}
	else {
		m_Vehicle = data.rnVehicle;
		m_FutureSeat = data.cSeat;
	}
}

void CNetworkPlayer::UpdateTargetPosition()
{
	if (HasTargetPosition() && !m_Lefting)
	{
		unsigned long ulCurrentTime = timeGetTime();
		float fAlpha = Math::UnlerpClamped(m_interp.pos.ulStartTime, ulCurrentTime, m_interp.pos.ulFinishTime);

		// Get the current error portion to compensate
		float fCurrentAlpha = (fAlpha - m_interp.pos.fLastAlpha);
		m_interp.pos.fLastAlpha = fAlpha;

		// Apply the error compensation
		CVector3 vecCompensation = Math::Lerp(CVector3(), fCurrentAlpha, m_interp.pos.vecError);

		if (m_Ragdoll) vecCompensation.fZ = 0;

		// If we finished compensating the error, finish it for the next pulse
		if (fAlpha == 1.0f)
			m_interp.pos.ulFinishTime = 0;

		// Get our position
		CVector3 vecCurrentPosition = GetPosition();

		// Calculate the new position
		CVector3 vecNewPosition = (vecCurrentPosition + vecCompensation);

		// Set our new position
		SetPosition(vecNewPosition, false);
	}
}

void CNetworkPlayer::UpdateTargetRotation()
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
		SetRotation(vecNewRotation, false);
	}
}

bool CNetworkPlayer::IsSpawned()
{
	return m_Spawned;
}

void CNetworkPlayer::SetPosition(const CVector3& vecPosition, bool bResetInterpolation)
{
	// Are we spawned?
	if (IsSpawned())
	{
		// Are we not in a vehicle and not entering a vehicle?
		//if (!InternalIsInVehicle() && !HasVehicleEnterExit())
		{
			// Set the position in the matrix
			CEntity::SetPosition(vecPosition);

			//m_pPlayerPed->GetPed()->UpdatePhysicsMatrix(true);

			//if (GetHealth() > 0 && IsDying())
			//	m_pPlayerPed->GetPedTaskManager()->RemoveTask(TASK_PRIORITY_EVENT_RESPONSE_NONTEMP);
		}
	}

	// Reset interpolation if requested
	if (bResetInterpolation)
		ResetInterpolation();
}

void CNetworkPlayer::SetRotation(const CVector3& vecRotation, bool bResetInterpolation)
{
	// Are we spawned?
	if (IsSpawned())
	{
		// Are we not in a vehicle and not entering a vehicle?
		//if (!InternalIsInVehicle() && !HasVehicleEnterExit())
		{
			// Set the position in the matrix
			CEntity::SetRotation(vecRotation);

			//m_pPlayerPed->GetPed()->UpdatePhysicsMatrix(true);

			//if (GetHealth() > 0 && IsDying())
			//	m_pPlayerPed->GetPedTaskManager()->RemoveTask(TASK_PRIORITY_EVENT_RESPONSE_NONTEMP);
		}
	}

	// Reset interpolation if requested
	if (bResetInterpolation)
		ResetInterpolation();
}

void CNetworkPlayer::Interpolate()
{
	if (PED::IS_PED_DEAD_OR_DYING(Handle, true) && !pedJustDead)
	{
		pedJustDead = true;
		ENTITY::DELETE_ENTITY(&Handle);
		return;
	}
		
	if (!m_InVehicle && !m_Lefting)
	{
		//if (!m_Shooting && !m_Aiming)
		//	UpdateTargetRotation();
		UpdateTargetPosition();
		//SetMovementVelocity(m_vecMove);
		PED::SET_PED_ACCURACY(Handle, 100);
	}
	//BuildTasksQueue();
}

void CNetworkPlayer::SetMoveToDirection(CVector3 vecPos, CVector3 vecMove, float iMoveSpeed)
{
	if (IsSpawned())
	{
		float tX = (vecPos.fX + (vecMove.fX * 10));
		float tY = (vecPos.fY + (vecMove.fY * 10));
		float tZ = (vecPos.fZ + (vecMove.fZ * 10));
		TaskMove(CVector3(tX, tY, tZ), iMoveSpeed);
	}
}

void CNetworkPlayer::SetMoveToDirectionAndAiming(CVector3 vecPos, CVector3 vecMove, CVector3 aimPos, float moveSpeed, bool shooting)
{
	if (IsSpawned())
	{
		float tX = (vecPos.fX + (vecMove.fX * 10));
		float tY = (vecPos.fY + (vecMove.fY * 10));
		float tZ = (vecPos.fZ + (vecMove.fZ * 10));
		//MemoryHook::call<void, Ped, float, float, float, float, float, float, float, BOOL, float, float, BOOL, Any, BOOL, Hash>((*GTA::CAddress::Get())[PED_TASK_AIM_AT_COORD_AND_STAND_STILL], Handle, tX, tY,
		//	tZ, aimPos.fX, aimPos.fY, aimPos.fZ, 1.f, 0, 0x3F000000, 0x40800000, 1, (shooting ? 0 : 1024), 1, 3337513804U);
		AI::TASK_GO_TO_COORD_WHILE_AIMING_AT_COORD(Handle, tX, tY,
			tZ, aimPos.fX, aimPos.fY, aimPos.fZ, moveSpeed, 0, 0x3F000000, 0x40800000, 1, (shooting ? 0 : 1024), 1, 3337513804U);
	}
}

void CNetworkPlayer::AssignTask(GTA::CTask *task)
{
	if (IsSpawned())
	{
		pedHandler->pedIntelligence->pedTaskManager.Primary->AssignTask((rage::CTask*)task, GTA::TASK_PRIORITY_NORMAL);
	}
}

void CNetworkPlayer::BuildTasksQueue()
{
	/*if (tasksToIgnore > 0)
	{
		tasksToIgnore--;
		return;
	}*/
	if (m_InVehicle)
	{
		//if (m_Entering && !PED::IS_PED_IN_ANY_VEHICLE(Handle, false)) return;
		if (!m_Entering)
		{
			CNetworkVehicle *veh = CNetworkVehicle::GetByGUID(m_Vehicle);
			if (veh)
			{
				m_Entering = true;
				/*AI::CLEAR_PED_TASKS(Handle);
				AI::CLEAR_PED_SECONDARY_TASK(Handle);
				AI::CLEAR_PED_TASKS_IMMEDIATELY(Handle);*/
				m_Seat = m_FutureSeat;
				PED::SET_PED_INTO_VEHICLE(Handle, veh->GetHandle(), -1);
				//AI::TASK_ENTER_VEHICLE(Handle, veh->GetHandle(), -1, m_Seat, 2, 0, 0);
			}
		}
		/*else if (m_FutureSeat != m_Seat)
		{
			CNetworkVehicle *veh = CNetworkVehicle::GetByGUID(m_Vehicle);
			if (veh)
			{
				log << "shuffle: " << m_Seat << " => " << m_FutureSeat << std::endl;
				AI::CLEAR_PED_TASKS(Handle);
				AI::TASK_SHUFFLE_TO_NEXT_VEHICLE_SEAT(Handle, veh->GetHandle());
				for (int i = -1; i < VEHICLE::GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(veh->GetHandle()); i++)
					if (!VEHICLE::IS_VEHICLE_SEAT_FREE(veh->GetHandle(), i) && VEHICLE::GET_PED_IN_VEHICLE_SEAT(veh->GetHandle(), i) == Handle) m_Seat = i;

			}
		}*/
	}
	/*else if (!m_Entering && !PED::IS_PED_IN_ANY_VEHICLE(Handle, false))
	{

	}*/
	/*else if (m_Entering)
	{
		AI::CLEAR_PED_TASKS(Handle);
		AI::CLEAR_PED_SECONDARY_TASK(Handle);
		AI::CLEAR_PED_TASKS_IMMEDIATELY(Handle);
		AI::TASK_LEAVE_VEHICLE(Handle, PED::GET_VEHICLE_PED_IS_IN(Handle, false), 0);
		m_Entering = false;
	}
	else if (m_Jumping)
	{
		if(!IsJumping()) TaskJump();
	}
	else if (m_Aiming && !m_Shooting)
	{
		if (m_MoveSpeed != .0f)
		{
			SetMoveToDirectionAndAiming(m_interp.pos.vecTarget, m_vecMove, m_vecAim, 3.0f, false);
		}
		else
		{
			TaskAimAt(m_vecAim, -1);
		}
	}
	else if (m_Shooting && m_MoveSpeed != .0f)
	{
		SetMoveToDirectionAndAiming(m_interp.pos.vecTarget, m_vecMove, m_vecAim, m_MoveSpeed, true);
		m_Shooting = false;
	}
	else if (m_Shooting && !m_Aiming)
	{
		TaskAimAt(m_vecAim, -1);
		TaskShootAt(m_vecAim, -1);
		m_Shooting = false;
	}
	else if (m_Shooting && m_MoveSpeed == .0f)
	{
		TaskShootAt(m_vecAim, 1);
		m_Shooting = false;
	}
	else if (m_MoveSpeed != .0f)
	{
		SetMoveToDirection(m_interp.pos.vecTarget, m_vecMove, m_MoveSpeed);
	}
	else
	{
		AI::CLEAR_PED_TASKS(Handle);
	}*/
}

void CNetworkPlayer::MakeTag()
{
	tag.bVisible = false;
	if (ENTITY::HAS_ENTITY_CLEAR_LOS_TO_ENTITY(CLocalPlayer::Get()->GetHandle(), Handle, 17))
	{
		Vector3 _camPos = CAM::GET_GAMEPLAY_CAM_COORD();
		CVector3 camPos(_camPos.x, _camPos.y, _camPos.z);

		CVector3 *vecCurPos = &pedHandler->vecPositionEntity;
		tag.distance = (((*vecCurPos) - camPos).Length() / std::powf(CAM::_GET_GAMEPLAY_CAM_ZOOM(), 1.1f));

		if (tag.distance > 70.f)
			return;

		tag.health = ((((m_Health - 100.f) < pedHandler->fMaxHealth ? (m_Health - 100.f) : pedHandler->fMaxHealth)) / (pedHandler->fMaxHealth - 100.f));
		
		if (tag.health > 1.f)
			tag.health = 1.f;

		tag.width = 0.08f * 800;
		tag.height = 0.012f * 600;

		tag.k = 1.3f - tag.distance / 100;

		CVector3 screenPos;
		CGraphics::Get()->WorldToScreen(CVector3(vecCurPos->fX, vecCurPos->fY, vecCurPos->fZ + 1.1f * tag.k + (tag.distance * 0.04f)), screenPos);
		auto viewPortGame = GTA::CViewportGame::Get();
		tag.x = screenPos.fX * viewPortGame->Width;
		tag.y = screenPos.fY * viewPortGame->Height;

		tag.bVisible = true;
	}
}

void CNetworkPlayer::DrawTag()
{
	if (tag.bVisible) {
		const char* _name = m_Name.c_str();
		float font_size = 20.0f * tag.k;
		ImVec2 textSize = CGlobals::Get().tagFont->CalcTextSizeA(font_size, 1000.f, 1000.f, _name);

		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().tagFont, font_size, ImVec2(tag.x - textSize.x / 2 - 1, tag.y - 1), ImColor(0, 0, 0, 255), _name);
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().tagFont, font_size, ImVec2(tag.x - textSize.x / 2 + 1, tag.y + 1), ImColor(0, 0, 0, 255), _name);
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().tagFont, font_size, ImVec2(tag.x - textSize.x / 2 + 1, tag.y - 1), ImColor(0, 0, 0, 255), _name);
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().tagFont, font_size, ImVec2(tag.x - textSize.x / 2 - 1, tag.y + 1), ImColor(0, 0, 0, 255), _name);
		ImGui::GetWindowDrawList()->AddText(CGlobals::Get().tagFont, font_size, ImVec2(tag.x - textSize.x / 2, tag.y), ImColor(0xFF, 0xFF, 0xFF, 0xFF), _name);

		color_t bgColor, fgColor;

		if (tag.health > 0.2f)
		{
			bgColor = { 50, 100, 50, 150 };
			fgColor = { 100, 200, 100, 150 };
		}
		else
		{
			bgColor = { 150, 30, 30, 150 };
			fgColor = { 230, 70, 70, 150 };
		}

		DWORD colorOut = ImColor(bgColor.red, bgColor.green, bgColor.blue, bgColor.alpha);
		DWORD colorIn = ImColor(fgColor.red, fgColor.green, fgColor.blue, fgColor.alpha);

		float width = tag.width * tag.k;
		float height = tag.height * tag.k;

		float x = tag.x - (width / 2);
		float y = tag.y + 24 * tag.k;

		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(x - 2, y - 2), ImVec2(x + width + 2, y + height + 2), ImColor(0, 0, 0, 255), 0.f, 15);
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + width, y + height), colorOut, 0.f);
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + (width * tag.health), y + height), colorIn, 0.f);
	}
}


void CNetworkPlayer::SetModel(Hash model)
{
	m_Model = model;
	CVector3 pos = GetPosition();
	float heading = GetHeading();
	ENTITY::DELETE_ENTITY(&Handle);
	if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model))
		STREAMING::REQUEST_MODEL(model);
	while (!STREAMING::HAS_MODEL_LOADED(model))
		scriptWait(0);
	Handle = PED::CREATE_PED(1, model, pos.fX, pos.fY, pos.fZ, heading, true, true);
	pedHandler = (rage::CPed*)CPed::GetFromScriptID(Handle);
	PED::SET_PED_DEFAULT_COMPONENT_VARIATION(Handle);
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
	AI::TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(Handle, true);
	PED::SET_PED_CAN_RAGDOLL_FROM_PLAYER_IMPACT(Handle, false);
	PED::SET_PED_FLEE_ATTRIBUTES(Handle, 0, 0);
	PED::SET_PED_COMBAT_ATTRIBUTES(Handle, 17, 1);
	PED::SET_PED_CAN_RAGDOLL(Handle, m_Ragdoll);
	//PED::_SET_PED_RAGDOLL_FLAG(Handle, 1 | 2 | 4);
#if _DEBUG
	//pedHandler->Flags |= 1 << 30;
#endif
	//pedHandler->Flags |= 1 << 6;
	ENTITY::SET_ENTITY_PROOFS(Handle, true, true, true, true, true, true, true, true);
	
}

void CNetworkPlayer::RemoveTargetPosition()
{
	m_interp.pos.ulFinishTime = 0;
}

void CNetworkPlayer::RemoveTargetRotation()
{
	m_interp.rot.ulFinishTime = 0;
}

void CNetworkPlayer::ResetInterpolation()
{
	RemoveTargetPosition();
	RemoveTargetRotation();
}
