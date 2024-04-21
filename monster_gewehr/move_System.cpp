#include "stdafx.h"
#include "move_System.h"
#include "Object_Entity.h"
#include "Player_Entity.h"
#include "Sever_Sysyem.h"

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
        Position_Component,
    Rotation_Component,
        player_Component>(
            [&](Entity* ent, ComponentHandle<Velocity_Component> velocity,
                ComponentHandle<Position_Component> position,
                ComponentHandle<Rotation_Component> rotation,
                ComponentHandle<player_Component> p) -> void {

                    position->Position = Vector3::Add(position->Position, velocity->m_velocity);                   

                    rotation->mfYaw += velocity->m_velRotate.x;
                    rotation->mfPitch += velocity->m_velRotate.y;
                    rotation->mfRoll += velocity->m_velRotate.z;

                    if (ent->has<Camera_Component>()) {
                        auto camera = ent->get<Camera_Component>();
                        auto eulerangle = ent->get<EulerAngle_Component>();
                        XMFLOAT3 LockPos = XMFLOAT3(position->Position.x, position->Position.y + 10.f, position->Position.z);

                        XMFLOAT3 camera_pos = position->Position;
                        camera_pos = Vector3::Add(camera_pos, eulerangle->m_xmf3Look, -30.f);
                        camera_pos = Vector3::Add(camera_pos, eulerangle->m_xmf3Up, 20.f);
                        camera->m_pCamera->SetPosition(camera_pos);
                        camera->m_pCamera->SetLookAt(LockPos, eulerangle->m_xmf3Up);
                        camera->m_pCamera->RegenerateViewMatrix();

#ifdef USE_NETWORK
                        if(ent->has<player_Component>())
                            world->emit<PacketSend_Event>({ (CHAR)ent->get<player_Component>()->id, position->Position, velocity->m_velocity , rotation->mfYaw, 0});
#endif
                    }
                    velocity->m_velocity = XMFLOAT3(0, 0, 0);
                    velocity->m_velRotate = XMFLOAT3(0, 0, 0);
        });
}
