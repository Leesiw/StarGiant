#pragma once

#include "Session.h"
#include "Scene.h"
#include "Timer.h"

extern array<SESSION, MAX_USER> clients;
extern std::unordered_map<MissionType, Level> levels;
extern mutex m;
extern concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	void worker_thread(HANDLE h_iocp);

	void Init();
	void SetMission();

	void ReleaseObjects();

	void ProcessPacket(int c_id, char* packet);
	void TimerThread(HANDLE h_iocp);

	int get_new_client_id();
	void disconnect(int c_id);
private:
	SOCKET g_s_socket, g_c_socket;
	OVER_EXP g_a_over;

	thread ClientProcessThread{};
};

