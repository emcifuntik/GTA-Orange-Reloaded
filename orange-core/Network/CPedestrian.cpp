#include "stdafx.h"

CPedestrian::CPedestrian(Hash Model, CVector3 Position, float Heading) :CEntity(-1)
{
	if (STREAMING::IS_MODEL_IN_CDIMAGE(Model) && STREAMING::IS_MODEL_VALID(Model))
	{
		STREAMING::REQUEST_MODEL(Model);
		while (!STREAMING::HAS_MODEL_LOADED(Model))
			scriptWait(0);

		CWorld::Get()->CPedFactoryPtr->Create = PedFactoryHook::Get()->CreateHook;
		Handle = (Entity)PED::CREATE_PED(1, Model, Position.fX, Position.fY, Position.fZ, Heading, true, false);
		CWorld::Get()->CPedFactoryPtr->Create = &hookCreatePed;
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(Model);
		AI::TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(Handle, true);
		/*PED::SET_PED_CAN_RAGDOLL_FROM_PLAYER_IMPACT(Handle, false);
		PED::SET_PED_FLEE_ATTRIBUTES(Handle, 0, 0);
		PED::SET_PED_COMBAT_ATTRIBUTES(Handle, 17, 1);
		PED::SET_PED_CAN_RAGDOLL(Handle, false);
		PED::_SET_PED_RAGDOLL_FLAG(Handle, 1 | 2 | 4);*/
		ENTITY::SET_ENTITY_PROOFS(Handle, true, true, true, true, true, true, false, true);
	}
}

CPedestrian::CPedestrian(Ped handle) :CEntity(handle)
{
}

CPedestrian::~CPedestrian()
{
	PED::DELETE_PED(&Handle);
}

void CPedestrian::SetDesiredHeading(float heading)
{
	PED::SET_PED_DESIRED_HEADING(Handle, heading);
}

void CPedestrian::TaskMove(CVector3 vecPos, float fMoveSpeed)
{
	//MemoryHook::call<void, Ped, float, float, float, float, int, float, float>((*GTA::CAddress::Get())[PED_TASK_MOVE_TO_COORD], Handle, vecPos.fX, vecPos.fY, vecPos.fZ, fMoveSpeed, -1, 0.0f, 0.0f);
	AI::TASK_GO_STRAIGHT_TO_COORD(Handle, vecPos.fX, vecPos.fY, vecPos.fZ, fMoveSpeed, -1, 0.0f, 0.0f);
}

void CPedestrian::TaskAimAt(CVector3 vecAim, int duration)
{
	//MemoryHook::call<void, Ped, float, float, float, int, BOOL, BOOL>((*GTA::CAddress::Get())[PED_TASK_AIM_AT_COORD_AND_STAND_STILL], Handle, vecAim.fX, vecAim.fY, vecAim.fZ, duration, 0, 0);
	AI::TASK_AIM_GUN_AT_COORD(Handle, vecAim.fX, vecAim.fY, vecAim.fZ, duration, 0, 0);
}

void CPedestrian::TaskShootAt(CVector3 vecAim, int duration)
{
	//AI::TASK_SHOOT_AT_COORD(Handle, vecAim.fX, vecAim.fY, vecAim.fZ, duration, 3337513804U);
	PED::SET_PED_SHOOTS_AT_COORD(Handle, vecAim.fX, vecAim.fY, vecAim.fZ, true);
}

void CPedestrian::ClearTasks(bool rightnow)
{
	if (!rightnow)
		AI::CLEAR_PED_TASKS(Handle);
	else
		AI::CLEAR_PED_TASKS_IMMEDIATELY(Handle);
}

bool CPedestrian::IsDucking()
{
	return (PED::IS_PED_DUCKING(Handle) ? true : false);
}

void CPedestrian::SetDucking(bool ducking)
{
	PED::SET_PED_DUCKING(Handle, ducking);
}

bool CPedestrian::IsJumping()
{
	return (PED::IS_PED_JUMPING(Handle) ? true : false);
}

void CPedestrian::TaskJump()
{
	AI::TASK_JUMP(Handle, false);
}

bool CPedestrian::IsShooting()
{
	return (PED::IS_PED_SHOOTING(Handle) ? true : false);
}

bool CPedestrian::IsFalling()
{
	return (PED::IS_PED_FALLING(Handle) ? true : false);
}

void CPedestrian::SetCoordsKeepVehicle(float x, float y, float z)
{
	PED::SET_PED_COORDS_KEEP_VEHICLE(Handle, x, y, z);
}

void CPedestrian::SetArmour(unsigned short armour)
{
	PED::SET_PED_ARMOUR(Handle, armour);
}

unsigned short CPedestrian::GetArmour()
{
	return PED::GET_PED_ARMOUR(Handle);
}

Hash CPedestrian::GetCurrentWeapon()
{
	Hash weapon;
	WEAPON::GET_CURRENT_PED_WEAPON(Handle, &weapon, TRUE);
	return weapon;
}

void CPedestrian::SetCurrentWeapon(Hash weapon, bool equipNow /*= TRUE*/)
{
	WEAPON::GIVE_WEAPON_TO_PED(Handle, weapon, 9999, true, true);
	WEAPON::SET_CURRENT_PED_WEAPON(Handle, weapon, equipNow);
}

unsigned int CPedestrian::GetCurrentWeaponAmmo()
{
	return WEAPON::GET_AMMO_IN_PED_WEAPON(Handle, GetCurrentWeapon());
}

unsigned int CPedestrian::GetAmmo(Hash weapon)
{
	return WEAPON::GET_AMMO_IN_PED_WEAPON(Handle, weapon);
}

void CPedestrian::SetAmmo(Hash weapon, unsigned int ammo)
{
	WEAPON::SET_PED_AMMO(Handle, weapon, ammo);
}

float CPedestrian::GetBlendRation()
{
	return AI::GET_PED_DESIRED_MOVE_BLEND_RATIO(Handle);
}

void CPedestrian::SetComponentVariation(int component, int drawable, int texture, int palette)
{
	PED::SET_PED_COMPONENT_VARIATION(Handle, component, drawable, texture, palette);
}

void CPedestrian::SetMoney(int money)
{
	PED::SET_PED_MONEY(Handle, money);
}

void CPedestrian::GiveMoney(int money)
{
	PED::SET_PED_MONEY(Handle, GetMoney() + money);
}

int CPedestrian::GetMoney()
{
	return PED::GET_PED_MONEY(Handle);
}

void CPedestrian::GiveWeapon(Hash weapon, int ammo)
{
	WEAPON::GIVE_WEAPON_TO_PED(Handle, weapon, ammo, false, false);
}

void CPedestrian::RemoveWeapon(Hash weapon)
{
	WEAPON::REMOVE_WEAPON_FROM_PED(Handle, weapon);
}

void CPedestrian::RemoveAllWeapons()
{
	WEAPON::REMOVE_ALL_PED_WEAPONS(Handle, true);
}

void CPedestrian::SetAmmo(Hash weapon, int ammo)
{
	WEAPON::SET_PED_AMMO(Handle, weapon, ammo);
}

void CPedestrian::AddAmmo(Hash weapon, int ammo)
{
	WEAPON::ADD_AMMO_TO_PED(Handle, weapon, ammo);
}

void CPedestrian::RemoveAmmo(Hash weapon, int ammo)
{
	WEAPON::SET_PED_AMMO(Handle, weapon, WEAPON::GET_AMMO_IN_PED_WEAPON(Handle, weapon) - ammo);
}
