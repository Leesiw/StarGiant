#pragma once

#include "Session.h"
#include "Scene.h"

extern array<SESSION, MAX_USER> clients;

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	void Init();

	void BuildObjects();
	void ReleaseObjects();

	void AnimateObjects(float fTimeElapsed);
	void FrameAdvance();

	void ProcessData(char* net_buf, size_t io_byte);
	void ProcessPacket(int c_id, char* packet);
	void send_packet(void* packet);
	void ClientProcess();

	int get_new_client_id();
	void disconnect(int c_id);
	CGameObject* m_pLockedObject = NULL;
private:
	CScene* m_pScene = NULL;

	CAirplanePlayer* m_pSpaceship = NULL;
	CTerrainPlayer* m_ppPlayers[MAX_USER] = { NULL, NULL, NULL };
	//CEnemyObject* m_Enemy = NULL;

	std::chrono::duration<double> fps;
	std::chrono::system_clock::time_point prev_process_time;

	float m_fEnemySpawnTime = 10.0f;
	float m_fEnemySpawnTimeRemaining = 0.0f;

	_TCHAR						m_pszFrameRate[70];

	thread ClientProcessThread{};
};

