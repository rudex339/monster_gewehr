// tcp/ip용 헤더들
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <winsock2.h> // 윈속2 메인 헤더
#include <ws2tcpip.h> // 윈속2 확장 헤더

#include <windows.h>
#include <iostream>
#include <vector>
#include <atomic>
#include <string>

#pragma comment(lib, "ws2_32.lib")

constexpr int MAX_CLIENTS = 500;
constexpr const char* SERVER_IP = "127.0.0.1";
constexpr int SERVER_PORT = 8000;
constexpr int TIMEOUT_MS = 10000;

struct ClientInfo {
    SOCKET socket;
    WSAEVENT event;
    bool connected = false;
};

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    std::vector<ClientInfo> clients;

    std::cout << "[INFO] 클라이언트 " << MAX_CLIENTS << "개 접속 시도 중...\n";

    // 1. 클라이언트 소켓들 초기화 및 connect 시도
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            std::cerr << "[ERROR] 소켓 생성 실패\n";
            continue;
        }

        unsigned long noblock = 1;
        ioctlsocket(sock, FIONBIO, &noblock);

        connect(sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)); // 비동기 connect

        WSAEVENT evt = WSACreateEvent();
        WSAEventSelect(sock, evt, FD_CONNECT);

        clients.push_back({ sock, evt, false });
    }

    DWORD startTime = GetTickCount();
    int successCount = 0;
    int finishedCount = 0;

    while (finishedCount < clients.size() && (GetTickCount() - startTime) < TIMEOUT_MS) {
        for (size_t i = 0; i < clients.size(); ++i) {
            ClientInfo& client = clients[i];

            if (client.connected) continue;

            DWORD result = WSAWaitForMultipleEvents(1, &client.event, TRUE, 0, FALSE);
            if (result == WSA_WAIT_TIMEOUT || result == WSA_WAIT_FAILED) continue;

            WSANETWORKEVENTS netEvents;
            WSAEnumNetworkEvents(client.socket, client.event, &netEvents);

            if (netEvents.lNetworkEvents & FD_CONNECT) {
                if (netEvents.iErrorCode[FD_CONNECT_BIT] == 0) {
                    client.connected = true;
                    successCount++;
                    std::cout << "[Client " << i << "] 연결 성공\n";
                }
                else {
                    std::cout << "[Client " << i << "] 연결 실패\n";
                    closesocket(client.socket);
                }
                finishedCount++;
                WSACloseEvent(client.event);
            }
        }
        Sleep(10); // 너무 CPU 잡아먹지 않게 잠깐 쉬기
    }

	
    std::cout << "\n=== 연결 결과 ===\n";
    std::cout << "총 클라이언트: " << clients.size() << "\n";
    std::cout << "성공한 연결 수: " << successCount << "\n";
    std::cout << "실패한 연결 수: " << (clients.size() - successCount) << "\n";

    Sleep(10000); // 모든 연결 시도 후 잠깐 대기
    // 정리
    for (auto& client : clients) {
        if (client.connected)
            closesocket(client.socket);
    }

    WSACleanup();
    return 0;
}