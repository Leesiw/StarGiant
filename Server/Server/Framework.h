#pragma once

#include "Session.h"
#include "Scene.h"

extern array<SESSION, MAX_USER> clients;
extern std::unordered_map<MissionType, Level> levels;
extern mutex m;

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	void worker_thread(HANDLE h_iocp);

	short GetSceneID();

	void Init();
	void SetMission();

	void BuildObjects();
	void ReleaseObjects();
	void ReleaseObjects(short room_id);

	void AnimateObjects(float fTimeElapsed);

	void ProcessPacket(int c_id, char* packet);
	void send_packet(void* packet);
	void ClientProcess();

	int get_new_client_id();
	void disconnect(int c_id);
private:
	SOCKET g_s_socket, g_c_socket;
	OVER_EXP g_a_over;

	std::chrono::duration<double> fps;

	thread ClientProcessThread{};
};

