#include "protocol.h"
#include "Sever_Sysyem.h"
#include "Player_Entity.h"
#include "Object_Entity.h"


void Sever_System::configure(World* world)
{
	world->subscribe<PacketSend_Event>(this);
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
						Position->Position = XMFLOAT3(1014.f, 1024.f, 1429.f);
						Rotation->mfYaw = 0.f;
					}
				}
				else
					return;

			});

	}

	}
	
	
}
