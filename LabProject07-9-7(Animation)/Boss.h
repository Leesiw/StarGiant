#pragma once

#include "Object.h"
#include "Camera.h"




class Boss : public CGameObject
{
public:
	Boss();
	virtual ~Boss() {};

public:
	BossState CurState = BossState::SLEEP;
	BossState PastState = CurState;
	BossState NextState = CurState;

	BossAnimation CurMotion = BossAnimation::SLEEP;
	int MAXBossHP = 100.0f;
	int BossHP = 100.0f;

	BossState GetState() { return CurState; };
	BossState SetState(BossState BState) { return CurState = BState; };
	int GetHP() { return BossHP; };
	int SetHP(int hp) { return BossHP = hp; }


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
	void BossObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	void Boss_Ai(BossState CurState, XMFLOAT3 TargetPos, int bossHp);

	void BossAi();

};