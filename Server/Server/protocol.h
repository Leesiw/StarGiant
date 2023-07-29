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
constexpr char METEOS = 10;
constexpr char BOSSMETEOS = 5;
constexpr char MISSILES = 8;

constexpr char LASER_ENEMY = 13;
constexpr char MISSILE_ENEMY = 4;
constexpr char PLASMACANNON_ENEMY = 10;
constexpr char ENEMIES = LASER_ENEMY + MISSILE_ENEMY + PLASMACANNON_ENEMY;
constexpr char ENEMY_BULLETS =50;
constexpr char GODRAY_SAMPLE = 100;
constexpr char SPRITE_CNT = 6;
#define UI_CNT static_cast<int>(UIType::COUNT) + ENEMIES + 1 - 1
#define UI_INSIDE_CNT /*static_cast<int>(UIInsideType::COUNT) */ 4



constexpr char MAX_ITEM = 15;	// 보석 최대 레벨?

constexpr int MAX_ROOM = 200;
constexpr int MAX_USER = MAX_ROOM * 3;

constexpr int PORT_NUM = 4000;
constexpr int RECV_BUF_SIZE = 10000;
constexpr int SEND_BUF_SIZE = 10000;
constexpr int NAME_SIZE = 20;

// boss id
constexpr char BOSS_ID = ENEMIES + 1;
constexpr char GOD_ID = BOSS_ID + 1;


// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_CHANGE = 1;
constexpr char CS_INSIDE_MOVE = 2;
constexpr char CS_SPACESHIP_QUATERNION = 3;
constexpr char CS_KEY_INPUT = 4;
constexpr char CS_ATTACK = 5;
constexpr char CS_HEAL = 6;
constexpr char CS_ANIMATION_CHANGE = 7;

constexpr char SC_LOGIN_INFO = 9;
constexpr char SC_CHANGE = 10;
constexpr char SC_ADD_PLAYER = 11;
constexpr char SC_REMOVE_PLAYER = 12;

constexpr char SC_METEO = 13;
constexpr char SC_ALL_METEOR = 14;

constexpr char SC_MOVE_SPACESHIP = 15;
constexpr char SC_SPACESHIP_QUATERNION = 16;
constexpr char SC_MOVE_INSIDEPLAYER = 17;
constexpr char SC_SPAWN_ENEMY = 18;
constexpr char SC_MOVE_ENEMY = 19;
constexpr char SC_BULLET = 20;
constexpr char SC_BULLET_HIT = 21;
constexpr char SC_SPAWN_MISSILE = 22;
constexpr char SC_MISSILE = 23;
constexpr char SC_REMOVE_MISSILE = 24;
constexpr char SC_ITEM = 25;
constexpr char SC_ANIMATION_CHANGE = 26;
constexpr char SC_HEAL = 27;
constexpr char SC_MISSION_START = 28;
constexpr char SC_KILL_NUM = 29;

constexpr char SC_START = 30;

constexpr char CS_CUTSCENE_END = 31;
constexpr char SC_CUTSCENE_END_NUM = 32;

constexpr char SC_BLACK_HOLE = 33;	// 블랙홀 위치 전송
constexpr char SC_BLACK_HOLE_TIME = 34;

constexpr char SC_ENEMY_STATE = 35;
constexpr char SC_MOVE_BOSS = 36;
constexpr char SC_MOVE_GOD = 37;

// cheat
constexpr char CS_NEXT_MISSION = 50;
constexpr char CS_START = 51;
constexpr char CS_INVINCIBLE_MODE = 52;



enum class PlayerType : char
{
	INSIDE, MOVE, ATTACK1, ATTACK2, ATTACK3
};

enum class EnemyState : char
{
	MOVE, AIMING
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

enum class GodState : char
{
	IDLE1, IDLE2, MELEE1, MELEE2, SHOT, HIT1, HIT2, ROAR, DEATH

	, COUNT
};

enum class GodAnimation : char
{
	IDLE1, IDLE2, MELEE1, MELEE2, SHOT, HIT1, HIT2, ROAR, DEATH

	, COUNT
};

enum class EnemyType : char
{
	MISSILE, LASER, PLASMACANNON 
};

enum class UIType : char
{
	CROSSHAIR, SCRATCH, HP, BITE

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
	CS_TURN,	// 컷씬 우주선 중심 한바퀴 돌기
	TU_SIT, TU_KILL, TU_HILL, GET_JEWELS, Kill_MONSTER, 
	CS_SHOW_PLANET,
	GO_PLANET, KILL_MONSTER_ONE_MORE_TIME, FIND_BOSS,
	CS_BOSS_SCREAM,
	DEFEAT_BOSS,
	CS_ANGRY_BOSS, DEFEAT_BOSS2,
	CS_SHOW_STARGIANT,
	GO_CENTER, KILL_MONSTER3, KILL_METEOR, 
	CS_SHOW_BLACK_HOLE,
	ESCAPE_BLACK_HOLE, GO_CENTER_REAL,
	CS_SHOW_GOD,
	KILL_GOD, 
	CS_ANGRY_GOD, KILL_GOD2,
	CS_ENDING,

	CS_BAD_ENDING
};

enum class SpriteType : char
{
	Ship, EnemyBoom
};

#pragma pack (push, 1)

// login
struct LOGIN_INFO {
	char	id;
	PlayerType player_type;
	float x, z;
	float yaw;
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
};

struct SPAWN_MISSILE_INFO {
	char id;
	XMFLOAT3 pos;
	XMFLOAT4 Quaternion;
};

// inside
struct INSIDE_INPUT_INFO {
	DWORD dwDirection;
	float yaw;
};

struct INSIDE_PLAYER_INFO {
	char id;

	XMFLOAT3					pos;
	float           			m_fYaw;
};

// spaceship

struct SPACESHIP_INFO {
	XMFLOAT3					pos;
};

// enemy
struct SPAWN_ENEMY_INFO {
	char id;

	EnemyState state;
	XMFLOAT3 pos;
	XMFLOAT4 Quaternion;
	XMFLOAT3 destination;
	char max_hp;
};

struct ENEMY_INFO {
	char id;

	XMFLOAT3 pos;
	//XMFLOAT4 Quaternion;
};

struct ENEMY_STATE {
	char id;
	EnemyState state;
};

struct BOSS_INFO {
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

// cheat / debugging
struct CS_NEXT_MISSION_PACKET {
	unsigned char size;
	char	type;
};


// login
struct CS_LOGIN_PACKET {
	unsigned char size;
	char	type;

	short room_id;
};

struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;

	LOGIN_INFO	data;
};

struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	char type;
	LOGIN_INFO	data;
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

struct SC_ALL_METEOR_PACKET {
	unsigned char size;
	char	type;
	XMFLOAT3 pos[METEOS];
};


// spaceship
struct CS_SPACESHIP_QUATERNION_PACKET {
	unsigned char size;
	char	type;

	XMFLOAT4 Quaternion;;
};

struct CS_KEY_INPUT_PACKET {
	unsigned char size;
	char	type;

	char key;	// 0: w 입력 1: a 입력 2: s 입력 3: d 입력
				// 4: w 해제 5: a 해제 6: s 해제 7: d 해제
};

struct SC_MOVE_SPACESHIP_PACKET {
	unsigned char size;
	char	type;

	XMFLOAT3 pos;
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
	unsigned int attack_time;
};

struct SC_BULLET_PACKET {
	unsigned char size;
	char	type;

	BULLET_INFO data;
	unsigned int attack_time;
};

struct SC_BULLET_HIT_PACKET {
	unsigned char size;
	char	type;

	BULLET_HIT_INFO data;
};

// missile

struct SC_SPAWN_MISSILE_PACKET {
	unsigned char size;
	char	type;

	SPAWN_MISSILE_INFO data;
};

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
struct SC_SPAWN_ENEMY_PACKET {
	unsigned char size;
	char	type;

	SPAWN_ENEMY_INFO data;
};

struct SC_MOVE_ENEMY_PACKET {
	unsigned char size;
	char	type;

	ENEMY_INFO data;
};

struct  SC_ENEMY_STATE_PACKET {
	unsigned char size;
	char	type;

	ENEMY_STATE data;
};

struct SC_MOVE_BOSS_PACKET {
	unsigned char size;
	char	type;

	BOSS_INFO data;
};


// animation
struct CS_ANIMATION_CHANGE_PACKET {
	unsigned char size;
	char	type;

	char state;
};


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

// mission
struct SC_MISSION_START_PACKET {
	unsigned char size;
	char type;

	MissionType next_mission;
};

struct SC_KILL_NUM_PACKET {
	unsigned char size;
	char type;

	char num;
};


struct SC_BLACK_HOLE_PACKET {
	unsigned char size;
	char	type;

	XMFLOAT3 pos;
};



struct SC_START_PACKET {
	unsigned char size;
	char	type;
};

struct CS_CUTSCENE_END_PACKET {
	unsigned char size;
	char	type;
};


struct SC_CUTSCENE_END_NUM_PACKET {
	unsigned char size;
	char	type;
	char num;
};

struct SC_BLACK_HOLE_TIME_PACKET {
	unsigned char size;
	char	type;
	float time;
};



#pragma pack (pop)