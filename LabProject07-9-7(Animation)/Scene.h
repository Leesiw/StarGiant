//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"
#include "UI.h"
#include "Boss.h"
#include "God.h"


#define MAX_LIGHTS						16 

#define POINT_LIGHT						1
#define SPOT_LIGHT						2
#define DIRECTIONAL_LIGHT				3

#define MAX_PARTICLES					50
#define MAX_CIRCLE_PARTICLES			4
#define MAX_MAGIC_CIRCLE_PARTICLES		3


#define MAX_HEAL_PARTICLES				20


#define MAX_FIRE				2



constexpr char BLACKHOLEMETEOR = 16;

struct LIGHT
{
	XMFLOAT4							m_xmf4Ambient;
	XMFLOAT4							m_xmf4Diffuse;
	XMFLOAT4							m_xmf4Specular;
	XMFLOAT3							m_xmf3Position;
	float 								m_fFalloff;
	XMFLOAT3							m_xmf3Direction;
	float 								m_fTheta; //cos(m_fTheta)
	XMFLOAT3							m_xmf3Attenuation;
	float								m_fPhi; //cos(m_fPhi)
	bool								m_bEnable;
	int									m_nType;
	float								m_fRange;
	float								padding;
};										
										
struct LIGHTS							
{										
	LIGHT								m_pLights[MAX_LIGHTS];
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights;
};

class CScene
{
public:
    CScene();
    ~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildDefaultLightsAndMaterials();
	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void BuildLobbyObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void BuildUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void BuildUIInside(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void BuildBoss(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void BuildGod(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void BuildInsideObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12DescriptorHeap* descriptor_heap);
	void ReleaseObjects();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	ID3D12DescriptorHeap* GetDescriptor() { return(m_pd3dCbvSrvDescriptorHeap); }


	void CheckObjectByBulletCollisions();
	void CheckMEByObjectCollisions(); 
	int CheckSitCollisions(); 
	bool CheckMascotCollisions(); 


	bool ProcessInput(UCHAR *pKeysBuffer);
    void AnimateObjects(float fTimeElapsed);

	void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);
	void OnPostRender(ID3D12GraphicsCommandList* pd3dCommandList);

    void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);

	void RenderUI(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	void RenderUIInside(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);



	void ReleaseUploadBuffers();

	CPlayer								*m_pPlayer[3] = {};

	// ����
	void RespawnMeteor(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, SPAWN_METEO_INFO m_info);
	void RespawnBossMeteor(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, SPAWN_METEO_INFO m_info);

	void TransformMeteor(METEO_INFO m_info);
	void TransformMeteor(XMFLOAT3 m_pos[]);

	void setBlackholePos(XMFLOAT3 m_pos);


	virtual void CheckBoomSprite(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void AddDieSprite(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, XMFLOAT3 Position, int Target);
	virtual void AddDieSprite(XMFLOAT3 Position, int Target);

	void setParticleStart(int cnt, XMFLOAT3 tarPos);
	void sethealParticleStart(int cnt, XMFLOAT3 tarPos);
	void setParticleStarts(int cnt, XMFLOAT3 tarPos, int num);






	void clear();



protected:
	ID3D12RootSignature					*m_pd3dGraphicsRootSignature = NULL;

	static ID3D12DescriptorHeap			*m_pd3dCbvSrvDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorNextHandle;

public:
	static void CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, UINT nRootParameter, bool bAutoIncrement);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorNextHandle() { return(m_d3dCbvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_d3dCbvGPUDescriptorNextHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorNextHandle() { return(m_d3dSrvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorNextHandle() { return(m_d3dSrvGPUDescriptorNextHandle); }

	float								m_fElapsedTime = 0.0f;

	float								m_fbosscutTime = 0.0f;
	float								m_fredbosscutTime = 0.0f;


	float								m_biteTime = 0.0f;




	float								m_eTime = 0.0f;


	int									m_nGameObjects = 0;
	CGameObject							**m_ppGameObjects = NULL;

	int									m_nHierarchicalGameObjects = 0;
	CGameObject							**m_ppHierarchicalGameObjects = NULL;


	Boss* m_ppBoss = NULL;
	God* m_ppGod = NULL;




	CMeteorObject						*m_ppMeteorObjects[METEOS];
	CMeteorObject						*m_ppBossMeteorObjects[BOSSMETEOS];

	CBlackHoleMeteorObject				*m_BlackholeMeteorObjects[BLACKHOLEMETEOR];

	CJewelObject* m_ppJewel;


	CMeteorObject* landob;


	CEnemyObject						*m_ppEnemies[ENEMIES];

	CBulletObject						*m_ppEnemyBullets[ENEMY_BULLETS];

	CMissileObject						*m_ppEnemyMissiles[ENEMY_BULLETS];

	CMascotObject* m_ppMascot;

	CBlackHole* m_ppBlackhole;

	CUI								*m_ppUI[UI_CNT];

	CUI								*m_ppLobbyUI[1];


	CUI								*m_ppUIInside[UI_INSIDE_CNT];

	CUI* m_ppUIName[3];

	CSpriteObject					* m_ppSprite[SPRITE_CNT];
	std::list<CSpriteObject*> m_pDieSprite;


	int									m_nShaders = 0;
	CShader								**m_ppShaders = NULL;

	CSkyBox								*m_pSkyBox = NULL;
	CHeightMapTerrain					*m_pTerrain = NULL;


	CFireObject* m_pFire[MAX_FIRE] = {};

	CParticleObject* m_pParticle[MAX_PARTICLES] = {};
	ChealParticleObject* m_phealParticle[MAX_HEAL_PARTICLES] = {};

	CFlameParticleObject* m_pFlameParticle[MAX_PARTICLES] = {};
	CSkullObject* m_pSkull[MAX_CIRCLE_PARTICLES] = {};
	CLineObject* m_pline[MAX_CIRCLE_PARTICLES] = {};
	CMagicCircleObject* m_pMagicCircle[MAX_CIRCLE_PARTICLES] = {};
	CMagicCircleObject* m_pMagicCircle2[MAX_CIRCLE_PARTICLES] = {};





	CDepthRenderShader* m_pDepthRenderShader = NULL;

	CShadowMapShader* m_pShadowShader = NULL;
	CTextureToViewportShader* m_pShadowMapToViewport = NULL;





	LIGHT								*m_pLights = NULL;
	LIGHT* m_pInsideLights = NULL;

	int									m_nLights = 0;
	int									m_nInsideLights = 0;

	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource						*m_pd3dcbLights = NULL;
	LIGHTS								*m_pcbMappedLights = NULL;


	//�̺�Ʈ �ٿ�� �ڽ� 
	BoundingBox xm_SitAABB[4];
	BoundingBox xm_MapAABB;
	XMFLOAT3 m_LookCamera[4];
	XMFLOAT3 m_SitPos[4];

	BoundingBox xm_Mascot;

	int m_nScenePlayer = 1;
	bool b_Inside = false;
	bool isDiedSprite = false;

	int m_temp = 2;



};

class CInsideScene : public CScene
{
public:
	CInsideScene() {};
	~CInsideScene() {};

	//이렇게 옳겨놓고싶음.
	//BoundingBox xm_SitAABB[4];
	//BoundingBox xm_MapAABB;
};