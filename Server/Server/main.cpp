#include "Framework.h"

array<SESSION, MAX_USER> clients;
std::unordered_map<MissionType, Level> levels;
CGameFramework framework;

int main() {
	framework.Init();
}