//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer()
{
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
}

CPlayer::~CPlayer()
{
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
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
	}
}

void CPlayer::Rotate(float x, float y, float z)
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

	if (y != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
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
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength) * fTimeElapsed * 30;
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength) * fTimeElapsed * 30;
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength) * fTimeElapsed * 30;

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	
	Move(xmf3Velocity, false);

	fLength = Vector3::Length(m_xmf3Velocity);
	//if (fLength > 100.0) {	// 최소 속도 지정
		float fDeceleration = (m_fFriction * fTimeElapsed);
		if (fDeceleration > fLength) fDeceleration = fLength;
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
	//}
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;

	m_xmf4x4ToParent = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4ToParent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CAirplanePlayer::CAirplanePlayer()
{
	SetFriction(250.0f);
	SetGravity(XMFLOAT3(0.0f, -0.0f, 0.0f));
	SetMaxVelocityXZ(300.0f);
	SetMaxVelocityY(400.0f);

	for (int i = 0; i < BULLETS; i++)
	{
		m_ppBullets[i] = new CBulletObject(m_fBulletEffectiveRange);
		m_ppBullets[i]->SetScale(2.0f, 2.0f, 2.0f);

		m_ppBullets[i]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_ppBullets[i]->SetMovingSpeed(300.0f);
		m_ppBullets[i]->SetActive(false);
	}
}

CAirplanePlayer::~CAirplanePlayer()
{
}

bool CAirplanePlayer::FireBullet(short attack_num)
{
	if (m_fFireWaitingTime[attack_num] > 0.0f)
		return false;

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

		pBulletObject->Rotate(m_fPitch, m_fYaw, m_fRoll);
		pBulletObject->m_fPitch = m_fPitch; pBulletObject->m_fYaw = m_fYaw; pBulletObject->m_fRoll = m_fRoll;
		pBulletObject->SetFirePosition(xmf3FirePosition);
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetActive(true);

		m_fFireWaitingTime[attack_num] = m_fFireDelayTime * 1.0f;

		return true;
	}
}

void CAirplanePlayer::Animate(float fTimeElapsed)
{
	for (int i = 0; i < BULLETS; i++)
	{
		if (m_ppBullets[i]->m_bActive) {
			std::cout << "b애니- ";
			m_ppBullets[i]->Animate(fTimeElapsed);
		};
	}

	CPlayer::Animate(fTimeElapsed);
}

void CAirplanePlayer::Update(float fTimeElapsed)
{
	CPlayer::Update(fTimeElapsed);

	for (int i = 0; i < MAX_USER; ++i) {
		if (m_fFireWaitingTime[i] > 0.0f)
			m_fFireWaitingTime[i] -= fTimeElapsed;
	}

	Rotate(0, input_info.yaw - m_fYaw, 0);

	if (input_info.dwDirection) {
		Move(input_info.dwDirection, 800.0f * fTimeElapsed, true);
		input_info.dwDirection = NULL;
	}

}

void CAirplanePlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CTerrainPlayer::CTerrainPlayer()
{
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
}

CTerrainPlayer::~CTerrainPlayer()
{
}

bool CTerrainPlayer::CheckCollision()
{
	// 외부
	if (m_xmf3Position.x < 295.4f || m_xmf3Position.x > 554.8f || m_xmf3Position.z < 565.1f || m_xmf3Position.z > 792.5f) {
		return true;
	}

	//모니터1
	if (389.75f < m_xmf3Position.x && m_xmf3Position.x < 446.53f &&  760.f < m_xmf3Position.z &&  m_xmf3Position.z < 767.f) {
		return true;
	}

	//모니터2
	if (524.7f < m_xmf3Position.x && m_xmf3Position.x < 531.1f && 646.4f < m_xmf3Position.z && m_xmf3Position.z < 704.34f) {
		return true;
	}

	//모니터3
	if (389.f < m_xmf3Position.x && m_xmf3Position.x < 447.7f && 591.02f < m_xmf3Position.z && m_xmf3Position.z < 597.53f) {
		return true;
	}

	//의자1
	if (410.03f < m_xmf3Position.x && m_xmf3Position.x < 425.65f && 610.7f < m_xmf3Position.z && m_xmf3Position.z < 626.77f) {
		return true;
	}

	//의자2
	if (497.44f < m_xmf3Position.x && m_xmf3Position.x < 513.16f && 668.66f < m_xmf3Position.z && m_xmf3Position.z < 685.f) {
		return true;
	}

	//의자3
	if (409.93f < m_xmf3Position.x && m_xmf3Position.x < 425.15f && 729.2f < m_xmf3Position.z && m_xmf3Position.z < 745.23f) {
		return true;
	}

	//기둥1
	if (425.5f < m_xmf3Position.x && m_xmf3Position.x < 445.9f && 648.22f < m_xmf3Position.z && m_xmf3Position.z < 669.4f) {
		return true;
	}

	//기둥2
	if (425.5f < m_xmf3Position.x && m_xmf3Position.x < 445.9f && 683.7f < m_xmf3Position.z && m_xmf3Position.z < 705.f) {
		return true;
	}

	//책상
	if (415.0f < m_xmf3Position.x && m_xmf3Position.x < 429.5f && 655.28f < m_xmf3Position.z && m_xmf3Position.z < 697.93f) {
		return true;
	}

	//중앙 의자
	if (396.46f < m_xmf3Position.x && m_xmf3Position.x < 411.03f && 668.5f < m_xmf3Position.z && m_xmf3Position.z < 684.32f) {
		return true;
	}

	return false;
}

void CTerrainPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{

		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);

		CPlayer::Move(xmf3Shift, bUpdateVelocity);

		if (CheckCollision()) { 
			xmf3Shift.x = -xmf3Shift.x; 
			xmf3Shift.z = -xmf3Shift.z;
			CPlayer::Move(xmf3Shift, bUpdateVelocity);
		}

	}

	//CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}

void CTerrainPlayer::Update(float fTimeElapsed)
{
	//if (input_info.yaw != 0) {
		Rotate(0, input_info.yaw - m_fYaw, 0);
	//}
	if (input_info.dwDirection) {
		
		//OnPrepareRender();
		//UpdateTransform();
		Move(input_info.dwDirection, 80.0f * fTimeElapsed, false);
		input_info.dwDirection = NULL;
		m_cAnimation = 1;
	}
	else {
		m_cAnimation = 0;
	}
}
