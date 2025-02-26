//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer()
{
	CGameObject::CGameObject();
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_fMaxVelocityXZ = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	is_update = true;
}

CPlayer::~CPlayer()
{
}

void CPlayer::Move(char cDirection, float fDistance, bool bUpdateVelocity)
{
	XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
	if (cDirection & option0) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
	if (cDirection & option1) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
	if (cDirection & option3) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
	if (cDirection & option2) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);

	Move(xmf3Shift, bUpdateVelocity);
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
		//if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
		//if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
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


	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Update(float fTimeElapsed)
{
	XMFLOAT3 pos = GetPosition();
	float dist = Vector3::Length(Vector3::Subtract(pos, XMFLOAT3(6000.f, 6000.f, 6000.f)));

	if (dist < 1000.f)
	{
		XMFLOAT3 ToGo = Vector3::Subtract(pos, XMFLOAT3(6000.f, 6000.f, 6000.f));
		ToGo = Vector3::ScalarProduct(ToGo, 1000.f);
		ToGo = Vector3::Add(XMFLOAT3(6000.f, 6000.f, 6000.f), ToGo);
		SetPosition(ToGo);
	}

	float fLength = Vector3::Length(m_xmf3Velocity);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, m_fMaxVelocityXZ, true);
	}

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	XMFLOAT3 LookVelocity = Vector3::ScalarProduct(m_xmf3Look, fTimeElapsed * 20.f, false);
	xmf3Velocity = Vector3::Add(xmf3Velocity, LookVelocity);
	Move(xmf3Velocity, false);

	float fDeceleration = (300.0f * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;

	if (Vector3::Length(m_xmf3Velocity) > fDeceleration) {
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
	}
	else {
		m_xmf3Velocity.x = 0; m_xmf3Velocity.y = 0; m_xmf3Velocity.z = 0;
	}
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._12 = m_xmf3Right.y; m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x; m_xmf4x4World._32 = m_xmf3Look.y; m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z;

	m_xmf4x4World = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4World);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CAirplanePlayer::CAirplanePlayer()
{
	CPlayer::CPlayer();

	cDirection = 0;
	Quaternion = XMFLOAT4(-0.f, 0.f, -0.f, 1.f);
	is_update = true;

	max_hp = 100;
	hp = 100;

	damage = 3;
	heal = 10;
	def = 0;

	SetFriction(250.0f);
	SetMaxVelocityXZ(100.0f);
}

CAirplanePlayer::~CAirplanePlayer()
{
}

void CAirplanePlayer::GetAttack(char damage)
{
	char real_damage = damage - def;
	if (real_damage <= 0) { real_damage = 1; }
	hp -= real_damage;
}

bool CAirplanePlayer::GetHeal()
{
	if (heal + hp < max_hp) {
		hp += heal;
	}
	else {
		if (hp < max_hp) {
			hp = max_hp;
		}
		else {
			return false;
		}
	}
	return true;
}

void CAirplanePlayer::Reset()
{
	cDirection = 0;
	Quaternion = XMFLOAT4(-0.f, 0.f, -0.f ,1.f);
	is_update = true;

	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();

	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	max_hp = 100;
	hp = 100;

	damage = 3;
	heal = 10;
	def = 0;
}

void CAirplanePlayer::SetKeyInput(char key_input)
{
	cDirection = key_input;
}

void CAirplanePlayer::Animate(float fTimeElapsed)
{
}

void CAirplanePlayer::Update(float fTimeElapsed)
{
	if (!is_update) {
		XMVECTOR a = XMLoadFloat4(&Quaternion);
		XMMATRIX mat = XMMatrixRotationQuaternion(a);
		XMFLOAT4X4 xmf4x4 = Matrix4x4::Multiply(Matrix4x4::Identity(), mat);
		m_xmf3Right.x = xmf4x4._11; m_xmf3Right.y = xmf4x4._12; m_xmf3Right.z = xmf4x4._13;
		m_xmf3Up.x = xmf4x4._21; m_xmf3Up.y = xmf4x4._22; m_xmf3Up.z = xmf4x4._23;
		m_xmf3Look.x = xmf4x4._31; m_xmf3Look.y = xmf4x4._32; m_xmf3Look.z = xmf4x4._33;
		is_update = true;

		if (cDirection != 0) {
			Move(cDirection, 800.0f * fTimeElapsed, true);
		}
	}

	CPlayer::Update(fTimeElapsed);
	OnPrepareRender();
}

void CAirplanePlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CTerrainPlayer::CTerrainPlayer()
{
	CPlayer::CPlayer();
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_fMaxVelocityXZ = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	input_info.dwDirection = NULL;
	input_info.yaw = 0.f;

	is_update = true;

	cutscene_end = false;
}

CTerrainPlayer::~CTerrainPlayer()
{
}

void CTerrainPlayer::Reset()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();

	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_fMaxVelocityXZ = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	input_info.dwDirection = NULL;
	input_info.yaw = 0.f;

	is_update = true;

	cutscene_end = false;
}

bool CTerrainPlayer::CheckCollision(const XMFLOAT3 pos[])
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

	//조각상
	if (333.f < m_xmf3Position.x && m_xmf3Position.x < 351.f && 663.f < m_xmf3Position.z && m_xmf3Position.z < 681.75f) {
		return true;
	}
	float x, z;

	for (int i = 0; i < 3; ++i) {
		x = pos[i].x - m_xmf3Position.x;
		z = pos[i].z - m_xmf3Position.z;
		if (fabs(x) < 5.f && fabs(z) < 5.f) { return true; }
	}

	return false;
}

void CTerrainPlayer::Move(DWORD dwDirection, float fDistance, const XMFLOAT3 pos[], bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);

		CPlayer::Move(xmf3Shift, bUpdateVelocity);

		if (CheckCollision(pos)) { 
			xmf3Shift.x = -xmf3Shift.x; 
			xmf3Shift.z = -xmf3Shift.z;
			CPlayer::Move(xmf3Shift, bUpdateVelocity);
		}
	}

	//CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}

void CTerrainPlayer::Update(float fTimeElapsed, const XMFLOAT3 pos[])
{
	if (input_info.yaw != m_fYaw) {
		Rotate(0, input_info.yaw - m_fYaw, 0);
	}
	if (input_info.dwDirection) {
		Move(input_info.dwDirection, 80.0f * fTimeElapsed, pos, false);
		input_info.dwDirection = NULL;
	}

	is_update = true;
}
