#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <winsock.h>
#include <Windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <chrono>
#include <queue>
#include <array>
#include <memory>
#include <random>

using namespace std;
using namespace chrono;

extern HWND		hWnd;

static std::random_device rdd;
static std::default_random_engine dree(rdd());
static std::uniform_real_distribution<float> urdRandom(-1000, 1000);

const static int MAX_TEST = 3000;
const static int MAX_CLIENTS = MAX_TEST * 2;
const static int INVALID_ID = -1;
const static int MAX_PACKET_SIZE = 255;
const static int MAX_BUFF_SIZE = 255;

#pragma comment (lib, "ws2_32.lib")

#include "..\..\Server\Server\protocol.h"

HANDLE g_hiocp;

enum OPTYPE { OP_SEND, OP_RECV, OP_DO_MOVE };

high_resolution_clock::time_point last_connect_time;

struct OverlappedEx {
	WSAOVERLAPPED over;
	WSABUF wsabuf;
	unsigned char IOCP_buf[MAX_BUFF_SIZE];
	OPTYPE event_type;
	int event_target;
};

struct CLIENT {
	int id;
	XMFLOAT3 pos;
	atomic_bool connected;

	SOCKET client_socket;
	OverlappedEx recv_over;
	unsigned char packet_buf[MAX_PACKET_SIZE];
	int prev_packet_data;
	int curr_packet_size;
	high_resolution_clock::time_point last_move_time;
};

array<int, MAX_CLIENTS> client_map;
array<CLIENT, MAX_CLIENTS> g_clients;
atomic_int num_connections;
atomic_int client_to_close;
atomic_int active_clients;

int			global_delay;				// ms단위, 1000이 넘으면 클라이언트 증가 종료

vector <thread*> worker_threads;
thread test_thread;

float point_cloud[MAX_TEST * 2];

void error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L"에러" << lpMsgBuf << std::endl;

	MessageBox(hWnd, lpMsgBuf, L"ERROR", 0);
	LocalFree(lpMsgBuf);
	// while (true);
}

void DisconnectClient(int ci)
{
	bool status = true;
	if (true == atomic_compare_exchange_strong(&g_clients[ci].connected, &status, false)) {
		closesocket(g_clients[ci].client_socket);
		active_clients--;
	}
	// cout << "Client [" << ci << "] Disconnected!\n";
}

void SendPacket(int cl, void* packet)
{
	int psize = reinterpret_cast<unsigned char*>(packet)[0];
	int ptype = reinterpret_cast<unsigned char*>(packet)[1];
	OverlappedEx* over = new OverlappedEx;
	over->event_type = OP_SEND;
	memcpy(over->IOCP_buf, packet, psize);
	ZeroMemory(&over->over, sizeof(over->over));
	over->wsabuf.buf = reinterpret_cast<CHAR*>(over->IOCP_buf);
	over->wsabuf.len = psize;
	int ret = WSASend(g_clients[cl].client_socket, &over->wsabuf, 1, NULL, 0,
		&over->over, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			error_display("Error in SendPacket:", err_no);
	}
	// std::cout << "Send Packet [" << ptype << "] To Client : " << cl << std::endl;
}

void ProcessPacket(int ci, unsigned char packet[])
{
	switch (packet[1]) {

	case SC_LOGIN_INFO:
	{
		//로그인 처리 : 아이디 0이면 move로 change 패킷 전송
		//	1, 2면 일단 attack1, 2로 change 패킷 전송
		SC_LOGIN_INFO_PACKET* login_packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(packet);
		active_clients++;
		g_clients[ci].connected = true;
		g_clients[ci].id = login_packet->data.id;
		
		if (login_packet->data.id == 2) {
			CS_NEXT_MISSION_PACKET start_packet;
			start_packet.size = sizeof(start_packet);
			start_packet.type = CS_START;
			SendPacket(ci, &start_packet);
		}
	}
	case SC_CHANGE: break;
	case SC_ADD_PLAYER: break;
	case SC_REMOVE_PLAYER: break;
	case SC_SPAWN_METEO: break;
	case SC_ALL_METEOR: break;
	case SC_METEO: break;
	case SC_MOVE_SPACESHIP:
	{
		//위치 set + time_delay 측정
		SC_MOVE_SPACESHIP_PACKET* move_packet = reinterpret_cast<SC_MOVE_SPACESHIP_PACKET*>(packet);
		g_clients[ci].pos = move_packet->data.pos;

		if (0 != move_packet->move_time) {
			auto d_ms = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - move_packet->move_time;

			if (global_delay < d_ms) global_delay++;
			else if (global_delay > d_ms) global_delay--;
		}
	}
	case SC_MOVE_INSIDEPLAYER: break;
	case SC_SPAWN_ENEMY: break;
	case SC_MOVE_ENEMY: break;
	case SC_BULLET:
	{
		//time_delay 측정
		SC_BULLET_PACKET* bullet_packet = reinterpret_cast<SC_BULLET_PACKET*>(packet);
		
		if (0 != bullet_packet->attack_time) {
			auto d_ms = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - bullet_packet->attack_time;

			if (global_delay < d_ms) global_delay++;
			else if (global_delay > d_ms) global_delay--;
		}
	}
	case SC_BULLET_HIT: break;
	case SC_MISSILE: break;
	case SC_REMOVE_MISSILE: break;
	case SC_ITEM: break;
	case SC_ANIMATION_CHANGE: break;
	case SC_HEAL: break;
	case SC_MISSION_START: break;
	case SC_KILL_NUM: break;
	case SC_START: {
		CS_NEXT_MISSION_PACKET cutscene_packet;

		cutscene_packet.size = sizeof(cutscene_packet);
		cutscene_packet.type = CS_CUTSCENE_END;
		SendPacket(ci, &cutscene_packet);

		CS_CHANGE_PACKET change_packet;
		change_packet.size = sizeof(change_packet);
		change_packet.type = CS_CHANGE;
		if (g_clients[ci].id == 0) {
			change_packet.player_type = PlayerType::MOVE;
		}
		else if (g_clients[ci].id == 1) {
			change_packet.player_type = PlayerType::ATTACK1;
		}
		else {
			change_packet.player_type = PlayerType::ATTACK2;
		}
		SendPacket(ci, &change_packet);
		break; }
	case SC_BLACK_HOLE: break;
	case SC_BLACK_HOLE_TIME: break;
	case SC_CUTSCENE_END_NUM: break;
	default: MessageBox(hWnd, L"Unknown Packet Type", L"ERROR", 0);
		while (true);
	}
}

void Worker_Thread()
{
	while (true) {
		DWORD io_size;
		unsigned long long ci;
		OverlappedEx* over;
		BOOL ret = GetQueuedCompletionStatus(g_hiocp, &io_size, &ci,
			reinterpret_cast<LPWSAOVERLAPPED*>(&over), INFINITE);
		// std::cout << "GQCS :";
		int client_id = static_cast<int>(ci);
		if (FALSE == ret) {
			int err_no = WSAGetLastError();
			if (64 == err_no) DisconnectClient(client_id);
			else {
				// error_display("GQCS : ", WSAGetLastError());
				DisconnectClient(client_id);
			}
			if (OP_SEND == over->event_type) delete over;
		}
		if (0 == io_size) {
			DisconnectClient(client_id);
			continue;
		}
		if (OP_RECV == over->event_type) {
			//std::cout << "RECV from Client :" << ci;
			//std::cout << "  IO_SIZE : " << io_size << std::endl;
			unsigned char* buf = g_clients[ci].recv_over.IOCP_buf;
			unsigned psize = g_clients[ci].curr_packet_size;
			unsigned pr_size = g_clients[ci].prev_packet_data;
			while (io_size > 0) {
				if (0 == psize) psize = buf[0];
				if (io_size + pr_size >= psize) {
					// 지금 패킷 완성 가능
					unsigned char packet[MAX_PACKET_SIZE];
					memcpy(packet, g_clients[ci].packet_buf, pr_size);
					memcpy(packet + pr_size, buf, psize - pr_size);
					ProcessPacket(static_cast<int>(ci), packet);
					io_size -= psize - pr_size;
					buf += psize - pr_size;
					psize = 0; pr_size = 0;
				}
				else {
					memcpy(g_clients[ci].packet_buf + pr_size, buf, io_size);
					pr_size += io_size;
					io_size = 0;
				}
			}
			g_clients[ci].curr_packet_size = psize;
			g_clients[ci].prev_packet_data = pr_size;
			DWORD recv_flag = 0;
			int ret = WSARecv(g_clients[ci].client_socket,
				&g_clients[ci].recv_over.wsabuf, 1,
				NULL, &recv_flag, &g_clients[ci].recv_over.over, NULL);
			if (SOCKET_ERROR == ret) {
				int err_no = WSAGetLastError();
				if (err_no != WSA_IO_PENDING)
				{
					//error_display("RECV ERROR", err_no);
					DisconnectClient(client_id);
				}
			}
		}
		else if (OP_SEND == over->event_type) {
			if (io_size != over->wsabuf.len) {
				// std::cout << "Send Incomplete Error!\n";
				DisconnectClient(client_id);
			}
			delete over;
		}
		else if (OP_DO_MOVE == over->event_type) {
			// Not Implemented Yet
			delete over;
		}
		else {
			std::cout << "Unknown GQCS event!\n";
			while (true);
		}
	}
}

constexpr int DELAY_LIMIT = 100;
constexpr int DELAY_LIMIT2 = 150;
constexpr int ACCEPT_DELY = 50;

void Adjust_Number_Of_Client()
{
	static int delay_multiplier = 1;
	static int max_limit = MAXINT;
	static bool increasing = true;

	if (active_clients >= MAX_TEST) return;
	if (num_connections >= MAX_CLIENTS) return;

	auto duration = high_resolution_clock::now() - last_connect_time;
	if (ACCEPT_DELY * delay_multiplier > duration_cast<milliseconds>(duration).count()) return;

	int t_delay = global_delay;
	if (DELAY_LIMIT2 < t_delay) {
		if (true == increasing) {
			max_limit = active_clients;
			increasing = false;
		}
		if (100 > active_clients) return;
		if (ACCEPT_DELY * 10 > duration_cast<milliseconds>(duration).count()) return;
		last_connect_time = high_resolution_clock::now();
		DisconnectClient(client_to_close);
		client_to_close++;
		return;
	}
	else
		if (DELAY_LIMIT < t_delay) {
			delay_multiplier = 10;
			return;
		}
	if (max_limit - (max_limit / 20) < active_clients) return;

	increasing = true;
	last_connect_time = high_resolution_clock::now();
	g_clients[num_connections].client_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(PORT_NUM);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");


	int Result = WSAConnect(g_clients[num_connections].client_socket, (sockaddr*)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);
	if (0 != Result) {
		error_display("WSAConnect : ", GetLastError());
	}

	g_clients[num_connections].curr_packet_size = 0;
	g_clients[num_connections].prev_packet_data = 0;
	ZeroMemory(&g_clients[num_connections].recv_over, sizeof(g_clients[num_connections].recv_over));
	g_clients[num_connections].recv_over.event_type = OP_RECV;
	g_clients[num_connections].recv_over.wsabuf.buf =
		reinterpret_cast<CHAR*>(g_clients[num_connections].recv_over.IOCP_buf);
	g_clients[num_connections].recv_over.wsabuf.len = sizeof(g_clients[num_connections].recv_over.IOCP_buf);

	DWORD recv_flag = 0;
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_clients[num_connections].client_socket), g_hiocp, num_connections, 0);

	CS_LOGIN_PACKET l_packet;

	int temp = num_connections;
	//sprintf_s(l_packet.name, "%d", temp);
	l_packet.size = sizeof(l_packet);
	l_packet.type = CS_LOGIN;
	l_packet.room_id = -1;
	SendPacket(num_connections, &l_packet);


	int ret = WSARecv(g_clients[num_connections].client_socket, &g_clients[num_connections].recv_over.wsabuf, 1,
		NULL, &recv_flag, &g_clients[num_connections].recv_over.over, NULL);
	if (SOCKET_ERROR == ret) {
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING)
		{
			error_display("RECV ERROR", err_no);
			goto fail_to_connect;
		}
	}
	num_connections++;
fail_to_connect:
	return;
}

void Test_Thread()
{
	while (true) {
		//Sleep(max(20, global_delay));
		Adjust_Number_Of_Client();

		for (int i = 0; i < num_connections; ++i) {
			if (false == g_clients[i].connected) continue;
			if (g_clients[i].id != 0 && g_clients[i].last_move_time + 1s > high_resolution_clock::now()) continue;
			if (g_clients[i].id == 0 && g_clients[i].last_move_time + 33ms > high_resolution_clock::now()) continue;
			g_clients[i].last_move_time = high_resolution_clock::now();
			if (g_clients[i].id == 0) {
				CS_SPACESHIP_PACKET my_packet;
				my_packet.size = sizeof(my_packet);
				my_packet.type = CS_SPACESHIP_MOVE;
				my_packet.data.dwDirection = 0;
				my_packet.data.Quaternion = { 1.0, 0.0, 0.0, 0.0 };
				my_packet.move_time = static_cast<unsigned>(duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count());
				SendPacket(i, &my_packet);
			}
			else {
				CS_ATTACK_PACKET my_packet;
				my_packet.size = sizeof(my_packet);
				my_packet.type = CS_ATTACK;
				my_packet.data.pos = { urdRandom(dree),  urdRandom(dree),  urdRandom(dree) };
				my_packet.data.direction = { 0.f,  1.f,  0.f };
				my_packet.attack_time = static_cast<unsigned>(duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count());
				SendPacket(i, &my_packet);
			}
		}
	}
}

void InitializeNetwork()
{
	for (auto& cl : g_clients) {
		cl.connected = false;
		cl.id = INVALID_ID;
	}

	for (auto& cl : client_map) cl = -1;
	num_connections = 0;
	last_connect_time = high_resolution_clock::now();

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);

	for (int i = 0; i < 6; ++i)
		worker_threads.push_back(new std::thread{ Worker_Thread });

	test_thread = thread{ Test_Thread };
}

void ShutdownNetwork()
{
	test_thread.join();
	for (auto pth : worker_threads) {
		pth->join();
		delete pth;
	}
}

void Do_Network()
{
	return;
}

void GetPointCloud(int* size, float** points)
{
	int index = 0;
	for (int i = 0; i < num_connections; ++i)
		if (true == g_clients[i].connected) {
			point_cloud[index * 2] = static_cast<float>(g_clients[i].pos.x);
			point_cloud[index * 2 + 1] = static_cast<float>(g_clients[i].pos.z);
			index++;
		}

	*size = index;
	*points = point_cloud;
}

