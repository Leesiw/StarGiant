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

	VelocityUpdate(fTimeElapsed, player);
	XMFLOAT3 player_pos = player->GetPosition();
	
	if (m_fCoolTimeRemaining > 0.0f) {
		m_fCoolTimeRemaining -= fTimeElapsed;
	}
	
	XMFLOAT3 pos = GetPosition();
	float dist;
	dist = Vector3::Length(Vector3::Subtract(pos, player_pos));
	
	if (dist < 100.f)
	{
		if (type != EnemyType::MISSILE) {
			XMFLOAT3 ToGo = Vector3::Subtract(pos, player_pos);
			ToGo = Vector3::ScalarProduct(ToGo, 100.f);
			ToGo = Vector3::Add(player_pos, ToGo);
			SetPosition(ToGo);
		}
		else if (attack_step != 2)
		{
			XMFLOAT3 ToGo = Vector3::Subtract(pos, player_pos);
			m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, ToGo, 5.f);
		}
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

			XMFLOAT3 player_vel = player->GetLook();
			m_xmf3Destination.y = urdPos3(dree);

			m_fMoveTimeRemaining = m_fMoveTime;
			state = EnemyState::MOVE;
		}
		else
		{
			if (type == EnemyType::MISSILE) 
			{
				attack_step = 0;
			}
			state = EnemyState::ATTACK;
		}
		break;
	case EnemyState::AIMING:	// 플레이어 방향을 바라보도록 한다
		AimingAI(fTimeElapsed, player);
		break;
	case EnemyState::ATTACK:
		AttackAI(fTimeElapsed, player);
		break;
	case EnemyState::MOVE:
		MoveAI(fTimeElapsed, player);
		break;
	}

	SendPos();
}

void CEnemy::MoveAI(float fTimeElapsed, CPlayer* player)
{	
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 player_pos = player->GetPosition();

	m_xmf3Destination.x += player_pos.x;
	m_xmf3Destination.y += player_pos.y;
	m_xmf3Destination.z += player_pos.z;
	XMFLOAT3 vec = Vector3::Subtract(m_xmf3Destination, xmf3Position);
	float dist = Vector3::Length(vec);
	XMFLOAT3 xmf3Look = GetLook();

	if (dist > 50.f )
	{
		XMMATRIX inv_mat = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World));
		XMFLOAT3 new_pos = Vector3::TransformCoord(m_xmf3Destination, inv_mat);
		new_pos = Vector3::Normalize(new_pos);

		float pitch = asin(-new_pos.y);
		float yaw = atan2(new_pos.x, new_pos.z);

		Rotate(pitch * fTimeElapsed * 360.f, yaw * fTimeElapsed * 360.f, 0.f);
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, GetLook() , fTimeElapsed * dist);
		UpdateTransform();

		//Rotate(0.f, 0.f, 30.f * fTimeElapsed);
	}
	else {
		state = EnemyState::AIMING;
	}

	m_xmf3Destination.x -= player_pos.x;
	m_xmf3Destination.y -= player_pos.y;
	m_xmf3Destination.z -= player_pos.z;
}

void CEnemy::AimingAI(float fTimeElapsed, CPlayer* player)
{
	XMFLOAT3 player_pos = player->GetPosition();
	XMMATRIX inv_mat = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World));	// 역행렬
	
	player_pos = Vector3::TransformCoord(player_pos, inv_mat); // 타겟의 위치를 적 자체의 좌표계로 변환
	player_pos = Vector3::Normalize(player_pos);

	float pitch = asin(-player_pos.y);
	float yaw = atan2(player_pos.x, player_pos.z);

	if (yaw < 0.05 && pitch < 0.05) {
		if (type == EnemyType::MISSILE)
		{
			attack_step = 0;
		}
		state = EnemyState::ATTACK;
	}

	Rotate(pitch * fTimeElapsed * 180.f, yaw * fTimeElapsed * 180.f, 0.f);

}

void CEnemy::AttackAI(float fTimeElapsed, CPlayer* player)
{
	if (m_fCoolTimeRemaining <= 0.0f)
	{
		// 플레이어 쪽이 아닌 반대 쪽을 보고 있는지 체크
		XMFLOAT3 xmf3RotateDir = GetLook();
		XMFLOAT3 xmf3Position = GetPosition();
		XMFLOAT3 xmfToPlayer = Vector3::Subtract(player->GetPosition(), xmf3Position);
		XMFLOAT3 xmf3Look = GetLook();

		xmf3Look = Vector3::Normalize(xmf3Look);
		xmfToPlayer = Vector3::Normalize(xmfToPlayer);
		double fAngle = Vector3::Angle(xmf3Look, xmfToPlayer);

		if (fAngle > 10.f) {
			state = EnemyState::AIMING;
		}
		else {
			float length = Vector3::Length(m_xmf3Destination);
			if (length < m_fAttackRange) {
				state = EnemyState::IDLE;
				Attack(fTimeElapsed, player);
				ResetCoolTime();
			}
			else {
				state = EnemyState::MOVE;
			}
		}
	}
	else {

		// 낮은 확률로 랜덤 움직임
		if (!::IsZero(m_xmf3Velocity.x)) return;
		if (urdEnemyAI(dree) < 10) {
			int dir = urdEnemyAI(dree);
			if (dir < 25) {
				m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, player->GetLook(), 100.f);
			}
			else if (dir < 50) {
				m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, player->GetLook(), -100.f);
			}
			else if (dir < 75) {
				m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, GetLook(), 100.f);
			}
			else {
				m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, GetLook(), -100.f);
			}
		}
	}
}

void CEnemy::Attack(float fTimeElapsed, CPlayer* player)
{
	XMFLOAT3 xmf3Pos = GetPosition();
	XMFLOAT3 player_pos = player->GetPosition();
	XMFLOAT3 xmfToPlayer = Vector3::Subtract(player_pos, xmf3Pos);

	for (auto& pl : clients)
	{
		pl.send_bullet_packet(0, xmf3Pos, xmfToPlayer);
	}

	if (urdEnemyAI(dree) < hit_probability) {	// 플레이어에게 공격 명중
		if (player->GetHP() <= 0) { return; }
		player->SetHP(player->GetHP() - damage);
		for (auto& pl : clients)
		{
			pl.send_bullet_hit_packet(0, -1, player->GetHP());
		}
	}
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

void CEnemy::VelocityUpdate(float fTimeElapsed, CPlayer* player)
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

	m_fMoveTimeRemaining -= fTimeElapsed;
	if (m_fMoveTimeRemaining < 0.f) { m_fSpeed = urdPos2(dree); m_fMoveTimeRemaining = 5.f; }
	XMFLOAT3 LookVelocity = Vector3::ScalarProduct(player->GetLook(), fTimeElapsed * 100.f, false);
	xmf3Velocity = Vector3::Add(LookVelocity, xmf3Velocity);
	XMFLOAT3 xmf3Position = Vector3::Add(GetPosition(), xmf3Velocity);
	SetPosition(xmf3Position);

	float fDeceleration = (100.0f * fTimeElapsed);
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
	type = EnemyType::MISSILE;
	hp = 10;			
	m_fCoolTime = 2.0f;		// 공격 간격
	m_fAttackRange = 300.0f;	// 사거리
	damage = 3;

	boundingbox = BoundingOrientedBox{ XMFLOAT3(0.f, -0.981136f, -3.06843f), XMFLOAT3(7.79472f, 8.2804f, 9.19255f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };

	isAlive = false;
}

CMissileEnemy::~CMissileEnemy()
{
}

void CMissileEnemy::AttackAI(float fTimeElapsed, CPlayer* player)
{
	if (attack_step == 0) 
	{
		XMFLOAT3 player_pos = player->GetPosition();
		XMMATRIX inv_mat = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World));	// 역행렬

		player_pos = Vector3::TransformCoord(player_pos, inv_mat); // 타겟의 위치를 적 자체의 좌표계로 변환
		player_pos = Vector3::Normalize(player_pos);

		float pitch = asin(-player_pos.y);
		float yaw = atan2(player_pos.x, player_pos.z);

		Rotate(pitch * fTimeElapsed * 180.f, yaw * fTimeElapsed * 180.f, 0.f);

		if (!::IsZero(m_xmf3Velocity.x)) return;
		if (urdEnemyAI(dree) < 10) {

			int dir = urdEnemyAI(dree);
			if (dir < 10) {
				XMFLOAT3 pl_pos = player->GetPosition();
				XMFLOAT3 dir{ urdPos3(dree), urdPos3(dree), urdPos3(dree) };
				dir = Vector3::Normalize(dir);
				m_xmf3Destination = Vector3::Add(pl_pos, dir, 200.f);

				attack_step = 1;
			}
			else if (dir < 25) {
				m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, player->GetLook(), 100.f);
			}
			else if (dir < 50) {
				m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, player->GetLook(), -100.f);
			}
			else if (dir < 75) {
				m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, GetLook(), 100.f);
			}
			else {
				m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, GetLook(), -100.f);
			}
		}
	}
	else if (attack_step == 1)
	{
		XMFLOAT3 xmf3Position = GetPosition();
		

		XMFLOAT3 player_pos = player->GetPosition();
		XMFLOAT3 vec = Vector3::Subtract(player_pos, xmf3Position);
		float dist = Vector3::Length(vec);

		XMMATRIX inv_mat = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World));	// 역행렬

		player_pos = Vector3::TransformCoord(player_pos, inv_mat); // 타겟의 위치를 적 자체의 좌표계로 변환
		player_pos = Vector3::Normalize(player_pos);

		float pitch = asin(-player_pos.y);
		float yaw = atan2(player_pos.x, player_pos.z);

		Rotate(pitch * fTimeElapsed * 180.f, yaw * fTimeElapsed * 180.f, 0.f);

		if (dist < 200.f)
		{
			m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, GetLook(), -10.f * fTimeElapsed);
		}
		else {
			m_fAttackTimeRemaining = m_fAttackTime;
			attack_step = 2;
		}
	}
	else 
	{
		if (m_fAttackTimeRemaining > 0.f) {
			m_fAttackTimeRemaining -= fTimeElapsed;
			
		}
		else {
			attack_step = 0;
			state = EnemyState::IDLE;
			return;
		}
		
		XMFLOAT3 xmf3Position = GetPosition();
		XMFLOAT3 p_pos = player->GetPosition();

		XMMATRIX inv_mat = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World));
		XMFLOAT3 new_pos = Vector3::TransformCoord(p_pos, inv_mat);
		new_pos = Vector3::Normalize(new_pos);

		float pitch = asin(-new_pos.y);
		float yaw = atan2(new_pos.x, new_pos.z);
		Rotate(pitch * fTimeElapsed * 360.f, yaw * fTimeElapsed * 360.f, 0.f);
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, GetLook(), fTimeElapsed * 200.f);		
	}

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
	type = EnemyType::LASER;
	hp = 10;
	m_fCoolTime = 2.0f;		// 공격 간격
	m_fAttackRange = 300.0f;	// 사거리
	damage = 3;

	boundingbox = BoundingOrientedBox{ XMFLOAT3(0.0188516f, -0.336726f, -0.0989028f), XMFLOAT3(1.739f, 1.51158f, 1.67118f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };

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
	type = EnemyType::PLASMACANNON;
	hp = 10;
	m_fCoolTime = 2.0f;		// 공격 간격
	m_fAttackRange = 300.0f;	// 사거리
	damage = 3;

	boundingbox = BoundingOrientedBox{ XMFLOAT3(0.f, 3.99769f, 7.3113f), XMFLOAT3(17.6804f, 11.0108f, 47.4969f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };

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

