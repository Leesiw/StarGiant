#pragma once
#include <DirectXMath.h>

using namespace DirectX;
//using namespace DirectX::PackedVector;

const unsigned char option0 = 0x1;  // hex for 0000 0001 
const unsigned char option1 = 0x2;  // hex for 0000 0010
const unsigned char option2 = 0x4;  // hex for 0000 0100
const unsigned char option3 = 0x8;  // hex for 0000 1000
const unsigned char option4 = 0x10; // hex for 0001 0000
const unsigned char option5 = 0x20; // hex for 0010 0000
const unsigned char option6 = 0x40; // hex for 0100 0000
const unsigned char option7 = 0x80; // hex for 1000 0000

constexpr char BULLETS = 50;
constexpr char METEOS = 20;

constexpr char LASER_ENEMY = 13;
constexpr char MISSILE_ENEMY = 4;
constexpr char PLASMACANNON_ENEMY = 10;
constexpr char ENEMIES = LASER_ENEMY + MISSILE_ENEMY + PLASMACANNON_ENEMY;
constexpr char ENEMY_BULLETS =50;
constexpr char GODRAY_SAMPLE = 30;
constexpr char SPRITE_CNT = 1;
#define UI_CNT static_cast<int>(UIType::COUNT) + ENEMIES - 1
#define UI_INSIDE_CNT /*static_cast<int>(UIInsideType::COUNT) */ 4



constexpr char MAX_ITEM = 15;	// 보석 최대 레벨?

constexpr int MAX_USER = 3;

constexpr int PORT_NUM = 4000;
constexpr int RECV_BUF_SIZE = 10000;
constexpr int SEND_BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

// boss id
constexpr char BOSS_ID = ENEMIES + 1;

// Packet ID
constexpr char CS_CHANGE = 0;
constexpr char CS_INSIDE_MOVE = 1;
constexpr char CS_SPACESHIP_MOVE = 2;
constexpr char CS_ATTACK = 3;
constexpr char CS_HEAL = 4;

constexpr char SC_LOGIN_INFO = 5;
constexpr char SC_CHANGE = 6;
constexpr char SC_ADD_PLAYER = 7;
constexpr char SC_REMOVE_PLAYER = 8;

constexpr char SC_SPAWN_METEO = 9;
constexpr char SC_METEO_DIRECTION = 10;
constexpr char SC_METEO = 11;

constexpr char SC_MOVE_SPACESHIP = 12;
constexpr char SC_MOVE_INSIDEPLAYER = 13;
constexpr char SC_MOVE_INFO = 14;
constexpr char SC_BULLET = 15;
constexpr char SC_BULLET_HIT = 16;
constexpr char SC_MISSILE = 17;
constexpr char SC_REMOVE_MISSILE = 18;
constexpr char SC_ITEM = 19;
constexpr char SC_ANIMATION_CHANGE = 20;
constexpr char SC_HEAL = 21;

enum class PlayerType : char
{
	INSIDE, MOVE, ATTACK1, ATTACK2, ATTACK3
};

enum class BossState : char
{
	SLEEP, IDLE, SIT_IDLE, SCREAM, GET_HIT, WALK, RUN, BASIC_ATTACT, CLAW_ATTACT, FLAME_ATTACK, DEFEND,
	TAKE_OFF, FLY_FLOAT, FLY_FLAME_ATTACK, FLY_FORWARD, FLY_GLIDE, LAND, DIE
	/*IDLE, SLEEPING, MOVING, CHASING, ATTACT, FLY_ATTACT, DEFEND, FLY, DIE*/


	, ATTACT, CHASE, APPEAR, DEL
	,COUNT
};

enum class BossAnimation : char
{
	SLEEP, IDLE, SIT_IDLE, SCREAM, GET_HIT, WALK, RUN, BASIC_ATTACT, CLAW_ATTACT, FLAME_ATTACK, DEFEND,
	TAKE_OFF, FLY_FLOAT, FLY_FLAME_ATTACK, FLY_FORWARD, FLY_GLIDE, LAND, DIE
	/*, SLEEPING, MOVING, CHASING, ATTACT*/

	,COUNT
};

enum class EnemyType : char
{
	MISSILE, LASER, PLASMACANNON 
};

enum class UIType : char
{
	CROSSHAIR, MINIMAP, HP

	,COUNT
};

enum class UIInsideType : char
{
	FSIT = static_cast<int>(UIType::COUNT)
	,NAME_1, NAME_2, NAME_3


	, END, COUNT = END - static_cast<int>(UIType::COUNT)
};

enum class ItemType : char
{
	JEWEL_ATT, JEWEL_DEF, JEWEL_HEAL, JEWEL_HP
	// 공격력/방어력/회복력/최대HP
};

enum class MissionType : char
{
	TU_SIT, TU_KILL, TU_HILL, TU_END,  Kill_MONSTER, GO_PLANET, FIND_BOSS, GET_JEWELS, KILL_MONSTER_ONE_MORE_TIME
};

#pragma pack (push, 1)

// login
struct LOGIN_INFO {
	short	id;
	PlayerType player_type;
};

// meteo
struct METEO_INFO {
	char id;
	XMFLOAT3 pos;
};

struct METEO_DIRECTION_INFO {
	char id;
	XMFLOAT3 dir;
};

struct SPAWN_METEO_INFO {
	char id;

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
	char id;
	short hp;
};

// missile
struct MISSILE_INFO {
	char id;
	XMFLOAT3 pos;
	XMFLOAT4 Quaternion;
};

// inside
struct INSIDE_INPUT_INFO {
	DWORD dwDirection;
	float yaw;
	float pitch;
};

struct INSIDE_PLAYER_INFO {
	char id;

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
	char id;

	XMFLOAT3 pos;
	XMFLOAT4 Quaternion;
};

// animation
struct ANIMATION_INFO {
	char id;
	char animation;
};

// item
struct ITEM_INFO {
	ItemType type;
	char num;
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

// heal
struct CS_HEAL_PACKET {
	unsigned char size;
	char	type;

	bool start;
};

struct SC_HEAL_PACKET {
	unsigned char size;
	char	type;
};


// meteo
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

struct SC_METEO_PACKET {
	unsigned char size;
	char	type;
	METEO_INFO data;
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

// missile

struct SC_MISSILE_PACKET {
	unsigned char size;
	char	type;

	MISSILE_INFO data;
};

struct SC_REMOVE_MISSILE_PACKET {
	unsigned char size;
	char	type;

	char id;
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