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


	//바운딩 박스 아직 설정안함 이거 드래곤꺼
	boundingbox = BoundingOrientedBox{ XMFLOAT3(-33.47668f, 41.86574f, 26.52405), XMFLOAT3(774.8785, 299.2372, 584.7963), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };

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

void God::SendPosition()
{
	SC_MOVE_ENEMY_PACKET p;

	p.size = sizeof(SC_MOVE_ENEMY_PACKET);
	p.type = SC_MOVE_ENEMY;

	p.data.id = GOD_ID;
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

void God::God_Ai(float fTimeElapsed, CAirplanePlayer* player, int godHp)
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

	SendPosition();
	SendAnimation();

	if (CurState != GodState::IDLE2)
	{
		LookAtPosition(fTimeElapsed, player->GetPosition());
	}
}
