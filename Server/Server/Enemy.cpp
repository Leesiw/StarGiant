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
			switch (urdEnemyAI(dree) % 4) {
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
			m_fMoveTimeRemaining = m_fMoveTime;
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

	XMFLOAT3 xmf3MovingDirection = GetLook();

	//m_fMoveTimeRemaining -= fTimeElapsed;

	XMFLOAT3 xmf3Position = GetPosition();

	m_xmf3Destination.x += player_pos.x;
	m_xmf3Destination.y += player_pos.y;
	m_xmf3Destination.z += player_pos.z;

	float dist = Vector3::Length(Vector3::Subtract(xmf3Position, m_xmf3Destination));
	if (dist > 100.0f) {
		if (m_fSpeed < 500.f) {
			m_fSpeed += 100.f * fTimeElapsed;
		}
	}
	else {
		if (m_fSpeed > 10.f) {
			m_fSpeed -= 100.f * fTimeElapsed;
		}
	}

	if (dist > 50.f)
	{
		XMFLOAT3 xmf3Look = GetLook();
		XMFLOAT3 ToDestination = Vector3::Subtract(xmf3Position, m_xmf3Destination);
		
		xmf3Look = Vector3::Normalize(xmf3Look);
		ToDestination = Vector3::Normalize(ToDestination);
		double a = xmf3Look.x * ToDestination.z - xmf3Look.z * ToDestination.x;
		double angle = asin(a);

		angle = XMConvertToDegrees(angle);
		double rotate_angle = fTimeElapsed * 180.0f;

		if (fabs(angle) > rotate_angle) {
			if (angle < 0) { Rotate(0, -rotate_angle, 0); }
			else{ Rotate(0, rotate_angle, 0); }
		}
		else {
			Rotate(0, angle, 0);
			MoveForward(fTimeElapsed * m_fSpeed);
		}
		//Rotate(0, angle / 50.0f, 0);

		

		float y_dist = -m_xmf3Destination.y + xmf3Position.y;

		if (fabs(y_dist) > fTimeElapsed * m_fSpeed) {
			if (y_dist < 0) { MoveUp(fTimeElapsed * m_fSpeed); }
			else { MoveUp(-fTimeElapsed * m_fSpeed); }
		}
		m_xmf3Destination.x -= player_pos.x;
		m_xmf3Destination.y -= player_pos.y;
		m_xmf3Destination.z -= player_pos.z;
	}
	else
	{
		state = EnemyState::AIMING;
	}

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
		// �÷��̾� ���� �ƴ� �ݴ� ���� ���� �ִ��� üũ
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
