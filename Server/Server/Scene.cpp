//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::Init()
{
}

std::random_device rdd;
std::default_random_engine dree(rdd());
std::uniform_real_distribution<float> urdPos(-500, 500);
std::uniform_real_distribution<float> urdScale(5, 8);
std::uniform_int_distribution<short>	urdModelID(0, 1);

void CScene::BuildObjects()
{
	// meteo
	CMeteoObject* meteo = NULL;


	for (int i = 0; i < METEOS; ++i) {
		meteo = new CMeteoObject();
		meteo->SetPosition(urdPos(dree), urdPos(dree), urdPos(dree));
		meteo->SetScale(urdScale(dree), urdScale(dree), urdScale(dree));
		meteo->SetMovingDirection(XMFLOAT3(urdPos(dree), urdPos(dree), urdPos(dree)));
		//short id = urdModelID(dree);
		//meteo->SetModelId(id);
		//meteo->mesh = true;
		if (i <  METEOS/2) {
			meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.188906f, 0.977625f, 0.315519f }, XMFLOAT3{ 1.402216f, 1.0f, 1.0f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
		}
		else {
			meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.188906f, 0.977625f, 0.315519f }, XMFLOAT3{ 1.402216f, 1.0f, 1.0f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
			//meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.188906f, 0.977625f, 0.315519f }, XMFLOAT3{ 1.402216f, 1.458820f, 1.499708f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
			//meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.000628f, -0.011224f, -0.003297f }, XMFLOAT3{ 2.89832f, 2.51931f, 2.78528f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
		}
		m_ppMeteoObjects[i] = meteo;
		m_ppMeteoObjects[i]->UpdateTransform(NULL);
	}
}

void CScene::ReleaseObjects()
{
	if (m_ppMeteoObjects)
	{
		delete[] m_ppMeteoObjects;
	}
}

bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CScene::CheckMeteoByPlayerCollisions()
{
	m_pSpaceship->OnPrepareRender();
	m_pSpaceship->UpdateTransform();
	m_pSpaceship->UpdateBoundingBox();

	for (int i = 0; i < METEOS; ++i) {
		
		m_ppMeteoObjects[i]->UpdateBoundingBox();
		
		if (m_pSpaceship->HierarchyIntersects(m_ppMeteoObjects[i]))
		{
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
		}

		//m_ppGameObjects[i]->m_pChild->aabb = BoundingBox(m_ppGameObjects[i]->m_pChild->GetPosition(), XMFLOAT3(10.0f, 10.0f, 10.0f));
		//m_pPlayer->m_pChild->aabb = BoundingBox(m_pPlayer->m_pChild->GetPosition(), XMFLOAT3(10.0f, 10.0f, 10.0f));

		//if (m_ppGameObjects[i]->m_pChild->aabb.Intersects(m_pPlayer->m_pChild->aabb)) {
		//	std::cout << "Ãæµ¹! ";
		//}



		/*if (m_pPlayer->m_xmOOBB.Intersects(Objects->m_ppObjects[i]->m_xmOOBB)) {

			XMFLOAT3 xmf3Sub = Objects->m_ppObjects[i]->GetPosition();
			xmf3Sub = Vector3::Subtract(m_pPlayer->GetPosition(), xmf3Sub);
			xmf3Sub = Vector3::Normalize(xmf3Sub);

			xmf3Sub.y = 0;
			std::cout << "Ãæµ¹! ";
			m_pPlayer->Move(xmf3Sub, false);
		}*/
	}
}

void CScene::CheckObjectByBulletCollisions()
{

	CBulletObject** ppBullets = m_pSpaceship->m_ppBullets;
	for (int i = 0; i < METEOS; ++i)
	{
		if (!m_ppMeteoObjects[i]->mesh) continue;
		for (int j = 0; j < BULLETS; j++)
		{
			//ppBullets[j]->UpdateBoundingBox();
			if (ppBullets[j]->m_bActive) {
				
				m_ppMeteoObjects[i]->aabb = BoundingBox(m_ppMeteoObjects[i]->GetPosition(), XMFLOAT3(10.0f, 10.0f, 10.0f));
				ppBullets[j]->aabb = BoundingBox(ppBullets[j]->GetPosition(), XMFLOAT3(20.0f, 20.0f, 20.0f));

				if (m_ppMeteoObjects[i]->aabb.Intersects(ppBullets[j]->aabb)) {
					//if (ppBullets[j]->HierarchyIntersects(m_ppGameObjects[i])) {
					//m_ppMeteoObjects[i]->hp -= 3;
					ppBullets[j]->Reset();

					for (auto& pl : clients) {
						if (false == pl.in_use) continue;
						pl.send_bullet_hit_packet(0, i, j);
					}
				}
			}
		}
	}
}

void CScene::CheckEnemyByBulletCollisions()
{

	CBulletObject** ppBullets = m_pSpaceship->m_ppBullets;
	/*
	if (m_enemy && m_enemy->hp < 0) return;
	for (int j = 0; j < BULLETS; j++)
	{
		if (ppBullets[j]->m_bActive) {
			XMFLOAT3 pos = m_enemy->GetPosition();
			pos.z += 130.0f;
			m_enemy->aabb = BoundingBox(pos, XMFLOAT3(8.0f, 8.0f, 3.0f));
			ppBullets[j]->aabb = BoundingBox(ppBullets[j]->GetPosition(), XMFLOAT3(20.0f, 20.0f, 50.0f));
			//m_enemy->m_xmOOBB.Center.z += 130.0f;
			if (m_enemy->aabb.Intersects(ppBullets[j]->aabb)) {
				//if (ppBullets[j]->HierarchyIntersects(m_ppGameObjects[i])) {
				m_enemy->hp -= 3;
				ppBullets[j]->Reset();
				if (m_enemy->hp < 0) {
					m_enemy->SetResetWaitingTime();
				}
			}
		}
	}
	*/
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	//m_ppGameObjects[1]->turnturn(fTimeElapsed);
	//MoveMeteo(m_GameTimer.GetTimeElapsed());
	for (int i = 0; i < METEOS; i++) {
		//if (!m_pSpaceship->GetBox().Intersects(m_ppGameObjects[i]->m_xmOOBB)) {
			//std::cout << "´«";
			//m_ppGameObjects[i]->Replace(m_pPlayer->GetPosition());
		//}

	}
	
	XMFLOAT3 p_pos = m_pSpaceship->GetPosition();
	for (int i = 0; i < METEOS; i++) { 
		XMFLOAT3 m_pos = m_ppMeteoObjects[i]->GetPosition();
		float dist = Vector3::Length(Vector3::Subtract(m_pos, p_pos));
		if (dist > 1000) {
			m_ppMeteoObjects[i]->SetPosition(urdPos(dree) + p_pos.x, urdPos(dree) + p_pos.y, urdPos(dree) + p_pos.z);
			//m_ppMeteoObjects[i]->SetScale(urdScale(dree), urdScale(dree), urdScale(dree));
			m_ppMeteoObjects[i]->SetMovingDirection(XMFLOAT3(urdPos(dree), urdPos(dree), urdPos(dree)));
		}
		m_ppMeteoObjects[i]->Animate(fTimeElapsed, NULL); 
	}

	CheckMeteoByPlayerCollisions();
	CheckObjectByBulletCollisions();
	//CheckEnemyByBulletCollisions();
}
