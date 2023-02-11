#pragma once
#include <DirectXMath.h>

using namespace DirectX;
//using namespace DirectX::PackedVector;

#define BULLETS					50
#define METEOS					20
#define ENEMIES					36
#define ENEMY_BULLETS			50


constexpr int MAX_USER = 3;

constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int W_WIDTH = 8;
constexpr int W_HEIGHT = 8;

// Packet ID
constexpr char CS_CHANGE = 0;
constexpr char CS_INSIDE_MOVE = 1;
constexpr char CS_SPACESHIP_MOVE = 2;
constexpr char CS_ATTACK = 3;

constexpr char SC_LOGIN_INFO = 4;
constexpr char SC_CHANGE = 5;
constexpr char SC_ADD_PLAYER = 6;
constexpr char SC_REMOVE_PLAYER = 7;

constexpr char SC_SPAWN_METEO = 8;
constexpr char SC_METEO_DIRECTION = 9;
constexpr char SC_METEO = 10;

constexpr char SC_MOVE_SPACESHIP = 11;
constexpr char SC_MOVE_INSIDEPLAYER = 12;
constexpr char SC_MOVE_INFO = 13;
constexpr char SC_BULLET = 14;
constexpr char SC_BULLET_HIT = 15;
constexpr char SC_ENEMY_DIE = 16;
constexpr char SC_ANIMATION_CHANGE = 17;

enum class PlayerType : char
{
	INSIDE, MOVE, ATTACK1, ATTACK2, ATTACK3
};

enum class EnemyType : char
{
	MISSILE, LASER, PLASMACANNON 
};

#pragma pack (push, 1)

struct METEO_INFO {
	XMFLOAT3 pos;
};

struct METEO_DIRECTION_INFO {
	short id;
	XMFLOAT3 dir;
};

struct SPAWN_METEO_INFO {
	short	id;

	XMFLOAT3 pos;
	XMFLOAT3 scale;
	XMFLOAT3 direction;
};

struct BULLET_INFO {
	XMFLOAT3 pos;
	XMFLOAT3 direction;
};

struct LOGIN_INFO {
	short	id;
	PlayerType player_type;
};

struct INSIDE_INPUT_INFO {
	DWORD dwDirection;
	float yaw;
	float pitch;
};

struct SPACESHIP_INPUT_INFO {
	DWORD dwDirection;
	XMFLOAT4 Quaternion;
};

struct SPACESHIP_INFO {
	XMFLOAT3					pos;
	XMFLOAT4					Quaternion;
};

struct INSIDE_PLAYER_INFO {
	short id;

	XMFLOAT3					pos;
	float           			m_fYaw;
	char						animation;
};

struct ENEMY_INFO {
	short id;

	XMFLOAT3 pos;

	float           			m_fPitch;
	//float           			m_fRoll;
	float           			m_fYaw;
};

struct BULLET_HIT_INFO {
	short id;
	short hp;
};

struct ANIMATION_INFO {
	short id;
	char animation;
};
//-----------------------------------------------------------------------

struct CS_CHANGE_PACKET {
	unsigned char size;
	char	type;

	PlayerType player_type;
};

struct CS_SPACESHIP_PACKET {
	unsigned char size;
	char	type;

	SPACESHIP_INPUT_INFO	data;
};

struct CS_INSIDE_PACKET {
	unsigned char size;
	char	type;

	INSIDE_INPUT_INFO	data;
};

struct CS_ATTACK_PACKET {
	unsigned char size;
	char	type;

	BULLET_INFO data;
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

struct SC_CHANGE_PACKET {
	unsigned char size;
	char	type;

	LOGIN_INFO	data;
};

struct SC_MOVE_SPACESHIP_PACKET {
	unsigned char size;
	char	type;

	SPACESHIP_INFO data;
};

struct SC_MOVE_INSIDE_PACKET {
	unsigned char size;
	char	type;

	INSIDE_PLAYER_INFO data;
};


struct SC_MOVE_ENEMY_PACKET {
	unsigned char size;
	char	type;

	ENEMY_INFO data;
};

struct SC_SPAWN_METEO_PACKET {
	unsigned char size;
	char	type;
	SPAWN_METEO_INFO data;
};

struct SC_METEO_DIRECTION_PACKET {
	unsigned char size;
	char	type;
	METEO_DIRECTION_INFO data;
};

struct SC_SPAWN_ALL_METEO_PACKET {
	unsigned char size;
	char	type;
	SPAWN_METEO_INFO data[METEOS];
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

struct SC_BULLET_HIT_PACKET {
	unsigned char size;
	char	type;
	
	BULLET_HIT_INFO data;
};

struct SC_ANIMATION_CHANGE_PACKET {
	unsigned char size;
	char	type;

	ANIMATION_INFO data;
};

#pragma pack (pop)