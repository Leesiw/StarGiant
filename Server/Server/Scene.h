#pragma once
#include "Session.h"
#include "Enemy.h"
#include "Boss.h"
#include "Missile.h"

extern array<SESSION, MAX_USER> clients;
extern std::unordered_map<MissionType, Level> levels;

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
	void CheckBossCollisions();

	void CheckMissionComplete();
	void MissionClear();

	void GetJewels();

	void SpawnEnemy();
	void SpawnMeteo(char i);

	CAirplanePlayer* m_pSpaceship = NULL;
	CTerrainPlayer* m_ppPlayers[3] = { NULL, NULL, NULL };

public:
	std::array<CMeteoObject*, METEOS>			m_ppMeteoObjects;
	std::array<CEnemy*, ENEMIES>				m_ppEnemies;
	std::array<CMissile*, ENEMY_BULLETS>		m_ppMissiles;

	std::unordered_map<ItemType, char>	items;

	Boss* m_pBoss;

	std::chrono::system_clock::time_point heal_start;
	char heal_player = -1;

	bool						m_bIsRunning = true;

	char						kill_monster_num = 0;
	char						cur_monster_num = 0;

	MissionType cur_mission;

	float m_fEnemySpawnTime = 20.0f;
	float m_fEnemySpawnTimeRemaining = 0.0f;
};

