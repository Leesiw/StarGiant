#include "Enemy.h"
#include <limits>

CEnemy::CEnemy()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
	m_fSpeed = 0.f;
	hp = 10;
	state = EnemyState::IDLE;

	isAlive = false;
}

CEnemy::~CEnemy()
{
}

void CEnemy::AI(float fTimeElapsed, XMFLOAT3 & player_pos)
{
	VelocityUpdate(fTimeElapsed);
	
	if (m_fCoolTimeRemaining > 0.0f) {
		m_fCoolTimeRemaining -= fTimeElapsed;
	}
	
	XMFLOAT3 pos = GetPosition();
	if(fabs(pos.x - player_pos.x) < 50.0f && fabs(pos.z - player_pos.z) < 50.0f)
	{
		XMFLOAT3 ToGo = Vector3::Add(pos, player_pos, -1.f);
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, ToGo, fTimeElapsed * 10.f);
	}

	float dist;
	switch (state)
	{
	case EnemyState::IDLE:
		// 플레이어와의 거리
		dist = Vector3::Length(Vector3::Subtract(pos, player_pos));
		if (dist > m_fAttackRange)	// 사거리 충족 안되면 이동
		{
			switch (urdEnemyAI(dree) % 4) {	// 목적지 설정
			case 0:
			{
				m_xmf3Destination.x = urdPos2(dree);
				m_xmf3Destination.z = urdPos3(dree);
				break;
			}
			case 1:
			{
				m_xmf3Destination.x = -urdPos2(dree);
				m_xmf3Destination.z = urdPos3(dree);
				break;
			}
			case 2:
			{
				m_xmf3Destination.x = urdPos3(dree);
				m_xmf3Destination.z = -urdPos2(dree);
				break;
			}
			case 3:
			{
				m_xmf3Destination.x = urdPos3(dree);
				m_xmf3Destination.z = urdPos2(dree);
				break;
			}
			}

			m_xmf3Destination.y = urdPos3(dree);
			//m_xmf3Destination.x += player_pos.x;
			//m_xmf3Destination.y += player_pos.y;
			//m_xmf3Destination.z += player_pos.z;
			state = EnemyState::MOVE;
		}
		else
		{
			if (urdEnemyAI(dree) > 10)
			{
				state = EnemyState::ATTACK;
			}
			else 
			{
				state = EnemyState::AVOID;
				if (urdEnemyAI(dree) > 50)
				{
					m_bAvoidDir = true;
				}
				else
				{
					m_bAvoidDir = false;
				}
			}
		}
		break;
	case EnemyState::AIMING:	// 플레이어 방향을 바라보도록 한다
		AimingAI(fTimeElapsed, player_pos);
		break;
	case EnemyState::ATTACK:
		AttackAI(fTimeElapsed, player_pos);
		break;
	case EnemyState::AVOID:
		AvoidAI(fTimeElapsed);
		break;
	case EnemyState::MOVE:
		MoveAI(fTimeElapsed, player_pos);
		break;
	}
	SendPos();
}

void CEnemy::MoveAI(float fTimeElapsed, XMFLOAT3& player_pos)
{
	XMFLOAT3 xmf3Position = GetPosition();

	m_xmf3Destination.x += player_pos.x;
	m_xmf3Destination.y += player_pos.y;
	m_xmf3Destination.z += player_pos.z;

	XMFLOAT3 vec = Vector3::Subtract(xmf3Position, m_xmf3Destination);
	float dist = Vector3::Length(Vector3::Subtract(xmf3Position, m_xmf3Destination));
	m_fSpeed = 500.0f;
	if (dist > 50.f)
	{
		XMFLOAT3 xmf3Look = GetLook();
		XMFLOAT3 ToDestination = Vector3::Subtract( m_xmf3Destination, xmf3Position);

		xmf3Look = Vector3::Normalize(xmf3Look);
		ToDestination = Vector3::Normalize(ToDestination);
		double a = xmf3Look.x * ToDestination.z - xmf3Look.z * ToDestination.x;
		double angle = asin(a);	//목표 지점을 보기 위해 돌려야 하는 각도

		angle = XMConvertToDegrees(angle);
		double rotate_angle = fTimeElapsed * 180.0f;	// 초당 180도 돌아감

		if (fabs(angle) > rotate_angle) {	// 목표지점을 보도록 rotate
			if (angle > 0) { Rotate(0, -rotate_angle, 0); }
			else { Rotate(0, rotate_angle, 0); }
		}

		ToDestination = Vector3::ScalarProduct(ToDestination, fTimeElapsed * m_fSpeed, false);
		ToDestination = Vector3::Add(xmf3Position, ToDestination);

		SetPosition(ToDestination);	// 보고 있는 방향과 상관없이 목표지점으로 속도만큼 이동
	}
	else {
		state = EnemyState::AIMING;
	}
	m_xmf3Destination.x -= player_pos.x;
	m_xmf3Destination.y -= player_pos.y;
	m_xmf3Destination.z -= player_pos.z;

	SendPos();
	
}

void CEnemy::AimingAI(float fTimeElapsed, XMFLOAT3& player_pos)
{
	XMFLOAT3 xmf3RotateDir = GetLook();

	XMFLOAT3 xmf3Position = GetPosition();

	XMFLOAT3 xmfToPlayer = Vector3::Subtract(xmf3Position, player_pos);

	XMFLOAT3 xmf3Look = GetLook();

	xmf3Look = Vector3::Normalize(xmf3Look);
	xmfToPlayer = Vector3::Normalize(xmfToPlayer);
	double a = xmf3Look.x * xmfToPlayer.z - xmf3Look.z * xmfToPlayer.x;
	double angle = asin(a);

	angle = XMConvertToDegrees(angle);
	double rotate_angle = fTimeElapsed * 90.0f;

	if (fabs(angle) > rotate_angle) {
		if (angle < 0) { Rotate(0, -rotate_angle, 0); }
		else { Rotate(0, rotate_angle, 0); }
	}
	else {
		Rotate(0, angle, 0);
		state = EnemyState::ATTACK;
	}

	SendPos();
}

void CEnemy::AttackAI(float fTimeElapsed, XMFLOAT3& player_pos)
{
	if (m_fCoolTimeRemaining <= 0.0f)
	{
		//printf("attack\n");
		// 플레이어 쪽이 아닌 반대 쪽을 보고 있는지 체크
		XMFLOAT3 xmf3RotateDir = GetLook();
		XMFLOAT3 xmf3Position = GetPosition();
		XMFLOAT3 xmfToPlayer = Vector3::Subtract(xmf3Position, player_pos);
		XMFLOAT3 xmf3Look = GetLook();

		xmf3Look = Vector3::Normalize(xmf3Look);
		xmfToPlayer = Vector3::Normalize(xmfToPlayer);
		double a = xmf3Look.x * xmfToPlayer.z - xmf3Look.z * xmfToPlayer.x;
		double angle = asin(a);

		angle = XMConvertToDegrees(angle);

		if (fabs(angle) > 20.f) {
			state = EnemyState::AIMING;
		}
		else {
			Attack(fTimeElapsed, player_pos);
			ResetCoolTime();
			state = EnemyState::IDLE;
		}
	}
}

void CEnemy::Attack(float fTimeElapsed, XMFLOAT3& player_pos)
{
	for (auto& pl : clients)
	{
		XMFLOAT3 pos = GetPosition();
		XMFLOAT3 xmfToPlayer = Vector3::Subtract(pos, player_pos);
		pl.send_bullet_packet(0, pos, xmfToPlayer);
	}
}

void CEnemy::AvoidAI(float fTimeElapsed)
{
	if (m_fAvoidTime > 1.0f)
	{
		m_fAvoidTime = 0.0f;
		state = EnemyState::IDLE;
	}
	/*
	if (m_bAvoidDir)
	{
		if (m_fAvoidTime < 0.5f) {
			CGameObject::MoveStrafe(fTimeElapsed * 100.f);
		}
		else {
			CGameObject::MoveStrafe(fTimeElapsed * -100.f);
		}
	}
	else
	{
		if (m_fAvoidTime < 0.5f) {
			CGameObject::MoveStrafe(fTimeElapsed * -100.f);
		}
		else {
			CGameObject::MoveStrafe(fTimeElapsed * 100.f);
		}
	}
	*/
	m_fAvoidTime += fTimeElapsed;

	//SendPos();
}

void CEnemy::Rotate(float x, float y, float z)
{
	if (x != 0.0f)
	{
		m_fPitch += x;
		if (m_fPitch > 360.0f) m_fPitch -= 360.0f;
		if (m_fPitch < 0.0f) m_fPitch += 360.0f;
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

	CGameObject::Rotate(x, y, z);
}

void CEnemy::Animate(float fElapsedTime)
{
}

void CEnemy::Animate(float fTimeElapsed, XMFLOAT3 player_pos)
{
	if (isAlive) { AI(fTimeElapsed, player_pos); }
}

void CEnemy::VelocityUpdate(float fTimeElapsed)
{
	

	float fLength = Vector3::Length(m_xmf3Velocity);
	float fMaxVelocity = 500.f;
	if (fLength > fMaxVelocity)
	{
		m_xmf3Velocity.x *= (fMaxVelocity / fLength) * fTimeElapsed * 30;
		m_xmf3Velocity.y *= (fMaxVelocity / fLength) * fTimeElapsed * 30;
		m_xmf3Velocity.z *= (fMaxVelocity / fLength) * fTimeElapsed * 30;
	}

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	XMFLOAT3 LookVelocity = Vector3::ScalarProduct(GetLook(), fTimeElapsed * 30.f, false);
	xmf3Velocity = Vector3::Add(xmf3Velocity, LookVelocity);
	//Move(xmf3Velocity, false);
	XMFLOAT3 xmf3Position = Vector3::Add(GetPosition(), xmf3Velocity);
	SetPosition(xmf3Position);

	float fDeceleration = (50.0f * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}

void CEnemy::SendPos()
{
	for (auto& pl : clients)
	{
		ENEMY_INFO info;
		info.id = id;
		info.Quaternion = GetQuaternion();
		info.pos = GetPosition();
		pl.send_enemy_packet(0, info);
	}
}

XMFLOAT4 CEnemy::GetQuaternion()
{
	XMMATRIX mat = XMLoadFloat4x4(&m_xmf4x4ToParent);
	XMVECTOR vec = XMQuaternionRotationMatrix(mat);
	XMFLOAT4 xmf4;
	XMStoreFloat4(&xmf4, vec);
	return xmf4;
}

//-------------------------------------------------------------------------------------

CMissileEnemy::CMissileEnemy()
{
	hp = 10;			
	m_fCoolTime = 2.0f;		// 공격 간격
	m_fAttackRange = 300.0f;	// 사거리
	damage = 3;

	isAlive = false;
}

CMissileEnemy::~CMissileEnemy()
{
}

bool CMissileEnemy::FireBullet(CGameObject* pLockedObject)
{
	return false;
}

void CMissileEnemy::Animate(float fTimeElapsed)
{
}

void CMissileEnemy::Animate(float fTimeElapsed, XMFLOAT3 player_pos)
{
	CEnemy::Animate(fTimeElapsed, player_pos);
}

//-------------------------------------------------------------------------------------

CLaserEnemy::CLaserEnemy()
{
	hp = 10;
	m_fCoolTime = 2.0f;		// 공격 간격
	m_fAttackRange = 300.0f;	// 사거리
	damage = 3;

	isAlive = false;
}

CLaserEnemy::~CLaserEnemy()
{
}

void CLaserEnemy::Animate(float fTimeElapsed)
{
}

void CLaserEnemy::Animate(float fTimeElapsed, XMFLOAT3 player_pos)
{
	CEnemy::Animate(fTimeElapsed, player_pos);
}

//-------------------------------------------------------------------------------------

CPlasmaCannonEnemy::CPlasmaCannonEnemy()
{
	hp = 10;
	m_fCoolTime = 2.0f;		// 공격 간격
	m_fAttackRange = 300.0f;	// 사거리
	damage = 3;

	isAlive = false;
}

CPlasmaCannonEnemy::~CPlasmaCannonEnemy()
{
}

void CPlasmaCannonEnemy::Animate(float fTimeElapsed)
{
}

void CPlasmaCannonEnemy::Animate(float fTimeElapsed, XMFLOAT3 player_pos)
{
	CEnemy::Animate(fTimeElapsed, player_pos);
}
