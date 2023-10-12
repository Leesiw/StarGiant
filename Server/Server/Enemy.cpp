#include "Enemy.h"
#include "SceneManager.h"
#include <limits>

extern SceneManager scene_manager;

CEnemy::CEnemy()
{
	CGameObject::CGameObject();

	m_fCoolTime = 2.0f;
	m_fCoolTimeRemaining = 0.0f;

	m_xmf3Velocity = XMFLOAT3(0.f, 0.f, 0.f);
	m_xmf3Destination = XMFLOAT3(0.f, 0.f, 100.f);

	damage = 3;

	ZeroMemory(&enemy_flags, 1);

	hp = 10;
	state = EnemyState::MOVE;
}

CEnemy::~CEnemy()
{
}

void CEnemy::AI(float fTimeElapsed, CAirplanePlayer* player)
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
		XMFLOAT3 ToGo = Vector3::Subtract(pos, player_pos);
		ToGo = Vector3::ScalarProduct(ToGo, 100.f);
		ToGo = Vector3::Add(player_pos, ToGo);
		SetPosition(ToGo);
	}
	
	switch (state)
	{
	case EnemyState::AIMING:	// �÷��̾� ������ �ٶ󺸵��� �Ѵ�
		AimingAI(fTimeElapsed, player);
		break;
	case EnemyState::MOVE:
		MoveAI(fTimeElapsed, player);
		break;
	}

	//SendPos();
}

void CEnemy::MoveAI(float fTimeElapsed, CAirplanePlayer* player)
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
		XMFLOAT3 ToGo = Vector3::Subtract(pos, player_pos);
		ToGo = Vector3::ScalarProduct(ToGo, 100.f);
		ToGo = Vector3::Add(player_pos, ToGo);
		SetPosition(ToGo);
		enemy_flags |= option3;
	}


	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 destination = Vector3::Add(m_xmf3Destination, player_pos);

	XMFLOAT3 vec = Vector3::Subtract(destination, xmf3Position);
	dist = Vector3::Length(vec);

	if (dist < 50.f)
	{
		state = EnemyState::AIMING;
		return;
	}

	LookAtPosition(fTimeElapsed, destination);

	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, GetLook(), fTimeElapsed * 200.f);
	UpdateTransform();
}

void CEnemy::AimingAI(float fTimeElapsed, CAirplanePlayer* player)
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
		XMFLOAT3 ToGo = Vector3::Subtract(pos, player_pos);
		ToGo = Vector3::ScalarProduct(ToGo, 100.f);
		ToGo = Vector3::Add(player_pos, ToGo);
		SetPosition(ToGo);
		enemy_flags |= option3;
	}
	else if (dist > 400.f)
	{
		state = EnemyState::MOVE;
		return;
	}

	LookAtPosition(fTimeElapsed, player_pos);	// �÷��̾ ������ ȸ��
	UpdateTransform();

	if (m_fCoolTimeRemaining <= 0.0f)
	{
		if (enemy_flags & option1)
		{
			Attack(fTimeElapsed, player);
		}
	}
}

void CEnemy::Attack(float fTimeElapsed, CAirplanePlayer* player)
{
	XMFLOAT3 xmf3Pos = GetPosition();
	XMFLOAT3 player_pos = player->GetPosition();
	XMFLOAT3 xmfToPlayer = Vector3::Subtract(player_pos, xmf3Pos);
	xmfToPlayer = Vector3::TransformCoord(xmfToPlayer, Matrix4x4::RotationAxis(GetUp(), urdAngle(dree)));

	XMFLOAT3 player_vel = player->GetVelocity();
	float h_probability = 50;
	if (Vector3::Length(player_vel) > 0.f) { h_probability -= 20; }

	if (urdEnemyAI(dree) < h_probability) {	// �÷��̾�� ���� ����
		if (player->GetHP() <= 0) { return; }
		player->GetAttack(damage);

		{
			SC_BULLET_PACKET p{};

			p.size = sizeof(SC_BULLET_PACKET);
			p.type = SC_BULLET;

			p.data.direction = xmfToPlayer;
			p.data.pos = xmf3Pos;
			p.attack_time = 0;

			scene_manager.Send(scene_num, (char*)&p);
		}

		{
			SC_BULLET_HIT_PACKET p{};
			p.size = sizeof(SC_BULLET_HIT_PACKET);
			p.type = SC_BULLET_HIT;
			p.data.id = -1;
			p.data.hp = player->GetHP();

			scene_manager.Send(scene_num, (char*)&p);
		}
	}

	ResetCoolTime();
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

void CEnemy::LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos)
{
	XMFLOAT3 new_pos = pos;
	XMMATRIX inv_mat = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World));	// �����

	new_pos = Vector3::TransformCoord(new_pos, inv_mat); // Ÿ���� ��ġ�� �� ��ü�� ��ǥ��� ��ȯ
	if (Vector3::Length(new_pos) > 0.0001f) {
		new_pos = Vector3::Normalize(new_pos);
	}
	

	float pitch = XMConvertToDegrees(asin(-new_pos.y));
	float yaw = XMConvertToDegrees(atan2(new_pos.x, new_pos.z));

	float rotate_angle = fTimeElapsed * 90.f;

	XMFLOAT3 p_y_r{ pitch, yaw, 0.f };
	if (Vector3::Length(p_y_r) > rotate_angle) {
		if (enemy_flags & option1) { enemy_flags &= ~option1; };
		p_y_r = Vector3::Normalize(p_y_r);
		Rotate(p_y_r.x * rotate_angle, p_y_r.y * rotate_angle, 0.f);
	}
	else {
		Rotate(pitch, yaw, 0.f);
		if (!(enemy_flags & option1)) { enemy_flags |= option1; };
	}
}


void CEnemy::Reset()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();

	//enemy_flags &= ~option2;

	ZeroMemory(&enemy_flags, 1);

	hp = 10;
	state = EnemyState::MOVE;
}

void CEnemy::SetisAliveTrue()
{
	enemy_flags |= option0;
}

void CEnemy::SetisAliveFalse()
{
	enemy_flags &= ~option0;
}


void CEnemy::SetAttackTimerFalse()
{
	enemy_flags &= ~option2;

}

void CEnemy::SetAttackTimerTrue()
{
	enemy_flags |= option2;

}

bool CEnemy::GetAttackTimer() { return enemy_flags & option2; }

void CEnemy::SetStatus(MissionType cur_mission)
{
	hp = 10;
	damage = 3;
}

void CEnemy::SetDestination()
{
	switch (urdEnemyAI(dree) % 4) {	// ������ ����
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
}

void CEnemy::Animate(float fElapsedTime)
{
}

void CEnemy::Animate(float fTimeElapsed, CAirplanePlayer* player)
{
	if (enemy_flags & option0) { AI(fTimeElapsed, player); }
}

void CEnemy::VelocityUpdate(float fTimeElapsed, CAirplanePlayer* player)
{
	float fLength = Vector3::Length(m_xmf3Velocity);
	if (IsZero(fLength)) { return; }

	if (fLength > 200.f)
	{
		m_xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, 200.f, true);
	}

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);

//	XMFLOAT3 LookVelocity = Vector3::ScalarProduct(player->GetLook(), fTimeElapsed * 50.f, false);
//	xmf3Velocity = Vector3::Add(LookVelocity, xmf3Velocity);
	XMFLOAT3 xmf3Position = Vector3::Add(GetPosition(), xmf3Velocity);
	SetPosition(xmf3Position);

	enemy_flags |= option3;

	float fDeceleration = (100.0f * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	if (Vector3::Length(m_xmf3Velocity) > fDeceleration) {
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
	}
	else {
		m_xmf3Velocity.x = 0; m_xmf3Velocity.y = 0; m_xmf3Velocity.z = 0;
	}
}

void CEnemy::SetIsMoveFalse() {
	enemy_flags &= ~option3;
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
	CEnemy::CEnemy();

	type = EnemyType::MISSILE;
	hp = 5;			
	m_fCoolTime = 15.0f;		// ���� ����
	damage = 6;

	enemy_flags &= ~option2;

	boundingbox = BoundingOrientedBox{ XMFLOAT3(0.f, -0.981136f, -3.06843f), XMFLOAT3(11.69208f, 12.4206f, 13.788825f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
}

CMissileEnemy::~CMissileEnemy()
{
}

void CMissileEnemy::Attack(float fTimeElapsed, CAirplanePlayer* player)
{
}

void CMissileEnemy::SetStatus(MissionType cur_mission)
{
	hp = levels[cur_mission].missile().max_hp();
	damage = levels[cur_mission].missile().atk();
}

void CMissileEnemy::Animate(float fTimeElapsed)
{
}

void CMissileEnemy::Animate(float fTimeElapsed, CAirplanePlayer* player)
{
	CEnemy::Animate(fTimeElapsed, player);
}

//-------------------------------------------------------------------------------------

CLaserEnemy::CLaserEnemy()
{
	CEnemy::CEnemy();

	type = EnemyType::LASER;
	hp = 3;
	m_fCoolTime = 5.0f;		// ���� ����
	damage = 2;

	enemy_flags &= ~option2;

	boundingbox = BoundingOrientedBox{ XMFLOAT3(0.037703f, -0.673452f, -0.197806f), XMFLOAT3(5.217003f, 4.534749f, 5.013531f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
}

CLaserEnemy::~CLaserEnemy()
{
}

void CLaserEnemy::SetStatus(MissionType cur_mission)
{
	hp = levels[cur_mission].laser().max_hp();
	damage = levels[cur_mission].laser().atk();
}

void CLaserEnemy::Animate(float fTimeElapsed)
{
}

void CLaserEnemy::Animate(float fTimeElapsed, CAirplanePlayer* player)
{
	CEnemy::Animate(fTimeElapsed, player);
}

//-------------------------------------------------------------------------------------

CPlasmaCannonEnemy::CPlasmaCannonEnemy()
{
	CEnemy::CEnemy();

	type = EnemyType::PLASMACANNON;
	hp = 10;
	m_fCoolTime = 8.0f;		// ���� ����
	damage = 4;

	boundingbox = BoundingOrientedBox{ XMFLOAT3(0.f, 3.99769f, 7.3113f), XMFLOAT3(17.6804f, 11.0108f, 47.4969f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
}

CPlasmaCannonEnemy::~CPlasmaCannonEnemy()
{
}

void CPlasmaCannonEnemy::SetStatus(MissionType cur_mission)
{
	hp = levels[cur_mission].plasmacannon().max_hp();
	damage = levels[cur_mission].plasmacannon().atk();
}

void CPlasmaCannonEnemy::Animate(float fTimeElapsed)
{
}

void CPlasmaCannonEnemy::Animate(float fTimeElapsed, CAirplanePlayer* player)
{
	CEnemy::Animate(fTimeElapsed, player);
}

