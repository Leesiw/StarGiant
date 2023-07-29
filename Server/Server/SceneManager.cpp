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
	array<CScene*, MAX_ROOM>::iterator iter = std::find_if(m_pScenes.begin(), m_pScenes.end(), [&id](CScene*& scene) {
		return scene->_id == id && scene->_state == SCENE_ALLOC;
	});

	if (iter != m_pScenes.end())
	{
		if ((*iter)->_state != SCENE_RESET) {	// lock 필요?
			return (*iter)->num;
		}
		else {
			return -1;
		}
	}
	else {	// 새로운씬으로 배정
		array<CScene*, MAX_ROOM>::iterator iter2 = std::find_if(m_pScenes.begin(), m_pScenes.end(), [&id](CScene*& scene) {
			return scene->_state == SCENE_FREE;
		});

		if (iter2 != m_pScenes.end())
		{
			(*iter2)->_id = id;
			return (*iter2)->num;
		}
	}

	return -2;
}

short SceneManager::FindScene(short pl_id)
{
	array<CScene*, MAX_ROOM>::iterator iter = std::find_if(m_pScenes.begin(), m_pScenes.end(), [](CScene*& scene) {
		if (scene->_state != SCENE_ALLOC || scene->_id != -1) { return false; }
		return std::any_of(scene->_plist.crbegin(), scene->_plist.crend(), [](short pl) {
			return pl == -1;
			});
		});

	if (iter != m_pScenes.end()){ return (*iter)->num; }

	iter = std::find_if(m_pScenes.begin(), m_pScenes.end(), [](CScene*& scene) {
		return scene->_state == SCENE_FREE;
		});

	if (iter != m_pScenes.end()) { return (*iter)->num; }

	return -1;
}

CScene* SceneManager::GetScene(short id)
{
	return m_pScenes[id];
}

void SceneManager::ResetScene(short num)
{
	m_pScenes[num]->_s_lock.lock();
	if (m_pScenes[num]->_state == SCENE_INGAME) {
		m_pScenes[num]->_state = SCENE_RESET;
		m_pScenes[num]->_s_lock.unlock();
		m_pScenes[num]->Reset();
		TIMER_EVENT ev{ 0, chrono::system_clock::now() + 60s, EV_RESET_SCENE, num };
		timer_queue.push(ev);
		return;
	}
	m_pScenes[num]->_s_lock.unlock();
}

char SceneManager::InsertPlayer(short num, short pl_id)
{
	return m_pScenes[num]->InsertPlayer(pl_id);
}

void SceneManager::Send(short num, char* p)
{
	m_pScenes[num]->Send(p);
}
