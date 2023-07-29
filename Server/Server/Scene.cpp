//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

CScene::CScene()
{
	for (int i = 0; i < m_ppEnemies.size(); ++i)
	{
		m_ppEnemies[i] = NULL;
	}

	for (int i = 0; i < m_ppMeteoObjects.size(); ++i)
	{
		m_ppMeteoObjects[i] = NULL;
	}

	for (int i = 0; i < m_ppMissiles.size(); ++i)
	{
		m_ppMissiles[i] = NULL;
	}

	items[ItemType::JEWEL_ATT] = 0;
	items[ItemType::JEWEL_DEF] = 0;
	items[ItemType::JEWEL_HEAL] = 0;
	items[ItemType::JEWEL_HP] = 0;

	_id = -1;

	_state = SCENE_FREE;
}

CScene::~CScene()
{
}

void CScene::BuildObjects()
{
	boss_timer_on = false;
	god_timer_on = false;

	cur_mission = MissionType::CS_TURN;
	black_hole_time = 30.f;

	invincible_mode = false;
	// player
	CAirplanePlayer* pAirplanePlayer = new CAirplanePlayer();
	pAirplanePlayer->Reset();
	pAirplanePlayer->SetPosition(XMFLOAT3(0.0f, 0.0f, 100.0f));
	pAirplanePlayer->boundingbox = BoundingOrientedBox{ XMFLOAT3(-0.000000f, -0.000000f, -0.000096f), XMFLOAT3(15.5f, 15.5f, 3.90426f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pSpaceship = pAirplanePlayer;

	_plist.fill(-1);

	for (int i = 0; i < 3; ++i) {
		m_ppPlayers[i] = new CTerrainPlayer();
		m_ppPlayers[i]->Reset();
		m_ppPlayers[i]->SetPosition(XMFLOAT3(425.0f + 10.0f * i, 10.0f, 740.0f));
		m_ppPlayers[i]->cutscene_end = false;
	}

	// meteo
	for (int i = 0; i < METEOS; ++i) {
		m_ppMeteoObjects[i] = new CMeteoObject();
		XMFLOAT3 p_pos = m_pSpaceship->GetPosition();
		m_ppMeteoObjects[i]->m_xmf4x4ToParent = Matrix4x4::Identity();
		XMFLOAT3 random_pos{ urdPos(dree) , urdPos(dree), urdPos(dree) };
		XMFLOAT3 random_dir{ urdDir(dree) , urdDir(dree), urdDir(dree) };
		if (urdEnemyAI(dree) > 50) { random_pos.x = -random_pos.x;  random_dir.x = -random_dir.x; }
		if (urdEnemyAI(dree) > 50) { random_pos.y = -random_pos.y; random_dir.y = -random_dir.y; }
		if (urdEnemyAI(dree) > 50) { random_pos.z = -random_pos.z; random_dir.z = -random_dir.z; }
		m_ppMeteoObjects[i]->SetPosition(random_pos.x + p_pos.x, random_pos.y + p_pos.y, random_pos.z + p_pos.z);

		if (i < METEOS / 2) {
			m_ppMeteoObjects[i]->SetScale(urdScale(dree), urdScale(dree), urdScale(dree));
			m_ppMeteoObjects[i]->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.188906f, 0.977625f, 0.315519f }, XMFLOAT3{ 1.402216f, 1.458820f, 1.499708f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
		}
		else {
			m_ppMeteoObjects[i]->SetScale(urdScale2(dree), urdScale2(dree), urdScale2(dree));
			m_ppMeteoObjects[i]->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.188906f, 0.977625f, 0.315519f }, XMFLOAT3{ 1.402216f, 1.458820f, 1.499708f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
		}
		m_ppMeteoObjects[i]->SetMovingDirection(random_dir);
		m_ppMeteoObjects[i]->SetMovingSpeed(urdSpeed(dree));
	}

	// enemy
	for (int i = 0; i < PLASMACANNON_ENEMY; ++i) {
		m_ppEnemies[i] = new CPlasmaCannonEnemy;
		m_ppEnemies[i]->id = i;
		m_ppEnemies[i]->scene_num = num;
	}
	for (int i = PLASMACANNON_ENEMY; i < PLASMACANNON_ENEMY + LASER_ENEMY; ++i) {
		m_ppEnemies[i] = new CLaserEnemy;
		m_ppEnemies[i]->id = i;
		m_ppEnemies[i]->scene_num = num;
	}
	for (int i = PLASMACANNON_ENEMY + LASER_ENEMY; i < ENEMIES; ++i) {
		m_ppEnemies[i] = new CMissileEnemy;
		m_ppEnemies[i]->id = i;
		m_ppEnemies[i]->scene_num = num;
	}

	// misile
	for (int i = 0; i < ENEMY_BULLETS; ++i) {
		m_ppMissiles[i] = new CMissile;
	}

	can_sit[0] = true;
	can_sit[1] = true;
	can_sit[2] = true;
	can_sit[3] = true;

	// boss
	m_pBoss = new Boss();
	m_pBoss->scene_num = num;
	//m_pBoss->SetPosition(3000.f, 3000.f, 3000.f);

	// god
	m_pGod = new God();
	m_pGod->scene_num = num;
}

void CScene::ReleaseObjects()
{
	if (m_pSpaceship) { delete m_pSpaceship; }
	for (int i = 0; i < 3; ++i)
	{
		if (m_ppPlayers[i]) { delete m_ppPlayers[i]; }
	}

	for (int i = 0; i < m_ppEnemies.size(); ++i)
	{
		if (m_ppEnemies[i]) { delete m_ppEnemies[i]; }
	}

	for (int i = 0; i < m_ppMeteoObjects.size(); ++i)
	{
		if (m_ppMeteoObjects[i]) { delete m_ppMeteoObjects[i]; }
	}

	for (int i = 0; i < m_ppMissiles.size(); ++i)
	{
		if (m_ppMissiles[i]) { delete m_ppMissiles[i]; }
	}
	if (m_pBoss) { delete m_pBoss; }
	if (m_pGod) { delete m_pGod; }

}

void CScene::ResetScene()
{
	_s_lock.lock();
	if (_state == SCENE_INGAME) {
		_state = SCENE_RESET;
		_s_lock.unlock();
		Reset();
		TIMER_EVENT ev{ 0, chrono::system_clock::now() + 60s, EV_RESET_SCENE, num };
		timer_queue.push(ev);
		return;
	}
	_s_lock.unlock();
}

void CScene::Reset()
{
	_id = -1;
	m_pSpaceship->Reset();
	m_pSpaceship->SetPosition(XMFLOAT3(0.f, 0.f, 100.f));

	black_hole_time = 30.f;
	invincible_mode = false;

	heal_player = -1;

	items[ItemType::JEWEL_ATT] = 0;
	items[ItemType::JEWEL_DEF] = 0;
	items[ItemType::JEWEL_HEAL] = 0;
	items[ItemType::JEWEL_HP] = 0;

	for (auto pl : _plist) {
		if (pl == -1) { continue; }
		clients[pl]._s_lock.lock();
		if (clients[pl]._state == ST_INGAME) {
			clients[pl]._state = ST_ALLOC;
			clients[pl].room_id = -1;
			clients[pl].room_pid = -1;
		}
		clients[pl]._s_lock.unlock();
		pl = -1;
	}

	cur_mission = MissionType::CS_TURN;
	prev_mission = MissionType::CS_TURN;
	
	for (char i = 0; i < 3; ++i) {
		m_ppPlayers[i]->Reset();
		m_ppPlayers[i]->SetPosition(XMFLOAT3(425.0f + 10.0f * i, 10.0f, 740.0f));
		m_ppPlayers[i]->cutscene_end = false;
	}

	for (int i = 0; i < METEOS; ++i) {
		XMFLOAT3 p_pos = m_pSpaceship->GetPosition();
		m_ppMeteoObjects[i]->m_xmf4x4ToParent = Matrix4x4::Identity();
		m_ppMeteoObjects[i]->m_xmf4x4World = Matrix4x4::Identity();
		XMFLOAT3 random_pos{ urdPos(dree) , urdPos(dree), urdPos(dree) };
		XMFLOAT3 random_dir{ urdDir(dree) , urdDir(dree), urdDir(dree) };
		if (urdEnemyAI(dree) > 50) { random_pos.x = -random_pos.x;  random_dir.x = -random_dir.x; }
		if (urdEnemyAI(dree) > 50) { random_pos.y = -random_pos.y; random_dir.y = -random_dir.y; }
		if (urdEnemyAI(dree) > 50) { random_pos.z = -random_pos.z; random_dir.z = -random_dir.z; }
		m_ppMeteoObjects[i]->SetPosition(random_pos.x + p_pos.x, random_pos.y + p_pos.y, random_pos.z + p_pos.z);
		m_ppMeteoObjects[i]->SetMovingDirection(random_dir);
		m_ppMeteoObjects[i]->SetMovingSpeed(urdSpeed(dree));
	}

	for (int i = 0; i < ENEMIES; ++i) {
		m_ppEnemies[i]->Reset();
	//	m_ppEnemies[i]->SetisAlive(false);
	}

	for (int i = 0; i < ENEMY_BULLETS; ++i) {
		m_ppMissiles[i]->Reset();
	}

	kill_monster_num = 0;
	heal_player = -1;
	can_sit[0] = true;
	can_sit[1] = true;
	can_sit[2] = true;
	can_sit[3] = true;

	m_pBoss->Reset();
	m_pBoss->SetPosition(3000.f, 3000.f, 3000.f);
	m_pGod->Reset();
	m_pGod->SetPosition(-3000.f, -3000.f, -3000.f);

}

void CScene::CheckEnemyByBulletCollisions(BULLET_INFO& data)
{
	float dist = 500.f; // 플레이어 사거리

	XMVECTOR pos = XMLoadFloat3(&data.pos);
	XMVECTOR dir = XMLoadFloat3(&data.direction);

	float dist_from_enemy = 1000.f;
	char num = -1;

	for (char i = 0; i < ENEMIES; ++i)
	{
		if (!m_ppEnemies[i]->GetisAlive()) { continue; }
		BoundingOrientedBox enemy_bbox = m_ppEnemies[i]->UpdateBoundingBox();

		if (enemy_bbox.Intersects(pos, dir, dist)) //총알/적 충돌시
		{
			float d_f_e = Vector3::Length(Vector3::Subtract(data.pos, m_ppEnemies[i]->GetPosition()));
			if (d_f_e < dist_from_enemy) {
				num = i;
				dist_from_enemy = d_f_e;
			}
		}
	}

	if (num != -1) {
		m_ppEnemies[num]->hp -= m_pSpaceship->damage;

		for (short pl_id : _plist) {
			if (pl_id == -1) continue;
			if (clients[pl_id]._state != ST_INGAME) continue;
			clients[pl_id].send_bullet_hit_packet(m_ppEnemies[num]->GetID(), m_ppEnemies[num]->hp);
		}


		if (m_ppEnemies[num]->hp <= 0) {
			m_ppEnemies[num]->SetisAliveFalse();

			// 미션
			if (cur_mission == MissionType::TU_KILL)
			{
				SetMission(MissionType::TU_HILL);
				//MissionClear();
			}

			// 미션
			else if (cur_mission == MissionType::Kill_MONSTER)
			{
				++kill_monster_num;
				for (short pl_id : _plist) {
					if (pl_id == -1) continue;
					if (clients[pl_id]._state != ST_INGAME) continue;
					clients[pl_id].send_kill_num_packet(kill_monster_num);
				}

				if (kill_monster_num >= 15) {
					kill_monster_num = 0;
					SetMission(MissionType::CS_SHOW_PLANET);
					//MissionClear();
				}
			}

			// 미션
			else if (cur_mission == MissionType::KILL_MONSTER_ONE_MORE_TIME)
			{
				++kill_monster_num;
				for (short pl_id : _plist) {
					if (pl_id == -1) continue;
					if (clients[pl_id]._state != ST_INGAME) continue;
					clients[pl_id].send_kill_num_packet(kill_monster_num);;
				}

				if (kill_monster_num >= 20) {
					kill_monster_num = 0;
					SetMissionFindBoss();
				}
			}

			else if (cur_mission == MissionType::KILL_MONSTER3)
			{
				++kill_monster_num;
				for (short pl_id : _plist) {
					if (pl_id == -1) continue;
					if (clients[pl_id]._state != ST_INGAME) continue;
					clients[pl_id].send_kill_num_packet(kill_monster_num);;
				}

				if (kill_monster_num >= 20) {
					kill_monster_num = 0;
					SetMission(MissionType::KILL_METEOR);
				}
			}

			GetJewels();
		}
	}

	if (cur_mission == MissionType::DEFEAT_BOSS || cur_mission == MissionType::DEFEAT_BOSS2) {
		BoundingOrientedBox boss_bbox = m_pBoss->UpdateBoundingBox();
		if (boss_bbox.Intersects(pos, dir, dist)) // 보스 충돌처리
		{
			m_pBoss->BossHP -= m_pSpaceship->damage;
			for (short pl_id : _plist) {
				if (pl_id == -1) continue;
				if (clients[pl_id]._state != ST_INGAME) continue;
				clients[pl_id].send_bullet_hit_packet(BOSS_ID, m_pBoss->BossHP);
			}
			return;
		}
	}

	if (cur_mission == MissionType::KILL_GOD || cur_mission == MissionType::KILL_GOD2) {
		BoundingOrientedBox god_bbox = m_pGod->UpdateBoundingBox();
		if (god_bbox.Intersects(pos, dir, dist)) // 갓 충돌처리
		{
			m_pGod->GodHP -= m_pSpaceship->damage;
			for (short pl_id : _plist) {
				if (pl_id == -1) continue;
				if (clients[pl_id]._state != ST_INGAME) continue;
				clients[pl_id].send_bullet_hit_packet(GOD_ID, m_pGod->GodHP);
			}
			return;
		}
	}
}

void CScene::CheckMeteoByBulletCollisions(BULLET_INFO& data)
{
	float dist = 1000.f; // 플레이어 사거리
	XMVECTOR pos = XMLoadFloat3(&data.pos);
	XMVECTOR dir = XMLoadFloat3(&data.direction);

	for (char i = 0; i < METEOS; ++i)
	{
		BoundingOrientedBox meteor_bbox = m_ppMeteoObjects[i]->UpdateBoundingBox();
		if (meteor_bbox.Intersects(pos, dir, dist)) //총알/적 충돌시
		{
			SpawnMeteo(i);

			if (cur_mission == MissionType::KILL_METEOR) {
				++kill_monster_num;
				for (short pl_id : _plist) {
					if (pl_id == -1) continue;
					if (clients[pl_id]._state != ST_INGAME) continue;
					clients[pl_id].send_kill_num_packet(kill_monster_num);;
				}

				if (kill_monster_num == 5) {
					kill_monster_num = 0;
					SetMission(MissionType::CS_SHOW_BLACK_HOLE);
				}
			}
			return;
		}
	}
}

void CScene::CheckMissionComplete()
{
	switch (cur_mission) {
	case MissionType::GO_PLANET: {
		XMFLOAT3 player_pos = m_pSpaceship->GetPosition();
		XMFLOAT3 planet_pos{ 10000.f, 10000.f, 10000.f }; // 임시 좌표

		float dist = Vector3::Length(Vector3::Subtract(player_pos, planet_pos));
		if (dist < 1000.f) {
			SetMission(MissionType::KILL_MONSTER_ONE_MORE_TIME);
		}
		return;
	}
	case MissionType::FIND_BOSS: {
		float dist = Vector3::Length(Vector3::Subtract(m_pSpaceship->GetPosition(), m_pBoss->GetPosition()));		// 임시 좌표
		if (dist < 1500.0f) {
			SetMission(MissionType::CS_BOSS_SCREAM);
		}
		return;
	}
	case MissionType::GO_CENTER_REAL: {
		XMFLOAT3 player_pos = m_pSpaceship->GetPosition();
		XMFLOAT3 planet_pos{ 0.f, 0.f, 0.f }; // 임시 좌표

		float dist = Vector3::Length(Vector3::Subtract(player_pos, planet_pos));
		if (dist < 1000.f) {
			SetMission(MissionType::CS_SHOW_GOD);
		}
		return;
	}
	}
	
}

void CScene::MissionClear()
{
	if (cur_mission != levels[cur_mission].NextMission) 
	{
		cur_mission = levels[cur_mission].NextMission;

		if (levels[cur_mission].cutscene) {
			for (char i = 0; i < 3; ++i) {
				if (_plist[i] == -1) { continue; }
				m_ppPlayers[i]->cutscene_end = false;
			}
			TIMER_EVENT ev{ static_cast<char>(levels[cur_mission].NextMission), chrono::system_clock::now() + 100ms, EV_CHECK_CUTSCENE_END, static_cast<short>(num) };
			timer_queue.push(ev);

			if (cur_mission == MissionType::CS_SHOW_GOD) {
				m_pGod->SetPosition(1300.f, 0.f, 0.f);
				m_pGod->GodHP = 100;
				m_pSpaceship->SetPosition(XMFLOAT3(1300.f, 0.f, -700.f));

				if (!god_timer_on) {
					god_timer_on = true;
					TIMER_EVENT ev{ 0, chrono::system_clock::now() + 33ms, EV_UPDATE_GOD, static_cast<short>(num) };
					timer_queue.push(ev);
				}
			}
			else if (cur_mission == MissionType::CS_BOSS_SCREAM) {
				
				m_pBoss->SetPosition(2300.f, 0.f, 0.f);
				m_pBoss->BossHP = 100;
				m_pSpaceship->SetPosition(XMFLOAT3(2300.f, 0.f, -1300.f));
				
				if (!boss_timer_on) {
					boss_timer_on = true;
					TIMER_EVENT ev{ 0, chrono::system_clock::now() + 33ms, EV_UPDATE_BOSS, static_cast<short>(num) };
					timer_queue.push(ev);
				}
			}
		}
		else if (cur_mission == MissionType::FIND_BOSS) {
			if (!boss_timer_on) {
				boss_timer_on = true;
				TIMER_EVENT ev{ 0, chrono::system_clock::now() + 33ms, EV_UPDATE_BOSS, static_cast<short>(num) };
				timer_queue.push(ev);
			}
		}
		else if (cur_mission == MissionType::GO_CENTER) {
			TIMER_EVENT ev{ 0, chrono::system_clock::now() + 20s, EV_MISSION_CLEAR, static_cast<short>(num) };
			timer_queue.push(ev);
		}
		else if (cur_mission == MissionType::ESCAPE_BLACK_HOLE) {
			black_hole_pos = Vector3::Add(m_pSpaceship->GetPosition(), m_pSpaceship->GetLook(), -200.f);

			SC_BLACK_HOLE_PACKET p{};
			p.size = sizeof(SC_BLACK_HOLE_PACKET);
			p.type = SC_BLACK_HOLE;
			p.pos = black_hole_pos;
			Send((char*) & p);

			b_prev_time = chrono::steady_clock::now();
			TIMER_EVENT ev{ 0, chrono::system_clock::now() + 33ms, EV_BLACK_HOLE, static_cast<short>(num) };
			timer_queue.push(ev);
		}

		for (short pl_id : _plist) {
			if (pl_id == -1) continue;
			if (clients[pl_id]._state != ST_INGAME) continue;
			clients[pl_id].send_mission_start_packet(cur_mission);
		}
	}
	else {

	}
}

void CScene::SetMission(MissionType mission)
{
	if (cur_mission != mission)
	{
		cur_mission = mission;
		if (levels[mission].cutscene) {
			for (char i = 0; i < 3; ++i) {
				if (_plist[i] == -1) { continue; }
				m_ppPlayers[i]->cutscene_end = false;
			}
			TIMER_EVENT ev{ static_cast<char>(levels[mission].NextMission), chrono::system_clock::now() + 100ms, EV_CHECK_CUTSCENE_END, static_cast<short>(num) };
			timer_queue.push(ev);

			if (cur_mission == MissionType::CS_SHOW_GOD) {
				m_pGod->SetPosition(1300.f, 0.f, 0.f);
				m_pGod->GodHP = 100;
				m_pSpaceship->SetPosition(XMFLOAT3(1300.f, 0.f, -700.f));

				if (!god_timer_on) {
					god_timer_on = true;
					TIMER_EVENT ev{ 0, chrono::system_clock::now() + 33ms, EV_UPDATE_GOD, static_cast<short>(num) };
					timer_queue.push(ev);
				}
			}
			else if (cur_mission == MissionType::CS_BOSS_SCREAM) {
				m_pBoss->SetPosition(2300.f, 0.f, 0.f);
				m_pBoss->BossHP = 100;
				m_pSpaceship->SetPosition(XMFLOAT3(2300.f, 0.f, -1300.f));
				
				if (!boss_timer_on) {
					boss_timer_on = true;
					TIMER_EVENT ev{ 0, chrono::system_clock::now() + 33ms, EV_UPDATE_BOSS, static_cast<short>(num) };
					timer_queue.push(ev);
				}
			}
		}
		else if (cur_mission == MissionType::FIND_BOSS) {

			if (!boss_timer_on) {
				boss_timer_on = true;
				TIMER_EVENT ev{ 0, chrono::system_clock::now() + 33ms, EV_UPDATE_BOSS, static_cast<short>(num) };
				timer_queue.push(ev);
			}
		}
		else if (cur_mission == MissionType::GO_CENTER) {
			TIMER_EVENT ev{ 0, chrono::system_clock::now() + 20s, EV_MISSION_CLEAR, static_cast<short>(num) };
			timer_queue.push(ev);
		}
		else if (cur_mission == MissionType::ESCAPE_BLACK_HOLE) {
			black_hole_pos = Vector3::Add(m_pSpaceship->GetPosition(), m_pSpaceship->GetLook(), -200.f);

			SC_BLACK_HOLE_PACKET p{};
			p.size = sizeof(SC_BLACK_HOLE_PACKET);
			p.type = SC_BLACK_HOLE;
			p.pos = black_hole_pos;
			Send((char*)&p);

			b_prev_time = chrono::steady_clock::now();
			TIMER_EVENT ev{ 0, chrono::system_clock::now() + 33ms, EV_BLACK_HOLE, static_cast<short>(num) };
			timer_queue.push(ev);
		}

		for (short pl_id : _plist) {
			if (pl_id == -1) continue;
			if (clients[pl_id]._state != ST_INGAME) continue;
			clients[pl_id].send_mission_start_packet(cur_mission);
		}
	}
}

void CScene::SetMissionFindBoss()
{
	if (cur_mission == MissionType::KILL_MONSTER_ONE_MORE_TIME) {
		boss_timer_on = true;
		cur_mission = MissionType::FIND_BOSS;

		TIMER_EVENT ev{ 0, chrono::system_clock::now() + 33ms, EV_UPDATE_BOSS, num };
		timer_queue.push(ev);

		for (short pl_id : _plist) {
			if (pl_id == -1) continue;
			if (clients[pl_id]._state != ST_INGAME) continue;
			clients[pl_id].send_mission_start_packet(cur_mission);
		}
	}
	
}

void CScene::GetJewels()
{

	short num = urdEnemyAI(dree);
	ItemType item_type;
	if (num < 10) {
		item_type = ItemType::JEWEL_ATT;
		if (items[item_type] < MAX_ITEM) {
			m_pSpaceship->damage = 4 + items[item_type];
		}
		else { return; }
	}
	else if (num < 20) {
		item_type = ItemType::JEWEL_DEF;
		if (items[item_type] < MAX_ITEM) {
			m_pSpaceship->def = 1 + items[item_type];
		}
		else { return; }
	}
	else if (num < 30) {
		item_type = ItemType::JEWEL_HEAL;
		if (items[item_type] < MAX_ITEM) {
			m_pSpaceship->heal = 11 + items[item_type];
		}
		else { return; }
	}
	else if (num < 40) {
		item_type = ItemType::JEWEL_HP;
		if (items[item_type] < MAX_ITEM) {
			m_pSpaceship->max_hp = 110 + 10 * items[item_type];
		}
		else { return; }
	}
	else {
		return;
	}

	++items[item_type];
	ITEM_INFO info{};
	info.type = item_type;
	info.num = items[item_type];

	for (short pl_id : _plist) {
		if (pl_id == -1) continue;
		if (clients[pl_id]._state != ST_INGAME) continue;
		clients[pl_id].send_item_packet(info);
	}

	// 미션
	if (cur_mission == MissionType::GET_JEWELS)
	{
		SetMission(MissionType::Kill_MONSTER);
	}
}

void CScene::SpawnEnemy()
{
	if (_state != ST_INGAME) { return; }
	if (levels[cur_mission].cutscene) {
		TIMER_EVENT ev{ 0, chrono::system_clock::now() + 20s, EV_SPAWN_ENEMY, static_cast<short>(num) };
		timer_queue.push(ev);
		return;
	}
	char cur_enemy_num = std::count_if(m_ppEnemies.begin(), m_ppEnemies.end(), [](CEnemy* enemy) {
		return enemy->GetisAlive();
	});

	std::array<CEnemy*, ENEMIES> ppEnemies{ m_ppEnemies };
	std::random_shuffle(ppEnemies.begin(), ppEnemies.end());

	char spawn_num = levels[cur_mission].SpawnMonsterNum;

	for (int i = 0; i < ENEMIES; ++i)
	{
		if (levels[cur_mission].MaxMonsterNum <= cur_enemy_num) { break; }
		if (spawn_num <= 0) { break; }
		if (!ppEnemies[i]->GetisAlive()) {
			SpawnEnemy(ppEnemies[i]->GetID());
			m_ppEnemies[ppEnemies[i]->GetID()]->prev_time = chrono::steady_clock::now();
			TIMER_EVENT ev_u{ ppEnemies[i]->GetID(), chrono::system_clock::now() + 33ms, EV_MOVE_ENEMY, static_cast<short>(num) };
			timer_queue.push(ev_u);
			++cur_enemy_num;
			--spawn_num;
		}
	}
	TIMER_EVENT ev{ 0, chrono::system_clock::now() + 20s, EV_SPAWN_ENEMY, static_cast<short>(num) };
	timer_queue.push(ev);
}

void CScene::MoveEnemy(char obj_id)
{
	if (_state != ST_INGAME) { return; }
	if (m_ppEnemies[obj_id]->hp <= 0) {
		m_ppEnemies[obj_id]->SetisAliveFalse();
		return;
	}
	if (levels[cur_mission].cutscene) {
		if (cur_mission == MissionType::CS_BAD_ENDING || 
			cur_mission == MissionType::CS_BOSS_SCREAM || cur_mission == MissionType::CS_SHOW_GOD) {
			m_ppEnemies[obj_id]->SetisAliveFalse();
			for (auto pl : _plist) {
				if (pl == -1) { continue; }
				clients[pl].send_bullet_hit_packet(obj_id, -1);
			}
			return;
		}
		m_ppEnemies[obj_id]->prev_time = chrono::steady_clock::now();
		TIMER_EVENT ev{ obj_id, chrono::system_clock::now() + 1s, EV_MOVE_ENEMY, static_cast<short>(num) };
		timer_queue.push(ev);
		return;
	}

	auto time_now = chrono::steady_clock::now();
	std::chrono::duration<float> elapsed_time = (time_now - m_ppEnemies[obj_id]->prev_time);
	m_ppEnemies[obj_id]->prev_time = time_now;

	m_ppEnemies[obj_id]->MoveAI(elapsed_time.count(), m_pSpaceship);
	//scene->m_ppEnemies[ex_over->obj_id]->UpdateBoundingBox();

	// 운석과 충돌처리
	for (int i = 0; i < METEOS; ++i)
	{
		if (Vector3::Length(Vector3::Subtract(m_ppEnemies[obj_id]->GetPosition(), m_ppMeteoObjects[i]->GetPosition())) < 30.f)
		{
			XMFLOAT3 xmf3Sub = m_ppMeteoObjects[i]->GetPosition();
			xmf3Sub = Vector3::Subtract(m_ppEnemies[obj_id]->GetPosition(), xmf3Sub);
			if (Vector3::Length(xmf3Sub) > 0.0001f) {
				xmf3Sub = Vector3::Normalize(xmf3Sub);
			}
			XMFLOAT3 vel = m_ppEnemies[obj_id]->GetVelocity();
			float fLen = Vector3::Length(vel) / 10.f;
			xmf3Sub = Vector3::ScalarProduct(xmf3Sub, fLen, false);

			m_ppEnemies[obj_id]->SetVelocity(Vector3::Add(vel, xmf3Sub));
		}
	}

	for (int i = 0; i < ENEMIES; ++i)
	{
		if (!m_ppEnemies[i]->GetisAlive()) { continue; }
		if (i == obj_id) { continue; }
		//if (scene->m_ppEnemies[ex_over->obj_id]->HierarchyIntersects(scene->m_ppEnemies[i]))
		if (Vector3::Length(Vector3::Subtract(m_ppEnemies[obj_id]->GetPosition(), m_ppEnemies[i]->GetPosition())) < 30.f)
		{
			XMFLOAT3 xmf3Sub = m_ppEnemies[i]->GetPosition();
			xmf3Sub = Vector3::Subtract(m_ppEnemies[obj_id]->GetPosition(), xmf3Sub);
			if (Vector3::Length(xmf3Sub) > 0.0001f) {
				xmf3Sub = Vector3::Normalize(xmf3Sub);
			}
			XMFLOAT3 vel = m_ppEnemies[obj_id]->GetVelocity();
			float fLen = Vector3::Length(vel) / 10.f;
			xmf3Sub = Vector3::ScalarProduct(xmf3Sub, fLen, false);
			XMFLOAT3 vel2 = m_ppEnemies[i]->GetVelocity();

			m_ppEnemies[obj_id]->SetVelocity(Vector3::Add(vel, xmf3Sub));
			m_ppEnemies[i]->SetVelocity(Vector3::Add(vel2, xmf3Sub, -1.f));
		}
	}

	if ( m_ppEnemies[obj_id]->state == EnemyState::AIMING) {
		if (m_ppEnemies[obj_id]->type == EnemyType::MISSILE && !m_ppEnemies[obj_id]->GetAttackTimer()) {
			m_ppEnemies[obj_id]->SetAttackTimerTrue();
			TIMER_EVENT ev{ obj_id, chrono::system_clock::now() + 10s, EV_SPAWN_MISSILE, static_cast<short>(num) };
			timer_queue.push(ev);
		}
		for (short pl_id : _plist) {
			if (pl_id == -1) continue;
			if (clients[pl_id]._state != ST_INGAME) continue;
			clients[pl_id].send_enemy_state_packet(obj_id, EnemyState::AIMING);
		}

		TIMER_EVENT ev{ obj_id, chrono::system_clock::now() + 10ms, EV_AIMING_ENEMY, static_cast<short>(num) };
		timer_queue.push(ev);
		return;
	}

	TIMER_EVENT ev{ obj_id, chrono::system_clock::now() + 10ms, EV_MOVE_ENEMY, static_cast<short>(num) };
	timer_queue.push(ev);
}

void CScene::AimingEnemy(char obj_id)
{
	if (_state != ST_INGAME) { return; }
	if (m_ppEnemies[obj_id]->hp <= 0) {
		m_ppEnemies[obj_id]->SetisAliveFalse();
		return;
	}
	if (levels[cur_mission].cutscene) {
		if (cur_mission == MissionType::CS_BAD_ENDING ||
			cur_mission == MissionType::CS_BOSS_SCREAM || cur_mission == MissionType::CS_SHOW_GOD) {
			m_ppEnemies[obj_id]->SetisAliveFalse();
			for (auto pl : _plist) {
				if (pl == -1) { continue; }
				clients[pl].send_bullet_hit_packet(obj_id, -1);
			}
			return;
		}
		m_ppEnemies[obj_id]->prev_time = chrono::steady_clock::now();
		TIMER_EVENT ev{ obj_id, chrono::system_clock::now() + 1s, EV_MOVE_ENEMY, static_cast<short>(num) };
		timer_queue.push(ev);
		return;
	}

	auto time_now = chrono::steady_clock::now();
	std::chrono::duration<float> elapsed_time = (time_now - m_ppEnemies[obj_id]->prev_time);
	m_ppEnemies[obj_id]->prev_time = time_now;

	m_ppEnemies[obj_id]->AimingAI(elapsed_time.count(), m_pSpaceship);
	//scene->m_ppEnemies[ex_over->obj_id]->UpdateBoundingBox();

	// 운석과 충돌처리
	for (int i = 0; i < METEOS; ++i)
	{
		if (Vector3::Length(Vector3::Subtract(m_ppEnemies[obj_id]->GetPosition(), m_ppMeteoObjects[i]->GetPosition())) < 30.f)
		{
			XMFLOAT3 xmf3Sub = m_ppMeteoObjects[i]->GetPosition();
			xmf3Sub = Vector3::Subtract(m_ppEnemies[obj_id]->GetPosition(), xmf3Sub);
			if (Vector3::Length(xmf3Sub) > 0.0001f) {
				xmf3Sub = Vector3::Normalize(xmf3Sub);
			}
			XMFLOAT3 vel = m_ppEnemies[obj_id]->GetVelocity();
			float fLen = Vector3::Length(vel) / 10.f;
			xmf3Sub = Vector3::ScalarProduct(xmf3Sub, fLen, false);

			m_ppEnemies[obj_id]->SetVelocity(Vector3::Add(vel, xmf3Sub));
		}
	}

	for (int i = 0; i < ENEMIES; ++i)
	{
		if (!m_ppEnemies[i]->GetisAlive()) { continue; }
		if (i == obj_id) { continue; }
		//if (scene->m_ppEnemies[ex_over->obj_id]->HierarchyIntersects(scene->m_ppEnemies[i]))
		if (Vector3::Length(Vector3::Subtract(m_ppEnemies[obj_id]->GetPosition(), m_ppEnemies[i]->GetPosition())) < 30.f)
		{
			XMFLOAT3 xmf3Sub = m_ppEnemies[i]->GetPosition();
			xmf3Sub = Vector3::Subtract(m_ppEnemies[obj_id]->GetPosition(), xmf3Sub);
			if (Vector3::Length(xmf3Sub) > 0.0001f) {
				xmf3Sub = Vector3::Normalize(xmf3Sub);
			}
			XMFLOAT3 vel = m_ppEnemies[obj_id]->GetVelocity();
			float fLen = Vector3::Length(vel) / 10.f;
			xmf3Sub = Vector3::ScalarProduct(xmf3Sub, fLen, false);
			XMFLOAT3 vel2 = m_ppEnemies[i]->GetVelocity();

			m_ppEnemies[obj_id]->SetVelocity(Vector3::Add(vel, xmf3Sub));
			m_ppEnemies[i]->SetVelocity(Vector3::Add(vel2, xmf3Sub, -1.f));
		}
	}

	if (m_ppEnemies[obj_id]->state == EnemyState::MOVE)
	{
		for (short pl_id : _plist) {
			if (pl_id == -1) continue;
			if (clients[pl_id]._state != ST_INGAME) continue;
			clients[pl_id].send_enemy_state_packet(obj_id, EnemyState::MOVE);
		}

		TIMER_EVENT ev{ obj_id, chrono::system_clock::now() + 10ms, EV_MOVE_ENEMY, static_cast<short>(num) };
		timer_queue.push(ev);
		return;
	}

	TIMER_EVENT ev{ obj_id, chrono::system_clock::now() + 10ms, EV_AIMING_ENEMY, static_cast<short>(num) };
	timer_queue.push(ev);

}

void CScene::UpdateMeteo(char obj_id)
{
	if (_state != ST_INGAME) { return; }
	if (levels[cur_mission].cutscene) {
		m_ppMeteoObjects[obj_id]->prev_time = chrono::steady_clock::now();
		TIMER_EVENT ev{ obj_id, chrono::system_clock::now() + 1s, EV_UPDATE_METEO, static_cast<short>(num) };
		timer_queue.push(ev);
		return;
	}

	auto time_now = chrono::steady_clock::now();
	std::chrono::duration<float> elapsed_time = (time_now - m_ppMeteoObjects[obj_id]->prev_time);
	m_ppMeteoObjects[obj_id]->prev_time = time_now;
	m_ppMeteoObjects[obj_id]->Animate(elapsed_time.count());

	XMFLOAT3 p_pos = m_pSpaceship->GetPosition();
	XMFLOAT3 m_pos = m_ppMeteoObjects[obj_id]->GetPosition();
	float dist = Vector3::Length(Vector3::Subtract(m_pos, p_pos));
	if (dist > 1200.0f) {
		SpawnMeteo(obj_id);
	}
	BoundingOrientedBox meteor_bbox = m_ppMeteoObjects[obj_id]->UpdateBoundingBox();
	BoundingOrientedBox spaceship_bbox = m_pSpaceship->UpdateBoundingBox();

	if (spaceship_bbox.Intersects(meteor_bbox))
	{
		XMFLOAT3 xmf3Sub = m_pSpaceship->GetPosition();
		xmf3Sub = Vector3::Subtract( xmf3Sub, m_ppMeteoObjects[obj_id]->GetPosition());
		if (Vector3::Length(xmf3Sub) > 0.0001f) {
			xmf3Sub = Vector3::Normalize(xmf3Sub);
		}
		XMFLOAT3 vel = m_pSpaceship->GetVelocity();
		float fLen = (Vector3::Length(vel) + 10.f) * 2;
		xmf3Sub = Vector3::ScalarProduct(xmf3Sub, fLen, false);

		m_pSpaceship->SetVelocity(Vector3::Add(vel, xmf3Sub));
		m_pSpaceship->SetHP(m_pSpaceship->GetHP() - 5);

		m_ppMeteoObjects[obj_id]->SetMovingDirection(Vector3::ScalarProduct(xmf3Sub, -1.f, false));
		m_ppMeteoObjects[obj_id]->SetMovingSpeed(fLen);

		for (short pl_id : _plist) {
			if (pl_id == -1) continue;
			if (clients[pl_id]._state != ST_INGAME) continue;
			clients[pl_id].send_bullet_hit_packet(-1, m_pSpaceship->GetHP());

		}
	}

	TIMER_EVENT ev{ obj_id, chrono::system_clock::now() + 10ms, EV_UPDATE_METEO, static_cast<short>(num) };
	timer_queue.push(ev);
}

void CScene::SpawnMissile(char obj_id)
{
	if (_state != ST_INGAME) { return; }
	if (!m_ppEnemies[obj_id]->GetisAlive()) {
		m_ppEnemies[obj_id]->SetAttackTimerFalse();
		return;
	}
	if (levels[cur_mission].cutscene) {
		if (cur_mission == MissionType::CS_BAD_ENDING) {
			m_ppEnemies[obj_id]->SetAttackTimerFalse();
			return;
		}
		TIMER_EVENT ev{ 0, chrono::system_clock::now() + 10s, EV_SPAWN_MISSILE, static_cast<short>(num) };
		timer_queue.push(ev);
		return;
	}

	MissileInfo info{};
	info.StartPos = m_ppEnemies[obj_id]->GetPosition();
	info.Quaternion = m_ppEnemies[obj_id]->GetQuaternion();
	info.damage = levels[ cur_mission].Missile.ATK;

	// 미사일 생성, 미사일 타이머 push
	for (char i = 0; i < ENEMY_BULLETS; ++i) {
		if (!m_ppMissiles[i]->GetisActive()) {
			m_ppMissiles[i]->SetNewMissile(info);

			SPAWN_MISSILE_INFO m_info{};
			m_info.id = i;
			m_info.pos = info.StartPos;
			m_info.Quaternion = info.Quaternion;

			for (short pl_id : _plist) {
				if (pl_id == -1) continue;
				if (clients[pl_id]._state != ST_INGAME) continue;
				clients[pl_id].send_spawn_missile_packet(m_info);
			}
			m_ppMissiles[i]->prev_time = chrono::steady_clock::now();
			TIMER_EVENT ev{ i, chrono::system_clock::now() + 30ms, EV_UPDATE_MISSILE, static_cast<short>(num) };
			timer_queue.push(ev);

			break;
		}
	}

	TIMER_EVENT ev{ obj_id, chrono::system_clock::now() + 10s, EV_SPAWN_MISSILE, static_cast<short>(num) };
	timer_queue.push(ev);
}

void CScene::UpdateMissile(char obj_id)
{
	if (_state != ST_INGAME) { return; }
	if (!m_ppMissiles[obj_id]->GetisActive()) { return; }
	if (levels[cur_mission].cutscene) {
		if (cur_mission == MissionType::CS_BAD_ENDING) {
			m_ppMissiles[obj_id]->SetisActive(false);
			return;
		}
		m_ppMissiles[obj_id]->prev_time = chrono::steady_clock::now();
		TIMER_EVENT ev{ obj_id, chrono::system_clock::now() + 1s, EV_UPDATE_MISSILE, static_cast<short>(num) };
		timer_queue.push(ev);
		return;
	}

	auto time_now = chrono::steady_clock::now();
	std::chrono::duration<float> elapsed_time = (time_now - m_ppMissiles[obj_id]->prev_time);
	m_ppMissiles[obj_id]->prev_time = time_now;
	m_ppMissiles[obj_id]->Animate(elapsed_time.count(), m_pSpaceship);

	BoundingOrientedBox missile_bbox = m_ppMissiles[obj_id]->UpdateBoundingBox();
	BoundingOrientedBox spaceship_bbox = m_pSpaceship->UpdateBoundingBox();

	if (spaceship_bbox.Intersects(missile_bbox))
	{
		m_ppMissiles[obj_id]->SetisActive(false);
		// 충돌처리

		if (m_pSpaceship->GetHP() > 0) {
			m_pSpaceship->GetAttack(m_ppMissiles[obj_id]->GetDamage());
		}

		for (short pl_id : _plist) {
			if (pl_id == -1) continue;
			if (clients[pl_id]._state != ST_INGAME) continue;
			clients[pl_id].send_bullet_hit_packet(-1, m_pSpaceship->GetHP());
		}
	}

	if (m_ppMissiles[obj_id]->GetisActive()) {
		TIMER_EVENT ev{ obj_id, chrono::system_clock::now() + 10ms, EV_UPDATE_MISSILE, static_cast<short>(num) };
		timer_queue.push(ev);
	}
	else {
		for (short pl_id : _plist) {
			if (pl_id == -1) continue;
			if (clients[pl_id]._state != ST_INGAME) continue;
			clients[pl_id].send_remove_missile_packet(obj_id);
		}
	}
}

void CScene::UpdateBoss()
{
	if (_state != ST_INGAME) { boss_timer_on = false;  return; }
	if (cur_mission != MissionType::DEFEAT_BOSS && cur_mission != MissionType::CS_BOSS_SCREAM && cur_mission != MissionType::CS_ANGRY_BOSS 
		&& cur_mission != MissionType::FIND_BOSS && cur_mission != MissionType::DEFEAT_BOSS2 && cur_mission != MissionType::CS_BAD_ENDING) { boss_timer_on = false; return; }
	if (m_pBoss->BossHP <= 0) {
		boss_timer_on = false;
		SetMission(MissionType::CS_SHOW_STARGIANT);
		return;
	}
	if (levels[cur_mission].cutscene) {
		TIMER_EVENT ev{ 0, chrono::system_clock::now() + 1s, EV_UPDATE_BOSS, static_cast<short>(num) };
		timer_queue.push(ev);
		return;
	}

	m_pBoss->Boss_Ai(0.025f, m_pSpaceship, m_pBoss->GetHP());

	if (cur_mission == MissionType::DEFEAT_BOSS && m_pBoss->BossHP <= 50) {
		SetMission(MissionType::CS_ANGRY_BOSS);
	}

	float dist;
	dist = Vector3::Length(Vector3::Subtract(m_pSpaceship->GetPosition(), m_pBoss->GetPosition()));
	if (dist < 1000.f) // boss 막기
	{
		XMFLOAT3 ToGo = Vector3::Subtract(m_pSpaceship->GetPosition(), m_pBoss->GetPosition());
		ToGo = Vector3::ScalarProduct(ToGo, 1000.f);
		ToGo = Vector3::Add(m_pBoss->GetPosition(), ToGo);
		m_pSpaceship->SetPosition(ToGo);
	}

	TIMER_EVENT ev{ 0, chrono::system_clock::now() + 25ms, EV_UPDATE_BOSS, static_cast<short>(num) };
	timer_queue.push(ev);
}

void CScene::UpdateGod()
{
	if (_state != ST_INGAME) { god_timer_on = false; return; }
	if (m_pGod->GetcurHp() <= 0) {
		god_timer_on = false;
		SetMission(MissionType::CS_ENDING);
		return;
	}
	if (cur_mission != MissionType::KILL_GOD && cur_mission != MissionType::CS_SHOW_GOD && cur_mission != MissionType::CS_ANGRY_GOD
		&& cur_mission != MissionType::KILL_GOD2 && cur_mission != MissionType::CS_BAD_ENDING) { god_timer_on = false; return; }
	if (levels[cur_mission].cutscene) {
		TIMER_EVENT ev{ 0, chrono::system_clock::now() + 1s, EV_UPDATE_GOD, static_cast<short>(num) };
		timer_queue.push(ev);
		return;
	}

	if (m_pGod->God_Ai(0.025f, m_pSpaceship, m_pGod->GetcurHp())) {
		SpawnEnemyFromGod();
	}

	if (cur_mission == MissionType::KILL_GOD && m_pGod->GodHP <= 50) {
		SetMission(MissionType::CS_ANGRY_GOD);
	}

	float dist;
	dist = Vector3::Length(Vector3::Subtract(m_pSpaceship->GetPosition(), m_pGod->GetPosition()));
	if (dist < 500.f) // boss 막기
	{
		XMFLOAT3 ToGo = Vector3::Subtract(m_pSpaceship->GetPosition(), m_pGod->GetPosition());
		ToGo = Vector3::ScalarProduct(ToGo, 500.f);
		ToGo = Vector3::Add(m_pGod->GetPosition(), ToGo);
		m_pSpaceship->SetPosition(ToGo);
	}

	TIMER_EVENT ev{ 0, chrono::system_clock::now() + 25ms, EV_UPDATE_GOD, static_cast<short>(num) };
	timer_queue.push(ev);
}

void CScene::UpdateSpaceship()
{
	if (_state != ST_INGAME) { return; }
	if (levels[cur_mission].cutscene) {
		TIMER_EVENT ev{ 0, chrono::system_clock::now() + 1s, EV_UPDATE_SPACESHIP, static_cast<short>(num) };
		timer_queue.push(ev);
		return;
	}
	if (m_pSpaceship->hp <= 0 && !invincible_mode) {
		prev_mission = cur_mission;
		SetMission(MissionType::CS_BAD_ENDING);
	}

	auto time_now = chrono::steady_clock::now();
	std::chrono::duration<float> elapsed_time = (time_now - m_pSpaceship->prev_time);
	m_pSpaceship->prev_time = time_now;
	m_pSpaceship->Update(elapsed_time.count());

	CheckMissionComplete();

	TIMER_EVENT ev{ 0, chrono::system_clock::now() + 10ms, EV_UPDATE_SPACESHIP, static_cast<short>(num) };
	timer_queue.push(ev);
}

void CScene::Heal()
{
	if (_state != ST_INGAME) { return; }
	if (levels[cur_mission].cutscene) {
		TIMER_EVENT ev{ 0, chrono::system_clock::now() + 1s, EV_HEAL, static_cast<short>(num) };
		timer_queue.push(ev);
		return;
	}

	if (heal_player != -1) {
		if (m_pSpaceship->GetHeal()) {
			for (short pl_id : _plist) {
				if (pl_id == -1) continue;
				if (clients[pl_id]._state != ST_INGAME) continue;
				clients[pl_id].send_bullet_hit_packet(-1, m_pSpaceship->GetHP());
			}
		}

		TIMER_EVENT ev{ 0, chrono::system_clock::now() + 1s, EV_HEAL, static_cast<short>(num) };
		timer_queue.push(ev);
	}
}

void CScene::SendSceneInfo()
{
	if (_state != ST_INGAME) { return; }
	if (levels[cur_mission].cutscene) {
		TIMER_EVENT ev{ 0, chrono::system_clock::now() + 500ms, EV_SEND_SCENE_INFO, static_cast<short>(num) };
		timer_queue.push(ev);
		return;
	}

	char send_buf[10000]{};
	short send_num = 0;

	SC_ALL_METEOR_PACKET m_packet{};
	m_packet.type = SC_ALL_METEOR;
	m_packet.size = sizeof(SC_ALL_METEOR_PACKET);
	for (char i = 0; i < METEOS; ++i) {
		m_packet.pos[i] = m_ppMeteoObjects[i]->GetPosition();
	}
	memcpy(&send_buf[0], &m_packet, m_packet.size);
	send_num += m_packet.size;

	SC_MOVE_SPACESHIP_PACKET s_packet{};
	s_packet.size = sizeof(s_packet);
	s_packet.type = SC_MOVE_SPACESHIP;
	s_packet.pos = m_pSpaceship->GetPosition();
	memcpy(&send_buf[send_num], &s_packet, s_packet.size);
	send_num += s_packet.size;
	
	for (char i = 0; i < ENEMIES; ++i) {
		if (!m_ppEnemies[i]->GetisAlive() || !m_ppEnemies[i]->GetisMove()) { continue; }
		SC_MOVE_ENEMY_PACKET e_packet{};
		e_packet.size = sizeof(e_packet);
		e_packet.type = SC_MOVE_ENEMY;
		e_packet.data.id = i;
		e_packet.data.pos = m_ppEnemies[i]->GetPosition();
		//e_packet.data.Quaternion = m_ppEnemies[i]->GetQuaternion();
		memcpy(&send_buf[send_num], &e_packet, e_packet.size);
		send_num += e_packet.size;
		m_ppEnemies[i]->SetIsMoveFalse();
	}

	for (char i = 0; i < MISSILES; ++i) {
		if (!m_ppMissiles[i]->GetisActive()) { continue; }
		SC_MISSILE_PACKET m_packet{};
		m_packet.size = sizeof(m_packet);
		m_packet.type = SC_MISSILE;
		m_packet.data.id = i;
		m_packet.data.pos = m_ppMissiles[i]->GetPosition();
		memcpy(&send_buf[send_num], &m_packet, m_packet.size);
		send_num += m_packet.size;
	}

	for (short pl_id : _plist) {
		if (pl_id == -1) continue;
		if (clients[pl_id]._state != ST_INGAME) continue;
		clients[pl_id].do_send(&send_buf, send_num);
	}

	TIMER_EVENT ev{ 0, chrono::system_clock::now() + 20ms, EV_SEND_SCENE_INFO, static_cast<short>(num) };
	timer_queue.push(ev);
}

void CScene::BlackHole()
{
	if (_state != ST_INGAME) { return; }
	if (cur_mission != MissionType::ESCAPE_BLACK_HOLE) { return; }

	auto time_now = chrono::steady_clock::now();
	std::chrono::duration<float> elapsed_time = (time_now - b_prev_time);
	b_prev_time = time_now;
	black_hole_time -= elapsed_time.count();
	if (black_hole_time <= 0.f) { SetMission(MissionType::GO_CENTER_REAL); return; }

	SC_BLACK_HOLE_TIME_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_BLACK_HOLE_TIME;
	packet.time = black_hole_time;
	Send((char*)&packet);

	XMFLOAT3 pos;
	XMFLOAT3 ToBlackHole;
	float dist;
	float speed;
	pos = m_pSpaceship->GetPosition();
	ToBlackHole = Vector3::Subtract(black_hole_pos, pos);
	dist = Vector3::Length(ToBlackHole);
	speed = 90.f - dist * 0.5f;
	if (speed < 50.f) { speed = 50.f; }
	ToBlackHole = Vector3::ScalarProduct(ToBlackHole, speed * elapsed_time.count());
	m_pSpaceship->SetPosition(Vector3::Add(pos, ToBlackHole));
	if (dist < 30.f) {
		m_pSpaceship->GetAttack(1);
		for (short pl_id : _plist) {
			if (pl_id == -1) continue;
			if (clients[pl_id]._state != ST_INGAME) continue;
			clients[pl_id].send_bullet_hit_packet(-1, m_pSpaceship->GetHP());
		}
	}

	for (char i = 0; i < METEOS; ++i) {
		pos = m_ppMeteoObjects[i]->GetPosition();
		dist = Vector3::Length(Vector3::Subtract(pos, black_hole_pos));
		if (dist < 20.f) {
			SpawnMeteo(i);
			continue;
		}
		ToBlackHole = Vector3::Subtract(black_hole_pos, pos);
		dist = Vector3::Length(ToBlackHole);
		speed = 90.f - dist * 0.5f;
		if (speed < 50.f) { speed = 50.f; }
		ToBlackHole = Vector3::ScalarProduct(ToBlackHole, speed * elapsed_time.count());
		m_ppMeteoObjects[i]->SetPosition(Vector3::Add(pos, ToBlackHole));
	}

	for (char i = 0; i < ENEMIES; ++i) {
		if (!m_ppEnemies[i]->GetisAlive()) { continue; }
		pos = m_ppEnemies[i]->GetPosition();
		dist = Vector3::Length(Vector3::Subtract(pos, black_hole_pos));
		if (dist < 50.f) {
			m_ppEnemies[i]->hp -= 3;
			for (short pl_id : _plist) {
				if (pl_id == -1) continue;
				if (clients[pl_id]._state != ST_INGAME) continue;
				clients[pl_id].send_bullet_hit_packet(i, m_ppEnemies[i]->hp);
			}
			continue;
		}
		ToBlackHole = Vector3::Subtract(black_hole_pos, pos);
		dist = Vector3::Length(ToBlackHole);
		speed = 90.f - dist * 0.5f;
		if (speed < 50.f) { speed = 50.f; }
		ToBlackHole = Vector3::ScalarProduct(ToBlackHole, speed * elapsed_time.count());
		m_ppEnemies[i]->SetPosition(Vector3::Add(pos, ToBlackHole));
	}

	TIMER_EVENT ev{ 0, chrono::system_clock::now() + 15ms, EV_BLACK_HOLE, static_cast<short>(num) };
	timer_queue.push(ev);

}

void CScene::CheckCutsceneEnd(MissionType next_mission)
{
	if (_state != ST_INGAME) { return; }
	if (!levels[cur_mission].cutscene) { return; }

	bool cutscene_end = true;
	for (char i = 0; i < 3; ++i) {
		if (_plist[i] == -1) { continue; }
		if (m_ppPlayers[i]->cutscene_end == false) { cutscene_end = false; }
	}

	if (cutscene_end == true) {
		if (cur_mission == MissionType::CS_BAD_ENDING ) {
			m_pSpaceship->SetPosition(levels[prev_mission].RestartPosition);
			for (char i = (char)ItemType::JEWEL_ATT; i < (char)ItemType::JEWEL_HP; ++i) {
				if (items[(ItemType)i] > 0) { 
					--items[(ItemType)i];
					ITEM_INFO i_info{};
					i_info.num = items[(ItemType)i];
					i_info.type = (ItemType)i;
					for (short pl_id : _plist) {
						if (pl_id == -1) continue;
						if (clients[pl_id]._state != ST_INGAME) continue;
						clients[pl_id].send_item_packet(i_info);
					}
				}
			}
			m_pSpaceship->hp = m_pSpaceship->max_hp;
			kill_monster_num = 0;
			for (short pl_id : _plist) {
				if (pl_id == -1) continue;
				if (clients[pl_id]._state != ST_INGAME) continue;
				clients[pl_id].send_bullet_hit_packet(-1, m_pSpaceship->hp);
			}

			SetMission(levels[prev_mission].RestartMission);
		}
		else if (cur_mission == MissionType::CS_ENDING) {
			ResetScene();
		}
		else if (levels[cur_mission].NextMission == next_mission) {
				SetMission(next_mission);
		}
		return;
	}

	TIMER_EVENT ev{ static_cast<char>(next_mission), chrono::system_clock::now() + 500ms, EV_CHECK_CUTSCENE_END, static_cast<short>(num)};
	timer_queue.push(ev);
}

void CScene::SpawnEnemyFromGod()
{
	std::array<CEnemy*, ENEMIES> ppEnemies{ m_ppEnemies };
	std::random_shuffle(ppEnemies.begin(), ppEnemies.end());

	char spawn_num = 3;

	for (int i = 0; i < ENEMIES; ++i)
	{
		if (spawn_num <= 0) { break; }
		if (!ppEnemies[i]->GetisAlive()) {
			SpawnEnemyFromGod(ppEnemies[i]->GetID(), spawn_num);
			m_ppEnemies[ppEnemies[i]->GetID()]->prev_time = chrono::steady_clock::now();
			TIMER_EVENT ev_u{ ppEnemies[i]->GetID(), chrono::system_clock::now() + 33ms, EV_MOVE_ENEMY, static_cast<short>(num) };
			timer_queue.push(ev_u);
			--spawn_num;
		}
	}
}

void CScene::SpawnEnemyFromGod(char id, char enemy_num)
{
	m_ppEnemies[id]->Reset();
	XMFLOAT3 g_pos = m_pGod->GetPosition();
	XMFLOAT3 random_pos{ urdPos(dree), urdPos(dree), urdPos(dree) / 5.f };

	m_ppEnemies[id]->SetStatus(cur_mission);
	switch (enemy_num) {
	case 1:
		m_ppEnemies[id]->SetPosition(g_pos.x + 50.f, g_pos.y, g_pos.z);
		break;
	case 2:
		m_ppEnemies[id]->SetPosition(g_pos.x, g_pos.y + 50.f, g_pos.z);
		break;
	case 3:
		m_ppEnemies[id]->SetPosition(g_pos.x, g_pos.y, g_pos.z + 50.f);
		break;
	}
	m_ppEnemies[id]->state = EnemyState::MOVE;
	m_ppEnemies[id]->SetDestination();
	m_ppEnemies[id]->SetisAliveTrue();

	SPAWN_ENEMY_INFO e_info{};
	e_info.id = m_ppEnemies[id]->GetID();
	e_info.Quaternion = m_ppEnemies[id]->GetQuaternion();
	e_info.pos = m_ppEnemies[id]->GetPosition();
	e_info.destination = m_ppEnemies[id]->GetDestination();
	e_info.max_hp = m_ppEnemies[id]->GetHP();
	e_info.state = EnemyState::MOVE;

	for (short pl_id : _plist) {
		if (pl_id == -1) continue;
		if (clients[pl_id]._state != ST_INGAME) continue;
		clients[pl_id].send_spawn_enemy_packet(e_info);
	}
}

void CScene::ChangeInvincibleMode()
{
	invincible_mode = !invincible_mode;
	if (invincible_mode) { printf("무적 모드 on\n"); }
	else { printf("무적 모드 off"); }
}

void CScene::SpawnEnemy(char id)
{
	m_ppEnemies[id]->Reset();
	XMFLOAT3 p_pos = m_pSpaceship->GetPosition();
	XMFLOAT3 random_pos{ urdPos(dree), urdPos(dree), urdPos(dree) / 5.f };
	if (urdEnemyAI(dree) > 50) { random_pos.x = -random_pos.x; }
	if (urdEnemyAI(dree) > 50) { random_pos.y = -random_pos.y; }
	if (urdEnemyAI(dree) > 50) { random_pos.z = -random_pos.z; }

	m_ppEnemies[id]->SetStatus(cur_mission);
	m_ppEnemies[id]->SetPosition(random_pos.x + p_pos.x, random_pos.y + p_pos.y, random_pos.z + p_pos.z);
	m_ppEnemies[id]->state = EnemyState::MOVE;
	m_ppEnemies[id]->SetDestination();
	m_ppEnemies[id]->SetisAliveTrue();

	SPAWN_ENEMY_INFO e_info{};
	e_info.id = m_ppEnemies[id]->GetID();
	e_info.Quaternion = m_ppEnemies[id]->GetQuaternion();
	e_info.pos = m_ppEnemies[id]->GetPosition();
	e_info.destination = m_ppEnemies[id]->GetDestination();
	e_info.max_hp = m_ppEnemies[id]->GetHP();
	e_info.state = EnemyState::MOVE;

	for (short pl_id : _plist) {
		if (pl_id == -1) continue;
		if (clients[pl_id]._state != ST_INGAME) continue;
		clients[pl_id].send_spawn_enemy_packet(e_info);
	}
	//++cur_monster_num;
}

void CScene::SpawnMeteo(char i)
{
	XMFLOAT3 p_pos = m_pSpaceship->GetPosition();
	m_ppMeteoObjects[i]->m_xmf4x4ToParent = Matrix4x4::Identity();
	XMFLOAT3 random_pos{ urdPos(dree) , urdPos(dree), urdPos(dree) };
	XMFLOAT3 random_dir{ urdDir(dree) , urdDir(dree), urdDir(dree) };
	if (urdEnemyAI(dree) > 50) { random_pos.x = -random_pos.x;  random_dir.x = -random_dir.x; }
	if (urdEnemyAI(dree) > 50) { random_pos.y = -random_pos.y; random_dir.y = -random_dir.y; }
	if (urdEnemyAI(dree) > 50) { random_pos.z = -random_pos.z; random_dir.z = -random_dir.z; }
	m_ppMeteoObjects[i]->SetPosition(random_pos.x + p_pos.x, random_pos.y + p_pos.y, random_pos.z + p_pos.z);

	if (i < METEOS / 2) {
		m_ppMeteoObjects[i]->SetScale(urdScale(dree), urdScale(dree), urdScale(dree));
	}
	else {
		m_ppMeteoObjects[i]->SetScale(urdScale2(dree), urdScale2(dree), urdScale2(dree));
	}
	m_ppMeteoObjects[i]->SetMovingDirection(random_dir);
	m_ppMeteoObjects[i]->SetMovingSpeed(urdSpeed(dree));
}

void CScene::Send(char* p)
{
	for (auto pl_id : _plist)
	{
		if (pl_id == -1) continue;
		if (clients[pl_id]._state != ST_INGAME) continue;
		clients[pl_id].do_send(p);
	}
}

//--------------------------------------------------------------------------------------------------------

bool CScene::Start()
{
	_s_lock.lock();
	if (_state == SCENE_ALLOC) {
		_state = SCENE_INGAME;
		_s_lock.unlock();

		for (short pl_id:_plist) {
			if (pl_id == -1) continue;
			clients[pl_id]._s_lock.lock();
			clients[pl_id]._state = ST_INGAME;
			clients[pl_id]._s_lock.unlock();
			// 게임 스타트 패킷 send
		}

		for (char i = 0; i < METEOS; ++i) {
			m_ppMeteoObjects[i]->prev_time = chrono::steady_clock::now();
			TIMER_EVENT ev{ i, chrono::system_clock::now() + 33ms, EV_UPDATE_METEO, static_cast<short>(num) };
			timer_queue.push(ev);
		}

		TIMER_EVENT ev{ 0, chrono::system_clock::now() + 10s, EV_SPAWN_ENEMY, static_cast<short>(num) };
		timer_queue.push(ev);
		m_pSpaceship->prev_time = chrono::steady_clock::now();
		TIMER_EVENT ev1{ 0, chrono::system_clock::now() + 33ms, EV_UPDATE_SPACESHIP, static_cast<short>(num) };
		timer_queue.push(ev1);

		TIMER_EVENT ev2{ 0, chrono::system_clock::now() + 33ms, EV_SEND_SCENE_INFO, static_cast<short>(num) };
		timer_queue.push(ev2);

		TIMER_EVENT ev3{ 1, chrono::system_clock::now() + 100ms, EV_CHECK_CUTSCENE_END, static_cast<short>(num) };
		timer_queue.push(ev3);
		return true;
	}
	_s_lock.unlock();
	return false;

}

char CScene::InsertPlayer(short pl_id)
{
	_plist_lock.lock();
	for (int i = 0; i < _plist.size(); ++i)
	{
		if (_plist[i] == -1) {
			_plist[i] = pl_id;
			clients[pl_id].room_id = num;
			clients[pl_id].room_pid = i;
			_plist_lock.unlock();
			//if (i == 2) { Start(); }
			return i;
		}
	}
	_plist_lock.unlock();

	return -1;
}
