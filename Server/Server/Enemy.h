#pragma once
#include "Session.h"

enum class EnemyState : char
{
	IDLE, MOVE, AIMING, ATTACK, AVOID
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

	float						m_fMoveTime = 1.0f;
	float						m_fMoveTimeRemaining = 1.0f;

	bool						m_bRotateToPlayer = false;

	float						m_fAvoidTime = 0.0f;
	bool						m_bAvoidDir;		// true면 오른쪽, false면 왼쪽

	XMFLOAT3					m_xmf3Destination;
	XMFLOAT3					m_xmf3RelativePos;

	short						damage;

	bool						isAlive;
public:
	short id;

	short hp;
	EnemyState state;

public:
	bool GetisAlive() { return isAlive; }
	void SetisAlive(bool i_a) { isAlive = i_a; }

	virtual void AI(float fTimeElapsed, CPlayer* player);
	virtual void MoveAI(float fTimeElapsed, CPlayer* player);
	virtual void AimingAI(float fTimeElapsed, CPlayer* player);
	virtual void AttackAI(float fTimeElapsed, CPlayer* player);
	virtual void Attack(float fTimeElapsed, CPlayer* player);
	virtual void AvoidAI(float fTimeElapsed);

	virtual void VelocityUpdate(float fTimeElapsed, CPlayer* player);

	void Rotate(float x, float y, float z);

	virtual void Animate(float fElapsedTime);
	virtual void Animate(float fTimeElapsed, CPlayer* player);

	float GetPitch() { return m_fPitch; }
	float GetYaw() { return m_fYaw; }
	float GetRoll() { return m_fRoll; }

	XMFLOAT4 GetQuaternion();

	void ResetCoolTime() { m_fCoolTimeRemaining = m_fCoolTime; }

	void SendPos();
};

class CMissileEnemy : public CEnemy
{
public:
	CMissileEnemy();
	virtual ~CMissileEnemy();

	CGameObject** m_BulletObjects = NULL;
	CBulletObject* m_ppBullets[BULLETS];
	bool FireBullet(CGameObject* pLockedObject);
	float						m_fBulletEffectiveRange = 150.0f;
	CGameObject* pBullet;

public:
	virtual void Animate(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed, CPlayer* player);
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
	virtual void Animate(float fTimeElapsed, CPlayer* player);
};

class CPlasmaCannonEnemy : public CLaserEnemy
{
public:
	CPlasmaCannonEnemy();
	virtual ~CPlasmaCannonEnemy();
public:
	virtual void Animate(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed, CPlayer* player);
};