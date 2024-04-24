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

		/*hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)client_sock, 0, NULL);*/
		std::thread hThread{ &ProcessClient, client_sock };
		hThread.detach();

		/*if (hThread == NULL) { closesocket(client_sock); }
		else { CloseHandle(hThread); }*/

	}

	closesocket(listen_sock);
}

void ProcessClient(SOCKET sock)
{
	int retval;
	SOCKET client_sock = sock;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	
	int id = global_id++;

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	players.try_emplace(id, id, client_sock);

	constexpr int MAX_FAME = 120;
	using frame = std::chrono::duration<int32_t, std::ratio<1, MAX_FAME>>;
	using ms = std::chrono::duration<float, std::milli>;
	std::chrono::time_point<std::chrono::steady_clock> fps_timer{ std::chrono::steady_clock::now() };
	build_bt(&souleater);

	frame fps{}, frame_count{};
	while (players[id].GetState() != S_STATE::LOG_OUT) {
		fps = std::chrono::duration_cast<frame>(std::chrono::steady_clock::now() - fps_timer);
		if (fps.count() < 1) continue;
		retval = players[id].RecvData();
		if (retval > 0) {
			PacketReassembly(id, retval);			
		}
		else if (retval == -1) {
			break;
		}
		else {
			
		}
		if (players[id].GetState() == S_STATE::IN_GAME) {
			run_bt(&souleater);			
		}
		fps_timer = std::chrono::steady_clock::now();
	}

	// 클라이언트 접속 종료시 자동차 정보 초기화
	players[id].SetID(-1);

	// 소켓 닫기
	closesocket(client_sock);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));
	return;
}

void PacketReassembly(int id, size_t recv_size)
{
	int remain_size = recv_size + players[id].GetRemainSize();
	char* p = players[id].m_recv_buf;
	while (remain_size > 0) {
		int packet_size = p[0];
		if (packet_size <= remain_size) {
			ProcessPacket(id, p);
			p = p + packet_size;
			remain_size -= packet_size;
		}
		else break;
	}
	if (remain_size > 0) {
		players[id].SetRemainSize(remain_size);
		memcpy(players[id].m_recv_buf, p, remain_size);
	}
}

void ProcessPacket(int id, char* p)
{
	switch (p[1]) {
	case CS_PACKET_LOGIN: {
		CS_LOGIN_PACKET* packet = reinterpret_cast<CS_LOGIN_PACKET*>(p);
		players[id].SetName(packet->name);
		players[id].SetPostion(packet->pos);
		players[id].SetVelocity(packet->vel);
		players[id].SetYaw(packet->yaw);
		players[id].SetWepon(packet->weapon);


		SendLoginInfo(id);

		break;
	}
	case CS_PACKET_PLAYER_MOVE: {
		CS_PLAYER_MOVE_PACKET* packet = reinterpret_cast<CS_PLAYER_MOVE_PACKET*>(p);
		players[id].SetPostion(packet->pos);
		players[id].SetYaw(packet->yaw);
		//std::cout << id << "에서 만큼 받아옴 " << packet->pos.x << std::endl;
		SendPlayerMove(id);
		break;
	}
	case CS_PACKET_CHANGE_ANIMATION: {
		CS_CHANGE_ANIMATION_PACKET* packet = reinterpret_cast<CS_CHANGE_ANIMATION_PACKET*>(p);
		players[id].SetAnimaition(packet->animation);

		SendAnimaition(id);
		break;
	}
	}
}

void SendLoginInfo(int id)
{
	int retval;
	SC_LOGIN_INFO_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_LOGIN_INFO;
	packet.id = id;
	std::cout << packet.id << std::endl;
	retval = players[id].DoSend(&packet, packet.size);
	if (retval == SOCKET_ERROR) {
		players[id].SetState(S_STATE::LOG_OUT);
	}

	SC_ADD_PLAYER_PACKET sub_packet;
	sub_packet.size = sizeof(sub_packet);
	sub_packet.type = SC_PACKET_ADD_PLAYER;
	strcpy(sub_packet.name, players[id].GetName().c_str());
	sub_packet.player_data = players[id].GetPlayerData();
	sub_packet.weapon = players[id].GetWeapon();

	for (auto& client : players) {
		if (client.second.GetID() == id) continue;
		if (client.second.GetState() != S_STATE::IN_GAME) continue;
		retval = client.second.DoSend(&sub_packet, sub_packet.size);
		if (retval == SOCKET_ERROR) {
			client.second.SetState(S_STATE::LOG_OUT);
		}
	}

	for (auto& client : players) {
		if (client.second.GetID() == id) continue;
		if (client.second.GetState() != S_STATE::IN_GAME) continue;
		SC_ADD_PLAYER_PACKET sub_packet2;
		sub_packet2.size = sizeof(sub_packet2);
		sub_packet2.type = SC_PACKET_ADD_PLAYER;
		strcpy(sub_packet2.name, client.second.GetName().c_str());
		sub_packet2.player_data = client.second.GetPlayerData();
		sub_packet2.weapon = client.second.GetWeapon();

		retval = players[id].DoSend(&sub_packet2, sub_packet2.size);
		if (retval == SOCKET_ERROR) {
			client.second.SetState(S_STATE::LOG_OUT);
		}
	}

	players[id].SetState(S_STATE::IN_GAME);
}

void SendPlayerMove(int id)
{
	int retval;
	SC_UPDATE_PLAYER_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_UPDATE_PLAYER;
	packet.player_data = players[id].GetPlayerData();

	for (auto& client : players) {
		if (client.second.GetID() == id) continue;
		if (client.second.GetState() != S_STATE::IN_GAME) continue;
		retval = client.second.DoSend(&packet, packet.size);
		if (retval == SOCKET_ERROR) {
			client.second.SetState(S_STATE::LOG_OUT);
		}
	}
}

void SendAnimaition(int id)
{
	int retval;
	SC_CHANGE_ANIMATION_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_CHANGE_ANIMATION;
	packet.animation = players[id].GetAnimaition();

	for (auto& client : players) {
		if (client.second.GetID() == id) continue;
		if (client.second.GetState() != S_STATE::IN_GAME) continue;
		retval = client.second.DoSend(&packet, packet.size);
		if (retval == SOCKET_ERROR) {
			client.second.SetState(S_STATE::LOG_OUT);
		}
	}
}