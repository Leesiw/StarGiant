#pragma once
#include <chrono>
#include <concurrent_priority_queue.h>

enum EVENT_TYPE { EV_RANDOM_MOVE };

struct TIMER_EVENT {
	char obj_id;
	std::chrono::system_clock::time_point wakeup_time;
	EVENT_TYPE event_id;
	int room_id;

	constexpr bool operator < (const TIMER_EVENT& L) const
	{
		return (wakeup_time > L.wakeup_time);
	}
};

concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;