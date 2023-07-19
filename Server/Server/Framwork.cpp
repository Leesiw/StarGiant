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
		case OP_SEND: {
			delete ex_over;
			break;
		}
		case OP_SPAWN_ENEMY: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			scene->SpawnEnemy();
			delete ex_over;
			break;
		}
		case OP_UPDATE_ENEMY: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			scene->UpdateEnemy(ex_over->obj_id);
			delete ex_over;
			break;
		}
		case OP_UPDATE_METEO: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			scene->UpdateMeteo(ex_over->obj_id);
			delete ex_over;
			break;
		}
		case OP_SPAWN_MISSILE: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			scene->SpawnMissile(ex_over->obj_id);
			delete ex_over;
			break;
		}
		case OP_UPDATE_MISSILE: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			scene->UpdateMissile(ex_over->obj_id);
			delete ex_over;
			break;
		}
		case OP_UPDATE_BOSS: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			scene->UpdateBoss();
			delete ex_over;
			break;
		}
		case OP_UPDATE_GOD: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			scene->UpdateGod();
			delete ex_over;
			break;
		}
		case OP_UPDATE_SPACESHIP: {	
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			scene->UpdateSpaceship();
			delete ex_over;
			break;
		}
		case OP_HEAL: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			scene->Heal();
			delete ex_over;
			break;
		}
		case OP_RESET_SCENE: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			scene->_s_lock.lock();

			if (scene->_state == SCENE_RESET) {
				scene->_state = SCENE_FREE;
			}
			scene->_s_lock.unlock();
			delete ex_over;
			break;
		}
		case OP_MISSION_CLEAR: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			if (scene->_state == ST_INGAME && scene->cur_mission == MissionType::GO_CENTER) {
				scene->SetMission(MissionType::KILL_MONSTER3);
			}
			delete ex_over;
			break;
		}
		case OP_SEND_SCENE_INFO: {	// 우주선 좌표, 적 좌표, 미사일 좌표 한번에 send 
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			scene->SendSceneInfo();
			delete ex_over;
			break;
		}
		case OP_BLACK_HOLE: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			scene->BlackHole();
			delete ex_over;
			break;
		}
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
	SOCKADDR_IN cl_addr{};
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
	thread timer_thread{ &CGameFramework::TimerThread, this, h_iocp };
	for (auto& th : worker_threads)
		th.join();
	timer_thread.join();
	closesocket(g_s_socket);
	WSACleanup();
}

void CGameFramework::SetMission()
{
	levels[MissionType::CS_TURN].NextMission = MissionType::TU_SIT;
	levels[MissionType::CS_TURN].cutscene = true;

	levels[MissionType::TU_SIT].NextMission = MissionType::TU_KILL;
	levels[MissionType::TU_KILL].NextMission = MissionType::TU_HILL;
	levels[MissionType::TU_HILL].NextMission = MissionType::TU_END;
	levels[MissionType::TU_END].NextMission = MissionType::GET_JEWELS;

	for (int i = static_cast<int>(MissionType::TU_SIT);
		i <= static_cast<int>(MissionType::TU_END); ++i) {
		MissionType m = static_cast<MissionType>(i);
		levels[m].MaxMonsterNum = 5;
		levels[m].SpawnMonsterNum = 3;
		levels[m].Laser.MAX_HP = 3;
		levels[m].Laser.ATK = 2;
		levels[m].Missile.MAX_HP = 5;
		levels[m].Missile.ATK = 3;
		levels[m].PlasmaCannon.MAX_HP = 10;
		levels[m].PlasmaCannon.ATK = 4;
	}

	levels[MissionType::GET_JEWELS].NextMission = MissionType::Kill_MONSTER;
	levels[MissionType::GET_JEWELS].MaxMonsterNum = 10;
	levels[MissionType::GET_JEWELS].SpawnMonsterNum = 4;
	levels[MissionType::GET_JEWELS].Laser.MAX_HP = 5;
	levels[MissionType::GET_JEWELS].Laser.ATK = 3;
	levels[MissionType::GET_JEWELS].Missile.MAX_HP = 7;
	levels[MissionType::GET_JEWELS].Missile.ATK = 4;
	levels[MissionType::GET_JEWELS].PlasmaCannon.MAX_HP = 12;
	levels[MissionType::GET_JEWELS].PlasmaCannon.ATK = 5;

	levels[MissionType::Kill_MONSTER].NextMission = MissionType::CS_SHOW_PLANET;
	levels[MissionType::Kill_MONSTER].MaxMonsterNum = 15;
	levels[MissionType::Kill_MONSTER].SpawnMonsterNum = 5;
	levels[MissionType::Kill_MONSTER].Laser.MAX_HP = 10;
	levels[MissionType::Kill_MONSTER].Laser.ATK = 6;
	levels[MissionType::Kill_MONSTER].Missile.MAX_HP = 14;
	levels[MissionType::Kill_MONSTER].Missile.ATK = 8;
	levels[MissionType::Kill_MONSTER].PlasmaCannon.MAX_HP = 15;
	levels[MissionType::Kill_MONSTER].PlasmaCannon.ATK = 7;

	levels[MissionType::CS_SHOW_PLANET].NextMission = MissionType::GO_PLANET;
	levels[MissionType::CS_SHOW_PLANET].cutscene = true;

	levels[MissionType::GO_PLANET].NextMission = MissionType::KILL_MONSTER_ONE_MORE_TIME;
	levels[MissionType::GO_PLANET].MaxMonsterNum = 15;
	levels[MissionType::GO_PLANET].SpawnMonsterNum = 5;
	levels[MissionType::GO_PLANET].Laser.MAX_HP = 15;
	levels[MissionType::GO_PLANET].Laser.ATK = 10;
	levels[MissionType::GO_PLANET].Missile.MAX_HP = 17;
	levels[MissionType::GO_PLANET].Missile.ATK = 12;
	levels[MissionType::GO_PLANET].PlasmaCannon.MAX_HP = 20;
	levels[MissionType::GO_PLANET].PlasmaCannon.ATK = 11;

	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].NextMission = MissionType::FIND_BOSS;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].MaxMonsterNum = 20;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].SpawnMonsterNum = 5;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].Laser.MAX_HP = 20;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].Laser.ATK = 18;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].Missile.MAX_HP = 35;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].Missile.ATK = 20;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].PlasmaCannon.MAX_HP = 40;
	levels[MissionType::KILL_MONSTER_ONE_MORE_TIME].PlasmaCannon.ATK = 30;

	levels[MissionType::FIND_BOSS].NextMission = MissionType::CS_BOSS_SCREAM;
	levels[MissionType::FIND_BOSS].MaxMonsterNum = ENEMIES;
	levels[MissionType::FIND_BOSS].SpawnMonsterNum = 9;
	levels[MissionType::FIND_BOSS].Laser.MAX_HP = 20;
	levels[MissionType::FIND_BOSS].Laser.ATK = 18;
	levels[MissionType::FIND_BOSS].Missile.MAX_HP = 35;
	levels[MissionType::FIND_BOSS].Missile.ATK = 20;
	levels[MissionType::FIND_BOSS].PlasmaCannon.MAX_HP = 40;
	levels[MissionType::FIND_BOSS].PlasmaCannon.ATK = 30;

	levels[MissionType::CS_BOSS_SCREAM].NextMission = MissionType::DEFEAT_BOSS;
	levels[MissionType::CS_BOSS_SCREAM].cutscene = true;

	levels[MissionType::DEFEAT_BOSS].NextMission = MissionType::CS_SHOW_STARGIANT;
	levels[MissionType::DEFEAT_BOSS].MaxMonsterNum = 0;
	levels[MissionType::DEFEAT_BOSS].SpawnMonsterNum = 0;
	levels[MissionType::DEFEAT_BOSS].Laser.MAX_HP = 20;
	levels[MissionType::DEFEAT_BOSS].Laser.ATK = 18;
	levels[MissionType::DEFEAT_BOSS].Missile.MAX_HP = 35;
	levels[MissionType::DEFEAT_BOSS].Missile.ATK = 20;
	levels[MissionType::DEFEAT_BOSS].PlasmaCannon.MAX_HP = 40;
	levels[MissionType::DEFEAT_BOSS].PlasmaCannon.ATK = 30;

	levels[MissionType::CS_SHOW_STARGIANT].NextMission = MissionType::GO_CENTER;
	levels[MissionType::CS_SHOW_STARGIANT].cutscene = true;

	levels[MissionType::GO_CENTER].NextMission = MissionType::KILL_MONSTER3;
	levels[MissionType::GO_CENTER].MaxMonsterNum = 5;
	levels[MissionType::GO_CENTER].SpawnMonsterNum = 3;
	levels[MissionType::GO_CENTER].Laser.MAX_HP = 40;
	levels[MissionType::GO_CENTER].Laser.ATK = 18;
	levels[MissionType::GO_CENTER].Missile.MAX_HP = 55;
	levels[MissionType::GO_CENTER].Missile.ATK = 20;
	levels[MissionType::GO_CENTER].PlasmaCannon.MAX_HP = 60;
	levels[MissionType::GO_CENTER].PlasmaCannon.ATK = 30;

	levels[MissionType::KILL_MONSTER3].NextMission = MissionType::KILL_METEOR;
	levels[MissionType::KILL_MONSTER3].MaxMonsterNum = 15;
	levels[MissionType::KILL_MONSTER3].SpawnMonsterNum = 6;
	levels[MissionType::KILL_MONSTER3].Laser.MAX_HP = 40;
	levels[MissionType::KILL_MONSTER3].Laser.ATK = 18;
	levels[MissionType::KILL_MONSTER3].Missile.MAX_HP = 55;
	levels[MissionType::KILL_MONSTER3].Missile.ATK = 20;
	levels[MissionType::KILL_MONSTER3].PlasmaCannon.MAX_HP = 60;
	levels[MissionType::KILL_MONSTER3].PlasmaCannon.ATK = 30;

	levels[MissionType::KILL_METEOR].NextMission = MissionType::CS_SHOW_BLACK_HOLE;
	levels[MissionType::KILL_METEOR].MaxMonsterNum = 10;
	levels[MissionType::KILL_METEOR].SpawnMonsterNum = 4;
	levels[MissionType::KILL_METEOR].Laser.MAX_HP = 40;
	levels[MissionType::KILL_METEOR].Laser.ATK = 18;
	levels[MissionType::KILL_METEOR].Missile.MAX_HP = 55;
	levels[MissionType::KILL_METEOR].Missile.ATK = 20;
	levels[MissionType::KILL_METEOR].PlasmaCannon.MAX_HP = 60;
	levels[MissionType::KILL_METEOR].PlasmaCannon.ATK = 30;

	levels[MissionType::CS_SHOW_BLACK_HOLE].NextMission = MissionType::ESCAPE_BLACK_HOLE;
	levels[MissionType::CS_SHOW_BLACK_HOLE].cutscene = true;

	levels[MissionType::ESCAPE_BLACK_HOLE].NextMission = MissionType::GO_CENTER_REAL;
	levels[MissionType::ESCAPE_BLACK_HOLE].MaxMonsterNum = 5;
	levels[MissionType::ESCAPE_BLACK_HOLE].SpawnMonsterNum = 2;
	levels[MissionType::ESCAPE_BLACK_HOLE].Laser.MAX_HP = 40;
	levels[MissionType::ESCAPE_BLACK_HOLE].Laser.ATK = 18;
	levels[MissionType::ESCAPE_BLACK_HOLE].Missile.MAX_HP = 55;
	levels[MissionType::ESCAPE_BLACK_HOLE].Missile.ATK = 20;
	levels[MissionType::ESCAPE_BLACK_HOLE].PlasmaCannon.MAX_HP = 60;
	levels[MissionType::ESCAPE_BLACK_HOLE].PlasmaCannon.ATK = 30;

	levels[MissionType::GO_CENTER_REAL].NextMission = MissionType::CS_SHOW_GOD;
	levels[MissionType::GO_CENTER_REAL].MaxMonsterNum = ENEMIES;
	levels[MissionType::GO_CENTER_REAL].SpawnMonsterNum = 5;
	levels[MissionType::GO_CENTER_REAL].Laser.MAX_HP = 40;
	levels[MissionType::GO_CENTER_REAL].Laser.ATK = 18;
	levels[MissionType::GO_CENTER_REAL].Missile.MAX_HP = 55;
	levels[MissionType::GO_CENTER_REAL].Missile.ATK = 20;
	levels[MissionType::GO_CENTER_REAL].PlasmaCannon.MAX_HP = 60;
	levels[MissionType::GO_CENTER_REAL].PlasmaCannon.ATK = 30;


	levels[MissionType::CS_SHOW_GOD].NextMission = MissionType::KILL_GOD;
	levels[MissionType::CS_SHOW_GOD].cutscene = true;

	levels[MissionType::KILL_GOD].NextMission = MissionType::CS_ENDING;
	levels[MissionType::KILL_GOD].MaxMonsterNum = 0;
	levels[MissionType::KILL_GOD].SpawnMonsterNum = 0;
	levels[MissionType::KILL_GOD].Laser.MAX_HP = 40;
	levels[MissionType::KILL_GOD].Laser.ATK = 18;
	levels[MissionType::KILL_GOD].Missile.MAX_HP = 55;
	levels[MissionType::KILL_GOD].Missile.ATK = 20;
	levels[MissionType::KILL_GOD].PlasmaCannon.MAX_HP = 60;
	levels[MissionType::KILL_GOD].PlasmaCannon.ATK = 30;

	levels[MissionType::CS_ENDING].NextMission = MissionType::CS_ENDING;
	levels[MissionType::CS_ENDING].cutscene = true;
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
		if (clients[c_id].room_id != -1) {	// 이미 배정된 방이 있을 때
			CScene* scene = scene_manager.GetScene(clients[c_id].room_id);
			scene->_plist_lock.lock();
			scene->_plist[clients[c_id].room_pid] = -1;
			clients[c_id].room_id = -1;
			clients[c_id].room_pid = -1;
			scene->_plist_lock.unlock();
		}


		if (p->room_id != -1) {
			scene_manager._scene_lock.lock();
			short scene_num = scene_manager.FindScene(p->room_id, c_id);
			scene_manager._scene_lock.unlock();

			if (scene_num >= 0) {
				char num = scene_manager.InsertPlayer(scene_num, c_id);
				//if (num == 2) { scene_manager.SceneStart(scene_num); }
				if (num == -1) { disconnect(c_id); return; }  // 일단 disconnect 이후 로그인 fail 패킷으로 변경
				else if(num == 0) {
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
			scene_manager._scene_lock.lock();
			short scene_num = scene_manager.FindScene(c_id);
			scene_manager._scene_lock.unlock();

			if(scene_num == -1) {
				disconnect(c_id);	return;// 일단 disconnect 이후 로그인 fail 패킷으로 변경
			}

			// 자동 배정 (비어있는 Scene 혹은 비어있는 자리)
		}

		clients[c_id].send_login_info_packet();

		SC_LOGIN_INFO_PACKET packet{};
		packet.type = SC_ADD_PLAYER;
		packet.size = sizeof(packet);
		packet.data.id = clients[c_id].room_pid;
		CScene* scene = scene_manager.GetScene(clients[c_id].room_id);
		packet.data.yaw = scene->m_ppPlayers[clients[c_id].room_pid]->GetYaw();
		packet.data.player_type = clients[c_id].type;
		packet.data.x = scene->m_ppPlayers[clients[c_id].room_pid]->GetPosition().x;
		packet.data.z = scene->m_ppPlayers[clients[c_id].room_pid]->GetPosition().z;
		
		scene_manager.Send(clients[c_id].room_id, (char*)& packet);
		
		for (auto pl_id : scene->_plist) {
			if (pl_id == -1) { continue; }
			SC_LOGIN_INFO_PACKET my_packet{};
			my_packet.type = SC_ADD_PLAYER;
			my_packet.size = sizeof(my_packet);
			my_packet.data.id = clients[pl_id].room_pid;
			CScene* scene = scene_manager.GetScene(clients[pl_id].room_id);
			my_packet.data.yaw = scene->m_ppPlayers[clients[pl_id].room_pid]->GetYaw();
			my_packet.data.player_type = clients[pl_id].type;
			my_packet.data.x = scene->m_ppPlayers[clients[pl_id].room_pid]->GetPosition().x;
			my_packet.data.z = scene->m_ppPlayers[clients[pl_id].room_pid]->GetPosition().z;

			clients[c_id].do_send(&my_packet);
			
		}

		SC_MISSION_START_PACKET miss_packet{};
		miss_packet.size = sizeof(miss_packet);
		miss_packet.type = SC_MISSION_START;
		miss_packet.next_mission = scene->cur_mission;
		clients[c_id].do_send(&miss_packet);

		if (scene->cur_mission == MissionType::Kill_MONSTER || scene->cur_mission == MissionType::KILL_MONSTER_ONE_MORE_TIME 
			|| scene->cur_mission == MissionType::KILL_MONSTER3 || scene->cur_mission == MissionType::KILL_METEOR)
		{
			SC_KILL_NUM_PACKET pack{};
			pack.size = sizeof(pack);
			pack.type = SC_KILL_NUM;
			pack.num = scene->kill_monster_num;
			clients[c_id].do_send(&pack);
		}

		for (int i = 0; i < 4; ++i) {
			ITEM_INFO info{};
			info.type = (ItemType)i;
			info.num = scene->items[info.type];
			clients[c_id].send_item_packet(info);
		}
		break;
	}
	case CS_CHANGE: {
		CS_CHANGE_PACKET* p = reinterpret_cast<CS_CHANGE_PACKET*>(packet);
		if (clients[c_id].room_id == -1) { break; }
		if (p->player_type == PlayerType::INSIDE)
		{
			CScene* scene = scene_manager.GetScene(clients[c_id].room_id);
			if (clients[c_id].type >= PlayerType::MOVE) {
				char sit_num = (char)clients[c_id].type - (char)PlayerType::MOVE;
				scene->can_sit[sit_num] = true;
			};
			clients[c_id].type = PlayerType::INSIDE;

			SC_LOGIN_INFO_PACKET packet{};
			packet.data.id = c_id;
			packet.data.player_type = p->player_type;
			packet.size = sizeof(SC_LOGIN_INFO_PACKET);
			packet.type = SC_CHANGE;

			scene_manager.Send(clients[c_id].room_id, (char*)&packet);	// 수정	

			SC_MOVE_INSIDE_PACKET pack{};
			pack.size = sizeof(pack);
			pack.type = SC_MOVE_INSIDEPLAYER;
			pack.data.id = clients[c_id].room_pid;
			pack.data.m_fYaw = scene->m_ppPlayers[clients[c_id].room_pid]->GetYaw();
			pack.data.pos = scene->m_ppPlayers[clients[c_id].room_pid]->GetPosition();

			scene->Send((char*)&pack);
		}
		else
		{
			CScene* scene = scene_manager.GetScene(clients[c_id].room_id);
			char sit_num = (char)p->player_type - (char)PlayerType::MOVE;
			bool o_state = true;
			if (false == atomic_compare_exchange_strong(&scene->can_sit[sit_num], &o_state, false))
				break;
		//	bool exists = scene_manager.GetCanSit(clients[c_id].room_id, p->player_type);
			clients[c_id].type = p->player_type;

			// 미션
			if (scene_manager.GetScene(clients[c_id].room_id)->cur_mission == MissionType::TU_SIT && p->player_type == PlayerType::MOVE)
			{
				scene->SetMission(MissionType::TU_KILL);
			}


			SC_LOGIN_INFO_PACKET packet{};
			packet.data.id = clients[c_id].room_pid;
			packet.data.player_type = p->player_type;
			packet.size = sizeof(SC_LOGIN_INFO_PACKET);
			packet.type = SC_CHANGE;

			scene_manager.Send(clients[c_id].room_id, (char*)&packet);
		}
		break;
	}
	case CS_SPACESHIP_MOVE: {
		CS_SPACESHIP_PACKET* p = reinterpret_cast<CS_SPACESHIP_PACKET*>(packet);
		if (clients[c_id].type == PlayerType::MOVE) {
			if (clients[c_id].room_id == -1) { break; }
			scene_manager.GetScene(clients[c_id].room_id)->m_pSpaceship->SetInputInfo(p->data, p->move_time);
		}
		break;
	}
	case CS_INSIDE_MOVE: {
		CS_INSIDE_PACKET* p = reinterpret_cast<CS_INSIDE_PACKET*>(packet);
		if (clients[c_id].type == PlayerType::INSIDE) {
			if (clients[c_id].room_id == -1) { break; }
			CScene* scene = scene_manager.GetScene(clients[c_id].room_id);
			float yaw = scene->m_ppPlayers[clients[c_id].room_pid]->GetYaw();
			if (yaw != p->data.yaw)
			{
				scene->m_ppPlayers[clients[c_id].room_pid]->Rotate(0, p->data.yaw - yaw, 0);
			}

			XMFLOAT3 pos[2]{};
			char num = 0;
			for (char i = 0; i < 3; ++i)
			{
				if (i == clients[c_id].room_pid) { continue; }
				if (scene->_plist[i] == -1) { continue; }
				pos[num] = scene->m_ppPlayers[i]->GetPosition();
				++num;
			}

			scene->m_ppPlayers[clients[c_id].room_pid]->Move(p->data.dwDirection, 2.64f, pos);

			SC_MOVE_INSIDE_PACKET pack{};
			pack.size = sizeof(pack);
			pack.type = SC_MOVE_INSIDEPLAYER;
			pack.data.id = clients[c_id].room_pid;
			pack.data.m_fYaw = yaw;
			pack.data.pos = scene->m_ppPlayers[clients[c_id].room_pid]->GetPosition();

			scene->Send((char*)&pack);
		}
		break;
	}
	case CS_ATTACK: {
		if (clients[c_id].room_id == -1) { break; }
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
				clients[pl_id].send_bullet_packet( p->data.pos, p->data.direction, p->attack_time);

			}
			//}
		}
		break;
	}
	case CS_HEAL: {
		if (clients[c_id].room_id == -1) { break; }
		CS_HEAL_PACKET* p = reinterpret_cast<CS_HEAL_PACKET*>(packet);
		CScene* m_pScene = scene_manager.GetScene(clients[c_id].room_id);

		if (p->start) {
			char o_state = -1;
			if (false == atomic_compare_exchange_strong(&m_pScene->heal_player, &o_state, clients[c_id].room_pid)) {
				break;
			}
			m_pScene->heal_start = std::chrono::system_clock::now();
			clients[c_id].send_heal_packet();
			TIMER_EVENT ev{ 0, chrono::system_clock::now() + 1s, EV_HEAL, clients[c_id].room_id};
			timer_queue.push(ev);
		}
		else {
			if (m_pScene->heal_player == clients[c_id].room_pid) {
				m_pScene->heal_player = -1;

				// 미션
				if (m_pScene->cur_mission == MissionType::TU_HILL)
				{
					m_pScene->SetMission(MissionType::GET_JEWELS);
					//m_pScene->MissionClear(); // 일단 TU_END 건너뜀
				}
			}
		}
		break;
	}
	case CS_NEXT_MISSION: {
		if (clients[c_id].room_id == -1) { break; }
		scene_manager.GetScene(clients[c_id].room_id)->MissionClear();
		break;
	}
	case CS_START: {
		if (clients[c_id].room_id != -1) {
			CScene* scene = scene_manager.GetScene(clients[c_id].room_id);
			scene->Start();

			SC_START_PACKET packet{};
			packet.size = sizeof(SC_START_PACKET);
			packet.type = SC_START;

			//printf("start %d\n", clients[c_id].room_id);
			scene_manager.Send(clients[c_id].room_id, (char*)&packet);
		}
		break;
	}
	case CS_CUTSCENE_END: {
		if (clients[c_id].room_id != -1) {
			CScene* scene = scene_manager.GetScene(clients[c_id].room_id);
			if (levels[scene->cur_mission].cutscene) {
				scene->m_ppPlayers[clients[c_id].room_pid]->cutscene_end = true;
				char num = 0;
				for (char i = 0; i < 3; ++i) {
					if (scene->m_ppPlayers[i]->cutscene_end) {
						++num;
					}
				}

				SC_CUTSCENE_END_NUM_PACKET packet{};
				packet.size = sizeof(SC_CUTSCENE_END_NUM_PACKET);
				packet.type = SC_CUTSCENE_END_NUM;
				packet.num = num;
				scene->Send((char*)& packet);
			}
		}	
		break;
	}
	}
	//prev_process_time = chrono::system_clock::now();
}

void CGameFramework::TimerThread(HANDLE h_iocp)
{
	while (true) {
		TIMER_EVENT ev;
		auto current_time = chrono::system_clock::now();

		if (true == timer_queue.try_pop(ev)) {
			if (ev.wakeup_time > current_time) {
				timer_queue.push(ev);		// 최적화 필요
				// timer_queue에 다시 넣지 않고 처리해야 한다.
				this_thread::sleep_for(1ms);  // 실행시간이 아직 안되었으므로 잠시 대기
				continue;
			}
			switch (ev.event_id) {
			case EV_SPAWN_ENEMY: {
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_SPAWN_ENEMY;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id, &ov->_over);
				break;
			}
			case EV_UPDATE_ENEMY: {
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_UPDATE_ENEMY;
				ov->obj_id = ev.obj_id;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id, &ov->_over);
				break;
			}
			case EV_UPDATE_METEO: {
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_UPDATE_METEO;
				ov->obj_id = ev.obj_id;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id, &ov->_over);
				break;
			}
			case EV_SPAWN_MISSILE: {
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_SPAWN_MISSILE;
				ov->obj_id = ev.obj_id;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id, &ov->_over);
				break;
			}
			case EV_UPDATE_MISSILE: {
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_UPDATE_MISSILE;
				ov->obj_id = ev.obj_id;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id, &ov->_over);
				break;
			}
			case EV_UPDATE_BOSS: {
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_UPDATE_BOSS;
				ov->obj_id = ev.obj_id;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id, &ov->_over);
				break;
			}
			case EV_UPDATE_GOD: {
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_UPDATE_GOD;
				ov->obj_id = ev.obj_id;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id, &ov->_over);
				break;
			}
			case EV_UPDATE_SPACESHIP: {
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_UPDATE_SPACESHIP;
				ov->obj_id = ev.obj_id;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id, &ov->_over);
				break;
			}
			case EV_HEAL: {
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_HEAL;
				ov->obj_id = ev.obj_id;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id, &ov->_over);
				break;
			}
			case EV_RESET_SCENE: {
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_RESET_SCENE;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id, &ov->_over);
				break;
			}
			case EV_MISSION_CLEAR: {
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_MISSION_CLEAR;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id, &ov->_over);
				break;
			}
			case EV_SEND_SCENE_INFO: {
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_SEND_SCENE_INFO;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id, &ov->_over);
				break;
			}
			case EV_BLACK_HOLE: {
				OVER_EXP* ov = new OVER_EXP;
				ov->_comp_type = OP_BLACK_HOLE;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id, &ov->_over);
				break;
			}
			}
			continue;		// 즉시 다음 작업 꺼내기
		}
		this_thread::sleep_for(1ms);   // timer_queue가 비어 있으니 잠시 기다렸다가 다시 시작
	}
}

void CGameFramework::ClientProcess()
{
	/*
	auto StartTime = chrono::system_clock::now();
	auto EndTime = chrono::system_clock::now();

	while (true) {
		EndTime = chrono::system_clock::now();
		fps = EndTime - StartTime;
		if (fps.count() > 0.0333333) {
			StartTime = chrono::system_clock::now();
			AnimateObjects(fps.count());
		//	printf("fps.count = %f\n", fps.count());
			
		}
	}*/


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
	SC_REMOVE_PLAYER_PACKET p{};
	p.id = c_id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;

	lock_guard<mutex> ll(clients[c_id]._s_lock);
	if (clients[c_id].room_id != -1) {
		scene_manager.Send(clients[c_id].room_id, (char*)&p);

		CScene* scene = scene_manager.GetScene(clients[c_id].room_id);
		if (scene->heal_player == clients[c_id].room_pid) {
			scene->heal_player = -1;
		}
		if (clients[c_id].type >= PlayerType::MOVE) {
			char sit_num = (char)clients[c_id].type - (char)PlayerType::MOVE;
			scene->can_sit[sit_num] = true;
		};
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
