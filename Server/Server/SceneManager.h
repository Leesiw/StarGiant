#pragma once
#include "Scene.h"

class SceneManager {
public:
	SceneManager();
	~SceneManager();

	short FindScene(short id, short pl_id);
	CScene* GetScene(short id);
	void SceneStart(short num);	// 게임 state INGAME으로 변경
	void ResetScene(short num);
	void InsertPlayer(short num, short pl_id);
	void SceneAnimate(float fTimeElapsed);	// 이후 타이머로 변경
	void Send(short num, char* p);

	bool GetCanSit(short scene_id, PlayerType type);

	mutex _scene_lock;

private:
	std::array<CScene*, MAX_ROOM> m_pScenes;
	std::array<mutex, MAX_ROOM> m_room;
};