#include "stdafx.h"
#include "God.h"

God::God()
{
	MAXGodHP = 100.0f;
	GodHP = MAXGodHP;
	CurState = GodState::IDLE1;
	CurMotion = GodAnimation::IDLE1;
}

void God::GodObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pGodModel = pModel;
	if (!pGodModel) pGodModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/God.bin", NULL);

	SetChild(pGodModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, static_cast<int>(GodAnimation::COUNT), pGodModel);

	for (int i = 0; i < static_cast<int>(GodAnimation::COUNT); ++i) {
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}

	//m_pSkinnedAnimationController->SetTrackAllUnable(static_cast<int>(BossAnimation::COUNT));
	m_pSkinnedAnimationController->SetTrackEnable(0, true);
}

void God::ChangeAnimation(GodAnimation CurMotion)
{
	if (CurMotion == GodAnimation::MELEE1) {
		soundon = static_cast<int>(Sounds::GOD1);
	}
	else if (CurMotion == GodAnimation::MELEE2) {
		soundon = static_cast<int>(Sounds::GOD1);
	}
	else if (CurMotion == GodAnimation::SHOT) {
		soundon = static_cast<int>(Sounds::GOD2);
	}

	else {
		soundon = -1;
	}

	if (CurMotion != PastMotion) {

		if (CurMotion == GodAnimation::HIT1) {
		heal = true;
		soundon = -1;
		}

		if (CurMotion == GodAnimation::MELEE2) {
			m2 = true;
			soundon = -1;
		}
		if (CurMotion == GodAnimation::SHOT) {
			shot = true;
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



	if (CurMotion == GodAnimation::DEATH)
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

void God::Animate(float fTimeElapsed)
{
	m_pHead = FindFrame("UpperMouth02");

	CGameObject::Animate(fTimeElapsed);
}
