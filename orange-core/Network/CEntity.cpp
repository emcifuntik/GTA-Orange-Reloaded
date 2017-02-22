#include "stdafx.h"

GetEntityOffsetFunc CEntity::_entityAddressFunc;

Entity CEntity::GetHandle()
{
	return Handle;
}

CVector3 CEntity::GetRotationVelocity()
{
	Vector3 vec = ENTITY::GET_ENTITY_ROTATION_VELOCITY(Handle);
	return CVector3(vec.x, vec.y, vec.z);
}

void CEntity::SetMovementVelocity(CVector3 velocity)
{
	ENTITY::SET_ENTITY_VELOCITY(Handle, velocity.fX, velocity.fY, velocity.fZ);
}

CVector3 CEntity::GetMovementVelocity()
{
	Vector3 vec = ENTITY::GET_ENTITY_VELOCITY(Handle);
	return CVector3(vec.x, vec.y, vec.z);
}

void CEntity::GetMoveSpeed(CVector3& vecMoveSpeed)
{
	Vector3 vec = ENTITY::GET_ENTITY_SPEED_VECTOR(Handle, false);
	vecMoveSpeed = CVector3(vec.x, vec.y, vec.z);
}

bool CEntity::GetDamageToEntity()
{
	return ENTITY::HAS_ENTITY_BEEN_DAMAGED_BY_ANY_PED(Handle) ? true : false;
}
void CEntity::ClearDamage()
{
	ENTITY::CLEAR_ENTITY_LAST_DAMAGE_ENTITY(Handle);
}

float CEntity::GetSpeed()
{
	return ENTITY::GET_ENTITY_SPEED(Handle);
}

void CEntity::SetPosition(CVector3 position)
{
	ENTITY::SET_ENTITY_COORDS_NO_OFFSET(Handle, position.fX, position.fY, position.fZ, false, false, false);
}

CVector3 CEntity::GetPosition()
{
	Vector3 vec = ENTITY::GET_ENTITY_COORDS(Handle, !ENTITY::IS_ENTITY_DEAD(Handle));
	return CVector3(vec.x, vec.y, vec.z);
}

void CEntity::SetHeading(float heading)
{
	ENTITY::SET_ENTITY_HEADING(Handle, heading);
}

float CEntity::GetHeading()
{
	return ENTITY::GET_ENTITY_HEADING(Handle);
}

void CEntity::SetRotation(CVector3 rotation)
{
	ENTITY::SET_ENTITY_ROTATION(Handle, rotation.fX, rotation.fY, rotation.fZ, 2, true);
}

CVector3 CEntity::GetRotation()
{
	Vector3 vec = ENTITY::GET_ENTITY_ROTATION(Handle, 2);
	return CVector3(vec.x, vec.y, vec.z);
}

float CEntity::GetPhysicsHeading()
{
	return ENTITY::_GET_ENTITY_PHYSICS_HEADING(Handle);
}

Hash CEntity::GetModel()
{
	return ENTITY::GET_ENTITY_MODEL(Handle);
}

unsigned short CEntity::GetHealth()
{
	return ENTITY::GET_ENTITY_HEALTH(Handle);
}

void CEntity::SetHealth(unsigned int health)
{
	ENTITY::SET_ENTITY_HEALTH(Handle, health);
}

bool CEntity::IsPlayingAnim(std::string animDict, std::string animName)
{
	return ENTITY::IS_ENTITY_PLAYING_ANIM(Handle, (char*)animDict.c_str(), (char*)animName.c_str(), 3) ? true : false;
}

void CEntity::StopAnim(std::string animDict, std::string animName)
{
	AI::STOP_ANIM_TASK(Handle, (char*)animDict.c_str(), (char*)animName.c_str(), -4.f);
}

void CEntity::PlayAnim(std::string animDict, std::string animName, float speed, int duration, DWORD animFlags, float playbackRate)
{
	AI::TASK_PLAY_ANIM(Handle, (char*)animDict.c_str(), (char*)animName.c_str(), speed, -8.f, duration, animFlags, playbackRate, false, false, false);
}

Blip CEntity::AddBlip()
{
	return UI::ADD_BLIP_FOR_ENTITY(Handle);
}

void CEntity::AttachBlip(CNetworkBlip *_blip)
{
	if(blip) UI::REMOVE_BLIP(&blip->Handle);
	blip = _blip;
}

void CEntity::FreezePosition(bool toggle)
{
	ENTITY::FREEZE_ENTITY_POSITION(Handle, toggle);
}

void CEntity::SetVisible(bool toggle)
{
	ENTITY::SET_ENTITY_VISIBLE(Handle, toggle, 0);
}

void CEntity::DisableCollision(const CEntity &entity)
{
	ENTITY::SET_ENTITY_NO_COLLISION_ENTITY(Handle, entity.Handle, false);
}

uintptr_t CEntity::GetAddress()
{
	return _entityAddressFunc(Handle);
}

void CEntity::InitOffsetFunc()
{
	uintptr_t address = CMemory((uintptr_t)GetModuleHandle(NULL) + 0xA29ECE)();
	_entityAddressFunc = reinterpret_cast<GetEntityOffsetFunc>(*reinterpret_cast<int *>(address + 3) + address + 7);
}

CEntity::CEntity(Entity handle) :Handle(handle)
{
}

CEntity::~CEntity()
{
	ENTITY::DELETE_ENTITY(&Handle);
}
