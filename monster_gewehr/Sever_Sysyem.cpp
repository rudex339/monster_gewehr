#include "protocol.h"
#include "Sever_Sysyem.h"
#include "Player_Entity.h"
#include "Object_Entity.h"
#include "Scene_Sysytem.h"


void Sever_System::configure(World* world)
{
	world->subscribe<PacketSend_Event>(this);
	world->subscribe<Shoot_Event>(this);
	world->subscribe<Login_Event>(this);
	world->subscribe<Game_Start>(this);
	world->subscribe<Demo_Event>(this);
	world->subscribe<Create_Room>(this);
	world->subscribe<Join_Room>(this);
	world->subscribe<Quit_Room>(this);
	world->subscribe<Select_Room>(this);
	world->subscribe<Ready_Room>(this);
	world->subscribe<Set_Equipment>(this);
	world->subscribe<Heal_Event>(this);
	world->subscribe<Buy_Item>(this);
}

void Sever_System::tick(World* world, float deltaTime)
{	
	char buf[BUF_SIZE] = { 0 };

	int retval = recv(g_socket, buf, BUF_SIZE, 0);

	if (retval > 0) {
		PacketReassembly(world, buf, retval);
	}	

}

void Sever_System::receive(World* world, const PacketSend_Event& event)
{
	CS_PLAYER_MOVE_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_PLAYER_MOVE;
	packet.pos = event.pos;
	packet.yaw = event.yaw;
	send(g_socket, (char*)&packet, packet.size, 0);

	CS_CHANGE_ANIMATION_PACKET sub_packet;
	sub_packet.size = sizeof(sub_packet);
	sub_packet.type = CS_PACKET_CHANGE_ANIMATION;
	sub_packet.animation = (CHAR)event.State;

	send(g_socket, (char*)&sub_packet, sub_packet.size, 0);


}

void Sever_System::receive(class World* world, const Shoot_Event& event)
{
	CS_PLAYER_ATTACK_PACKET atk_packet;
	atk_packet.size = sizeof(atk_packet);
	atk_packet.type = CS_PACKET_PLAYER_ATTACK;
	atk_packet.dir = event.c_dir;
	atk_packet.pos = event.c_pos;

	send(g_socket, (char*)&atk_packet, atk_packet.size, 0);
}

void Sever_System::receive(World* world, const Login_Event& event)
{
	CS_LOGIN_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_LOGIN;

	strcpy_s(packet.name, event.id.c_str());
	strcpy_s(packet.password, event.password.c_str());

	send(g_socket, (char*)&packet, sizeof(packet), 0);
}

void Sever_System::receive(World* world, const Game_Start& event)
{
	CS_START_GAME_PACKET p;
	p.size = sizeof(p);
	p.type = CS_PACKET_START_GAME;

	send(g_socket, (char*)&p, p.size, 0);
}

void Sever_System::receive(class World* world, const Create_Room& event)
{
	CS_CREATE_ROOM_PACKET p;
	p.size = sizeof(p);
	p.type = CS_PACKET_CREATE_ROOM;	

	send(g_socket, (char*)&p, p.size, 0);
}

void Sever_System::receive(class World* world, const Join_Room& event)
{
	CS_JOIN_ROOM_PACKET p;
	p.size = sizeof(p);
	p.type = CS_PACKET_JOIN_ROOM;
	p.room_num = event.room_num;

	send(g_socket, (char*)&p, p.size, 0);
}

void Sever_System::receive(class World* world, const Quit_Room& event)
{
	CS_QUIT_ROOM_PACKET p;
	p.size = sizeof(p);
	p.type = CS_PACKET_QUIT_ROOM;

	send(g_socket, (char*)&p, p.size, 0);
}

void Sever_System::receive(class World* world, const Select_Room& event)
{
	CS_SELECT_ROOM_PACKET p;
	p.size = sizeof(p);
	p.type = CS_PACKET_SELECT_ROOM;
	p.room_num = event.room_num;

	send(g_socket, (char*)&p, p.size, 0);
}

void Sever_System::receive(World* world, const Ready_Room& event)
{	
	CS_READY_ROOM_PACKET p;
	p.size = sizeof(p);
	p.type = CS_PACKET_READY_ROOM;
	

	send(g_socket, (char*)&p, p.size, 0);
}

void Sever_System::receive(World* world, const Set_Equipment& event)
{
	CS_SET_EQUIPMENT_PACKET p;
	p.size = sizeof(p);
	p.type = CS_PACKET_SET_EQUIPMENT;
	p.weapon = event.weapon;
	p.armor = event.armor;
	p.grenade = event.grenade;

	send(g_socket, (char*)&p, p.size, 0);
}

void Sever_System::receive(World* world, const Heal_Event& event)
{
	CS_HEAL_PACKET p;
	p.size = sizeof(p);
	p.type = CS_PACKET_HEAL;
	p.hp = event.hp;
	p.item_type = event.item_type;

	send(g_socket, (char*)&p, p.size, 0);
}

void Sever_System::receive(World* world, const Buy_Item& event)
{
	CS_BUY_PACKET p;
	p.size = sizeof(p);
	p.type = CS_PACKET_BUY;
	p.money = event.money;
	p.item_type = event.item_type;

	send(g_socket, (char*)&p, p.size, 0);
}

void Sever_System::receive(World* world, const Demo_Event& event)
{
	CS_DEMO_PACKET demo;
	demo.size = sizeof(demo);
	demo.type = event.type;

	send(g_socket, (char*)&demo, demo.size, 0);

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
	case SC_PACKET_LOGIN_INFO: {
		SC_LOGIN_INFO_PACKET* pk = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(packet);
		m_id = (int)pk->id;
		//m_login = true;
		world->emit<LoginCheck_Event>({ (int)m_id });
		world->emit< ChangeScene_Event>({ LOBBY });
		cout << "고유 아이디 : " << m_id << endl;
		Sound_Componet::GetInstance().PlayMusic(Sound_Componet::Music::Title);
		break;
	}
	case SC_PACKET_LOGIN_FAIL: {
		cout << "로그인 실패" << endl;
		break;
	}
	case SC_PACKET_GAME_START: {
		SC_GAME_START_PACKET* pk = reinterpret_cast<SC_GAME_START_PACKET*>(packet);
		world->emit<StartRoom_Event>({ m_id, pk->room_num });
		break;
	}
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
					Player->m_weapon = pk->weapon;

					auto& weapon = ent->get<Model_Component>().get().m_pchildObjects.begin();
					for (int i = 0; i < 3; ++i) {
						if (i == pk->weapon) {
							weapon[i]->draw = true;
						}
						else weapon[i]->draw = false;
					}
					cout << Player->id << "번 추가, 무기 : " << Player->m_weapon << endl;
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

		world->each<player_Component, Position_Component, AnimationController_Component>(
			[&](Entity* ent,
				ComponentHandle<player_Component> Player,
				ComponentHandle<Position_Component> Position,
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
					Player->hp = pk->monster.hp;
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
					Sound_Componet::GetInstance().PlaySound(Sound_Componet::Sound::Hurt);
					if (ent->has<Camera_Component>() && Player->hp <= 0) {
						ComponentHandle<EulerAngle_Component> eulerangle =
							ent->get<EulerAngle_Component>();
						ComponentHandle<ControllAngle_Component> controllangle =
							ent->get<ControllAngle_Component>();

						Position->Position = XMFLOAT3(2465.f, 2.f, 826.f);
						Rotation->mfYaw = 0.f;
						eulerangle->m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
						eulerangle->m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
						eulerangle->m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

						controllangle->m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
						controllangle->m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
						controllangle->m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
						Player->hp = 100;						
					}
				}
				else
					return;

			});
		break;

	}
	case SC_PACKET_END_GAME: {
		SC_END_GAME_PACKET* pk = reinterpret_cast<SC_END_GAME_PACKET*>(packet);
		// 나중엔 점수 창이 뜨면 점수가 나오고 이를 확인하는 버튼을 눌러서 로비로 가게 하고싶음
		world->emit< ChangeScene_Event>({ END, pk->score });
		break;
	}
	case SC_PACKET_CREATE_ROOM: {
		SC_CREATE_ROOM_PACKET* pk = reinterpret_cast<SC_CREATE_ROOM_PACKET*>(packet);
		int len = MultiByteToWideChar(CP_ACP, 0, pk->name, -1, nullptr, 0);
		std::wstring wstr(len, L'\0');
		MultiByteToWideChar(CP_ACP, 0, pk->name, -1, &wstr[0], len);

		m_scene->AddRoom(pk->room_num, false, wstr);
		//world->emit< ChangeScene_Event>({ ROOMS });
		world->emit<EnterRoom_Event>({ INROOM, pk->room_num, true });
		break;
	}
	case SC_PACKET_ADD_ROOM: {
		SC_ADD_ROOM_PACKET* pk = reinterpret_cast<SC_ADD_ROOM_PACKET*>(packet);
		int len = MultiByteToWideChar(CP_ACP, 0, pk->name, -1, nullptr, 0);
		std::wstring wstr(len, L'\0');
		MultiByteToWideChar(CP_ACP, 0, pk->name, -1, &wstr[0], len);
		m_scene->AddRoom(pk->room_num, pk->start, wstr);
		world->emit<Refresh_Scene>({ ROOMS });
		break;
	}
	case SC_PACKET_SELECT_ROOM: {
		SC_SELECT_ROOM_PACKET* pk = reinterpret_cast<SC_SELECT_ROOM_PACKET*>(packet);
		// 만약 여기에 있는 정보로 방에 들어갈때 누가 들어가있는지 알고있다면 새로 추가된 pk->id도 인자로 넣어주셈 (나중에 누군가 나갈때 누가 나간건지 알고싶음)
		int len = MultiByteToWideChar(CP_ACP, 0, pk->name, -1, nullptr, 0);
		std::wstring wstr(len, L'\0');
		MultiByteToWideChar(CP_ACP, 0, pk->name, -1, &wstr[0], len);

		m_scene->AddRoomPlayers(wstr, pk->weapon);
		world->emit<ChangeScene_Event>({ ROOMS });
		break;
	}
	case SC_PACKET_READY_ROOM: {
		SC_READY_ROOM_PACKET* pk = reinterpret_cast<SC_READY_ROOM_PACKET*>(packet);
		m_scene->ReadyCheck(pk->id, pk->ready);
		world->emit<Refresh_Scene>({ INROOM });
		break;
	}
	case SC_PACKET_BREAK_ROOM: {
		// 여기에 방이 터졌다 뭐시기뭐시기 안내하는 창 뜨는 이벤트 같은거 넣으면 됨
		world->emit<ChangeScene_Event>({ ROOMS });
		break;
	}
	case SC_PACKET_DELETE_ROOM: {
		SC_DELETE_ROOM_PACKET* pk = reinterpret_cast<SC_DELETE_ROOM_PACKET*>(packet);
		m_scene->DeleteRoom(pk->room_num);
		world->emit<Refresh_Scene>({ ROOMS });
		break;
	}
	case SC_PACKET_JOIN_ROOM: {
		SC_JOIN_ROOM_PACKET* pk = reinterpret_cast<SC_JOIN_ROOM_PACKET*>(packet);
		world->emit<ChangeScene_Event>({ INROOM });
		break;
	}
	case SC_PACKET_QUIT_ROOM: {
		SC_QUIT_ROOM_PACKET* pk = reinterpret_cast<SC_QUIT_ROOM_PACKET*>(packet);
		// 여기에 방에서 방금 나간에 누군지 pk->id로 알아내고 삭제하면 됨
		m_scene->RemoveInRoomPlayers(pk->id);
		world->emit<Refresh_Scene>({ INROOM });
		break;
	}
	case SC_PACKET_ADD_ROOM_PLAYER: {
		SC_ADD_ROOM_PLAYER_PACKET* pk = reinterpret_cast<SC_ADD_ROOM_PLAYER_PACKET*>(packet);

		int len = MultiByteToWideChar(CP_ACP, 0, pk->name, -1, nullptr, 0);
		std::wstring wstr(len, L'\0');
		MultiByteToWideChar(CP_ACP, 0, pk->name, -1, &wstr[0], len);

		cout << "name : " << pk->name << " weapon : " << (int)pk->weapon << endl;

		RoomPlayer_Info info;
		info.id = pk->id;
		info.name = wstr;
		info.weapon = (int)pk->weapon;
		info.armor = pk->armor;
		info.host = pk->host;
		info.ready = pk->ready;
		m_scene->AddInRoomPlayers(info);
		world->emit<Refresh_Scene>({ INROOM });
		break;
	}
	case SC_PACKET_ITEM_INFO: {
		SC_ITEM_INFO_PACKET* pk = reinterpret_cast<SC_ITEM_INFO_PACKET*>(packet);

		world->emit<GetUserData_Event>({ pk->money, pk->item_info });
		break;
	}
	case SC_PACKET_SHOT: {	// 쏘는 소리 출력하라는 패킷
		SC_SHOT_PACKET* pk = reinterpret_cast<SC_SHOT_PACKET*>(packet);

		switch (pk->weapon) {
		case 0:
			Sound_Componet::GetInstance().Play3DSound(pk->pos, Sound_Componet::TDSound::TDRifle);
			break;
		case 1:
			Sound_Componet::GetInstance().Play3DSound(pk->pos, Sound_Componet::TDSound::TDShotGun);
			break;
		case 2:
			Sound_Componet::GetInstance().Play3DSound(pk->pos, Sound_Componet::TDSound::TDSniper);
			break;
		}
		break;
	}

	}


}
