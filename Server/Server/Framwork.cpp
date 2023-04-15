#include "Framework.h"

CGameFramework::CGameFramework()
{
	m_pScene = NULL; 
}

CGameFramework::~CGameFramework()
{
	
}

void CGameFramework::Init() {

	SetMission();
	BuildObjects();

	HANDLE h_iocp;

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	SOCKET server = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(server, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	int client_id = 0;

	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(server), h_iocp, 9999, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EXP a_over;
	a_over._comp_type = OP_ACCEPT;
	AcceptEx(server, c_socket, a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over._over);


	for (auto& pl : clients) {
		if (false == pl.in_use) continue;
		pl.type = PlayerType::INSIDE;
	}

	ClientProcessThread = thread{ &CGameFramework::ClientProcess, this };

	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) std::cout << "Accept Error";
			else {
				std::cout << "GQCS Error on client[" << key << "]\n";
				disconnect(key);
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}
		
		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = get_new_client_id();
			if (client_id != -1) {
				clients[client_id].in_use = true;
				clients[client_id]._id = client_id;
				clients[client_id]._name[0] = 0;
				clients[client_id]._prev_remain = 0;
				clients[client_id]._socket = c_socket;

				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket),
					h_iocp, client_id, 0);
				clients[client_id].do_recv();
				clients[client_id].send_login_info_packet();
				clients[client_id].send_spawn_all_meteo_packet(0, m_pScene->m_ppMeteoObjects);

				LOGIN_INFO info;
				info.id = client_id;
				info.player_type = clients[client_id].type;
				info.x = m_pScene->m_ppPlayers[client_id]->GetPosition().x;
				info.z = m_pScene->m_ppPlayers[client_id]->GetPosition().z;
				info.yaw = m_pScene->m_ppPlayers[client_id]->GetYaw();

				for (auto& pl : clients) {
					if (false == pl.in_use) continue;
				//	if (pl._id == client_id) continue;
					pl.send_add_player_packet(info);
				}

				for (auto& pl : clients) {
					if (false == pl.in_use) continue;
					if (pl._id == client_id) continue;
					LOGIN_INFO linfo;
					linfo.id = pl._id;
					linfo.player_type = pl.type;
					linfo.x = m_pScene->m_ppPlayers[pl._id]->GetPosition().x;
					linfo.z = m_pScene->m_ppPlayers[pl._id]->GetPosition().z;
					linfo.yaw = m_pScene->m_ppPlayers[pl._id]->GetYaw();

					clients[client_id].send_add_player_packet(linfo);
				}

				ENEMY_INFO e_info[ENEMIES];
				bool Alive[ENEMIES];
				for (int i = 0; i < ENEMIES; ++i) {
					Alive[i] = m_pScene->m_ppEnemies[i]->GetisAlive();
					if (Alive[i]) {
						SPAWN_ENEMY_INFO e_info;
						e_info.id = m_pScene->m_ppEnemies[i]->GetID();
						e_info.Quaternion = m_pScene->m_ppEnemies[i]->GetQuaternion();
						e_info.pos = m_pScene->m_ppEnemies[i]->GetPosition();
						e_info.destination = m_pScene->m_ppEnemies[i]->GetDestination();
						e_info.max_hp = m_pScene->m_ppEnemies[i]->GetHP();
						e_info.state = m_pScene->m_ppEnemies[i]->GetState();

						clients[client_id].send_spawn_enemy_packet(0, e_info);
					}
				}
				clients[client_id].send_mission_start_packet(m_pScene->cur_mission);

				if (m_pScene->cur_mission == MissionType::Kill_MONSTER ||
					m_pScene->cur_mission == MissionType::KILL_MONSTER_ONE_MORE_TIME) {
					clients[client_id].send_kill_num_packet(m_pScene->kill_monster_num);
				}


				c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				std::cout << "Max user exceeded.\n";
			}
			ZeroMemory(&a_over._over, sizeof(a_over._over));
			AcceptEx(server, c_socket, a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over._over);
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
	if (ClientProcessThread.joinable())
		ClientProcessThread.join();
	closesocket(server);
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
	m_pScene = new CScene();

	if (m_pScene) m_pScene->BuildObjects();
}

void CGameFramework::ReleaseObjects()
{
	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
}

void CGameFramework::AnimateObjects(float fTimeElapsed)
{
	if (!m_pScene->m_bIsRunning) { return; }
	if (m_pScene) m_pScene->AnimateObjects(fTimeElapsed);

	// 플레이어 hp가 소진될 시 게임 안 돌아가도록
	//if (m_pScene->m_pSpaceship->GetHP() <= 0) { m_pScene->m_bIsRunning = false; }
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
	case CS_CHANGE: {
		CS_CHANGE_PACKET* p = reinterpret_cast<CS_CHANGE_PACKET*>(packet);
		if (p->player_type == PlayerType::INSIDE)
		{
			clients[c_id].type = PlayerType::INSIDE;
			for (auto& pl : clients) {
				if (false == pl.in_use) continue;
				pl.send_change_packet(c_id, p->player_type);
			}
		}
		else 
		{
			bool exists = std::any_of(std::begin(clients), std::end(clients),
				[&](SESSION pl) {
				return pl.in_use && pl.type == p->player_type;
			});

			if (!exists) {
				clients[c_id].type = p->player_type;

				// 미션
				if (m_pScene->cur_mission == MissionType::TU_SIT && p->player_type == PlayerType::MOVE)
				{
					m_pScene->MissionClear();
				}

				for (auto& pl : clients) {
					if (false == pl.in_use) continue;
					pl.send_change_packet(c_id, p->player_type);
				}
			}
		}
		break;
	}
	case CS_SPACESHIP_MOVE: {
		CS_SPACESHIP_PACKET* p = reinterpret_cast<CS_SPACESHIP_PACKET*>(packet);
		if (clients[c_id].type == PlayerType::MOVE) {
			m_pScene->m_pSpaceship->SetInputInfo(p->data);
		}
		break;
	}
	case CS_INSIDE_MOVE: {
		CS_INSIDE_PACKET* p = reinterpret_cast<CS_INSIDE_PACKET*>(packet);
		if (clients[c_id].type == PlayerType::INSIDE) {
			m_pScene->m_ppPlayers[c_id]->SetInputInfo(p->data);
		}
		break;
	}
	case CS_ATTACK: {
		CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
		
		if (clients[c_id].type >= PlayerType::ATTACK1 && clients[c_id].type <= PlayerType::ATTACK3) {
			if (m_pScene->m_pSpaceship->CanAttack((short)clients[c_id].type - (short)PlayerType::ATTACK1)) {
				m_pScene->CheckEnemyByBulletCollisions(p->data);
				m_pScene->CheckMeteoByBulletCollisions(p->data);
				for (auto& pl : clients)
				{
					if (false == pl.in_use) continue;
					pl.send_bullet_packet(0, p->data.pos, p->data.direction);
				}
			}
		}
		break;
	}
	case CS_HEAL: {
		CS_HEAL_PACKET* p = reinterpret_cast<CS_HEAL_PACKET*>(packet);
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
	for (int i = 0; i < MAX_USER; ++i)
		if (clients[i].in_use == false)
			return i;
	return -1;
}


void CGameFramework::disconnect(int c_id)
{
	clients[c_id].type = PlayerType::INSIDE;

	SC_REMOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;
	for (auto& pl : clients) {
		if (pl.in_use == false) continue;
		if (pl._id == c_id) continue;
		pl.do_send(&p);
	}
	closesocket(clients[c_id]._socket);
	clients[c_id].in_use = false;
}



//------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//---------------------------------------------------------------------
