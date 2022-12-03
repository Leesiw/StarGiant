#include "Session.h"

void SESSION::send_move_packet(int c_id, CPlayer* m_pPlayer)
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