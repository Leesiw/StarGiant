#pragma once
#include <chrono>
#include <concurrent_priority_queue.h>

enum EVENT_TYPE { EV_SPAWN_ENEMY, EV_UPDATE_ENEMY, EV_UPDATE_METEO, EV_SPAWN_MISSILE,  
	EV_UPDATE_MISSILE, EV_UPDATE_BOSS, EV_UPDATE_SPACESHIP, EV_UPDATE_PLAYER, EV_HEAL };

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

static concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;