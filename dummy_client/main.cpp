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

#pragma comment (lib, "WS2_32.LIB")
#pragma comment (lib, "MSWSock.LIB")

using namespace std;
using namespace chrono;

constexpr int MAX_CLIENTS = 500;
constexpr const char* SERVER_IP = "127.0.0.1";
constexpr int SERVER_PORT = 8000;
constexpr int BUF_SIZE = 500;

HANDLE iocp_handle;

struct Client {
    SOCKET socket;
	EXP_OVER recv_over;

    atomic_bool connected = false;
};

enum COMP_TYPE { OP_RECV, OP_SEND };

class EXP_OVER {
public:
    WSAOVERLAPPED _wsa_over;
    WSABUF _wsa_buf;
    char _send_buf[BUF_SIZE];
    COMP_TYPE _comp_type;

    EXP_OVER();
    EXP_OVER(char* packet);
    ~EXP_OVER() = default;
};

array<Client, MAX_CLIENTS> clients;
atomic_int connected_clients = 0;
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
				closesocket(clients[key].socket);
			}
			cout << "GQCS Error on client[" << static_cast<int>(key) << "]" << endl;
			continue;
		}
		// Handle the received data here
		switch (exp_over->_comp_type) {
		case OP_RECV: {
			break;
		}
		case OP_SEND: {
			break;
		}
		}
	}
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

	iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

 //   sockaddr_in server_addr{};
 //   server_addr.sin_family = AF_INET;
 //   server_addr.sin_port = htons(SERVER_PORT);
	//inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    for(int i = 0; i < 4; ++i)
		worker_threads.emplace_back(WorkerThread);

	for (auto& thread : worker_threads)
		thread.join();

    WSACleanup();
    return 0;
}