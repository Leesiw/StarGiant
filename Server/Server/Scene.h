#pragma once
#include "Session.h"
#include "Enemy.h"
#include "Boss.h"
#include "God.h"
#include "Missile.h"
#include "Timer.h"

extern concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;

extern array<SESSION, MAX_USER> clients;
extern std::unordered_map<MissionType, Level> levels;

enum SCENE_STATE { SCENE_FREE, SCENE_ALLOC, SCENE_INGAME, SCENE_RESET };
					// 아무도x, 누군가 기다림, 게임 중, 리셋 중
class CScene
{
public:
	CScene();
	~CScene();

	void Init();

	void BuildObjects();
	void ReleaseObjects();

	void ResetScene();
	void Reset();

	void AnimateObjects(float fTimeElapsed);

	void CheckMeteoByPlayerCollisions();
	void CheckEnemyByBulletCollisions(BULLET_INFO& data);
	void CheckMeteoByBulletCollisions(BULLET_INFO& data);
	void CheckEnemyCollisions();
	void CheckMissileCollisions();
	void CheckBossCollisions();

	void CheckMissionComplete();
	void MissionClear();
	void SetMission(MissionType mission);
	void SetMissionFindBoss();

	void GetJewels();

	void SpawnEnemy();
	void MoveEnemy(char obj_id);
	void AimingEnemy(char obj_id);
	void UpdateMeteo(char obj_id);
	void SpawnMissile(char obj_id);
	void UpdateMissile(char obj_id);
	void UpdateBoss();
	void UpdateGod();
	void UpdateSpaceship();
	void Heal();
	void SendSceneInfo();
	void BlackHole();
	void CheckCutsceneEnd(MissionType next_mission);

	void SpawnEnemyFromGod();
	void SpawnEnemyFromGod(char id, char num);

	void ChangeInvincibleMode();

	void SpawnEnemy(char id);

	void SpawnMeteo(char i);

	void Send(char* p);

	void Start();
	char InsertPlayer(short pl_id);

	CAirplanePlayer* m_pSpaceship = NULL;
	CTerrainPlayer* m_ppPlayers[3] = { NULL, NULL, NULL };

public:
	std::array<CMeteoObject*, METEOS>			m_ppMeteoObjects;
	std::array<CEnemy*, ENEMIES>				m_ppEnemies;
	std::array<CMissile*, ENEMY_BULLETS>		m_ppMissiles;

	std::unordered_map<ItemType, char>	items;

	Boss* m_pBoss;
	God* m_pGod;


	short _id;
	short num;

	std::chrono::system_clock::time_point heal_start;
	std::atomic_char heal_player = -1;

//	bool						m_bIsRunning = true;

	char						kill_monster_num = 0;

	MissionType cur_mission;
	MissionType prev_mission;

	std::atomic_bool can_sit[4];

	SCENE_STATE _state;
	mutex _s_lock;

	array<short, 3> _plist;
	mutex _plist_lock;

	XMFLOAT3 black_hole_pos;
	chrono::steady_clock::time_point b_prev_time;
	float black_hole_time;

	bool invincible_mode = false;

	bool boss_timer_on = false;
	bool god_timer_on = false;
};
