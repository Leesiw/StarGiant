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

	bool SceneChange = false;

private:
	CScene* m_pScene = NULL;
	CScene* m_TwiceScene = NULL;
	CPlayer* m_pPlayer = NULL;
	CPlayer* m_TwicePlayer = NULL;
	CEnemyObject* m_Enemy = NULL;


	POINT						m_ptOldCursorPos;

	std::chrono::duration<double> fps;
	std::chrono::system_clock::time_point prev_process_time;

	_TCHAR						m_pszFrameRate[70];

	thread ClientProcessThread{};
};

