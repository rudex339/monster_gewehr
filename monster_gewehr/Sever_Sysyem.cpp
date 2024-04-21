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
	SC_PLAYER_PACKET pk;	

	recv(g_socket, (char*)&pk, sizeof(pk), 0);
		
	world->each<player_Component, Position_Component, Rotation_Component>(
		[&](Entity* ent,
			ComponentHandle<player_Component> Player,
			ComponentHandle<Position_Component> Position,
			ComponentHandle<Rotation_Component> Rotation)->
		void {
			for (int i = 0; i < MAX_CLIENT_ROOM; ++i) {
				if (pk.players[i].id != -1 && (Player->id == pk.players[i].id || Player->id == -1)) {
					if (!ent->has<Camera_Component>()) {
						Player->id = pk.players[i].id;
						Position->Position = pk.players[i].pos;
						Rotation->mfYaw = pk.players[i].yaw;
						pk.players[i].id = -1;
					}
				}
			}
		});
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
