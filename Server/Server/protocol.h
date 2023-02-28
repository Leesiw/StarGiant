#pragma once
#include <DirectXMath.h>

using namespace DirectX;
//using namespace DirectX::PackedVector;

#define BULLETS					50
#define METEOS					20
#define ENEMIES					36
#define ENEMY_BULLETS			50
#define GODRAY_SAMPLE			30
#define UI_CNT					2

#define MAX_ITEM				15	// 보석 최대 레벨?

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
constexpr char SC_ITEM = 16;
constexpr char SC_ANIMATION_CHANGE = 17;

enum class PlayerType : char
{
	INSIDE, MOVE, ATTACK1, ATTACK2, ATTACK3
};

enum class EnemyType : char
{
	MISSILE, LASER, PLASMACANNON 
};

enum class UIType : char
{
	CROSSHAIR, MINIMAP, HP
};

enum class ItemType : char
{
	JEWEL_ATT, JEWEL_DEF, JEWEL_HEAL
};

#pragma pack (push, 1)

// login
struct LOGIN_INFO {
	short	id;
	PlayerType player_type;
};

// meteo
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

// bullet
struct BULLET_INFO {
	XMFLOAT3 pos;
	XMFLOAT3 direction;
};

struct BULLET_HIT_INFO {
	short id;
	short hp;
};

// inside
struct INSIDE_INPUT_INFO {
	DWORD dwDirection;
	float yaw;
	float pitch;
};

struct INSIDE_PLAYER_INFO {
	short id;

	XMFLOAT3					pos;
	float           			m_fYaw;
	char						animation;
};

// spaceship
struct SPACESHIP_INPUT_INFO {
	DWORD dwDirection;
	XMFLOAT4 Quaternion;
};

struct SPACESHIP_INFO {
	XMFLOAT3					pos;
	XMFLOAT4					Quaternion;
};

// enemy
struct ENEMY_INFO {
	short id;

	XMFLOAT3 pos;
	XMFLOAT4 Quaternion;
};

// animation
struct ANIMATION_INFO {
	short id;
	char animation;
};

// item
struct ITEM_INFO {
	ItemType type;
	short num;
};
//-----------------------------------------------------------------------

// login
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

// type
struct CS_CHANGE_PACKET {
	unsigned char size;
	char	type;

	PlayerType player_type;
};

struct SC_CHANGE_PACKET {
	unsigned char size;
	char	type;

	LOGIN_INFO	data;
};

// meteo
struct SC_SPAWN_METEO_PACKET {
	unsigned char size;
	char	type;
	SPAWN_METEO_INFO data;
};

struct SC_SPAWN_ALL_METEO_PACKET {
	unsigned char size;
	char	type;
	SPAWN_METEO_INFO data[METEOS];
};

struct SC_METEO_DIRECTION_PACKET {
	unsigned char size;
	char	type;
	METEO_DIRECTION_INFO data;
};

struct SC_METEO_PACKET {
	unsigned char size;
	char	type;
	METEO_INFO meteo[METEOS];
};

// spaceship
struct CS_SPACESHIP_PACKET {
	unsigned char size;
	char	type;

	SPACESHIP_INPUT_INFO	data;
};

struct SC_MOVE_SPACESHIP_PACKET {
	unsigned char size;
	char	type;

	SPACESHIP_INFO data;
};

// inside
struct CS_INSIDE_PACKET {
	unsigned char size;
	char	type;

	INSIDE_INPUT_INFO	data;
};

struct SC_MOVE_INSIDE_PACKET {
	unsigned char size;
	char	type;

	INSIDE_PLAYER_INFO data;
};

// attack
struct CS_ATTACK_PACKET {
	unsigned char size;
	char	type;

	BULLET_INFO data;
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

// enemy
struct SC_MOVE_ENEMY_PACKET {
	unsigned char size;
	char	type;

	ENEMY_INFO data;
};

// animation
struct SC_ANIMATION_CHANGE_PACKET {
	unsigned char size;
	char	type;

	ANIMATION_INFO data;
};

// item
struct SC_ITEM_PACKET {
	unsigned char size;
	char	type;

	ITEM_INFO data;
};


#pragma pack (pop)