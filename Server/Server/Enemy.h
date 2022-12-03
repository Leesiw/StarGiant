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

	virtual void OnPrepareRender();
	virtual void Animate(float fElapsedTime);
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