//-----------------------------------------------------------------------------
// File: SceneManager.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "SceneManager.h"

SceneManager::SceneManager()
{
	m_pScenes.fill(NULL);
	int num = 0;
	for (auto& scene : m_pScenes) {
		scene = new CScene();
		scene->num = num;
		scene->BuildObjects();
		++num;
	}
}

SceneManager::~SceneManager()
{
	for (auto& scene : m_pScenes) {
		if (scene) { delete scene; }
		scene = NULL;
	}
}



short SceneManager::FindScene(short id, short pl_id)		// 수정 필요 lock / unlock
{
	//_scene_lock.lock();
	array<CScene*, MAX_ROOM>::iterator iter = std::find_if(m_pScenes.begin(), m_pScenes.end(), [&id](CScene*& scene) {
		return scene->_id == id;
	});

	if (iter != m_pScenes.end())
	{
		if ((*iter)->_state != SCENE_RESET) {	// lock 필요?
			//(*iter)->InsertPlayer(pl_id);
			//_scene_lock.unlock();
			return (*iter)->num;
		}
		else {
			//_scene_lock.unlock();
			return -1;
		}
	}
	else {	// 새로운씬으로 배정
		array<CScene*, MAX_ROOM>::iterator iter2 = std::find_if(m_pScenes.begin(), m_pScenes.end(), [&id](CScene*& scene) {
			return scene->_id == -1 && scene->_state == SCENE_FREE;
		});

		if (iter2 != m_pScenes.end())
		{
			(*iter2)->_id = id;
			//(*iter2)->InsertPlayer(pl_id);
			//_scene_lock.unlock();
			return (*iter2)->num;
		}
	}

	//_scene_lock.unlock();
	return -2;
}

CScene* SceneManager::GetScene(short id)
{
	return m_pScenes[id];
}

void SceneManager::SceneStart(short num)
{
	m_pScenes[num]->Start();
}

void SceneManager::ResetScene(short num)
{
	m_pScenes[num]->_s_lock.lock();
	if (m_pScenes[num]->_state == SCENE_INGAME) {
		m_pScenes[num]->_state = SCENE_RESET;
		m_pScenes[num]->_s_lock.unlock();
		m_pScenes[num]->Reset();

		m_pScenes[num]->_s_lock.lock();
		m_pScenes[num]->_state = SCENE_FREE;


	}
	m_pScenes[num]->_s_lock.unlock();

}

char SceneManager::InsertPlayer(short num, short pl_id)
{
	return m_pScenes[num]->InsertPlayer(pl_id);
}

void SceneManager::SceneAnimate(float fTimeElapsed)
{
	for (auto& scene : m_pScenes) {
		if (scene->_state == ST_INGAME) {
			scene->AnimateObjects(fTimeElapsed);
		}
	}
}

void SceneManager::Send(short num, char* p)
{
	m_pScenes[num]->Send(p);
}

bool SceneManager::GetCanSit(short scene_id, PlayerType type)
{
	for (auto pl_id : m_pScenes[scene_id]->_plist) {
		if (pl_id == -1)continue;
		if (clients[pl_id].type == type) {
			return false;
		}
	}

	return true;
}
