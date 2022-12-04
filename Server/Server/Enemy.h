#pragma once
#include "Object.h"

class CEnemyObject : public CGameObject
{
public:
	CEnemyObject();
	virtual ~CEnemyObject();
protected:
	XMFLOAT3					m_xmf3Position;
	XMFLOAT3					m_xmf3Right;
	XMFLOAT3					m_xmf3Up;
	XMFLOAT3					m_xmf3Look;


	float           			m_fPitch = 0;
	float           			m_fYaw = 0;
	float           			m_fRoll = 0;


public:
	int hp;

public:
	void Fallowing(float fTimeElapsed, XMFLOAT3 Look);
	void Rotate(float x, float y, float z);
	void SetPosition(const XMFLOAT3& xmf3Position) {
		m_xmf3Position = Vector3::Add(m_xmf3Position, XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z));
	}

	virtual void OnPrepareRender();
	virtual void Animate(float fElapsedTime);
	void Animate(float fTimeElapsed, XMFLOAT3 player_pos);

	float GetPitch() { return m_fPitch; }
	float GetYaw() { return m_fYaw; }
	float GetRoll() { return m_fRoll; }
};


class CEnemyShip : public CEnemyObject
{
public:
	CEnemyShip();
	virtual ~CEnemyShip();

	CGameObject** m_BulletObjects = NULL;
	CBulletObject* m_ppBullets[BULLETS];
	void FireBullet(CGameObject* pLockedObject);
	float						m_fBulletEffectiveRange = 150.0f;
	CGameObject* pBullet;


public:
	virtual void Animate(float fTimeElapsed);
	virtual void OnPrepareRender();
};