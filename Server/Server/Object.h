#pragma once
#include "stdafx.h"

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

public:
	char							m_pstrFrameName[64];

	XMFLOAT4X4						m_xmf4x4ToParent;
	XMFLOAT4X4						m_xmf4x4World;

	bool						mesh = false;

	bool coll = false;
	BoundingOrientedBox				boundingbox;	// 직접 크기 지정
	BoundingOrientedBox				m_xmOOBB;		// 위의 것에 위치 행렬 이용해 이동 시킨 것. 충돌체크를 이것으로 한다
	BoundingBox			aabb;

	virtual void OnInitialize() { }
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL) {};
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	chrono::steady_clock::time_point prev_time;

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	virtual void SetScale(float x, float y, float z);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4* pxmf4Quaternion);

	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);

	void UpdateBoundingBox();
	bool HierarchyIntersects(CGameObject* pCollisionGameObject, bool isSecond = false);
};

class CMeteoObject : public CGameObject
{
public:
	CMeteoObject();
	virtual ~CMeteoObject();

	short model_id;
public:	
	float						m_fMovingSpeed;
	XMFLOAT3					m_xmf3MovingDirection;

	XMFLOAT3						m_xmf3Scale;

	int coll_time;
	char send_num = 5;

	void SetModelId(short id) { model_id = id; }
	short GetModelID() { return model_id; }
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
	void SetMovingDirection(XMFLOAT3& xmf3MovingDirection) { m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection); }
	XMFLOAT3 GetMovingDirection() { return m_xmf3MovingDirection; }
	virtual void SetScale(float x, float y, float z);
	XMFLOAT3 GetScale() { return m_xmf3Scale; }

	virtual void OnInitialize();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};

//===================================
/*

class CBulletObject : public CGameObject
{
public:
	CBulletObject(float fEffectiveRange);
	virtual ~CBulletObject();

public:
	virtual void Animate(float fElapsedTime);

	float						m_fBulletEffectiveRange = 150.0f;
	float						m_fMovingDistance = 0.0f;
	float						m_fRotationAngle = 0.0f;
	XMFLOAT3					m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float						m_fElapsedTimeAfterFire = 0.0f;
	float						m_fLockingDelayTime = 0.3f;
	float						m_fLockingTime = 4.0f;
	CGameObject* m_pLockedObject = NULL;

	XMFLOAT4X4						m_xmf4x4Transform;

	float						m_fMovingSpeed;
	XMFLOAT3					m_xmf3MovingDirection;
	float						m_fRotationSpeed = 0.0f;
	XMFLOAT3					m_xmf3RotationAxis;

	float m_fPitch, m_fYaw, m_fRoll;

	bool m_bActive;
	void SetActive(bool bActive) { m_bActive = bActive; }

	void SetRotationAxis(XMFLOAT3& xmf3RotationAxis) { m_xmf3RotationAxis = Vector3::Normalize(xmf3RotationAxis); }
	void SetRotationSpeed(float fSpeed) { m_fRotationSpeed = fSpeed; }
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
	void SetMovingDirection(XMFLOAT3& xmf3MovingDirection) { m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection); }

	void SetFirePosition(XMFLOAT3 xmf3FirePosition);
	void Reset();
};*/