#pragma once
#include <chrono>
#include <concurrent_priority_queue.h>

enum EVENT_TYPE {
	EV_RESET_SCENE, EV_SPAWN_ENEMY, EV_MOVE_ENEMY, EV_AIMING_ENEMY, EV_UPDATE_METEO, EV_SPAWN_MISSILE,
	EV_UPDATE_MISSILE, EV_UPDATE_BOSS, EV_UPDATE_GOD, EV_UPDATE_SPACESHIP, EV_HEAL, EV_BLACK_HOLE,
	EV_MISSION_CLEAR,
	EV_SEND_SCENE_INFO
};

struct TIMER_EVENT {
	char obj_id;
	std::chrono::system_clock::time_point wakeup_time;
	EVENT_TYPE event_id;
	short room_id;

	constexpr bool operator < (const TIMER_EVENT& L) const
	{
		return (wakeup_time > L.wakeup_time);
	}
};
