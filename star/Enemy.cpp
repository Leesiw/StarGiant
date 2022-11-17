#include "stdafx.h"
#include "Enemy.h"
#include "Shader.h"



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

}

void CEnemyObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, NULL);
}


//============================================================

CEnemyShip::CEnemyShip(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	CGameObject* pGameObject = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/ship.bin");
	pGameObject->SetPosition(0, 0, 130);
	pGameObject->Rotate(0, 90, 0.0f);
	pGameObject->SetScale(20, 20, 20);

	
	SetChild(pGameObject, true);

	OnInitialize();
}

CEnemyShip::~CEnemyShip()
{
}

void CEnemyShip::FireBullet(CGameObject* pLockedObject)
{
}


void CEnemyShip::Animate(float fTimeElapsed)
{
	m_xmf3Position = Vector3::Add(m_xmf3Position, XMFLOAT3(0.f,0.f,-0.1f));

	//this->MoveForward(10.0f);
	CEnemyObject::Animate(fTimeElapsed);
}

void CEnemyShip::OnPrepareRender()
{
	CEnemyObject::OnPrepareRender();
}

void CEnemyShip::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	
	CEnemyObject::Render(pd3dCommandList, NULL);
}
