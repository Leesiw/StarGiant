#include "Framework.h"

CGameFramework::CGameFramework()
{
	m_pScene = NULL;
	m_pPlayer = NULL;
}

CGameFramework::~CGameFramework()
{
	
}

void CGameFramework::Init() {

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

	clients[0].type = MOVE;
	clients[1].type = ATTACK;
	
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
			
				clients[client_id].send_meteo_packet(client_id, m_pScene->m_ppGameObjects);
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


void CGameFramework::BuildObjects()
{
	m_pScene = new CScene();

	if (m_pScene) m_pScene->BuildObjects();


	CAirplanePlayer* pAirplanePlayer = new CAirplanePlayer();
	pAirplanePlayer->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	pAirplanePlayer->mesh = true;
	pAirplanePlayer->boundingbox = BoundingOrientedBox{ XMFLOAT3(-0.000000f, -0.000000f, -0.000096f), XMFLOAT3(15.5f, 15.5f, 3.90426f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pScene->m_pPlayer = m_pPlayer = pAirplanePlayer;
	//m_pCamera = m_pPlayer->GetCamera();

	////////////////////////////////////////

	m_TwiceScene = new CScene();
	if (m_TwiceScene) m_TwiceScene->BuildObjects2();
	CAirplanePlayer* pTwicePlayer = new CAirplanePlayer();
	pTwicePlayer->SetPosition(XMFLOAT3(0.0f, 10.0f, 0.0f));
	m_TwiceScene->m_pPlayer = m_TwicePlayer = pTwicePlayer;
	//m_TwiceCamera = m_TwicePlayer->GetCamera();

	////////////////////////////////////////
}

void CGameFramework::ReleaseObjects()
{
	if (m_pPlayer) m_pPlayer->Release();
	if (m_TwicePlayer) m_pPlayer->Release();

	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;

	if (m_TwiceScene) m_pScene->ReleaseObjects();
	if (m_TwiceScene) delete m_TwiceScene;
}

void CGameFramework::AnimateObjects(float fTimeElapsed)
{
	if (m_pScene) m_pScene->AnimateObjects(fTimeElapsed);

	if (m_TwiceScene) m_TwiceScene->AnimateObjects(fTimeElapsed);

	m_pPlayer->Animate(fTimeElapsed, NULL);
	m_pPlayer->Update(fTimeElapsed);
	//
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
		swap(clients[0].type, clients[1].type);
		for (auto& pl : clients) {
			if (false == pl.in_use) continue;
			pl.send_login_info_packet();
		}
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		if (clients[c_id].type == MOVE) {
			m_pPlayer->SetdwDirection(p->dwDirection);
			m_pPlayer->SetcxcyDelta(p->cxDelta, p->cyDelta, p->isRButton);
		}
		break;
	}
	case CS_ATTACK: {
		CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
		if (clients[c_id].type == ATTACK) {
			((CAirplanePlayer*)m_pPlayer)->FireBullet(m_pLockedObject);
			for (auto& pl : clients) {
				if (false == pl.in_use) continue;
				pl.send_bullet_packet(0,m_pPlayer);
			}
			m_pLockedObject = NULL;
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
		fps += EndTime - StartTime;
		//cout << fps.count() << endl;
		StartTime = chrono::system_clock::now();

		if (fps.count() > 0.01f) {
			AnimateObjects(fps.count());
			for (auto& pl : clients) {
				if (false == pl.in_use) continue;
				pl.send_move_packet(0, m_pPlayer);
			}
			fps = EndTime - EndTime;
		}
		EndTime = chrono::system_clock::now();
	}
}

//#define _WITH_PLAYER_TOP

void CGameFramework::FrameAdvance()
{

	//AnimateObjects();

	//m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	//size_t nLength = _tcslen(m_pszFrameRate);
	//XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
	//_stprintf_s(m_pszFrameRate + nLength, 70 - nLength, _T("(%4f, %4f, %4f)"), xmf3Position.x, xmf3Position.y, xmf3Position.z);
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
	for (auto& pl : clients) {
		if (pl.in_use == false) continue;
		if (pl._id == c_id) continue;
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_PLAYER;
		pl.do_send(&p);
	}
	closesocket(clients[c_id]._socket);
	clients[c_id].in_use = false;
}



//------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//---------------------------------------------------------------------

void SESSION::send_move_packet(int c_id, CPlayer* m_pPlayer)
{
	SC_MOVE_PLAYER_PACKET p;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.data.pos = m_pPlayer->GetPosition();
	p.data.m_fPitch = m_pPlayer->GetPitch();
	p.data.m_fRoll = m_pPlayer->GetRoll();
	p.data.m_fYaw = m_pPlayer->GetYaw();
	p.data.velocity = m_pPlayer->GetVelocity();
	p.data.shift = m_pPlayer->GetShift();
	do_send(&p);
}

void SESSION::send_bullet_packet(int c_id, CPlayer* m_pPlayer)
{
	
	SC_BULLET_PACKET p;

	p.size = sizeof(SC_BULLET_PACKET);
	p.type = SC_BULLET;
	
	p.data.direction = m_pPlayer->GetLookVector();
	p.data.pos = m_pPlayer->GetPosition();
	p.data.pitch = m_pPlayer->GetPitch();
	p.data.yaw = m_pPlayer->GetYaw();
	p.data.roll = m_pPlayer->GetRoll();

	char buf[sizeof(SC_BULLET_PACKET)];
	memcpy(buf, reinterpret_cast<char*>(&p), sizeof(p));
	WSABUF wsabuf{ sizeof(buf), buf };
	DWORD sent_byte;

	WSASend(_socket, &wsabuf, 1, &sent_byte, 0, nullptr, 0);

}

void SESSION::send_meteo_packet(int c_id, CGameObject* meteo[])
{
	SC_METEO_PACKET p;
	p.size = sizeof(SC_METEO_PACKET);
	p.type = SC_METEO;

	for (int i = 0; i < METEOS; ++i) {
		p.meteo[i].m_xmf4x4Transform = meteo[i]->m_xmf4x4Transform;
		p.meteo[i].m_fRotationSpeed = meteo[i]->m_fRotationSpeed;
	}
	char buf[sizeof(SC_METEO_PACKET)];
	memcpy(buf, reinterpret_cast<char*>(&p), sizeof(p));
	WSABUF wsabuf{ sizeof(buf), buf };
	DWORD sent_byte;

	WSASend(_socket, &wsabuf, 1, &sent_byte, 0, nullptr, 0);

}
