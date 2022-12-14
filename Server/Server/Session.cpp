#include "Session.h"

void SESSION::send_move_packet(int c_id, CPlayer* m_pPlayer, CEnemyObject* m_pEnemy)	// 플레이어/적의 좌표 각도 전송
{
	SC_MOVE_PLAYER_PACKET p;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.data.pos = m_pPlayer->GetPosition();
	p.data.m_fPitch = m_pPlayer->GetPitch();
	p.data.m_fRoll = m_pPlayer->GetRoll();
	p.data.m_fYaw = m_pPlayer->GetYaw();
	//p.data.velocity = m_pPlayer->GetVelocity();
	//p.data.shift = m_pPlayer->GetShift();
	do_send(&p);

	SC_MOVE_ENEMY_PACKET e;
	e.size = sizeof(SC_MOVE_ENEMY_PACKET);
	e.type = SC_MOVE_ENEMY;
	if (m_pEnemy->hp < 0) {
		e.data.appeared = false;
	}
	else {
		e.data.appeared = true;
	}
	e.data.pos = m_pEnemy->GetPosition();
	e.data.m_fPitch = m_pEnemy->GetPitch();
	e.data.m_fRoll = m_pEnemy->GetRoll();
	e.data.m_fYaw = m_pEnemy->GetYaw();
	do_send(&e);
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

void SESSION::send_bullet_packet(int c_id, CEnemyObject* m_pEnemy, XMFLOAT3 player_pos)
{

	SC_BULLET_PACKET p;

	p.size = sizeof(SC_BULLET_PACKET);
	p.type = SC_BULLET;

	XMFLOAT3 pos{};// = m_pEnemy->GetPosition();
	//pos.z += 130.0f;

	//pos.x = cos(m_pEnemy->GetYaw() + 180.0f);
	//pos.z = sin(m_pEnemy->GetYaw() + 180.0f);
	
	//p.data.direction = pos;
	p.data.direction = Vector3::Add(player_pos, pos, -1.0f);
	//Vector3::ScalarProduct(m_pEnemy->GetLook(), -1.0f, true);
	p.data.direction = pos;
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

void SESSION::send_meteo_packet(int c_id, CGameObject* meteo[])
{
	SC_METEO_PACKET p;
	p.size = sizeof(SC_METEO_PACKET);
	p.type = SC_METEO;

	for (int i = 0; i < METEOS; ++i) {
		p.meteo[i].m_xmf4x4Transform = meteo[i]->m_xmf4x4Transform;
		p.meteo[i].m_fRotationSpeed = meteo[i]->m_fRotationSpeed;
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
