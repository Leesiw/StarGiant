#pragma once
#pragma once

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08



#define DIR_UP					0x10
#define DIR_DOWN				0x20

#define DIR_STOP				0x40


#include "Object.h"

class CPlayer : public CGameObject
{
protected:
	XMFLOAT3					m_xmf3Position;
	XMFLOAT3					m_xmf3Right;
	XMFLOAT3					m_xmf3Up;
	XMFLOAT3					m_xmf3Look;

	float           			m_fPitch;
	float           			m_fYaw;
	float           			m_fRoll;

	XMFLOAT3					m_xmf3Velocity;
	float           			m_fMaxVelocityXZ;
	float           			m_fMaxVelocityY;
	float           			m_fFriction;

	float						m_fFireDelayTime;
	float						m_fFireWaitingTime;

	XMFLOAT3					m_xmf3Shift;

	DWORD						dwDirection;
	float						cxDelta;
	float						cyDelta;
	bool						isRButton;

public:
	CPlayer();
	virtual ~CPlayer();

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }

	XMFLOAT3 GetShift() { return(m_xmf3Shift); }
	XMFLOAT3 GetRotate() { return(XMFLOAT3(m_fPitch, m_fYaw, m_fRoll)); }

	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	BoundingBox					Eyesightbox = BoundingBox(m_xmf3Position, XMFLOAT3(500.0f, 500.0f, 500.0f)); // 플레이어 시야 범위
	BoundingBox GetBox() { return Eyesightbox; }
	void UpdateEyesightBox() { Eyesightbox.Center.x = GetPosition().x;	Eyesightbox.Center.y = GetPosition().y;	Eyesightbox.Center.z = GetPosition().z; };

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	void SetdwDirection(DWORD dw);
	void SetcxcyDelta(float cx, float cy, bool r_button);

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(DWORD dwDirection, float fDistance, float ftime, bool bUpdateVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Rotate(float x, float y, float z);

	void Update(float fTimeElapsed);
	void OnPrepareRender();
};

class CAirplanePlayer : public CPlayer
{
public:
	CAirplanePlayer();
	virtual ~CAirplanePlayer();

	CGameObject* m_pMainRotorFrame = NULL;
	CGameObject* m_pTailRotorFrame = NULL;


	CGameObject** m_BulletObjects = NULL;
	CBulletObject* m_ppBullets[BULLETS];
	void FireBullet(CGameObject* pLockedObject);
	float						m_fBulletEffectiveRange = 150.0f;
	CGameObject* pBullet;

private:
	virtual void OnInitialize();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void OnPrepareRender();
public:
};


