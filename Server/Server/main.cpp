#include "Framework.h"

CGameFramework framework;
array<SESSION, MAX_USER> clients;
mutex m;

int main() {
	framework.Init();
}