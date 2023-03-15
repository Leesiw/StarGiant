#pragma once
#include "Session.h"
#include "Enemy.h"
#include "Boss.h"
#include "Missile.h"

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
	void CheckMeteoByBulletCollisions(BULLET_INFO& data);
	void CheckEnemyCollisions();
	void CheckMissileCollisions();

	void SpawnEnemy();
	void SpawnMeteo(char i);

	CAirplanePlayer* m_pSpaceship = NULL;
	CTerrainPlayer* m_ppPlayers[3] = { NULL, NULL, NULL };

public:
	CMeteoObject*				m_ppMeteoObjects[METEOS];
	CEnemy*						m_ppEnemies[ENEMIES];
	CMissile*					m_ppMissiles[ENEMY_BULLETS];

	Boss*						m_pBoss;

	std::unordered_map<ItemType, char>	items;

	bool						m_bIsRunning = true;

	float m_fEnemySpawnTime = 10.0f;
	float m_fEnemySpawnTimeRemaining = 0.0f;
};

