#include "stdafx.h"
#include "move_System.h"
#include "Object_Entity.h"
#include "Player_Entity.h"
#include "Sever_Sysyem.h"

float CalculateDistanceToPlane(const DirectX::XMVECTOR& point, const DirectX::XMVECTOR& planePoint, const DirectX::XMVECTOR& planeNormal) {
    // 평면의 법선 벡터 (a, b, c)
    float a = DirectX::XMVectorGetX(planeNormal);
    float b = DirectX::XMVectorGetY(planeNormal);
    float c = DirectX::XMVectorGetZ(planeNormal);

    // 평면 위의 한 점 (x0, y0, z0)
    float x0 = DirectX::XMVectorGetX(planePoint);
    float y0 = DirectX::XMVectorGetY(planePoint);
    float z0 = DirectX::XMVectorGetZ(planePoint);

    // 평면 방정식의 d 계산
    float d = -(a * x0 + b * y0 + c * z0);

    // 점 (x1, y1, z1)
    float x1 = DirectX::XMVectorGetX(point);
    float y1 = DirectX::XMVectorGetY(point);
    float z1 = DirectX::XMVectorGetZ(point);

    // 점과 평면 사이의 거리 계산
    float distance = fabsf(a * x1 + b * y1 + c * z1 + d) / sqrtf(a * a + b * b + c * c);

    return distance;
}

void DetermineNormal(const DirectX::XMVECTOR& relativePosition,
    const DirectX::XMVECTOR& axisX,
    const DirectX::XMVECTOR& axisY,
    const DirectX::XMVECTOR& axisZ,
    const DirectX::XMVECTOR& upSide,
    const DirectX::XMVECTOR& underSide,
    const DirectX::XMVECTOR& frontSide,
    const DirectX::XMVECTOR& backSide,
    const DirectX::XMVECTOR& leftSide,
    const DirectX::XMVECTOR& rightSide,
    DirectX::XMVECTOR& normal) {

    // 각 축에 대한 도트 프로덕트의 절대값 계산
    float dotUp = XMVectorGetX(DirectX::XMVector3Dot(relativePosition, upSide));
    float dotDown = XMVectorGetX(DirectX::XMVector3Dot(relativePosition, underSide));
    float dotFront = XMVectorGetX(DirectX::XMVector3Dot(relativePosition, frontSide));
    float dotBack = XMVectorGetX(DirectX::XMVector3Dot(relativePosition, backSide));
    float dotLeft = XMVectorGetX(DirectX::XMVector3Dot(relativePosition, leftSide));
    float dotRight = XMVectorGetX(DirectX::XMVector3Dot(relativePosition, rightSide));

    float disUp = (dotUp > 0.f) ? CalculateDistanceToPlane(relativePosition, upSide, axisY) : FLT_MAX;
    float disDown = (dotDown > 0.f) ? CalculateDistanceToPlane(relativePosition, underSide, axisY) : FLT_MAX;
    float disFront = (dotFront > 0.f) ? CalculateDistanceToPlane(relativePosition, frontSide, axisZ) : FLT_MAX;
    float disBack = (dotBack > 0.f) ? CalculateDistanceToPlane(relativePosition, backSide, axisZ) : FLT_MAX;
    float disLeft = (dotLeft > 0.f) ? CalculateDistanceToPlane(relativePosition, leftSide, axisX) : FLT_MAX;
    float disRight = (dotRight > 0.f) ? CalculateDistanceToPlane(relativePosition, rightSide, axisX) : FLT_MAX;

    //float minDot = ;
    float minDot = FLT_MAX;
    normal = axisY;
    
    if (disUp < minDot) {
        minDot = disUp;
        normal = axisY;
        std::cout << "Collided with the up side." << std::endl;
    }
    if (disDown < minDot) {
        minDot = disDown;
        normal = DirectX::XMVectorNegate(axisY);
        std::cout << "Collided with the down side." << std::endl;
    }
    if (disFront < minDot) {
        minDot = disFront;
        normal = axisZ;
        std::cout << "Collided with the front side." << std::endl;
    }
    if (disBack < minDot) {
        minDot = disBack;
        normal = DirectX::XMVectorNegate(axisZ);
        std::cout << "Collided with the back side." << std::endl;
    }
    if (disLeft < minDot) {
        minDot = disLeft;
        normal = DirectX::XMVectorNegate(axisX);
        std::cout << "Collided with the left side." << std::endl;
    }
    if (disRight < minDot) {
        minDot = disRight;
        normal = axisX;
        std::cout << "Collided with the right side." << std::endl;
    }
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
            XMFLOAT3 next_Center = BBox->m_bounding_box.Center;
            next_Center.x += velocity->m_velocity.x;
            next_Center.y += velocity->m_velocity.y;
            next_Center.z += velocity->m_velocity.z;
           
            world->each<BoundingBox_Component>([&](Entity* another,
            ComponentHandle<BoundingBox_Component> another_BBox) {
                    if (ent == another)return; 
                    BBox->m_bounding_box.Center = next_Center;
                    if (another_BBox->m_bounding_box.Intersects(BBox->m_bounding_box)) {
                        BBox->m_bounding_box.Center = cur_Center;
                        cout << ent->get<Model_Component>()->model_name;
                        cout << " hit " << another->get<Model_Component>()->model_name << endl;



                        const DirectX::XMVECTOR centerA = DirectX::XMLoadFloat3(&BBox->m_bounding_box.Center);
                        const DirectX::XMVECTOR centerB = DirectX::XMLoadFloat3(&another_BBox->m_bounding_box.Center);
                        const DirectX::XMVECTOR relativePosition = (DirectX::XMVectorSubtract(centerA, centerB));

                        // 박스 A의 회전 쿼터니언
                        //const DirectX::XMVECTOR orientationA = DirectX::XMLoadFloat4(&another_BBox->m_bounding_box.Orientation);

                        DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&another_BBox->m_bounding_box.Orientation));

                        XMVECTOR    axisX = rotationMatrix.r[0];
                        XMVECTOR axisY = rotationMatrix.r[1];
                        XMVECTOR axisZ = rotationMatrix.r[2];
                        DirectX::XMVECTOR upSide = axisZ;      // 위쪽
                        DirectX::XMVECTOR underSide = DirectX::XMVectorNegate(axisZ);  // 아래쪽
                        DirectX::XMVECTOR frontSide = axisY;   // 앞쪽
                        DirectX::XMVECTOR backSide = DirectX::XMVectorNegate(axisY);   // 뒤쪽
                        DirectX::XMVECTOR leftSide = DirectX::XMVectorNegate(axisX);   // 왼쪽
                        DirectX::XMVECTOR rightSide = axisX;   // 오른쪽

                        upSide = XMVectorSetW(upSide, another_BBox->m_bounding_box.Extents.y);      // 위쪽
                        underSide = XMVectorSetW(underSide, another_BBox->m_bounding_box.Extents.y);  // 아래쪽
                        frontSide = XMVectorSetW(frontSide, another_BBox->m_bounding_box.Extents.z);   // 앞쪽
                        backSide = XMVectorSetW(backSide, another_BBox->m_bounding_box.Extents.z);   // 뒤쪽
                        leftSide = XMVectorSetW(leftSide, another_BBox->m_bounding_box.Extents.x);   // 왼쪽
                        rightSide = XMVectorSetW(rightSide, another_BBox->m_bounding_box.Extents.x);   // 오른쪽

                        DirectX::XMVECTOR normal[3];
                        
                        //DetermineNormal(relativePosition, axisX, axisY, axisZ, upSide, underSide, frontSide, backSide, leftSide, rightSide, normal);
                        if (BBox->m_bounding_box.Intersects(upSide) == FRONT) {
                            normal[0] = axisZ;
                            std::cout << "Collided with the up side." << std::endl;
                        }
                        if (BBox->m_bounding_box.Intersects(underSide) == FRONT) {
                            normal[0] = DirectX::XMVectorNegate(axisZ);
                            std::cout << "Collided with the under side." << std::endl;
                        }

                        if (BBox->m_bounding_box.Intersects(frontSide) == FRONT) {
                            normal[1] = axisY;
                            std::cout << "Collided with the front side." << std::endl;
                        }
                        if (BBox->m_bounding_box.Intersects(backSide) == FRONT) {
                            normal[1] = DirectX::XMVectorNegate(axisY);
                            std::cout << "Collided with the back side." << std::endl;
                        }

                        if (BBox->m_bounding_box.Intersects(leftSide) == FRONT) {
                            normal[2] = DirectX::XMVectorNegate(axisX);
                            std::cout << "Collided with the left side." << std::endl;
                        }
                        if (BBox->m_bounding_box.Intersects(rightSide) == FRONT) {
                            normal[2] = axisX;
                            std::cout << "Collided with the right side." << std::endl;
                        }

                        /*if (XMVector4EqualInt(DirectX::XMVectorEqual(normal, axisY), XMVectorTrueInt())) {
                            std::cout << "nothing." << std::endl;
                        }*/

                        // 미끄러짐 벡터 계산
                        XMFLOAT3 tempVel[3];
                        for (int i = 0; i < 3; i++) {
                            const DirectX::XMVECTOR velocityVec = DirectX::XMLoadFloat3(&velocity->m_velocity);
                            const DirectX::XMVECTOR dotProduct = DirectX::XMVector3Dot(velocityVec, normal[i]);
                            const DirectX::XMVECTOR slidingVector = DirectX::XMVectorSubtract(velocityVec, DirectX::XMVectorMultiply(normal[i], dotProduct));
                            DirectX::XMStoreFloat3(&tempVel[i], slidingVector);
                        }
                        // 속도 및 위치 업데이트
                        velocity->m_velocity.x= tempVel[0].x+ tempVel[1].x+ tempVel[2].x;
                        velocity->m_velocity.y = tempVel[0].y + tempVel[1].y + tempVel[2].y;
                        velocity->m_velocity.z = tempVel[0].z + tempVel[1].z + tempVel[2].z;
                        //cout << velocity->m_velocity.x << " " << velocity->m_velocity.y << " " << velocity->m_velocity.z << endl;
                    }
                        
                });
            BBox->m_bounding_box.Center = cur_Center;
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
