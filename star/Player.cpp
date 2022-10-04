//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer()
{
	m_pCamera = NULL;

	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_fFireDelayTime = 0.2f;
	m_fFireWaitingTime = 0.0f;

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		//XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		//가속도 최대크기를 정해놓고, 1) 항상 가속도는 줄어든다. 2) 최대크기 이상으로 커지지 않음 . 을 구현해야 
		//will.y = (1 - 0.5) * will.y + 0.5 * (will.y+will.velocity_y*game_framework.frame_time)
		//velocity_y -> distance를 뜻함... 

		//m_xmf3Shift = Vector3::Add(XMFLOAT3(0, 0, 0), m_xmf3Shift, 0.95);
		if (dwDirection & DIR_FORWARD) m_xmf3Shift = Vector3::Add(m_xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) m_xmf3Shift = Vector3::Add(m_xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) Rotate(0.0f, 0.5f, 0.0f);
		if (dwDirection & DIR_LEFT) Rotate(0.0f, -0.5f, 0.0f);
		if (dwDirection & DIR_UP) m_xmf3Shift = Vector3::Add(m_xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) m_xmf3Shift = Vector3::Add(m_xmf3Shift, m_xmf3Up, -fDistance);

		if (dwDirection & DIR_STOP) SetVelocity({ 0,0,0 });


		Move(m_xmf3Shift, bUpdateVelocity);
	}


	//if (dwDirection)
	//{
	//	XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);


	//	if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
	//	if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
	//	if (dwDirection & DIR_RIGHT) Rotate(0.0f, 0.5f, 0.0f);
	////	if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);

	//	if (dwDirection & DIR_LEFT) Rotate(0.0f, -0.5f, 0.0f);
	//	//if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);

	//	if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
	//	if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

	//	Move(xmf3Shift, bUpdateVelocity);


	//}
}
//void CPlayer::Move(DWORD dwDirection, float fDistance, float ftime, bool bUpdateVelocity = false)
//{
//	if (dwDirection)
//	{
//		//XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
//		//가속도 최대크기를 정해놓고, 1) 항상 가속도는 줄어든다. 2) 최대크기 이상으로 커지지 않음 . 을 구현해야 
//		//will.y = (1 - 0.5) * will.y + 0.5 * (will.y+will.velocity_y*game_framework.frame_time)
//		//velocity_y -> distance를 뜻함... 
//
//		//m_xmf3Shift = Vector3::Add(XMFLOAT3(0, 0, 0), m_xmf3Shift, 0.95);
//		if (dwDirection & DIR_FORWARD) m_xmf3Shift = Vector3::Add(m_xmf3Shift, m_xmf3Look, fDistance);
//		if (dwDirection & DIR_BACKWARD) m_xmf3Shift = Vector3::Add(m_xmf3Shift, m_xmf3Look, -fDistance);
//		if (dwDirection & DIR_RIGHT) Rotate(0.0f, 0.5f, 0.0f);
//		if (dwDirection & DIR_LEFT) Rotate(0.0f, -0.5f, 0.0f);
//		if (dwDirection & DIR_UP) m_xmf3Shift = Vector3::Add(m_xmf3Shift, m_xmf3Up, fDistance);
//		if (dwDirection & DIR_DOWN) m_xmf3Shift = Vector3::Add(m_xmf3Shift, m_xmf3Up, -fDistance);
//
//		Move(m_xmf3Shift, bUpdateVelocity);
//	}
//}
//void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
//{
//	if (bUpdateVelocity)
//	{
//		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
//	}
//	else
//	{
//		m_xmf3Shift = Vector3::Add(XMFLOAT3(0, 0, 0), m_xmf3Shift, 0.95);
//		m_xmf3Position = Vector3::Add(m_xmf3Position, m_xmf3Shift);
//		m_pCamera->Move(m_xmf3Shift);
//	}
//}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		m_xmf3Shift = Vector3::Add(XMFLOAT3(0, 0, 0), m_xmf3Shift, 0.95);
		m_xmf3Velocity = Vector3::Add(XMFLOAT3(0, 0, 0), m_xmf3Velocity,0.97);
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		m_pCamera->Move(xmf3Shift);
	}
	//m_xmf3Shift = xmf3Shift;

	//if (bUpdateVelocity) m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	//else m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
}

void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA) || (nCurrentCameraMode == ATTACT_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Update(float fTimeElapsed)
{
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (nCurrentCameraMode == ATTACT_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	if (nCurrentCameraMode == ATTACT_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);

	m_pCamera->RegenerateViewMatrix();

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));

	if (m_fFireWaitingTime > 0.0f) 
		m_fFireWaitingTime -= fTimeElapsed;

	UpdateBoundingBox();

	UpdateEyesightBox();

}


CCamera *CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera *pNewCamera = NULL;
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			pNewCamera = new CFirstPersonCamera(m_pCamera);
			break;
		case THIRD_PERSON_CAMERA:
			pNewCamera = new CThirdPersonCamera(m_pCamera);
			break;
		case SPACESHIP_CAMERA:
			pNewCamera = new CSpaceShipCamera(m_pCamera);
			break;
		case ATTACT_CAMERA:
			pNewCamera = new CAttactCamera(m_pCamera);
			break;
	}
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));

		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}

	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4Transform._11 = m_xmf3Right.x; m_xmf4x4Transform._12 = m_xmf3Right.y; m_xmf4x4Transform._13 = m_xmf3Right.z;
	m_xmf4x4Transform._21 = m_xmf3Up.x; m_xmf4x4Transform._22 = m_xmf3Up.y; m_xmf4x4Transform._23 = m_xmf3Up.z;
	m_xmf4x4Transform._31 = m_xmf3Look.x; m_xmf4x4Transform._32 = m_xmf3Look.y; m_xmf4x4Transform._33 = m_xmf3Look.z;
	m_xmf4x4Transform._41 = m_xmf3Position.x; m_xmf4x4Transform._42 = m_xmf3Position.y; m_xmf4x4Transform._43 = m_xmf3Position.z;

	UpdateTransform(NULL);
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA) CGameObject::Render(pd3dCommandList, pCamera);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAirplanePlayer

CAirplanePlayer::CAirplanePlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	m_pCamera = ChangeCamera(/*SPACESHIP_CAMERA*/THIRD_PERSON_CAMERA, 0.0f);

//	CGameObject *pGameObject = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Apache.bin");
	CGameObject *pGameObject = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/ship.bin");

	pGameObject->Rotate(0, -90.0f, 0.0f);
	pGameObject->SetScale(15.5f, 15.5f, 15.5f);
	SetChild(pGameObject, true);

	OnInitialize();

	for (int i = 0; i < BULLETS; i++)
	{
		CGameObject* pBulletMesh = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Missile.bin");

		m_ppBullets[i] = new CBulletObject(m_fBulletEffectiveRange);
		m_ppBullets[i]->SetScale(5.0f, 5.0f, 5.0f);
		m_ppBullets[i]->SetChild(pBulletMesh, true);



		m_ppBullets[i]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_ppBullets[i]->SetRotationSpeed(360.0f);
		m_ppBullets[i]->SetMovingSpeed(160.0f);
		m_ppBullets[i]->SetActive(false);
		m_ppBullets[i]->OnInitialize();

		m_ppBullets[i]->UpdateBoundingBox();
	
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CAirplanePlayer::~CAirplanePlayer()
{
}

void CAirplanePlayer::OnInitialize()
{
//	m_pMainRotorFrame = FindFrame("rotor");
//	m_pTailRotorFrame = FindFrame("black_m_7");

	m_pMainRotorFrame = FindFrame("Rotor");
	m_pTailRotorFrame = FindFrame("Back_Rotor");
}

void CAirplanePlayer::Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent)
{
	if (m_pMainRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4Transform);
	}
	if (m_pTailRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4Transform);
	}

	for (int i = 0; i < BULLETS; i++)
	{
		if (m_ppBullets[i]->m_bActive) {
			m_ppBullets[i]->Animate(fTimeElapsed);				
		};
	}

	CPlayer::Animate(fTimeElapsed, pxmf4x4Parent);
}

void CAirplanePlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CPlayer::Render(pd3dCommandList, pCamera);

	for (int i = 0; i < BULLETS; i++) if (m_ppBullets[i]->m_bActive) m_ppBullets[i]->Render(pd3dCommandList, pCamera);
}

void CAirplanePlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
}

CCamera *CAirplanePlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			SetFriction(2.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(2.5f);
			SetMaxVelocityY(40.0f);
			m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case SPACESHIP_CAMERA:
			SetFriction(100.5f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(40.0f);
			SetMaxVelocityY(40.0f);
			m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case THIRD_PERSON_CAMERA:
			SetFriction(20.5f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(150.0f);
			SetMaxVelocityY(150.0f);
			m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 105.0f, -140.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		//default:

		case ATTACT_CAMERA:
			SetFriction(2.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(0.0f);
			SetMaxVelocityY(40.0f);
			m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 10.0f, 10.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;

		default:
			break;
	}

	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}





void CAirplanePlayer::FireBullet(CGameObject* pLockedObject)
{
	/*
		if (pLockedObject)
		{
			LookAt(pLockedObject->GetPosition(), XMFLOAT3(0.0f, 1.0f, 0.0f));
			OnUpdateTransform();
		}
	*/
	if (m_fFireWaitingTime > 0.0f)
		return;

	CBulletObject* pBulletObject = NULL;
	for (int i = 0; i < BULLETS; i++)
	{
		if (!m_ppBullets[i]->m_bActive)
		{
			pBulletObject = m_ppBullets[i];
			break;
		}
	}

	if (pBulletObject)
	{
		XMFLOAT3 xmf3Position = GetPosition();
		XMFLOAT3 xmf3Direction = GetLook();
		XMFLOAT3 xmf3Right = GetRight();
		XMFLOAT3 xmf3Up = GetUp();
		XMFLOAT3 xmf3FirePosition;
		xmf3FirePosition.x = xmf3Position.x;
		xmf3FirePosition.y = xmf3Position.y;
		xmf3FirePosition.z = xmf3Position.z;



		//xmf3FirePosition = Vector3::Add(Vector3::ScalarProduct(xmf3Right, -40.0f, false), Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 70.0f, false)));
		//xmf3Position = Vector3::Add(Vector3::ScalarProduct(xmf3Right, -40.0f, false), Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 70.0f, false)));

		//xmf3FirePosition = Vector3::Add(Vector3::ScalarProduct(xmf3Right, -40.0f, false), Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Up, -10.0f, false)));
		//xmf3FirePosition.x = xmf3FirePosition.x - 50;

		cout <<"x,y,z" << xmf3FirePosition.x<<"\t"<< xmf3FirePosition.y << "\t"<< xmf3FirePosition.z;


	//	XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 6.0f, false));

		pBulletObject->m_xmf4x4Transform = m_xmf4x4Transform;
		//pBulletObject->m_xmf4x4World = m_xmf4x4World;

		pBulletObject->Rotate(m_fPitch, m_fYaw, m_fRoll);
		pBulletObject->m_fPitch = m_fPitch; pBulletObject->m_fYaw = m_fYaw; pBulletObject->m_fRoll = m_fRoll;
		pBulletObject->SetFirePosition(xmf3FirePosition);
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetActive(true);
		pBulletObject->UpdateBoundingBox();

		//for (int i = 0; i < BULLETS; i++)
		//{
		//	m_ppBullets[i]->UpdateBoundingBox();
		//}

	


		m_fFireWaitingTime = m_fFireDelayTime * 1.0f;

		if (pLockedObject)
		{
			pBulletObject->m_pLockedObject = pLockedObject;
		}
	}
}