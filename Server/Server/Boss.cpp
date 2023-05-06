#include "stdafx.h"
#include "Boss.h"

Boss::Boss()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();

	MAXBossHP = 100.0f;
	BossHP = MAXBossHP;
	CurMotion = BossAnimation::SLEEP;
	CurState = BossState::SLEEP;

	boundingbox = BoundingOrientedBox{ XMFLOAT3(0.f, 34.65389f, -10.1982f), XMFLOAT3(65.5064392f, 35.0004547f, 77.9787476f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
}


void Boss::SendPosition()	// ��ġ/���� ��ȭ�� �� ���. 
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

void Boss::SendAnimation() // �ִϸ��̼� ��ȭ���� �� ���
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

void Boss::MoveBoss(float fTimeElapsed, XMFLOAT3 TargetPos, float dist)
{
	/*XMFLOAT3 BossPos = GetPosition();
	XMFLOAT3 chaseDirection = Vector3::Subtract(TargetPos, BossPos);*/

	LookAtPosition(fTimeElapsed, TargetPos);

	XMFLOAT3 BossPos = GetPosition();
	//�ӵ� ��������ߵ�
	XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(GetLook(), speed * fTimeElapsed, true);
	BossPos = Vector3::Add(BossPos, xmf3Movement);
	SetPosition(BossPos);


}


void  Boss::LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos)
{
	XMFLOAT3 new_pos = pos;
	XMMATRIX inv_mat = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World)); // �����

	new_pos = Vector3::TransformCoord(new_pos, inv_mat); // Ÿ���� ��ġ�� �� ��ü�� ��ǥ��� ��ȯ
	new_pos = Vector3::Normalize(new_pos);

	float pitch = XMConvertToDegrees(asin(-new_pos.y));
	float yaw = XMConvertToDegrees(atan2(new_pos.x, new_pos.z));
	 
	float rotate_angle = fTimeElapsed * 360.f;

	XMFLOAT3 p_y_r{ pitch, yaw, 0.f };
	if (Vector3::Length(p_y_r) > rotate_angle) {
		p_y_r = Vector3::Normalize(p_y_r);
		Rotate(p_y_r.x * rotate_angle, p_y_r.y * rotate_angle, 0.f);
	}
	else {
		Rotate(pitch, yaw, 0.f);
	}
}


void Boss::Boss_Ai(float fTimeElapsed, BossState CurState, CAirplanePlayer* player, int bossHP)
{
	XMFLOAT3 TargetPos = player->GetPosition();
	XMFLOAT3 BossPos = GetPosition();
	XMFLOAT3 SubTarget = Vector3::Subtract(TargetPos, BossPos);
	float Dist = Vector3::Length(SubTarget);


	switch (CurState) {
	case  BossState::APPEAR:
	{
		//�� óġ ���� ������ ���� ���� appear�� �ٲ㼭 ��ġ �̴ϸ� ��򰡿� ǥ�� �� �� �ִ� ���� �Ǵ� ������ �����̵� �����̵� setpos
		//this->SetPosition(TargetPos.x + 200.0f, TargetPos.y, TargetPos.z + 200.0f);
		SetPosition(0.0f + 10, 250.0f, 640.0f); 
		SendPosition();


		//�ϴ� ÷�� ��
		SetState(BossState::SLEEP);
		stateStartTime = steady_clock::now(); //�̰� �� ���� �� ai������ �ʱ�ȭ�� �ٲ�߰ٴ� ���߿�..

		break;
	}

	case  BossState::SLEEP:
	{

		CurMotion = BossAnimation::SLEEP;
		if(CurMotion !=PastMotion)
			SendAnimation();
		SendPosition();
		//���࿡ �÷��̾ ������ ���� idle�� ���� 
		if (Dist < 200.0f) {
			SetState(BossState::IDLE);
			stateStartTime = steady_clock::now();
		}
		break;
	}

	case BossState::IDLE: {

		CurMotion = BossAnimation::IDLE;
		if (CurMotion != PastMotion)
			SendAnimation();

		LookAtPosition(fTimeElapsed, TargetPos);
		SendPosition();

		/*if (Dist > 400.0f)
			SetState(BossState::SIT_IDLE);*/


		//�÷��̾�� �Ÿ��� �־����� �÷��̾� ����
		if (Dist > 400.0f) {
			SetState(BossState::CHASE);
			stateStartTime = steady_clock::now();
		}


		//attactCoolTime �ʸ��� ����
		if (duration_cast<seconds>(steady_clock::now() - stateStartTime).count() >= attactCoolTime) {
			SetState(BossState::ATTACT);
			randAttact = urdAttack(dree);
			stateStartTime = steady_clock::now();
			lastAttackTime = steady_clock::now();
		}


		//10�� ������ ��ũ�� �� ���ֱ�
		else if (float(MAXBossHP / bossHP) <= 0.1)
		{
			SetState(BossState::SCREAM);
			stateStartTime = steady_clock::now();
		}

		//���ǵǸ� �ǰݸ�� �ѹ����ֱ�
		else if (float(bossHP / MAXBossHP) <= 0.5)
		{
			SetState(BossState::GET_HIT);
			stateStartTime = steady_clock::now();
		}


		break;
	}

	case BossState::ATTACT: {

		LookAtPosition(fTimeElapsed, TargetPos);
		SendPosition();

		if (randAttact > 0.5f) {
			CurMotion = BossAnimation::BASIC_ATTACT;
			if (CurMotion != PastMotion)
				SendAnimation();
			PastState = (BossState)(BossAnimation::BASIC_ATTACT);
			if (player->GetHP() > 0) {
				player->GetAttack(2);
			}

			for (auto& pl : clients)
			{
				if (pl.in_use == false) continue;
				pl.send_bullet_hit_packet(0, -1, player->GetHP());
			}

			if (duration_cast<seconds>(steady_clock::now() - lastAttackTime).count() >= 2) {
				lastAttackTime = steady_clock::now();
				SetState(BossState::IDLE);
			}
		}
		else if (randAttact < 0.2f) {
			CurMotion = BossAnimation::FLAME_ATTACK;
			if (CurMotion != PastMotion)
				SendAnimation();;
			PastState = (BossState)(BossAnimation::FLAME_ATTACK);
			if (duration_cast<seconds>(steady_clock::now() - lastAttackTime).count() >= 4) {
				lastAttackTime = steady_clock::now();
				SetState(BossState::IDLE);
				
				if (player->GetHP() > 0) {
					player->GetAttack(10);
				}

				for (auto& pl : clients)
				{
					if (pl.in_use == false) continue;
					pl.send_bullet_hit_packet(0, -1, player->GetHP());
				}

			}
		}
		else
		{
			CurMotion = BossAnimation::CLAW_ATTACT;
			if (CurMotion != PastMotion)
				SendAnimation();
			PastState = (BossState)(BossAnimation::CLAW_ATTACT);

			if (player->GetHP() > 0) {
				player->GetAttack(5);
			}

			for (auto& pl : clients)
			{
				if (pl.in_use == false) continue;
				pl.send_bullet_hit_packet(0, -1, player->GetHP());
			}


			if (duration_cast<seconds>(steady_clock::now() - lastAttackTime).count() >= 3) {
				lastAttackTime = steady_clock::now();
				SetState(BossState::IDLE);

			}
		}

		break;
	}

	case BossState::SIT_IDLE: {
		CurMotion = BossAnimation::SIT_IDLE;
		if (CurMotion != PastMotion)
			SendAnimation();
		if (duration_cast<seconds>(steady_clock::now() - stateStartTime).count() >= 3) {
			SetState(BossState::IDLE);
			stateStartTime = steady_clock::now();
		}

		break;
	}
	case BossState::SCREAM: {
		CurMotion = BossAnimation::SCREAM;
		if (CurMotion != PastMotion)
			SendAnimation();;
		break;
	}

	case BossState::GET_HIT: {
		CurMotion = BossAnimation::GET_HIT;
		if (CurMotion != PastMotion)
			SendAnimation();
		cout << "GET_HIT" << endl;

		if (duration_cast<seconds>(steady_clock::now() - stateStartTime).count() >= 3) {
			SetState(BossState::IDLE);
			stateStartTime = steady_clock::now();
		}
		break;

	}

	case BossState::CHASE: {

		/*if(BossPos.z>=TargetPos.z - 10.0f && BossPos.z <= TargetPos.z + 10.0f)
		{
			CurMotion = BossAnimation::FLY_FORWARD;

			if (CurMotion != PastMotion)
				SendAnimation();
			PastState = (BossState)(BossAnimation::FLY_FORWARD);

		}*/

		{
			CurMotion = BossAnimation::FLY_FORWARD;

			if (CurMotion != PastMotion)
				SendAnimation();
			PastState = (BossState)(BossAnimation::FLY_FORWARD);
		}

		MoveBoss(fTimeElapsed, TargetPos ,Dist);
		SendPosition();

		if (Dist <= 400.0f) {
			SetState(BossState::IDLE);
			stateStartTime = steady_clock::now();
		}

		break;

	}

	case BossState::WALK: {
		CurMotion = BossAnimation::WALK;
		if (CurMotion != PastMotion)
			SendAnimation();
		cout << "WALK" << endl;

		break;

	}
	case BossState::RUN: {
		CurMotion = BossAnimation::RUN;
		if (CurMotion != PastMotion)
			SendAnimation();
		cout << "RUN" << endl;

		break;

	}
	case BossState::BASIC_ATTACT: {

		if (duration_cast<seconds>(steady_clock::now() - stateStartTime).count() >= 1) {
			SetState(BossState::IDLE);
			stateStartTime = steady_clock::now();
		}


		//cout << "BASIC_ATTACT" << endl;

		break;

	}
	case BossState::CLAW_ATTACT: {

		if (duration_cast<seconds>(steady_clock::now() - stateStartTime).count() >= 1) {
			SetState(BossState::IDLE);
			stateStartTime = steady_clock::now();
		}

		//cout << "CLAW_ATTACT" << endl;

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

	if ((CurState != BossState::ATTACT) || (CurState != BossState::CHASE))
		PastState = CurState;
}



