#include "stdafx.h"
#include "move_System.h"
#include "Object_Entity.h"
#include "Player_Entity.h"
#include "Sever_Sysyem.h"

float AngleBetweenVectors(XMVECTOR v1, XMVECTOR v2) {
    // 내적을 계산
    float dotProduct = XMVectorGetX(XMVector3Dot(v1, v2));
    // 벡터의 크기를 계산
    float len1 = XMVectorGetX(XMVector3Length(v1));
    float len2 = XMVectorGetX(XMVector3Length(v2));
    // 코사인 값을 계산
    float cosTheta = dotProduct / (len1 * len2);
    // 아크 코사인을 사용하여 각도를 계산
    float angle = acos(cosTheta);
    return angle;
}

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
    
    
    //velocity 적용
    world->each<Velocity_Component,
        Position_Component,
    Rotation_Component>(
            [&](Entity* ent, ComponentHandle<Velocity_Component> velocity,
                ComponentHandle<Position_Component> position,
                ComponentHandle<Rotation_Component> rotation) -> void {
                    if (ent->has<player_Component>()) {
                        ComponentHandle<player_Component> p = ent->get<player_Component>();
                        XMFLOAT3 next_pos = Vector3::Add(position->Position, velocity->m_velocity);
                        //cout << next_pos.x << "  " << next_pos.z << endl;
                        if ((m_Terrain->m_pTerrain->GetHeight(next_pos
                            .x, next_pos.z) - next_pos.y) < 5.f) {
                            //next_pos.y = m_Terrain->m_pTerrain->GetHeight(next_pos.x, next_pos.z);
                            //position->Position = next_pos;
                        }
                        position->Position = next_pos;

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

                                //if (m_Terrain->m_pTerrain->GetHeight(camera_pos.x, camera_pos.z) > camera_pos.y) {
                                //    camera->m_pCamera->SetPosition(camera_pos);
                                //    camera->m_pCamera->SetLookAt(LockPos, eulerangle->m_xmf3Up);
                                //    while (m_Terrain->m_pTerrain->GetHeight(camera_pos.x, camera_pos.z) > camera_pos.y) {
                                //        camera_pos = Vector3::Add(camera_pos, controllangle->m_xmf3Look, 2.f);
                                //    }
                                //    camera->m_pCamera->SetPosition(camera_pos);
                                //    camera->m_pCamera->RegenerateViewMatrix();
                                //}
                                ///*if (m_Terrain->m_pTerrain->GetHeight(camera_pos.x, camera_pos.z) > camera_pos.y) {

                                //   camera_pos.y = m_Terrain->m_pTerrain->GetHeight(camera_pos.x, camera_pos.z) + 2.f;
                                //}*/
                                //else {
                                camera->m_pCamera->SetPosition(camera_pos);
                                camera->m_pCamera->SetLookAt(LockPos, eulerangle->m_xmf3Up);
                                camera->m_pCamera->RegenerateViewMatrix();
                                //}
                            }
#ifdef USE_NETWORK
                            if (ent->has<player_Component>()) {
                                UINT state = ent->get<AnimationController_Component>()->cur_State;
                                world->emit<PacketSend_Event>({ position->Position, velocity->m_velocity,
                                    rotation->mfYaw, state, 0 });
                            }
#endif
                            // sound 듣는 위치 설정
                            Sound_Componet::GetInstance().ListenerUpdate(position->Position, velocity->m_velocity, eulerangle->m_xmf3Look, eulerangle->m_xmf3Up);
                        }
                        velocity->m_velocity = XMFLOAT3(0, 0, 0);
                        velocity->m_velRotate = XMFLOAT3(0, 0, 0);
                    }
                    else {
                        position->Position = Vector3::Add(position->Position, velocity->m_velocity);
                        if(velocity->gravity)
                            velocity->m_velocity = 
                            Vector3::Add(velocity->m_velocity, XMFLOAT3(0, -65.4f * deltaTime, 0));
                    }
        });
}
