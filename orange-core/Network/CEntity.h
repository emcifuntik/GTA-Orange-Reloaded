#pragma once

typedef uintptr_t(*GetEntityOffsetFunc)(int);

class CEntity
{
protected:
	Entity Handle;
	static GetEntityOffsetFunc _entityAddressFunc;
public:
	Entity GetHandle();
	CVector3 GetRotationVelocity();
	void SetMovementVelocity(CVector3 velocity);
	CVector3 GetMovementVelocity();
	void GetMoveSpeed(CVector3 & vecMoveSpeed);
	bool GetDamageToEntity();
	void ClearDamage();
	float GetSpeed();
	void SetPosition(CVector3 position);
	CVector3 GetPosition();
	void SetHeading(float heading);
	float GetHeading();
	void SetRotation(CVector3 rotation);
	CVector3 GetRotation();
	float GetPhysicsHeading();
	Hash GetModel();
	unsigned short GetHealth();
	void SetHealth(unsigned int health);

	bool IsPlayingAnim(std::string animDict, std::string animName);
	void StopAnim(std::string animDict, std::string animName);
	void PlayAnim(std::string animDict, std::string animName, float speed, int duration, DWORD animFlags, float playbackRate);

	Blip AddBlip();

	void FreezePosition(bool toggle);

	void SetVisible(bool toggle);

	void DisableCollision(const CEntity & entity);

	uintptr_t GetAddress();

	static void InitOffsetFunc();
	
	CEntity(Entity handle);
	~CEntity();
};
