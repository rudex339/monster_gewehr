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
    //collision
    //같은 콜리전을 두번 반복하지 않도록하자. 그렇다면 플레이어만
    world->each<BoundingBox_Component, player_Component, Velocity_Component, Position_Component>([&](Entity* ent,
        ComponentHandle<BoundingBox_Component> BBox,
        ComponentHandle<player_Component> p,
        ComponentHandle<Velocity_Component> velocity,
        ComponentHandle<Position_Component> position) {
            XMFLOAT3 cur_Center = BBox->m_bounding_box.Center;
            
           
            world->each<BoundingBox_Component>([&](Entity* another,
            ComponentHandle<BoundingBox_Component> another_BBox) {
                    if (another == ent)
                        return;
                    XMFLOAT3 next_Center = BBox->m_bounding_box.Center;
                    next_Center.x += velocity->m_velocity.x;
                    next_Center.y += velocity->m_velocity.y;
                    next_Center.z += velocity->m_velocity.z;
                    BBox->m_bounding_box.Center = next_Center;
                    if (another_BBox->m_bounding_box.Intersects(BBox->m_bounding_box)) {
                        //BBox->m_bounding_box.Center = cur_Center;
                        cout << ent->get<Model_Component>()->model_name;
                        cout << " hit " << another->get<Model_Component>()->model_name << endl;

                        // Find the minimal translation vector (MTV)
                        XMVECTOR center1 = XMLoadFloat3(&BBox->m_bounding_box.Center);
                        XMVECTOR center2 = XMLoadFloat3(&another_BBox->m_bounding_box.Center);

                        // Calculate the difference vector
                        XMVECTOR delta = XMVectorSubtract(center2, center1);

                        // Calculate the extents along the delta vector
                        float distance = XMVectorGetX(XMVector3Length(delta));
                        float extent1 = XMVectorGetX(XMVector3Dot(delta, XMVector3Normalize(XMLoadFloat3(&BBox->m_bounding_box.Extents))));
                        float extent2 = XMVectorGetX(XMVector3Dot(delta, XMVector3Normalize(XMLoadFloat3(&another_BBox->m_bounding_box.Extents))));

                        // Calculate the penetration depth
                        float penetrationDepth = extent1 + extent2 - distance;

                        // Calculate the minimum translation vector (MTV) to resolve collision
                        XMFLOAT3 outmovement = XMFLOAT3(0.f, 0.f, 0.f);
                        DirectX::XMStoreFloat3(&outmovement, XMVectorScale(XMVector3Normalize(delta), penetrationDepth));

                        
                        


                        // 미끄러짐 벡터 계산
                        //const DirectX::XMVECTOR velocityVec = DirectX::XMLoadFloat3(&velocity->m_velocity);
                        //const DirectX::XMVECTOR dotProduct = DirectX::XMVector3Dot(velocityVec, normal);
                        //const DirectX::XMVECTOR slidingVector = DirectX::XMVectorSubtract(velocityVec, DirectX::XMVectorMultiply(normal, dotProduct));
                        // 속도 및 위치 업데이트
                        //DirectX::XMStoreFloat3(&veloOcity->m_velocity, slidingVector);
                        velocity->m_velocity.x += outmovement.x;
                        velocity->m_velocity.y += outmovement.y;
                        velocity->m_velocity.z += outmovement.z;
                        cout << velocity->m_velocity.x << " " << velocity->m_velocity.y << " " << velocity->m_velocity.z << endl;
                    }
                        
                });
            BBox->m_bounding_box.Center = cur_Center;
    //cout << endl << "final: ";
            //cout << velocity->m_velocity.x <<" " << velocity->m_velocity.y << " " << velocity->m_velocity.z <<endl;
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
                                world->emit<PacketSend_Event>({ (CHAR)ent->get<player_Component>()->id, position->Position, velocity->m_velocity,
                                    rotation->mfYaw, state, 0 });
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
                    }
                    else {
                        position->Position = Vector3::Add(position->Position, velocity->m_velocity);
                        if(velocity->gravity)
                            velocity->m_velocity = 
                            Vector3::Add(velocity->m_velocity, XMFLOAT3(0, -65.4f * deltaTime, 0));
                    }
        });
}
