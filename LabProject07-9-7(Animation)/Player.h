#pragma once

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#define BULLETS					50

#include "Object.h"
#include "Camera.h"
#include "Sound.h"

class CPlayer : public CGameObject
{
protected:
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fMaxVelocityY = 0.0f;
	float           			m_fFriction = 0.0f;

	//�ҷ�
	float						m_fFireDelayTime;
	float						m_fFireWaitingTime;

	LPVOID						m_pPlayerUpdatedContext = NULL;
	LPVOID						m_pCameraUpdatedContext = NULL;

	CCamera						*m_pCamera = NULL;

	bool						b_Issit = false;
	int pastcam = 0;
	float r = 0;

public:
	AnimationState motion;
	MissionType curMissionType;
	bool						isAlive = false;
	
public:
	CPlayer();
	virtual ~CPlayer();


	void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);
	int m_nScreenWidth; int m_nScreenHeight;
	void SetScreenSize(int width, int height) {
		m_nScreenWidth = width;
		m_nScreenHeight = height;
	};
	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT2 GetPositionXY() { return(XMFLOAT2{ m_xmf3Position.x, m_xmf3Position.y }); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }
	bool setting = false;

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }

	void SetScale(XMFLOAT3& xmf3Scale) { m_xmf3Scale = xmf3Scale; }
	void SetLook(XMFLOAT3 LookAt) { m_xmf3Look = LookAt; }

	void SetSitState(bool state) { b_Issit = state; }


	bool GetSitState() { return b_Issit; }

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }
	XMFLOAT3 GetYawPitchRoll() { return XMFLOAT3(m_fYaw,m_fPitch,m_fRoll); }

	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	virtual void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Rotate(float x, float y, float z, int mode = 0);
	void Rotate2(float x, float y, float z, int mode = 0);


	virtual void Update(float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	CCamera* OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual CCamera* ChangeToCutSceneCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual CCamera* ChangeToBeforeCamera(CCamera* pCamera, float fTimeElapsed);


	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	virtual void UpdateBoundingBox() { aabb.Center = m_xmf3Position; }
	bool HierarchyIntersects(CSkinnedMesh* pCollisionGameObject, bool isSecond = false);

	virtual XMFLOAT3 getSpritePos(int num) { return XMFLOAT3(0.f, 0.f, 0.f); }


public:
		CGameObject* m_pEngine[2] = {};


};

class CAirplanePlayer : public CPlayer
{
public:
	CAirplanePlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext=NULL);
	virtual ~CAirplanePlayer();

	void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);
	void ReleaseUploadBuffers();
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);

	CGameObject					*m_pMainRotorFrame = NULL;
	CGameObject					*m_pTailRotorFrame = NULL;
	
	CSpriteObject* m_pAirSprites[2];
	CFireObject* m_pEngineFlame[2];

	CGameObject** m_BulletObjects = NULL;
	CBulletObject* m_ppBullets[BULLETS];
	void FireBullet(CGameObject* pLockedObject);
	void SetBulletFromServer(BULLET_INFO bulletInfo);
	float						m_fBulletEffectiveRange = 300.0f;


	XMFLOAT3	player_info;
	XMFLOAT4	player_quaternion;
	bool							is_update = true;
	bool							is_update_q = true;
	short							hp = 100;
	short							max_hp = 100;
	virtual void OnPrepareAnimate();
	 void Animate(float fTimeElapsed);
	void SetModelSprite(CGameObject* Loot, CTexture* LootTexture, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	//virtual XMFLOAT3 getSpritePos(int num) { return m_pAirSprites[num]->GetPosition(); }
	short getHp(){ return hp; }



private:

public:
	//���� 
	void SetPlayerInfo(XMFLOAT3 p_pos) { player_info = p_pos; is_update = false; }
	void SetQuaternion(XMFLOAT4 p_quaternion) { player_quaternion = p_quaternion; is_update_q = false; }
	void UpdateOnServer(bool rotate_update = true);
	XMVECTOR GetQuaternion();

	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPrepareRender();
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};

class CSoundCallbackHandler : public CAnimationCallbackHandler
{
public:
	CSoundCallbackHandler() { }
	~CSoundCallbackHandler() { }

public:
	virtual void HandleCallback(void *pCallbackData, float fTrackPosition); 
};

class CTerrainPlayer : public CPlayer
{
public:
	CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext=NULL, int i = 0);
	virtual ~CTerrainPlayer();



	INSIDE_PLAYER_INFO			player_info;
	bool						is_update = true;
	

	PlayerType type;

	void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);
	void ReleaseUploadBuffers();
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
	void Animate(float fTimeElapsed);


	CSound* p_effectSound[static_cast<int>(PlayerSounds::COUNT)] = {};
public:
	//���� 
	void SetPlayerInfo(INSIDE_PLAYER_INFO p_info) { player_info = p_info; is_update = false; }
	void UpdateOnServer(bool rotate_update = true);

	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	virtual void OnCameraUpdateCallback(float fTimeElapsed);

	virtual void Move(ULONG nDirection, float fDistance, bool bVelocity = false);

	virtual void Update(float fTimeElapsed);

};

