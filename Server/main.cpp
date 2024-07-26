#include "main.h"

int main(int argc, char* argv[])
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// ���ú� Ÿ�Ӿƿ�
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

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;

	std::thread B_Thread{ &BossThread };
	B_Thread.detach();

	while (1) {
		addrlen = sizeof(clientaddr);

		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {			
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
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

	constexpr int MAX_FRAME = 60;
	using frame = std::chrono::duration<int32_t, std::ratio<1, MAX_FRAME>>;
	std::chrono::time_point<std::chrono::steady_clock> fps_timer{ std::chrono::steady_clock::now() };
	
	// �� �߻� ���� �������ε� ���� ��� ������ ����
	int hit_timer = MAX_FRAME * 5;

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	players.try_emplace(id, id, client_sock);
	

	frame fps{}, frame_count{};
	//std::cout << id << std::endl;

	while (players[id].GetState() != S_STATE::LOG_OUT) {
		fps = std::chrono::duration_cast<frame>(std::chrono::steady_clock::now() - fps_timer);

		if (fps.count() < 1) continue; // 1/MAX_FRAME

		// �����͸� �޾Ƽ�
		retval = players[id].RecvData();
		
		if (retval > 0) {
			PacketReassembly(id, retval);			
		}
		else if (retval == -1) {
			std::cout << "����" << std::endl;
			break;
		}
		
		if (players[id].GetState() == S_STATE::IN_GAME) {
			if (players[id].hit_on) {
				if (hit_timer <= 0) {
					hit_timer = MAX_FRAME * 3;
					players[id].hit_on = 0;
				}
				else {
					hit_timer--;
				}
			}

			if (players[id].GetHp() <= 0) {
				players[id].death_count += 1;
				players[id].SetHp(100);
			}

		}
		fps_timer = std::chrono::steady_clock::now();
	}

	//database.Update(&players[id]);
	Disconnect(id);

	// ���� �ݱ�
	closesocket(client_sock);
	players[id].closesock();
	printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		addr, ntohs(clientaddr.sin_port));
	return;
}

void BossThread()
{
	constexpr int MAX_FRAME = 30;
	using frame = std::chrono::duration<int32_t, std::ratio<1, MAX_FRAME>>;
	using ms = std::chrono::duration<float, std::milli>;
	std::chrono::time_point<std::chrono::steady_clock> fps_timer{ std::chrono::steady_clock::now() };

	int bite_cooltime = 13;

	frame fps{}, frame_count{};
	while (1) {
		fps = std::chrono::duration_cast<frame>(std::chrono::steady_clock::now() - fps_timer);
		if (fps.count() < 1) continue; // 1/MAX_FRAME

		for (int i = 0; i < MAX_GAME_ROOM; i++) {
			if (gamerooms[i].GetState() == GameRoomState::G_INGAME) {
				run_bt(&souleaters[i], &players, &gamerooms[i]);

				if (souleaters[i].GetAnimation() == dash_ani) {
					for (int ply_id : gamerooms[i].GetPlyId()) {
						if (ply_id == -1) continue;
						if (players[ply_id].GetState() != S_STATE::IN_GAME) continue;
						if (players[ply_id].hit_on) continue;

						if (players[ply_id].GetBoundingBox().Intersects(souleaters[i].GetBoundingBox())) {
							players[ply_id].hit_on = 1;
							players[ply_id].SetHp(players[ply_id].GetHp() - 50);
							SendHitPlayer(players[ply_id].GetID());
						}						
					}
				}

				if (souleaters[i].GetAnimation() == bite_ani) {
					if (!bite_cooltime) {
						bite_cooltime = 13;
						for (int ply_id : gamerooms[i].GetPlyId()) {
							if (ply_id == -1) continue;
							if (players[ply_id].GetState() != S_STATE::IN_GAME) continue;
							if (players[ply_id].hit_on) continue;

							if (players[ply_id].GetBoundingBox().Intersects(souleaters[i].GetBoundingBox())) {
								players[ply_id].hit_on = 1;
								players[ply_id].SetHp(players[ply_id].GetHp() - 25);
								SendHitPlayer(players[ply_id].GetID());
								std::cout << "����" << std::endl;
							}							
						}
					}
					else {
						bite_cooltime--;
					}
				}
				else {
					bite_cooltime = 13;
				}

				SC_UPDATE_MONSTER_PACKET monster_packet;
				monster_packet.size = sizeof(monster_packet);
				monster_packet.type = SC_PACKET_UPDATE_MONSTER;
				monster_packet.monster = souleaters[i].GetData();
				monster_packet.animation = souleaters[i].GetAnimation();

				for (int ply_id : gamerooms[i].GetPlyId()) {
					if (ply_id == -1) continue;
					if (players[ply_id].GetState() != S_STATE::IN_GAME) continue;
					players[ply_id].DoSend(&monster_packet, monster_packet.size);
				}

				// ���Ͱ� ������ Ŭ����
				if (souleaters[i].GetHp() <= 0 && monster_packet.animation == die_ani) {
					for (int ply_id : gamerooms[i].GetPlyId()) {
						if (ply_id == -1) continue;
						if (players[ply_id].GetState() != S_STATE::IN_GAME) continue;
						SendEndGame(players[ply_id].GetID(), true);
						//database.Update(&players[ply_id]);
						players[ply_id].PlayerInit();
					}
					souleaters[i].InitMonster(); // �̰� data_race�� �Ǽ� ������ 2��° �÷��̾�� �״� ��ġ�� ���� ��ġ�� �ƴ� �̻��� ��ġ�� �Ű���
					gamerooms[i].InitGameRoom();
					SendDeleteRoom(i);
				}
				// �濡�� 3�� �׾ ���ӿ���
				else if (gamerooms[i].m_all_life <= 0) {
					for (int ply_id : gamerooms[i].GetPlyId()) {
						if (ply_id == -1) continue;
						if (players[ply_id].GetState() != S_STATE::IN_GAME) continue;
						SendEndGame(players[ply_id].GetID(), false);
						//database.Update(&players[ply_id]);
						players[ply_id].PlayerInit();
					}
					souleaters[i].InitMonster(); // �̰� data_race�� �Ǽ� ������ 2��° �÷��̾�� �״� ��ġ�� ���� ��ġ�� �ƴ� �̻��� ��ġ�� �Ű���
					gamerooms[i].InitGameRoom();
					SendDeleteRoom(i);
				}

			}
		}


		fps_timer = std::chrono::steady_clock::now();
	}

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
		players[id].SetPassword(packet->password);
		players[id].SetWeapon(0);
		players[id].SetRoomID(-1);

		std::cout << "���� : " << id << ", " << players[id].GetName().c_str() << ", " << players[id].GetPassword().c_str() << std::endl;
		SendLoginInfo(id);
		SendRoomList(id);
		SendItemInfo(id);

		// ������ ���̽� ������ ���
		/*if (database.Login(&players[id])) {
			SendLoginInfo(id);
			SendRoomList(id);
			SendItemInfo(id);
		}
		else {
			SendLoginFail(id);
		}*/

		break;
	}
	case CS_PACKET_START_GAME: {
		for (int ply_id : gamerooms[players[id].GetRoomID()].GetPlyId()) {
			if (ply_id == -1) continue;
			if (ply_id == id) continue;
			if (!players[ply_id].GetReady()) return;
		}
		SHORT room_id = players[id].GetRoomID();
		if (gamerooms[room_id].SetStartGame()) {
			build_bt(&souleaters[room_id], &players, &gamerooms[room_id]);

		}
		SendStartGame(id);
		break;
	}
	case CS_PACKET_PLAYER_MOVE: {
		CS_PLAYER_MOVE_PACKET* packet = reinterpret_cast<CS_PLAYER_MOVE_PACKET*>(p);
		players[id].SetPostion(packet->pos);
		players[id].SetYaw(packet->yaw);
		//std::cout << id << "���� ��ŭ �޾ƿ� " << packet->pos.x << std::endl;
		players[id].SetBoundingBox();
		//std::cout << players[id].GetBoundingBox().Center.x << std::endl;
		players[id].RotateBoundingBox();

		SendPlayerMove(id);
		break;
	}
	case CS_PACKET_CHANGE_ANIMATION: {
		CS_CHANGE_ANIMATION_PACKET* packet = reinterpret_cast<CS_CHANGE_ANIMATION_PACKET*>(p);
		players[id].SetAnimaition(packet->animation);

		SendAnimaition(id);
		break;
	}
	case CS_PACKET_PLAYER_ATTACK: {
		int room_id = players[id].GetRoomID();

		CS_PLAYER_ATTACK_PACKET* packet = reinterpret_cast<CS_PLAYER_ATTACK_PACKET*>(p);
		players[id].SetAtkDir(packet->dir);
		players[id].SetAtkPos(packet->pos);
		
		DirectX::XMVECTOR positionVec = XMLoadFloat3(&packet->pos);
		DirectX::XMVECTOR directionVec = XMLoadFloat3(&packet->dir);
		float shot_range = players[id].GetRange();

		XMFLOAT3 souleaterPosition = souleaters[room_id].GetPosition();
		DirectX::XMVECTOR souleaterPos = XMLoadFloat3(&souleaterPosition);
		DirectX::XMVECTOR distanceVec = souleaterPos - positionVec;
		float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(distanceVec));

		if (distance <= shot_range) {
			if (souleaters[room_id].GetState() == runaway_state || souleaters[room_id].GetState() == gohome_state) {

			}

			else if (souleaters[room_id].GetBoundingBox().Intersects(positionVec, directionVec, shot_range)) {
				souleaters[room_id].m_lock.lock();
				souleaters[room_id].SetHp(souleaters[room_id].GetHp() - players[id].GetAtk());
				souleaters[room_id].SetLatestAttackPlayer(&players[id]);
				if (souleaters[room_id].GetState() == idle_state || souleaters[room_id].GetState() == alert_state) {
					souleaters[room_id].SetState(fight_state);
					souleaters[room_id].SetTarget(&players[id]);
				}
				souleaters[room_id].m_lock.unlock();
				std::cout << souleaters[room_id].GetHp() << std::endl;
				build_bt(&souleaters[room_id], &players, &gamerooms[room_id]);
			}
		}

		SendShot(id);
		break;
	}
	case CS_PACKET_CREATE_ROOM: {
		for (int i = 0; i < MAX_GAME_ROOM; i++) {
			if (gamerooms[i].GetState() == G_FREE) {
				gamerooms[i].SetCreateRoom();
				gamerooms[i].SetPlayerId(id);
				gamerooms[i].SetHostName(players[id].GetName());
				players[id].SetRoomID(i);
				players[id].SetHost(true);
				SendRoomCreate(id, i);
				break;
			}
		}
		break;
	}
	case CS_PACKET_SELECT_ROOM: {
		CS_SELECT_ROOM_PACKET* packet = reinterpret_cast<CS_SELECT_ROOM_PACKET*>(p);
		SendRoomSelect(id, packet->room_num);
		break;
	}
	case CS_PACKET_JOIN_ROOM: {
		CS_JOIN_ROOM_PACKET *packet = reinterpret_cast<CS_JOIN_ROOM_PACKET*>(p);		
		// ���� Ǯ���� �ƴϸ� ���� �����ߴٰ� ����
		if (gamerooms[packet->room_num].SetPlayerId(id)) {
			players[id].SetRoomID(packet->room_num);
			SendRoomJoin(id);
		}
		break;
	}
	case CS_PACKET_QUIT_ROOM: {
		CS_QUIT_ROOM_PACKET *packet = reinterpret_cast<CS_QUIT_ROOM_PACKET*>(p);
		
		if (gamerooms[players[id].GetRoomID()].GetPlyId()[0] == id) { // ������ ������
			players[id].SetHost(false);
			// ���⿡ �� ���� ���ĵǾ��ٴ� �޼��� �˸��°� ������ ��
			SendDeleteRoom(players[id].GetRoomID());
			SendBreakRoom(id);
			players[id].SetRoomID(-1);
			players[id].SetReady(false);
		}
		else {
			SendRoomQuit(id);
			gamerooms[players[id].GetRoomID()].DeletePlayerId(id);
			players[id].SetRoomID(-1);
			players[id].SetReady(false);
		}
		break;
	}
	case CS_PACKET_READY_ROOM: {
		if (players[id].GetReady()) {
			players[id].SetReady(false);
		}
		else {
			players[id].SetReady(true);
		}
		SC_READY_ROOM_PACKET packet;
		packet.size = sizeof(SC_READY_ROOM_PACKET);
		packet.type = SC_PACKET_READY_ROOM;
		packet.id = id;
		packet.ready = players[id].GetReady();

		for (int ply_id : gamerooms[players[id].GetRoomID()].GetPlyId()) {
			if (ply_id == -1) continue;
			if (ply_id == id) continue;
			players[ply_id].DoSend(&packet, packet.size);
		}
		break;
	}
	case CS_PACKET_SET_EQUIPMENT: {
		CS_SET_EQUIPMENT_PACKET *packet = reinterpret_cast<CS_SET_EQUIPMENT_PACKET*>(p);
		players[id].SetWeapon(packet->weapon);
		players[id].SetArmor(packet->armor);
		players[id].SetThrowWp(packet->grenade);

		std::cout << "��� ���� : " << (int)players[id].GetWeapon() << ", " << (int)players[id].GetArmor() << ", " << (int)players[id].GetThrowWp() << std::endl;		
		break;
	}
	case CS_PACKET_BUY: {
		CS_BUY_PACKET* packet = reinterpret_cast<CS_BUY_PACKET*>(p);

		players[id].SetMoney(packet->money);
		players[id].SetItem(packet->item_type, players[id].GetItem(packet->item_type) + 1);
		std::cout << "�� : " << players[id].GetMoney() << std::endl;
		std::cout << "������ ������ : " << players[id].GetItem(packet->item_type) << std::endl;

		//database.Update(&players[id]);
		break;
	}
	case CS_PACKET_HEAL: {
		CS_HEAL_PACKET* packet = reinterpret_cast<CS_HEAL_PACKET*>(p);
		players[id].SetHp(packet->hp);

		int use_item = packet->item_type + S_BANDAGE;

		players[id].SetItem(use_item, players[id].GetItem(use_item) - 1);
		std::cout << "�� �Ϸ� : " << players[id].GetHp() << ", ���� : " << players[id].GetItem(use_item) << std::endl;

		// ���߿� ģ���� ü�� ���϶� ����ٰ� ģ�������׵� ü�� ���� �����°� �����
		// SendHitPlayer ��Ȱ�� �ϸ� �ɵ�?
		break;
	}
	case CS_DEMO_MONSTER_SETPOS: {
		int room_id = players[id].GetRoomID();
		souleaters[room_id].m_lock.lock();
		souleaters[room_id].SetPostion(XMFLOAT3(2465.f, 0.f, 826.f));
		souleaters[room_id].SetBoundingBox();
		souleaters[room_id].m_lock.unlock();
		souleaters[room_id].SetState(idle_state);
		build_bt(&souleaters[room_id], &players, &gamerooms[players[id].GetRoomID()]);
		break;
	}
	case CS_DEMO_MONSTER_SETHP: {
		int room_id = players[id].GetRoomID();
		souleaters[room_id].m_lock.lock();
		souleaters[room_id].SetHp(50);
		souleaters[room_id].SetRAHp(0);
		souleaters[room_id].m_lock.unlock();
		break;
	}
	case CS_DEMO_MONSTER_BEHAVIOR: {
		int room_id = players[id].GetRoomID();
		//souleaters[room_id].dash(7500.f);
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
	retval = players[id].DoSend(&packet, packet.size);
	if (retval == SOCKET_ERROR) {
		players[id].SetState(S_STATE::LOG_OUT);
	}

	players[id].SetState(S_STATE::LOBBY);
}

void SendLoginFail(int id)
{
	int retval;
	SC_LOGIN_INFO_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_LOGIN_FAIL;
	packet.id = -1;
	retval = players[id].DoSend(&packet, packet.size);

}

void SendStartGame(int id) // �̰� ������ ������ �ϸ� ������ ������ ������ �ٸ� ��������׵� ��� ������ ������ �Ǿ��ٴ� ��ȣ�� ���� ������
{
	int retval;

	int gameroom_id = players[id].GetRoomID();
	auto plys_id = gamerooms[gameroom_id].GetPlyId();

	SC_GAME_START_PACKET start_p;
	start_p.size = sizeof(start_p);
	start_p.type = SC_PACKET_GAME_START;
	start_p.room_num = (SHORT)gameroom_id;
	for (auto& ply : players) {
		if (ply.second.GetID() == -1) continue;
		retval = ply.second.DoSend(&start_p, start_p.size);
		if (retval == SOCKET_ERROR) {
			ply.second.SetState(S_STATE::LOG_OUT);
		}
	}

	// �̰��� �������� �÷��̾� ù ��ġ�� �����ؼ� �� �ڽſ��Ե� ù ���������� ������� ������ ����
	// ��Ŷ ������ ���̱� ���� ��������� �����ϴ� �����忡�� �ٸ� Ŭ���̾�Ʈ�� ��ġ�������� �����ϰ� �����°��� �� �����Ұ���
	for (int send_id : plys_id) {
		if (send_id == -1) continue;
		players[send_id].PlayerInit();

		SC_ADD_PLAYER_PACKET add_p;
		add_p.size = sizeof(add_p);
		add_p.type = SC_PACKET_ADD_PLAYER;
		strcpy(add_p.name, players[send_id].GetName().c_str());
		add_p.player_data = players[send_id].GetPlayerData();
		add_p.weapon = players[send_id].GetWeapon();
		add_p.armor = players[send_id].GetArmor();
		add_p.grenade = players[send_id].GetThrowWp();
		for (int recv_id : plys_id) {
			if (recv_id == -1) continue;
			if (recv_id == send_id) continue;
			retval = players[recv_id].DoSend(&add_p, add_p.size);

		}
		SC_ADD_MONSTER_PACKET monster_p;
		monster_p.size = sizeof(SC_ADD_MONSTER_PACKET);
		monster_p.type = SC_PACKET_ADD_MONSTER;
		monster_p.monster = souleaters[gameroom_id].GetData();

		players[send_id].DoSend(&monster_p, monster_p.size);

		players[send_id].SetState(S_STATE::IN_GAME);
	}

	
}

void SendPlayerMove(int id)
{
	int retval;
	SC_UPDATE_PLAYER_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_UPDATE_PLAYER;
	packet.player_data = players[id].GetPlayerData();

	int room_id = players[id].GetRoomID();

	// ���� ���ӷ� ���� ����� ���׸� ������
	for (int ply_id : gamerooms[room_id].GetPlyId()) {
		if (ply_id < 0) continue;
		if (ply_id == id) continue;
		if (players[ply_id].GetState() != S_STATE::IN_GAME) continue;
		retval = players[ply_id].DoSend(&packet, packet.size);
		if (retval == SOCKET_ERROR) {
			players[ply_id].SetState(S_STATE::LOG_OUT);
		}
	}
}

void SendAnimaition(int id)
{
	int retval;
	SC_CHANGE_ANIMATION_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_CHANGE_ANIMATION;
	packet.id = players[id].GetID();
	packet.animation = players[id].GetAnimaition();

	int room_id = players[id].GetRoomID();

	// ���� ���ӷ� ���� ����� ���׸� ������
	for (int ply_id : gamerooms[room_id].GetPlyId()) {
		if (ply_id < 0) continue;
		if (ply_id == id) continue;
		if (players[ply_id].GetState() != S_STATE::IN_GAME) continue;
		retval = players[ply_id].DoSend(&packet, packet.size);
		if (retval == SOCKET_ERROR) {
			players[ply_id].SetState(S_STATE::LOG_OUT);
		}
	}
}

void SendShot(int id)
{
	int retval;
	SC_SHOT_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_SHOT;
	packet.pos = players[id].GetPosition();
	packet.weapon = players[id].GetWeapon(); // 0, 1, 2

	int room_id = players[id].GetRoomID();

	// ���� ���ӷ� ���� ����� ���׸� ������
	for (int ply_id : gamerooms[room_id].GetPlyId()) {
		if (ply_id < 0) continue;
		if (ply_id == id) continue;
		if (players[ply_id].GetState() != S_STATE::IN_GAME) continue;
		retval = players[ply_id].DoSend(&packet, packet.size);
		if (retval == SOCKET_ERROR) {
			players[ply_id].SetState(S_STATE::LOG_OUT);
		}
	}
}

void Disconnect(int id)
{
	players[id].PlayerInit();
	players[id].SetState(S_STATE::LOG_OUT);

	short room_num = players[id].GetRoomID();

	if (room_num > -1) {
		SC_LOGOUT_PACKET packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET_LOGOUT;
		packet.id = id;
		for (int i : gamerooms[room_num].GetPlyId()) {
			if (i < 0) continue;
			if (players[i].GetID() == id) continue;
			players[i].DoSend(&packet, packet.size);
		}

		gamerooms[room_num].DeletePlayerId(id);
		if (gamerooms[room_num].IsRoomEmpty()) {
			souleaters[room_num].InitMonster(); 
			gamerooms[room_num].InitGameRoom();
			SendDeleteRoom(room_num);
		}
	}

}

void SendHitPlayer(int id)
{
	SC_HIT_PLAYER_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_HIT_PLAYER;
	packet.id = id;
	packet.hp = players[id].GetHp();

	int room_id = players[id].GetRoomID();
	if (packet.hp <= 0) {
		gamerooms[room_id].m_all_life -= 1;
	}

	for (int ply_id : gamerooms[room_id].GetPlyId()) {
		if (ply_id < 0) continue;
		if (players[ply_id].GetState() != S_STATE::IN_GAME) continue;
		players[ply_id].DoSend(&packet, packet.size);
	}
}

void SendEndGame(int id, bool clear)
{
	SC_END_GAME_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_END_GAME;
	if (clear)
		packet.score = 1000 - players[id].death_count * 200;
	else
		packet.score = 0;

	players[id].SetMoney(players[id].GetMoney() + packet.score);

	players[id].DoSend(&packet, packet.size);
}

void SendRoomList(int id)
{
	for (int i = 0; i < MAX_GAME_ROOM; i++) {
		gamerooms[i].SetStateLock();
		if (gamerooms[i].GetState() != G_FREE) {
			SC_ADD_ROOM_PACKET sub_packet;
			sub_packet.size = sizeof(sub_packet);
			sub_packet.type = SC_PACKET_ADD_ROOM;
			sub_packet.room_num = i;
			sub_packet.start = false;
			strcpy(sub_packet.name, gamerooms[i].GetHostName().c_str());
			if (gamerooms[i].GetState() == G_INGAME)
				sub_packet.start = true;

			players[id].DoSend(&sub_packet, sub_packet.size);
		}
		gamerooms[i].SetStateUnLock();
	}
}

void SendRoomCreate(int ply_id, int room_num)
{
	int retval;
	SC_CREATE_ROOM_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_CREATE_ROOM;
	packet.room_num = room_num;
	players[ply_id].DoSend(&packet, packet.size);

	SC_ADD_ROOM_PACKET sub_packet;
	sub_packet.size = sizeof(sub_packet);
	sub_packet.type = SC_PACKET_ADD_ROOM;
	sub_packet.room_num = room_num;
	sub_packet.start = false;
	strcpy(sub_packet.name, players[ply_id].GetName().c_str());

	for (auto& client : players) {
		if (client.second.GetID() == ply_id) continue;
		if (client.second.GetState() == S_STATE::LOG_OUT) continue;
		retval = client.second.DoSend(&sub_packet, sub_packet.size);
		if (retval == SOCKET_ERROR) {
			client.second.SetState(S_STATE::LOG_OUT);
		}
	}
}

void SendRoomSelect(int id, short room_num)
{
	int retval;
	
	for (int ply_id : gamerooms[room_num].GetPlyId()) {
		if (ply_id < 0) continue;
		SC_SELECT_ROOM_PACKET packet;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET_SELECT_ROOM;
		packet.id = ply_id;
		strcpy_s(packet.name, players[ply_id].GetName().c_str());
		packet.weapon = players[ply_id].GetWeapon();
		packet.armor = players[ply_id].GetArmor();
		// ���� ���� �濡 �ִ� �÷��̾� �������� ������ ������ �ٽ� ����
		retval = players[id].DoSend(&packet, packet.size);
		if (retval == SOCKET_ERROR) {
			players[id].SetState(S_STATE::LOG_OUT);
			return;
		}
	}
}

void SendBreakRoom(int id)
{
	short room_num = players[id].GetRoomID();

	SC_BREAK_ROOM_PACKET packet;
	packet.size = sizeof(SC_BREAK_ROOM_PACKET);
	packet.type = SC_PACKET_BREAK_ROOM;
	for (int ply_id : gamerooms[room_num].GetPlyId()) {
		if (ply_id == -1) continue;
		if (ply_id == id) continue;
		players[ply_id].SetRoomID(-1);
		players[ply_id].SetReady(false);
		players[ply_id].DoSend(&packet, packet.size);
	}

	gamerooms[room_num].InitGameRoom();
}

void SendDeleteRoom(short room_num)
{
	int retval;

	SC_DELETE_ROOM_PACKET packet;
	packet.size = sizeof(SC_DELETE_ROOM_PACKET);
	packet.type = SC_PACKET_DELETE_ROOM;
	packet.room_num = room_num;

	for (auto& client : players) {
		if (client.second.GetState() == S_STATE::LOG_OUT) continue;
		retval = client.second.DoSend(&packet, packet.size);
		if (retval == SOCKET_ERROR) {
			client.second.SetState(S_STATE::LOG_OUT);
		}
	}
}

// ���� �濡 ����ִ� ������״� ���� ���԰� � �������� �˷������
void SendRoomJoin(int id)
{
	short room_num = players[id].GetRoomID();

	// �� ���� Ŭ���̾�Ʈ���� �� ���� �ִٰ� �˷��ִ� ��Ŷ(Ǯ���̸� �� �Լ���ü�� ���� �ȵ�)
	SC_JOIN_ROOM_PACKET join_p;
	join_p.size = sizeof(join_p);
	join_p.type = SC_PACKET_JOIN_ROOM;

	players[id].DoSend(&join_p, join_p.size);


	// ���� �濡 ���ͼ� ������ �����ִ� �ֵ����� �ź��Դٰ� �˷��ִ� ��Ŷ
	SC_ADD_ROOM_PLAYER_PACKET add_p;
	add_p.size = sizeof(add_p);
	add_p.type = SC_PACKET_ADD_ROOM_PLAYER;
	add_p.id = id;	
	strcpy_s(add_p.name, players[id].GetName().c_str());
	add_p.weapon = players[id].GetWeapon();
	add_p.armor = players[id].GetArmor();
	add_p.host = players[id].GetHost();
	add_p.ready = players[id].GetReady();

	for (int ply_id : gamerooms[room_num].GetPlyId()) {
		if (ply_id == -1) continue;
		if (ply_id == id) continue;
		players[ply_id].DoSend(&add_p, add_p.size);

		// �ź����� ������ ���� �˷��ִ� ��Ŷ
		SC_ADD_ROOM_PLAYER_PACKET add_p2;
		add_p2.size = sizeof(add_p2);
		add_p2.type = SC_PACKET_ADD_ROOM_PLAYER;
		add_p2.id = ply_id;
		strcpy_s(add_p2.name, players[ply_id].GetName().c_str());
		add_p2.weapon = players[ply_id].GetWeapon();
		add_p2.armor = players[ply_id].GetArmor();
		add_p2.host = players[ply_id].GetHost();
		add_p2.ready = players[ply_id].GetReady();

		players[id].DoSend(&add_p2, add_p2.size);
	}
}

void SendRoomQuit(int id)
{
	short room_num = players[id].GetRoomID();

	SC_QUIT_ROOM_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_QUIT_ROOM;
	packet.id = id;

	for (int ply_id : gamerooms[room_num].GetPlyId()) {
		if (ply_id == -1) continue;
		if (ply_id == id) continue;
		players[ply_id].DoSend(&packet, packet.size);
	}

}

void SendItemInfo(int id)
{
	SC_ITEM_INFO_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_ITEM_INFO;
	packet.money = players[id].GetMoney();
	for (int i = 0; i < 10; i++) {
		packet.item_info[i] = players[id].GetItem(i);
	}

	players[id].DoSend(&packet, packet.size);
}