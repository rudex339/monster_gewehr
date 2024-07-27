#include "stdafx.h"
#include "Collision_Sysytem.h"
#include "Object_Entity.h"
#include "Scene_Sysytem.h"


float DistancePointToPlane(const XMVECTOR& planeNormal, const XMVECTOR& planePoint, const XMVECTOR& point) {
    // Plane equation: Ax + By + Cz + D = 0, where A, B, C are the components of the normal vector
    float D = -XMVectorGetX(XMVector3Dot(planeNormal, planePoint));
    return XMVectorGetX(XMVector3Dot(planeNormal, point)) + D;
}

// Helper function to compute the orthogonal vector from a point to a plane
XMVECTOR OrthogonalVectorToPlane(const XMVECTOR& planeNormal, const XMVECTOR& planePoint, const XMVECTOR& point) {
    float distance = DistancePointToPlane(planeNormal, planePoint, point);
    return XMVectorScale(planeNormal, distance);
}



void Collision_Sysytem::configure(World* world)
{
    world->subscribe<AddObjectlayer_Event>(this);
    world->subscribe<Clearlayer_Event>(this);
    world->subscribe<delObjectlayer_Event>(this);
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
                            //Granade->get<Scale_Component>()->mx = 5.f;
                            //Granade->get<Scale_Component>()->my = 5.f;
                            //Granade->get<Scale_Component>()->mz = 100.f;
                            granade->Boom = true;
                            Granade->get<Velocity_Component>()->gravity = false;
                            Granade->get<Velocity_Component>()->m_velocity = XMFLOAT3(0.f, 0.f, 0.f);

                            world->emit<CreateObject_Event>({ explotion,Granade->get<Position_Component>()->Position
                                ,XMFLOAT3(0.f,0.f,0.f),XMFLOAT3(0.f,0.f,0.f) });
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
                            world->emit<CreateObject_Event>({ explotion,Granade->get<Position_Component>()->Position
                                ,XMFLOAT3(0.f,0.f,0.f),XMFLOAT3(0.f,0.f,0.f) });
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
                for (auto object_it = layers["Object"].begin(); object_it != layers["Object"].end(); object_it++){
                    Entity* object = *object_it;
                    ComponentHandle<BoundingBox_Component> Another_boundingBox = object->get<BoundingBox_Component>();

                    boundingBox->m_bounding_box.Center = Vector3::Add(cur_center,velocity->m_velocity);

                    if (boundingBox->m_bounding_box.Intersects(Another_boundingBox->m_bounding_box) || Another_boundingBox->m_bounding_box.Intersects(boundingBox->m_bounding_box)) {
                        BoundingOrientedBox boxA = boundingBox->m_bounding_box;
                        BoundingOrientedBox boxB = Another_boundingBox->m_bounding_box;
                        
                        XMFLOAT3 corners[8];
                        boxA.GetCorners(corners);

                        XMFLOAT3 min1 = corners[0];
                        XMFLOAT3 max1 = corners[0];

                        // 각 꼭짓점을 순회하면서 최소값 및 최대값 업데이트
                        for (int i = 1; i < 8; ++i) {
                            min1.x = std::min<float>(min1.x, corners[i].x);
                            min1.y = std::min<float>(min1.y, corners[i].y);
                            min1.z = std::min<float>(min1.z, corners[i].z);
                                               
                            max1.x = std::max<float>(max1.x, corners[i].x);
                            max1.y = std::max<float>(max1.y, corners[i].y);
                            max1.z = std::max<float>(max1.z, corners[i].z);
                        }

                        boxB.GetCorners(corners);

                        XMFLOAT3 min2 = corners[0];
                        XMFLOAT3 max2 = corners[0];

                        // 각 꼭짓점을 순회하면서 최소값 및 최대값 업데이트
                        for (int i = 1; i < 8; ++i) {
                            min2.x = std::min<float>(min2.x, corners[i].x);
                            min2.y = std::min<float>(min2.y, corners[i].y);
                            min2.z = std::min<float>(min2.z, corners[i].z);
                                                       
                            max2.x = std::max<float>(max2.x, corners[i].x);
                            max2.y = std::max<float>(max2.y, corners[i].y);
                            max2.z = std::max<float>(max2.z, corners[i].z);
                        }

                        float distance = 0.0f;
                        // 충돌면은 미니맵에서 보이는 방향 기준
                        // 물체의 왼쪽면과 충돌
                        if (max1.z >= min2.z && min1.z < min2.z) {
                            distance = max1.z - min2.z;
                            position->Position.z -= distance;
                        }

                        // 물체의 오른쪽 면과 충돌
                        else if (min1.z <= max2.z && max1.z > max2.z) {
                            distance = max2.z - min1.z;
                            position->Position.z += distance;
                        }

                        // 물체의 위쪽과 충돌
                        else if (max1.x >= min2.x && min1.x < min2.x) {
                            distance = max1.x - min2.x;
                            position->Position.x -= distance;
                        }

                        // 물체의 아래쪽과 충돌
                        else if (min1.x <= max2.x && max1.x > max2.x) {
                            distance = max2.x - min1.x;
                            position->Position.x -= distance;
                        }

                    }
                    else {
                        
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

void Collision_Sysytem::receive(World* world, const delObjectlayer_Event& event)
{
    std::remove(layers[event.layer].begin(), layers[event.layer].end(), event.ent);
}
