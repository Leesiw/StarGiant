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
	short						m_fAvoidReductionRate = 20;

	unsigned char enemy_flags = 0;	// 0 : 살아있는지 1 : 플레이어를 보고 있는지
	// 2 : 공격 타이머가 켜져 있는지
public:
	short scene_num;

	char id;
	EnemyType type;

	char hp;
	EnemyState state;

public:
	bool GetisAlive() { return enemy_flags & option0; }
	void SetisAlive(bool i_a);
	void SetAttackTimerFalse();

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

	void Rotate(float x, float y, float z);
	void LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos);

	virtual void Animate(float fElapsedTime);
	virtual void Animate(float fTimeElapsed, CAirplanePlayer* player);

	float GetPitch() { return m_fPitch; }
	float GetYaw() { return m_fYaw; }
	float GetRoll() { return m_fRoll; }

	EnemyState GetState() { return state; }

	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }

	const short GetDamage() const { return(damage); }

	XMFLOAT4 GetQuaternion();

	void ResetCoolTime() { m_fCoolTimeRemaining = m_fCoolTime; }

	void SendPos();

	virtual MissileInfo GetMissileInfo() { MissileInfo info; info.damage = 0; return info; }
};

class CMissileEnemy : public CEnemy
{
public:
	CMissileEnemy();
	virtual ~CMissileEnemy();

	char missile_damage = 4;

	MissileInfo info;
public:
	virtual MissileInfo GetMissileInfo() { enemy_flags &= ~option2; return info; }
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

	float m_fHitProbability;		// 명중 확률
	float m_fAvoidReductionRate;	// 상대 가속/방향 전환 시 명중률 떨어지는 정도
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