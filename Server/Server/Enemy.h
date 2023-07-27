#pragma once
#include "Session.h"
#include "Missile.h"

extern array<SESSION, MAX_USER> clients;
extern unordered_map<MissionType, Level> levels;
extern mutex m;

class CEnemy : public CGameObject
{
public:
	CEnemy();
	virtual ~CEnemy();
protected:
	float						m_fCoolTime = 2.0f;
	float						m_fCoolTimeRemaining = 0.0f;

	XMFLOAT3					m_xmf3Velocity;
	XMFLOAT3					m_xmf3Destination;

	char						damage = 3;

	unsigned char enemy_flags = 0;	// 0 : 살아있는지 1 : 플레이어를 보고 있는지
	// 2 : 공격 타이머가 켜져 있는지	// 3 : 움직였는지
public:
	short scene_num;

	char id;
	EnemyType type;

	char hp;
	EnemyState state;

public:
	void Reset();

	bool GetisAlive() { return enemy_flags & option0; }
	void SetisAliveTrue();
	void SetisAliveFalse();
	void SetAttackTimerFalse();

	bool GetisMove() { return enemy_flags & option3; }

	void SetAttackTimerTrue();

	bool GetAttackTimer();

	char GetHP() { return hp; }

	char GetID() { return id; }
	virtual void SetStatus(MissionType cur_mission);

	bool GetLaunchMissile() { return enemy_flags & option2; }

	void SetDestination();
	const XMFLOAT3& GetDestination() const { return(m_xmf3Destination); }
	virtual void AI(float fTimeElapsed, CAirplanePlayer* player);
	virtual void MoveAI(float fTimeElapsed, CAirplanePlayer* player);
	virtual void AimingAI(float fTimeElapsed, CAirplanePlayer* player);
	virtual void Attack(float fTimeElapsed, CAirplanePlayer* player);

	virtual void VelocityUpdate(float fTimeElapsed, CAirplanePlayer* player);

	void SetIsMoveFalse();

	void Rotate(float x, float y, float z);
	void LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos);

	virtual void Animate(float fElapsedTime);
	virtual void Animate(float fTimeElapsed, CAirplanePlayer* player);

	EnemyState GetState() { return state; }

	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }

	const short GetDamage() const { return(damage); }

	XMFLOAT4 GetQuaternion();

	void ResetCoolTime() { m_fCoolTimeRemaining = m_fCoolTime; }
};

class CMissileEnemy : public CEnemy
{
public:
	CMissileEnemy();
	virtual ~CMissileEnemy();
public:
	//virtual MissileInfo GetMissileInfo() { enemy_flags &= ~option2; return info; }
	virtual void Attack(float fTimeElapsed, CAirplanePlayer* player);

	virtual void SetStatus(MissionType cur_mission);

	virtual void Animate(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed, CAirplanePlayer* player);
};

class CLaserEnemy : public CEnemy
{
public:
	CLaserEnemy();
	virtual ~CLaserEnemy();
public:
	virtual void SetStatus(MissionType cur_mission);

	virtual void Animate(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed, CAirplanePlayer* player);
};

class CPlasmaCannonEnemy : public CLaserEnemy
{
public:
	CPlasmaCannonEnemy();
	virtual ~CPlasmaCannonEnemy();
public:
	virtual void SetStatus(MissionType cur_mission);

	virtual void Animate(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed, CAirplanePlayer* player);
};