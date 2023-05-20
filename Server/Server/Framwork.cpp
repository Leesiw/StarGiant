#include "Framework.h"
#include "SceneManager.h"

extern SceneManager scene_manager;

CGameFramework::CGameFramework()
{
}

CGameFramework::~CGameFramework()
{
	
}


void CGameFramework::worker_thread(HANDLE h_iocp)
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		if ((0 == num_bytes) && ((ex_over->_comp_type == OP_RECV) || (ex_over->_comp_type == OP_SEND))) {
			disconnect(static_cast<int>(key));
			if (ex_over->_comp_type == OP_SEND) delete ex_over;
			continue;
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = get_new_client_id();
			if (client_id != -1) {
				{
					lock_guard<mutex> ll(clients[client_id]._s_lock);
					clients[client_id]._state = ST_ALLOC;
				}
				clients[client_id].room_id = -1;
				clients[client_id].room_pid = -1;
				clients[client_id].type = PlayerType::INSIDE;
				clients[client_id]._id = client_id;
				//clients[client_id]._name[0] = 0;
				clients[client_id]._prev_remain = 0;
				clients[client_id]._socket = g_c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket),
					h_iocp, client_id, 0);
				clients[client_id].do_recv();
				g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
			break;
		}
		case OP_RECV: {
			int remain_data = num_bytes + clients[key]._prev_remain;
			char* p = ex_over->_send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					ProcessPacket(static_cast<int>(key), p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			clients[key]._prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			clients[key].do_recv();
			break;
		}
		case OP_SEND:
			delete ex_over;
			break;
		}
	}
}

void CGameFramework::Init() {

	SetMission();
	BuildObjects();

	HANDLE h_iocp;

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_socket, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), h_iocp, 9999, 0);
	g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_a_over._comp_type = OP_ACCEPT;
	AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);

	vector <thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();
	for (int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back(&CGameFramework::worker_thread, this, h_iocp);
	thread timer_thread{ &CGameFramework::ClientProcess, this };
	for (auto& th : worker_threads)
		th.join();
	timer_thread.join();
	closesocket(g_s_socket);
	WSACleanup();
}

void CGameFramework::SetMission()
{
	levels[MissionType::TU_SIT].NextMission = MissionType::TU_KILL;
	levels[MissionType::TU_KILL].NextMission = MissionType::TU_HILL;
	levels[MissionType::TU_HILL].NextMission = MissionType::TU_END;
	levels[MissionType::TU_END].NextMission = MissionType::GET_JEWELS;

	for (int i = static_cast<int>(MissionType::TU_SIT);
		i <= static_cast<int>(MissionType::TU_END); ++i) {
		MissionType m = static_cast<MissionType>(i);
		levels[m].MaxMonsterNum = 10;
		levels[m].SpawnMonsterNum = 5;
		levels[m].Laser.MAX_HP = 3;
		levels[m].Laser.ATK = 2;
		levels[m].Missile.MAX_HP = 5;
		levels[m].Missile.ATK = 3;
		levels[m].PlasmaCannon.MAX_HP = 10;
		levels[m].PlasmaCannon.ATK = 4;
	}

	levels[MissionType::GET_JEWELS].NextMission = MissionType::Kill_MONSTER;
	levels[MissionType::GET_JEWELS].MaxMonsterNum = 15;
	levels[MissionType::GET_JEWELS].SpawnMonsterNum = 7;
	levels[MissionType::GET_JEWELS].Laser.MAX_HP = 5;
	levels[MissionType::GET_JEWELS].Laser.ATK = 3;
	levels[MissionType::GET_JEWELS].Missile.MAX_HP = 7;
	levels[MissionType::GET_JEWELS].Missile.ATK = 4;
	levels[MissionType::GET_JEWELS].PlasmaCannon.MAX_HP = 12;
	levels[MissionType::GET_JEWELS].PlasmaCannon.ATK = 5;

	levels[MissionType::Kill_MONSTER].NextMission = MissionType::GO_PLANET;
	levels[MissionType::Kill_MONSTER].MaxMonsterNum = 20;
	levels[MissionType::Kill_MONSTER].SpawnMonsterNum = 9;
	levels[MissionType::Kill_MONSTER].Laser.MAX_HP = 10;
	levels[MissionType::Kill_MONSTER].Laser.ATK = 6;
	levels[MissionType::Kill_MONSTER].Missile.MAX_HP = 14;
	levels[MissionType::Kill_MONSTER].Missile.ATK = 8;
	levels[MissionType::Kill_MONSTER].PlasmaCannon.MAX_HP = 15;
	levels[MissionType::Kill_MONSTER].PlasmaCannon.ATK = 7;

	levels[MissionType::GO_PLANET].NextMission = MissionType::KILL_MONSTER_ONE_MORE_TIME;
	levels[MissionType::GO_PLANET].MaxMonsterNum = ENEMIES;
	levels[MissionType::GO_PLANET].SpawnMonsterNum = 9;
	levels[MissionType::GO_PLANET].Laser.MAX_HP = 15;
	levels[MissionType::GO_PLANET].Laser.ATK = 10;
	levels[MissionType::GO_PLANET].Missile.MAX_HP = 17;
	levels[MissionType::GO_PLANET].Missile.ATK = 12;
	levels[MissionType::GO_PLANET].PlasmaCannon.MAX_HP = 20;
	levels[MissionType::GO_PLANET].PlasmaCannon.ATK = 11;

	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].NextMission = MissionType::FIND_BOSS;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].MaxMonsterNum = ENEMIES;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].SpawnMonsterNum = 9;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].Laser.MAX_HP = 20;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].Laser.ATK = 18;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].Missile.MAX_HP = 35;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].Missile.ATK = 20;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].PlasmaCannon.MAX_HP = 40;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].PlasmaCannon.ATK = 30;

	levels[MissionType::FIND_BOSS].NextMission = MissionType::DEFEAT_BOSS;
	levels[MissionType::FIND_BOSS].MaxMonsterNum = ENEMIES;
	levels[MissionType::FIND_BOSS].SpawnMonsterNum = 9;
	levels[MissionType::FIND_BOSS].Laser.MAX_HP = 20;
	levels[MissionType::FIND_BOSS].Laser.ATK = 18;
	levels[MissionType::FIND_BOSS].Missile.MAX_HP = 35;
	levels[MissionType::FIND_BOSS].Missile.ATK = 20;
	levels[MissionType::FIND_BOSS].PlasmaCannon.MAX_HP = 40;
	levels[MissionType::FIND_BOSS].PlasmaCannon.ATK = 30;

	levels[MissionType::DEFEAT_BOSS].NextMission = MissionType::DEFEAT_BOSS;
	levels[MissionType::DEFEAT_BOSS].MaxMonsterNum = 0;
	levels[MissionType::DEFEAT_BOSS].SpawnMonsterNum = 0;
	levels[MissionType::DEFEAT_BOSS].Laser.MAX_HP = 20;
	levels[MissionType::DEFEAT_BOSS].Laser.ATK = 18;
	levels[MissionType::DEFEAT_BOSS].Missile.MAX_HP = 35;
	levels[MissionType::DEFEAT_BOSS].Missile.ATK = 20;
	levels[MissionType::DEFEAT_BOSS].PlasmaCannon.MAX_HP = 40;
	levels[MissionType::DEFEAT_BOSS].PlasmaCannon.ATK = 30;
}

void CGameFramework::BuildObjects()
{
}

void CGameFramework::ReleaseObjects()
{
}

void CGameFramework::AnimateObjects(float fTimeElapsed)
{

	scene_manager.SceneAnimate(fTimeElapsed);
}


void CGameFramework::ProcessPacket(int c_id, char* packet)
{
	/*
	auto StartTime = chrono::system_clock::now();
	fps =  StartTime - prev_process_time;
	if (fps.count() > 0.1f) {
		fps = StartTime - StartTime;
	}*/

	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		if (p->room_id != -1) {
			scene_manager._scene_lock.lock();
			short scene_num = scene_manager.FindScene(p->room_id, c_id);
			scene_manager._scene_lock.unlock();

			if (scene_num >= 0) {
				char num = scene_manager.InsertPlayer(scene_num, c_id);
				printf("num : %d\n", num);
				if (num == 2) { scene_manager.SceneStart(scene_num); }
				else if (num == -1) { disconnect(c_id); return; }  // 일단 disconnect 이후 로그인 fail 패킷으로 변경
				else {
					CScene* scene = scene_manager.GetScene(scene_num);
					scene->_s_lock.lock();
					if (scene->_state == SCENE_FREE) {
						scene_manager.GetScene(scene_num)->_state = SCENE_ALLOC;
					}
					scene->_s_lock.unlock();
				}
			}
			else {
				disconnect(c_id);	return;// 일단 disconnect 이후 로그인 fail 패킷으로 변경
			}
		}
		else {
			// 자동 배정 (비어있는 Scene 혹은 비어있는 자리)
		}

		printf("send login packet\n");
		clients[c_id].send_login_info_packet();

		SC_LOGIN_INFO_PACKET packet;
		packet.type = SC_ADD_PLAYER;
		packet.size = sizeof(packet);
		packet.data.id = clients[c_id].room_pid;
		CScene* scene = scene_manager.GetScene(clients[c_id].room_id);
		packet.data.yaw = scene->m_ppPlayers[clients[c_id].room_pid]->GetYaw();
		packet.data.player_type = clients[c_id].type;
		packet.data.x = scene->m_ppPlayers[clients[c_id].room_pid]->GetPosition().x;
		packet.data.z = scene->m_ppPlayers[clients[c_id].room_pid]->GetPosition().z;
		
		scene_manager.Send(clients[c_id].room_id, (char*)& packet);
		break;
	}
	case CS_CHANGE: {
		CS_CHANGE_PACKET* p = reinterpret_cast<CS_CHANGE_PACKET*>(packet);
		if (p->player_type == PlayerType::INSIDE)
		{
			clients[c_id].type = PlayerType::INSIDE;

			SC_LOGIN_INFO_PACKET packet;
			packet.data.id = c_id;
			packet.data.player_type = p->player_type;
			packet.size = sizeof(SC_LOGIN_INFO_PACKET);
			packet.type = SC_CHANGE;

			scene_manager.Send(clients[c_id].room_id, (char*)&packet);	// 수정	
		}
		else
		{
			bool exists = scene_manager.GetCanSit(clients[c_id].room_id, p->player_type);

			if (exists) {
				clients[c_id].type = p->player_type;

				// 미션
				if (scene_manager.GetScene(clients[c_id].room_id)->cur_mission == MissionType::TU_SIT && p->player_type == PlayerType::MOVE)
				{
					scene_manager.GetScene(clients[c_id].room_id)->MissionClear();
				}


				SC_LOGIN_INFO_PACKET packet;
				packet.data.id = c_id;
				packet.data.player_type = p->player_type;
				packet.size = sizeof(SC_LOGIN_INFO_PACKET);
				packet.type = SC_CHANGE;

				scene_manager.Send(clients[c_id].room_id, (char*)&packet);
			}
		}
		break;
	}
	case CS_SPACESHIP_MOVE: {
		CS_SPACESHIP_PACKET* p = reinterpret_cast<CS_SPACESHIP_PACKET*>(packet);
		if (clients[c_id].type == PlayerType::MOVE) {
			scene_manager.GetScene(clients[c_id].room_id)->m_pSpaceship->SetInputInfo(p->data);
		}
		break;
	}
	case CS_INSIDE_MOVE: {
		CS_INSIDE_PACKET* p = reinterpret_cast<CS_INSIDE_PACKET*>(packet);
		if (clients[c_id].type == PlayerType::INSIDE) {
			scene_manager.GetScene(clients[c_id].room_id)->m_ppPlayers[clients[c_id].room_pid]->SetInputInfo(p->data);
		}
		break;
	}
	case CS_ATTACK: {
		CScene* m_pScene = scene_manager.GetScene(clients[c_id].room_id);

		CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
		if (isnan(p->data.direction.x) || isnan(p->data.direction.y) || isnan(p->data.direction.z)) { break; }
		if (clients[c_id].type >= PlayerType::ATTACK1 && clients[c_id].type <= PlayerType::ATTACK3) {
			//if (m_pScene->m_pSpaceship->CanAttack((short)clients[c_id].type - (short)PlayerType::ATTACK1)) {
			m_pScene->CheckEnemyByBulletCollisions(p->data);
			m_pScene->CheckMeteoByBulletCollisions(p->data);

			for (short pl_id : m_pScene->_plist) {
				if (pl_id == -1) continue;
				if (clients[pl_id]._state != ST_INGAME) continue;
				clients[pl_id].send_bullet_packet(0, p->data.pos, p->data.direction);

			}
			//}
		}
		break;
	}
	case CS_HEAL: {
		CS_HEAL_PACKET* p = reinterpret_cast<CS_HEAL_PACKET*>(packet);
		CScene* m_pScene = scene_manager.GetScene(clients[c_id].room_id);

		if (p->start && m_pScene->heal_player == -1) {
			m_pScene->heal_start = std::chrono::system_clock::now();
			m_pScene->heal_player = c_id;
			clients[c_id].send_heal_packet();
		}
		else {
			if (m_pScene->heal_player == c_id) {
				m_pScene->heal_player = -1;

				// 미션
				if (m_pScene->cur_mission == MissionType::TU_HILL)
				{
					m_pScene->MissionClear();
					m_pScene->MissionClear(); // 일단 TU_END 건너뜀
				}
			}
		}
		break;
	}
	case CS_NEXT_MISSION: {
		scene_manager.GetScene(clients[c_id].room_id)->MissionClear();
		break;
	}
	case CS_START: {
		if (clients[c_id].room_id != -1) {
			CScene* scene = scene_manager.GetScene(clients[c_id].room_id);
			scene->Start();

			SC_START_PACKET packet;
			packet.size = sizeof(SC_START_PACKET);
			packet.type = SC_START;

			printf("start %d\n", clients[c_id].room_id);
			scene_manager.Send(clients[c_id].room_id, (char*)&packet);
		}
		break;
	}
	}
	//prev_process_time = chrono::system_clock::now();
}

void CGameFramework::ClientProcess()
{
	auto StartTime = chrono::system_clock::now();
	auto EndTime = chrono::system_clock::now();

	while (true) {
		EndTime = chrono::system_clock::now();
		fps = EndTime - StartTime;
		if (fps.count() > 0.0333333) {
			StartTime = chrono::system_clock::now();
			AnimateObjects(fps.count());
		}
	}
}

int CGameFramework::get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		lock_guard <mutex> ll{ clients[i]._s_lock };
		if (clients[i]._state == ST_FREE)
			return i;
	}
	return -1;
}


void CGameFramework::disconnect(int c_id)
{	
	SC_REMOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;

	lock_guard<mutex> ll(clients[c_id]._s_lock);
	if (clients[c_id].room_id != -1) {
		scene_manager.Send(clients[c_id].room_id, (char*)&p);

		CScene* scene = scene_manager.GetScene(clients[c_id].room_id);

		scene->_plist_lock.lock();
		scene->_plist[clients[c_id].room_pid] = -1;


		if (std::all_of(scene->_plist.begin(), scene->_plist.end(), [](short i) {return i == -1; })) {
			scene_manager.ResetScene(clients[c_id].room_id);
		}
		scene->_plist_lock.unlock();
	}

	clients[c_id]._state = ST_FREE;
	closesocket(clients[c_id]._socket);
}



//------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//---------------------------------------------------------------------
