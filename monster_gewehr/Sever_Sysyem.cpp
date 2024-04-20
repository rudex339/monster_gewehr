#include "protocol.h"
#include "Sever_Sysyem.h"


void Sever_System::configure(World* world)
{
	world->subscribe<PacketSend_Event>(this);
}

void Sever_System::receive(World* world, const PacketSend_Event& event)
{
	CS_PLAYER_PACKET pk;
	pk.id = event.id;
	pk.pos = event.pos;
	pk.vel = event.vel;
	pk.yaw = event.yaw;

	send(g_socket, (char*)&pk, sizeof(pk), 0);
}
