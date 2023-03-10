#pragma once
#include "Session.h"

enum class EnemyState : char
{
	IDLE, MOVE, AIMING
};

extern array<SESSION, MAX_USER> clients;

class CEnemy : public CGameObject
{
public:
	CEnemy();
	virtual ~CEnemy();
protected:
	float           			m_fPitch = 0;
	float           			m_fYaw = 0;
	float           			m_fRoll = 0;

	float						m_fCoolTime = 2.0f;
	float						m_fCoolTimeRemaining = 0.0f;

	float						m_fAttackRange = 300.0f;

	XMFLOAT3					m_xmf3Velocity;

	XMFLOAT3					m_xmf3Destination;

	short						damage = 3;
	short						hit_probability = 50;

	unsigned char enemy_flags = 0;	// 0 : 살아있는지 1 : 플레이어를 보고 있는지 
public:
	short id;
	EnemyType type;

	short hp;
	EnemyState state;

public:
	bool GetisAlive() { return enemy_flags & option0; }
	void SetisAlive(bool i_a);

	void SetDestination();
	virtual void AI(float fTimeElapsed, CAirplanePlayer* player);
	virtual void MoveAI(float fTimeElapsed, CAirplanePlayer* player);
	virtual void AimingAI(float fTimeElapsed, CAirplanePlayer* player);
	virtual void Attack(float fTimeElapsed, CAirplanePlayer* player);

	virtual void VelocityUpdate(float fTimeElapsed, CAirplanePlayer* player);

	void Rotate(float x, float y, float z);
	void LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos);

	virtual void Animate(float fElapsedTime);
	virtual void Animate(float fTimeElapsed, CAirplanePlayer* player);

	float GetPitch() { return m_fPitch; }
	float GetYaw() { return m_fYaw; }
	float GetRoll() { return m_fRoll; }

	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }

	const short GetDamage() const { return(damage); }

	XMFLOAT4 GetQuaternion();

	void ResetCoolTime() { m_fCoolTimeRemaining = m_fCoolTime; }

	void SendPos();
};

class CMissileEnemy : public CEnemy
{
public:
	CMissileEnemy();
	virtual ~CMissileEnemy();

public:

	virtual void Animate(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed, CAirplanePlayer* player);
};

class CLaserEnemy : public CEnemy
{
public:
	CLaserEnemy();
	virtual ~CLaserEnemy();

	float m_fHitProbability;		// 명중 확률
	float m_fAvoidReductionRate;	// 상대 가속/방향 전환 시 명중률 떨어지는 정도
public:
	virtual void Animate(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed, CAirplanePlayer* player);
};

class CPlasmaCannonEnemy : public CLaserEnemy
{
public:
	CPlasmaCannonEnemy();
	virtual ~CPlasmaCannonEnemy();
public:
	virtual void Animate(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed, CAirplanePlayer* player);
};