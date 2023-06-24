#include "Framework.h"
#include "SceneManager.h"

array<SESSION, MAX_USER> clients;
std::unordered_map<MissionType, Level> levels;
CGameFramework framework;
SceneManager scene_manager;
concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;

int main() {
	framework.Init();
}