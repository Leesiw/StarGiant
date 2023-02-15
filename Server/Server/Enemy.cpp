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

void CEnemy::AI(float fTimeElapsed, CPlayer* player)
{

	XMFLOAT3 player_pos = player->GetPosition();
	VelocityUpdate(fTimeElapsed);
	if (m_fCoolTimeRemaining > 0.0f) {
		m_fCoolTimeRemaining -= fTimeElapsed;
	}
	
	XMFLOAT3 pos = GetPosition();
	float dist;
	dist = Vector3::Length(Vector3::Subtract(pos, player_pos));

	if (dist < 150.f)
	{
		XMFLOAT3 ToGo = Vector3::Add(pos, player_pos, -1.f);
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, ToGo, fTimeElapsed * 5.f);
	}
	switch (state)
	{
	case EnemyState::IDLE:
		// 플레이어와의 거리
		
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

			XMFLOAT3 player_vel = player->GetVelocity();
			m_xmf3Destination.y = urdPos3(dree);
			m_xmf3Destination.x += player_pos.x * player_vel.x * 2.f;
			m_xmf3Destination.y += player_pos.y * player_vel.y * 2.f;
			m_xmf3Destination.z += player_pos.z * player_vel.z * 2.f;

			m_fMoveTimeRemaining = m_fMoveTime;
			state = EnemyState::MOVE;
		}
		else
		{
			if (urdEnemyAI(dree) > 30)
			{
				state = EnemyState::ATTACK;
			}
			else 
			{/*
				state = EnemyState::AVOID;
				if (urdEnemyAI(dree) > 50)
				{
					m_bAvoidDir = true;
				}
				else
				{
					m_bAvoidDir = false;
				}*/
			}
		}
		break;
	case EnemyState::AIMING:	// 플레이어 방향을 바라보도록 한다
		AimingAI(fTimeElapsed, player);
		break;
	case EnemyState::ATTACK:
		AttackAI(fTimeElapsed, player);
		break;
	case EnemyState::AVOID:
		AvoidAI(fTimeElapsed);
		break;
	case EnemyState::MOVE:
		MoveAI(fTimeElapsed, player);
		break;
	}

	SendPos();
}

void CEnemy::MoveAI(float fTimeElapsed, CPlayer* player)
{
	if (id == 0) {
		SetPosition(0.f, 0.f, 0.f);
	}
	
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 player_pos = player->GetPosition();
	XMFLOAT3 vec = Vector3::Subtract(player_pos, xmf3Position);
	float dist = Vector3::Length(vec);
	m_fMoveTimeRemaining = 1.f;
	m_bRotateToPlayer = true;
	XMFLOAT3 xmf3Look = GetLook();
	if (m_fMoveTimeRemaining > 0.f) {
		m_fMoveTimeRemaining -= fTimeElapsed;
	}
	else {
		if (Vector3::Length(Vector3::Subtract(m_xmf3Destination, xmf3Position)) < m_fAttackRange)
		{
			state = EnemyState::AIMING;
			return;
		}
		else if (Vector3::Length(Vector3::Subtract(player_pos, m_xmf3Destination)) > m_fAttackRange) {
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

			XMFLOAT3 player_vel = player->GetVelocity();
			m_xmf3Destination.y = urdPos3(dree);
			m_xmf3Destination.x += player_pos.x * player_vel.x * 2.f;
			m_xmf3Destination.y += player_pos.y * player_vel.y * 2.f;
			m_xmf3Destination.z += player_pos.z * player_vel.z * 2.f;
			m_bRotateToPlayer = false;
		}
		else {
			
			float fAngle = Vector3::Angle(vec, xmf3Look);

			if (!m_bRotateToPlayer) {
				m_bRotateToPlayer = true;
			}
		}
		m_fMoveTimeRemaining = m_fMoveTime;
	}
	if (dist > 50.f )
	{
		if (m_bRotateToPlayer) {
			XMMATRIX inv_mat = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World));	
			player_pos = Vector3::TransformCoord(player_pos, inv_mat);
			player_pos = Vector3::Normalize(player_pos);

			float pitch = asin(-player_pos.y);
			float yaw = atan2(player_pos.x, player_pos.z);

			Rotate(pitch * fTimeElapsed * 180.f, yaw * fTimeElapsed * 180.f, 0.f);
		}
		else {
				m_bRotateToPlayer = false;
		}
		//else if(dist > 600.f) {
			//m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, vec, fTimeElapsed * 300.f);
		//}
		
	}
	else {
		state = EnemyState::AIMING;
	}
}

void CEnemy::AimingAI(float fTimeElapsed, CPlayer* player)
{
	XMMATRIX inv_mat = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World));	// 역행렬
	XMFLOAT3 player_pos = player->GetPosition();
	player_pos = Vector3::TransformCoord(player_pos, inv_mat); // 타겟의 위치를 적 자체의 좌표계로 변환
	player_pos = Vector3::Normalize(player_pos);

	float pitch = asin(-player_pos.y);
	float yaw = atan2(player_pos.x, player_pos.z);

	Rotate(pitch * fTimeElapsed * 180.f, yaw * fTimeElapsed * 180.f, 0.f);
}

void CEnemy::AttackAI(float fTimeElapsed, CPlayer* player)
{
	if (m_fCoolTimeRemaining <= 0.0f)
	{
		// 플레이어 쪽이 아닌 반대 쪽을 보고 있는지 체크
		XMFLOAT3 xmf3RotateDir = GetLook();
		XMFLOAT3 xmf3Position = GetPosition();
		XMFLOAT3 xmfToPlayer = Vector3::Subtract(xmf3Position, player->GetPosition());
		XMFLOAT3 xmf3Look = GetLook();

		xmf3Look = Vector3::Normalize(xmf3Look);
		xmfToPlayer = Vector3::Normalize(xmfToPlayer);
		double fAngle = Vector3::Angle(xmf3Look, xmfToPlayer);


		if (fAngle > 30.f) {
			state = EnemyState::AIMING;
		}
		else {
			Attack(fTimeElapsed, player);
			ResetCoolTime();
			state = EnemyState::IDLE;
		}
	}
}

void CEnemy::Attack(float fTimeElapsed, CPlayer* player)
{
	for (auto& pl : clients)
	{
		XMFLOAT3 xmf3Pos = GetPosition();
		XMFLOAT3 player_pos = player->GetPosition();
		XMFLOAT3 xmfToPlayer = Vector3::Subtract(xmf3Pos, player_pos);
		pl.send_bullet_packet(0, xmf3Pos, xmfToPlayer);
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
}

void CEnemy::Rotate(float x, float y, float z)
{
	XMFLOAT3 m_xmf3Look = GetLook(), m_xmf3Up = GetUp(), m_xmf3Right = GetRight();

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

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
}

void CEnemy::Animate(float fElapsedTime)
{
}

void CEnemy::Animate(float fTimeElapsed, CPlayer* player)
{
	if (isAlive) { AI(fTimeElapsed, player); }
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

void CMissileEnemy::Animate(float fTimeElapsed, CPlayer* player)
{
	CEnemy::Animate(fTimeElapsed, player);
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

void CLaserEnemy::Animate(float fTimeElapsed, CPlayer* player)
{
	CEnemy::Animate(fTimeElapsed, player);
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

void CPlasmaCannonEnemy::Animate(float fTimeElapsed, CPlayer* player)
{
	CEnemy::Animate(fTimeElapsed, player);
}

