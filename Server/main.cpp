#include "main.h"

int main(int argc, char* argv[])
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// 리시브 타임아웃
	DWORD optval = 10;
	retval = setsockopt(listen_sock, SOL_SOCKET, SO_RCVTIMEO,
		(const char*)&optval, sizeof(optval));

	// nagle off
	/*DWORD optval2 = 1;
	retval = setsockopt(listen_sock, IPPROTO_TCP, TCP_NODELAY,
		(const char*)&optval2, sizeof(optval2));*/

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVER_PORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	HANDLE hThread;

	for (int i = 0; i < MAX_CLIENT_ROOM; ++i) {
		send_players.players[i].id = -1;
		std::cout << (int)send_players.players[i].id << " 넣어짐" << std::endl;
	}


	while (1) {
		addrlen = sizeof(clientaddr);

		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {			
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));

		hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)client_sock, 0, NULL);

		if (hThread == NULL) { closesocket(client_sock); }
		else { CloseHandle(hThread); }

	}

	closesocket(listen_sock);
}

DWORD WINAPI ProcessClient(LPVOID arg)
{
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	// 이후 아이디는 데이터베이스 연동후에 고유 아이디를 배정하고 사용할 예정
	int id = global_id++;
	int room_id = -1;
	S_STATE state = S_STATE::IN_GAME;
	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	players.try_emplace(id, id, client_sock);
	for (int i = 0; i < MAX_CLIENT_ROOM; ++i) {
		if (send_players.players[i].id == -1) {
			room_id = i;
			players[id].SetRoomID(i);
			send_players.players[i] = players[id].GetData();
			break;
		}
	}

	players[id].RecvLogin();

	PLAYER_DATA ply = players[id].GetData();
	
	send(client_sock, (char*)&ply, sizeof(ply), 0);
	//players[id].SendPlayerData(&send_players, sizeof(send_players));

	constexpr int MAX_FAME = 60;
	using frame = std::chrono::duration<int32_t, std::ratio<1, MAX_FAME>>;
	using ms = std::chrono::duration<float, std::milli>;
	std::chrono::time_point<std::chrono::steady_clock> fps_timer{ std::chrono::steady_clock::now() };
	
	frame fps{}, frame_count{};
	while (1) {
		if (state == S_STATE::SHOP) { 
			// item정보는 처음 login을 했을때 플레이어 데이터 보내줄때 보내줄거임
			// 지금은 인게임 테스트가 먼저다 보니 인게임 데이터를 바로 보냄
			int retval = players[id].RecvItemData();
			if (retval == SOCKET_ERROR)
				if (WSAGetLastError() == WSAETIMEDOUT)
					continue;
			players[id].SendItemData();
			
		}
		if (state == S_STATE::IN_GAME) {
			players[id].RecvPlayerData();
			send_players.players[room_id] = players[id].GetData();
			fps = std::chrono::duration_cast<frame>(std::chrono::steady_clock::now() - fps_timer);
			if (fps.count() < 1) continue;
			int retval = players[id].SendPlayerData(&send_players, sizeof(send_players));
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
			fps_timer = std::chrono::steady_clock::now();
		}
	}

	// 클라이언트 접속 종료시 자동차 정보 초기화
	players[id].SetID(-1);
	send_players.players[room_id].id = -1;

	// 소켓 닫기
	closesocket(client_sock);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));
	return 0;
}