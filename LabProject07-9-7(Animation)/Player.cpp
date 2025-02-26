//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "Scene.h"

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

	motion = AnimationState::IDLE;

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::SetChild(CGameObject* pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();

	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;

	}
	else
	{
		m_pChild = pChild;

	}
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
	if (dwDirection&&!b_Issit)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Rotate(float x, float y, float z, int mode)
{
	if (!b_Issit) {
		DWORD nCurrentCameraMode = m_pCamera->GetMode();
		if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
		{
			if (y != 0.0f) // && mode == 0) > 서버 클라 연동 시 회전 정보 필요
			{
				m_fYaw += y;
				if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
				if (m_fYaw < 0.0f) m_fYaw += 360.0f;
			}
			m_pCamera->Rotate(x, y, z);
			if (y != 0.0f)
			{
				XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
				m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
				m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
			}
		}

		else if (nCurrentCameraMode == DRIVE_CAMERA)
		{
			if (x != 0.0f)
			{
				m_fPitch += x;
			}
			if (y != 0.0f)
			{
				m_fYaw += y;
			
			}
			if (z != 0.0f)
			{
				m_fRoll += z;
			}

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

		else if (nCurrentCameraMode == ATTACT_CAMERA_C || nCurrentCameraMode == ATTACT_CAMERA_L || nCurrentCameraMode == ATTACT_CAMERA_R || nCurrentCameraMode == CUT_SCENE_CAMERA)
		{
			if (x != 0.0f)
			{
				m_fPitch += x;
				if (m_fPitch > +90.0f) { x -= (m_fPitch - 90.0f); m_fPitch = +90.0f; }
				if (m_fPitch < -90.0f) { x -= (m_fPitch + 90.0f); m_fPitch = -90.0f; }
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
				if (m_fRoll > +90.0f) { z -= (m_fRoll - 90.0f); m_fRoll = +90.0f; }
				if (m_fRoll < -90.0f) { z -= (m_fRoll + 90.0f); m_fRoll = -90.0f; }
			}

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
}

void CPlayer::Rotate2(float x, float y, float z, int mode)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();

	if (nCurrentCameraMode == ATTACT_CAMERA_C || nCurrentCameraMode == ATTACT_CAMERA_L || nCurrentCameraMode == ATTACT_CAMERA_R)
	{
		// 카메라 회전을 위한 Pitch, Yaw, Roll 값을 계산합니다.
		float fPitch = y;
		float fYaw = x;
		float fRoll = z ;

		// Pitch, Yaw, Roll 값이 0이 아닐 때만 회전합니다.
		if (fPitch != 0.0f || fYaw != 0.0f || fRoll != 0.0f)
		{
			// Pitch, Yaw, Roll 값을 적용하여 카메라를 회전시킵니다.
			m_pCamera->Rotate(fPitch, fYaw, fRoll);

			// 카메라의 방향 벡터를 업 벡터와 오른쪽 벡터로부터 재설정합니다.
			m_xmf3Look = m_pCamera->GetLookVector();
			m_xmf3Up = m_pCamera->GetUpVector();
			m_xmf3Right = m_pCamera->GetRightVector();
		}
	}
}

void CPlayer::Update(float fTimeElapsed)
{
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength) * fTimeElapsed * 30;
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength) * fTimeElapsed * 30;
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength) * fTimeElapsed * 30;

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (nCurrentCameraMode == DRIVE_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (nCurrentCameraMode == CUT_SCENE_CAMERA)	m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	//if (nCurrentCameraMode == ATTACT_CAMERA_L) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	//if (nCurrentCameraMode == ATTACT_CAMERA_C) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	//if (nCurrentCameraMode == ATTACT_CAMERA_R) m_pCamera->Update(m_xmf3Position, fTimeElapsed);



	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	if (nCurrentCameraMode == DRIVE_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	/*if (nCurrentCameraMode == ATTACT_CAMERA_L) m_pCamera->SetLookAt(m_xmf3Position);
	if (nCurrentCameraMode == ATTACT_CAMERA_C) m_pCamera->SetLookAt(m_xmf3Position);
	if (nCurrentCameraMode == ATTACT_CAMERA_R) m_pCamera->SetLookAt(m_xmf3Position);*/

	//if (nCurrentCameraMode == ATTACT_CAMERA_L) m_pCamera->SetLookAt(m_xmf3Position);


	m_pCamera->RegenerateViewMatrix();

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
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
		case DRIVE_CAMERA:
			pNewCamera = new CDriveCamera(m_pCamera);
			break;
		case ATTACT_CAMERA_C:
			pNewCamera = new CAttactCamera(m_pCamera);
			pNewCamera->m_nMode = ATTACT_CAMERA_C;
			break;
		case ATTACT_CAMERA_L:
			pNewCamera = new CAttactCamera(m_pCamera);
			pNewCamera->m_nMode = ATTACT_CAMERA_L;
			break;
		case ATTACT_CAMERA_R:
			pNewCamera = new CAttactCamera(m_pCamera);
			pNewCamera->m_nMode = ATTACT_CAMERA_R;
			break;

		case CUT_SCENE_CAMERA:
			pNewCamera = new CCutSceneCamera(m_pCamera);
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

CCamera* CPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	return(NULL);
}

CCamera* CPlayer::ChangeToCutSceneCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	//if (nCurrentCameraMode == nNewCameraMode) { cout << "같아서 안온거구나\n"; return(m_pCamera); }
	switch (nNewCameraMode)
	{
	case CUT_SCENE_CAMERA:
		cout << "CUT_SCENE_CAMERA\n";
		SetMaxVelocityXZ(0.0f);
		SetMaxVelocityY(0.0f);
		m_pCamera = OnChangeCamera(CUT_SCENE_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, float(m_nScreenWidth / float(m_nScreenHeight)), 60.0f);
		m_pCamera->SetViewport(0, 0, m_nScreenWidth, m_nScreenHeight, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, m_nScreenWidth, m_nScreenHeight);
		break;

	default:
		cout << "아무것도 안해요\n";
		break;
	}

	//m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);

}

CCamera* CPlayer::ChangeToBeforeCamera(CCamera* pCamera, float fTimeElapsed)
{
	m_pCamera = pCamera;
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, pCamera->GetOffset()));
	Update(fTimeElapsed);



	return(m_pCamera);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;

	m_xmf4x4ToParent = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4ToParent);
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA) CGameObject::Render(pd3dCommandList, pCamera);
	if (nCameraMode == DRIVE_CAMERA) CGameObject::Render(pd3dCommandList, pCamera);
	//if (nCameraMode == ATTACT_CAMERA) CGameObject::Render(pd3dCommandList, pCamera);

}

bool CPlayer::HierarchyIntersects(CSkinnedMesh* pCollisionGameObject, bool isSecond)
{
	//if (OOBB.Intersects(pCollisionGameObject->m_xmOOBB) && pCollisionGameObject)
	//	return true;
	//if (isSecond)
	//	return false;

	return this->HierarchyIntersects(pCollisionGameObject, true);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CAirplanePlayer::CAirplanePlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext)
{
	m_pCamera = ChangeCamera(/*SPACESHIP_CAMERA*/THIRD_PERSON_CAMERA, 0.0f);

	CLoadedModelInfo *pModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/CephalonShip.bin", NULL);
	//CLoadedModelInfo *pModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Doggy_V1.bin", NULL);
	
	SetChild(pModel->m_pModelRootObject, true);


	//==flame==================================================
	CFireShader* pFireShader = new CFireShader();
	pFireShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pFireShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pfireTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pfireTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"data/fire02.dds", 0); // fire mix color

	CTexture* pnoiseTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pnoiseTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"data/noise01.dds", 0);//fre material texture

	CTexture* palphaTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	palphaTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"data/alpha01.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, pfireTexture, 22, false);
	CScene::CreateShaderResourceViews(pd3dDevice, pnoiseTexture, 23, false);
	CScene::CreateShaderResourceViews(pd3dDevice, palphaTexture, 24, false);

	CMaterial* pFireMaterial = new CMaterial(3);
	pFireMaterial->SetTexture(pfireTexture, 0);
	pFireMaterial->SetTexture(pnoiseTexture, 1);
	pFireMaterial->SetTexture(palphaTexture, 2);

	pFireMaterial->SetShader(pFireShader);

	m_pEngine[0] = FindFrame("Feather_L");
	m_pEngine[1] = FindFrame("Feather_R");


	for (int i = 0; i < 2; i++) {
		m_pEngineFlame[i] = new CFireObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_pEngineFlame[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}

	m_pEngineFlame[0] = (CFireObject*)m_pEngine[0];
	m_pEngineFlame[1] = (CFireObject*)m_pEngine[1];

	m_pEngineFlame[0]->SetMaterial(0, pFireMaterial);
	m_pEngineFlame[1]->SetMaterial(0, pFireMaterial);



	//SetMaterial(0, pFireMaterial);


	//===================================================


	//cout << "child : " << this->m_pChild << endl;
	//cout << "m_pSibling : " << this->m_pChild->m_pSibling << endl;


	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 1, pModel);
	m_pSkinnedAnimationController->SetTrackEnable(0, true);
	/*m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_pSkinnedAnimationController->SetCallbackKeys(0, 1);*/

	for (int i = 0; i < BULLETS; i++)
	{
		CLoadedModelInfo* pBulletMesh = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/cube.bin", NULL);

		m_ppBullets[i] = new CBulletObject(m_fBulletEffectiveRange);
		m_ppBullets[i]->SetScale(0.5f, 0.5f, 1.0f);
		m_ppBullets[i]->SetChild(pBulletMesh->m_pModelRootObject, true);
		m_ppBullets[i]->SetMovingSpeed(2000.0f);
		m_ppBullets[i]->SetActive(false);

		//오브젝트 설정시 각각의 skinnedAnimationcontroller을 설정해주어야하는 거엿다.  
		m_ppBullets[i]->m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 1, pBulletMesh);
		m_ppBullets[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppBullets[i]->m_pSkinnedAnimationController->SetCallbackKeys(0, 1);

	}


	CreateShaderVariables(pd3dDevice, pd3dCommandList);


	if (pModel) delete pModel;
}

CAirplanePlayer::~CAirplanePlayer()
{
}

void CAirplanePlayer::SetChild(CGameObject* pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();

	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;

	}
	else
	{
		m_pChild = pChild;

	}
}

void CAirplanePlayer::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
	}

	if (m_pSibling != NULL)
		m_pSibling->ReleaseUploadBuffers();
	if (m_pChild != NULL)
		m_pChild->ReleaseUploadBuffers();
}

void CAirplanePlayer::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

void CAirplanePlayer::SetModelSprite(CGameObject* Loot,CTexture* LootTexture, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	//모델들의 이름을 다 돌아들어가보면서 이름찾고, 그 이름의 메쉬를 가지고 잇는놈의 텍스처와.... 메쉬를 CSpriteObject로 바꾼다. 
	int num= 0;
	if (Loot->m_pSibling != NULL) {
		if ((0 == strcmp(Loot->m_pSibling->m_pstrFrameName, "Feather_L")) || (0 == strcmp(Loot->m_pSibling->m_pstrFrameName, "Feather_R")))
		{
			if (0 == strcmp(Loot->m_pSibling->m_pstrFrameName, "Feather_L")) num = 0;
			else if (0 == strcmp(Loot->m_pSibling->m_pstrFrameName, "Feather_R")) num = 1;

			cout << "find Test " << endl;
			//CSpriteObject* Temp = (CSpriteObject*)(Loot->m_pSibling);
			m_pAirSprites[num] = (CSpriteObject*)(Loot->m_pSibling);


			CSprite2Shader* CSpriteObjectShader = new CSprite2Shader();
			CSpriteObjectShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
			CSpriteObjectShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			CMaterial* pStriteMaterial = new CMaterial(1);
			pStriteMaterial->SetTexture(LootTexture);
			pStriteMaterial->SetShader(CSpriteObjectShader);


			m_pAirSprites[num]->SetMaterial(0, pStriteMaterial);
			m_pAirSprites[num]->SetRowColumn(1, 8);
			//m_pAirSprites[num]->SetSpeed(3.0f / 8);			//why Error? 
			m_pAirSprites[num]->Animate(0.0f);

			m_pAirSprites[num]->SpriteMode = static_cast<int>(SpriteType::Ship);
			m_pAirSprites[num]->is_Alive = true;

			if (num == 1)SetModelSprite(Loot->m_pSibling, LootTexture, pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		}
		else {
			SetModelSprite(Loot->m_pSibling, LootTexture, pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		};
	}
}



void CAirplanePlayer::FireBullet(CGameObject* pLockedObject)
{
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

		XMFLOAT3 m_xmf3Look = GetLookVector();
		XMFLOAT3 m_xmf3LookC = m_pCamera->GetLookVector();

		m_xmf3LookC.y = m_xmf3Look.y;
		pBulletObject->m_xmf3Look = m_xmf3Look;
		//cout << "카메라 GetLookVector: " << m_pCamera->GetLookVector().x <<" " << m_pCamera->GetLookVector().y << " " << m_pCamera->GetLookVector().z << " " <<endl;
		//cout << "GetLookVector: " << GetLookVector().x << " " << GetLookVector().y << " " << GetLookVector().z << " " << endl;

		pBulletObject->m_xmf4x4ToParent = m_xmf4x4ToParent;
		xmf3FirePosition.x = xmf3Position.x;
		xmf3FirePosition.y = xmf3Position.y;
		xmf3FirePosition.z = xmf3Position.z;
		pBulletObject->SetPosition(xmf3FirePosition);
		pBulletObject->SetActive(true);
	
		m_fFireWaitingTime = m_fFireDelayTime * 1.0f;

		if (pLockedObject)
		{
			pBulletObject->m_pLockedObject = pLockedObject;
		}
	}
}

void CAirplanePlayer::SetBulletFromServer(BULLET_INFO bulletInfo)
{
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
		
		//pBulletObject->Rotate(bulletInfo.pitch, bulletInfo.yaw, bulletInfo.roll);
		//pBulletObject->m_fPitch = bulletInfo.pitch; pBulletObject->m_fYaw = bulletInfo.yaw; pBulletObject->m_fRoll = bulletInfo.roll;
		pBulletObject->SetFirePosition(bulletInfo.pos);
		pBulletObject->SetMovingDirection(bulletInfo.direction);
		pBulletObject->SetActive(true);
		//pBulletObject->UpdateBoundingBox();

		/*
		if (pLockedObject)
		{
			pBulletObject->m_pLockedObject = pLockedObject;
		}*/
	}
}


void CAirplanePlayer::OnPrepareAnimate()
{
	m_pEngine[0] = FindFrame("Feather_L");
	m_pEngine[1] = FindFrame("Feather_R");
}

void CAirplanePlayer::Animate(float fTimeElapsed)
{
	OnPrepareAnimate();
	CPlayer::OnPrepareRender();

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);

	if (m_pSibling) m_pSibling->Animate(fTimeElapsed);
	if (m_pChild) m_pChild->Animate(fTimeElapsed);


	for (int i = 0; i < BULLETS; i++)
	{
		if (m_ppBullets[i]->m_bActive) {
			m_ppBullets[i]->Animate(fTimeElapsed);
		};
	}


}

void CAirplanePlayer::OnPrepareRender()
{

	CPlayer::OnPrepareRender();
}

void CAirplanePlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	for (int i = 0; i < BULLETS; i++)
		if(m_ppBullets[i])
			if (m_ppBullets[i]->m_bActive)
			{
				m_ppBullets[i]->Render(pd3dCommandList, pCamera); 
			};
//	for(int i=0; i<2; i++) if(m_pAirSprites[i])m_pAirSprites[i]->CSpriteObject::Render(pd3dCommandList, pCamera);

	//for (int i = 0; i < 2; i++) {
	//	m_pEngineFlame[i]->Animate(0.0);
	//	m_pEngineFlame[i]->UpdateShaderVariables(pd3dCommandList, m_pEngineFlame[i]->GetShaderVariables());
	//	m_pEngineFlame[i]->Render(pd3dCommandList, pCamera);
	//}

	CPlayer::Render(pd3dCommandList, pCamera);

}

void CAirplanePlayer::UpdateOnServer(bool rotate_update)
{
	if (!is_update) {
		SetPosition(player_info);
		is_update = true;
		//m_pCamera->Update(player_info.pos, 0);
	}

	if (rotate_update && !is_update_q) {
		XMVECTOR a = XMLoadFloat4(&player_quaternion);
		XMMATRIX mat = XMMatrixRotationQuaternion(a);
		XMFLOAT4X4 xmf4x4 = Matrix4x4::Multiply(Matrix4x4::Identity(), mat);
		m_xmf3Right.x = xmf4x4._11; m_xmf3Right.y = xmf4x4._12; m_xmf3Right.z = xmf4x4._13;
		m_xmf3Up.x = xmf4x4._21; m_xmf3Up.y = xmf4x4._22; m_xmf3Up.z = xmf4x4._23;
		m_xmf3Look.x = xmf4x4._31; m_xmf3Look.y = xmf4x4._32; m_xmf3Look.z = xmf4x4._33;
		is_update_q = true;
	}
}

XMVECTOR CAirplanePlayer::GetQuaternion()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;
	XMMATRIX mat = XMLoadFloat4x4(&m_xmf4x4ToParent);
	XMVECTOR vec = XMQuaternionRotationMatrix(mat);
	return XMVECTOR(vec);
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
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, float(m_nScreenWidth / float(m_nScreenHeight)), 60.0f);
			m_pCamera->SetViewport(0, 0, m_nScreenWidth, m_nScreenHeight, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, m_nScreenWidth, m_nScreenHeight);
			break;
		case SPACESHIP_CAMERA:
			SetFriction(100.5f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(40.0f);
			SetMaxVelocityY(40.0f);
			m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, float(m_nScreenWidth / float(m_nScreenHeight)), 60.0f);
			m_pCamera->SetViewport(0, 0, m_nScreenWidth, m_nScreenHeight, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, m_nScreenWidth, m_nScreenHeight);
			break;
		case THIRD_PERSON_CAMERA:
			//SetFriction(20.5f);
			//SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			//SetMaxVelocityXZ(25.5f);
			//SetMaxVelocityY(20.0f);
			//m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			//m_pCamera->SetTimeLag(0.25f);
			////m_pCamera->SetOffset(XMFLOAT3(0.0f, 15.0f, -30.0f));
			//m_pCamera->SetOffset(XMFLOAT3(0.0f, 105.0f, -140.0f));
			//m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			//m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			//m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			//break;

			SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, -0.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, float(m_nScreenWidth / float(m_nScreenHeight)), 60.0f);
			m_pCamera->SetViewport(0, 0, m_nScreenWidth, m_nScreenHeight, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, m_nScreenWidth, m_nScreenHeight);
			break;

		case DRIVE_CAMERA:
			cout << "drive mode \n";
			SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(DRIVE_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, float(m_nScreenWidth / float(m_nScreenHeight)), 60.0f);
			m_pCamera->SetViewport(0, 0, m_nScreenWidth, m_nScreenHeight, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, m_nScreenWidth, m_nScreenHeight);
			pastcam = DRIVE_CAMERA;

			break;

		case ATTACT_CAMERA_C:
			SetFriction(2.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(0.0f);
			SetMaxVelocityY(0.0f);
			m_pCamera = OnChangeCamera(ATTACT_CAMERA_C, nCurrentCameraMode);
			if (pastcam != ATTACT_CAMERA_C)
				m_pCamera->Rotate(0.0f, -r, 0.0f);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 10.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, float(m_nScreenWidth / float(m_nScreenHeight)), 60.0f);
			m_pCamera->SetViewport(0, 0, m_nScreenWidth, m_nScreenHeight, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, m_nScreenWidth, m_nScreenHeight);
			pastcam = ATTACT_CAMERA_C;
			
			break;
		case ATTACT_CAMERA_L:
			SetFriction(2.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(0.0f);
			SetMaxVelocityY(0.0f);
			m_pCamera = OnChangeCamera(ATTACT_CAMERA_L, nCurrentCameraMode);
			if (pastcam == ATTACT_CAMERA_R)
				m_pCamera->Rotate(0.0f, -r, 0.0f);
			r = -90.0f;
			if (pastcam != ATTACT_CAMERA_L)
				m_pCamera->Rotate(0.0f, r, 0.0f);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 10.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, float(m_nScreenWidth / float(m_nScreenHeight)), 60.0f);
			m_pCamera->SetViewport(0, 0, m_nScreenWidth, m_nScreenHeight, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, m_nScreenWidth, m_nScreenHeight);
			pastcam = ATTACT_CAMERA_L;

			break;
		case ATTACT_CAMERA_R:
			SetFriction(2.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(0.0f);
			SetMaxVelocityY(0.0f);
			m_pCamera = OnChangeCamera(ATTACT_CAMERA_R, nCurrentCameraMode);
			if (pastcam == ATTACT_CAMERA_L)
				m_pCamera->Rotate(0.0f, -r, 0.0f);
			r = 90.0f;
			if (pastcam != ATTACT_CAMERA_R)
				m_pCamera->Rotate(0.0f, r, 0.0f);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 10.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, float(m_nScreenWidth / float(m_nScreenHeight)), 60.0f);
			m_pCamera->SetViewport(0, 0, m_nScreenWidth, m_nScreenHeight, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, m_nScreenWidth, m_nScreenHeight);
			pastcam = ATTACT_CAMERA_R;
			break;
		default:
			break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
#define _WITH_DEBUG_CALLBACK_DATA

void CSoundCallbackHandler::HandleCallback(void *pCallbackData, float fTrackPosition)
{
   _TCHAR *pWavName = (_TCHAR *)pCallbackData; 
#ifdef _WITH_DEBUG_CALLBACK_DATA
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("%s(%f)\n"), pWavName, fTrackPosition);
	OutputDebugString(pstrDebug);
#endif
#ifdef _WITH_SOUND_RESOURCE
   PlaySound(pWavName, ::ghAppInstance, SND_RESOURCE | SND_ASYNC);
#else
   PlaySound(pWavName, NULL, SND_FILENAME | SND_ASYNC);
#endif
}

CTerrainPlayer::CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext, int i)
{
	type = PlayerType::INSIDE;
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	//CLoadedModelInfo *pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Angrybot.bin", NULL);
	CLoadedModelInfo* pDoggyModel;
	if (i == 0)	pDoggyModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Doggy_V0.bin", NULL); //Doggy_V0
	else if (i == 1)pDoggyModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Doggy_V1.bin", NULL);
	else pDoggyModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Doggy_V2.bin", NULL);


	cout << "플레이어~";
	SetChild(pDoggyModel->m_pModelRootObject, true);
	cout << "child : " << m_pChild<<endl;
	cout << "m_pSibling : " << m_pSibling << endl;


	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 4, pDoggyModel);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
	m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
	m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);


	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);



	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);


	p_effectSound[0] = new CSound("Sound/walk.mp3", false, 1.0f);


#ifdef _WITH_SOUND_RESOURCE
	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
#else
	//m_pSkinnedAnimationController->SetCallbackKey(1, 0, 0.001f, _T("Sound/walk.wav"));
	//m_pSkinnedAnimationController->SetCallbackKey(1, 1, 0.125f, _T("Sound/Footstep02.wav"));
//	m_pSkinnedAnimationController->SetCallbackKey(1, 2, 0.39f, _T("Sound/Footstep03.wav"));
#endif
	CAnimationCallbackHandler *pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	
	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;
	//SetPosition(XMFLOAT3(310.0f, pTerrain->GetHeight(310.0f, 590.0f), 590.0f));
	SetPosition(XMFLOAT3(310.0f, pTerrain->GetHeight(310.0f, 590.0f), 590.0f));

	aabb = BoundingBox(GetPosition(), XMFLOAT3(5.0f, 5.0f, 5.0f));

	if (pDoggyModel) delete pDoggyModel;
}

CTerrainPlayer::~CTerrainPlayer()
{
}


void CTerrainPlayer::SetChild(CGameObject* pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();

	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;

	}
	else
	{
		m_pChild = pChild;
	}
}

void CTerrainPlayer::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
	}

	if (m_pSibling != NULL)
		m_pSibling->ReleaseUploadBuffers();
	if (m_pChild != NULL)
		m_pChild->ReleaseUploadBuffers();
}

void CTerrainPlayer::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

void CTerrainPlayer::Animate(float fTimeElapsed)
{
	CPlayer::OnPrepareRender();

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);

	if (m_pSibling) m_pSibling->Animate(fTimeElapsed);
	if (m_pChild) m_pChild->Animate(fTimeElapsed);
}

void CTerrainPlayer::UpdateOnServer(bool rotate_update)
{
	if (type != PlayerType::INSIDE) { return; }

	if (!is_update) {
		SetPosition(player_info.pos);
		if (rotate_update) {
			Rotate(0.0f, player_info.m_fYaw - m_fYaw, 0.0f);
		}
		//m_pCamera->Update(player_info.pos, 0);
		is_update = true;
	}
}

CCamera *CTerrainPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	cout << "ChangeCamera!!" << endl;
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, -400.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, float(m_nScreenWidth / float(m_nScreenHeight)), 60.0f);
			m_pCamera->SetViewport(0, 0, m_nScreenWidth, m_nScreenHeight, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, m_nScreenWidth, m_nScreenHeight);
			break;
		case SPACESHIP_CAMERA:
			SetFriction(125.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, float(m_nScreenWidth / float(m_nScreenHeight)), 60.0f);
			m_pCamera->SetViewport(0, 0, m_nScreenWidth, m_nScreenHeight, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, m_nScreenWidth, m_nScreenHeight);
			break;
		case THIRD_PERSON_CAMERA:
			SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, float(m_nScreenWidth / float(m_nScreenHeight)), 60.0f);
			m_pCamera->SetViewport(0, 0, m_nScreenWidth, m_nScreenHeight, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, m_nScreenWidth, m_nScreenHeight);
			break;
		default:
			break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}

void CTerrainPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	/*float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 0.0f;
	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}*/
	XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
	xmf3PlayerVelocity.y = 0.0f;
	SetVelocity(xmf3PlayerVelocity);
	if(this->motion!= AnimationState::SIT)
		xmf3PlayerPosition.y = 224.0f;
	else
		xmf3PlayerPosition.y = 229.0f;
	SetPosition(xmf3PlayerPosition);

}

void CTerrainPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	int z = (int)(xmf3CameraPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z, bReverseQuad) + 5.0f;
	if (xmf3CameraPosition.y <= fHeight)
	{
		xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			CThirdPersonCamera *p3rdPersonCamera = (CThirdPersonCamera *)m_pCamera;
			p3rdPersonCamera->SetLookAt(GetPosition());
		}
	}
}

void CTerrainPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_pSkinnedAnimationController->SetTrackEnable(3, false);

	}

	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);

}

void CTerrainPlayer::Update(float fTimeElapsed)
{
	CPlayer::Update(fTimeElapsed);

	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
		if (::IsZero(fLength))
		{

			if (motion == AnimationState::SIT) {
				m_pSkinnedAnimationController->SetTrackEnable(0, false);
				m_pSkinnedAnimationController->SetTrackEnable(1, false);
				m_pSkinnedAnimationController->SetTrackEnable(2, false);
				m_pSkinnedAnimationController->SetTrackEnable(3, true);

			}
			else if(motion == AnimationState::IDLE) {
				p_effectSound[0]->pause();
				m_pSkinnedAnimationController->SetTrackEnable(0, true);
				m_pSkinnedAnimationController->SetTrackEnable(1, false);
				m_pSkinnedAnimationController->SetTrackEnable(2, false);
				m_pSkinnedAnimationController->SetTrackEnable(3, false);
				m_pSkinnedAnimationController->SetTrackPosition(1, 0.0f);
				m_pSkinnedAnimationController->SetTrackPosition(2, 0.0f);


			}
			else {
				p_effectSound[0]->resume();
				p_effectSound[0]->play();
			
				m_pSkinnedAnimationController->SetTrackEnable(0, false);
				m_pSkinnedAnimationController->SetTrackEnable(1, true);
				m_pSkinnedAnimationController->SetTrackEnable(2, false);
				m_pSkinnedAnimationController->SetTrackEnable(3, false);
			}
		}
	}
	UpdateBoundingBox(); // 나중에 씬 전체 boundingbox업뎃 묶어서 함수만들기 
}
