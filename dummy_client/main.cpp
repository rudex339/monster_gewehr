// tcp/ip용 헤더들
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <winsock2.h> // 윈속2 메인 헤더
#include <ws2tcpip.h> // 윈속2 확장 헤더
#include <windows.h>

#include <iostream>
#include <vector>
#include <array>
#include <atomic>
#include <string>
#include <chrono>
#include <thread>
#include <random>

#pragma comment (lib, "WS2_32.LIB")
#pragma comment (lib, "MSWSock.LIB")

#include "..\Server\protocol.h"

using namespace std;
using namespace chrono;

constexpr int MAX_CLIENTS = 500;
string SERVER_IP{ "127.0.0.1" };

HANDLE iocp_handle;
high_resolution_clock::time_point connect_time;

enum COMP_TYPE { OP_RECV, OP_SEND };

// 랜덤한 문자열 생성 함수
string create_random_string(size_t length) {
	const string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	string result;
	result.reserve(length);

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> uid(0, characters.size() - 1);

	for (size_t i = 0; i < length; ++i) {
		result += characters[uid(gen)];
	}
	return result;
}

class EXP_OVER {
public:
	WSAOVERLAPPED _wsa_over;
	WSABUF _wsa_buf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;

	EXP_OVER()
	{
		_wsa_buf.len = BUF_SIZE;
		_wsa_buf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
	}

	EXP_OVER(char* packet)
	{
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
		_wsa_buf.len = packet[0];
		_wsa_buf.buf = _send_buf;
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}
	~EXP_OVER() = default;
};

struct Client {
    SOCKET m_socket;
	EXP_OVER m_recv_over;
	INT m_remain_size = 0;
    atomic_bool connected = false;

	void DoRecv() {
		DWORD flag = 0;
		ZeroMemory(&m_recv_over._wsa_over, sizeof(m_recv_over._wsa_over));
		m_recv_over._wsa_buf.len = BUF_SIZE - m_remain_size;
		m_recv_over._wsa_buf.buf = m_recv_over._send_buf + m_remain_size;
		int retval = WSARecv(m_socket, &m_recv_over._wsa_buf, 1, NULL, &flag, &m_recv_over._wsa_over, NULL);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				std::cout << "WSARecv() failed with error " << WSAGetLastError() << std::endl;
			}
		}
	}

	void DoSend(void* p) {
		EXP_OVER* send_over = new EXP_OVER(reinterpret_cast<char*>(p));
		int retval = WSASend(m_socket, &send_over->_wsa_buf, 1, 0, 0, &send_over->_wsa_over, nullptr);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				std::cout << "WSASend() failed with error " << WSAGetLastError() << std::endl;
			}
		}
	}
};

void ProcessPacket(int id, char* p);
void DisconnectClient(int id);
void ConnectClient();

array<Client, MAX_CLIENTS> clients;
atomic_int connected_clients = 0;
atomic_int login_clients = 0;
vector <thread> worker_threads;


void WorkerThread() {
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL retval = GetQueuedCompletionStatus(iocp_handle, &num_bytes, &key, &over, INFINITE);
		EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(over);

		if (retval == FALSE) {
			if (exp_over->_comp_type == OP_SEND) {
				cout << "Send Error" << endl;
				delete exp_over;
			}
			else if (exp_over->_comp_type == OP_RECV) {
				cout << "Recv Error" << endl;
				closesocket(clients[key].m_socket);
			}
			cout << "GQCS Error on client[" << static_cast<int>(key) << "]" << endl;
			continue;
		}
		// Handle the received data here
		switch (exp_over->_comp_type) {
		case OP_RECV: {
			int remain_size = num_bytes + clients[key].m_remain_size;
			char* p = exp_over->_send_buf;
			while (remain_size > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_size) {
					ProcessPacket(key, p);
					p = p + packet_size;
					remain_size -= packet_size;
				}
				else break;
			}
			if (remain_size > 0) {
				clients[key].m_remain_size = remain_size;
				memcpy(&exp_over->_send_buf, p, remain_size);
			}
			clients[key].DoRecv();
			break;
		}
		case OP_SEND: {
			delete exp_over;
			break;
		}
		}
	}
}

void ProcessPacket(int id, char* p)
{
	switch (p[1]) {
	case SC_PACKET_LOGIN_INFO: {
		login_clients += 1;
		cout << "로그인 성공 : " << id << endl;
		break;
	}
	}
}

void DisconnectClient(int id) 
{
	bool status = true;
	if (true == atomic_compare_exchange_strong(&clients[id].connected, &status, false)) {	// CAS를 사용해서 lock 없이 접속여부 변경
		closesocket(clients[id].m_socket);		
	}
}

constexpr int ACCEPT_DELY = 50;		// 50ms마다 접속 시도

void ConnectClient()
{
	static int delay = 1;			// 연결 간격

	if (login_clients >= MAX_CLIENTS) return;
	if (connected_clients >= MAX_CLIENTS) return;	// 테스트 최대 접속이면 연결 안함

	auto duration = high_resolution_clock::now() - connect_time;
	if (ACCEPT_DELY * delay > duration_cast<milliseconds>(duration).count()) return;	// 너무 짧은 주기로 접속 시도

	connect_time = high_resolution_clock::now();
	clients[connected_clients].m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP.c_str(), &serveraddr.sin_addr);

	int ret = WSAConnect(clients[connected_clients].m_socket, (sockaddr*)&serveraddr, sizeof(serveraddr), NULL, NULL, NULL, NULL);

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(clients[connected_clients].m_socket), iocp_handle, connected_clients, 0);

	CS_ACCOUNT_PACKET packet;
	packet.size = sizeof(CS_ACCOUNT_PACKET);
	packet.type = CS_PACKET_LOGIN;
	string random_id = create_random_string(15);
	strcpy_s(packet.name, random_id.c_str());
	string random_password = create_random_string(15);
	strcpy_s(packet.password, random_password.c_str());

	clients[connected_clients].DoSend(&packet);

	clients[connected_clients].DoRecv();

	connected_clients += 1;
}



int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

	connect_time = high_resolution_clock::now();
	iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

 //   sockaddr_in server_addr{};
 //   server_addr.sin_family = AF_INET;
 //   server_addr.sin_port = htons(SERVER_PORT);
	//inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    for(int i = 0; i < 4; ++i)
		worker_threads.emplace_back(WorkerThread);

	for (auto& thread : worker_threads)
		thread.detach();

	while (1) {
		ConnectClient();
	}

    WSACleanup();
    return 0;
}