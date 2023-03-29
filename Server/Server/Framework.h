#pragma once

#include "Session.h"
#include "Scene.h"

extern array<SESSION, MAX_USER> clients;
extern mutex m;

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	void Init();

	void BuildObjects();
	void ReleaseObjects();

	void AnimateObjects(float fTimeElapsed);

	void ProcessPacket(int c_id, char* packet);
	void send_packet(void* packet);
	void ClientProcess();

	int get_new_client_id();
	void disconnect(int c_id);
private:
	CScene* m_pScene = NULL;

	std::chrono::duration<double> fps;

	std::array<Level, MISSION_NUM> levels;

	thread ClientProcessThread{};
};

