//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

CScene::CScene()
{
	for (int i = 0; i < ENEMIES; ++i)
	{
		m_ppEnemies[i] = NULL;
	}
}

CScene::~CScene()
{
}

void CScene::Init()
{
}

void CScene::BuildObjects()
{
	CAirplanePlayer* pAirplanePlayer = new CAirplanePlayer();
	pAirplanePlayer->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	pAirplanePlayer->mesh = true;
	pAirplanePlayer->boundingbox = BoundingOrientedBox{ XMFLOAT3(-0.000000f, -0.000000f, -0.000096f), XMFLOAT3(15.5f, 15.5f, 3.90426f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pSpaceship = pAirplanePlayer;
	//m_pCamera = m_pPlayer->GetCamera();
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

	for (int i = 0; i < ENEMIES / 3; ++i) {
		m_ppEnemies[i] = new CLaserEnemy;
		m_ppEnemies[i]->id = i;
	}
	for (int i = ENEMIES / 3; i < ENEMIES / 3 * 2; ++i) {
		m_ppEnemies[i] = new CPlasmaCannonEnemy;
		m_ppEnemies[i]->id = i;
	}
	for (int i = ENEMIES / 3 * 2; i < ENEMIES; ++i) {
		m_ppEnemies[i] = new CMissileEnemy;
		m_ppEnemies[i]->id = i;
	}
}

void CScene::ReleaseObjects()
{
	if (m_pSpaceship) { delete m_pSpaceship; }
	if (m_ppPlayers) { delete[] m_ppPlayers; }
	if (m_ppEnemies) { delete[] m_ppEnemies; }

	if (m_ppMeteoObjects)
	{
		delete[] m_ppMeteoObjects;
	}
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

				for (auto& pl : clients)
				{
					pl.send_meteo_direction_packet(0, i, m_ppMeteoObjects[i]);
				}
			}
		}
	}
}

void CScene::CheckEnemyByBulletCollisions(BULLET_INFO& data)
{
	float dist = 500.f; // 플레이어 사거리

	for (int i = 0; i < ENEMIES; ++i)
	{
		if (!m_ppEnemies[i]->GetisAlive()) { continue; }
		XMVECTOR pos = XMLoadFloat3(&data.pos);
		XMVECTOR dir = XMLoadFloat3(&data.direction);
		m_ppEnemies[i]->UpdateBoundingBox();
		if (m_ppEnemies[i]->m_xmOOBB.Intersects(pos, dir, dist)) //총알/적 충돌시
		{
			printf("hit");
			m_ppEnemies[i]->hp -= m_pSpaceship->damage;
			if (m_ppEnemies[i]->hp <= 0) { m_ppEnemies[i]->SetisAlive(false); }

			for (auto& pl : clients)
			{
				pl.send_bullet_hit_packet(0, i, m_ppEnemies[i]->hp);
			}
		}
	}
}

void CScene::CheckEnemyCollisions()
{
	// 적들끼리
	for (int i = 0; i < ENEMIES; ++i)
	{
		m_ppEnemies[i]->UpdateBoundingBox();
		for (int j = i + 1; j < ENEMIES; ++j) 
		{
			if (m_ppEnemies[i]->HierarchyIntersects(m_ppEnemies[j]))
			{
				XMFLOAT3 vel1 = m_ppEnemies[i]->GetVelocity();
				XMFLOAT3 vel2 = m_ppEnemies[j]->GetVelocity();
				float m1 = 1.0f; float m2 = 5.0f;
				float finalVelX1 = ((m1 - m2) / (m1 + m2)) * vel1.x + ((2.f * m2) / (m1 + m2)) * vel2.x;
				float finalVelY1 = ((m1 - m2) / (m1 + m2)) * vel1.y + ((2.f * m2) / (m1 + m2)) * vel2.y;
				float finalVelZ1 = ((m1 - m2) / (m1 + m2)) * vel1.z + ((2.f * m2) / (m1 + m2)) * vel2.z;
				float finalVelX2 = ((2.f * m1) / (m1 + m2)) * vel1.x + ((m2 - m1) / (m1 + m2)) * vel2.x;
				float finalVelY2 = ((2.f * m1) / (m1 + m2)) * vel1.y + ((m2 - m1) / (m1 + m2)) * vel2.y;
				float finalVelZ2 = ((2.f * m1) / (m1 + m2)) * vel1.z + ((m2 - m1) / (m1 + m2)) * vel2.z;

				m_ppEnemies[i]->SetVelocity(XMFLOAT3(finalVelX1, finalVelY1, finalVelZ1));
				m_ppEnemies[j]->SetVelocity(XMFLOAT3(finalVelX2, finalVelY2, finalVelZ2));
			}
		}
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
						pl.send_meteo_direction_packet(0, i, m_ppMeteoObjects[i]);
					}
				}
			}
		}
	}
}

void CScene::SpawnEnemy()
{
	XMFLOAT3 p_pos = m_pSpaceship->GetPosition();
	for (int i = 0; i < 9; ++i)
	{
		int enemy_num = ENEMIES / 3;
		int type = urdEnemyType(dree) * enemy_num;

		for (int j = type; j < type + enemy_num; ++j) {
			if (!m_ppEnemies[j]->GetisAlive()) {
				XMFLOAT3 random_pos{ urdPos(dree), urdPos(dree), urdPos(dree) / 5.f };
				if (urdEnemyAI(dree) > 50) { random_pos.x = -random_pos.x; }
				if (urdEnemyAI(dree) > 50) { random_pos.y = -random_pos.y; }
				if (urdEnemyAI(dree) > 50) { random_pos.z = -random_pos.z; }
				m_ppEnemies[j]->SetisAlive(true);
				m_ppEnemies[j]->SetPosition(random_pos.x + p_pos.x, random_pos.y + p_pos.y, random_pos.z + p_pos.z);
				m_ppEnemies[j]->state = EnemyState::IDLE;
				for (auto& pl : clients) {
					ENEMY_INFO e_info;
					e_info.id = j;
					e_info.Quaternion = m_ppEnemies[j]->GetQuaternion();
					e_info.pos = m_ppEnemies[j]->GetPosition();
					pl.send_enemy_packet(0, e_info);
				}
	
				break;
			}
		}
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
		//XMFLOAT3 sub = Vector3::Subtract(m_pos, p_pos);
		//if (fabs(sub.x) > 1000.0f || fabs(sub.y) > 1000.0f || fabs(sub.z) > 1000.0f) {
		if (dist > 1500.0f){

			m_ppMeteoObjects[i]->m_xmf4x4ToParent = Matrix4x4::Identity();
			XMFLOAT3 random_pos{ urdPos(dree) , urdPos(dree), urdPos(dree)};
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
		m_ppMeteoObjects[i]->Animate(fTimeElapsed, NULL); 
	}

	for (int i = 0; i < ENEMIES; ++i)
	{
		if (m_ppEnemies[i]->GetisAlive()) { m_ppEnemies[i]->Animate(fTimeElapsed, m_pSpaceship); }
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
				if (!pl.in_use) continue;
				pl.send_inside_packet(i, m_ppPlayers[i]);
			}
		}
	}

	for (auto& pl : clients) {	// 주기적으로 보내줘야 하는 것
		if (false == pl.in_use) continue;
		pl.send_spaceship_packet(3, m_pSpaceship);
		pl.send_meteo_packet(0, m_ppMeteoObjects);
		// 적 위치?
	}

	CheckMeteoByPlayerCollisions();
	CheckEnemyCollisions();
	//CheckObjectByBulletCollisions();
	//CheckEnemyByBulletCollisions();
}
