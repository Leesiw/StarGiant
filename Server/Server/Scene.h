#pragma once
#include "Session.h"

extern array<SESSION, MAX_USER> clients;

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

	void CheckObjectByPlayerCollisions();
	void CheckObjectByBulletCollisions();
	void CheckEnemyByBulletCollisions();

	CAirplanePlayer* m_pSpaceship = NULL;
	CPlayer* m_ppPlayers[3] = { NULL, NULL, NULL };
	//CEnemyObject* m_enemy = NULL;

public:
	CMeteoObject**				m_ppMeteoObjects = NULL;
	int							m_nMeteoObjects = 0;

	float						m_fElapsedTime = 0.0f;
};

