#pragma once

#include "Object.h"
#include "Camera.h"
#include "Sound.h"





class Boss : public CGameObject
{
public:
	Boss();
	virtual ~Boss() {};

public:
	BossState CurState = BossState::SLEEP;
	BossState PastState = CurState;
	BossState NextState = CurState;
	bool condition = false;

	chrono::steady_clock::time_point stateStartTime;
	steady_clock::time_point lastAttackTime;
	steady_clock::time_point BASIC_ATTACT_LastTime;
	steady_clock::time_point CLAW_ATTACT_LastTime;

	BossAnimation CurMotion = BossAnimation::SLEEP;
	BossAnimation PastMotion = CurMotion;

	int MAXBossHP = 100.0f;
	int BossHP = 100.0f;

	int GetcurHp() { return BossHP; }
	int GetMaxHp() { return MAXBossHP; }

	int attactCoolTime = 3;

	BossState GetState() { return CurState; };
	BossState SetState(BossState BState) { return CurState = BState; };
	BossAnimation GetAnimation() { return CurMotion; };
	BossAnimation SetAnimation(BossAnimation BMotion) { return CurMotion = BMotion; };


	int GetHP() { return BossHP; };
	int SetHP(int hp) { return BossHP = hp; }


	char soundon = -1;
	char soundo0nPAST = -1;

	
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

	bool onceScream = true;

public:
	void BossObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	void Boss_Ai(BossState CurState, XMFLOAT3 TargetPos, int bossHp);

	void BossAi();
	void ChangeAnimation(BossAnimation CurState);


};