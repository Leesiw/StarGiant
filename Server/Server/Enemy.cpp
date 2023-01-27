#include "Enemy.h"
#include <limits>

CEnemy::CEnemy()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
	
	hp = 10;
	state = EnemyState::IDLE;

	isAlive = false;
}

CEnemy::~CEnemy()
{
}

void CEnemy::AI(float fTimeElapsed, XMFLOAT3 & player_pos)
{
	if (m_fCoolTimeRemaining > 0.0f) {
		m_fCoolTimeRemaining -= fTimeElapsed;
	}
	XMFLOAT3 pos = GetPosition();
	float dist;
	switch (state)
	{
	case EnemyState::IDLE:
		// �÷��̾���� �Ÿ�
		dist = Vector3::Length(Vector3::Subtract(pos, player_pos));
		if (dist > m_fAttackRange)	// ��Ÿ� ���� �ȵǸ� �̵�
		{
			state = EnemyState::MOVE;
			m_fMoveTimeRemaining = m_fMoveTime;
		}
		else
		{
			if (urdEnemyAI(dree) > 90)
			{
				//state = EnemyState::ATTACK;
				state = EnemyState::AVOID;
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
	case EnemyState::AIMING:	// �÷��̾� ������ �ٶ󺸵��� �Ѵ�
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
	// �׽�Ʈ �ʿ�
	
	float fDistance = 300.0 * fTimeElapsed;
	XMFLOAT3 xmf3MovingDirection = GetLook();

	m_fMoveTimeRemaining -= fTimeElapsed;

	if (m_fMoveTimeRemaining > 0.0)
	{
		XMFLOAT3 xmf3Position = GetPosition();
		XMFLOAT3 xmf3Look = GetLook();
		XMFLOAT3 ToPlayer = Vector3::Subtract(xmf3Position, player_pos);

		xmf3Look = Vector3::Normalize(xmf3Look);
		ToPlayer = Vector3::Normalize(ToPlayer);
		double a = xmf3Look.x * ToPlayer.z - xmf3Look.z * ToPlayer.x;
		double angle = asin(a);

		angle = XMConvertToDegrees(angle);
		double rotate_angle = fTimeElapsed * 30.0f;

		if (fabs(angle) > rotate_angle) {
			if (angle < 0) { Rotate(0, -rotate_angle, 0); }
			else{ Rotate(0, rotate_angle, 0); }
		}

		MoveForward(fTimeElapsed * 100.0f);
		float y_dist = xmf3Position.y - player_pos.y;

		if (fabs(y_dist) > fTimeElapsed * 100.0f) {
			if (y_dist < 0) { MoveUp(fTimeElapsed * 100.0f); }
			else { MoveUp(-fTimeElapsed * 100.0f); }
		}
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

	// �ϴ� yaw �� ȸ���ϵ���
	xmf3RotateDir.y = 0.0f;
	xmf3RotateDir = Vector3::Normalize(xmf3RotateDir);
	xmfToPlayer.y = 0.0f;
	xmfToPlayer = Vector3::Normalize(xmfToPlayer);

	XMFLOAT3 xmf3RotationAxis = Vector3::CrossProduct(xmfToPlayer, xmf3RotateDir, true);
	float fDotProduct = Vector3::DotProduct(GetLook(), xmf3RotateDir);
	
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

		// �÷��̾� ���� �ƴ� �ݴ� ���� ���� �ִ��� üũ
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
	m_fCoolTime = 2.0f;		// ���� ����
	m_fMoveTime = 1.0f;		// ��Ÿ� ������ ���� �����̴� �ð�
	m_fAttackRange = 300.0f;	// ��Ÿ�
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
	m_fCoolTime = 2.0f;		// ���� ����
	m_fMoveTime = 1.0f;		// ��Ÿ� ������ ���� �����̴� �ð�
	m_fAttackRange = 300.0f;	// ��Ÿ�
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
	m_fCoolTime = 2.0f;		// ���� ����
	m_fMoveTime = 1.0f;		// ��Ÿ� ������ ���� �����̴� �ð�
	m_fAttackRange = 300.0f;	// ��Ÿ�
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
