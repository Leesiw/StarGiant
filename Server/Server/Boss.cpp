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

	CMeteoObject* meteo;
	for (int i = 0; i < BOSSMETEOS; ++i) {
		meteo = new CMeteoObject();
		meteo->SetPosition(urdPos(dree), urdPos(dree), urdPos(dree));
		meteo->SetMovingDirection(XMFLOAT3(urdPos(dree), urdPos(dree), urdPos(dree)));
		meteo->SetScale(3.f, 3.f, 3.f);
		if (i < BOSSMETEOS / 2) {
			// 바운딩 박스 설정 부탁
			meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.188906f, 0.977625f, 0.315519f }, XMFLOAT3{ 1.402216f, 1.458820f, 1.499708f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
		}
		else {
			meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{  -0.0167256, 0.71804,  -0.0466012 }, XMFLOAT3{ 4.414825, 4.29032, 4.14356 }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
		}
		m_ppBossMeteoObjects[i] = meteo;
		m_ppBossMeteoObjects[i]->UpdateTransform(NULL);
	}

	//boundingbox = BoundingOrientedBox{ XMFLOAT3(0.f, 34.65389f, -10.1982f), XMFLOAT3(65.5064392f, 35.0004547f, 77.9787476f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	boundingbox = BoundingOrientedBox{ XMFLOAT3(-33.47668f, 41.86574f, 26.52405), XMFLOAT3(774.8785, 299.2372, 584.7963), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
}

Boss::~Boss() {
	for (int i = 0; i < m_ppBossMeteoObjects.size(); ++i)
	{
		if (m_ppBossMeteoObjects[i]) { delete m_ppBossMeteoObjects[i]; }
	}
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

void Boss::MeteoAttack(float fTimeElapsed, const XMFLOAT3& TargetPos) // 공격 시작 시 한 번 실행
{
	XMFLOAT3 xmf3Pos = GetPosition();
	XMFLOAT3 player_pos = TargetPos;
	XMFLOAT3 xmfToPlayer = Vector3::Subtract(player_pos, xmf3Pos);
	xmfToPlayer = Vector3::TransformCoord(xmfToPlayer, Matrix4x4::RotationAxis(GetUp(), urdAngle(dree)));
	
	for (int i = 0; i < BOSSMETEOS; ++i) {
		m_ppBossMeteoObjects[i]->SetPosition(xmf3Pos);
		// 현재 한 방향으로만 감 벡터 흔들기 등으로 방향 변경
		m_ppBossMeteoObjects[i]->SetMovingDirection(xmfToPlayer);
		m_ppBossMeteoObjects[i]->SetMovingSpeed(5.f);
	}
}

void Boss::MoveMeteo(float fTimeElapsed)		// 메테오 움직여야 할때 계속 실행. send까지 포함됨
{
	for (int i = 0; i < BOSSMETEOS; ++i) {
		m_ppBossMeteoObjects[i]->Animate(fTimeElapsed);
	}

	for (auto& pl : clients) {
		if (pl.in_use == false) continue;
		pl.send_boss_meteo_packet(0, m_ppBossMeteoObjects);
	}
}

void Boss::MoveBoss(float fTimeElapsed, XMFLOAT3 TargetPos, float dist)
{
	/*XMFLOAT3 BossPos = GetPosition();
	XMFLOAT3 chaseDirection = Vector3::Subtract(TargetPos, BossPos);*/

	LookAtPosition(fTimeElapsed, TargetPos);

	XMFLOAT3 BossPos = GetPosition();
	//속도 변경해줘야됨
	XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(GetLook(), speed * fTimeElapsed, true);
	BossPos = Vector3::Add(BossPos, xmf3Movement);
	SetPosition(BossPos);


}


void  Boss::LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos)
{
	XMFLOAT3 new_pos = pos;
	XMMATRIX inv_mat = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World)); // 역행렬

	new_pos = Vector3::TransformCoord(new_pos, inv_mat); // 타겟의 위치를 적 자체의 좌표계로 변환
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

	static int a=0;

	switch (CurState) {
	case  BossState::APPEAR:
	{
		//적 처치 조건 맞으면 보스 상태 appear로 바꿔서 위치 미니맵 어딘가에 표시 될 수 있는 정도 되는 곳으로 랜덤이든 고정이든 setpos
		//this->SetPosition(TargetPos.x + 200.0f, TargetPos.y, TargetPos.z + 200.0f);
		SetPosition(0.0f + 10, 250.0f, 640.0f); 
		SendPosition();


		//일단 첨엔 잠
		SetState(BossState::SLEEP);
		stateStartTime = steady_clock::now(); //이거 다 쓰면 걍 ai끝날때 초기화로 바꿔야겟다 나중에..

		break;
	}

	case  BossState::SLEEP:
	{

		CurMotion = BossAnimation::SLEEP;
		if(CurMotion !=PastMotion)
			SendAnimation();
		SendPosition();
		//만약에 플레이어가 가까이 오면 idle로 가기 
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


		//플레이어와 거리가 멀어지면 플레이어 추적
		if (Dist > 400.0f) {
			SetState(BossState::CHASE);
			stateStartTime = steady_clock::now();
		}


		//attactCoolTime 초마다 공격
		if (duration_cast<seconds>(steady_clock::now() - stateStartTime).count() >= attactCoolTime) {
			SetState(BossState::ATTACT);
			randAttact = urdAttack(dree);
			stateStartTime = steady_clock::now();
			lastAttackTime = steady_clock::now();
		}


		//10퍼 남으면 스크림 함 해주기
		else if (float(MAXBossHP / bossHP) <= 0.1 && a == 1)
		{
			SetState(BossState::SCREAM);
			stateStartTime = steady_clock::now();
			a = 2;
		}

		//반피되면 피격모션 한번해주기
		else if (float(bossHP / MAXBossHP) <= 0.5&& a == 0)
		{
			SetState(BossState::GET_HIT);
			stateStartTime = steady_clock::now();
			a = 1;
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
		//cout << "GET_HIT" << endl;

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



