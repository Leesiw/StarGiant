#pragma once
#include "Player.h"
#include "stdafx.h"

#include <iostream>
#include <array>
#include <chrono>
#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_RESET_SCENE,
	OP_SPAWN_ENEMY, OP_MOVE_ENEMY, OP_AIMING_ENEMY, OP_UPDATE_METEO, OP_SPAWN_MISSILE, OP_UPDATE_MISSILE, OP_UPDATE_BOSS, OP_UPDATE_GOD,
	OP_UPDATE_SPACESHIP, OP_HEAL, OP_BLACK_HOLE,
	OP_MISSION_CLEAR,
	OP_SEND_SCENE_INFO,
	OP_CHECK_CUTSCENE_END
};


class OVER_EXP {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[SEND_BUF_SIZE];
	COMP_TYPE _comp_type;
	char obj_id;
	OVER_EXP()
	{
		_wsabuf.len = SEND_BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OVER_EXP(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}
	OVER_EXP(char* packet, int size)
	{
		_wsabuf.len = size;
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, size);
	}
};

enum S_STATE { ST_FREE, ST_ALLOC, ST_INGAME };
class SESSION {
	OVER_EXP _recv_over;

public:
	mutex _s_lock;

	S_STATE _state;
	int _id;
	PlayerType type;	// ����/����
	SOCKET _socket;
	//char	_name[NAME_SIZE]; // 일단 사용x

	short room_id;
	char room_pid;

	int		_prev_remain;
public:
	SESSION()
	{
		_id = -1;
		_socket = 0;
		_state = ST_FREE;
		_prev_remain = 0;

		type = PlayerType::INSIDE;
		room_id = -1;
		room_pid = -1;
	}

	~SESSION() {}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = SEND_BUF_SIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag,
			&_recv_over._over, 0);
	}

	void do_send(void* packet)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
	}

	void do_send(void* packet, int size)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet), size };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
	}

	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p{};
		p.data.id = room_pid;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.data.player_type = type;
		do_send(&p);
	}

	void send_add_player_packet(LOGIN_INFO& info);
	void send_change_packet(int c_id, PlayerType p_type);
	void send_spaceship_quaternion_packet(XMFLOAT4& Quaternion);
	void send_inside_packet(int c_id, CTerrainPlayer* m_pPlayer);
	void send_enemy_packet(ENEMY_INFO& enemy_info);
	void send_spawn_enemy_packet(SPAWN_ENEMY_INFO& enemy_info);
	void send_bullet_packet(XMFLOAT3& pos, XMFLOAT3& direction, unsigned int time);
	void send_spawn_missile_packet(SPAWN_MISSILE_INFO& info);
	void send_missile_packet(char id, XMFLOAT3& pos);
	void send_remove_missile_packet(char id);
	void send_heal_packet();
	void send_boss_meteo_packet(std::array<CMeteoObject*, BOSSMETEOS> meteo);
	void send_bullet_hit_packet(char id, short hp);
	void send_item_packet(ITEM_INFO& item);
	void send_animation_packet(char id, char animation);
	void send_mission_start_packet(MissionType mission);
	void send_kill_num_packet(char num);
	void send_enemy_state_packet(char id, EnemyState state);
};

