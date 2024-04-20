#include "stdafx.h"
#include "Sever_Sysyem.h"
#include "protocol.h"

void Sever_System::configure(World* world)
{
	world->subscribe<PacketSend_Event>(this);
}

void Sever_System::receive(World* world, const PacketSend_Event& event)
{

}
