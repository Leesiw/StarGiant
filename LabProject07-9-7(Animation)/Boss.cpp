#include "stdafx.h"
#include "Boss.h"

Boss::Boss()
{
	MAXBossHP = 100.0f;
	BossHP = MAXBossHP;
	CurMotion = BossAnimation::SLEEP;
	CurState = BossState::SLEEP;

}

void Boss::BossObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pBossModel = pModel;
	if (!pBossModel) pBossModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Red.bin", NULL);

	SetChild(pBossModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, static_cast<int>(BossAnimation::COUNT), pBossModel);

	for (int i = 0; i < static_cast<int>(BossAnimation::COUNT); ++i) {
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, false);	
	}

	//m_pSkinnedAnimationController->SetTrackAllUnable(static_cast<int>(BossAnimation::COUNT));
	m_pSkinnedAnimationController->SetTrackEnable(0, true);

}



void Boss::Boss_Ai(BossState CurState, XMFLOAT3 TargetPos, int bossHP)
{
	XMFLOAT3 BossPos = GetPosition();
	XMFLOAT3 SubTarget = Vector3::Subtract(TargetPos, BossPos);
	float Dist = Vector3::Length(SubTarget);

	switch (CurState) {
	case  BossState::SLEEP:
	{
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(0, true);

		//만약에 플레이어가 가까이 오면 idle로 가기 
		//cout << "dist: " << Dist << endl;
		if (Dist < 200.0f)
			SetState(BossState::IDLE);
		break;
	}

	case BossState::IDLE: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::IDLE), true);

		if (Dist > 400.0f)
			SetState(BossState::SIT_IDLE);

		break;
	}

	case BossState::SIT_IDLE: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::SIT_IDLE), true);

		if (Dist < 200.0f)
			SetState(BossState::IDLE);

		break;
	}
	case BossState::SCREAM: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::SCREAM), true);
	
		break;

	}

	case BossState::GET_HIT: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::GET_HIT), true);
		cout << "GET_HIT" << endl;

		break;

	}
	case BossState::WALK: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::WALK), true);
		cout << "WALK" << endl;

		break;

	}
	case BossState::RUN: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::RUN), true);
		cout << "RUN" << endl;

		break;

	}
	case BossState::BASIC_ATTACT: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::BASIC_ATTACT), true);
		cout << "BASIC_ATTACT" << endl;

		break;

	}
	case BossState::CLAW_ATTACT: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::CLAW_ATTACT), true);
		cout << "CLAW_ATTACT" << endl;

		break;

	}
	case BossState::FLAME_ATTACK: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::FLAME_ATTACK), true);
		cout << "FLAME_ATTACK" << endl;

		break;

	}	case BossState::DEFEND: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::DEFEND), true);
		cout << "DEFEND" << endl;

		break;

	}	case BossState::TAKE_OFF: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::TAKE_OFF), true);
		cout << "TAKE_OFF" << endl;

		break;

	}

	case BossState::FLY_FLOAT: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::FLY_FLOAT), true);
		cout << "FLY_FLOAT" << endl;
		break;

	}


	case BossState::FLY_FLAME_ATTACK: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::FLY_FLAME_ATTACK), true);
		cout << "FLY_FLAME_ATTACK" << endl;

		break;
	}

	case BossState::FLY_FORWARD: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::FLY_FORWARD), true);
		cout << "FLY_FORWARD" << endl;

		break;
	}	
	case BossState::FLY_GLIDE: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::FLY_GLIDE), true);
		cout << "FLY_GLIDE" << endl;

		break;
	}	
	case BossState::LAND: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::LAND), true);
		cout << "LAND" << endl;

		break;
	}

	case BossState::DIE: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::DIE), true);
		cout << "DIE" << endl;

		break;
	}
	default:
		break;
	}

	PastState = CurState;

}

void Boss::BossAi()
{

}
