#pragma once

class CPedestrian: public CEntity
{
public:
	CPedestrian(Ped handle);
	CPedestrian(Hash Model, CVector3 Position, float Heading);
	~CPedestrian();
	void SetDesiredHeading(float heading);
	float GetDesiredHeading() { return CEntity::GetHeading(); }
	void TaskMove(CVector3 vecPos, float fMoveSpeed);
	void TaskAimAt(CVector3 vecAim, int duration);
	void TaskShootAt(CVector3 vecAim, int duration);
	void ClearTasks(bool rightnow = false);
	void SetCurrentHeading(float heading) { CEntity::SetHeading(heading); }
	float GetCurrentHeading() { return CEntity::GetHeading(); }
	bool IsDucking();
	void SetDucking(bool ducking);
	bool IsJumping();
	void TaskJump();
	bool IsShooting();
	bool IsFalling();
	void SetCoordsKeepVehicle(float x, float y, float z);
	void SetArmour(unsigned short armour);
	unsigned short GetArmour();
	Hash GetCurrentWeapon();
	void SetCurrentWeapon(Hash weapon, bool equipNow = TRUE);
	unsigned int GetCurrentWeaponAmmo();
	unsigned int GetAmmo(Hash weapon);
	void SetAmmo(Hash weapon, unsigned int ammo);
	float GetBlendRation();
	void SetComponentVariation(int component, int drawable, int texture, int palette);
	void SetMoney(int money);
	void GiveMoney(int money);
	int GetMoney();
	void GiveWeapon(Hash weapon, int ammo);
	void RemoveWeapon(Hash weapon);
	void RemoveAllWeapons();
	void SetAmmo(Hash weapon, int ammo);
	void AddAmmo(Hash weapon, int ammo);
	void RemoveAmmo(Hash weapon, int ammo);
};
