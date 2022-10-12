#pragma once
#include "stdafx.h"

class CGameObject
{
private:
	int								m_nReferences = 0;

	float					m_fxRotationSpeed = 0.0f;
	float					m_fyRotationSpeed = 0.0f;
	float					m_fzRotationSpeed = 0.0f;

public:
	void AddRef();
	void Release();

public:
	CGameObject();
	virtual ~CGameObject();

public:
	char							m_pstrFrameName[64];

	XMFLOAT4X4						m_xmf4x4Transform;
	XMFLOAT4X4						m_xmf4x4World;

	CGameObject* m_pParent = NULL;
	CGameObject* m_pChild = NULL;
	CGameObject* m_pSibling = NULL;

	float							m_Speed{ 10.0 };
	float							m_RoateAngle{ 5.0 };
	XMFLOAT3						m_xmLook{ -1.0, 0.0, -1.0 };

	XMFLOAT3					m_xmf3MovingDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	float						m_fMovingSpeed = 0.0f;
	float						m_fMovingRange = 0.0f;
	XMFLOAT3					m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	float						m_fRotationSpeed = 0.0f;


	float						m_AccelSpeedXZ;
	float GetAccelSpeedXZ() const { return(m_AccelSpeedXZ); }



	bool						m_bActive = true;

	bool coll = false;
	BoundingOrientedBox				boundingbox;	// 직접 크기 지정
	BoundingOrientedBox				m_xmOOBB;		// 위의 것에 위치 행렬 이용해 이동 시킨 것. 충돌체크를 이것으로 한다
	BoundingBox			aabb;


	int					hp = 3;

	void SetOOBB();

	//void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);

	virtual void OnInitialize() { }
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float x, float y, float z);

	//===============================================================================

	void SetMovingDirection(XMFLOAT3& xmf3MovingDirection) { m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection); }
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
	void SetMovingRange(float fRange) { m_fMovingRange = fRange; }
	void SetRotationAxis(XMFLOAT3& xmf3RotationAxis) { m_xmf3RotationAxis = Vector3::Normalize(xmf3RotationAxis); }
	void SetRotationSpeed(float fSpeed) { m_fRotationSpeed = fSpeed; }

	//===============================================================================
	void Move(XMFLOAT3& vDirection, float fSpeed);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	//===============================================================================
	void MoveMeteo(XMFLOAT3 xmf3Look, float fDistance = 1.0f);
	void MakeSpeed();
	void UpdateSpeed(float fTimeElapsed);
	void UpdateRespawn(BoundingBox Player, XMFLOAT3 Switch, XMFLOAT3 m_xmf3Look);
	//===============================================================================
	void TurnSpeed();
	void Replace(XMFLOAT3 Point);
	void MoveToP(XMFLOAT3 Point, float fTimeElapsed);

	//===============================================================================


	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3& xmf3RotationAxis, float fAngle);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4* pxmf4Quaternion);

	void SetRotationSpeed(float fPitch, float fYaw, float fRoll) { m_fxRotationSpeed = fPitch; m_fyRotationSpeed = fYaw; m_fzRotationSpeed = fRoll; }

	CGameObject* GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);

	void MakeBoundingBox();
	void UpdateBoundingBox();
	bool HierarchyIntersects(CGameObject* pCollisionGameObject, bool isSecond = false);

public:
	static void PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent);

	void SetActive(bool bActive) { m_bActive = bActive; }
};

class CRotatingObject : public CGameObject
{
public:
	CRotatingObject();
	virtual ~CRotatingObject();

private:
	XMFLOAT3					m_xmf3RotationAxis;
	float						m_fRotationSpeed;

public:
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; }

	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};

class CRevolvingObject : public CGameObject
{
public:
	CRevolvingObject();
	virtual ~CRevolvingObject();

private:
	XMFLOAT3					m_xmf3RevolutionAxis;
	float						m_fRevolutionSpeed;

public:
	void SetRevolutionSpeed(float fRevolutionSpeed) { m_fRevolutionSpeed = fRevolutionSpeed; }
	void SetRevolutionAxis(XMFLOAT3 xmf3RevolutionAxis) { m_xmf3RevolutionAxis = xmf3RevolutionAxis; }

	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};

class CHellicopterObject : public CGameObject
{
public:
	CHellicopterObject();
	virtual ~CHellicopterObject();

protected:
	CGameObject* m_pMainRotorFrame = NULL;
	CGameObject* m_pTailRotorFrame = NULL;

public:
	virtual void OnInitialize();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};

class MeteoObject : public CHellicopterObject
{
public:
	MeteoObject();
	virtual ~MeteoObject();

public:
	virtual void OnInitialize();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};

class MeteoriteObject : public CHellicopterObject
{
public:
	MeteoriteObject();
	virtual ~MeteoriteObject();

public:
	virtual void OnInitialize();
};

class CSuperCobraObject : public CHellicopterObject
{
public:
	CSuperCobraObject();
	virtual ~CSuperCobraObject();

public:
	virtual void OnInitialize();
};

class CMi24Object : public CHellicopterObject
{
public:
	CMi24Object();
	virtual ~CMi24Object();

public:
	virtual void OnInitialize();
};



//===================================


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



	float						m_fRotationSpeed = 0.0f;

	float m_fPitch, m_fYaw, m_fRoll;


	void SetRotationAxis(XMFLOAT3& xmf3RotationAxis) { m_xmf3RotationAxis = Vector3::Normalize(xmf3RotationAxis); }
	void SetRotationSpeed(float fSpeed) { m_fRotationSpeed = fSpeed; }
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }

	void SetFirePosition(XMFLOAT3 xmf3FirePosition);
	void Reset();


};