#pragma once
#include "Session.h"
#include "Enemy.h"

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

	bool ProcessInput(UCHAR* pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);

	void CheckMeteoByPlayerCollisions();
	void CheckEnemyByBulletCollisions(BULLET_INFO& data);
	void CheckEnemyCollisions();

	void SpawnEnemy();

	CAirplanePlayer* m_pSpaceship = NULL;
	CPlayer* m_ppPlayers[3] = { NULL, NULL, NULL };

public:
	CMeteoObject*				m_ppMeteoObjects[METEOS];
	CEnemy*						m_ppEnemies[ENEMIES];
	short						m_sEnemyNum;

	float						m_fElapsedTime = 0.0f;
};

