#include "protocol.h"
#include "Sever_Sysyem.h"
#include "Player_Entity.h"
#include "Object_Entity.h"
#include "Scene_Sysytem.h"


void Sever_System::configure(World* world)
{
	world->subscribe<PacketSend_Event>(this);
	world->subscribe<Login_Event>(this);
	world->subscribe<Game_Start>(this);
}

void Sever_System::tick(World* world, float deltaTime)
{
	if (m_login) {
		char buf[BUF_SIZE] = { 0 };

		int retval = recv(g_socket, buf, BUF_SIZE, 0);

		if (retval > 0) {
			PacketReassembly(world, buf, retval);
		}
	}
	
}

void Sever_System::receive(World* world, const PacketSend_Event& event)
{
	CS_PLAYER_MOVE_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PLAYER_MOVE;
	packet.pos = event.pos;
	packet.yaw = event.yaw;
	//cout << (int)pk.id << evnet.pos << endl;
	send(g_socket, (char*)&packet, packet.size, 0);

	CS_CHANGE_ANIMATION_PACKET sub_packet;
	sub_packet.size = sizeof(sub_packet);
	sub_packet.type = CS_PACKET_CHANGE_ANIMATION;
	sub_packet.animation = event.State;

	send(g_socket, (char*)&sub_packet, sub_packet.size, 0);

	if (event.State == (UINT)SHOOT) {
		CS_PLAYER_ATTACK_PACKET atk_packet;
		atk_packet.size = sizeof(atk_packet);
		atk_packet.type = CS_PACKET_PLAYER_ATTACK;
		atk_packet.dir = event.c_dir;
		atk_packet.pos = event.c_pos;
		//cout << atk_packet.dir.x << endl;

		send(g_socket, (char*)&atk_packet, atk_packet.size, 0);
	}

}

void Sever_System::receive(World* world, const Login_Event& event)
{
	world->each<
		player_Component>(
			[&](Entity* ent, 
				ComponentHandle<player_Component> data) -> void {					
					WSADATA wsa;
					WSAStartup(MAKEWORD(2, 2), &wsa);

					g_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);

					ZeroMemory(&server_addr, sizeof(server_addr));
					server_addr.sin_family = AF_INET;
					server_addr.sin_port = htons(SERVER_PORT);
					inet_pton(AF_INET, SERVER_IP.c_str(), &server_addr.sin_addr);

					connect(g_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

					unsigned long noblock = 1;
					ioctlsocket(g_socket, FIONBIO, &noblock);

					CS_LOGIN_PACKET packet;
					packet.size = sizeof(packet);
					packet.type = CS_PACKET_LOGIN;
					int weapon = 0;
					cout << "이름 입력" << endl;
					cin >> packet.name;
					cout << "무기 입력" << endl;
					cin >> weapon;
					packet.weapon = weapon;


					int retval = send(g_socket, (char*)&packet, sizeof(packet), 0);

					SC_LOGIN_INFO_PACKET sub_packet;
					while (1) {
						int retval = recv(g_socket, (char*)&sub_packet, sizeof(sub_packet), 0);
						if (retval > 0) {
							if (retval != sizeof(SC_LOGIN_INFO_PACKET)) {
								cout << "ㅈ됬어..." << endl;
							}

							break;
						}
						else {
							cout << "못받음" << endl;
						}
					}
					//cout << (int)ply.id << endl;
					data->id = (int)sub_packet.id;
					std::cout << "성공했음 일단" << (int)sub_packet.id << std::endl;
					m_login = true;
					//cout << Data->id << endl;
			});

}

void Sever_System::receive(World* world, const Game_Start& event)
{

	cout << "실행됨" << endl;
	world->each<player_Component, Position_Component, Velocity_Component, Rotation_Component>(
		[&](Entity* ent,
			ComponentHandle<player_Component> Player,
			ComponentHandle<Position_Component> Position,
			ComponentHandle< Velocity_Component> Velocity,
			ComponentHandle<Rotation_Component> Rotation) ->
		void {
			if (ent->has<Camera_Component>()) {
				
				CS_START_GAME_PACKET p;
				p.size = sizeof(p);
				p.type = CS_PACKET_START_GAME;
				p.pos = Position->Position;
				p.vel = Velocity->m_velocity;
				p.yaw = Rotation->mfYaw;

				send(g_socket, (char*)&p, p.size, 0);
			}

		});
}

void Sever_System::PacketReassembly(World* world, char* recv_buf, size_t recv_size)
{
	char* p = recv_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != recv_size) {
		if (0 == in_packet_size) in_packet_size = p[0];
		if (recv_size + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, p, in_packet_size - saved_packet_size);
			ProcessPacket(world, packet_buffer);
			p += in_packet_size - saved_packet_size;
			recv_size -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, p, recv_size);
			saved_packet_size += recv_size;
			recv_size = 0;
		}
	}
}

void Sever_System::ProcessPacket(World* world, char* packet)
{
	switch (packet[1])
	{
	case SC_PACKET_ADD_PLAYER: {
		SC_ADD_PLAYER_PACKET* pk = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(packet);

		world->each<player_Component, Position_Component, Rotation_Component>(
			[&](Entity* ent,
				ComponentHandle<player_Component> Player,
				ComponentHandle<Position_Component> Position,
				ComponentHandle<Rotation_Component> Rotation) ->
			void {
				if (pk->player_data.id == -1)
					return;
				else if (Player->id == -1) {
					Player->id = pk->player_data.id;
					Position->Position = pk->player_data.pos;
					Rotation->mfYaw = pk->player_data.yaw;
					pk->player_data.id = -1;
				}
				else
					return;

			});
		break;
	}
	case SC_PACKET_UPDATE_PLAYER: {
		SC_UPDATE_PLAYER_PACKET* pk = reinterpret_cast<SC_UPDATE_PLAYER_PACKET*>(packet);

		world->each<player_Component, Position_Component, Rotation_Component>(
			[&](Entity* ent,
				ComponentHandle<player_Component> Player,
				ComponentHandle<Position_Component> Position,
				ComponentHandle<Rotation_Component> Rotation) ->
			void {
				if (Player->id == pk->player_data.id) {
					Position->Position = pk->player_data.pos;
					Rotation->mfYaw = pk->player_data.yaw;
				}
				else
					return;

			});
		break;
	}
	case SC_PACKET_CHANGE_ANIMATION: {
		SC_CHANGE_ANIMATION_PACKET* pk = reinterpret_cast<SC_CHANGE_ANIMATION_PACKET*>(packet);

		world->each<player_Component, AnimationController_Component>(
			[&](Entity* ent,
				ComponentHandle<player_Component> Player,
				ComponentHandle<AnimationController_Component> AnimationController)->
			void {
				if (Player->id == pk->id) {
					AnimationController->next_State = pk->animation;
				}
				else
					return;

			});
		break;
	}
	case SC_PACKET_ADD_MONSTER: {
		SC_ADD_MONSTER_PACKET* pk = reinterpret_cast<SC_ADD_MONSTER_PACKET*>(packet);
		
		world->each<player_Component, Position_Component, Rotation_Component>(
			[&](Entity* ent,
				ComponentHandle<player_Component> Player,
				ComponentHandle<Position_Component> Position,
				ComponentHandle<Rotation_Component> Rotation) ->
			void {
				if (Player->id == pk->monster.id) {
					Player->hp == pk->monster.hp;
					Player->max_hp = 1000;
					Position->Position = pk->monster.pos;
					Rotation->mfYaw = pk->monster.yaw;
				}
				else
					return;

			});
		break;
	}
	case SC_PACKET_UPDATE_MONSTER: {
		SC_UPDATE_MONSTER_PACKET* pk = reinterpret_cast<SC_UPDATE_MONSTER_PACKET*>(packet);

		world->each<player_Component, Position_Component, Rotation_Component, AnimationController_Component>(
			[&](Entity* ent,
				ComponentHandle<player_Component> Player,
				ComponentHandle<Position_Component> Position,
				ComponentHandle<Rotation_Component> Rotation,
				ComponentHandle<AnimationController_Component> AnimationController) ->
			void {
				if (Player->id == pk->monster.id) {
					Player->hp = pk->monster.hp;
					Position->Position = pk->monster.pos;
					Rotation->mfYaw = pk->monster.yaw;
					AnimationController->next_State = pk->animation;
				}
				else
					return;

			});
		break;
	}
	case SC_PACKET_LOGOUT: {
		SC_LOGOUT_PACKET* pk = reinterpret_cast<SC_LOGOUT_PACKET*>(packet);

		world->each<player_Component>(
			[&](Entity* ent,
				ComponentHandle<player_Component> Player) ->
			void {
				if (Player->id == pk->id) {
					Player->id = -1;
				}
				else
					return;

			});
		break;

	}
	case SC_PACKET_HIT_PLAYER: {
		SC_HIT_PLAYER_PACKET* pk = reinterpret_cast<SC_HIT_PLAYER_PACKET*>(packet);

		world->each<player_Component, Position_Component, Rotation_Component>(
			[&](Entity* ent,
				ComponentHandle<player_Component> Player,
				ComponentHandle<Position_Component> Position,
				ComponentHandle<Rotation_Component> Rotation) ->
			void {
				if (Player->id == pk->id) {
					Player->hp = pk->hp;
					cout << Player->hp << endl;
					if (ent->has<Camera_Component>() && Player->hp <= 0) {
						ComponentHandle<EulerAngle_Component> eulerangle =
							ent->get<EulerAngle_Component>();
						ComponentHandle<ControllAngle_Component> controllangle =
							ent->get<ControllAngle_Component>();

						Position->Position = XMFLOAT3(1014.f, 1024.f, 1429.f);
						Rotation->mfYaw = 0.f;
						eulerangle->m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
						eulerangle->m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
						eulerangle->m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

						controllangle->m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
						controllangle->m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
						controllangle->m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
					}
				}
				else
					return;

			});

	}
	case SC_PACKET_END_GAME: {
		SC_END_GAME_PACKET* pk = reinterpret_cast<SC_END_GAME_PACKET*>(packet);
		// 나중엔 점수 창이 뜨면 점수가 나오고 이를 확인하는 버튼을 눌러서 로비로 가게 하고싶음
		cout << "게임 클리어!" << endl;
		cout << "스코어 : " << pk->score << endl << endl;
		cout << "로비로 가고싶으면 아무 키나 입력" << endl;

		
	}

	}
	
	
}
