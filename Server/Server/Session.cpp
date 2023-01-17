#include "Session.h"

void SESSION::send_move_packet(int c_id, CPlayer* m_pPlayer)	// 플레이어 좌표 각도 전송
{
	SC_MOVE_PLAYER_PACKET p;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.data.pos = m_pPlayer->GetPosition();
	p.data.m_fYaw = m_pPlayer->GetYaw();
	//p.data.velocity = m_pPlayer->GetVelocity();
	//p.data.shift = m_pPlayer->GetShift();
	do_send(&p);
}

void SESSION::send_bullet_packet(int c_id, CPlayer* m_pPlayer)
{
	SC_BULLET_PACKET p;

	p.size = sizeof(SC_BULLET_PACKET);
	p.type = SC_BULLET;

	p.data.direction = m_pPlayer->GetLookVector();
	p.data.pos = m_pPlayer->GetPosition();
	p.data.pitch = m_pPlayer->GetPitch();
	p.data.yaw = m_pPlayer->GetYaw();
	p.data.roll = m_pPlayer->GetRoll();

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

void SESSION::send_meteo_packet(int c_id, CMeteoObject* meteo[])
{
	SC_METEO_PACKET p;
	p.size = sizeof(SC_METEO_PACKET);
	p.type = SC_METEO;

	for (int i = 0; i < METEOS; ++i) {
		p.meteo->pos = meteo[i]->GetPosition();
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
