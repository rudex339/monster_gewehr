#include "stdafx.h"
#include "Collision_Sysytem.h"
#include "Object_Entity.h"


float DistancePointToPlane(const XMVECTOR& normal_vector, const XMVECTOR& point_A, const XMVECTOR& point_B) {
    // 평면 위의 점 A에서 점 B까지의 벡터를 계산
    XMVECTOR point_A_to_B = XMVectorSubtract(point_B, point_A);

    // 법선 벡터와 점 A에서 점 B까지의 벡터의 내적을 계산
    float dot_product = XMVectorGetX(XMVector3Dot(normal_vector, point_A_to_B));

    // 법선 벡터의 길이를 계산
    float normal_length = XMVectorGetX(XMVector3Length(normal_vector));

    // 점 B와 평면 사이의 거리 계산
    float distance = std::fabs(dot_product) / normal_length;

    return distance;
}

XMVECTOR OrthogonalVectorToPlane(const XMVECTOR& normal_vector, const XMVECTOR& point_on_plane, const XMVECTOR& point_q) {
    XMVECTOR v = XMVectorSubtract(point_on_plane, point_q);
    return XMVector3Cross(v, normal_vector);
}



void Collision_Sysytem::configure(World* world)
{
    world->subscribe<AddObjectlayer_Event>(this);
    world->subscribe<Clearlayer_Event>(this);
}

void Collision_Sysytem::tick(World* world, float deltaTime)
{
    if (layers.find("Granade") != layers.end()) {
        for (auto Granade_it = layers["Granade"].begin(); Granade_it != layers["Granade"].end(); /* no increment */) {
            Entity* Granade = *Granade_it;
            XMFLOAT3 position = Granade->get<Position_Component>()->Position;
            BoundingOrientedBox boundingBox = Granade->get<BoundingBox_Component>()->m_bounding_box;
            boundingBox.Center = position;
            ComponentHandle<Grande_Component> granade = Granade->get<Grande_Component>();

            bool GranadeDelete = false;

            if (granade->Boom) {
                granade->sphere.Center = position;
                if (layers.find("Monster") != layers.end()) {
                    for (auto object_it = layers["Monster"].begin(); object_it != layers["Monster"].end(); /* no increment */) {
                        Entity* object = *object_it;
                        
                        if (granade->sphere.Intersects(object->get<BoundingBox_Component>()->m_bounding_box)) {

                            cout << "HIT" << endl;
                            GranadeDelete = true;
                            break;
                        }
                        else {
                            ++object_it;
                        }
                    }
                }
            }
            else {
                if (layers.find("Object") != layers.end()) {
                    for (auto object_it = layers["Object"].begin(); object_it != layers["Object"].end(); ++object_it) {
                        Entity* object = *object_it;

                        if (boundingBox.Intersects(object->get<BoundingBox_Component>()->m_bounding_box)) {

                            cout << "Boom" << endl;
                            Granade->get<Scale_Component>()->mx = 5.f;
                            Granade->get<Scale_Component>()->my = 5.f;
                            Granade->get<Scale_Component>()->mz = 100.f;
                            granade->Boom = true;
                            Granade->get<Velocity_Component>()->gravity = false;
                            Granade->get<Velocity_Component>()->m_velocity = XMFLOAT3(0.f, 0.f, 0.f);
                            break;
                        }

                    }
                }
                if (layers.find("Monster") != layers.end()) {
                    for (auto object_it = layers["Monster"].begin(); object_it != layers["Monster"].end(); ++object_it) {
                        Entity* object = *object_it;

                        if (boundingBox.Intersects(object->get<BoundingBox_Component>()->m_bounding_box)) {

                            cout << "Boom" << endl;
                            Granade->get<Scale_Component>()->mx = 200.f;
                            Granade->get<Scale_Component>()->my = 200.f;
                            Granade->get<Scale_Component>()->mz = 200.f;
                            granade->Boom = true;
                            Granade->get<Velocity_Component>()->gravity = false;
                            Granade->get<Velocity_Component>()->m_velocity = XMFLOAT3(0.f, 0.f, 0.f);
                            break;
                        }

                    }
                }
            }
            granade->coolTime -= deltaTime;
            if (granade->coolTime <= 0.f) {
                GranadeDelete = true;
                cout << "coolTime 0" << endl;
            }
            // Remove Granade from the "Granade" layer if it collided
            if (GranadeDelete) {
                Granade_it = layers["Granade"].erase(Granade_it);
                world->destroy(Granade);
                cout << "destroy" << endl;
            }
            else {
                
                ++Granade_it;
            }
        }
    }

    if (layers.find("Player") != layers.end()) {

        for (auto Player_it = layers["Player"].begin();  Player_it != layers["Player"].end(); Player_it++) {
            Entity* Player = *Player_it;
            ComponentHandle<Position_Component> position= Player->get<Position_Component>();
            ComponentHandle<BoundingBox_Component> boundingBox = Player->get<BoundingBox_Component>();
            ComponentHandle<Velocity_Component> velocity = Player->get<Velocity_Component>();
            
            XMFLOAT3 cur_center = boundingBox->m_bounding_box.Center;
            if (layers.find("Object") != layers.end()) {
                for (auto object_it = layers["Object"].begin(); object_it != layers["Object"].end();  object_it++){
                    Entity* object = *object_it;
                    ComponentHandle<BoundingBox_Component> Another_boundingBox = object->get<BoundingBox_Component>();

                    boundingBox->m_bounding_box.Center = Vector3::Add(cur_center,velocity->m_velocity);

                    if (boundingBox->m_bounding_box.Intersects(Another_boundingBox->m_bounding_box)) {
                        cout << Player->get<Model_Component>()->model_name;
                        cout << " hit " << object->get<Model_Component>()->model_name << endl;
                        XMFLOAT3 Corners[8];
                        boundingBox->m_bounding_box.GetCorners(Corners);
                        XMVECTOR vExtents = XMLoadFloat3(&boundingBox->m_bounding_box.Extents);

                        
                       // DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion();
                        XMVECTOR vOrientation = DirectX::XMLoadFloat4(&Another_boundingBox->m_bounding_box.Orientation);
                        assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));
                        

                        XMVECTOR axisX = XMVector3Rotate(XMVectorSet(1.0f, 0.f, 0.f, 0.f), vOrientation);
                        XMVECTOR axisY = XMVector3Rotate(XMVectorSet(0.0f, 1.f, 0.f, 0.f), vOrientation);
                        XMVECTOR axisZ = XMVector3Rotate(XMVectorSet(0.0f, 0.f, 1.f, 0.f), vOrientation);
                        XMVECTOR axismX = XMVector3Rotate(XMVectorSet(-1.0f, 0.f, 0.f, 0.f), vOrientation);
                        XMVECTOR axismY = XMVector3Rotate(XMVectorSet(0.0f, -1.f, 0.f, 0.f), vOrientation);
                        XMVECTOR axismZ = XMVector3Rotate(XMVectorSet(0.0f, 0.f, -1.f, 0.f), vOrientation);
                        
                        DirectX::XMVECTOR upSide = XMVector3Rotate(XMVectorMultiply(vExtents, XMVectorSet(0.0f, 1.f, 0.f, 0.f)), vOrientation);     // ����
                        DirectX::XMVECTOR underSide = XMVector3Rotate(XMVectorMultiply(vExtents, XMVectorSet(0.0f, -1.f, 0.f, 0.f)), vOrientation);  // �Ʒ���
                        DirectX::XMVECTOR frontSide = XMVector3Rotate(XMVectorMultiply(vExtents, XMVectorSet(0.0f, 0.f, 1.f, 0.f)), vOrientation); // ����
                        DirectX::XMVECTOR backSide = XMVector3Rotate(XMVectorMultiply(vExtents, XMVectorSet(0.0f, 0.f, -1.f, 0.f)), vOrientation);   // ����
                        DirectX::XMVECTOR leftSide = XMVector3Rotate(XMVectorMultiply(vExtents, XMVectorSet(-1.0f, 0.f, 0.f, 0.f)), vOrientation);   // ����
                        DirectX::XMVECTOR rightSide = XMVector3Rotate(XMVectorMultiply(vExtents, XMVectorSet(1.0f, 0.f, 0.f, 0.f)), vOrientation);   // ������

                        vector<pair<float, XMVECTOR>> vectorList1;
                        vector<pair<float, XMVECTOR>> vectorList2;
                        vector<pair<float, XMVECTOR>> vectorList3;
                        vector<pair<float, XMVECTOR>> vectorList4;
                        vector<pair<float, XMVECTOR>> vectorList5;
                        vector<pair<float, XMVECTOR>> vectorList6;
                        for(int i = 0;i<8;i++){
                            XMVECTOR point = XMLoadFloat3(&Corners[i]);
                            if (Another_boundingBox->m_bounding_box.Contains(point)) {
                                cout << "corner hit" << endl;
                                point = XMLoadFloat3(&Vector3::Subtract(Corners[i], Another_boundingBox->m_bounding_box.Center));

                                {                                    
                                    float distance = DistancePointToPlane(axisY,upSide, point);
                                    XMVECTOR orthogonal_vector = OrthogonalVectorToPlane(axisY, upSide, point);
                                    vectorList1.push_back(make_pair(distance, orthogonal_vector));
                                }
                                // underSide
                                {
                                    float distance = DistancePointToPlane(axismY, underSide, point);
                                    XMVECTOR orthogonal_vector = OrthogonalVectorToPlane(axismY, underSide, point);
                                    vectorList2.push_back(make_pair(distance, orthogonal_vector));
                                }
                                // frontSide
                                {
                                    float distance = DistancePointToPlane(axisZ, frontSide, point);
                                    XMVECTOR orthogonal_vector = OrthogonalVectorToPlane(axisZ, frontSide, point);
                                    vectorList3.push_back(make_pair(distance, orthogonal_vector));
                                }
                                // backSide
                                {
                                    float distance = DistancePointToPlane(axismZ, backSide, point);
                                    XMVECTOR orthogonal_vector = OrthogonalVectorToPlane(axismZ, backSide, point);
                                    vectorList4.push_back(make_pair(distance, orthogonal_vector));
                                }
                                // leftSide
                                {
                                    float distance = DistancePointToPlane(axismX, leftSide, point);
                                    XMVECTOR orthogonal_vector = OrthogonalVectorToPlane(axismX, leftSide, point);
                                    vectorList5.push_back(make_pair(distance, orthogonal_vector));
                                }
                                // rightSide
                                {
                                    float distance = DistancePointToPlane(axisX, rightSide, point);
                                    XMVECTOR orthogonal_vector = OrthogonalVectorToPlane(axisX, rightSide, point);
                                    vectorList6.push_back(make_pair(distance, orthogonal_vector));
                                }

                            }
                        }
                        
                        auto maxPairIter1 = max_element(vectorList1.begin(), vectorList1.end(),
                            [](const pair<float, XMVECTOR>& a, const pair<float, XMVECTOR>& b) {
                                return a.first > b.first;       
                            });
                        auto maxPairIter2 = max_element(vectorList2.begin(), vectorList2.end(),
                            [](const pair<float, XMVECTOR>& a, const pair<float, XMVECTOR>& b) {
                                return a.first > b.first;
                            });
                        auto maxPairIter3 = max_element(vectorList3.begin(), vectorList3.end(),
                            [](const pair<float, XMVECTOR>& a, const pair<float, XMVECTOR>& b) {
                                return a.first > b.first;
                            });
                        auto maxPairIter4 = max_element(vectorList4.begin(), vectorList4.end(),
                            [](const pair<float, XMVECTOR>& a, const pair<float, XMVECTOR>& b) {
                                return a.first > b.first;
                            });
                        auto maxPairIter5 = max_element(vectorList5.begin(), vectorList5.end(),
                            [](const pair<float, XMVECTOR>& a, const pair<float, XMVECTOR>& b) {
                                return a.first > b.first;
                            });
                        auto maxPairIter6 = max_element(vectorList6.begin(), vectorList6.end(),
                            [](const pair<float, XMVECTOR>& a, const pair<float, XMVECTOR>& b) {
                                return a.first > b.first;
                            });
                        float max_distance = FLT_MAX;
                        XMVECTOR v1 = XMVectorSet(0.f,0.f,0.f,0.f);
                        {
                            if(maxPairIter1 != vectorList1.end())
                            if (maxPairIter1->first < max_distance) {
                                max_distance = maxPairIter1->first;
                                v1 = maxPairIter1->second;
                                cout << XMVectorGetX(v1) << " " << XMVectorGetY(v1) << " " << XMVectorGetZ(v1) << endl;
                            }

                            if (maxPairIter2 != vectorList2.end())
                            if (maxPairIter2->first < max_distance) {
                                max_distance = maxPairIter2->first;
                                v1 = maxPairIter2->second;
                            }

                            if (maxPairIter3 != vectorList3.end())
                            if (maxPairIter3->first < max_distance) {
                                max_distance = maxPairIter3->first;
                                v1 = maxPairIter3->second;
                            }
                            if (maxPairIter4 != vectorList4.end())
                            if (maxPairIter4->first < max_distance) {
                                max_distance = maxPairIter4->first;
                                v1 = maxPairIter4->second;
                            }

                            if (maxPairIter5 != vectorList5.end())
                            if (maxPairIter5->first < max_distance) {
                                max_distance = maxPairIter5->first;
                                v1 = maxPairIter5->second;
                            }

                            if (maxPairIter6 != vectorList6.end())
                            if (maxPairIter6->first < max_distance) {
                                max_distance = maxPairIter6->first;
                                v1 = maxPairIter6->second;
                            }
                            
                        }
                        if (true) {
                            
                            velocity->m_velocity = Vector3::Add(velocity->m_velocity, Vector3::XMVectorToFloat3(v1));
                        }
                    }
                    
                }
            }

        }
    }
}

void Collision_Sysytem::receive(World* world, const AddObjectlayer_Event& event)
{
    if (layers.find(event.layer) != layers.end()) {
        // If the key exists, add the index to the existing vector
        layers[event.layer].push_back(event.ent);
    }
    else {
        // If the key does not exist, create a new vector and add the index
        layers[event.layer] = vector<Entity*>{ event.ent };
    }
}

void Collision_Sysytem::receive(World* world, const Clearlayer_Event& event)
{
    layers.clear();
}
