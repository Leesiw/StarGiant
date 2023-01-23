#include "Enemy.h"
#include <limits>

CEnemy::CEnemy()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
	
	hp = 10;
	state = EnemyState::IDLE;
}

CEnemy::~CEnemy()
{
}

void CEnemy::AI(float fTimeElapsed, XMFLOAT3 & player_pos)
{
	XMFLOAT3 pos = GetPosition();
	float dist;
	switch (state)
	{
	case EnemyState::IDLE:
		// 플레이어와의 거리
		dist = Vector3::Length(Vector3::Subtract(pos, player_pos));
		if (dist < m_fAttackRange)	// 사거리 충족 안되면 이동
		{
			state = EnemyState::MOVE;
			m_fMoveTimeRemaining = m_fMoveTime;
		}
		else
		{
			if (urdEnemyAI(dree) > 90)
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
}

void CEnemy::MoveAI(float fTimeElapsed, XMFLOAT3& player_pos)
{
	// 테스트 필요

	float fDistance = 500.0 * fTimeElapsed;
	XMFLOAT3 xmf3MovingDirection = GetLook();

	m_fMoveTimeRemaining -= fTimeElapsed;

	if (m_fMoveTimeRemaining > 0.0)
	{
		XMFLOAT3 xmf3Position = GetPosition();
		XMVECTOR xmvPosition = XMLoadFloat3(&xmf3Position);

		XMVECTOR xmvPlayerPosition = XMLoadFloat3(&player_pos);
		XMVECTOR xmvToPlayer = xmvPlayerPosition - xmvPosition;
		xmvToPlayer = XMVector3Normalize(xmvToPlayer);

		XMVECTOR xmvMovingDirection = XMLoadFloat3(&xmf3MovingDirection);
		xmvMovingDirection = XMVector3Normalize(XMVectorLerp(xmvMovingDirection, xmvToPlayer, 0.25f));
		XMStoreFloat3(&xmf3MovingDirection, xmvMovingDirection);

		// 일단 yaw 만 회전하도록
		XMFLOAT3 xmf3RotateDir = xmf3MovingDirection;
		xmf3RotateDir.y = 0.0f;
		xmf3RotateDir = Vector3::Normalize(xmf3RotateDir);

		float fDotProduct = Vector3::DotProduct(XMFLOAT3(0.0f, 1.0f, 0.0f), xmf3RotateDir);
		float fRotationAngle = (fabsf(fDotProduct - 1.0f) < numeric_limits<float>::epsilon()) ? 0.0f : (float)XMConvertToDegrees(acos(fDotProduct));
		Rotate(0, fRotationAngle, 0);

		XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(xmf3MovingDirection, fDistance, false);
		xmf3Position = Vector3::Add(xmf3Position, xmf3Movement);
		SetPosition(xmf3Position);
	}
	else
	{
		state = EnemyState::IDLE;
	}

	SendPos();
}

void CEnemy::AimingAI(float fTimeElapsed, XMFLOAT3& player_pos)
{
	XMFLOAT3 xmf3RotateDir = GetLook();

	XMFLOAT3 xmf3Position = GetPosition();

	XMFLOAT3 xmfToPlayer = Vector3::Subtract(xmf3Position, player_pos);

	// 일단 yaw 만 회전하도록
	xmf3RotateDir.y = 0.0f;
	xmf3RotateDir = Vector3::Normalize(xmf3RotateDir);
	xmfToPlayer.y = 0.0f;
	xmfToPlayer = Vector3::Normalize(xmfToPlayer);

	XMFLOAT3 xmf3RotationAxis = Vector3::CrossProduct(xmfToPlayer, xmf3RotateDir, true);
	float fDotProduct = Vector3::DotProduct(XMFLOAT3(0.0f, 1.0f, 0.0f), xmf3RotateDir);
	
	float fRotationAngle = (fabsf(fDotProduct - 1.0f) < numeric_limits<float>::epsilon()) ? 0.0f : (float)XMConvertToDegrees(acos(fDotProduct));

	if (fRotationAngle == 0.0f)
	{
		state = EnemyState::ATTACK;
	}

	Rotate(0, fRotationAngle, 0);

	SendPos();
}

void CEnemy::AttackAI(float fTimeElapsed, XMFLOAT3& player_pos)
{
	if (m_fCoolTimeRemaining <= 0.0f)
	{
		XMFLOAT3 xmf3RotateDir = GetLook();
		XMFLOAT3 xmf3Position = GetPosition();
		XMFLOAT3 xmfToPlayer = Vector3::Subtract(xmf3Position, player_pos);

		// 플레이어 쪽이 아닌 반대 쪽을 보고 있는지 체크
		xmf3RotateDir.y = 0.0f;
		xmf3RotateDir = Vector3::Normalize(xmf3RotateDir);
		xmfToPlayer.y = 0.0f;
		xmfToPlayer = Vector3::Normalize(xmfToPlayer);

		XMFLOAT3 xmf3RotationAxis = Vector3::CrossProduct(xmfToPlayer, xmf3RotateDir, true);
		if (xmf3RotationAxis.y < 0.0) {
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
	if (m_fAvoidTime > 2.0f)
	{
		m_fAvoidTime = 0.0f;
		state = EnemyState::IDLE;
	}

	if (m_bAvoidDir)
	{
		if (m_fAvoidTime < 1.0f) {
			CGameObject::MoveStrafe(fTimeElapsed * 500.f);
		}
		else {
			CGameObject::MoveStrafe(fTimeElapsed * -500.f);
		}
	}
	else
	{
		if (m_fAvoidTime < 1.0f) {
			CGameObject::MoveStrafe(fTimeElapsed * -500.f);
		}
		else {
			CGameObject::MoveStrafe(fTimeElapsed * 500.f);
		}
	}

	m_fAvoidTime += fTimeElapsed;

	SendPos();
}

void CEnemy::Rotate(float x, float y, float z)
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

	CGameObject::Rotate(x, y, z);
}

void CEnemy::Animate(float fElapsedTime)
{
}

void CEnemy::Animate(float fTimeElapsed, XMFLOAT3 player_pos)
{
	AI(fTimeElapsed, player_pos);
}

void CEnemy::SendPos()
{
	for (auto& pl : clients)
	{
		ENEMY_INFO info;
		info.id = id;
		info.m_fYaw = m_fYaw;
		info.pos = GetPosition();
		pl.send_enemy_packet(0, info);
	}
}

//-------------------------------------------------------------------------------------

CMissileEnemy::CMissileEnemy()
{
	hp = 10;			
	m_fCoolTime = 2.0f;		// 공격 간격
	m_fMoveTime = 1.0f;		// 사거리 안으로 들어가려 움직이는 시간
	m_fAttackRange = 300.0f;	// 사거리
	damage = 3;
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
}

//-------------------------------------------------------------------------------------

CLaserEnemy::CLaserEnemy()
{
	hp = 10;
	m_fCoolTime = 2.0f;		// 공격 간격
	m_fMoveTime = 1.0f;		// 사거리 안으로 들어가려 움직이는 시간
	m_fAttackRange = 300.0f;	// 사거리
	damage = 3;
}

CLaserEnemy::~CLaserEnemy()
{
}

void CLaserEnemy::Animate(float fTimeElapsed)
{
}

void CLaserEnemy::Animate(float fTimeElapsed, XMFLOAT3 player_pos)
{
}

//-------------------------------------------------------------------------------------

CPlasmaCannonEnemy::CPlasmaCannonEnemy()
{
	hp = 10;
	m_fCoolTime = 2.0f;		// 공격 간격
	m_fMoveTime = 1.0f;		// 사거리 안으로 들어가려 움직이는 시간
	m_fAttackRange = 300.0f;	// 사거리
	damage = 3;
}

CPlasmaCannonEnemy::~CPlasmaCannonEnemy()
{
}

void CPlasmaCannonEnemy::Animate(float fTimeElapsed)
{
}

void CPlasmaCannonEnemy::Animate(float fTimeElapsed, XMFLOAT3 player_pos)
{
}
