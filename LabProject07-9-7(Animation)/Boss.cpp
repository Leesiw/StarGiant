#include "stdafx.h"
#include "Boss.h"
#include <random>

Boss::Boss()
{
	MAXBossHP = 100.0f;
	BossHP = MAXBossHP;
	CurMotion = BossAnimation::SLEEP;
	CurState = BossState::SLEEP;

	stateStartTime = steady_clock::now();


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




std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
float randomValue = distribution(generator);

void Boss::Boss_Ai(BossState CurState, XMFLOAT3 TargetPos, int bossHP)
{
	XMFLOAT3 BossPos = GetPosition();
	XMFLOAT3 SubTarget = Vector3::Subtract(TargetPos, BossPos);
	float Dist = Vector3::Length(SubTarget);


	switch (CurState) {
	case  BossState::APPEAR:
	{
		//적 처치 조건 맞으면 보스 상태 appear로 바꿔서 위치 미니맵 어딘가에 표시 될 수 있는 정도 되는 곳으로 랜덤이든 고정이든 setpos
		this->SetPosition(TargetPos.x + 200.0f, TargetPos.y, TargetPos.z + 200.0f);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::SLEEP), true);

		//일단 첨엔 잠
		SetState(BossState::SLEEP);
		stateStartTime = steady_clock::now(); //이거 다 쓰면 걍 ai끝날때 초기화로 바꿔야겟다 나중에..

		break;
	}

	case  BossState::SLEEP: 
	{

		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::SLEEP), true);

		//만약에 플레이어가 가까이 오면 idle로 가기 
		if (Dist < 200.0f) {
			SetState(BossState::IDLE);
			stateStartTime = steady_clock::now();
		}
		break;
	}

	case BossState::IDLE: {

		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::IDLE), true);

		/*if (Dist > 400.0f)
			SetState(BossState::SIT_IDLE);*/


		if (Dist > 400.0f) {
			SetState(BossState::WALK);
			stateStartTime = steady_clock::now();
		}


		//attactCoolTime 초마다 공격
		if (duration_cast<seconds>(steady_clock::now() - stateStartTime).count() >= attactCoolTime) {
			SetState(BossState::ATTACT);
			randomValue = distribution(generator);
			stateStartTime = steady_clock::now();
			lastAttackTime = steady_clock::now();
		}


		//10퍼 남으면 스크림 함 해주기
		else if (float(MAXBossHP / bossHP) <= 0.1)
		{
			SetState(BossState::SCREAM);
			stateStartTime = steady_clock::now();
		}

		//반피되면 피격모션 한번해주기
		else if (float(bossHP / MAXBossHP) <= 0.5)
		{
			SetState(BossState::GET_HIT);
			stateStartTime = steady_clock::now();
		}


		break;
	}

	case BossState::ATTACT: {
			if (randomValue > 0.5f) {
				m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
				m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::BASIC_ATTACT), true);
				PastState = (BossState)(BossAnimation::BASIC_ATTACT);

				if (duration_cast<seconds>(steady_clock::now() - lastAttackTime).count() >= 2) {
					lastAttackTime = steady_clock::now();
					SetState(BossState::IDLE);
					PastState = (BossState)(BossAnimation::BASIC_ATTACT);
				}
			}
			else if (randomValue < 0.2f) {
				m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
				m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::FLAME_ATTACK), true);
				PastState = (BossState)(BossAnimation::FLAME_ATTACK);

				if (duration_cast<seconds>(steady_clock::now() - lastAttackTime).count() >= 4) {
					lastAttackTime = steady_clock::now();
					SetState(BossState::IDLE);
					PastState = (BossState)(BossAnimation::FLAME_ATTACK);
				}
			}
			else
			{
				m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
				m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::CLAW_ATTACT), true);
				PastState = (BossState)(BossAnimation::CLAW_ATTACT);

				if (duration_cast<seconds>(steady_clock::now() - lastAttackTime).count() >= 3) {
					lastAttackTime = steady_clock::now();
					SetState(BossState::IDLE);

					PastState = (BossState)(BossAnimation::CLAW_ATTACT);
				}
			}
		
			break;
	}

	case BossState::SIT_IDLE: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::SIT_IDLE), true);


		/*if (Dist < 200.0f)
			SetState(BossState::IDLE);*/

		if (duration_cast<seconds>(steady_clock::now() - stateStartTime).count() >= 3) {
			SetState(BossState::IDLE);
			stateStartTime = steady_clock::now();
		}

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

		if (duration_cast<seconds>(steady_clock::now() - stateStartTime).count() >= 3) {
			SetState(BossState::IDLE);
			stateStartTime = steady_clock::now();
		}
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

		if (duration_cast<seconds>(steady_clock::now() - stateStartTime).count() >= 1) {
			SetState(BossState::IDLE);
			stateStartTime = steady_clock::now();
		}


		//cout << "BASIC_ATTACT" << endl;

		break;

	}
	case BossState::CLAW_ATTACT: {
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastState), false);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(BossAnimation::CLAW_ATTACT), true);

		if (duration_cast<seconds>(steady_clock::now() - stateStartTime).count() >= 1) {
			SetState(BossState::IDLE);
			stateStartTime = steady_clock::now();
		}

		//cout << "CLAW_ATTACT" << endl;

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

	if(CurState != BossState::ATTACT)
		PastState = CurState;
}


void Boss::BossAi()
{

}

void Boss::ChangeAnimation(BossAnimation CurMotion)
{

	//cout << "curmotion - " << static_cast<int>(CurMotion) << endl;
	//모션 받음 n모션 -> p모션으로 저장함 -> p모션 false -> n모션 true
	//다르면 먼저 모션 바꿔주고, c를 p에 저장
	if (CurMotion != PastMotion) {

		if (CurMotion == BossAnimation::SCREAM && onceScream) {
			soundon = static_cast<int>(Sounds::ROAR);
		}

		else if (CurMotion == BossAnimation::BASIC_ATTACT) {
			soundon = static_cast<int>(Sounds::BASIC_ATTACT);


		}
		else if (CurMotion == BossAnimation::CLAW_ATTACT) {
			soundon = static_cast<int>(Sounds::CLAW);


		}
		else if (CurMotion == BossAnimation::FLAME_ATTACK) {
			soundon = static_cast<int>(Sounds::ROAR);


		}
		else {
			soundon = -1;
		}


		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(PastMotion), false);
		m_pSkinnedAnimationController->SetTrackPosition(static_cast<int>(CurMotion), 0.0f);
		m_pSkinnedAnimationController->SetTrackPosition(static_cast<int>(PastMotion), 0.0f);
		SetTrackAnimationPosition(static_cast<int>(PastMotion), 0.0f);
		SetTrackAnimationPosition(static_cast<int>(CurMotion), 0.0f);
		m_pSkinnedAnimationController->SetTrackEnable(static_cast<int>(CurMotion), true);
		PastMotion = CurMotion;

	}



	if (CurMotion == BossAnimation::DIE)
	{
		static std::chrono::time_point<std::chrono::steady_clock> target_time;
		// 처음에 target_time 초기화
		if (target_time.time_since_epoch().count() == 0) {
			target_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(1700);
		}



		auto now = std::chrono::steady_clock::now();

		if (std::chrono::steady_clock::now() > target_time) {
			cout << "d";
			m_pSkinnedAnimationController->SetTrackPosition(static_cast<int>(CurMotion), 1.0f);
			SetTrackAnimationPosition(static_cast<int>(CurMotion), 1.0f);
			m_pSkinnedAnimationController->SetTrackSpeed(static_cast<int>(CurMotion), 0.0f);
		}
	}

}
