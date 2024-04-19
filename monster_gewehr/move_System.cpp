#include "stdafx.h"
#include "move_System.h"
#include "Object_Entity.h"

void move_System::configure(World* world)
{
}

void move_System::unconfigure(World* world)
{
}

void move_System::tick(World* world, float deltaTime)
{
    world->each<Velocity_Component,
        Position_Component>(
            [&](Entity* ent, ComponentHandle<Velocity_Component> velocity,
                ComponentHandle<Position_Component> position) -> void {
                    position->m_xmf4x4World._41 += velocity->m_velocity.x;
                    position->m_xmf4x4World._42 += velocity->m_velocity.y;
                    position->m_xmf4x4World._43 += velocity->m_velocity.z;
        });
}
