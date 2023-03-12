#pragma once
#include "Object.h"

struct MissileInfo {
	XMFLOAT3 StartPos;
	XMFLOAT4 Quaternion;
	char damage;
};

class CMissile : public CGameObject {
private:
	bool isActive = false;

	float m_fTrackingTime = 5.f;
	float m_fTrackingTimeRemaining;

	float m_fMovingDistance; 
	float m_fBulletEffectiveRange = 1000.f;

	XMFLOAT3 m_xmf3MovingDirection;

	char damage;
public:
	CMissile();

	virtual void Animate(float fTimeElapsed, CGameObject* target);

	void LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos);

	void SetisActive(bool active);
	bool GetisActive() { return isActive; }
	XMFLOAT4 GetQuaternion();
	char GetDamage() { return damage; }
	void SetNewMissile(const MissileInfo& info);
	void Reset();

};