//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

int g_myid;

ID3D12DescriptorHeap* CScene::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorNextHandle;

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 6;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.55f, 0.55f, 0.55f, 1.0f);

	m_pLights[0].m_bEnable = false;
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 300.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.3f, 0.8f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(230.0f, 330.0f, 480.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
	m_pLights[1].m_bEnable = true;
	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 500.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff = 8.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);
	m_pLights[3].m_bEnable = false;
	m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights[3].m_fRange = 600.0f;
	m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
	m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[3].m_xmf3Position = XMFLOAT3(550.0f, 330.0f, 530.0f);
	m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[3].m_fFalloff = 8.0f;
	m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));
	m_pLights[4].m_bEnable = false;
	m_pLights[4].m_nType = POINT_LIGHT;
	m_pLights[4].m_fRange = 200.0f;
	m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.3f, 0.3f, 1.0f);
	m_pLights[4].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[4].m_xmf3Position = XMFLOAT3(600.0f, 250.0f, 700.0f);
	m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
	/// //GodRay Light
	m_pLights[5].m_nType = POINT_LIGHT;
	m_pLights[5].m_fRange = 200.0f;
	m_pLights[5].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[5].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[5].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[5].m_xmf3Position = XMFLOAT3(425.0f, 250.0f, 640.0f);
	m_pLights[5].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 362); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot()

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	BuildDefaultLightsAndMaterials();

	m_nScenePlayer = 1;

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);



	XMFLOAT3 xmf3Scale(8.0f, 2.0f, 8.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.3f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 257, 257, xmf3Scale, xmf4Color);

	m_nHierarchicalGameObjects = 13;
	m_ppHierarchicalGameObjects = new CGameObject * [m_nHierarchicalGameObjects];

	CLoadedModelInfo* pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/planet.bin", NULL);
	m_ppHierarchicalGameObjects[0] = new CAngrybotObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pAngrybotModel, 1);
	m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[0]->SetPosition(1000.0f, 1000.0f, 1000.0f);
	m_ppHierarchicalGameObjects[0]->SetScale(100.f, 100.f, 100.f);
	if (pAngrybotModel) delete pAngrybotModel;

	CLoadedModelInfo* pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Monster.bin", NULL);
	m_ppHierarchicalGameObjects[1] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMonsterModel, 1);
	m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[1]->SetPosition(430.0f, m_pTerrain->GetHeight(430.0f, 700.0f), 700.0f);
	m_ppHierarchicalGameObjects[1]->SetScale(3.0f, 3.0f, 3.0f);
	m_ppHierarchicalGameObjects[2] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMonsterModel, 1);
	m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	m_ppHierarchicalGameObjects[2]->SetPosition(400.0f, m_pTerrain->GetHeight(400.0f, 720.0f), 720.0f);
	m_ppHierarchicalGameObjects[2]->SetScale(3.0f, 3.0f, 3.0f);
	m_ppHierarchicalGameObjects[3] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMonsterModel, 1);
	m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	m_ppHierarchicalGameObjects[3]->SetPosition(380.0f, m_pTerrain->GetHeight(380.0f, 750.0f), 750.0f);
	m_ppHierarchicalGameObjects[3]->SetScale(3.0f, 3.0f, 3.0f);
	if (pMonsterModel) delete pMonsterModel;

	CLoadedModelInfo* pHumanoidModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Humanoid.bin", NULL);
	m_ppHierarchicalGameObjects[4] = new CHumanoidObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHumanoidModel, 1);
	m_ppHierarchicalGameObjects[4]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[4]->SetPosition(400.0f, m_pTerrain->GetHeight(400.0f, 670.0f), 670.0f);
	m_ppHierarchicalGameObjects[4]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[4]->SetScale(5.0f, 5.0f, 5.0f);

	m_ppHierarchicalGameObjects[5] = new CHumanoidObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHumanoidModel, 1);
	m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	m_ppHierarchicalGameObjects[5]->SetPosition(410.0f, m_pTerrain->GetHeight(410.0f, 660.0f), 660.0f);
	m_ppHierarchicalGameObjects[5]->SetScale(5.0f, 5.0f, 5.0f);
	if (pHumanoidModel) delete pHumanoidModel;

	CLoadedModelInfo* pEthanModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Ethan.bin", NULL);
	m_ppHierarchicalGameObjects[6] = new CEthanObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEthanModel, 2);
	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetTrackWeight(0, 0.8f);
	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetTrackWeight(1, 0.2f);
	m_ppHierarchicalGameObjects[6]->SetPosition(350.0f, m_pTerrain->GetHeight(350.0f, 670.0f), 670.0f);

	CLoadedModelInfo* pZebraModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Red.bin", NULL);
	m_ppHierarchicalGameObjects[7] = new CEnemyObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pZebraModel, 1);
	m_ppHierarchicalGameObjects[7]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[7]->SetPosition(280.0f, m_pTerrain->GetHeight(280.0f, 640.0f), 620.0f);
	m_ppHierarchicalGameObjects[7]->SetScale(1.0f, 1.0f, 1.0f);
	if (pZebraModel) delete pZebraModel;

	CLoadedModelInfo* pLionModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/arrow.bin", NULL);
	m_ppHierarchicalGameObjects[8] = new CLionObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLionModel, 1);
	m_ppHierarchicalGameObjects[8]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[8]->SetPosition(420.0f, 420.0f, 420.0f);
	m_ppHierarchicalGameObjects[8]->SetScale(100.0f, 100.0f, 100.0f);
	m_ppHierarchicalGameObjects[9] = new CLionObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLionModel, 1);
	m_ppHierarchicalGameObjects[9]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[9]->SetPosition(420.0f,250.0f, 640.0f);
	m_ppHierarchicalGameObjects[9]->SetScale(1500.0f, 15.0f, 1500.0f);
	m_ppHierarchicalGameObjects[10] = new CLionObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLionModel, 1);
	m_ppHierarchicalGameObjects[10]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[10]->SetPosition(250.0f, m_pTerrain->GetHeight(250.0f, 600.0f), 600.0f);
	m_ppHierarchicalGameObjects[10]->SetScale(100.0f, 100.0f, 100.0f);
	m_ppHierarchicalGameObjects[11] = new CLionObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLionModel, 1);
	m_ppHierarchicalGameObjects[11]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[11]->SetPosition(270.0f, m_pTerrain->GetHeight(270.0f, 620.0f), 620.0f);
	m_ppHierarchicalGameObjects[11]->SetScale(100.0f, 100.0f, 100.0f);
	if (pLionModel) delete pLionModel;

	CLoadedModelInfo* pMeteoModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/meteo2.bin", NULL);
	m_ppHierarchicalGameObjects[12] = new CMeteorObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMeteoModel, 1);
	m_ppHierarchicalGameObjects[12]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[12]->SetPosition(330.0f, m_pTerrain->GetHeight(330.0f, 590.0f) + 20.0f, 590.0f);
	m_ppHierarchicalGameObjects[12]->SetScale(3.0f, 3.0f, 3.0f);
	if (pMeteoModel) delete pMeteoModel;

	//=====================================
	for (int i = 0; i < METEOS / 2; ++i) {
		CLoadedModelInfo* pMeteoModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/meteo2.bin", NULL);
		m_ppMeteorObjects[i] = new CMeteorObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMeteoModel, 1);
		m_ppMeteorObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppMeteorObjects[i]->SetPosition(330.0f + i * 10, m_pTerrain->GetHeight(330.0f, 590.0f) + 20.0f, 590.0f);
		m_ppMeteorObjects[i]->SetScale(3.0f, 3.0f, 3.0f);
		if (pMeteoModel) delete pMeteoModel;
	}

	for (int i = METEOS / 2; i < METEOS; ++i) {
		CLoadedModelInfo* pMeteoModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Rock.bin", NULL);
		m_ppMeteorObjects[i] = new CMeteorObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMeteoModel, 1);
		m_ppMeteorObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppMeteorObjects[i]->SetPosition(330.0f + i * 10, m_pTerrain->GetHeight(330.0f, 590.0f) + 20.0f, 590.0f);
		m_ppMeteorObjects[i]->SetScale(3.0f, 3.0f, 3.0f);
		if (pMeteoModel) delete pMeteoModel;
	}
	//=====================================
	for (int i = 0; i < PLASMACANNON_ENEMY; ++i) {
		CLoadedModelInfo* pEnemyModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/AlienDestroyer.bin", NULL);
		m_ppEnemies[i] = new CEnemyObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyModel, 1);
		m_ppEnemies[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppEnemies[i]->SetPosition(330.0f + i * 10, m_pTerrain->GetHeight(330.0f, 590.0f) + 20.0f, 590.0f);
		m_ppEnemies[i]->SetScale(5.0f, 5.0f, 5.0f);
		if (pEnemyModel) delete pEnemyModel;
	}
	for (int i = PLASMACANNON_ENEMY; i < PLASMACANNON_ENEMY + LASER_ENEMY; ++i) {
		CLoadedModelInfo* pEnemyModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/BioTorpedo.bin", NULL);
		m_ppEnemies[i] = new CEnemyObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyModel, 1);
		m_ppEnemies[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppEnemies[i]->SetPosition(330.0f + i * 10, m_pTerrain->GetHeight(330.0f, 590.0f) + 20.0f, 590.0f);
		m_ppEnemies[i]->SetScale(60.0f, 60.0f, 60.0f);
		if (pEnemyModel) delete pEnemyModel;
	}
	for (int i = PLASMACANNON_ENEMY + LASER_ENEMY; i < ENEMIES; ++i) {
		CLoadedModelInfo* pEnemyModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/AlienFighter.bin", NULL);
		m_ppEnemies[i] = new CEnemyObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyModel, 1);
		m_ppEnemies[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppEnemies[i]->SetPosition(330.0f + i * 10, m_pTerrain->GetHeight(330.0f, 590.0f) + 20.0f, 590.0f);
		m_ppEnemies[i]->SetScale(10.0f, 10.0f, 10.0f);
		if (pEnemyModel) delete pEnemyModel;
	}
	//=====================================
	for (int i = 0; i < ENEMY_BULLETS; ++i) {
		CLoadedModelInfo* pEnemyModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Cube2.bin", NULL);
		m_ppEnemyBullets[i] = new CBulletObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyModel, 1);
		m_ppEnemyBullets[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppEnemyBullets[i]->SetPosition(330.0f + i * 10, m_pTerrain->GetHeight(330.0f, 590.0f) + 20.0f, 590.0f);
		m_ppEnemyBullets[i]->SetScale(1.0f, 1.0f, 1.0f);
		m_ppEnemyBullets[i]->SetMovingSpeed(1000.f);
		m_ppEnemyBullets[i]->is_fire = false;
		m_ppEnemyBullets[i]->is_enemy_fire = true;
		if (pEnemyModel) delete pEnemyModel;
	}

	for (int i = 0; i < ENEMY_BULLETS; ++i) {
		CLoadedModelInfo* pEnemyModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Rocket10.bin", NULL);
		m_ppEnemyMissiles[i] = new CMissileObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyModel, 1);
		m_ppEnemyMissiles[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppEnemyMissiles[i]->SetPosition(330.0f + i * 10, m_pTerrain->GetHeight(330.0f, 590.0f) + 20.0f, 590.0f);
		m_ppEnemyMissiles[i]->SetScale(1.0f, 1.0f, 1.0f);
		if (pEnemyModel) delete pEnemyModel;
	}
	//=====================================

	for (int i = BOSSMETEOS / 2; i < METEOS; ++i) {
		CLoadedModelInfo* pMeteoModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Rock4.bin", NULL);
		m_ppBossMeteorObjects[i] = new CMeteorObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMeteoModel, 1);
		m_ppBossMeteorObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppBossMeteorObjects[i]->SetPosition(330.0f + i * 10, m_pTerrain->GetHeight(330.0f, 590.0f) + 20.0f, 590.0f);
		m_ppBossMeteorObjects[i]->SetScale(3.0f, 3.0f, 3.0f);
		if (pMeteoModel) delete pMeteoModel;
	}

	//=====================================
	for (int i = 0; i < SPRITE_CNT; ++i) {
		m_ppSprite[i] = new CSpriteObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, XMFLOAT3(0,0,0), XMFLOAT3(0.f,0.f,0.f), static_cast<int>(SpriteType::EnemyBoom));
		m_ppSprite[i]->SetPosition(435.f, 250.f, 640.f);
		m_ppSprite[i]->CreateShaderVariable(pd3dDevice, pd3dCommandList);
		//AddDieSprite(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, XMFLOAT3(435.f, 250.f, 640.f));
	}
	//=====================================XMFLOAT3(425.0f, 250.0f, 640.0f);
	BuildBoss(pd3dDevice, pd3dCommandList);
	BuildUI(pd3dDevice, pd3dCommandList);


	m_nShaders = 2;
	m_ppShaders = new CShader * [2];

	CEthanObjectsShader* pEthanObjectsShader = new CEthanObjectsShader();
	pEthanObjectsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEthanModel, m_pTerrain);

	m_ppShaders[0] = pEthanObjectsShader;

	CGodRayShader* pGodRayShader = new CGodRayShader();
	pGodRayShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEthanModel, m_pTerrain);

	m_ppShaders[1] = pGodRayShader;

	if (pEthanModel) delete pEthanModel;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::BuildUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	//CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 300);

	float fx = FRAME_BUFFER_WIDTH / 2;
	float fy = FRAME_BUFFER_HEIGHT / 2;

	m_ppUI[0] = new CUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, static_cast<int>(UIType::CROSSHAIR), 2, 2, 0);
	m_ppUI[0]->SetPosition(fx, fy, 0.0f);

	m_ppUI[1] = new CUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, static_cast<int>(UIType::SCRATCH), 1, 1, 0);
	m_ppUI[1]->SetPosition(fx + 10.0f, fy, 10.0f);

	for (int i = 2; i < ENEMIES + 2; ++i) { //  UI_CNT
		m_ppUI[i] = new CUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, static_cast<int>(UIType::HP), 15, 2, 0);
		m_ppUI[i]->SetPosition(fx + 10.0f + 20.0f * i, fy, 10.0f);
	}
	m_ppUI[ENEMIES + 2] = new CUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, static_cast<int>(UIType::HP), 30, 4, 0);
	m_ppUI[ENEMIES + 2]->SetPosition(0.0f, 0.0f, 0.0f);

}

void CScene::BuildUIInside(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int i = 0; i < UI_INSIDE_CNT; ++i) {
		m_ppUIInside[i] = new CUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, static_cast<int>(UIInsideType::FSIT), 15, 15, 0);
	}
	

	
	m_ppUIName[0] = new CUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, static_cast<int>(UIInsideType::NAME_1), 5, 5, 0);
	m_ppUIName[1] = new CUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, static_cast<int>(UIInsideType::NAME_2), 5, 5, 0);
	m_ppUIName[2] = new CUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, static_cast<int>(UIInsideType::NAME_3), 5, 5, 0);

	
}

void CScene::BuildBoss(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CLoadedModelInfo* pBossModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Red.bin", NULL);
	m_ppBoss = new Boss();
	m_ppBoss->BossObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBossModel);
	m_ppBoss->SetPosition(0.0f + 10, 250.0f, 640.0f);
	m_ppBoss->SetScale(100.0f, 100.0f, 100.0f);
	if (pBossModel) delete pBossModel;

	CLoadedModelInfo* pLandModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Slime.bin", NULL);
	landob = new CMeteorObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLandModel, 1);
	landob->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	landob->SetPosition(m_ppBoss->GetPosition().x, m_ppBoss->GetPosition().y - 10.0f, m_ppBoss->GetPosition().z);
	landob->SetScale(50.0f, 50.0f, 50.0f);
	if (pLandModel) delete pLandModel;


}

void CScene::BuildInsideObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12DescriptorHeap* descriptor_heap)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	//CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 176); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot() // �Ʒ������� �� �Լ��� �κ��̴�

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = descriptor_heap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * 1);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * 1);

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	BuildDefaultLightsAndMaterials();

	m_nScenePlayer = 3;

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	XMFLOAT3 xmf3Scale(8.0f, 2.0f, 8.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.3f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 257, 257, xmf3Scale, xmf4Color);

	m_nHierarchicalGameObjects = 2;
	m_ppHierarchicalGameObjects = new CGameObject * [m_nHierarchicalGameObjects];

	//����� ���� 
	CLoadedModelInfo* pInsideModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/InsideShip.bin", NULL);
	m_ppHierarchicalGameObjects[0] = new CInsideShipObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pInsideModel, 1);
	m_ppHierarchicalGameObjects[0]->SetPosition(425.0f, 250.f - 30.0f, 590.0f);
	m_ppHierarchicalGameObjects[0]->SetScale(9.0f, 9.0f, 9.0f);
	if (pInsideModel) delete pInsideModel;

	CLoadedModelInfo* pSeatModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/InsideSeat.bin", NULL);
	m_ppHierarchicalGameObjects[1] = new CInsideShipObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSeatModel, 1);
	m_ppHierarchicalGameObjects[1]->SetPosition(390.0f, 250.f - 30.0f, 590.0f);
	m_ppHierarchicalGameObjects[1]->SetScale(1.0f, 1.0f, 1.0f);
	if (pSeatModel) delete pSeatModel;

	//=====================================
	CLoadedModelInfo* pMascotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Doggy_V0.bin", NULL);
	m_ppMascot = new CMascotObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMascotModel, 1);
	m_ppMascot->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppMascot->SetPosition(530.219f, 250.f - 30.0f, 593.263f);
	m_ppMascot->SetScale(10.0f, 10.0f, 10.0f);
	if (pMascotModel) delete pMascotModel;

	//=====================================
	
	//���Ƿ� �����ϴ� �ٿ���ڽ� (���� ���� Height���� ���� �浹�˻�� �ٲٰ����) 
	b_Inside = true;
	xm_MapAABB = BoundingBox(XMFLOAT3(m_ppHierarchicalGameObjects[0]->GetPosition().x, m_ppHierarchicalGameObjects[0]->GetPosition().y, m_ppHierarchicalGameObjects[0]->GetPosition().z + 90.f), XMFLOAT3(125.f, 100.0f, 110.0f));

	xm_SitAABB[0] = BoundingBox(XMFLOAT3(417.f, 224.f, 737.f), XMFLOAT3(4.f, 10.0f, 8.0f)); //LEFT
	xm_SitAABB[1] = BoundingBox(XMFLOAT3(505.f, 224.f, 676.f), XMFLOAT3(4.f, 10.0f, 8.0f)); //UP
	xm_SitAABB[2] = BoundingBox(XMFLOAT3(416.f, 224.f, 620.f), XMFLOAT3(4.f, 10.0f, 8.0f)); //RIGHT
	xm_SitAABB[3] = BoundingBox(XMFLOAT3(404.f, 224.f, 677.f), XMFLOAT3(4.f, 10.0f, 8.0f)); //CENTER


	xm_Mascot = BoundingBox(XMFLOAT3(530.219f, 250.f - 30.0f, 593.263f), XMFLOAT3(10.0f, 10.0f, 10.0f));

	m_LookCamera[0] = XMFLOAT3(0.0f, -0.0f, 1.0f);  //LEFT
	m_LookCamera[1] = XMFLOAT3(1.0f, -0.0f, 0.0f);	  //UP
	m_LookCamera[2] = XMFLOAT3(0.0f, 0.0f, -1.0f);   //RIGHT
	m_LookCamera[3] = XMFLOAT3(1.0f, -0.0f, 0.0f); //CENTER

	m_SitPos[0] = XMFLOAT3(416.0f, 230.0f, 734.0f); //LEFT
	m_SitPos[1] = XMFLOAT3(505.0f, 230.0f, 673.0f); //UP
	m_SitPos[2] = XMFLOAT3(416.0f, 230.0f, 613.0f); //RIGHT
	m_SitPos[3] = XMFLOAT3(405.0f, 230.0f, 673.0);  //CENTER


	for (int i = 0; i < METEOS; ++i) {
		m_ppMeteorObjects[i] = NULL;
	}
	for (int i = 0; i < METEOS; ++i) {
		m_ppBossMeteorObjects[i] = NULL;
	}

	

	BuildUIInside(pd3dDevice, pd3dCommandList);

	/*m_nShaders = 1;
	m_ppShaders = new CShader * [m_nShaders];

	CGodRayShader* pRayShader = new CGodRayShader();
	pRayShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEthanModel, m_pTerrain);

	m_ppShaders[0] = pRayShader;*/

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();

	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Release();
		delete[] m_ppGameObjects;
	}

	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			m_ppShaders[i]->Release();
		}
		delete[] m_ppShaders;
	}

	if (m_pTerrain) delete m_pTerrain;
	if (m_pSkyBox) delete m_pSkyBox;

	if (m_ppHierarchicalGameObjects)
	{
		for (int i = 0; i < m_nHierarchicalGameObjects; i++) if (m_ppHierarchicalGameObjects[i]) m_ppHierarchicalGameObjects[i]->Release();
		delete[] m_ppHierarchicalGameObjects;
	}

	if (m_ppMeteorObjects)
	{
		for (int i = 0; i < METEOS; i++) if (m_ppMeteorObjects[i]) m_ppMeteorObjects[i]->Release();
		delete[] m_ppMeteorObjects;
	}	
	
	if (m_ppBossMeteorObjects)
	{
		for (int i = 0; i < BOSSMETEOS; i++) if (m_ppBossMeteorObjects[i]) m_ppBossMeteorObjects[i]->Release();
		delete[] m_ppBossMeteorObjects;
	}
	
	if (m_ppEnemies)
	{
		for (int i = 0; i < ENEMIES; i++) {
			if (m_ppEnemies[i]) m_ppEnemies[i]->Release();
		}
		delete[] m_ppEnemies;
	}

	if (m_ppEnemyBullets)
	{
		for (int i = 0; i < ENEMY_BULLETS; i++) if (m_ppEnemyBullets[i]) m_ppEnemyBullets[i]->Release();
		delete[] m_ppEnemyBullets;
	}

	if (m_ppEnemyMissiles)
	{
		for (int i = 0; i < ENEMY_BULLETS; i++) if (m_ppEnemyMissiles[i]) m_ppEnemyMissiles[i]->Release();
		delete[] m_ppEnemyMissiles;
	}

	if (m_pPlayer[g_myid]) {
		m_pPlayer[g_myid]->ReleaseUploadBuffers();
		delete[]m_pPlayer[g_myid];
	}


	if (m_ppUI)
	{
		for (int i = 0; i < UI_CNT; i++) if (m_ppUI[i]) m_ppUI[i]->Release();
		delete[] m_ppUI;
	}

	if (m_ppUIInside)
	{
		for (int i = 0; i < UI_INSIDE_CNT; i++) if (m_ppUIInside[i]) m_ppUIInside[i]->Release();
		delete[] m_ppUIInside;
	}

	if (m_ppUIName)
	{
		for (int i = 0; i < 3; i++) if (m_ppUIName[i]) m_ppUIName[i]->Release();
		delete[] m_ppUIName;
	}
	if (m_ppSprite)
	{
		for (int i = 0; i < SPRITE_CNT; i++) if (m_ppSprite[i]) m_ppSprite[i]->Release();
		delete[] m_ppSprite;
	}


	if (landob)
	{
		landob->Release();
		delete[] landob;
	}

	if (m_ppBoss)
	{
		m_ppBoss->Release();
		delete[] m_ppBoss;
	}

	if(m_ppMascot)
	{
		m_ppMascot->Release();
		delete[] m_ppMascot;
	}

	/*if (!m_pDieSprite.empty()) {
		for (std::list<CSpriteObject*>::iterator i = m_pDieSprite.begin(); i != m_pDieSprite.end();)
		{
			if (!(*i)->is_Alive)
			{
				delete (*i);
				i = m_pDieSprite.erase(i);
			}
			else i++;
		}
		
	}*/
	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;
}


ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[15];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtAlbedoTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7; //t7: gtxtSpecularTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8; //t8: gtxtNormalTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 9; //t9: gtxtMetallicTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 10; //t10: gtxtEmissionTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 11; //t11: gtxtEmissionTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 12; //t12: gtxtEmissionTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 1; //t1: gtxtTerrainBaseTexture
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[9].NumDescriptors = 1;
	pd3dDescriptorRanges[9].BaseShaderRegister = 2; //t2: gtxtTerrainDetailTexture
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[10].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[10].NumDescriptors = 1;
	pd3dDescriptorRanges[10].BaseShaderRegister = 14; //t14: gtxtUITexture
	pd3dDescriptorRanges[10].RegisterSpace = 0;
	pd3dDescriptorRanges[10].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[11].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[11].NumDescriptors = 1;
	pd3dDescriptorRanges[11].BaseShaderRegister = 15; //t15: gtxtGODNoiseTexture
	pd3dDescriptorRanges[11].RegisterSpace = 0;
	pd3dDescriptorRanges[11].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[12].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[12].NumDescriptors = 1;
	pd3dDescriptorRanges[12].BaseShaderRegister = 16; //t16: gtxtGODNoiseTexture2
	pd3dDescriptorRanges[12].RegisterSpace = 0;
	pd3dDescriptorRanges[12].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[13].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[13].NumDescriptors = 1;
	pd3dDescriptorRanges[13].BaseShaderRegister = 17; //t17: gtxtSpriteTexture
	pd3dDescriptorRanges[13].RegisterSpace = 0;
	pd3dDescriptorRanges[13].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[14].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[14].NumDescriptors = 1;
	pd3dDescriptorRanges[14].BaseShaderRegister = 18; //t18: gtxtSprite2Texture2
	pd3dDescriptorRanges[14].RegisterSpace = 0;
	pd3dDescriptorRanges[14].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[21];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 33;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[11].Descriptor.ShaderRegister = 7; //Skinned Bone Offsets
	pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 8; //Skinned Bone Transforms
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[14].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[14].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[15].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[15].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[10]);
	pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[16].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[16].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[11]); //
	pd3dRootParameters[16].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[17].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[17].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[17].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[12]); //
	pd3dRootParameters[17].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[18].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[18].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[18].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[13]); //Sprite texture
	pd3dRootParameters[18].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[19].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	//pd3dRootParameters[19].Constants.Num32BitValues = 2;
	pd3dRootParameters[19].Constants.ShaderRegister = 9; //plus SpriteTransform
	pd3dRootParameters[19].Constants.RegisterSpace = 0;
	pd3dRootParameters[19].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[20].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[20].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[20].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[14]); //Sprite2 texture
	pd3dRootParameters[20].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256�� ���
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nHierarchicalGameObjects; i++) m_ppHierarchicalGameObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < METEOS; i++)if (m_ppMeteorObjects[i] != NULL) m_ppMeteorObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < ENEMIES; i++)if (m_ppEnemies[i])	m_ppEnemies[i]->ReleaseUploadBuffers();
	if (m_pPlayer[g_myid])m_pPlayer[g_myid]->ReleaseUploadBuffers();


}

void CScene::RespawnMeteor(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, SPAWN_METEO_INFO m_info)
{
	if (m_ppMeteorObjects[m_info.id])
	{
		m_ppMeteorObjects[m_info.id]->ResetScale();
		m_ppMeteorObjects[m_info.id]->SetScale(m_info.scale.x, m_info.scale.y, m_info.scale.z);
		m_ppMeteorObjects[m_info.id]->SetPosition(m_info.pos);
		m_ppMeteorObjects[m_info.id]->m_xmf3MovingDirection = m_info.direction;
	}
}

void CScene::RespawnBossMeteor(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, SPAWN_METEO_INFO m_info)
{

}

void CScene::TransformMeteor(METEO_INFO m_info)
{
	if (m_info.id < METEOS) {
		if (m_ppMeteorObjects[m_info.id]) {
			m_ppMeteorObjects[m_info.id]->SetPosition(m_info.pos);
		}
	}
	else {
		if (m_ppBossMeteorObjects[m_info.id - METEOS]) {
			m_ppBossMeteorObjects[m_info.id - METEOS]->SetPosition(m_info.pos);
		}
	}
}


void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY:
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = 0;
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nRootParameter, bool bAutoIncrement)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorNextHandle;
	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		int nTextureType = pTexture->GetTextureType();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetTexture(i);
			D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

			pTexture->SetRootArgument(i, (bAutoIncrement) ? (nRootParameter + i) : nRootParameter, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	return(d3dSrvGPUDescriptorHandle);
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		break;
	default:
		break;
	}
	return(false);
}

void CScene::CheckObjectByBulletCollisions()
{
	//메쉬 생성시 바운딩 박스 생기도록 해야함 
	for (int i = 0; i < m_nHierarchicalGameObjects; ++i)
	{
		m_ppHierarchicalGameObjects[i]->m_pChild->aabb = BoundingBox(m_ppHierarchicalGameObjects[i]->GetPosition(), XMFLOAT3(10.0f, 10.0f, 10.0f));
		if (b_Inside)m_pPlayer[g_myid]->m_pChild->aabb = BoundingBox(m_pPlayer[0]->GetPosition(), XMFLOAT3(5.0f, 5.0f, 5.0f));
		else m_pPlayer[0]->m_pChild->aabb = BoundingBox(m_pPlayer[0]->GetPosition(), XMFLOAT3(5.0f, 5.0f, 5.0f));

		if (m_ppHierarchicalGameObjects[i]->m_pChild->aabb.Intersects(m_pPlayer[0]->m_pChild->aabb)) {
			cout << i << " - �浹\n";
		}

	}

}

void CScene::CheckMEByObjectCollisions()
{
	//if(m_ppHierarchicalGameObjects[0])std::cout << m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->m_ppSkinnedMeshes[0]->m_xmOOBB.Center.x << std::endl;
	/*for (int i = 0; i < m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->m_nSkinnedMeshes; i++)
	{
		if (m_pPlayer->HierarchyIntersects(m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->m_ppSkinnedMeshes[i]))
		{
			XMFLOAT3 xmf3Sub = m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->m_ppSkinnedMeshes[i]->GetPosition();
				xmf3Sub = Vector3::Subtract(m_pPlayer->GetPosition(), xmf3Sub);

				xmf3Sub = Vector3::Normalize(xmf3Sub);

				float fLen = 5.0f;
				xmf3Sub = Vector3::ScalarProduct(xmf3Sub, fLen, false);
				xmf3Sub.y = 0.0f;
				m_pPlayer->Move(xmf3Sub, true);
		}
	}*/
	if (b_Inside) {
		if (!m_pPlayer[g_myid]->aabb.Intersects(xm_MapAABB))
		{
			std::cout << "내부" << std::endl;
			XMFLOAT3 xmf3Sub = Vector3::Subtract(m_pPlayer[g_myid]->GetPosition(), xm_MapAABB.Center);
			xmf3Sub = Vector3::Normalize(xmf3Sub);

			float fLen = 15.0f; //속도로 들어갈 수 있게 바꿔야함 
			xmf3Sub = Vector3::ScalarProduct(XMFLOAT3(-xmf3Sub.x, -xmf3Sub.y, -xmf3Sub.z), fLen, false);
			xmf3Sub = XMFLOAT3(xmf3Sub.x, 0.0f, xmf3Sub.z);
			m_pPlayer[g_myid]->Move(xmf3Sub, true);

		}

	}

}

int CScene::CheckSitCollisions()
{	//의자 충돌검사 
	if (b_Inside) {
		for (int i = 0; i < 4; i++) {
			if (m_pPlayer[g_myid]->aabb.Intersects(xm_SitAABB[i]))
			{
				std::cout << i << "bound" << std::endl;
				//i번째 방향으로 카메라돌리고 앉게하기 
				if (((CTerrainPlayer*)m_pPlayer[g_myid])->motion != AnimationState::SIT) {
					((CTerrainPlayer*)m_pPlayer[g_myid])->motion = AnimationState::SIT;
					std::cout << "시점전환앉기\n";
					m_pPlayer[g_myid]->SetPosition(m_SitPos[i]);
					//m_pPlayer[g_myid]->SetLook(m_LookCamera[i]); > 회전 연동 시 문제 생김 + right 벡터같은거 상관 없이 행렬 자체 건드려서 플레이어 일그러짐
					m_pPlayer[g_myid]->SetSitState(true);
					return i;
				}
				else
				{
					std::cout << "서기";
					((CTerrainPlayer*)m_pPlayer[g_myid])->motion = AnimationState::IDLE;

					//m_pPlayer[g_myid]->SetLook(m_LookCamera[i]);
					m_pPlayer[g_myid]->SetSitState(false);
					return i;
				}
			}
		}
	}

	return -1;
}

bool CScene::CheckMascotCollisions()
{
	if (m_pPlayer[g_myid]->aabb.Intersects(xm_Mascot))
	{
		return true;
	}
}

bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	if (m_ppHierarchicalGameObjects)
	{
	}


	//CheckObjectByBulletCollisions();
	CheckMEByObjectCollisions();

	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Animate(fTimeElapsed);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->AnimateObjects(fTimeElapsed);


	if (m_pLights)
	{
		if (b_Inside) {
			m_pLights[1].m_xmf3Position = m_pPlayer[g_myid]->GetPosition();
			m_pLights[1].m_xmf3Direction = m_pPlayer[g_myid]->GetLookVector();
		}
		else {
			m_pLights[1].m_xmf3Position = m_pPlayer[0]->GetPosition();
			m_pLights[1].m_xmf3Direction = m_pPlayer[0]->GetLookVector();
		}
	}

	/*
	for (int i = 0; i < ENEMIES; ++i) {
		if (m_ppEnemies[i] && m_ppEnemies[i]->isAlive){// && !m_ppEnemies[i]->isUpdate) {
			m_ppEnemies[i]->AI(fTimeElapsed, m_pPlayer[0]->GetLook(), m_pPlayer[0]->GetPosition());

		}
		else {
			if (m_ppEnemies[i]) {
				m_ppEnemies[i]->isUpdate = false;
			}
		}
	}*/

	for (int i = 0; i < UI_CNT; i++)
	{
		if (m_ppUI[i])
		{
			//m_ppUI[i]->SetPosition(m_pPlayer[0]->GetPosition().x, m_pPlayer[0]->GetPosition().y, m_pPlayer[0]->GetPosition().z);
		}
	}
}


//chrono::steady_clock::time_point stateStartTime; //q

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights

	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain && !b_Inside) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->Render(pd3dCommandList, pCamera);
	if (m_nShaders>1) m_ppShaders[1]->SetPlayerPosition(m_pPlayer[0]->GetPosition());


	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	XMFLOAT3 xmf3CameraLook = pCamera->GetLookVector();
	XMFLOAT3 xmf3Position = Vector3::Add(xmf3CameraPosition, Vector3::ScalarProduct(xmf3CameraLook, 50.0f, false));


	XMFLOAT3 xmf3Position2 = Vector3::Add(xmf3CameraPosition, Vector3::ScalarProduct(xmf3CameraLook, 10.0f, false));
	XMFLOAT3 tar = { 1000.0f,1000.0f,1000.0f };
	if (m_pPlayer[0]->curMissionType == MissionType::GO_PLANET) {
		m_ppHierarchicalGameObjects[9]->SetPosition(xmf3Position2);
		m_ppHierarchicalGameObjects[9]->SetLookAt(tar, XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_ppHierarchicalGameObjects[9]->Rotate(90, 0, 0);
		m_ppHierarchicalGameObjects[9]->SetScale(10, 5, 10);
	}

	if (m_pPlayer[0]->curMissionType == MissionType::FIND_BOSS) {
		m_ppHierarchicalGameObjects[9]->SetPosition(xmf3Position2);
		m_ppHierarchicalGameObjects[9]->SetLookAt(m_ppBoss->GetPosition(), XMFLOAT3(0.0f, 0.5f, 0.0f));
		m_ppHierarchicalGameObjects[9]->Rotate(90, 0, 0);
		m_ppHierarchicalGameObjects[9]->SetScale(10, 5, 10);
	}




	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{
		if (m_ppHierarchicalGameObjects[i])
		{
			m_ppHierarchicalGameObjects[i]->Animate(m_fElapsedTime);
			if (!m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController) m_ppHierarchicalGameObjects[i]->UpdateTransform(NULL);
			m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}

	for (int i = 0; i < METEOS; i++)
	{
		if (m_ppMeteorObjects[i])
		{
			m_ppMeteorObjects[i]->Animate(m_fElapsedTime);
			if (!m_ppMeteorObjects[i]->m_pSkinnedAnimationController) m_ppMeteorObjects[i]->UpdateTransform(NULL);
			m_ppMeteorObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}

	for (int i = 0; i < METEOS; i++)
	{
		if (m_ppBossMeteorObjects[i])
		{
			m_ppBossMeteorObjects[i]->Animate(m_fElapsedTime);
			if (!m_ppBossMeteorObjects[i]->m_pSkinnedAnimationController) m_ppBossMeteorObjects[i]->UpdateTransform(NULL);
			m_ppBossMeteorObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}

	

	for (int i = 0; i < ENEMIES; i++)
	{
		if (m_ppEnemies[i] && m_ppEnemies[i]->isAlive)
		{
			m_ppEnemies[i]->Animate(m_fElapsedTime);
			if (!m_ppEnemies[i]->m_pSkinnedAnimationController) m_ppEnemies[i]->UpdateTransform(NULL);
			m_ppEnemies[i]->Render(pd3dCommandList, pCamera);
		}
	}



	for (int i = 0; i < ENEMY_BULLETS; i++)
	{
		if (m_ppEnemyBullets[i] && m_ppEnemyBullets[i]->m_bActive)
		{
			m_ppEnemyBullets[i]->Animate(m_fElapsedTime);
			if (!m_ppEnemyBullets[i]->m_pSkinnedAnimationController) m_ppEnemyBullets[i]->UpdateTransform(NULL);
			m_ppEnemyBullets[i]->Render(pd3dCommandList, pCamera);
		}
	}

	for (int i = 0; i < ENEMY_BULLETS; i++)
	{
		if (m_ppEnemyMissiles[i] && m_ppEnemyMissiles[i]->m_bActive)
		{
			//m_ppEnemyMissiles[i]->Animate(m_fElapsedTime);
			if (!m_ppEnemyMissiles[i]->m_pSkinnedAnimationController) m_ppEnemyMissiles[i]->UpdateTransform(NULL);
			m_ppEnemyMissiles[i]->Render(pd3dCommandList, pCamera);
		}
	}

	if (m_ppBoss) {
		m_ppBoss->Animate(m_fElapsedTime);
		if (!m_ppBoss->m_pSkinnedAnimationController) m_ppBoss->UpdateTransform(NULL);
		//m_ppBoss->Boss_Ai(m_ppBoss->GetState(), m_pPlayer[0]->GetPosition(), m_ppBoss->GetHP());
		m_ppBoss->ChangeAnimation(m_ppBoss->GetAnimation());
		m_ppBoss->Render(pd3dCommandList, pCamera); 
	}

	if (m_ppMascot) {
		m_ppMascot->Animate(m_fElapsedTime);
		if (!m_ppMascot->m_pSkinnedAnimationController) m_ppMascot->UpdateTransform(NULL);
		m_ppMascot->Render(pd3dCommandList, pCamera);
	}
	//우선 내부일떄 안써서 제한해놓음. 
	if (!b_Inside) {
		for (int i = 1; i < SPRITE_CNT; i++) {
			
				m_ppSprite[i]->Animate(m_fElapsedTime);
				//m_ppSprite[1]->SetfollowPosition(m_pPlayer[0]->GetPosition(), XMFLOAT3(30.0f, -60.0f, 0.0f), m_pPlayer[0]->GetLook());
				m_ppSprite[i]->UpdateShaderVariables(pd3dCommandList, m_ppSprite[i]->GetShaderVariables());
				if (m_ppSprite[i]->is_Alive) {
					m_ppSprite[i]->Render(pd3dCommandList, pCamera);
				}
		}
	}
	/*if (!m_pDieSprite.empty()) {
		for (std::list<CSpriteObject*>::iterator i = m_pDieSprite.begin(); i != m_pDieSprite.end();)
		{
			if (!(*i)->is_Alive)
			{
				delete (*i);
				i = m_pDieSprite.erase(i);
			}
			else i++;
		}
		int i = 0;
		for (auto object : m_pDieSprite) {
			object->Animate(m_fElapsedTime);
			object->UpdateShaderVariables(pd3dCommandList, object->GetShaderVariables());
			object->SetfollowPosition(m_pPlayer[0]->GetPosition(), XMFLOAT3(30.0f, -60.0f+20.0f*i, 0.0f), m_pPlayer[0]->GetLook());
			object->Render(pd3dCommandList, pCamera);
			i++;
		}
		
	}*/

}

void CScene::RenderUI(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	XMFLOAT3 xmf3CameraLook = pCamera->GetLookVector();
	XMFLOAT3 xmf3Position = Vector3::Add(xmf3CameraPosition, Vector3::ScalarProduct(xmf3CameraLook, 50.0f, false));

	
	m_ppUI[0]->SetPosition(xmf3Position); //static_cast<int>(UIType::CROSSHAIR)
	m_ppUI[0]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));

	m_ppUI[1]->SetPosition(xmf3Position); 
	m_ppUI[1]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_ppUI[1]->SetScale(80.0f, 80.0f, 80.0f);

	for (int i = 2; i < ENEMIES + 2; i++) // hp bar
	{
		if (m_ppEnemies[i- 2]&& m_ppEnemies[i - 2]->isAlive == true) {
			m_ppUI[i]->SetPosition(m_ppEnemies[i - 2]->GetPosition().x, m_ppEnemies[i - 2]->GetPosition().y + 10.0f, m_ppEnemies[i - 2]->GetPosition().z);
			m_ppUI[i]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 0.5f, 0.0f));
			m_ppUI[i]->HpbarUpdate(m_ppEnemies[i - 2]->GetPosition(), m_ppEnemies[i - 2]->GetMaxHp(), m_ppEnemies[i - 2]->GetcurHp());
		}
		else if (m_ppEnemies[i - 2] && m_ppEnemies[i - 2]->isAlive == false)
		{
			m_ppUI[i]->HpbarUpdate(m_ppEnemies[i - 2]->GetPosition(), m_ppEnemies[i - 2]->GetMaxHp(), 0);
		}
	}

	m_ppUI[ENEMIES + 2]->SetPosition(m_ppBoss->GetPosition().x, m_ppBoss->GetPosition().y + 10.0f, m_ppBoss->GetPosition().z);
	m_ppUI[ENEMIES + 2]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 0.5f, 0.0f));
	m_ppUI[ENEMIES + 2]->HpbarUpdate(m_ppBoss->GetPosition(), m_ppBoss->GetMaxHp(), m_ppBoss->GetcurHp());

	////hp테스트
	//m_ppUI[3]->SetPosition(m_ppEnemies[0]->GetPosition());
	//m_ppUI[3]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 0.5f, 0.0f));
	//m_ppUI[3]->HpbarUpdate(m_ppEnemies[0]->GetPosition(), 10, 1);
	//m_ppUI[5]->SetPosition(m_ppEnemies[0]->GetPosition());
	//m_ppUI[5]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 0.5f, 0.0f));
	//m_ppUI[5]->HpbarUpdate(m_ppEnemies[0]->GetPosition(), 10, 5);
	//m_ppUI[4]->SetPosition(m_ppEnemies[0]->GetPosition().x, m_ppEnemies[0]->GetPosition().y + 10.0f, m_ppEnemies[0]->GetPosition().z);
	//m_ppUI[4]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 0.5f, 0.0f));

	CCamera a = m_pPlayer[0]->GetCamera();
	
	if (m_ppUI[0]&& !(a.GetMode()== DRIVE_CAMERA))
	{
		m_ppUI[0]->Render(pd3dCommandList, pCamera);

	}

	if (m_ppUI[1] && m_ppBoss->CurMotion == BossAnimation::CLAW_ATTACT)
	{
		m_ppUI[1]->Render(pd3dCommandList, pCamera);
	}

	for (int i = 2; i < UI_CNT; i++)
	{
		if (m_ppUI[i])
		{
			m_ppUI[i]->Render(pd3dCommandList, pCamera);
		}
	}


	if (landob)landob->Render(pd3dCommandList, pCamera);
}

void CScene::RenderUIInside(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);


	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	XMFLOAT3 xmf3CameraLook = pCamera->GetLookVector();
	XMFLOAT3 xmf3Position = Vector3::Add(xmf3CameraPosition, Vector3::ScalarProduct(xmf3CameraLook, 50.0f, false));

	m_SitPos[0] = XMFLOAT3(416.0f, 230.0f, 734.0f); //LEFT
	m_SitPos[1] = XMFLOAT3(505.0f, 230.0f, 673.0f); //UP
	m_SitPos[2] = XMFLOAT3(416.0f, 230.0f, 613.0f); //RIGHT
	m_SitPos[3] = XMFLOAT3(405.0f, 230.0f, 673.0);  //CENTER

	m_ppUIInside[0]->SetPosition(m_SitPos[0].x, m_SitPos[0].y + 10.0f, m_SitPos[0].z);
	m_ppUIInside[1]->SetPosition(m_SitPos[1].x, m_SitPos[1].y + 10.0f, m_SitPos[1].z);
	m_ppUIInside[2]->SetPosition(m_SitPos[2].x, m_SitPos[2].y + 10.0f, m_SitPos[2].z);
	m_ppUIInside[3]->SetPosition(m_SitPos[3].x, m_SitPos[3].y + 10.0f, m_SitPos[3].z);


	m_ppUIName[0]->SetPosition(m_pPlayer[0]->GetPosition().x, m_pPlayer[0]->GetPosition().y + 15.0f, m_pPlayer[0]->GetPosition().z);
	m_ppUIName[1]->SetPosition(m_pPlayer[1]->GetPosition().x, m_pPlayer[1]->GetPosition().y + 15.0f, m_pPlayer[1]->GetPosition().z);
	m_ppUIName[2]->SetPosition(m_pPlayer[2]->GetPosition().x, m_pPlayer[2]->GetPosition().y + 15.0f, m_pPlayer[2]->GetPosition().z);


	for (int i = 0; i < UI_INSIDE_CNT; i++)
	{
		if (m_ppUIInside[i])
		{
			m_ppUIInside[i]->SetScale(2.0f, 2.0f, 2.0f);
			m_ppUIInside[i]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 0.5f, 0.0f));
			m_ppUIInside[i]->Render(pd3dCommandList, pCamera);
		}
	}

	for (int i = 0; i < 3; i++)
	{
		if (m_ppUIName[i])
		{
			m_ppUIName[i]->SetScale(1.0f, 1.0f, 1.0f);
			m_ppUIName[i]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 0.5f, 0.0f));
			m_ppUIName[i]->Render(pd3dCommandList, pCamera);
		}
	}
}


void CScene::AddDieSprite(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, XMFLOAT3 Position, int Target)
{//m_pDieSprite 의 함수를 발동하면, 그만큼 있다가 알아서 사라지게하기.
	/*CSpriteObject* m_pSpritdump = new CSpriteObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, XMFLOAT3(0, 0, 0), XMFLOAT3(0.f, 0.f, 0.f), static_cast<int>(SpriteType::EnemyBoom));
	m_pSpritdump->SetPosition(Position);
	m_pSpritdump->CreateShaderVariable(pd3dDevice, pd3dCommandList);
	m_pSpritdump->TargetNum=Target;
	m_pDieSprite.push_back(m_pSpritdump);*/


}

void CScene::AddDieSprite(XMFLOAT3 Position, int Target)
{
	CSpriteObject* pBoomObject = NULL;
	bool makeSprite = true;
	for (int i = 0; i < SPRITE_CNT; i++)if (m_ppSprite[i]->TargetNum == Target)makeSprite = false;
	if (makeSprite) {
		for (int i = 1; i < SPRITE_CNT; i++)
		{
			if (!m_ppSprite[i]->is_Alive)
			{
				pBoomObject = m_ppSprite[i];
				m_ppSprite[i]->is_Alive = true;
				m_ppSprite[i]->SpriteMode = static_cast<int>(SpriteType::EnemyBoom);
				m_ppSprite[i]->TargetNum = Target;
				break;
			}
		}
		if (pBoomObject)
		{
			pBoomObject->SetPosition(Position);
		}
	}

	

}

void CScene::CheckBoomSprite(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{//렉걸리는데 이렇게 쓰고싶음. 남겨놓음. 
	//bool needMake = false;
	//for (int i = 1; i < ENEMIES; i++)
	//{
	//	if (m_ppEnemies[i] && !m_ppEnemies[i]->isAlive)
	//	{
	//		needMake = true;
	//		if (!m_pDieSprite.empty()) {
	//			for (std::list<CSpriteObject*>::iterator t = m_pDieSprite.begin(); t != m_pDieSprite.end();)
	//			{
	//				if ((*t)->TargetNum == i)
	//				{//끝내기
	//					needMake = false;
	//				}
	//				t++;
	//			}
	//		}
	//		if(needMake)
	//			AddDieSprite(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_ppEnemies[i]->GetPosition(), i);
	//	}
	//}
}