#include "Session.h"

void SESSION::send_add_player_packet(LOGIN_INFO& info)
{
	SC_ADD_PLAYER_PACKET p;
	p.data.id = info.id;
	p.data.player_type = info.player_type;
	p.data.x = info.x;
	p.data.z = info.z;
	p.data.yaw = info.yaw;
	p.size = sizeof(SC_ADD_PLAYER_PACKET);
	p.type = SC_ADD_PLAYER;
	do_send(&p);
}

void SESSION::send_change_packet(int c_id, PlayerType p_type)
{
	SC_LOGIN_INFO_PACKET p;
	p.data.id = c_id;
	p.data.player_type = p_type;
	p.size = sizeof(SC_LOGIN_INFO_PACKET);
	p.type = SC_CHANGE;
	do_send(&p);
}

void SESSION::send_spaceship_quaternion_packet(XMFLOAT4& Quaternion)
{
	CS_SPACESHIP_QUATERNION_PACKET p{};
	p.size = sizeof(CS_SPACESHIP_QUATERNION_PACKET);
	p.type = SC_SPACESHIP_QUATERNION;
	p.Quaternion = Quaternion;
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

	do_send(&p);
}

void SESSION::send_enemy_packet(ENEMY_INFO& enemy_info)
{
	SC_MOVE_ENEMY_PACKET p;

	p.size = sizeof(SC_MOVE_ENEMY_PACKET);
	p.type = SC_MOVE_ENEMY;

	p.data = enemy_info;

	do_send(&p);
}

void SESSION::send_spawn_enemy_packet(SPAWN_ENEMY_INFO& enemy_info)
{
	SC_SPAWN_ENEMY_PACKET p;

	p.size = sizeof(SC_SPAWN_ENEMY_PACKET);
	p.type = SC_SPAWN_ENEMY;

	p.data.id = enemy_info.id;
	p.data.Quaternion = enemy_info.Quaternion;
	p.data.pos = enemy_info.pos;
	p.data.max_hp = enemy_info.max_hp;
	p.data.destination = enemy_info.destination;
	p.data.state = enemy_info.state;

	do_send(&p);
}

void SESSION::send_bullet_packet(XMFLOAT3& pos, XMFLOAT3& direction, unsigned int time)
{
	SC_BULLET_PACKET p;

	p.size = sizeof(SC_BULLET_PACKET);
	p.type = SC_BULLET;

	p.data.direction = direction;
	p.data.pos = pos;
	p.attack_time = time;

	do_send(&p);
}

void SESSION::send_spawn_missile_packet(SPAWN_MISSILE_INFO& info)
{
	SC_SPAWN_MISSILE_PACKET p;
	p.size = sizeof(SC_SPAWN_MISSILE_PACKET);
	p.type = SC_SPAWN_MISSILE;

	p.data.id = info.id;
	p.data.pos = info.pos;
	p.data.Quaternion = info.Quaternion;

	do_send(&p);
}

void SESSION::send_missile_packet(char id, XMFLOAT3& pos)
{
	SC_MISSILE_PACKET p;
	p.size = sizeof(SC_MISSILE_PACKET);
	p.type = SC_MISSILE;

	p.data.id = id;
	p.data.pos = pos;

	do_send(&p);
}

void SESSION::send_remove_missile_packet(char id)
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

void SESSION::send_boss_meteo_packet(std::array<CMeteoObject*, BOSSMETEOS> meteo)
{
	SC_METEO_PACKET p;
	p.size = sizeof(SC_METEO_PACKET);
	p.type = SC_METEO;

	for (int i = 0; i < BOSSMETEOS; ++i) {
		p.data.id = METEOS + i;
		p.data.pos = meteo[i]->GetPosition();
		do_send(&p);
	}
}

void SESSION::send_bullet_hit_packet(char id, short hp)
{
	SC_BULLET_HIT_PACKET p;
	p.size = sizeof(SC_BULLET_HIT_PACKET);
	p.type = SC_BULLET_HIT;
	p.data.id = id;
	p.data.hp = hp;

	do_send(&p);
}

void SESSION::send_item_packet(ITEM_INFO& item)
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

void SESSION::send_enemy_state_packet(char id, EnemyState state)
{
	SC_ENEMY_STATE_PACKET p;
	p.size = sizeof(SC_ENEMY_STATE_PACKET);
	p.type = SC_ENEMY_STATE;
	p.data.id = id;
	p.data.state = state;

	do_send(&p);
}


