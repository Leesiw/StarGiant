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
std::uniform_real_distribution<float> urdPos(-1000, 1000);
std::uniform_real_distribution<float> urdScale(0, 10);

void CScene::BuildObjects()
{
	m_nGameObjects = 200;
	m_ppGameObjects = new CGameObject * [m_nGameObjects];


	//CGameObject* meteoModel = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/meteo.bin");
	MeteoObject* meteo = NULL;


	meteo = new MeteoObject();
	meteo->SetPosition(-40, 0, 70);
	meteo->SetScale(10, 10, 10);
	meteo->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	meteo->TurnSpeed();
	meteo->mesh = true;
	meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.188906f, 0.977625f, 0.315519f }, XMFLOAT3{ 1.402216f, 1.458820f, 1.499708f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
	m_ppGameObjects[0] = meteo;

	meteo = new MeteoObject();
	meteo->SetPosition(0, 0, 0);
	meteo->SetScale(10, 10, 10);
	meteo->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	meteo->TurnSpeed();
	meteo->mesh = true;
	meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.188906f, 0.977625f, 0.315519f }, XMFLOAT3{ 1.402216f, 1.458820f, 1.499708f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
	m_ppGameObjects[1] = meteo;

	for (int i = 2; i < 99; ++i) {
		meteo = new MeteoObject();
		meteo->SetPosition(urdPos(dree), urdPos(dree), urdPos(dree));
		//meteo->SetOOBB();
		//meteo->SetScale(urdScale(dree), urdScale(dree), urdScale(dree));
		meteo->SetScale(10, 10, 10);

		meteo->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
		meteo->TurnSpeed();
		meteo->mesh = true;
		meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.188906f, 0.977625f, 0.315519f }, XMFLOAT3{ 1.402216f, 1.458820f, 1.499708f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
		m_ppGameObjects[i] = meteo;
		m_ppGameObjects[i]->UpdateTransform(NULL);
	}

	for (int i = 99; i < 200; ++i) {
		meteo = new MeteoObject();
		
		//meteo->SetOOBB();
		meteo->SetScale(urdScale(dree), urdScale(dree), urdScale(dree));
		meteo->boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.000628f, -0.011224f, -0.003297f }, XMFLOAT3{ 0.057967f, 0.050386f, 0.055706f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
		//meteo->UpdateBoundingBox();
		//meteo->boundingbox = meteo->m_xmOOBB;
		meteo->SetPosition(urdPos(dree), urdPos(dree), urdPos(dree));
		meteo->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
		meteo->mesh = true;
		meteo->UpdateBoundingBox();
		
		meteo->TurnSpeed();


		m_ppGameObjects[i] = meteo;
		m_ppGameObjects[i]->UpdateTransform(NULL);
	}


	//CGameObject *meteoModel = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/meteo.bin");
	//MeteoObject* meteo = NULL;

	//meteo = new MeteoObject();
	//meteo->SetChild(meteoModel, true);
	//meteo->SetPosition(+230.0f, 0.0f, 260.0f);
	//meteo->SetScale(20.0f, 20.0f, 20.0f);
	////meteo->Rotate(0.0f, 90.0f, 0.0f);
	//meteo->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	//meteo->TurnSpeed();
	//meteo->SetMovingSpeed(20.0f);
	//m_ppGameObjects[0] = meteo;

	//CGameObject* meteoModel1 = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/meteo.bin");

	//meteo = new MeteoObject();
	//meteo->SetChild(meteoModel1, true);
	//meteo->SetPosition(-170.0f, -30.0f, 120.0f);
	//meteo->SetScale(10.0f, 10.0f, 10.0f);
	//meteo->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));

	//meteo->TurnSpeed();
	//meteo->SetMovingSpeed(40.5f);

	//m_ppGameObjects[1] = meteo;


	//CGameObject *meteoriteModel = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/meteorite.bin");
	//MeteoriteObject* meteorite = NULL;

	//meteorite = new MeteoriteObject();
	//meteorite->SetChild(meteoriteModel, true);
	//meteorite->SetPosition(-200.0f, 40.0f, 320.0f);
	//meteorite->SetScale(8.5f, 8.5f, 8.5f);
	//meteorite->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	//meteorite->TurnSpeed();
	//meteorite->SetMovingSpeed(100.5f);
	//m_ppGameObjects[2] = meteorite;

	//CGameObject* meteoriteModel1 = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/meteorite.bin");
	//meteorite = new MeteoriteObject();
	//meteorite->SetChild(meteoriteModel1, true);
	//meteorite->SetPosition(190.0f, -50.0f, 150.0f);
	//meteorite->SetScale(4.5f, 4.5f, 4.5f);
	//meteorite->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	//meteorite->TurnSpeed();
	//meteorite->SetMovingSpeed(100.5f);
	//m_ppGameObjects[3] = meteorite;

	//
	//meteorite = new MeteoriteObject();
	//meteorite->SetChild(meteoriteModel1, true);
	//meteorite->SetPosition(0.0f, -50.0f, 150.0f);
	//meteorite->SetScale(4.5f, 4.5f, 4.5f);
	//meteorite->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	//meteorite->TurnSpeed();
	//meteorite->SetMovingSpeed(100.5f);
	//m_ppGameObjects[4] = meteorite;

	//
	//for (int i = 5; i < 7; ++i) {
	//	meteorite = new MeteoriteObject();
	//	meteorite->SetChild(meteoriteModel1, true);
	//	meteorite->SetPosition(0.0f, -50.0f + i*10, 150.0f);
	//	meteorite->SetScale(4.5f, 4.5f, 4.5f);
	//	meteorite->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	//	meteorite->TurnSpeed();
	//	meteorite->SetMovingSpeed(100.5f);
	//	m_ppGameObjects[i] = meteorite;
	//}

}
void CScene::BuildObjects2()
{
	m_nGameObjects = 1;
	m_ppGameObjects = new CGameObject * [m_nGameObjects];

	m_ppGameObjects[0] = new CGameObject;
	m_ppGameObjects[0]->SetPosition(0.0f, 0.0f, -110.0f);
	m_ppGameObjects[0]->SetScale(100.0f, 100.0f, 100.0f);
}
void CScene::ReleaseObjects()
{

	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Release();
		delete[] m_ppGameObjects;
	}
}

bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CScene::CheckObjectByPlayerCollisions()
{

	for (int i = 0; i < m_nGameObjects; ++i) {
		//if (m_pPlayer->m_pChild->m_xmOOBB.Intersects(m_ppGameObjects[i]->m_pChild->m_xmOOBB))
		if (m_pPlayer->HierarchyIntersects(m_ppGameObjects[i]))
		{
			XMFLOAT3 xmf3Sub = m_ppGameObjects[i]->GetPosition();
			xmf3Sub = Vector3::Subtract(m_pPlayer->GetPosition(), xmf3Sub);
			xmf3Sub = Vector3::Normalize(xmf3Sub);

			xmf3Sub.y = 0;
			//std::cout << "충돌! ";

			m_pPlayer->Move(xmf3Sub, false);
		}

		//m_ppGameObjects[i]->m_pChild->aabb = BoundingBox(m_ppGameObjects[i]->m_pChild->GetPosition(), XMFLOAT3(10.0f, 10.0f, 10.0f));
		//m_pPlayer->m_pChild->aabb = BoundingBox(m_pPlayer->m_pChild->GetPosition(), XMFLOAT3(10.0f, 10.0f, 10.0f));

		//if (m_ppGameObjects[i]->m_pChild->aabb.Intersects(m_pPlayer->m_pChild->aabb)) {
		//	std::cout << "충돌! ";
		//}



		/*if (m_pPlayer->m_xmOOBB.Intersects(Objects->m_ppObjects[i]->m_xmOOBB)) {

			XMFLOAT3 xmf3Sub = Objects->m_ppObjects[i]->GetPosition();
			xmf3Sub = Vector3::Subtract(m_pPlayer->GetPosition(), xmf3Sub);
			xmf3Sub = Vector3::Normalize(xmf3Sub);

			xmf3Sub.y = 0;
			std::cout << "충돌! ";
			m_pPlayer->Move(xmf3Sub, false);
		}*/
	}
}

int ant = 0;

void CScene::CheckObjectByBulletCollisions()
{

	CBulletObject** ppBullets = ((CAirplanePlayer*)m_pPlayer)->m_ppBullets;
	for (int i = 0; i < m_nGameObjects; ++i)
	{
		for (int j = 0; j < BULLETS; j++)
		{

			ppBullets[j]->UpdateBoundingBox();


			if (ppBullets[j]->m_bActive) {
				//cout << ppBullets[0]->m_xmOOBB.Center.z;

				if (ppBullets[j]->HierarchyIntersects(m_ppGameObjects[i]))
					cout << "Dd";
				if (ant == 0) {
					cout << "\n" << ppBullets[j]->GetPosition().z << "- dd\n";

				}
				//m_ppGameObjects[i]->m_pChild->m_xmOOBB

				//cout<<"z다  : "<< m_ppGameObjects[0]->m_pChild->m_xmOOBB.Center.z;
				//cout << "z다 : " << ppBullets[j]->m_pChild->m_xmOOBB.Center.z;




				XMFLOAT3 hi = { 0,0,0 };
				/*
				m_ppGameObjects[i]->m_pChild->aabb = BoundingBox(m_ppGameObjects[i]->GetPosition(), XMFLOAT3(10.0f, 10.0f, 10.0f));
				ppBullets[j]->m_pChild->aabb = BoundingBox(ppBullets[j]->GetPosition(), XMFLOAT3(20.0f, 20.0f, 20.0f));
				if (ant == 0) {
					// cout << "\ny : " << ppBullets[j]->m_pChild->aabb.Center.x<< ppBullets[j]->m_pChild->aabb.Center.y<< ppBullets[j]->m_pChild->aabb.Center.z;
					ant++;
				}
				if (m_ppGameObjects[i]->m_pChild->aabb.Intersects(ppBullets[j]->m_pChild->aabb)) {
					//if (ppBullets[j]->HierarchyIntersects(m_ppGameObjects[i])) {
					//cout << "충돌\n";
					m_ppGameObjects[i]->hp -= 3;
					ppBullets[j]->Reset();
					//}
				}*/
			}
		}
	}
}

void CScene::MoveMeteo(float fTimeElapsed)
{
	for (int i = 0; i < m_nGameObjects; ++i) {
		m_ppGameObjects[i]->UpdateSpeed(fTimeElapsed);
		m_ppGameObjects[i]->UpdateRespawn(m_pPlayer->GetBox(), m_pPlayer->GetPosition(), m_pPlayer->GetLook());
	}

}

void CScene::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	//m_ppGameObjects[1]->turnturn(fTimeElapsed);
	//MoveMeteo(m_GameTimer.GetTimeElapsed());
	for (int i = 0; i < m_nGameObjects; i++) {
		if (!m_pPlayer->GetBox().Intersects(m_ppGameObjects[i]->m_xmOOBB)) {
			//std::cout << "눈";
			//m_ppGameObjects[i]->Replace(m_pPlayer->GetPosition());
		}
	}

	for (int i = 0; i < m_nGameObjects; i++) { m_ppGameObjects[i]->Animate(fTimeElapsed, NULL); }

	CheckObjectByPlayerCollisions();
	//CheckObjectByBulletCollisions();
}
