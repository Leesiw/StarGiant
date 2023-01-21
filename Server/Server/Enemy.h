#pragma once
#include "Object.h"
#include "Session.h"

enum class EnemyState : char
{
	IDLE, MOVE, ATTACK, AVOID
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
public:
	int hp;
	EnemyState state;

public:
	virtual void AI(float fTimeElapsed, XMFLOAT3& player_pos);
	virtual void MoveAI(float fTimeElapsed, XMFLOAT3& player_pos);
	virtual void AttackAI();
	virtual void AvoidAI();

	void Rotate(float x, float y, float z);

	virtual void Animate(float fElapsedTime);
	virtual void Animate(float fTimeElapsed, XMFLOAT3 player_pos);

	float GetPitch() { return m_fPitch; }
	float GetYaw() { return m_fYaw; }
	float GetRoll() { return m_fRoll; }

	void ResetCoolTime() { m_fCoolTimeRemaining = m_fCoolTime; }
};


class CLaserEnemy : public CEnemy
{
public:
	CLaserEnemy();
	virtual ~CLaserEnemy();

public:
	virtual void Animate(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed, XMFLOAT3 player_pos);
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
	virtual void Animate(float fTimeElapsed, XMFLOAT3 player_pos);
};

class CPlasmaCannonEnemy : public CEnemy
{
public:
	CPlasmaCannonEnemy();
	virtual ~CPlasmaCannonEnemy();
public:
	virtual void Animate(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed, XMFLOAT3 player_pos);
};