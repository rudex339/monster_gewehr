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
    //collision
    //같은 콜리전을 두번 반복하지 않도록하자. 그렇다면 플레이어만
    world->each<BoundingBox_Component, player_Component, Velocity_Component, Position_Component>([&](Entity* ent,
        ComponentHandle<BoundingBox_Component> BBox,
        ComponentHandle<player_Component> p,
        ComponentHandle<Velocity_Component> velocity,
        ComponentHandle<Position_Component> position) {
            world->each<BoundingBox_Component>([&](Entity* another,
            ComponentHandle<BoundingBox_Component> another_BBox) {
                    if (ent == another)return; 
                    if (another_BBox->m_bounding_box.Intersects(BBox->m_bounding_box)) {
                        cout << ent->get<Model_Component>()->model_name;
                        cout << " hit " << another->get<Model_Component>()->model_name << endl;
                        const DirectX::XMVECTOR centerA = DirectX::XMLoadFloat3(&BBox->m_bounding_box.Center);
                        const DirectX::XMVECTOR centerB = DirectX::XMLoadFloat3(&another_BBox->m_bounding_box.Center);
                        const DirectX::XMVECTOR relativePosition = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(centerB, centerA));

                        // 박스 A의 회전 쿼터니언
                        const DirectX::XMVECTOR orientationA = DirectX::XMLoadFloat4(&BBox->m_bounding_box.Orientation);

                        // 박스 A의 축(법선 벡터)
                        const DirectX::XMVECTOR axisX = DirectX::XMVector3Rotate(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), orientationA);
                        const DirectX::XMVECTOR axisY = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), orientationA);
                        const DirectX::XMVECTOR axisZ = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), orientationA);

                        DirectX::XMVECTOR upSide = DirectX::XMVectorScale(axisY, another_BBox->m_bounding_box.Extents.y);      // 위쪽
                        DirectX::XMVECTOR underSide = DirectX::XMVectorScale(axisY, -another_BBox->m_bounding_box.Extents.y);  // 아래쪽
                        DirectX::XMVECTOR frontSide = DirectX::XMVectorScale(axisZ, another_BBox->m_bounding_box.Extents.z);   // 앞쪽
                        DirectX::XMVECTOR backSide = DirectX::XMVectorScale(axisZ, -another_BBox->m_bounding_box.Extents.z);   // 뒤쪽
                        DirectX::XMVECTOR leftSide = DirectX::XMVectorScale(axisX, -another_BBox->m_bounding_box.Extents.x);   // 왼쪽
                        DirectX::XMVECTOR rightSide = DirectX::XMVectorScale(axisX, another_BBox->m_bounding_box.Extents.x);   // 오른쪽


                        DirectX::XMVECTOR normal = DirectX::XMVectorZero();
                        if (XMVectorGetX(DirectX::XMVector3Dot(relativePosition, upSide)) < 0.0f) {
                            // relativePosition이 위쪽 면에 위치할 경우
                            normal= axisY;
                        }
                        else if (XMVectorGetX(DirectX::XMVector3Dot(relativePosition, underSide)) > 0.0f) {
                            // relativePosition이 아래쪽 면에 위치할 경우
                            normal = DirectX::XMVectorNegate(axisY);
                        }
                        else if (XMVectorGetX(DirectX::XMVector3Dot(relativePosition, frontSide)) < 0.0f) {
                            // relativePosition이 앞쪽 면에 위치할 경우
                            normal = axisZ;
                        }
                        else if (XMVectorGetX(DirectX::XMVector3Dot(relativePosition, backSide)) > 0.0f) {
                            // relativePosition이 뒤쪽 면에 위치할 경우
                            normal = DirectX::XMVectorNegate(axisZ);
                        }
                        else if (XMVectorGetX(DirectX::XMVector3Dot(relativePosition, leftSide)) > 0.0f) {
                            // relativePosition이 왼쪽 면에 위치할 경우
                            normal = DirectX::XMVectorNegate(axisX);
                        }
                        else if (XMVectorGetX(DirectX::XMVector3Dot(relativePosition, rightSide)) < 0.0f) {
                            // relativePosition이 오른쪽 면에 위치할 경우
                            normal = axisX;
                        }
                        else {

                        }


                        // 미끄러짐 벡터 계산
                        const DirectX::XMVECTOR velocityVec = DirectX::XMLoadFloat3(&velocity->m_velocity);
                        const DirectX::XMVECTOR dotProduct = DirectX::XMVector3Dot(velocityVec, normal);
                        const DirectX::XMVECTOR slidingVector = DirectX::XMVectorSubtract(velocityVec, DirectX::XMVectorMultiply(normal, dotProduct));
                        // 속도 및 위치 업데이트
                        DirectX::XMStoreFloat3(&velocity->m_velocity, slidingVector);
                        cout << velocity->m_velocity.x << " " << velocity->m_velocity.y << " " << velocity->m_velocity.z << endl;
                    }
                        
                });
    cout << endl << "final: ";
            cout << velocity->m_velocity.x <<" " << velocity->m_velocity.y << " " << velocity->m_velocity.z <<endl;
        });
    



    //velocity 적용
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
