#include "stdafx.h"
#include "move_System.h"
#include "Object_Entity.h"


void Move_System::configure(World* world)
{
}

void Move_System::unconfigure(World* world)
{
}

void Move_System::tick(World* world, float deltaTime)
{
    ComponentHandle<Terrain_Component> m_Terrain;
    world->each<Terrain_Component>([&](Entity * ent, ComponentHandle<Terrain_Component> Terrain)-> void {
        m_Terrain = Terrain;
    });
    world->each<Velocity_Component,
        Position_Component>(
            [&](Entity* ent, ComponentHandle<Velocity_Component> velocity,
                ComponentHandle<Position_Component> position) -> void {

                    //position->Position = Vector3::Add(position->Position, velocity->m_velocity);
                    //velocity->m_velocity = XMFLOAT3(0, 0, 0);

                    
        });
}
