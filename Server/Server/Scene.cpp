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

void CScene::Init()
{
}

void CScene::BuildObjects()
{
	cur_mission = MissionType::TU_SIT;

	// player
	CAirplanePlayer* pAirplanePlayer = new CAirplanePlayer();
	pAirplanePlayer->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	pAirplanePlayer->mesh = true;
	pAirplanePlayer->boundingbox = BoundingOrientedBox{ XMFLOAT3(-0.000000f, -0.000000f, -0.000096f), XMFLOAT3(15.5f, 15.5f, 3.90426f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pSpaceship = pAirplanePlayer;

	_plist.fill(-1);

	for (int i = 0; i < 3; ++i) {
		CTerrainPlayer* pPlayer = new CTerrainPlayer();
		pPlayer->SetPosition(XMFLOAT3(425.0f + 10.0f * i, 10.0f, 740.0f));
		m_ppPlayers[i] = pPlayer;
	}

	// meteo
	CMeteoObject* meteo = NULL;

	for (int i = 0; i < METEOS; ++i) {
		meteo = new CMeteoObject();
		meteo->SetPosition(urdPos(dree), urdPos(dree), urdPos(dree));
		meteo->SetMovingDirection(XMFLOAT3(urdPos(dree), urdPos(dree), urdPos(dree)));
		//short id = urdModelID(dree);
		//meteo->SetModelId(id);
		//meteo->mesh = true;
		if (i <  METEOS/2) {
			meteo->SetScale(urdScale(dree), urdScale(dree), urdScale(dree));
			meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.188906f, 0.977625f, 0.315519f }, XMFLOAT3{ 1.402216f, 1.458820f, 1.499708f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
		}
		else {
			meteo->SetScale(urdScale2(dree), urdScale2(dree), urdScale2(dree));
			// 크기 정확히 모르겠음 임시
			//meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{  -0.0167256, 0.71804,  -0.0466012 }, XMFLOAT3{ 0.882965, 0.858064, 0.828712 }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
			meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{  -0.0167256f, 0.71804f,  -0.0466012f }, XMFLOAT3{ 4.414825f, 4.29032f, 4.14356f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
			
			//meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.0f, 0.0f, -0.066881f }, XMFLOAT3{ 10.0928127f, 10.0928127f, 10.218262f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
			//meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.188906f, 0.977625f, 0.315519f }, XMFLOAT3{ 1.402216f, 1.458820f, 1.499708f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
			//meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.000628f, -0.011224f, -0.003297f }, XMFLOAT3{ 2.89832f, 2.51931f, 2.78528f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
		}
		m_ppMeteoObjects[i] = meteo;
		m_ppMeteoObjects[i]->UpdateTransform(NULL);
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

	// boss
	m_pBoss = new Boss();
	m_pBoss->scene_num = num;
	m_pBoss->SetPosition(3000.f, 3000.f, 3000.f);
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
}

void CScene::Reset()
{
	_id = -1;
	m_pSpaceship->SetPosition(XMFLOAT3(0.f, 0.f, 0.f));
	m_pSpaceship->Reset();

	items[ItemType::JEWEL_ATT] = 0;
	items[ItemType::JEWEL_DEF] = 0;
	items[ItemType::JEWEL_HEAL] = 0;
	items[ItemType::JEWEL_HP] = 0;

	//_plist_lock.lock();
	_plist.fill(-1);
	//_plist_lock.unlock();

	cur_mission = MissionType::TU_SIT;
	for (char i = 0; i < 3; ++i) {
		m_ppPlayers[i]->SetPosition(XMFLOAT3(425.0f + 10.0f * i, 10.0f, 740.0f));
	}

	for (int i = 0; i < METEOS; ++i) {
		m_ppMeteoObjects[i]->SetPosition(urdPos(dree), urdPos(dree), urdPos(dree));
		m_ppMeteoObjects[i]->SetMovingDirection(XMFLOAT3(urdPos(dree), urdPos(dree), urdPos(dree)));
		m_ppMeteoObjects[i]->SetScale(urdScale(dree), urdScale(dree), urdScale(dree));
		if (i < METEOS / 2) {
			m_ppMeteoObjects[i]->SetScale(urdScale(dree), urdScale(dree), urdScale(dree));
		}
		else {
			m_ppMeteoObjects[i]->SetScale(urdScale2(dree), urdScale2(dree), urdScale2(dree));
		}
		m_ppMeteoObjects[i]->UpdateTransform(NULL);
	}

	for (int i = 0; i < ENEMIES; ++i) {
		m_ppEnemies[i]->SetisAlive(false);
	}

	for (int i = 0; i < ENEMY_BULLETS; ++i) {
		m_ppMissiles[i]->SetisActive(false);
	}

	kill_monster_num = 0;
	cur_monster_num = 0;
	heal_player = -1;

	m_pBoss->SetPosition(3000.f, 3000.f, 3000.f);
}

void CScene::CheckMeteoByPlayerCollisions()
{
	/*
	m_pSpaceship->OnPrepareRender();
	m_pSpaceship->UpdateTransform();
	m_pSpaceship->UpdateBoundingBox();

	for (int i = 0; i < METEOS; ++i) {
	//	if (time(NULL) - m_ppMeteoObjects[i]->coll_time >= 1) {
			m_ppMeteoObjects[i]->UpdateBoundingBox();

			if (m_pSpaceship->HierarchyIntersects(m_ppMeteoObjects[i]))
			{
		//		m_ppMeteoObjects[i]->coll_time = time(NULL);
				XMFLOAT3 vel1 = m_pSpaceship->GetVelocity();
				XMFLOAT3 vel2 = m_ppMeteoObjects[i]->GetMovingDirection();
				float m1 = 1.0f; float m2 = 5.0f;
				float finalVelX1 = ((m1 - m2) / (m1 + m2)) * vel1.x + ((2.f * m2) / (m1 + m2)) * vel2.x;
				float finalVelY1 = ((m1 - m2) / (m1 + m2)) * vel1.y + ((2.f * m2) / (m1 + m2)) * vel2.y;
				float finalVelZ1 = ((m1 - m2) / (m1 + m2)) * vel1.z + ((2.f * m2) / (m1 + m2)) * vel2.z;
				float finalVelX2 = ((2.f * m1) / (m1 + m2)) * vel1.x + ((m2 - m1) / (m1 + m2)) * vel2.x;
				float finalVelY2 = ((2.f * m1) / (m1 + m2)) * vel1.y + ((m2 - m1) / (m1 + m2)) * vel2.y;
				float finalVelZ2 = ((2.f * m1) / (m1 + m2)) * vel1.z + ((m2 - m1) / (m1 + m2)) * vel2.z;

				m_pSpaceship->SetVelocity(XMFLOAT3(finalVelX1, finalVelY1, finalVelZ1));
				m_ppMeteoObjects[i]->SetMovingDirection(XMFLOAT3(finalVelX2, finalVelY2, finalVelZ2));

				m_pSpaceship->Move(m_pSpaceship->GetVelocity(), true);
				if (m_pSpaceship->GetHP() > 0) {
					m_pSpaceship->SetHP(m_pSpaceship->GetHP() - 2);
				}

				for (short pl_id : _plist) {
					if (pl_id == -1) continue;
					if (clients[pl_id]._state != ST_INGAME) continue;
					clients[pl_id].send_meteo_direction_packet(0, i, m_ppMeteoObjects[i]);
					clients[pl_id].send_bullet_hit_packet(0, -1, m_pSpaceship->GetHP());

				}
			}
		//}
	}*/
}

void CScene::CheckEnemyByBulletCollisions(BULLET_INFO& data)
{
	float dist = 500.f; // 플레이어 사거리

	XMVECTOR pos = XMLoadFloat3(&data.pos);
	XMVECTOR dir = XMLoadFloat3(&data.direction);

	for (int i = 0; i < ENEMIES; ++i)
	{
		if (!m_ppEnemies[i]->GetisAlive()) { continue; }
		if (m_ppEnemies[i]->m_xmOOBB.Intersects(pos, dir, dist)) //총알/적 충돌시
		{
			m_ppEnemies[i]->hp -= m_pSpaceship->damage;

			for (short pl_id : _plist) {
				if (pl_id == -1) continue;
				if (clients[pl_id]._state != ST_INGAME) continue;
				clients[pl_id].send_bullet_hit_packet(m_ppEnemies[i]->GetID(), m_ppEnemies[i]->hp);
			}


			if (m_ppEnemies[i]->hp <= 0) { 
				m_ppEnemies[i]->SetisAlive(false);
				--cur_monster_num;

				// 미션
				if (cur_mission == MissionType::TU_KILL) 
				{
					MissionClear();
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

					if (kill_monster_num == 15) {
						kill_monster_num = 0;
						MissionClear();
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

					if (kill_monster_num == 20) {
						kill_monster_num = 0;
						MissionClear();
					}
				}

				GetJewels();
			}
			return;
		}
	}

	m_pBoss->UpdateBoundingBox();
	if (m_pBoss->m_xmOOBB.Intersects(pos, dir, dist)) // 보스 충돌처리
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

void CScene::CheckMeteoByBulletCollisions(BULLET_INFO& data)
{
	float dist = 500.f; // 플레이어 사거리
	XMVECTOR pos = XMLoadFloat3(&data.pos);
	XMVECTOR dir = XMLoadFloat3(&data.direction);

	for (int i = 0; i < METEOS; ++i)
	{
		if (m_ppMeteoObjects[i]->m_xmOOBB.Intersects(pos, dir, dist)) //총알/적 충돌시
		{
			SpawnMeteo(i);
			return;
		}
	}
}

void CScene::CheckEnemyCollisions()
{

	for (int i = 0; i < ENEMIES; ++i) {
		m_ppEnemies[i]->UpdateBoundingBox();
	}

	//적&운석
	for (int i = 0; i < METEOS; ++i) {
		if (time(NULL) - m_ppMeteoObjects[i]->coll_time >= 1) {
			m_ppMeteoObjects[i]->UpdateBoundingBox();
			for (int j = 0; j < ENEMIES; ++j) {
				
				if (m_ppEnemies[j]->HierarchyIntersects(m_ppMeteoObjects[i]))
				{
					//m_ppMeteoObjects[i]->coll_time = time(NULL);
					XMFLOAT3 vel1 = m_ppEnemies[j]->GetVelocity();
					XMFLOAT3 vel2 = m_ppMeteoObjects[i]->GetMovingDirection();
					float m1 = 1.0f; float m2 = 5.0f;
					float finalVelX1 = ((m1 - m2) / (m1 + m2)) * vel1.x + ((2.f * m2) / (m1 + m2)) * vel2.x;
					float finalVelY1 = ((m1 - m2) / (m1 + m2)) * vel1.y + ((2.f * m2) / (m1 + m2)) * vel2.y;
					float finalVelZ1 = ((m1 - m2) / (m1 + m2)) * vel1.z + ((2.f * m2) / (m1 + m2)) * vel2.z;
					float finalVelX2 = ((2.f * m1) / (m1 + m2)) * vel1.x + ((m2 - m1) / (m1 + m2)) * vel2.x;
					float finalVelY2 = ((2.f * m1) / (m1 + m2)) * vel1.y + ((m2 - m1) / (m1 + m2)) * vel2.y;
					float finalVelZ2 = ((2.f * m1) / (m1 + m2)) * vel1.z + ((m2 - m1) / (m1 + m2)) * vel2.z;

					m_ppEnemies[j]->SetVelocity(XMFLOAT3(finalVelX1, finalVelY1, finalVelZ1));
					m_ppMeteoObjects[i]->SetMovingDirection(XMFLOAT3(finalVelX2, finalVelY2, finalVelZ2));

					for (short pl_id : _plist) {
						if (pl_id == -1) continue;
						if (clients[pl_id]._state != ST_INGAME) continue;
						clients[pl_id].send_meteo_direction_packet(i, m_ppMeteoObjects[i]);
					}
				}
			}
		}
	}

	// 적들끼리
	for (int i = 0; i < ENEMIES; ++i)
	{
		if (!m_ppEnemies[i]->GetisAlive()) { continue; }
		for (int j = i + 1; j < ENEMIES; ++j) 
		{
			if (!m_ppEnemies[j]->GetisAlive()) { continue; }
			if (m_ppEnemies[i]->HierarchyIntersects(m_ppEnemies[j]))
			{
				XMFLOAT3 xmf3Sub = m_ppEnemies[j]->GetPosition();
				xmf3Sub = Vector3::Subtract(m_ppEnemies[i]->GetPosition(), xmf3Sub);
				if (Vector3::Length(xmf3Sub) > 0.0001f) {
					xmf3Sub = Vector3::Normalize(xmf3Sub);
				}
				XMFLOAT3 vel = m_ppEnemies[i]->GetVelocity();
				float fLen = Vector3::Length(vel) / 30.f;
				xmf3Sub = Vector3::ScalarProduct(xmf3Sub, fLen, false);
				XMFLOAT3 vel2 = m_ppEnemies[j]->GetVelocity();

				m_ppEnemies[i]->SetVelocity(Vector3::Add(vel, xmf3Sub));
				m_ppEnemies[j]->SetVelocity(Vector3::Add(vel2, xmf3Sub, -1.f));
			}
		}
	}

	
}


void CScene::CheckMissileCollisions()
{
	for (int i = 0; i < ENEMY_BULLETS; ++i) {
		if (m_ppMissiles[i]->GetisActive()) {
			m_ppMissiles[i]->UpdateBoundingBox();
			if (m_ppMissiles[i]->HierarchyIntersects(m_pSpaceship))
			{
				m_ppMissiles[i]->SetisActive(false);
				// 충돌처리
				
				XMFLOAT3 xmf3Sub = m_pSpaceship->GetPosition();
				xmf3Sub = Vector3::Subtract(m_ppMissiles[i]->GetPosition(), xmf3Sub);
				if (Vector3::Length(xmf3Sub) > 0.0001f) {
					xmf3Sub = Vector3::Normalize(xmf3Sub);
				}
				float fLen = 100.f;
				xmf3Sub = Vector3::ScalarProduct(xmf3Sub, fLen, false);

				XMFLOAT3 vel2 = m_pSpaceship->GetVelocity();

				m_pSpaceship->SetVelocity(Vector3::Add(vel2, xmf3Sub, -1.f));
				
				if (m_pSpaceship->GetHP() > 0) {
					m_pSpaceship->GetAttack(m_ppMissiles[i]->GetDamage());
				}

				for (short pl_id : _plist) {
					if (pl_id == -1) continue;
					if (clients[pl_id]._state != ST_INGAME) continue;
					clients[pl_id].send_remove_missile_packet(i);
					clients[pl_id].send_bullet_hit_packet(-1, m_pSpaceship->GetHP());
				}
			}
		}
	}

}

void CScene::CheckBossCollisions()
{
	if (m_pBoss)
		m_pBoss->UpdateBoundingBox();
}

void CScene::CheckMissionComplete()
{
	switch (cur_mission) {
		/*
	case MissionType::TU_SIT: {
		break;
	}
	case MissionType::TU_KILL: {
		break;
	}
	case MissionType::TU_HILL: {
		break;
	}
	case MissionType::TU_END: {
		break;
	}
	case MissionType::GET_JEWELS: {
		break;
	}
	case MissionType::Kill_MONSTER: {
		break;
	}*/
	case MissionType::GO_PLANET: {
		XMFLOAT3 player_pos = m_pSpaceship->GetPosition();
		XMFLOAT3 planet_pos{ 10000.f, 10000.f, 10000.f }; // 임시 좌표

		float dist = Vector3::Length(Vector3::Subtract(player_pos, planet_pos));
		if (dist < 1000.f) {
			MissionClear();
		}
		break;
	}/*
	case MissionType::KILL_MONSTER_ONE_MORE_TIME: {
		break;
	}*/
	case MissionType::FIND_BOSS: {
		float dist = Vector3::Length(Vector3::Subtract(m_pSpaceship->GetPosition(), m_pBoss->GetPosition()));		// 임시 좌표
		if (dist < 1500.0f) {
			MissionClear();
		}

		break;
	}
	}
	
}

void CScene::MissionClear()
{
	if (cur_mission != levels[cur_mission].NextMission) 
	{
		cur_mission = levels[cur_mission].NextMission;
		if (cur_mission == MissionType::FIND_BOSS)
			m_pBoss->SetPosition(Vector3::Add(m_pSpaceship->GetPosition(), XMFLOAT3(2400.0f, 0.f, 0.f)));

		if (cur_mission == MissionType::DEFEAT_BOSS) {
			TIMER_EVENT ev{ 0, chrono::system_clock::now() + 33ms, EV_UPDATE_BOSS, num };
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

void CScene::GetJewels()
{

	short num = urdEnemyAI(dree);
	ItemType item_type;
	if (num < 15) {
		item_type = ItemType::JEWEL_ATT;
		if (items[item_type] < MAX_ITEM) {
			m_pSpaceship->damage = 4 + items[item_type];
		}
		else { return; }
	}
	else if (num < 30) {
		item_type = ItemType::JEWEL_DEF;
		if (items[item_type] < MAX_ITEM) {
			m_pSpaceship->def = 1 + items[item_type];
		}
		else { return; }
	}
	else if (num < 45) {
		item_type = ItemType::JEWEL_HEAL;
		if (items[item_type] < MAX_ITEM) {
			m_pSpaceship->heal = 11 + items[item_type];
		}
		else { return; }
	}
	else if (num < 60) {
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
		if (std::find_if(items.begin(), items.end(),
			[](const auto& item) {
			return item.second == 0;
		}) == items.end()) {	// 0이 없을 때
			MissionClear();
		}
	}
}

void CScene::SpawnEnemy(char id)
{
	XMFLOAT3 p_pos = m_pSpaceship->GetPosition();
	XMFLOAT3 random_pos{ urdPos(dree), urdPos(dree), urdPos(dree) / 5.f };
	if (urdEnemyAI(dree) > 50) { random_pos.x = -random_pos.x; }
	if (urdEnemyAI(dree) > 50) { random_pos.y = -random_pos.y; }
	if (urdEnemyAI(dree) > 50) { random_pos.z = -random_pos.z; }
	m_ppEnemies[id]->SetisAlive(true);
	m_ppEnemies[id]->SetPosition(random_pos.x + p_pos.x, random_pos.y + p_pos.y, random_pos.z + p_pos.z);
	m_ppEnemies[id]->state = EnemyState::IDLE;
	m_ppEnemies[id]->SetDestination();

	SPAWN_ENEMY_INFO e_info{};
	e_info.id = m_ppEnemies[id]->GetID();
	e_info.Quaternion = m_ppEnemies[id]->GetQuaternion();
	e_info.pos = m_ppEnemies[id]->GetPosition();
	e_info.destination = m_ppEnemies[id]->GetDestination();
	e_info.max_hp = m_ppEnemies[id]->GetHP();
	e_info.state = EnemyState::IDLE;

	for (short pl_id : _plist) {
		if (pl_id == -1) continue;
		if (clients[pl_id]._state != ST_INGAME) continue;
		clients[pl_id].send_spawn_enemy_packet(e_info);
	}
	++cur_monster_num;
}

void CScene::SpawnMeteo(char i)
{
	XMFLOAT3 p_pos = m_pSpaceship->GetPosition();
	m_ppMeteoObjects[i]->m_xmf4x4ToParent = Matrix4x4::Identity();
	XMFLOAT3 random_pos{ urdPos(dree) , urdPos(dree), urdPos(dree) };
	if (urdEnemyAI(dree) > 50) { random_pos.x = -random_pos.x; }
	if (urdEnemyAI(dree) > 50) { random_pos.y = -random_pos.y; }
	if (urdEnemyAI(dree) > 50) { random_pos.z = -random_pos.z; }
	m_ppMeteoObjects[i]->SetPosition(random_pos.x + p_pos.x, random_pos.y + p_pos.y, random_pos.z + p_pos.z);

	if (i < METEOS / 2) {
		m_ppMeteoObjects[i]->SetScale(urdScale(dree), urdScale(dree), urdScale(dree));
	}
	else {
		m_ppMeteoObjects[i]->SetScale(urdScale2(dree), urdScale2(dree), urdScale2(dree));
	}
	m_ppMeteoObjects[i]->SetMovingDirection(XMFLOAT3(urdPos3(dree), urdPos3(dree), urdPos3(dree)));

	for (short pl_id : _plist) {
		if (pl_id == -1) continue;
		if (clients[pl_id]._state != ST_INGAME) continue;
		clients[pl_id].send_spawn_meteo_packet(i, m_ppMeteoObjects[i]);
	}
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

void CScene::AnimateObjects(float fTimeElapsed)
{

	float dist;
	dist = Vector3::Length(Vector3::Subtract(m_pSpaceship->GetPosition(), m_pBoss->GetPosition()));
	if (dist < 1000.f) // boss 막기
	{
		XMFLOAT3 ToGo = Vector3::Subtract(m_pSpaceship->GetPosition(), m_pBoss->GetPosition());
		ToGo = Vector3::ScalarProduct(ToGo, 800.f);
		ToGo = Vector3::Add(m_pBoss->GetPosition(), ToGo);
		m_pSpaceship->SetPosition(ToGo);
	}


	XMFLOAT3 p_pos = m_pSpaceship->GetPosition();
	for (int i = 0; i < METEOS; ++i) 
	{ 
		XMFLOAT3 m_pos = m_ppMeteoObjects[i]->GetPosition();
		float dist = Vector3::Length(Vector3::Subtract(m_pos, p_pos));
		if (dist > 1500.0f){
			SpawnMeteo(i);
		}
		m_ppMeteoObjects[i]->Animate(fTimeElapsed, NULL); 
	}

	for (int i = 0; i < ENEMIES; ++i)
	{
		if (m_ppEnemies[i]->GetisAlive()) { 
			m_ppEnemies[i]->Animate(fTimeElapsed, m_pSpaceship); 
			/*
			if (m_ppEnemies[i]->GetLaunchMissile()) {
				//MissileInfo info = m_ppEnemies[i]->GetMissileInfo();
				if (info.damage == 0) continue;
				for (int j = 0; j < ENEMY_BULLETS; ++j) {
					if (!m_ppMissiles[j]->GetisActive()) {
						m_ppMissiles[j]->SetNewMissile(info);
						MISSILE_INFO m_info{};
						m_info.id = j;
						m_info.pos = info.StartPos;
						m_info.Quaternion = info.Quaternion;
						
						for (short pl_id : _plist) {
							if (pl_id == -1) continue;
							if (clients[pl_id]._state != ST_INGAME) continue;
							clients[pl_id].send_missile_packet(m_info);
						}
						break;
					}
				}
			}*/
		}
	}

	for (int i = 0; i < ENEMY_BULLETS; ++i) {
		if (m_ppMissiles[i]->GetisActive()) {
			m_ppMissiles[i]->Animate(fTimeElapsed, m_pSpaceship);
			MISSILE_INFO m_info{};
			m_info.id = i;
			m_info.pos = m_ppMissiles[i]->GetPosition();
			//printf("%f %f %f\n", m_info.pos.x, m_info.pos.y, m_info.pos.z);
			m_info.Quaternion = m_ppMissiles[i]->GetQuaternion();

			for (short pl_id : _plist) {
				if (pl_id == -1) continue;
				if (clients[pl_id]._state != ST_INGAME) continue;
				if (m_ppMissiles[i]->GetisActive()) {
					clients[pl_id].send_missile_packet(m_info);
				}
				else {
					clients[pl_id].send_remove_missile_packet(i);
				}
			}
		}
	}

	if (m_pBoss) {
		m_pBoss->Animate(fTimeElapsed);
		m_pBoss->Boss_Ai(fTimeElapsed, m_pBoss->GetState(), m_pSpaceship, m_pBoss->GetHP());;
	}

	m_pSpaceship->Update(fTimeElapsed);

	
	for (int i = 0; i < 3; ++i)
	{
		if (_plist[i] == -1) { continue; }
		XMFLOAT3 pos[2]{};
		int num = 0;
		for (int j = 0; j < 3; ++j) {
			if (j == i) { continue; }
			if (_plist[j] == -1) { continue; }
			if (clients[_plist[j]]._state == ST_INGAME) {
				pos[num] = m_ppPlayers[j]->GetPosition();
			}
			else { pos[num] = { 0.f, 0.f, 0.f }; }
			++num;
		}
		m_ppPlayers[i]->Update(fTimeElapsed, pos);

		if (clients[_plist[i]]._state == ST_INGAME && clients[_plist[i]].type == PlayerType::INSIDE) {
			for (short pl_id : _plist) {
				if (pl_id == -1) continue;
				if (clients[pl_id]._state != ST_INGAME) continue;
				clients[pl_id].send_inside_packet(i, m_ppPlayers[i]);
			}
		}
	}



	for (short pl_id : _plist) {
		if (pl_id == -1) continue;
		if (clients[pl_id]._state != ST_INGAME) continue;
		clients[pl_id].send_spaceship_packet(m_pSpaceship);
	}

	m_pSpaceship->move_time = 0;

//	if (send_time % 30 == 0) {
		for (int i = 0; i < ENEMIES; ++i) {
			if (m_ppEnemies[i]->GetisAlive()) {
				ENEMY_INFO info{};
				info.id = m_ppEnemies[i]->GetID();
				info.Quaternion = m_ppEnemies[i]->GetQuaternion();
				info.pos = m_ppEnemies[i]->GetPosition();
				info.velocity = m_ppEnemies[i]->GetVelocity();

				for (short pl_id : _plist) {
					if (pl_id == -1) continue;
					if (clients[pl_id]._state != ST_INGAME) continue;
					clients[pl_id].send_enemy_packet(info);
				}
			}
		}

		for (short pl_id : _plist) {
			if (pl_id == -1) continue;
			if (clients[pl_id]._state != ST_INGAME) continue;
			clients[pl_id].send_meteo_packet(m_ppMeteoObjects);
			//for (int i = 0; i < 3; ++i) {
			//	if (_plist[i] == -1) continue;
			//	if (clients[_plist[i]]._state == !ST_INGAME) { continue; }
			//	clients[pl_id].send_change_packet(i, clients[_plist[i]].type);
			//}
		}
		send_time = 0;
//	}

		/*
	if (heal_player != -1) {
		std::chrono::duration<double>sec = std::chrono::system_clock::now() - heal_start;
		heal_start = std::chrono::system_clock::now();
		if (m_pSpaceship->GetHeal(sec.count())) {
			for (short pl_id : _plist) {
				if (pl_id == -1) continue;
				if (clients[pl_id]._state != ST_INGAME) continue;
				clients[pl_id].send_bullet_hit_packet(0, -1, m_pSpaceship->GetHP());
			}
		}
	}*/

	CheckMeteoByPlayerCollisions();
	CheckEnemyCollisions();
	CheckMissileCollisions();

	CheckMissionComplete();
	//CheckObjectByBulletCollisions();
	//CheckEnemyByBulletCollisions();

	++send_time;
}



//--------------------------------------------------------------------------------------------------------

void CScene::Start()
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
			TIMER_EVENT ev{ i, chrono::system_clock::now() + 33ms, EV_UPDATE_METEO, num };
			timer_queue.push(ev);
		}

		TIMER_EVENT ev{ 0, chrono::system_clock::now() + 20s, EV_SPAWN_ENEMY, num };
		timer_queue.push(ev);

		TIMER_EVENT ev1{ 0, chrono::system_clock::now() + 33ms, EV_UPDATE_SPACESHIP, num };
		timer_queue.push(ev1);

		return;
	}
	_s_lock.unlock();

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

	_s_lock.lock();
	if (_state == SCENE_INGAME) {
		clients[pl_id]._s_lock.lock();
		clients[pl_id]._state = ST_INGAME;
		clients[pl_id]._s_lock.unlock();
	}
	_s_lock.unlock();

	return -1;
}
