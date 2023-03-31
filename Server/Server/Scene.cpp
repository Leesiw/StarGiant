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

	for (int i = 0; i < MAX_USER; ++i) {
		CTerrainPlayer* pPlayer = new CTerrainPlayer();
		pPlayer->SetPosition(XMFLOAT3(425.0f + 10.0f * i, 250.0f, 740.0f));
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
			meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.0f, 0.0f, -0.066881f }, XMFLOAT3{ 10.0928127f, 10.0928127f, 10.218262f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
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
	}
	for (int i = PLASMACANNON_ENEMY; i < PLASMACANNON_ENEMY + LASER_ENEMY; ++i) {
		m_ppEnemies[i] = new CLaserEnemy;
		m_ppEnemies[i]->id = i;
	}
	for (int i = PLASMACANNON_ENEMY + LASER_ENEMY; i < ENEMIES; ++i) {
		m_ppEnemies[i] = new CMissileEnemy;
		m_ppEnemies[i]->id = i;
	}

	// misile
	for (int i = 0; i < ENEMY_BULLETS; ++i) {
		m_ppMissiles[i] = new CMissile;
	}

	// boss
	m_pBoss = new Boss();
	m_pBoss->SetPosition(10.f, 250.f, 640.f);
}

void CScene::ReleaseObjects()
{
	if (m_pSpaceship) { delete m_pSpaceship; }
	if (m_ppPlayers) { delete[] m_ppPlayers; }
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

void CScene::CheckMeteoByPlayerCollisions()
{
	m_pSpaceship->OnPrepareRender();
	m_pSpaceship->UpdateTransform();
	m_pSpaceship->UpdateBoundingBox();

	for (int i = 0; i < METEOS; ++i) {
		if (time(NULL) - m_ppMeteoObjects[i]->coll_time >= 1) {
			m_ppMeteoObjects[i]->UpdateBoundingBox();

			if (m_pSpaceship->HierarchyIntersects(m_ppMeteoObjects[i]))
			{
				m_ppMeteoObjects[i]->coll_time = time(NULL);
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
					m.lock();
					m_pSpaceship->SetHP(m_pSpaceship->GetHP() - 2);
					m.unlock();
				}

				for (auto& pl : clients)
				{
					if (pl.in_use == false) continue;
					pl.send_meteo_direction_packet(0, i, m_ppMeteoObjects[i]);
					pl.send_bullet_hit_packet(0, -1, m_pSpaceship->GetHP());
				}
			}
		}
	}
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
			printf("hit");
			m_ppEnemies[i]->hp -= m_pSpaceship->damage;
			for (auto& pl : clients)
			{
				if (pl.in_use == false) continue;
				pl.send_bullet_hit_packet(0, m_ppEnemies[i]->GetID(), m_ppEnemies[i]->hp);
			}

			if (m_ppEnemies[i]->hp <= 0) { 
				m_ppEnemies[i]->SetisAlive(false);
				--cur_monster_num;

				short num = urdEnemyAI(dree);
				ItemType item_type;
				if (num < 3) { 
					item_type = ItemType::JEWEL_ATT;
					if (items[item_type] < MAX_ITEM) {
						m_pSpaceship->damage = 4 + items[item_type];
					}
					else { return; }
				}
				else if(num < 6){ 
					item_type = ItemType::JEWEL_DEF;
					if (items[item_type] < MAX_ITEM) {
						m_pSpaceship->def = 1 + items[item_type];
					}
					else { return; }
				}
				else if(num < 9){ 
					item_type = ItemType::JEWEL_HEAL;
					if (items[item_type] < MAX_ITEM) {
						m_pSpaceship->heal = 11 + items[item_type];
					}
					else { return; }
				}
				else if (num < 12) { 
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
				ITEM_INFO info;
				info.type = item_type;
				info.num = items[item_type];
				for (auto& pl : clients)
				{
					if (pl.in_use == false) continue;
					pl.send_item_packet(0, info);
				}
			}
			return;
		}
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
	//플레이어와
	/*
	for (int i = 0; i < ENEMIES; ++i)
	{
		if (!m_ppEnemies[i]->GetisAlive()) { continue; }
		m_ppEnemies[i]->UpdateBoundingBox();

		if (m_pSpaceship->HierarchyIntersects(m_ppEnemies[i]))
		{
			XMFLOAT3 xmf3Sub = m_pSpaceship->GetPosition();
			xmf3Sub = Vector3::Subtract(m_ppEnemies[i]->GetPosition(), xmf3Sub);
			xmf3Sub = Vector3::Normalize(xmf3Sub);
			XMFLOAT3 vel = m_ppEnemies[i]->GetVelocity();
			float fLen = Vector3::Length(vel);
			xmf3Sub = Vector3::ScalarProduct(xmf3Sub, fLen, true);
			
			XMFLOAT3 vel2 = m_pSpaceship->GetVelocity();

			m_ppEnemies[i]->hp = -1;
			m_ppEnemies[i]->SetisAlive(false);
			
			m_pSpaceship->SetVelocity(Vector3::Add(vel2, xmf3Sub, -1.f));

			short real_damage = m_ppEnemies[i]->GetDamage() - m_pSpaceship->def;
			if (real_damage <= 0) { real_damage = 1; }
			short new_hp = m_pSpaceship->GetHP() - real_damage;
			m_pSpaceship->SetHP(new_hp);

			for (auto& pl : clients)
			{
				if (pl.in_use == false) continue;
				pl.send_bullet_hit_packet(0, i, m_ppEnemies[i]->hp);
				if (m_pSpaceship->GetHP() <= 0) { continue; }
				pl.send_bullet_hit_packet(0, -1, m_pSpaceship->GetHP());
			}
		}
	}
	*/

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
					m_ppMeteoObjects[i]->coll_time = time(NULL);
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

					for (auto& pl : clients)
					{
						if (pl.in_use == false) continue;
						pl.send_meteo_direction_packet(0, i, m_ppMeteoObjects[i]);
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
				xmf3Sub = Vector3::Normalize(xmf3Sub);
				XMFLOAT3 vel = m_ppEnemies[i]->GetVelocity();
				float fLen = Vector3::Length(vel) / 30.f;
				xmf3Sub = Vector3::ScalarProduct(xmf3Sub, fLen, true);
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
				xmf3Sub = Vector3::Normalize(xmf3Sub);
				float fLen = 100.f;
				xmf3Sub = Vector3::ScalarProduct(xmf3Sub, fLen, false);

				XMFLOAT3 vel2 = m_pSpaceship->GetVelocity();

				m_pSpaceship->SetVelocity(Vector3::Add(vel2, xmf3Sub, -1.f));
				
				if (m_pSpaceship->GetHP() > 0) {
					m.lock();
					m_pSpaceship->GetAttack(m_ppMissiles[i]->GetDamage());
					m.unlock();
				}

				for (auto& pl : clients)
				{
					if (pl.in_use == false) continue;
					pl.send_remove_missile_packet(0, i);
					pl.send_bullet_hit_packet(0, -1, m_pSpaceship->GetHP());
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
	}
	case MissionType::GO_PLANET: {
		break;
	}
	case MissionType::KILL_MONSTER_ONE_MORE_TIME: {
		break;
	}
	case MissionType::FIND_BOSS: {
		break;
	}
	}
	
}

void CScene::MissionClear()
{
	if (cur_mission != levels[cur_mission].NextMission) 
	{
		cur_mission = levels[cur_mission].NextMission;
		for (auto& pl : clients) {
			pl.send_mission_start_packet(cur_mission);
		}
	}
	else {

	}
}

void CScene::SpawnEnemy()
{
	XMFLOAT3 p_pos = m_pSpaceship->GetPosition();
	for (int i = 0; i < levels[cur_mission].SpawnMonsterNum; ++i)
	{
		if (cur_monster_num >= levels[cur_mission].MaxMonsterNum) { return; }
		
		std::random_shuffle(m_ppEnemies.begin(), m_ppEnemies.end());

		for (int j = 0; j < ENEMIES; ++j) {
			if (!m_ppEnemies[j]->GetisAlive()) {
				XMFLOAT3 random_pos{ urdPos(dree), urdPos(dree), urdPos(dree) / 5.f };
				if (urdEnemyAI(dree) > 50) { random_pos.x = -random_pos.x; }
				if (urdEnemyAI(dree) > 50) { random_pos.y = -random_pos.y; }
				if (urdEnemyAI(dree) > 50) { random_pos.z = -random_pos.z; }
				m_ppEnemies[j]->SetisAlive(true);
				m_ppEnemies[j]->SetPosition(random_pos.x + p_pos.x, random_pos.y + p_pos.y, random_pos.z + p_pos.z);
				m_ppEnemies[j]->state = EnemyState::IDLE;
				m_ppEnemies[j]->SetDestination();
				m_ppEnemies[j]->SetStatus(cur_mission);
				ENEMY_INFO e_info;
				e_info.id = m_ppEnemies[j]->GetID();
				e_info.Quaternion = m_ppEnemies[j]->GetQuaternion();
				e_info.pos = m_ppEnemies[j]->GetPosition();

				for (auto& pl : clients) {
					if (false == pl.in_use) continue;
					pl.send_enemy_packet(0, e_info);
				}
				++cur_monster_num;
				break;
			}
		}
	}
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

	for (auto& pl : clients) {
		if (false == pl.in_use) continue;
		pl.send_spawn_meteo_packet(0, i, m_ppMeteoObjects[i]);
	}
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	m_fEnemySpawnTimeRemaining -= fTimeElapsed;
	if (m_fEnemySpawnTimeRemaining < 0.0f)
	{
		SpawnEnemy();// 적 스폰
		m_fEnemySpawnTimeRemaining = m_fEnemySpawnTime;
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

			if (m_ppEnemies[i]->GetLaunchMissile()) {
				MissileInfo info = m_ppEnemies[i]->GetMissileInfo();
				if (info.damage == 0) continue;
				for (int j = 0; j < ENEMY_BULLETS; ++j) {
					if (!m_ppMissiles[j]->GetisActive()) {
						m_ppMissiles[j]->SetNewMissile(info);
						MISSILE_INFO m_info;
						m_info.id = j;
						m_info.pos = info.StartPos;
						m_info.Quaternion = info.Quaternion;
						for (auto& pl : clients) {	// 주기적으로 보내줘야 하는 것
							if (false == pl.in_use) continue;
							pl.send_missile_packet(0, m_info);
						}
						break;
					}
				}
			}
		}
	}

	for (int i = 0; i < ENEMY_BULLETS; ++i) {
		if (m_ppMissiles[i]->GetisActive()) {
			m_ppMissiles[i]->Animate(fTimeElapsed, m_pSpaceship);
			MISSILE_INFO m_info;
			m_info.id = i;
			m_info.pos = m_ppMissiles[i]->GetPosition();
			//printf("%f %f %f\n", m_info.pos.x, m_info.pos.y, m_info.pos.z);
			m_info.Quaternion = m_ppMissiles[i]->GetQuaternion();
			for (auto& pl : clients) {	// 주기적으로 보내줘야 하는 것
				if (false == pl.in_use) continue;
				if (m_ppMissiles[i]->GetisActive()) {
					pl.send_missile_packet(0, m_info);
				}
				else {
					pl.send_remove_missile_packet(0, i);
				}
			}
		}
	}

	if (m_pBoss) {
		m_pBoss->Animate(fTimeElapsed);
		m_pBoss->Boss_Ai(fTimeElapsed, m_pBoss->GetState(), m_pSpaceship->GetPosition(), m_pBoss->GetHP());;
	}

	m_pSpaceship->Animate(fTimeElapsed);
	m_pSpaceship->Update(fTimeElapsed);

	for (int i = 0; i < MAX_USER; ++i)
	{
		XMFLOAT3 pos[MAX_USER - 1]{};
		int num = 0;
		for (int j = 0; j < MAX_USER; ++j) {
			if (j == i) { continue; }
			if (clients[j].in_use) {
				pos[num] = m_ppPlayers[j]->GetPosition();
			}
			else { pos[num] = { 0.f, 0.f, 0.f }; }
			++num;
		}
		m_ppPlayers[i]->Update(fTimeElapsed, pos);
		if (clients[i].in_use && clients[i].type == PlayerType::INSIDE) {
			for (auto& pl : clients) {
				if (pl.in_use == false) continue;
				pl.send_inside_packet(i, m_ppPlayers[i]);
			}
		}
	}

	for (auto& pl : clients) {	// 주기적으로 보내줘야 하는 것
		if (false == pl.in_use) continue;
		pl.send_spaceship_packet(3, m_pSpaceship);
		pl.send_meteo_packet(0, m_ppMeteoObjects);
		for (int i = 0; i < MAX_USER; ++i) {
			if (clients[i].in_use == false) { continue; }
			pl.send_change_packet(i,clients[i].type);
		}
		// 적 위치?
	}

	CheckMeteoByPlayerCollisions();
	CheckEnemyCollisions();
	CheckMissileCollisions();

	CheckMissionComplete();
	//CheckObjectByBulletCollisions();
	//CheckEnemyByBulletCollisions();
}
