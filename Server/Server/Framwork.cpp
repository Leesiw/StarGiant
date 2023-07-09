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
			if (scene->_state != ST_INGAME) { break; }
			if (levels[scene->cur_mission].cutscene) {
				TIMER_EVENT ev{ 0, chrono::system_clock::now() + 20s, EV_SPAWN_ENEMY, static_cast<short>(key) };
				timer_queue.push(ev);
				break;
			}
			std::array<CEnemy*, ENEMIES> ppEnemies{ scene->m_ppEnemies };
			std::random_shuffle(ppEnemies.begin(), ppEnemies.end());

			char spawn_num = levels[scene->cur_mission].SpawnMonsterNum;
			
			for (int i = 0; i < ENEMIES; ++i)
			{
				if (levels[scene->cur_mission].MaxMonsterNum <= scene->cur_monster_num) { break; }
				if (spawn_num <= 0) { break; }
				if (!ppEnemies[i]->GetisAlive()) {
					scene->SpawnEnemy(ppEnemies[i]->GetID());
					scene->m_ppEnemies[ppEnemies[i]->GetID()]->prev_time = chrono::steady_clock::now();
					TIMER_EVENT ev_u{ ppEnemies[i]->GetID(), chrono::system_clock::now() + 33ms, EV_UPDATE_ENEMY, static_cast<short>(key) };
					timer_queue.push(ev_u);
					--spawn_num;
				}
			}
			TIMER_EVENT ev{0, chrono::system_clock::now() + 20s, EV_SPAWN_ENEMY, static_cast<short>(key) };
			timer_queue.push(ev);

			delete ex_over;
			break;
		}
		case OP_UPDATE_ENEMY: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			if (scene->_state != ST_INGAME) { break; }
			if (scene->m_ppEnemies[ex_over->obj_id]->hp <= 0) { 
				scene->m_ppEnemies[ex_over->obj_id]->SetisAlive(false);
				--scene->cur_monster_num;
				break; 
			}
			if (levels[scene->cur_mission].cutscene) {
				scene->m_ppEnemies[ex_over->obj_id]->prev_time = chrono::steady_clock::now();
				TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 1s, EV_UPDATE_ENEMY, static_cast<short>(key) };
				timer_queue.push(ev);
				break;
			}
			
			auto time_now = chrono::steady_clock::now();
			std::chrono::duration<float> elapsed_time = (time_now - scene->m_ppEnemies[ex_over->obj_id]->prev_time);
			scene->m_ppEnemies[ex_over->obj_id]->prev_time = time_now;

			scene->m_ppEnemies[ex_over->obj_id]->AI(elapsed_time.count(), scene->m_pSpaceship);
			scene->m_ppEnemies[ex_over->obj_id]->UpdateBoundingBox();

			// 운석과 충돌처리
			
			for (int i = 0; i < ENEMIES; ++i)
			{
				if (!scene->m_ppEnemies[i]->GetisAlive()) { continue; }
				if (scene->m_ppEnemies[ex_over->obj_id]->HierarchyIntersects(scene->m_ppEnemies[i]))
				{
					XMFLOAT3 xmf3Sub = scene->m_ppEnemies[i]->GetPosition();
					xmf3Sub = Vector3::Subtract(scene->m_ppEnemies[ex_over->obj_id]->GetPosition(), xmf3Sub);
					if (Vector3::Length(xmf3Sub) > 0.0001f) {
						xmf3Sub = Vector3::Normalize(xmf3Sub);
					}
					XMFLOAT3 vel = scene->m_ppEnemies[ex_over->obj_id]->GetVelocity();
					float fLen = Vector3::Length(vel) / 10.f;
					xmf3Sub = Vector3::ScalarProduct(xmf3Sub, fLen, false);
					XMFLOAT3 vel2 = scene->m_ppEnemies[i]->GetVelocity();

					scene->m_ppEnemies[ex_over->obj_id]->SetVelocity(Vector3::Add(vel, xmf3Sub));
					scene->m_ppEnemies[i]->SetVelocity(Vector3::Add(vel2, xmf3Sub, -1.f));
				}
			}

				ENEMY_INFO info{};
				info.id = ex_over->obj_id;
				info.pos = scene->m_ppEnemies[ex_over->obj_id]->GetPosition();
				info.Quaternion = scene->m_ppEnemies[ex_over->obj_id]->GetQuaternion();
				info.velocity = scene->m_ppEnemies[ex_over->obj_id]->GetVelocity();
				for (short pl_id : scene->_plist) {
					if (pl_id == -1) continue;
					if (clients[pl_id]._state != ST_INGAME) continue;
					//clients[pl_id].send_enemy_packet(info);
				}

				if (ex_over->obj_id >= 23
					&& scene->m_ppEnemies[ex_over->obj_id]->state == EnemyState::AIMING &&
					!scene->m_ppEnemies[ex_over->obj_id]->GetAttackTimer()) {
					scene->m_ppEnemies[ex_over->obj_id]->SetAttackTimerTrue();
					TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 10s, EV_SPAWN_MISSILE, static_cast<short>(key) };
					timer_queue.push(ev);
				}


		
			TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 10ms, EV_UPDATE_ENEMY, static_cast<short>(key) };
			timer_queue.push(ev);

			delete ex_over;
			break;
		}
		case OP_UPDATE_METEO: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			if (scene->_state != ST_INGAME) { break; }
			if (levels[scene->cur_mission].cutscene) {
				scene->m_ppMeteoObjects[ex_over->obj_id]->prev_time = chrono::steady_clock::now();
				TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 1s, EV_UPDATE_METEO, static_cast<short>(key) };
				timer_queue.push(ev);
				break;
			}

			auto time_now = chrono::steady_clock::now();
			std::chrono::duration<float> elapsed_time = (time_now - scene->m_ppMeteoObjects[ex_over->obj_id]->prev_time);
			scene->m_ppMeteoObjects[ex_over->obj_id]->prev_time = time_now;
			scene->m_ppMeteoObjects[ex_over->obj_id]->Animate(elapsed_time.count());
			
			XMFLOAT3 p_pos = scene->m_pSpaceship->GetPosition();
			XMFLOAT3 m_pos = scene->m_ppMeteoObjects[ex_over->obj_id]->GetPosition();
			float dist = Vector3::Length(Vector3::Subtract(m_pos, p_pos));
			if (dist > 1500.0f) {
				scene->SpawnMeteo(ex_over->obj_id);
			}
			scene->m_ppMeteoObjects[ex_over->obj_id]->UpdateBoundingBox();
			if (scene->m_pSpaceship->HierarchyIntersects(scene->m_ppMeteoObjects[ex_over->obj_id]))
			{
				XMFLOAT3 xmf3Sub = scene->m_pSpaceship->GetPosition();
				xmf3Sub = Vector3::Subtract(scene->m_ppMeteoObjects[ex_over->obj_id]->GetPosition(), xmf3Sub);
				if (Vector3::Length(xmf3Sub) > 0.0001f) {
					xmf3Sub = Vector3::Normalize(xmf3Sub);
				}
				XMFLOAT3 vel = scene->m_pSpaceship->GetVelocity();
				float fLen = Vector3::Length(vel);
				xmf3Sub = Vector3::ScalarProduct(xmf3Sub, fLen, false);

				scene->m_pSpaceship->SetVelocity(Vector3::Add(vel, xmf3Sub));
				scene->m_pSpaceship->SetHP(scene->m_pSpaceship->GetHP() - 2);
				
				scene->SpawnMeteo(ex_over->obj_id);
				
				for (short pl_id : scene->_plist) {
					if (pl_id == -1) continue;
					if (clients[pl_id]._state != ST_INGAME) continue;
					clients[pl_id].send_bullet_hit_packet(-1, scene->m_pSpaceship->GetHP());

				}
			}

			TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 10ms, EV_UPDATE_METEO, static_cast<short>(key) };
			timer_queue.push(ev);

			delete ex_over;
			break;
		}
		case OP_SPAWN_MISSILE: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			if (scene->_state != ST_INGAME) { break; }
			if (!scene->m_ppEnemies[ex_over->obj_id]->GetisAlive() || scene->m_ppEnemies[ex_over->obj_id]->state != EnemyState::AIMING) {
				scene->m_ppEnemies[ex_over->obj_id]->SetAttackTimerFalse();
				break; 
			}
			if (levels[scene->cur_mission].cutscene) {
				TIMER_EVENT ev{ 0, chrono::system_clock::now() + 10s, EV_SPAWN_MISSILE, static_cast<short>(key) };
				timer_queue.push(ev);
				break;
			}

			MissileInfo info{};
			info.StartPos = scene->m_ppEnemies[ex_over->obj_id]->GetPosition();
			info.Quaternion = scene->m_ppEnemies[ex_over->obj_id]->GetQuaternion();
			info.damage = levels[scene->cur_mission].Missile.ATK;

			// 미사일 생성, 미사일 타이머 push
			for (int i = 0; i < ENEMY_BULLETS; ++i) {
				if (!scene->m_ppMissiles[i]->GetisActive()) {
					scene->m_ppMissiles[i]->SetNewMissile(info);

					MISSILE_INFO m_info{};
					m_info.id = i;
					m_info.pos = info.StartPos;
					m_info.Quaternion = info.Quaternion;

					for (short pl_id : scene->_plist) {
						if (pl_id == -1) continue;
						if (clients[pl_id]._state != ST_INGAME) continue;
						clients[pl_id].send_missile_packet(m_info);
					}
					scene->m_ppMissiles[i]->prev_time = chrono::steady_clock::now();
					TIMER_EVENT ev{ static_cast<char>(i), chrono::system_clock::now() + 30ms, EV_UPDATE_MISSILE, static_cast<short>(key) };
					timer_queue.push(ev);

					break;
				}
			}

			TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 10s, EV_SPAWN_MISSILE, static_cast<short>(key) };
			timer_queue.push(ev);
			delete ex_over;
			break;
		}

		case OP_UPDATE_MISSILE: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			if (scene->_state != ST_INGAME) { break; }
			if (!scene->m_ppMissiles[ex_over->obj_id]->GetisActive()) { break; }
			if (levels[scene->cur_mission].cutscene) {
				scene->m_ppMissiles[ex_over->obj_id]->prev_time = chrono::steady_clock::now();
				TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 1s, EV_UPDATE_MISSILE, static_cast<short>(key) };
				timer_queue.push(ev);
				break;
			}

			auto time_now = chrono::steady_clock::now();
			std::chrono::duration<float> elapsed_time = (time_now - scene->m_ppMissiles[ex_over->obj_id]->prev_time);
			scene->m_ppMissiles[ex_over->obj_id]->prev_time = time_now;
			scene->m_ppMissiles[ex_over->obj_id]->Animate(elapsed_time.count(), scene->m_pSpaceship);
			
			MISSILE_INFO m_info{};
			m_info.id = ex_over->obj_id;
			m_info.pos = scene->m_ppMissiles[ex_over->obj_id]->GetPosition();
			//printf("%f %f %f\n", m_info.pos.x, m_info.pos.y, m_info.pos.z);
			m_info.Quaternion = scene->m_ppMissiles[ex_over->obj_id]->GetQuaternion();

			scene->m_ppMissiles[ex_over->obj_id]->UpdateBoundingBox();
			if (scene->m_ppMissiles[ex_over->obj_id]->HierarchyIntersects(scene->m_pSpaceship))
			{
				scene->m_ppMissiles[ex_over->obj_id]->SetisActive(false);
				// 충돌처리
				/*
				XMFLOAT3 xmf3Sub = scene->m_pSpaceship->GetPosition();
				xmf3Sub = Vector3::Subtract(scene->m_ppMissiles[ex_over->obj_id]->GetPosition(), xmf3Sub);
				if (Vector3::Length(xmf3Sub) > 0.0001f) {
					xmf3Sub = Vector3::Normalize(xmf3Sub);
				}
				float fLen = 100.f;
				xmf3Sub = Vector3::ScalarProduct(xmf3Sub, fLen, false);

				XMFLOAT3 vel2 = scene->m_pSpaceship->GetVelocity();

			//	scene->m_pSpaceship->SetVelocity(Vector3::Add(vel2, xmf3Sub, -1.f));
			*/
				if (scene->m_pSpaceship->GetHP() > 0) {
					scene->m_pSpaceship->GetAttack(scene->m_ppMissiles[ex_over->obj_id]->GetDamage());
				}

				for (short pl_id : scene->_plist) {
					if (pl_id == -1) continue;
					if (clients[pl_id]._state != ST_INGAME) continue;
					clients[pl_id].send_bullet_hit_packet( -1, scene->m_pSpaceship->GetHP());
				}
			}

			if (scene->m_ppMissiles[ex_over->obj_id]->GetisActive()) {
				for (short pl_id : scene->_plist) {
					if (pl_id == -1) continue;
					if (clients[pl_id]._state != ST_INGAME) continue;
					//clients[pl_id].send_missile_packet(m_info);
				}
				TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 10ms, EV_UPDATE_MISSILE, static_cast<short>(key) };
				timer_queue.push(ev);
			}
			else {
				for (short pl_id : scene->_plist) {
					if (pl_id == -1) continue;
					if (clients[pl_id]._state != ST_INGAME) continue;
					clients[pl_id].send_remove_missile_packet(ex_over->obj_id);
				}
			}
			delete ex_over;
			break;
		}
		case OP_UPDATE_BOSS: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			if (scene->_state != ST_INGAME) { break; }
			if (scene->m_pBoss->BossHP <= 0) { 
				scene->MissionClear();
				TIMER_EVENT ev{ 0, chrono::system_clock::now() + 20s, EV_MISSION_CLEAR, key };
				timer_queue.push(ev);
				break; 
			}
			if (levels[scene->cur_mission].cutscene) {
				TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 1s, EV_UPDATE_BOSS, static_cast<short>(key) };
				timer_queue.push(ev);
				break;
			}

			scene->m_pBoss->Boss_Ai(0.01f, scene->m_pSpaceship, scene->m_pBoss->GetHP());

			float dist;
			dist = Vector3::Length(Vector3::Subtract(scene->m_pSpaceship->GetPosition(), scene->m_pBoss->GetPosition()));
			if (dist < 1000.f) // boss 막기
			{
				XMFLOAT3 ToGo = Vector3::Subtract(scene->m_pSpaceship->GetPosition(), scene->m_pBoss->GetPosition());
				ToGo = Vector3::ScalarProduct(ToGo, 800.f);
				ToGo = Vector3::Add(scene->m_pBoss->GetPosition(), ToGo);
				scene->m_pSpaceship->SetPosition(ToGo);
			}

			TIMER_EVENT ev{ 0, chrono::system_clock::now() + 10ms, EV_UPDATE_BOSS, static_cast<short>(key) };
			timer_queue.push(ev);
			delete ex_over;
			break;
		}
		case OP_UPDATE_GOD: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			if (scene->_state != ST_INGAME) { break; }
			if (scene->m_pGod->GetcurHp() <= 0) {
				scene->MissionClear();
				break;
			}
			if (levels[scene->cur_mission].cutscene) {
				TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 1s, EV_UPDATE_GOD, static_cast<short>(key) };
				timer_queue.push(ev);
				break;
			}

			scene->m_pGod->God_Ai(0.01f, scene->m_pSpaceship, scene->m_pGod->GetcurHp());

			float dist;
			dist = Vector3::Length(Vector3::Subtract(scene->m_pSpaceship->GetPosition(), scene->m_pGod->GetPosition()));
			if (dist < 2000.f) // boss 막기
			{
				XMFLOAT3 ToGo = Vector3::Subtract(scene->m_pSpaceship->GetPosition(), scene->m_pGod->GetPosition());
				ToGo = Vector3::ScalarProduct(ToGo, 800.f);
				ToGo = Vector3::Add(scene->m_pGod->GetPosition(), ToGo);
				scene->m_pSpaceship->SetPosition(ToGo);
			}

			TIMER_EVENT ev{ 0, chrono::system_clock::now() + 10ms, EV_UPDATE_GOD, static_cast<short>(key) };
			timer_queue.push(ev);
			delete ex_over;
			break;
		}
		case OP_UPDATE_SPACESHIP: {
			
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			if (scene->_state != ST_INGAME) { break; }
			if (levels[scene->cur_mission].cutscene) {
				bool cutscene_end = true;
				for (char i = 0; i < 3; ++i) {
					if (scene->_plist[i] == -1) { continue; }
					if (scene->m_ppPlayers[i]->cutscene_end == false) { cutscene_end = false; }
				}

				if(cutscene_end == true){
					scene->MissionClear();
					for (char i = 0; i < 3; ++i) {
						if (scene->_plist[i] == -1) { continue; }
						scene->m_ppPlayers[i]->cutscene_end = false;
					}
					scene->m_pSpaceship->prev_time = chrono::steady_clock::now();
					TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 10ms, EV_UPDATE_SPACESHIP, static_cast<short>(key) };
					timer_queue.push(ev);
					break;
				}

				TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 1s, EV_UPDATE_SPACESHIP, static_cast<short>(key) };
				timer_queue.push(ev);
				break;
			}
			auto time_now = chrono::steady_clock::now();
			std::chrono::duration<float> elapsed_time = (time_now - scene->m_pSpaceship->prev_time);
			scene->m_pSpaceship->prev_time = time_now;
			scene->m_pSpaceship->Update(elapsed_time.count());

			scene->CheckMissionComplete();

			TIMER_EVENT ev{ 0, chrono::system_clock::now() + 10ms, EV_UPDATE_SPACESHIP, static_cast<short>(key) };
			timer_queue.push(ev);
			delete ex_over;
			break;
		}
		case OP_HEAL: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			if (scene->_state != ST_INGAME) { break; }
			if (levels[scene->cur_mission].cutscene) {
				TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 1s, EV_HEAL, static_cast<short>(key) };
				timer_queue.push(ev);
				break;
			}

			if (scene->heal_player != -1) {
				if (scene->m_pSpaceship->GetHeal()) {
					for (short pl_id : scene->_plist) {
						if (pl_id == -1) continue;
						if (clients[pl_id]._state != ST_INGAME) continue;
						clients[pl_id].send_bullet_hit_packet(-1, scene->m_pSpaceship->GetHP());
					}
				}

				TIMER_EVENT ev{ 0, chrono::system_clock::now() + 1s, EV_HEAL, static_cast<short>(key) };
				timer_queue.push(ev);
			}
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
			if (scene->_state != ST_INGAME) { break; }
			scene->MissionClear();
			delete ex_over;
			break;
		}
		case OP_SEND_SCENE_INFO: {	// 우주선 좌표, 적 좌표, 미사일 좌표 한번에 send 
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			if (scene->_state != ST_INGAME) { break; }
			if (levels[scene->cur_mission].cutscene) {
				TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 500ms, EV_SEND_SCENE_INFO, static_cast<short>(key) };
				timer_queue.push(ev);
				break;
			}
			
			char send_buf[10000];
			short send_num = 0;

			SC_ALL_METEOR_PACKET m_packet;
			m_packet.type = SC_ALL_METEOR;
			m_packet.size = sizeof(SC_ALL_METEOR_PACKET);
			for (char i = 0; i < METEOS; ++i) {
				m_packet.pos[i] = scene->m_ppMeteoObjects[i]->GetPosition();
			}
			memcpy(&send_buf[0], &m_packet, m_packet.size);
			send_num += m_packet.size;

			SC_MOVE_SPACESHIP_PACKET s_packet;
			s_packet.size = sizeof(s_packet);
			s_packet.type = SC_MOVE_SPACESHIP;
			s_packet.move_time = scene->m_pSpaceship->move_time;
			s_packet.data.pos = scene->m_pSpaceship->GetPosition();
			s_packet.data.Quaternion = scene->m_pSpaceship->input_info.Quaternion;
			memcpy(&send_buf[send_num], &s_packet, s_packet.size);
			send_num += s_packet.size;

			for (char i = 0; i < ENEMIES; ++i) {
				if (!scene->m_ppEnemies[i]->hp < 0) { continue; }
				SC_MOVE_ENEMY_PACKET e_packet;
				e_packet.size = sizeof(e_packet);
				e_packet.type = SC_MOVE_ENEMY;
				e_packet.data.id = i;
				e_packet.data.pos = scene->m_ppEnemies[i]->GetPosition();
				e_packet.data.Quaternion = scene->m_ppEnemies[i]->GetQuaternion();
				memcpy(&send_buf[send_num], &e_packet, e_packet.size);
				send_num += e_packet.size;
			}

			for (char i = 0; i < MISSILES; ++i) {
				if (!scene->m_ppMissiles[i]->GetisActive()) { continue; }
				SC_MISSILE_PACKET e_packet;
				e_packet.size = sizeof(e_packet);
				e_packet.type = SC_MISSILE;
				e_packet.data.id = i;
				e_packet.data.pos = scene->m_ppMissiles[i]->GetPosition();
				e_packet.data.Quaternion = scene->m_ppMissiles[i]->GetQuaternion();
				memcpy(&send_buf[send_num], &e_packet, e_packet.size);
				send_num += e_packet.size;
			}

			for (short pl_id : scene->_plist) {
				if (pl_id == -1) continue;
				if (clients[pl_id]._state != ST_INGAME) continue;
				clients[pl_id].do_send(&send_buf, send_num);
			}


			TIMER_EVENT ev{ 0, chrono::system_clock::now() + 20ms, EV_SEND_SCENE_INFO, static_cast<short>(key) };
			timer_queue.push(ev);
			delete ex_over;
			break;
		}
		case OP_BLACK_HOLE: {
			CScene* scene = scene_manager.GetScene(static_cast<short>(key));
			if (scene->_state != ST_INGAME) { break; }
			if (scene->cur_mission != MissionType::ESCAPE_BLACK_HOLE) { break; }

			auto time_now = chrono::steady_clock::now();
			std::chrono::duration<float> elapsed_time = (time_now - scene->b_prev_time);
			scene->b_prev_time = time_now;

			XMFLOAT3 pos;
			XMFLOAT3 ToBlackHole;
			float dist;
			float speed;
			pos = scene->m_pSpaceship->GetPosition();
			ToBlackHole = Vector3::Subtract(scene->black_hole_pos, pos);
			dist = Vector3::Length(ToBlackHole);
			speed = 90.f - dist * 0.5f;
			if (speed < 50.f) { speed = 50.f; }
			ToBlackHole = Vector3::ScalarProduct(ToBlackHole, speed * elapsed_time.count());
			scene->m_pSpaceship->SetPosition(Vector3::Add(pos, ToBlackHole));
			if (dist < 30.f) {
				scene->m_pSpaceship->GetAttack(1);
				for (short pl_id : scene->_plist) {
					if (pl_id == -1) continue;
					if (clients[pl_id]._state != ST_INGAME) continue;
					clients[pl_id].send_bullet_hit_packet(-1, scene->m_pSpaceship->GetHP());
				}
			}

			for (char i = 0; i < METEOS; ++i) {
				pos = scene->m_ppMeteoObjects[i]->GetPosition();
				dist = Vector3::Length(Vector3::Subtract(pos, scene->black_hole_pos));
				if (dist < 20.f) {
					scene->SpawnMeteo(i);
					continue;
				}
				ToBlackHole = Vector3::Subtract(scene->black_hole_pos, pos);
				dist = Vector3::Length(ToBlackHole);
				speed = 90.f - dist * 0.5f;
				if (speed < 50.f) { speed = 50.f; }
				ToBlackHole = Vector3::ScalarProduct(ToBlackHole, speed * elapsed_time.count());
				scene->m_ppMeteoObjects[i]->SetPosition(Vector3::Add(pos, ToBlackHole));
			}

			for (char i = 0; i < ENEMIES; ++i) {
				if (!scene->m_ppEnemies[i]->GetisAlive()) { continue; }
				pos = scene->m_ppEnemies[i]->GetPosition();
				dist = Vector3::Length(Vector3::Subtract(pos, scene->black_hole_pos));
				if (dist < 50.f) {
					scene->m_ppEnemies[i]->hp -= 3;
					for (short pl_id : scene->_plist) {
						if (pl_id == -1) continue;
						if (clients[pl_id]._state != ST_INGAME) continue;
						clients[pl_id].send_bullet_hit_packet(i, scene->m_ppEnemies[i]->hp);
					}
					continue;
				}
				ToBlackHole = Vector3::Subtract(scene->black_hole_pos, pos);
				dist = Vector3::Length(ToBlackHole);
				speed = 90.f - dist * 0.5f;
				if (speed < 50.f) { speed = 50.f; }
				ToBlackHole = Vector3::ScalarProduct(ToBlackHole, speed * elapsed_time.count());
				scene->m_ppEnemies[i]->SetPosition(Vector3::Add(pos, ToBlackHole));
			}

			TIMER_EVENT ev{ ex_over->obj_id, chrono::system_clock::now() + 10ms, EV_BLACK_HOLE, static_cast<short>(key) };
			timer_queue.push(ev);
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
		if (p->player_type == PlayerType::INSIDE)
		{
			clients[c_id].type = PlayerType::INSIDE;

			SC_LOGIN_INFO_PACKET packet{};
			packet.data.id = c_id;
			packet.data.player_type = p->player_type;
			packet.size = sizeof(SC_LOGIN_INFO_PACKET);
			packet.type = SC_CHANGE;

			scene_manager.Send(clients[c_id].room_id, (char*)&packet);	// 수정	

			CScene* scene = scene_manager.GetScene(clients[c_id].room_id);

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
			bool exists = scene_manager.GetCanSit(clients[c_id].room_id, p->player_type);

			if (exists) {
				clients[c_id].type = p->player_type;

				// 미션
				if (scene_manager.GetScene(clients[c_id].room_id)->cur_mission == MissionType::TU_SIT && p->player_type == PlayerType::MOVE)
				{
					scene_manager.GetScene(clients[c_id].room_id)->MissionClear();
				}


				SC_LOGIN_INFO_PACKET packet{};
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
			scene_manager.GetScene(clients[c_id].room_id)->m_pSpaceship->SetInputInfo(p->data, p->move_time);
		}
		break;
	}
	case CS_INSIDE_MOVE: {
		CS_INSIDE_PACKET* p = reinterpret_cast<CS_INSIDE_PACKET*>(packet);
		if (clients[c_id].type == PlayerType::INSIDE) {
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
		CS_HEAL_PACKET* p = reinterpret_cast<CS_HEAL_PACKET*>(packet);
		CScene* m_pScene = scene_manager.GetScene(clients[c_id].room_id);

		if (p->start && m_pScene->heal_player == -1) {
			m_pScene->heal_start = std::chrono::system_clock::now();
			m_pScene->heal_player = c_id;
			clients[c_id].send_heal_packet();
			TIMER_EVENT ev{ 0, chrono::system_clock::now() + 1s, EV_HEAL, clients[c_id].room_id};
			timer_queue.push(ev);
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
