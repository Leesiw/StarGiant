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
	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fFriction = 0.0f;

	//¼­¹ö
	volatile bool is_update = true;

public:
	CPlayer();
	virtual ~CPlayer();

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }

	void SetScale(XMFLOAT3& xmf3Scale) { m_xmf3Scale = xmf3Scale; }

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	virtual void Move(char nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Rotate(float x, float y, float z);

	virtual void Update(float fTimeElapsed);

	virtual void OnPrepareRender();
};

class CAirplanePlayer : public CPlayer
{
public:
	CAirplanePlayer();
	virtual ~CAirplanePlayer();

	short max_hp = 100;
	short hp = 100;

	short damage = 3;
	char heal = 10;
	char def = 0;

	XMFLOAT4 Quaternion;
	char cDirection = 0;	// option1 : forward(w) option2 : left(a) option3 : backward(s) option4 : right(d)
public:
	void SetHP(short s) { hp = s; }
	short GetHP() { return hp; }

	void GetAttack(char damage);
	bool GetHeal();

	void Reset();

	void SetInputInfo(XMFLOAT4& quaternion) { Quaternion = quaternion; is_update = false; }
	void SetKeyInput(char key_input);
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

	INSIDE_INPUT_INFO input_info;

	bool cutscene_end = false;
public:
	void Reset();

	void SetInputInfo(INSIDE_INPUT_INFO i_info) { input_info = i_info; is_update = false; }

	bool CheckCollision(const XMFLOAT3 pos[]);

	void Move(DWORD dwDirection, float fDistance, const XMFLOAT3 pos[], bool bUpdateVelocity = false);

	void Update(float fTimeElapsed, const XMFLOAT3 pos[]);
};