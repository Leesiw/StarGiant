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

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };


class OVER_EXP {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[SEND_BUF_SIZE];
	COMP_TYPE _comp_type;
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
};


class SESSION {
	OVER_EXP _recv_over;

public:
	bool in_use;
	int _id;
	PlayerType type;	// ����/����
	SOCKET _socket;
	char	_name[NAME_SIZE];

	int		_prev_remain;
public:
	SESSION()
	{
		_id = -1;
		_socket = 0;
		_name[0] = 0;
		in_use = false;
		_prev_remain = 0;
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
	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.data.id = _id;
		p.data.player_type = type;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		do_send(&p);
	}
	void send_change_packet(int c_id, PlayerType p_type);
	void send_spaceship_packet(int c_id, CAirplanePlayer* m_pPlayer);
	void send_inside_packet(int c_id, CTerrainPlayer* m_pPlayer);
	void send_enemy_packet(int c_id, ENEMY_INFO& enemy_info);
	void send_bullet_packet(int c_id, XMFLOAT3& pos, XMFLOAT3& direction);
	void send_missile_packet(int c_id, MISSILE_INFO& info);
	void send_remove_missile_packet(int c_id, short id);
	void send_heal_packet();
	//void send_bullet_packet(int c_id, CEnemyObject* m_pEnemy, XMFLOAT3 player_pos);
	void send_spawn_meteo_packet(int c_id, short id, CMeteoObject* meteo);
	void send_spawn_all_meteo_packet(int c_id, CMeteoObject* meteo[]);
	void send_all_enemy_packet(int c_id, ENEMY_INFO[], bool[]);
	void send_meteo_direction_packet(int c_id, short id, CMeteoObject* meteo);
	void send_meteo_packet(int c_id, CMeteoObject* []);
	void send_bullet_hit_packet(int c_id, short id, short hp);
	void send_item_packet(int c_id, ITEM_INFO& item);
	void send_animation_packet(char id, char animation);
};