#pragma once

#define ASPECT_RATIO				(float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))

#define FIRST_PERSON_CAMERA			0x01
#define SPACESHIP_CAMERA			0x02
#define THIRD_PERSON_CAMERA			0x03
#define SIT_EVENT_CAMERA			0x08
#define DRIVE_CAMERA				0x04
#define ATTACT_CAMERA_C				0x05
#define ATTACT_CAMERA_L				0x06
#define ATTACT_CAMERA_R				0x07
#define CUT_SCENE_CAMERA			0x09




#include "3DDefinitions.h"
class Frustrum;

struct VS_CB_CAMERA_INFO
{
	XMFLOAT4X4						m_xmf4x4View;
	XMFLOAT4X4						m_xmf4x4Projection;
	XMFLOAT3						m_xmf3Position;
};

class CPlayer;

class CCamera
{
protected:
	XMFLOAT3						m_xmf3Position;
	XMFLOAT3						m_xmf3Right;
	XMFLOAT3						m_xmf3Up;
	XMFLOAT3						m_xmf3Look;

	float           				m_fPitch;
	float           				m_fRoll;
	float           				m_fYaw;

	int m_nScreenWidth;
	int m_nScreenHeight;


	XMFLOAT3						m_xmf3LookAtWorld;
	XMFLOAT3						m_xmf3Offset;
	float           				m_fTimeLag;

	D3D12_VIEWPORT					m_d3dViewport;
	D3D12_RECT						m_d3dScissorRect;

	CPlayer							*m_pPlayer = NULL;

	ID3D12Resource					*m_pd3dcbCamera = NULL;
	VS_CB_CAMERA_INFO				*m_pcbMappedCamera = NULL;

	float m_shakingTime = 0.0f;
public:
	CCamera();
	CCamera(CCamera *pCamera, int m_nScreenWidth, int m_nScreenHeight);
	virtual ~CCamera();

	void SetScreenSize(int width, int height) {
		m_nScreenWidth = width;
		m_nScreenHeight = height;
	};


	DWORD							m_nMode;
	XMFLOAT4X4						m_xmf4x4View;
	XMFLOAT4X4						m_xmf4x4Projection;

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	void GenerateViewMatrix();
	void GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up);
	void RegenerateViewMatrix();

	void GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle);

	void SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight, float fMinZ = 0.0f, float fMaxZ = 1.0f);
	void SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom);

	virtual void SetViewportsAndScissorRects(ID3D12GraphicsCommandList *pd3dCommandList);

	void SetPlayer(CPlayer *pPlayer) { m_pPlayer = pPlayer; }
	CPlayer *GetPlayer() { return(m_pPlayer); }

	void SetMode(DWORD nMode) { m_nMode = nMode; }
	DWORD GetMode() { return(m_nMode); }

	void SetPosition(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	XMFLOAT3& GetPosition() { return(m_xmf3Position); }

	void SetLookAtPosition(XMFLOAT3 xmf3LookAtWorld) { m_xmf3LookAtWorld = xmf3LookAtWorld; }
	XMFLOAT3& GetLookAtPosition() { return(m_xmf3LookAtWorld); }

	XMFLOAT3& GetRightVector() { return(m_xmf3Right); }
	XMFLOAT3& GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3& GetLookVector() { return(m_xmf3Look); }

	void SetRightVector(XMFLOAT3 xmf3Right) { m_xmf3Right = xmf3Right; }
	void SetUpVector(XMFLOAT3 xmf3Up) { m_xmf3Up = xmf3Up; }
	void SetLookVector(XMFLOAT3 xmf3Look) { m_xmf3Look = xmf3Look; }


	float& GetPitch() { return(m_fPitch); }
	float& GetRoll() { return(m_fRoll); }
	float& GetYaw() { return(m_fYaw); }

	void SetOffset(XMFLOAT3 xmf3Offset) { m_xmf3Offset = xmf3Offset; }
//	void SetOffset(XMFLOAT3 xmf3Offset) { m_xmf3Offset = xmf3Offset; m_xmf3Position.x += xmf3Offset.x; m_xmf3Position.y += xmf3Offset.y; m_xmf3Position.z += xmf3Offset.z; }
	XMFLOAT3& GetOffset() { return(m_xmf3Offset); }

	void SetTimeLag(float fTimeLag) { m_fTimeLag = fTimeLag; }
	float GetTimeLag() { return(m_fTimeLag); }

	XMFLOAT4X4 GetViewMatrix() { return(m_xmf4x4View); }
	XMFLOAT4X4 GetProjectionMatrix() { return(m_xmf4x4Projection); }
	D3D12_VIEWPORT GetViewport() { return(m_d3dViewport); }
	D3D12_RECT GetScissorRect() { return(m_d3dScissorRect); }

	virtual void Move(const XMFLOAT3& xmf3Shift) { m_xmf3Position.x += xmf3Shift.x; m_xmf3Position.y += xmf3Shift.y; m_xmf3Position.z += xmf3Shift.z; }
	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f) { }
	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed) { }
	virtual void SetLookAt(XMFLOAT3& xmf3LookAt) { }
	virtual void Shaking(float fShakeAmount, float fTimeElapsed);

	int SceneTimer = 0;
	bool CameraSence1(bool ON);

	XMFLOAT3 tarPos = { 0,0,0 };
	float dist = -10.0f;

	void SetTarget(XMFLOAT3 tarpos) { tarPos = tarpos; }
	void SetDist(float sdist) { dist = sdist; }

	bool canTurn = true;
	bool getTurn() { return canTurn; }
	bool canDolly = false;
	bool getDolly() { return canDolly; }
	bool m_bCameraShaking = false;
	float maxShakingTime = 1.f;

	bool canchange = true;
	bool getcanchange() { return canchange; }



	float fAnglenu = 0;
	bool endc = false;
	float fendtime = 0;

};

class CSpaceShipCamera : public CCamera
{
public:
	CSpaceShipCamera(CCamera *pCamera);
	virtual ~CSpaceShipCamera() { }

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
};

class CFirstPersonCamera : public CCamera
{
public:
	CFirstPersonCamera(CCamera *pCamera);
	virtual ~CFirstPersonCamera() { }

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
};

class CThirdPersonCamera : public CCamera
{
public:
	CThirdPersonCamera(CCamera *pCamera);
	virtual ~CThirdPersonCamera() { }

	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed);
	virtual void SetLookAt(XMFLOAT3& vLookAt);
};

class CAttactCamera : public CCamera
{
public:
	CAttactCamera(CCamera* pCamera);
	virtual ~CAttactCamera() { }

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);

};

class CDriveCamera : public CCamera
{
public:
	CDriveCamera(CCamera* pCamera);
	virtual ~CDriveCamera() { }

	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed);
	virtual void SetLookAt(XMFLOAT3& vLookAt);

};

class CSitEventCamera : public CCamera
{
public:
	CSitEventCamera(CCamera* pCamera);
	virtual ~CSitEventCamera() { }

	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed);
	virtual void SetLookAt(XMFLOAT3& vLookAt);
};

////////////////////////////////////////////////////////////////////////
class CFrustrumCamera : public CCamera
{
public:
	CFrustrumCamera(int renderWidth, int renderHeight, Vertex& positionIn, Vertex& lookAtIn, float zNearIn, float aFarin);
	virtual ~CFrustrumCamera() { }

	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed, UINT8* tKeys=NULL);
	virtual void SetLookAt(XMFLOAT3& vLookAt); //apply

	void SetupPerspective();

	//void UpdateMouse(int deltax, int deltay);
	void Render();
	void Info_Print();

	float Getfovy() { return fovy; };
	float Getaspect() { return aspect; };
	float GetzNear() { return zNear; };
	float GetzFar() { return zFar; };

	// Frustrum ����
	float fovy;
	float aspect;
	float zNear;
	float zFar;

	void UpdateFrustrum();
	Frustrum* frustrum;

};

//�ƾ� ���� ī�޶�
class CCutSceneCamera : public CCamera
{
public:


	CCutSceneCamera(CCamera* pCamera);
	virtual ~CCutSceneCamera() { }

	virtual void SetLookAt(XMFLOAT3& vLookAt);
	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed);
};
