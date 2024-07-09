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
                    XMFLOAT3 next_pos = Vector3::Add(position->Position, velocity->m_velocity);
                    //cout << next_pos.x << "  " << next_pos.z << endl;
                    if ((m_Terrain->m_pTerrain->GetHeight(next_pos
                        .x, next_pos.z)- next_pos.y) < 5.f) {
                        next_pos.y = m_Terrain->m_pTerrain->GetHeight(next_pos.x, next_pos.z);
                        position->Position = next_pos;
                    }
                    //position->Position = next_pos;

                    rotation->mfYaw += velocity->m_velRotate.x;
                    rotation->mfPitch += velocity->m_velRotate.y;
                    rotation->mfRoll += velocity->m_velRotate.z;

                    if (ent->has<Camera_Component>()) {
                        auto camera = ent->get<Camera_Component>();
                        auto eulerangle = ent->get<EulerAngle_Component>();
                        auto controllangle = ent->get<ControllAngle_Component>();
                        XMFLOAT3 LockPos = XMFLOAT3(position->Position.x, position->Position.y + 13.f, position->Position.z);

                        XMFLOAT3 camera_pos = LockPos; // position->Position;

                        if (p->aim_mode) {
                            
                            camera_pos = Vector3::Add(camera_pos, eulerangle->m_xmf3Look, -6.f);
                            camera_pos = Vector3::Add(camera_pos, controllangle->m_xmf3Right, 5.f);
                            LockPos = Vector3::Add(LockPos, eulerangle->m_xmf3Right, 5.f);
                            
                            camera->m_pCamera->SetPosition(camera_pos);
                            camera->m_pCamera->SetLookAt(LockPos, controllangle->m_xmf3Up);
                            camera->m_pCamera->RegenerateViewMatrix();
                        }

                        else {
                            camera_pos = Vector3::Add(camera_pos, eulerangle->m_xmf3Look, -20.f);

                            if (m_Terrain->m_pTerrain->GetHeight(camera_pos.x, camera_pos.z) > camera_pos.y) {
                                camera->m_pCamera->SetPosition(camera_pos);
                                camera->m_pCamera->SetLookAt(LockPos, eulerangle->m_xmf3Up);
                                while (m_Terrain->m_pTerrain->GetHeight(camera_pos.x, camera_pos.z) > camera_pos.y) {
                                    camera_pos = Vector3::Add(camera_pos, controllangle->m_xmf3Look, 2.f);
                                }
                                camera->m_pCamera->SetPosition(camera_pos);
                                camera->m_pCamera->RegenerateViewMatrix();
                            }
                            /*if (m_Terrain->m_pTerrain->GetHeight(camera_pos.x, camera_pos.z) > camera_pos.y) {

                               camera_pos.y = m_Terrain->m_pTerrain->GetHeight(camera_pos.x, camera_pos.z) + 2.f;
                            }*/
                            else {
                                camera->m_pCamera->SetPosition(camera_pos);
                                camera->m_pCamera->SetLookAt(LockPos, eulerangle->m_xmf3Up);
                                camera->m_pCamera->RegenerateViewMatrix();
                            }
                        }
#ifdef USE_NETWORK
                        if (ent->has<player_Component>()) {
                            UINT state = ent->get<AnimationController_Component>()->cur_State;
                            world->emit<PacketSend_Event>({ (CHAR)ent->get<player_Component>()->id, position->Position, velocity->m_velocity, 
                                rotation->mfYaw, state, 0});
                        }
#endif
                        if (p->reload) {
                            if (p->reload_coolTime <= 0) {
                                cout << "리로드 완료" << endl;
                                p->ammo = 30;
                                p->reload_coolTime = 3.5f;
                                p->reload = false;
                            }
                            else {
                                p->reload_coolTime -= deltaTime;
                            }
                        }
                    }
                    velocity->m_velocity = XMFLOAT3(0, 0, 0);
                    velocity->m_velRotate = XMFLOAT3(0, 0, 0);
        });
}
