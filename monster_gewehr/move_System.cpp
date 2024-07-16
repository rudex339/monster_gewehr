#include "stdafx.h"
#include "move_System.h"
#include "Object_Entity.h"
#include "Player_Entity.h"
#include "Sever_Sysyem.h"

float AngleBetweenVectors(XMVECTOR v1, XMVECTOR v2) {
    // ������ ���
    float dotProduct = XMVectorGetX(XMVector3Dot(v1, v2));
    // ������ ũ�⸦ ���
    float len1 = XMVectorGetX(XMVector3Length(v1));
    float len2 = XMVectorGetX(XMVector3Length(v2));
    // �ڻ��� ���� ���
    float cosTheta = dotProduct / (len1 * len2);
    // ��ũ �ڻ����� ����Ͽ� ������ ���
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
    //���� �ݸ����� �ι� �ݺ����� �ʵ�������. �׷��ٸ� �÷��̾
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
                        const DirectX::XMVECTOR direction = (DirectX::XMVectorSubtract(centerA, centerB));

                        // �ڽ� A�� ȸ�� ���ʹϾ�
                        //const DirectX::XMVECTOR orientationA = DirectX::XMLoadFloat4(&another_BBox->m_bounding_box.Orientation);

                        DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&another_BBox->m_bounding_box.Orientation));

                        XMVECTOR    axisX = rotationMatrix.r[0];
                        XMVECTOR axisY = rotationMatrix.r[1];
                        XMVECTOR axisZ = rotationMatrix.r[2];
                        DirectX::XMVECTOR upSide = axisZ;      // ����
                        DirectX::XMVECTOR underSide = DirectX::XMVectorNegate(axisZ);  // �Ʒ���
                        DirectX::XMVECTOR frontSide = axisY;   // ����
                        DirectX::XMVECTOR backSide = DirectX::XMVectorNegate(axisY);   // ����
                        DirectX::XMVECTOR leftSide = DirectX::XMVectorNegate(axisX);   // ����
                        DirectX::XMVECTOR rightSide = axisX;   // ������

                        upSide = XMVectorSetW(upSide, another_BBox->m_bounding_box.Extents.y);      // ����
                        underSide = XMVectorSetW(underSide, another_BBox->m_bounding_box.Extents.y);  // �Ʒ���
                        frontSide = XMVectorSetW(frontSide, another_BBox->m_bounding_box.Extents.z);   // ����
                        backSide = XMVectorSetW(backSide, another_BBox->m_bounding_box.Extents.z);   // ����
                        leftSide = XMVectorSetW(leftSide, another_BBox->m_bounding_box.Extents.x);   // ����
                        rightSide = XMVectorSetW(rightSide, another_BBox->m_bounding_box.Extents.x);   // ������

                        DirectX::XMVECTOR normal= axisY;
                        
                        float angleX = AngleBetweenVectors(direction, axisX);
                        float angleY = AngleBetweenVectors(direction, axisY);
                        float angleZ = AngleBetweenVectors(direction, axisZ);

                        // ���� ���� ������ ã��
                        XMFLOAT3 corners[8];
                        another_BBox->m_bounding_box.GetCorners(corners);
                        XMVECTOR Vcorners[8];
                        for (int i = 0; i < 8; i++) {
                            Vcorners[i] = DirectX::XMLoadFloat3(&corners[i]);
                        }
                        //float minAngle = std::min<float>({ angleX, angleY, angleZ });
                        {
                            /*if (BBox->m_bounding_box.Intersects(Vcorners[2], Vcorners[3], Vcorners[7]) ||
                                BBox->m_bounding_box.Intersects(Vcorners[2], Vcorners[7], Vcorners[6])) {
                                normal = axisY;
                                std::cout << "Collided with the up side." << std::endl;
                            }
                            else if (BBox->m_bounding_box.Intersects(Vcorners[0], Vcorners[1], Vcorners[5]) ||
                                BBox->m_bounding_box.Intersects(Vcorners[0], Vcorners[5], Vcorners[4])) {
                                normal = DirectX::XMVectorNegate(axisY);
                                std::cout << "Collided with the under side." << std::endl;
                            }*/
                            if (BBox->m_bounding_box.Intersects(Vcorners[0], Vcorners[1], Vcorners[2]) ||
                                BBox->m_bounding_box.Intersects(Vcorners[0], Vcorners[2], Vcorners[3])) {
                                normal = axisZ;
                                std::cout << "Collided with the front side." << std::endl;
                            }
                            else if (BBox->m_bounding_box.Intersects(Vcorners[4], Vcorners[5], Vcorners[6]) ||
                                BBox->m_bounding_box.Intersects(Vcorners[4], Vcorners[6], Vcorners[7])) {
                                normal = DirectX::XMVectorNegate(axisZ);
                                std::cout << "Collided with the back side." << std::endl;
                            }
                            /*else if (BBox->m_bounding_box.Intersects(Vcorners[0], Vcorners[3], Vcorners[7]) ||
                                BBox->m_bounding_box.Intersects(Vcorners[0], Vcorners[7], Vcorners[4])) {
                                normal = DirectX::XMVectorNegate(axisX);
                                std::cout << "Collided with the left side." << std::endl;
                            }
                            else if (BBox->m_bounding_box.Intersects(Vcorners[1], Vcorners[2], Vcorners[6]) ||
                                BBox->m_bounding_box.Intersects(Vcorners[1], Vcorners[6], Vcorners[5])) {
                                normal = axisX;
                                std::cout << "Collided with the right side." << std::endl;
                            }*/
                        }
                        

                        

                        


                        // �̲����� ���� ���
                        const DirectX::XMVECTOR velocityVec = DirectX::XMLoadFloat3(&velocity->m_velocity);
                        const DirectX::XMVECTOR dotProduct = DirectX::XMVector3Dot(velocityVec, normal);
                        const DirectX::XMVECTOR slidingVector = DirectX::XMVectorSubtract(velocityVec, DirectX::XMVectorMultiply(normal, dotProduct));
                        // �ӵ� �� ��ġ ������Ʈ
                        DirectX::XMStoreFloat3(&velocity->m_velocity, slidingVector);
                        cout << velocity->m_velocity.x << " " << velocity->m_velocity.y << " " << velocity->m_velocity.z << endl;
                    }
                        
                });
            BBox->m_bounding_box.Center = cur_Center;
    cout << endl << "final: ";
            cout << velocity->m_velocity.x <<" " << velocity->m_velocity.y << " " << velocity->m_velocity.z <<endl;
        });
    



    //velocity ����
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
                                cout << "���ε� �Ϸ�" << endl;
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
