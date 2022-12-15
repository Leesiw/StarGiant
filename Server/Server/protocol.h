#pragma once
#include <DirectXMath.h>

using namespace DirectX;
//using namespace DirectX::PackedVector;

#define BULLETS					50
#define METEOS					200

constexpr int MAX_USER = 2;

constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int W_WIDTH = 8;
constexpr int W_HEIGHT = 8;

// Packet ID
constexpr char CS_CHANGE = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_ATTACK = 2;

constexpr char SC_LOGIN_INFO = 3;
constexpr char SC_ADD_PLAYER = 4;
constexpr char SC_REMOVE_PLAYER = 5;

constexpr char SC_METEO = 6;

constexpr char SC_MOVE_PLAYER = 7;
constexpr char SC_MOVE_ENEMY = 8;
constexpr char SC_BULLET = 9;
constexpr char SC_REMOVE_BULLET = 10;
constexpr char SC_BULLET_HIT = 11;
constexpr char SC_ENEMY_DIE = 11;

// Player type 
constexpr char MOVE = 0;
constexpr char ATTACK = 1;

#pragma pack (push, 1)

struct METEO_INFO {
	float m_fRotationSpeed;
	XMFLOAT4X4 m_xmf4x4Transform;
};

struct BULLET_INFO {
	XMFLOAT3 pos;
	XMFLOAT3 direction;
	float pitch, yaw, roll;
};

struct LOGIN_INFO {
	short	id;
	char player_type;
};

struct PLAYER_INFO {
	XMFLOAT3 pos;
	//XMFLOAT3 velocity;
	//XMFLOAT3 shift;

	float           			m_fPitch;
	float           			m_fRoll;
	float           			m_fYaw;
};

struct ENEMY_INFO {
	XMFLOAT3 pos;
	//XMFLOAT3 velocity;
	//XMFLOAT3 shift;

	float           			m_fPitch;
	float           			m_fRoll;
	float           			m_fYaw;

	bool						appeared;
};

struct BULLET_HIT_INFO {
	int meteo_id;
	int bullet_id;
};

//-----------------------------------------------------------------------

struct CS_CHANGE_PACKET {
	unsigned char size;
	char	type;
};

struct CS_MOVE_PACKET {
	unsigned char size;
	char	type;
	DWORD dwDirection;
	float cxDelta;
	float cyDelta;
	bool isRButton;
};

struct CS_ATTACK_PACKET {
	unsigned char size;
	char	type;
};

struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;

	LOGIN_INFO	data;
};

struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	x, y;
	char	name[NAME_SIZE];
};

struct SC_REMOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;

	PLAYER_INFO data;
};

struct SC_MOVE_ENEMY_PACKET {
	unsigned char size;
	char	type;

	ENEMY_INFO data;
};

struct SC_METEO_PACKET {
	unsigned char size;
	char	type;
	METEO_INFO meteo[METEOS];
};

struct SC_BULLET_PACKET {
	unsigned char size;
	char	type;
	BULLET_INFO data;
};

struct SC_REMOVE_BULLET_PACKET {
	unsigned char size;
	char	type;
	short	id;

	int num;
};

struct SC_BULLET_HIT_PACKET {
	unsigned char size;
	char	type;
	
	BULLET_HIT_INFO data;
};

#pragma pack (pop)