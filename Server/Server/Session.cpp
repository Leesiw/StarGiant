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

void SESSION::send_spaceship_packet(int c_id, CAirplanePlayer* m_pPlayer)	// 플레이어 좌표 각도 전송
{
	SC_MOVE_SPACESHIP_PACKET p;
	p.size = sizeof(SC_MOVE_SPACESHIP_PACKET);
	p.type = SC_MOVE_SPACESHIP;
	p.data.pos = m_pPlayer->GetPosition();
	XMFLOAT4 a;
	p.data.Quaternion = m_pPlayer->input_info.Quaternion;

	//p.data.velocity = m_pPlayer->GetVelocity();
	//p.data.shift = m_pPlayer->GetShift();
	do_send(&p);
}

void SESSION::send_inside_packet(int c_id, CTerrainPlayer* m_pPlayer)
{
	SC_MOVE_INSIDE_PACKET p;
	p.size = sizeof(SC_MOVE_INSIDE_PACKET);
	p.type = SC_MOVE_INSIDEPLAYER;
	p.data.id = c_id;
	p.data.pos = m_pPlayer->GetPosition();
	p.data.m_fYaw = m_pPlayer->GetYaw();
	p.data.animation = m_pPlayer->GetAnimation();

	do_send(&p);
}

void SESSION::send_enemy_packet(int c_id, ENEMY_INFO& enemy_info)
{
	SC_MOVE_ENEMY_PACKET p;

	p.size = sizeof(SC_MOVE_ENEMY_PACKET);
	p.type = SC_MOVE_INFO;

	p.data.id = enemy_info.id;
	p.data.Quaternion = enemy_info.Quaternion;
	p.data.pos = enemy_info.pos;

	do_send(&p);
}

void SESSION::send_bullet_packet(int c_id, XMFLOAT3& pos, XMFLOAT3& direction)
{
	SC_BULLET_PACKET p;

	p.size = sizeof(SC_BULLET_PACKET);
	p.type = SC_BULLET;

	p.data.direction = direction;
	p.data.pos = pos;

	do_send(&p);
}

void SESSION::send_missile_packet(int c_id, MISSILE_INFO& info)
{
	SC_MISSILE_PACKET p;
	p.size = sizeof(SC_MISSILE_PACKET);
	p.type = SC_MISSILE;

	p.data.id = info.id;
	p.data.pos = info.pos;
	p.data.Quaternion = info.Quaternion;

	do_send(&p);
}

void SESSION::send_remove_missile_packet(int c_id, short id)
{
	SC_REMOVE_MISSILE_PACKET p;
	p.size = sizeof(SC_REMOVE_MISSILE_PACKET);
	p.type = SC_REMOVE_MISSILE;

	p.id = id;

	do_send(&p);
}

void SESSION::send_heal_packet()
{
	SC_HEAL_PACKET p;

	p.size = sizeof(SC_HEAL_PACKET);
	p.type = SC_HEAL;
	do_send(&p);
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

	do_send(&p);
}

void SESSION::send_spawn_all_meteo_packet(int c_id, std::array<CMeteoObject*,METEOS> meteo) 
{
	for (int i = 0; i < METEOS; ++i) {
		send_spawn_meteo_packet(0, i, meteo[i]);
	}
}

void SESSION::send_all_enemy_packet(int c_id, ENEMY_INFO e_info[], bool alive[])
{
	for (int i = 0; i < ENEMIES; ++i) {
		if (alive[i]) {
			send_enemy_packet(i, e_info[i]);
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

	do_send(&p);
}

void SESSION::send_meteo_packet(int c_id, std::array<CMeteoObject*, METEOS> meteo)
{
	SC_METEO_PACKET p;
	p.size = sizeof(SC_METEO_PACKET);
	p.type = SC_METEO;

	for (int i = 0; i < METEOS; ++i) {
		p.data.id = i;
		p.data.pos = meteo[i]->GetPosition();
		do_send(&p);
	}
}

void SESSION::send_bullet_hit_packet(int c_id, short id, short hp)
{
	SC_BULLET_HIT_PACKET p;
	p.size = sizeof(SC_BULLET_HIT_PACKET);
	p.type = SC_BULLET_HIT;
	p.data.id = id;
	p.data.hp = hp;

	do_send(&p);
}

void SESSION::send_item_packet(int c_id, ITEM_INFO& item)
{
	SC_ITEM_PACKET p;
	p.size = sizeof(SC_ITEM_PACKET);
	p.type = SC_ITEM;
	p.data.type = item.type;
	p.data.num = item.num;

	do_send(&p);
}

void SESSION::send_animation_packet(char id, char animation)
{
	SC_ANIMATION_CHANGE_PACKET p;
	p.size = sizeof(SC_ANIMATION_CHANGE_PACKET);
	p.type = SC_ANIMATION_CHANGE;
	p.data.id = id;
	p.data.animation = animation;

	do_send(&p);
}

void SESSION::send_mission_start_packet(MissionType mission)
{
	SC_MISSION_START_PACKET p;
	p.size = sizeof(SC_MISSION_START_PACKET);
	p.type = SC_MISSION_START;
	p.next_mission = mission;
	
	do_send(&p);
}

void SESSION::send_kill_num_packet(char num)
{
	SC_KILL_NUM_PACKET p;
	p.size = sizeof(SC_KILL_NUM_PACKET);
	p.type = SC_KILL_NUM;
	p.num = num;

	do_send(&p);
}
