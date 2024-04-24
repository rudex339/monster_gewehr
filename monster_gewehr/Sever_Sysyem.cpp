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
				else if(Player->id == -1) {
					Player->id = pk->player_data.id;
					Position->Position = pk->player_data.pos;
					Rotation->mfYaw = pk->player_data.yaw;
				}
			});

	}
	}
}
