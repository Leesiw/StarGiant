//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject::CGameObject()
{
	m_xmf4x4Transform = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
	boundingbox = BoundingOrientedBox{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
}

CGameObject::~CGameObject()
{
}

void CGameObject::AddRef()
{
	m_nReferences++;

	if (m_pSibling) m_pSibling->AddRef();
	if (m_pChild) m_pChild->AddRef();
}

void CGameObject::Release()
{
	if (m_pChild) m_pChild->Release();
	if (m_pSibling) m_pSibling->Release();

	if (--m_nReferences <= 0) delete this;
}

void CGameObject::SetOOBB() {
	m_xmOOBB.Center.x = m_xmf4x4Transform._41;
	m_xmOOBB.Center.y = m_xmf4x4Transform._42;
	m_xmOOBB.Center.z = m_xmf4x4Transform._43;
}

void CGameObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	UpdateBoundingBox();

	if (m_fRotationSpeed != 0.0f) Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);
	//if (m_fMovingSpeed != 0.0f) Move(m_xmf3MovingDirection, m_fMovingSpeed * fTimeElapsed);

	if (m_pSibling) m_pSibling->Animate(fTimeElapsed, pxmf4x4Parent);
	if (m_pChild) m_pChild->Animate(fTimeElapsed, &m_xmf4x4World);
}

void CGameObject::MakeBoundingBox()
{
	//m_xmOOBB.Center = pMeshInfo->m_xmf3AABBCenter.Center;
	//m_xmOOBB.Extents = m_xmBoundingBox.Extents;

}

void CGameObject::UpdateBoundingBox()
{
	boundingbox.Transform(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));
	XMStoreFloat4(&m_xmOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
}

bool CGameObject::HierarchyIntersects(CGameObject* pCollisionGameObject, bool isSecond)
{

	if (m_xmOOBB.Intersects(pCollisionGameObject->m_xmOOBB)) return true;
	if (m_pSibling) if (m_pSibling->HierarchyIntersects(pCollisionGameObject, isSecond)) return true;
	if (m_pChild) if (m_pChild->HierarchyIntersects(pCollisionGameObject, isSecond)) return true;

	if (isSecond)
		return false;

	return pCollisionGameObject->HierarchyIntersects(this, true);
}

void CGameObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4Transform, *pxmf4x4Parent) : m_xmf4x4Transform;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4Transform._41 = x;
	m_xmf4x4Transform._42 = y;
	m_xmf4x4Transform._43 = z;

	UpdateTransform(NULL);
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxScale, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
//=====================================================================
void CGameObject::MoveMeteo(XMFLOAT3 xmf3Look, float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	xmf3Look.y = 0.0f;
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, -fDistance);
	CGameObject::SetPosition(xmf3Position);
}

std::default_random_engine re;
std::uniform_int_distribution<int>uid(1, 5);

void CGameObject::MakeSpeed()
{
	m_Speed = (float)uid(re) * 10;
	m_RoateAngle = (float)uid(re);
}
void CGameObject::UpdateSpeed(float fTimeElapsed)
{//(float)uid(re))
	float distance = fTimeElapsed * m_RoateAngle / 100;

	//MoveMeteo(m_Speed/100);
	MoveMeteo(m_xmLook, fTimeElapsed * m_Speed / 100);
	Rotate(distance, distance, distance);
}
void CGameObject::UpdateRespawn(BoundingBox Player, XMFLOAT3 Switch, XMFLOAT3 m_xmf3Look) {


	XMFLOAT3 After = GetPosition();
	//After.z = Switch.z + 50.f; //소환 위치 조정
	//After.x = Switch.x + 50.f;
	After = Vector3::Add(Switch, m_xmf3Look, 50.0f);
	After.y = Switch.x;
	SetPosition(After);
	m_xmLook = m_xmf3Look;
	std::cout << "리스폰";


}
//=====================================================================
std::random_device rd;
std::default_random_engine dre(rd());
std::uniform_real_distribution<float> urd(0, 180);
std::uniform_real_distribution<float> urd_box(-150, 150);

void CGameObject::Rotate(XMFLOAT3& xmf3RotationAxis, float fAngle)
{
	XMFLOAT4X4 mtxRotate = Matrix4x4::RotationAxis(xmf3RotationAxis, fAngle);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}
void CGameObject::Move(XMFLOAT3& vDirection, float fSpeed)
{
	SetPosition(m_xmf4x4World._41 + vDirection.x * fSpeed, m_xmf4x4World._42 + vDirection.y * fSpeed, m_xmf4x4World._43 + vDirection.z * fSpeed);
}

void CGameObject::Replace(XMFLOAT3 Point)
{
	SetPosition(Point.x + urd_box(dre), Point.y + urd_box(dre), Point.z + urd_box(dre));


}

void CGameObject::MoveToP(XMFLOAT3 Point, float fTimeElapsed)
{
	SetMovingDirection(Point);
	if (m_fMovingSpeed != 0.0f)  Move(m_xmf3MovingDirection, -m_fMovingSpeed * fTimeElapsed);
}


void CGameObject::TurnSpeed()
{
	//XMMATRIX xmmtxRotate = XMMatrixRotationZ(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
	//m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_xmf4x4Transform);
	//SetRotationSpeed(rand() / 100);
	m_fRotationSpeed = urd(dre);
	//std::cout << m_fRotationSpeed;
	//Rotate(m_fxRotationSpeed * fTimeElapsed, m_fyRotationSpeed * fTimeElapsed, m_fzRotationSpeed * fTimeElapsed);
}



//=====================================================================


void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT4* pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CRotatingObject::CRotatingObject()
{
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 15.0f;
}

CRotatingObject::~CRotatingObject()
{
}

void CRotatingObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	CGameObject::Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);
	CGameObject::Animate(fTimeElapsed, pxmf4x4Parent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CRevolvingObject::CRevolvingObject()
{
	m_xmf3RevolutionAxis = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_fRevolutionSpeed = 0.0f;
}

CRevolvingObject::~CRevolvingObject()
{
}

void CRevolvingObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3RevolutionAxis), XMConvertToRadians(m_fRevolutionSpeed * fTimeElapsed));
	m_xmf4x4Transform = Matrix4x4::Multiply(m_xmf4x4Transform, mtxRotate);

	CGameObject::Animate(fTimeElapsed, pxmf4x4Parent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHellicopterObject::CHellicopterObject()
{
}

CHellicopterObject::~CHellicopterObject()
{
}

void CHellicopterObject::OnInitialize()
{
}

void CHellicopterObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
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

	CGameObject::Animate(fTimeElapsed, pxmf4x4Parent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
MeteoObject::MeteoObject()
{
	boundingbox = BoundingOrientedBox{ XMFLOAT3{ 0.0f, 0.0f, - 0.004253f }, XMFLOAT3{ 0.016793f, 0.016793f, 0.020887f }, XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f } };
}

MeteoObject::~MeteoObject()
{
}

void MeteoObject::OnInitialize()
{
	//m_pMainRotorFrame = FindFrame("rotor");
	//m_pTailRotorFrame = FindFrame("black_m_7");
}

void MeteoObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	if (m_pMainRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4Transform);
	}
	if (m_pTailRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4Transform);
	}

	CGameObject::Animate(fTimeElapsed, pxmf4x4Parent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSuperCobraObject::CSuperCobraObject()
{
}

CSuperCobraObject::~CSuperCobraObject()
{
}

void CSuperCobraObject::OnInitialize()
{
	//m_pMainRotorFrame = FindFrame("MainRotor_LOD0");
	//m_pTailRotorFrame = FindFrame("TailRotor_LOD0");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
MeteoriteObject::MeteoriteObject()
{
}

MeteoriteObject::~MeteoriteObject()
{
}

void MeteoriteObject::OnInitialize()
{
	//m_pMainRotorFrame = FindFrame("Rotor");
	//m_pTailRotorFrame = FindFrame("Back_Rotor");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMi24Object::CMi24Object()
{
}

CMi24Object::~CMi24Object()
{
}

void CMi24Object::OnInitialize()
{
	//m_pMainRotorFrame = FindFrame("Top_Rotor");
	//m_pTailRotorFrame = FindFrame("Tail_Rotor");
}






//============================================


CBulletObject::CBulletObject(float fEffectiveRange)
{
	m_fBulletEffectiveRange = fEffectiveRange;
}

CBulletObject::~CBulletObject()
{
}

void CBulletObject::SetFirePosition(XMFLOAT3 xmf3FirePosition)
{
	m_xmf3FirePosition = xmf3FirePosition;
	SetPosition(xmf3FirePosition);
}

void CBulletObject::Reset()
{
	Rotate(-m_fPitch, -m_fYaw, -m_fRoll);
	m_fPitch = 0; m_fYaw = 0; m_fRoll = 0;
	m_pLockedObject = NULL;
	m_fElapsedTimeAfterFire = 0;
	m_fMovingDistance = 0;
	m_fRotationAngle = 0.0f;

	m_bActive = false;
}

void CBulletObject::Animate(float fElapsedTime)
{
	m_fElapsedTimeAfterFire += fElapsedTime;

	float fDistance = m_fMovingSpeed * fElapsedTime;

	if ((m_fElapsedTimeAfterFire > m_fLockingDelayTime) && m_pLockedObject)
	{
		XMFLOAT3 xmf3Position = GetPosition();
		XMVECTOR xmvPosition = XMLoadFloat3(&xmf3Position);

		XMFLOAT3 xmf3LockedObjectPosition = m_pLockedObject->GetPosition();
		XMVECTOR xmvLockedObjectPosition = XMLoadFloat3(&xmf3LockedObjectPosition);
		XMVECTOR xmvToLockedObject = xmvLockedObjectPosition - xmvPosition;
		xmvToLockedObject = XMVector3Normalize(xmvToLockedObject);

		XMVECTOR xmvMovingDirection = XMLoadFloat3(&m_xmf3MovingDirection);
		xmvMovingDirection = XMVector3Normalize(XMVectorLerp(xmvMovingDirection, xmvToLockedObject, 0.25f));
		XMStoreFloat3(&m_xmf3MovingDirection, xmvMovingDirection);
	}
#ifdef _WITH_VECTOR_OPERATION
	XMFLOAT3 xmf3Position = GetPosition();

	m_fRotationAngle += m_fRotationSpeed * fElapsedTime;
	if (m_fRotationAngle > 360.0f) m_fRotationAngle = m_fRotationAngle - 360.0f;


	XMFLOAT3 xmf3RotationAxis = Vector3::CrossProduct(m_xmf3RotationAxis, m_xmf3MovingDirection, true);
	float fDotProduct = Vector3::DotProduct(m_xmf3RotationAxis, m_xmf3MovingDirection);
	float fRotationAngle = ::IsEqual(fDotProduct, 1.0f) ? 0.0f : (float)XMConvertToDegrees(acos(fDotProduct));
	XMFLOAT4X4 mtxRotate2 = Matrix4x4::RotationAxis(xmf3RotationAxis, fRotationAngle);

	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate1, mtxRotate2);

	XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(m_xmf3MovingDirection, fDistance, false);
	xmf3Position = Vector3::Add(xmf3Position, xmf3Movement);
	SetPosition(xmf3Position);
#else

	XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(m_xmf3MovingDirection, fDistance, false);
	XMFLOAT3 xmf3Position = GetPosition();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Movement);
	SetPosition(xmf3Position);
	m_fMovingDistance += fDistance;
#endif


	if ((m_fMovingDistance > m_fBulletEffectiveRange) || (m_fElapsedTimeAfterFire > m_fLockingTime)) Reset();
}

