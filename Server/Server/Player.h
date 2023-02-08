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
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fMaxVelocityY = 0.0f;
	float           			m_fFriction = 0.0f;

	char						m_cAnimation = 0;

	//서버
	INPUT_INFO					input_info;
	bool is_update = true;

public:
	CPlayer();
	virtual ~CPlayer();

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	char GetAnimation() { return(m_cAnimation); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }

	void SetScale(XMFLOAT3& xmf3Scale) { m_xmf3Scale = xmf3Scale; }

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	virtual void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Rotate(float x, float y, float z);

	virtual void Update(float fTimeElapsed);

	virtual void OnPrepareRender();

	void SetInputInfo(INPUT_INFO i_info) { input_info = i_info; is_update = false; }


};

class CAirplanePlayer : public CPlayer
{
public:
	CAirplanePlayer();
	virtual ~CAirplanePlayer();

	CGameObject** m_BulletObjects = NULL;
	CBulletObject* m_ppBullets[BULLETS];
	float						m_fBulletEffectiveRange = 150.0f;
	CGameObject* pBullet;

	//불렛
	float						m_fFireDelayTime;
	float						m_fFireWaitingTime[3];

	bool FireBullet(short attack_num);

public:
	virtual void Animate(float fTimeElapsed);
	virtual void Update(float fTimeElapsed);
	virtual void OnPrepareRender();
public:
};

class CTerrainPlayer : public CPlayer
{
public:
	CTerrainPlayer();
	virtual ~CTerrainPlayer();



	bool walk;	//애니메이션
public:
	bool CheckCollision(const XMFLOAT3 pos[]);

	void Move(DWORD dwDirection, float fDistance, const XMFLOAT3 pos[], bool bUpdateVelocity = false);

	void Update(float fTimeElapsed, const XMFLOAT3 pos[]);
};