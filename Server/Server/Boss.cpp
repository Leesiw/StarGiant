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
		meteo->SetMovingDirection(XMFLOAT3(urdPos(dree), urdPos(dree), urdPos(dree)));
		meteo->SetScale(100.0f, 100.0f, 100.0f);
		if (i < BOSSMETEOS / 2) {
			// �ٿ�� �ڽ� ���� ��Ź
			meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{  -0.0167256, 0.71804,  -0.0466012 }, XMFLOAT3{ 4.414825, 4.29032, 4.14356 }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
			//meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.188906f, 0.977625f, 0.315519f }, XMFLOAT3{ 1.402216f, 1.458820f, 1.499708f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
		}
		else {
			meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{  -0.0167256, 0.71804,  -0.0466012 }, XMFLOAT3{ 4.414825, 4.29032, 4.14356 }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
		}
		m_ppBossMeteoObjects[i] = meteo;
		m_ppBossMeteoObjects[i]->UpdateTransform(NULL);
	}

	//boundingbox = BoundingOrientedBox{ XMFLOAT3(0.f, 34.65389f, -10.1982f), XMFLOAT3(65.5064392f, 35.0004547f, 77.9787476f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	boundingbox = BoundingOrientedBox{ XMFLOAT3(-33.47668f, 41.86574f, 26.52405), XMFLOAT3(774.8785, 299.2372, 584.7963), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };

	// Lua ���� ����
	m_L = luaL_newstate();
	luaL_openlibs(m_L);

	// Lua ���� �ҷ�����
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


	//lua_pcall(m_L, 5, 0, 0);	// �Ķ���� ����, ���ϰ� ����, �ڵ鷯
	////int result = lua_tonumber(m_L, -1);	// ���ϰ� �� ����� ��
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

	// Lua ���� �ݱ�
	lua_close(m_L);
}



void Boss::SendPosition()	// ��ġ/���� ��ȭ�� �� ���. 
{
	SC_MOVE_ENEMY_PACKET p;

	p.size = sizeof(SC_MOVE_ENEMY_PACKET);
	p.type = SC_MOVE_ENEMY;

	p.data.id = BOSS_ID;
	p.data.Quaternion = GetQuaternion();
	p.data.pos = GetPosition();

	scene_manager.Send(scene_num, (char*)&p);
}

void Boss::SendAnimation() // �ִϸ��̼� ��ȭ���� �� ���
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

void Boss::MeteoAttack(float fTimeElapsed, const XMFLOAT3& TargetPos) // ���� ���� �� �� �� ����
{
	XMFLOAT3 xmf3Pos = GetPosition();
	xmf3Pos.y + 1000.f;
	XMFLOAT3 player_pos = TargetPos;
	XMFLOAT3 xmfToPlayer = Vector3::Subtract(player_pos, xmf3Pos);
	xmfToPlayer = Vector3::TransformCoord(xmfToPlayer, Matrix4x4::RotationAxis(GetUp(), urdAngle(dree)));
	
	XMFLOAT3 directions[5] = {
	   xmfToPlayer, // �߽� ����
	   Vector3::TransformNormal(XMFLOAT3(0.f, 0.f, 1.f), XMMatrixRotationY(XMConvertToRadians(45.f))), // ���� ����
	   Vector3::TransformNormal(XMFLOAT3(0.f, 0.f, 1.f), XMMatrixRotationY(XMConvertToRadians(-45.f))), // �Ʒ��� ����
	   Vector3::TransformNormal(XMFLOAT3(1.f, 0.f, 0.f), XMMatrixRotationY(XMConvertToRadians(45.f))), // ���� ����
	   Vector3::TransformNormal(XMFLOAT3(1.f, 0.f, 0.f), XMMatrixRotationY(XMConvertToRadians(-45.f))) // ������ ����
	};

	for (int i = 0; i < BOSSMETEOS; ++i) {
		m_ppBossMeteoObjects[i]->SetPosition(xmf3Pos);
		m_ppBossMeteoObjects[i]->SetMovingDirection(directions[i]);
		m_ppBossMeteoObjects[i]->SetMovingSpeed(10000.f);
	}

}

void Boss::MoveMeteo(float fTimeElapsed)		// ���׿� �������� �Ҷ� ��� ����. send���� ���Ե�
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
	//�ӵ� ��������ߵ�
	XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(GetLook(), speed * fTimeElapsed, true);
	BossPos = Vector3::Add(BossPos, xmf3Movement);
	SetPosition(BossPos);


}


void Boss::LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos)
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

	lua_pcall(m_L, 5, 0, 0);	// �Ķ���� ����, ���ϰ� ����, �ڵ鷯
	//int result = lua_tonumber(m_L, -1);	// ���ϰ� �� ����� ��
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

	lua_getglobal(m_L, "boss_z");
	lua_getglobal(m_L, "state");
	lua_getglobal(m_L, "motion");


	CurState = BossState(lua_tonumber(m_L, -2));
	CurMotion = BossAnimation(lua_tonumber(m_L, -1));
	lua_pop(m_L, 2);
	//cout << " CurState -" << int(CurState) << endl;
	//cout << " CurMotion -" << int(CurMotion) << endl;



	SendPosition();
	SendAnimation();
	

	if (CurState != BossState::SLEEP)
	{
		LookAtPosition(fTimeElapsed, player->GetPosition());
	}


}



