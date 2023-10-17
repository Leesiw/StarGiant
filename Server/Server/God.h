#pragma once

#include "Object.h"
#include "Session.h"
#include "include/lua.hpp"
#pragma comment (lib, "lua54.lib")

class God : public CGameObject
{
public:
	God();
	virtual ~God();

public:
	GodState CurState = GodState::IDLE1;
	GodState PastState = CurState;
	GodAnimation CurMotion = GodAnimation::IDLE1;
	GodAnimation PastMotion = CurMotion;

	int MAXGodHP = 100.0f;
	int GodHP = 100.0f;

	// Lua ป๓ลย
	lua_State* m_L;

public:
	short scene_num;

public:
	void Reset();
	void modifyLua();

	GodState GetState() { return CurState; };
	GodState SetState(GodState GState) { return CurState = GState; };
	GodAnimation GetAnimation() { return CurMotion; };
	GodAnimation SetAnimation(GodAnimation GMotion) { return CurMotion = GMotion; };

	int GetMaxHp() { return MAXGodHP; }
	int GetcurHp() { return GodHP; }

	void SendPosition();
	void SendAnimation();

	XMFLOAT4 GetQuaternion();
	void LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos);

	bool God_Ai(float fTimeElapsed, CAirplanePlayer* player, int godHp);
};