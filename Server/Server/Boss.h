#pragma once

#include "Object.h"
#include "Session.h"
#include "include/lua.hpp"
#pragma comment (lib, "lua54.lib")


extern array<SESSION, MAX_USER> clients;

class Boss : public CGameObject
{
public:
	Boss();
	virtual ~Boss();

public:
	BossState CurState = BossState::SLEEP;
	BossState PastState = CurState;
	BossState NextState = CurState;

	BossAnimation CurMotion = BossAnimation::SLEEP;
	BossAnimation PastMotion = CurMotion;

	int MAXBossHP = 100.0f;
	int BossHP = 100.0f;
	bool condition = false;
	int attactCoolTime = 7;
	float speed = 100.0f;

	float randAttact = urdAttack(dree);

	short scene_num;

	chrono::steady_clock::time_point stateStartTime;
	chrono::steady_clock::time_point lastAttackTime;

	std::array<CMeteoObject*, BOSSMETEOS>		m_ppBossMeteoObjects;

	BossState GetState() { return CurState; };
	BossState SetState(BossState BState) { return CurState = BState; };
	int GetHP() { return BossHP; };
	int SetHP(int hp) { return BossHP = hp; }

	void SendPosition();
	void SendAnimation();

	XMFLOAT4 GetQuaternion();



protected:
	/*XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);*/

	/*XMFLOAT3					m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);*/

	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fMaxVelocityY = 0.0f;

public:
	void MeteoAttack(float fTimeElapsed, const XMFLOAT3& TargetPos); // 공격 시작 시 한 번 실행
	void MoveMeteo(float fTimeElapsed); // 메테오 움직여야 할때 계속 실행. send까지 포함됨

	void MoveBoss(float fTimeElapsed, XMFLOAT3 TargetPos, float dist);
	void LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos);
	void Boss_Ai(float fTimeElapsed, BossState CurState, CAirplanePlayer* player, int bossHp);


	// Lua 상태
	lua_State* m_L;
};