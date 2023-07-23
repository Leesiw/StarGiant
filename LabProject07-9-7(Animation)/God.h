#pragma once

#include "Object.h"
#include "Camera.h"
#include "Sound.h"

class God : public CGameObject
{
public:
	God();
	virtual ~God() {};

public:
	GodState CurState = GodState::IDLE1;
	GodState PastState = CurState;
	GodAnimation CurMotion = GodAnimation::IDLE1;
	GodAnimation PastMotion = CurMotion;

	int MAXGodHP = 100.0f;
	int GodHP = 100.0f;

public:
	void GodObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	GodState GetState() { return CurState; };
	GodState SetState(GodState GState) { return CurState = GState; };
	GodAnimation GetAnimation() { return CurMotion; };
	GodAnimation SetAnimation(GodAnimation GMotion) { return CurMotion = GMotion; };

	int GetMaxHp() { return MAXGodHP; }
	int GetcurHp() { return GodHP; }

	void ChangeAnimation(GodAnimation CurState);


	char soundon = -1;
	char soundo0nPAST = -1;
};