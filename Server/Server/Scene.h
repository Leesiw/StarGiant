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
	void ReleaseObjects();

	void AnimateObjects(float fTimeElapsed);

	void CheckMeteoByPlayerCollisions();
	void CheckEnemyByBulletCollisions(BULLET_INFO& data);
	void CheckEnemyCollisions();

	void SpawnEnemy();

	CAirplanePlayer* m_pSpaceship = NULL;
	CTerrainPlayer* m_ppPlayers[3] = { NULL, NULL, NULL };

public:
	CMeteoObject*				m_ppMeteoObjects[METEOS];
	CEnemy*						m_ppEnemies[ENEMIES];

	std::map<ItemType, short>	items;

	bool						m_bIsRunning = true;

	float m_fEnemySpawnTime = 10.0f;
	float m_fEnemySpawnTimeRemaining = 0.0f;
};

