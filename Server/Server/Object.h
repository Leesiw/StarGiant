#pragma once
#include "stdafx.h"

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

public:
	XMFLOAT4X4						m_xmf4x4ToParent;
	XMFLOAT4X4						m_xmf4x4World;

	BoundingOrientedBox				boundingbox;	// 직접 크기 지정

	chrono::steady_clock::time_point prev_time;

	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL) {};
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	virtual void SetScale(float x, float y, float z);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4* pxmf4Quaternion);

	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);

	BoundingOrientedBox UpdateBoundingBox();
};

class CMeteoObject : public CGameObject
{
public:
	CMeteoObject();
	virtual ~CMeteoObject();
public:	
	XMFLOAT3					m_xmf3MovingDirection;

	XMFLOAT3						m_xmf3Scale;

	int coll_time;

	void SetMovingSpeed(float fSpeed);
	void SetMovingDirection(XMFLOAT3& xmf3MovingDirection) { m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection); }
	XMFLOAT3 GetMovingDirection() { return m_xmf3MovingDirection; }
	virtual void SetScale(float x, float y, float z);
	XMFLOAT3 GetScale() { return m_xmf3Scale; }

	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};