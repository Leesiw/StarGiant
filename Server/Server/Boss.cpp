#include "stdafx.h"
#include "Boss.h"

Boss::Boss()
{
	MAXBossHP = 100.0f;
	BossHP = MAXBossHP;
	CurMotion = BossAnimation::SLEEP;
	CurState = BossState::SLEEP;
}


void Boss::SendPosition()	// 위치/각도 변화할 때 사용. 
{
	ENEMY_INFO info;
	info.id = BOSS_ID;
	info.pos = GetPosition();
	info.Quaternion = GetQuaternion();

	for (auto& pl : clients) {
		if (false == pl.in_use) continue;
		pl.send_enemy_packet(0, info);
	}
}

void Boss::SendAnimation() // 애니메이션 변화했을 때 사용
{
	for (auto& pl : clients) {
		if (false == pl.in_use) continue;
		pl.send_animation_packet(BOSS_ID, (char)CurMotion);
	}
}

XMFLOAT4 Boss::GetQuaternion()
{
	XMMATRIX mat = XMLoadFloat4x4(&m_xmf4x4ToParent);
	XMVECTOR vec = XMQuaternionRotationMatrix(mat);
	XMFLOAT4 xmf4;
	XMStoreFloat4(&xmf4, vec);
	return xmf4;
}

void Boss::Boss_Ai(BossState CurState, XMFLOAT3 TargetPos, int bossHP)
{
	XMFLOAT3 BossPos = GetPosition();
	XMFLOAT3 SubTarget = Vector3::Subtract(TargetPos, BossPos);
	float Dist = Vector3::Length(SubTarget);

	switch (CurState) {
	case  BossState::SLEEP:
	{
		//만약에 플레이어가 가까이 오면 idle로 가기 
		//cout << "dist: " << Dist << endl;
		if (Dist < 200.0f)
			SetState(BossState::IDLE);
		break;
	}

	case BossState::IDLE: {
		if (Dist > 400.0f)
			SetState(BossState::SIT_IDLE);

		break;
	}

	case BossState::SIT_IDLE: {
		if (Dist < 200.0f)
			SetState(BossState::IDLE);

		break;
	}
	case BossState::SCREAM: {
		break;

	}

	case BossState::GET_HIT: {
		cout << "GET_HIT" << endl;

		break;

	}
	case BossState::WALK: {
		cout << "WALK" << endl;

		break;

	}
	case BossState::RUN: {
		cout << "RUN" << endl;

		break;

	}
	case BossState::BASIC_ATTACT: {
		cout << "BASIC_ATTACT" << endl;

		break;

	}
	case BossState::CLAW_ATTACT: {
		cout << "CLAW_ATTACT" << endl;

		break;

	}
	case BossState::FLAME_ATTACK: {
		cout << "FLAME_ATTACK" << endl;

		break;

	}	case BossState::DEFEND: {
		cout << "DEFEND" << endl;

		break;

	}	case BossState::TAKE_OFF: {
		cout << "TAKE_OFF" << endl;

		break;

	}

	case BossState::FLY_FLOAT: {
		cout << "FLY_FLOAT" << endl;
		break;

	}


	case BossState::FLY_FLAME_ATTACK: {
		cout << "FLY_FLAME_ATTACK" << endl;

		break;
	}

	case BossState::FLY_FORWARD: {
		cout << "FLY_FORWARD" << endl;

		break;
	}
	case BossState::FLY_GLIDE: {
		cout << "FLY_GLIDE" << endl;

		break;
	}
	case BossState::LAND: {
		cout << "LAND" << endl;

		break;
	}

	case BossState::DIE: {
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
