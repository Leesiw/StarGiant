#include "stdafx.h"
#include "Enemy.h"
#include <cmath>

CEnemyObject::CEnemyObject()
{
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	hp = 10;
}

CEnemyObject::~CEnemyObject()
{

}



void CEnemyObject::Fallowing(float fTimeElapsed, XMFLOAT3 Look)
{

	m_xmf3Position = Vector3::Add(m_xmf3Position, Look, -10.0);
	//m_xmf3Position = Vector3::Add(m_xmf3Position, Vector3::mul(Look, -1.0));
	CGameObject::SetPosition(m_xmf3Position);
	//m_xmf3Position = Vector3::Add(m_xmf3Position, XMFLOAT3(0.f,0.f,-0.1f));

	//this->MoveForward(10.0f);
	CEnemyObject::Animate(fTimeElapsed);
}

void CEnemyObject::Rotate(float x, float y, float z)
{
	if (x != 0.0f)
	{
		m_fPitch += x;
		if (m_fPitch > 89.0f) { m_fPitch = 89.0f; }
		if (m_fPitch < -89.0f) m_fPitch = -89.0f;
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
		if (m_fRoll > 89.0f) { m_fRoll = 89.0f; }
		if (m_fRoll < -89.0f) m_fRoll = -89.0f;
	}

	if (x != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
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
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Up, m_xmf3Look));
	m_xmf3Up = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Look, m_xmf3Right));
}


void CEnemyObject::OnPrepareRender()
{
	m_xmf4x4Transform._11 = m_xmf3Right.x; m_xmf4x4Transform._12 = m_xmf3Right.y; m_xmf4x4Transform._13 = m_xmf3Right.z;
	m_xmf4x4Transform._21 = m_xmf3Up.x; m_xmf4x4Transform._22 = m_xmf3Up.y; m_xmf4x4Transform._23 = m_xmf3Up.z;
	m_xmf4x4Transform._31 = m_xmf3Look.x; m_xmf4x4Transform._32 = m_xmf3Look.y; m_xmf4x4Transform._33 = m_xmf3Look.z;
	m_xmf4x4Transform._41 = m_xmf3Position.x; m_xmf4x4Transform._42 = m_xmf3Position.y; m_xmf4x4Transform._43 = m_xmf3Position.z;

	UpdateTransform(NULL);
}

void CEnemyObject::Animate(float fElapsedTime)
{
	UpdateBoundingBox();
}

void CEnemyObject::Animate(float fElapsedTime, XMFLOAT3 player_pos)
{
	if (m_fResetWaitingTime > 0.0f) {
		m_fResetWaitingTime -= fElapsedTime;
		if (m_fResetWaitingTime < 0.0f) {
			hp = 10;
		}
		else {
			return;
		}
	}

	XMFLOAT3 xmf3Position = GetPosition();
	XMVECTOR xmvPosition = XMLoadFloat3(&xmf3Position);

	xmf3Position.z += 100.0;	// 플레이어와 Enemy 위치 차이 커버...

	float x = player_pos.x - xmf3Position.x;
	float y = player_pos.y - xmf3Position.y;
	float z = player_pos.z - xmf3Position.z;

	float yaw = 180.0 + XMConvertToDegrees(atan2(x, z));
	/*
	XMVECTOR xmvPlayerPosition = XMLoadFloat3(&player_pos);
	XMVECTOR xmvToPlayer = xmvPlayerPosition - xmvPosition;
	xmvToPlayer = XMVector3Normalize(xmvToPlayer);
	
	XMFLOAT3 xmf3ToPlayer{};

	XMStoreFloat3(&xmf3ToPlayer, xmvToPlayer);
	//float roll;// = XMConvertToDegrees(asin(xmf3ToPlayer.y));
	
	
	//float yaw = 180.0 + XMConvertToDegrees(atan2(xmf3ToPlayer.x, xmf3ToPlayer.z));
	*/
	Rotate(0, yaw - m_fYaw, 0);//roll - m_fRoll);


	float distance = x * x + y * y;		// +z * z;

	if (player_pos.y > xmf3Position.y) {
		if (player_pos.y < xmf3Position.y + 1.0f) {
			MoveUp(player_pos.y - xmf3Position.y);
		}
		else {
			MoveUp();
		}
	}
	else if (player_pos.y < xmf3Position.y) {

		if (player_pos.y < xmf3Position.y - 1.0f) {
			MoveUp(player_pos.y - xmf3Position.y);
		}
		else {
			MoveUp(-1.0f);
		}
	}

	if (distance > 4000) {
		Move(m_xmf3Look, -fElapsedTime * 80.0);
	}
	
	UpdateBoundingBox();
}

void CEnemyObject::SetResetWaitingTime()
{
	m_fResetWaitingTime = m_fResetDelayTime;
}

//============================================================

CEnemyShip::CEnemyShip()
{

	boundingbox = BoundingOrientedBox{ XMFLOAT3(-0.000000f, -0.000000f, -0.000096f), XMFLOAT3(15.5f, 15.5f, 3.90426f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };

	SetPosition(XMFLOAT3{ 0, 0, 130 });
	Rotate(0, 90, 0.0f);
	SetScale(20, 20, 20);

	OnInitialize();

	for (int i = 0; i < BULLETS; i++)
	{
		m_ppBullets[i] = new CBulletObject(m_fBulletEffectiveRange);
		m_ppBullets[i]->SetScale(1.0f, 1.0f, 1.0f);

		m_ppBullets[i]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
		//m_ppBullets[i]->SetRotationSpeed(360.0f);
		m_ppBullets[i]->SetMovingSpeed(320.0f);
		m_ppBullets[i]->SetActive(false);
		m_ppBullets[i]->OnInitialize();

		m_ppBullets[i]->UpdateBoundingBox();
	}
}

CEnemyShip::~CEnemyShip()
{
}

bool CEnemyShip::FireBullet(CGameObject* pLockedObject)
{

	if (m_fFireWaitingTime > 0.0f)
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



		//xmf3FirePosition = Vector3::Add(Vector3::ScalarProduct(xmf3Right, -40.0f, false), Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 70.0f, false)));
		//xmf3Position = Vector3::Add(Vector3::ScalarProduct(xmf3Right, -40.0f, false), Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 70.0f, false)));

		//xmf3FirePosition = Vector3::Add(Vector3::ScalarProduct(xmf3Right, -40.0f, false), Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Up, -10.0f, false)));
		//xmf3FirePosition.x = xmf3FirePosition.x - 50;

		//cout <<"x,y,z" << xmf3FirePosition.x<<"\t"<< xmf3FirePosition.y << "\t"<< xmf3FirePosition.z;


	//	XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 6.0f, false));

		//pBulletObject->m_xmf4x4Transform = m_xmf4x4Transform;
		//pBulletObject->m_xmf4x4World = m_xmf4x4World;

		pBulletObject->Rotate(m_fPitch, m_fYaw, m_fRoll);
		pBulletObject->m_fPitch = m_fPitch; pBulletObject->m_fYaw = m_fYaw; pBulletObject->m_fRoll = m_fRoll;
		pBulletObject->SetFirePosition(xmf3FirePosition);
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetActive(true);
		pBulletObject->UpdateBoundingBox();

		//for (int i = 0; i < BULLETS; i++)
		//{
		//	m_ppBullets[i]->UpdateBoundingBox();
		//}

		m_fFireWaitingTime = m_fFireDelayTime * 1.0f;

		if (pLockedObject)
		{
			pBulletObject->m_pLockedObject = pLockedObject;
		}
	}
	return true;
}


void CEnemyShip::Animate(float fTimeElapsed)
{
	//m_xmf3Position = Vector3::Add(m_xmf3Position, XMFLOAT3(0.f, 0.f, -0.1f));

	if (m_fFireWaitingTime > 0.0f)
		m_fFireWaitingTime -= fTimeElapsed;

	//this->MoveForward(10.0f);
	CEnemyObject::Animate(fTimeElapsed);
}


void CEnemyShip::Animate(float fTimeElapsed, XMFLOAT3 player_pos)
{
	//m_xmf3Position = Vector3::Add(m_xmf3Position, XMFLOAT3(0.f, 0.f, -0.1f));

	if (m_fFireWaitingTime > 0.0f)
		m_fFireWaitingTime -= fTimeElapsed;

	//this->MoveForward(10.0f);
	CEnemyObject::Animate(fTimeElapsed, player_pos);
}

void CEnemyShip::OnPrepareRender()
{
	CEnemyObject::OnPrepareRender();
}

