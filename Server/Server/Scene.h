#pragma once
#include "Player.h"


class CScene
{
public:
	CScene();
	~CScene();

	void Init();

	void BuildObjects();
	void BuildObjects2();
	void ReleaseObjects();
	void MoveMeteo(float fTimeElapsed);

	bool ProcessInput(UCHAR* pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);

	void ReleaseUploadBuffers();
	void CheckObjectByPlayerCollisions();


	void CheckObjectByBulletCollisions();

	CPlayer* m_pPlayer = NULL;

public:
	CGameObject** m_ppGameObjects = NULL;
	int							m_nGameObjects = 0;

	float						m_fElapsedTime = 0.0f;

};

