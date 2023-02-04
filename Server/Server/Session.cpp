#include "Session.h"

void SESSION::send_change_packet(int c_id, PlayerType p_type)
{
	SC_LOGIN_INFO_PACKET p;
	p.data.id = c_id;
	p.data.player_type = p_type;
	p.size = sizeof(SC_LOGIN_INFO_PACKET);
	p.type = SC_CHANGE;
	do_send(&p);
}

void SESSION::send_move_packet(int c_id, CPlayer* m_pPlayer)	// 플레이어 좌표 각도 전송
{
	SC_MOVE_PLAYER_PACKET p;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.data.id = c_id;
	p.data.pos = m_pPlayer->GetPosition();
	p.data.m_fYaw = m_pPlayer->GetYaw();
	p.data.m_fPitch = m_pPlayer->GetPitch();
	p.data.animation = m_pPlayer->GetAnimation();

	//p.data.velocity = m_pPlayer->GetVelocity();
	//p.data.shift = m_pPlayer->GetShift();
	do_send(&p);
}

void SESSION::send_enemy_packet(int c_id, ENEMY_INFO& enemy_info)
{
	SC_MOVE_ENEMY_PACKET p;

	p.size = sizeof(SC_MOVE_ENEMY_PACKET);
	p.type = SC_MOVE_INFO;

	p.data.id = enemy_info.id;
	p.data.m_fYaw = enemy_info.m_fYaw;
	p.data.pos = enemy_info.pos;

	char buf[sizeof(SC_MOVE_ENEMY_PACKET)];
	memcpy(buf, reinterpret_cast<char*>(&p), sizeof(p));
	WSABUF wsabuf{ sizeof(buf), buf };
	DWORD sent_byte;

	WSASend(_socket, &wsabuf, 1, &sent_byte, 0, nullptr, 0);
}

void SESSION::send_bullet_packet(int c_id, XMFLOAT3& pos, XMFLOAT3& direction)
{
	SC_BULLET_PACKET p;

	p.size = sizeof(SC_BULLET_PACKET);
	p.type = SC_BULLET;

	p.data.direction = direction;
	p.data.pos = pos;

	char buf[sizeof(SC_BULLET_PACKET)];
	memcpy(buf, reinterpret_cast<char*>(&p), sizeof(p));
	WSABUF wsabuf{ sizeof(buf), buf };
	DWORD sent_byte;

	WSASend(_socket, &wsabuf, 1, &sent_byte, 0, nullptr, 0);
}
/*
void SESSION::send_bullet_packet(int c_id, CEnemyObject* m_pEnemy, XMFLOAT3 player_pos)
{

	SC_BULLET_PACKET p;

	p.size = sizeof(SC_BULLET_PACKET);
	p.type = SC_BULLET;

	XMFLOAT3 pos{};// = m_pEnemy->GetPosition();
	//pos.z += 130.0f;

	/*pos.x = cos(m_pEnemy->GetYaw() + 180.0f);
	pos.z = sin(m_pEnemy->GetYaw() + 180.0f);
	XMFLOAT3 T_Position = m_pEnemy->GetPosition();
	T_Position.z += +120.0f;

	/*pos.x =  T_Position.x;
	pos.z =  T_Position.z;
	//XMFLOAT3 Pos = Vector3::ScalarProduct(direction, 20.0f * fTimeElapsed);

	float yaw = 180.0 + XMConvertToDegrees(atan2(player_pos.x - T_Position.x, player_pos.z - T_Position.z));

	pos = m_pEnemy->GetRotateLook(0, yaw - m_pEnemy->GetYaw(), 0);
	pos = XMFLOAT3{ -pos.x, -pos.y, -pos.z };

	p.data.direction = pos;
//	p.data.direction = Vector3::Add(player_pos, pos, -1.0f);///////////////////////////////////
	//Vector3::ScalarProduct(m_pEnemy->GetLook(), -1.0f, true);
	p.data.pos = m_pEnemy->GetPosition();
	p.data.pos.z += 130.0;
	p.data.pitch = m_pEnemy->GetPitch();
	p.data.yaw = m_pEnemy->GetYaw();
	p.data.roll = m_pEnemy->GetRoll();

	char buf[sizeof(SC_BULLET_PACKET)];
	memcpy(buf, reinterpret_cast<char*>(&p), sizeof(p));
	WSABUF wsabuf{ sizeof(buf), buf };
	DWORD sent_byte;

	WSASend(_socket, &wsabuf, 1, &sent_byte, 0, nullptr, 0);
}
*/

void SESSION::send_spawn_meteo_packet(int c_id, short id, CMeteoObject* meteo)
{
	SC_SPAWN_METEO_PACKET p;
	p.size = sizeof(SC_SPAWN_METEO_PACKET);
	p.type = SC_SPAWN_METEO;

	p.data.id = id;
	p.data.pos = meteo->GetPosition();
	p.data.scale = meteo->GetScale();
	p.data.direction = meteo->GetMovingDirection();

	char buf[sizeof(SC_SPAWN_METEO_PACKET)];
	memcpy(buf, reinterpret_cast<char*>(&p), sizeof(p));
	WSABUF wsabuf{ sizeof(buf), buf };
	DWORD sent_byte;

	WSASend(_socket, &wsabuf, 1, &sent_byte, 0, nullptr, 0);
}

void SESSION::send_spawn_all_meteo_packet(int c_id, CMeteoObject* meteo[])
{
	SC_SPAWN_ALL_METEO_PACKET p;
	p.size = sizeof(SC_SPAWN_ALL_METEO_PACKET);
	p.type = SC_SPAWN_METEO;

	for (int i = 0; i < METEOS; ++i) {
		p.data[i].pos = meteo[i]->GetPosition();
		p.data[i].scale = meteo[i]->GetScale();
		p.data[i].direction = meteo[i]->GetMovingDirection();
	}

	char buf[sizeof(SC_SPAWN_ALL_METEO_PACKET)];
	memcpy(buf, reinterpret_cast<char*>(&p), sizeof(p));
	WSABUF wsabuf{ sizeof(buf), buf };
	DWORD sent_byte;

	WSASend(_socket, &wsabuf, 1, &sent_byte, 0, nullptr, 0);
}

void SESSION::send_all_enemy_packet(int c_id, ENEMY_INFO e_info[], bool alive[])
{
	for (int i = 0; i < ENEMIES; ++i) {
		if (alive[i]) {
			send_enemy_packet(c_id, e_info[i]);
		}
	}
}

void SESSION::send_meteo_direction_packet(int c_id, short id, CMeteoObject* meteo)
{
	SC_METEO_DIRECTION_PACKET p;
	p.size = sizeof(SC_METEO_DIRECTION_PACKET);
	p.type = SC_METEO_DIRECTION;

	p.data.id = id;
	p.data.dir = meteo->GetMovingDirection();

	char buf[sizeof(SC_METEO_DIRECTION_PACKET)];
	memcpy(buf, reinterpret_cast<char*>(&p), sizeof(p));
	WSABUF wsabuf{ sizeof(buf), buf };
	DWORD sent_byte;

	WSASend(_socket, &wsabuf, 1, &sent_byte, 0, nullptr, 0);
}

void SESSION::send_meteo_packet(int c_id, CMeteoObject* meteo[])
{
	SC_METEO_PACKET p;
	p.size = sizeof(SC_METEO_PACKET);
	p.type = SC_METEO;

	for (int i = 0; i < METEOS; ++i) {
		p.meteo[i].pos = meteo[i]->GetPosition();
	}
	for (int i = 0; i < METEOS; ++i)
	{
		//printf("meteo %d : %f %f %f\n", i, p.meteo[i].pos.x, meteo[i].pos.y, meteo[i].pos.z);
	}
	char buf[sizeof(SC_METEO_PACKET)];
	memcpy(buf, reinterpret_cast<char*>(&p), sizeof(p));
	WSABUF wsabuf{ sizeof(buf), buf };
	DWORD sent_byte;

	WSASend(_socket, &wsabuf, 1, &sent_byte, 0, nullptr, 0);
}

void SESSION::send_bullet_hit_packet(int c_id, int meteo_id, int bullet_id)
{
	SC_BULLET_HIT_PACKET p;
	p.size = sizeof(SC_BULLET_HIT_PACKET);
	p.type = SC_BULLET_HIT;
	p.data.meteo_id = meteo_id;
	p.data.bullet_id = bullet_id;

	do_send(&p);
}
