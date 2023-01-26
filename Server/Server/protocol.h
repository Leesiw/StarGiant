#pragma once
#include <DirectXMath.h>

using namespace DirectX;
//using namespace DirectX::PackedVector;

#define BULLETS					50
#define METEOS					20
#define ENEMIES					36

constexpr int MAX_USER = 3;

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
constexpr char SC_CHANGE = 4;
constexpr char SC_ADD_PLAYER = 5;
constexpr char SC_REMOVE_PLAYER = 6;

constexpr char SC_SPAWN_METEO = 7;
constexpr char SC_METEO_DIRECTION = 8;
constexpr char SC_METEO = 9;

constexpr char SC_MOVE_PLAYER = 10;
constexpr char SC_MOVE_INFO = 11;
constexpr char SC_BULLET = 12;
constexpr char SC_BULLET_HIT = 13;
constexpr char SC_SPAWN_ENEMY = 14;
constexpr char SC_ENEMY_DIE = 15;

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
	// 방향/속도?
};

struct BULLET_INFO {
	XMFLOAT3 pos;
	XMFLOAT3 direction;
};

struct LOGIN_INFO {
	short	id;
	PlayerType player_type;
};

struct INPUT_INFO {
	DWORD dwDirection;
	float yaw;
};

struct PLAYER_INFO {
	short id;

	XMFLOAT3					pos;
	float           			m_fYaw;
};

struct SPAWN_ENEMY_INFO {
	short id;

	EnemyType type;
	XMFLOAT3 pos;
};

struct ENEMY_INFO {
	short id;

	XMFLOAT3 pos;

	//float           			m_fPitch;
	//float           			m_fRoll;
	float           			m_fYaw;
};

struct BULLET_HIT_INFO {
	int meteo_id;
	int bullet_id;
};

//-----------------------------------------------------------------------

struct CS_CHANGE_PACKET {
	unsigned char size;
	char	type;

	PlayerType player_type;
};

struct CS_MOVE_PACKET {
	unsigned char size;
	char	type;

	INPUT_INFO	data;
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

struct SC_CHANGE_PACKET {
	unsigned char size;
	char	type;

	LOGIN_INFO	data;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;

	PLAYER_INFO data[4];
};

struct SC_MOVE_ENEMY_PACKET {
	unsigned char size;
	char	type;

	ENEMY_INFO data;
};

struct SC_SPAWN_ENEMY_PACKET {
	unsigned char size;
	char	type;

	SPAWN_ENEMY_INFO data;
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