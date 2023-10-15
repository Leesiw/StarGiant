#pragma once

#include "stdafx.h"
#include "SceneManager.h"
#include "Boss.h"
extern SceneManager scene_manager;


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
		meteo->SetMovingDirection(XMFLOAT3(urdScale(dree), urdScale(dree), urdScale(dree)));
		meteo->SetMovingSpeed(urdSpeed(dree));
		//meteo->SetScale(100.0f, 100.0f, 100.0f);
		if (i < BOSSMETEOS / 2) {
			// 바운딩 박스 설정 부탁
			meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{  -0.0167256, 0.71804,  -0.0466012 }, XMFLOAT3{ 441.4825, 429.032, 414.356 }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
			//meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.188906f, 0.977625f, 0.315519f }, XMFLOAT3{ 1.402216f, 1.458820f, 1.499708f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
		}
		else {
			meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{  -0.0167256, 0.71804,  -0.0466012 }, XMFLOAT3{ 441.4825, 429.032, 414.356 }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
		}
		m_ppBossMeteoObjects[i] = meteo;
		m_ppBossMeteoObjects[i]->UpdateTransform(NULL);
	}

	//boundingbox = BoundingOrientedBox{ XMFLOAT3(0.f, 34.65389f, -10.1982f), XMFLOAT3(65.5064392f, 35.0004547f, 77.9787476f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	boundingbox = BoundingOrientedBox{ XMFLOAT3(-33.47668f, 41.86574f, 26.52405), XMFLOAT3(774.8785, 299.2372, 584.7963), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };

	// Lua 상태 생성
	m_L = luaL_newstate();
	luaL_openlibs(m_L);

	// Lua 파일 불러오기
	if (luaL_loadfile(m_L, "boss_ai.lua") || lua_pcall(m_L, 0, 0, 0))
	{
		const char* error = lua_tostring(m_L, -1);
		printf("Error loading boss_ai.lua: %s\n", error);
	}

	XMFLOAT3 position;

	position = { position.x = 0.0, position.y = 0.0, position.z = 0.0 };


	lua_getglobal(m_L, "state");
	lua_getglobal(m_L, "motion");
	lua_getglobal(m_L, "MaxHp");

	lua_getglobal(m_L, "boss_x");
	lua_getglobal(m_L, "boss_y");
	lua_getglobal(m_L, "boss_z");


	CurState = BossState(lua_tonumber(m_L, -6));
	CurMotion = BossAnimation(lua_tonumber(m_L, -5));

	MAXBossHP = lua_tonumber(m_L, -4);
	SetPosition(lua_tonumber(m_L, -3), lua_tonumber(m_L, -2), lua_tonumber(m_L, -1));


	//cout << "getpos - " << GetPosition().x << "\n";
	//cout << "getpos - " << GetPosition().y << "\n";
	//cout << "getpos - " << GetPosition().z << "\n";
	//cout << "CurState - " << int(CurState) <<"\n";
	//cout << "CurMotion - " << int(CurMotion) << "\n";
	//cout << "MAXBossHP - "<< MAXBossHP << "\n";
	//cout << "getpos - " << GetPosition().y << "\n";
	lua_pop(m_L, 6);

	////test
	//float x, y, z;
	//x = float(GetPosition().x);
	//y = float(GetPosition().y);
	//z = float(GetPosition().z);

	//lua_getglobal(m_L, "updateBossAI");
	//lua_pushnumber(m_L, MAXBossHP);

	//lua_pushnumber(m_L, x);
	//lua_pushnumber(m_L, y);
	//lua_pushnumber(m_L, z);
	//lua_pushnumber(m_L, 0);


	//lua_pcall(m_L, 5, 0, 0);	// 파라미터 개수, 리턴값 개수, 핸들러
	////int result = lua_tonumber(m_L, -1);	// 리턴값 맨 꼭대기 값
	////lua_pop(m_L, 1);

	//if (lua_pcall(m_L, 0, 0, 0) != LUA_OK)
	//{
	//	const char* error = lua_tostring(m_L, -1);
	//	printf("Error calling updateBossAI: %s\n", error);
	//}

	//lua_getglobal(m_L, "boss_x"); lua_getglobal(m_L, "boss_y"); lua_getglobal(m_L, "boss_z");
	//SetPosition(lua_tonumber(m_L, -3), lua_tonumber(m_L, -2), lua_tonumber(m_L, -1));
	//lua_pop(m_L, 3);
	//cout << "getpos - " << GetPosition().x << "\n";

}
Boss::~Boss() {
	for (int i = 0; i < m_ppBossMeteoObjects.size(); ++i)
	{
		if (m_ppBossMeteoObjects[i]) { delete m_ppBossMeteoObjects[i]; }
	}

	// Lua 상태 닫기
	lua_close(m_L);
}



void Boss::SendPosition()	// 위치/각도 변화할 때 사용. 
{
	SC_MOVE_BOSS_PACKET p;

	p.size = sizeof(SC_MOVE_BOSS_PACKET);
	p.type = SC_MOVE_BOSS;

	p.data.Quaternion = GetQuaternion();
	p.data.pos = GetPosition();

	scene_manager.Send(scene_num, (char*)&p);
}

void Boss::SendAnimation() // 애니메이션 변화했을 때 사용
{
	SC_ANIMATION_CHANGE_PACKET p;
	p.size = sizeof(SC_ANIMATION_CHANGE_PACKET);
	p.type = SC_ANIMATION_CHANGE;
	p.data.id = BOSS_ID;
	p.data.animation = (char)CurMotion;

	scene_manager.Send(scene_num, (char*)&p);
}

XMFLOAT4 Boss::GetQuaternion()
{
	XMMATRIX mat = XMLoadFloat4x4(&m_xmf4x4ToParent);
	XMVECTOR vec = XMQuaternionRotationMatrix(mat);
	XMFLOAT4 xmf4;
	XMStoreFloat4(&xmf4, vec);
	return xmf4;
}

void Boss::Reset()
{
	//m_xmf4x4ToParent = Matrix4x4::Identity();
	//m_xmf4x4World = Matrix4x4::Identity();

	CurState = BossState::SLEEP;
	PastState = CurState;
	NextState = CurState;

	CurMotion = BossAnimation::SLEEP;
	PastMotion = CurMotion;

	condition = false;

	SetPosition(2300.f, 0.f, 0.f);
	BossHP = 100;

	MAXBossHP = 100.0f;
	BossHP = MAXBossHP;
	CurMotion = BossAnimation::SLEEP;
	CurState = BossState::SLEEP;
}

void Boss::MeteoAttack(float fTimeElapsed, const XMFLOAT3& TargetPos) // 공격 시작 시 한 번 실행
{
	XMFLOAT3 xmf3Pos = GetPosition();
	xmf3Pos.y + 1000.f;
	XMFLOAT3 player_pos = TargetPos;
	XMFLOAT3 xmfToPlayer = Vector3::Subtract(player_pos, xmf3Pos);
	xmfToPlayer = Vector3::TransformCoord(xmfToPlayer, Matrix4x4::RotationAxis(GetUp(), urdAngle(dree)));

	XMFLOAT3 directions[5] = {
	   xmfToPlayer, // 중심 방향
	   Vector3::TransformNormal(XMFLOAT3(0.f, 0.f, 1.f), XMMatrixRotationY(XMConvertToRadians(45.f))), // 위쪽 방향
	   Vector3::TransformNormal(XMFLOAT3(0.f, 0.f, 1.f), XMMatrixRotationY(XMConvertToRadians(-45.f))), // 아래쪽 방향
	   Vector3::TransformNormal(XMFLOAT3(1.f, 0.f, 0.f), XMMatrixRotationY(XMConvertToRadians(45.f))), // 왼쪽 방향
	   Vector3::TransformNormal(XMFLOAT3(1.f, 0.f, 0.f), XMMatrixRotationY(XMConvertToRadians(-45.f))) // 오른쪽 방향
	};

	for (int i = 0; i < BOSSMETEOS; ++i) {
		m_ppBossMeteoObjects[i]->SetPosition(xmf3Pos);
		m_ppBossMeteoObjects[i]->SetMovingDirection(directions[i]);
		m_ppBossMeteoObjects[i]->SetMovingSpeed(urdSpeed(dree));
	}

}

void Boss::MoveMeteo(float fTimeElapsed)		// 메테오 움직여야 할때 계속 실행. send까지 포함됨
{
	for (int i = 0; i < BOSSMETEOS; ++i) {
		m_ppBossMeteoObjects[i]->Animate(fTimeElapsed);

		/*printf("pos : %f, %f, %f\n",
			m_ppBossMeteoObjects[0]->GetPosition().x, m_ppBossMeteoObjects[0]->GetPosition().y, m_ppBossMeteoObjects[0]->GetPosition().z);*/
	}


	SC_METEO_PACKET p;
	p.size = sizeof(SC_METEO_PACKET);
	p.type = SC_METEO;

	for (int i = 0; i < BOSSMETEOS; ++i) {
		p.data.id = METEOS + i;
		p.data.pos = m_ppBossMeteoObjects[i]->GetPosition();
		scene_manager.Send(scene_num, (char*)&p);
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


void Boss::LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos)
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
		Rotate(0.f, p_y_r.y * rotate_angle, 0.f);
	}
	else {
		Rotate(0.f, yaw, 0.f);
	}
}


void Boss::Boss_Ai(float fTimeElapsed, CAirplanePlayer* player, int bossHP)
{
	//test
	float x, y, z;
	x = float(player->GetPosition().x);
	y = float(player->GetPosition().y);
	z = float(player->GetPosition().z);

	lua_getglobal(m_L, "updateBossAI");
	lua_pushnumber(m_L, bossHP);
	lua_pushnumber(m_L, x);
	lua_pushnumber(m_L, y);
	lua_pushnumber(m_L, z);
	lua_pushnumber(m_L, fTimeElapsed);

	lua_pcall(m_L, 5, 0, 0);	// 파라미터 개수, 리턴값 개수, 핸들러
	//int result = lua_tonumber(m_L, -1);	// 리턴값 맨 꼭대기 값
	//lua_pop(m_L, 1);

	//if (lua_pcall(m_L, 0, 0, 0) != LUA_OK)
	//{
	//	const char* error = lua_tostring(m_L, -1);
	//	printf("Error calling updateBossAI: %s\n", error);
	//}

	lua_getglobal(m_L, "boss_x"); lua_getglobal(m_L, "boss_y"); lua_getglobal(m_L, "boss_z");
	SetPosition(lua_tonumber(m_L, -3), lua_tonumber(m_L, -2), lua_tonumber(m_L, -1));
	lua_pop(m_L, 3);
	/*cout << "getpos - " << GetPosition().x << "\n";
	cout << "getpos - " << GetPosition().y << "\n";
	cout << "getpos - " << GetPosition().z << "\n";*/

	lua_getglobal(m_L, "state");
	lua_getglobal(m_L, "motion");


	CurState = BossState(lua_tonumber(m_L, -2));
	CurMotion = BossAnimation(lua_tonumber(m_L, -1));
	lua_pop(m_L, 2);
	//cout << " CurState -" << int(CurState) << endl;
	//cout << " CurMotion -" << int(CurMotion) << endl;


	SendPosition();
	if (PastMotion != CurMotion) {
		SendAnimation();

		if (CurMotion == BossAnimation::BASIC_ATTACT) player->GetAttack(2);
		if (CurMotion == BossAnimation::CLAW_ATTACT) player->GetAttack(4);
		if (CurMotion == BossAnimation::FLAME_ATTACK) player->GetAttack(6);


		PastMotion = CurMotion;
		SC_BULLET_HIT_PACKET p;
		p.size = sizeof(SC_BULLET_HIT_PACKET);
		p.type = SC_BULLET_HIT;
		p.data.id = -1;
		p.data.hp = player->GetHP();
		scene_manager.Send(scene_num, (char*)&p);
	}



	if (CurState != BossState::SLEEP)
	{
		LookAtPosition(fTimeElapsed, player->GetPosition());
	}


}



