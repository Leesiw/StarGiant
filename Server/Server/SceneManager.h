#pragma once
#include "Scene.h"
#include "Timer.h"

extern concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;

class SceneManager {
public:
	SceneManager();
	~SceneManager();

	short FindScene(short id, short pl_id);
	short FindScene(short pl_id);
	CScene* GetScene(short id);
	void SceneStart(short num);	// 게임 state INGAME으로 변경
	void ResetScene(short num);
	char InsertPlayer(short num, short pl_id);
	void Send(short num, char* p);

	bool GetCanSit(short scene_id, PlayerType type);

	mutex _scene_lock;

private:
	std::array<CScene*, MAX_ROOM> m_pScenes;
	std::array<mutex, MAX_ROOM> m_room;
};