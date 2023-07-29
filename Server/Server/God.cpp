#pragma once

#include "stdafx.h"
#include "SceneManager.h"
#include "Boss.h"
#include "God.h"
extern SceneManager scene_manager;


God::God()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();


	//바운딩 박스 맞는지..확인필요함
	//boundingbox = BoundingOrientedBox{ XMFLOAT3(0.0674829, -1.20364, -0.872784), XMFLOAT3(20.2724, 11.4494, 13.567), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	boundingbox = BoundingOrientedBox{ XMFLOAT3(0.3990545, 0.241102, 0.0173169), XMFLOAT3(215.49016, 132.472916, 273.17238), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };

	// Lua 상태 생성
	m_L = luaL_newstate();
	luaL_openlibs(m_L);

	// Lua 파일 불러오기
	if (luaL_loadfile(m_L, "god_ai.lua") || lua_pcall(m_L, 0, 0, 0))
	{
		const char* error = lua_tostring(m_L, -1);
		printf("Error loading boss_ai.lua: %s\n", error);
	}

	XMFLOAT3 position;

	position = { position.x = 0.0, position.y = 0.0, position.z = 0.0 };


	lua_getglobal(m_L, "state");
	lua_getglobal(m_L, "motion");
	lua_getglobal(m_L, "MaxHp");

	lua_getglobal(m_L, "god_x");
	lua_getglobal(m_L, "god_y");
	lua_getglobal(m_L, "god_z");

	CurState = GodState(lua_tonumber(m_L, -6));
	CurMotion = GodAnimation(lua_tonumber(m_L, -5));

	MAXGodHP = lua_tonumber(m_L, -4);
	SetPosition(lua_tonumber(m_L, -3), lua_tonumber(m_L, -2), lua_tonumber(m_L, -1));

	lua_pop(m_L, 6);
}

God::~God()
{
	//Lua 상태 닫기
	lua_close(m_L);
}

void God::Reset()
{
	GodHP = MAXGodHP;

	CurState = GodState::IDLE1;
	PastState = CurState;
	CurMotion = GodAnimation::IDLE1;
	PastMotion = CurMotion;
}

void God::SendPosition()
{
	SC_MOVE_BOSS_PACKET p;

	p.size = sizeof(SC_MOVE_BOSS_PACKET);
	p.type = SC_MOVE_GOD;

	p.data.Quaternion = GetQuaternion();
	p.data.pos = GetPosition();

	scene_manager.Send(scene_num, (char*)&p);
}

void God::SendAnimation()
{
	SC_ANIMATION_CHANGE_PACKET p;
	p.size = sizeof(SC_ANIMATION_CHANGE_PACKET);
	p.type = SC_ANIMATION_CHANGE;
	p.data.id = GOD_ID;
	p.data.animation = (char)CurMotion;

	scene_manager.Send(scene_num, (char*)&p);
}

XMFLOAT4 God::GetQuaternion()
{
	XMMATRIX mat = XMLoadFloat4x4(&m_xmf4x4ToParent);
	XMVECTOR vec = XMQuaternionRotationMatrix(mat); 
	XMFLOAT4 xmf4;
	XMStoreFloat4(&xmf4, vec);
	return xmf4;
}

void God::LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos)
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

bool God::God_Ai(float fTimeElapsed, CAirplanePlayer* player, int godHp)
{
	float x, y, z;
	x = float(player->GetPosition().x);
	y = float(player->GetPosition().y);
	z = float(player->GetPosition().z);

	lua_getglobal(m_L, "updateGodAI");
	lua_pushnumber(m_L, GodHP);
	lua_pushnumber(m_L, x);
	lua_pushnumber(m_L, y);
	lua_pushnumber(m_L, z);
	lua_pushnumber(m_L, fTimeElapsed);
	lua_pcall(m_L, 5, 0, 0);

	lua_getglobal(m_L, "god_x"); lua_getglobal(m_L, "god_y"); lua_getglobal(m_L, "god_z");
	SetPosition(lua_tonumber(m_L, -3), lua_tonumber(m_L, -2), lua_tonumber(m_L, -1));
	lua_pop(m_L, 3);

	lua_getglobal(m_L, "state");
	lua_getglobal(m_L, "motion");

	CurState = GodState(lua_tonumber(m_L, -2));
	CurMotion = GodAnimation(lua_tonumber(m_L, -1));
	lua_pop(m_L, 2);

	if (CurMotion != GodAnimation::IDLE2)
	{
		LookAtPosition(fTimeElapsed, player->GetPosition());
	}

	SendPosition();

	if (PastMotion != CurMotion) {
		SendAnimation();

		if (CurMotion == GodAnimation::SHOT) player->GetAttack(2);
		if (CurMotion == GodAnimation::MELEE1) player->GetAttack(4);


		PastMotion = CurMotion;
		SC_BULLET_HIT_PACKET p;
		p.size = sizeof(SC_BULLET_HIT_PACKET);
		p.type = SC_BULLET_HIT;
		p.data.id = -1;
		p.data.hp = player->GetHP();
		scene_manager.Send(scene_num, (char*)&p);


		if (CurMotion == GodAnimation::ROAR)
			return true;

		
	}

	return false;

}
