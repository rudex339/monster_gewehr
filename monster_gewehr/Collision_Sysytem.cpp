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

                    if (boundingBox->m_bounding_box.Intersects(Another_boundingBox->m_bounding_box)) {
                        cout << Player->get<Model_Component>()->model_name;
                        cout << " hit " << object->get<Model_Component>()->model_name << endl;
                        
                        
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
                        
                        XMVECTOR normals[6];
                        normals[0] = XMVector3Rotate(XMVectorMultiply(vExtents, XMVectorSet(0.0f, 1.f, 0.f, 0.f)), vOrientation);     // ����
                        normals[1] = XMVector3Rotate(XMVectorMultiply(vExtents, XMVectorSet(0.0f, -1.f, 0.f, 0.f)), vOrientation);  // �Ʒ���
                        normals[2] = XMVector3Rotate(XMVectorMultiply(vExtents, XMVectorSet(0.0f, 0.f, 1.f, 0.f)), vOrientation); // ����
                        normals[3] = XMVector3Rotate(XMVectorMultiply(vExtents, XMVectorSet(0.0f, 0.f, -1.f, 0.f)), vOrientation);   // ����
                        normals[4] = XMVector3Rotate(XMVectorMultiply(vExtents, XMVectorSet(-1.0f, 0.f, 0.f, 0.f)), vOrientation);   // ����
                        normals[5] = XMVector3Rotate(XMVectorMultiply(vExtents, XMVectorSet(1.0f, 0.f, 0.f, 0.f)), vOrientation);   // ������

                        float minDepth = FLT_MAX;
                        XMVECTOR minNormal;

                        for (int i = 0; i < 6; ++i) {
                            float depth = (DistancePointToPlane(normals[i], normals[i],XMLoadFloat3(&Vector3::Subtract(boundingBox->m_bounding_box.Center, Another_boundingBox->m_bounding_box.Center))));
                            if (depth < minDepth&&depth>0) {
                                minDepth = depth;
                                minNormal = normals[i];
                            }
                        }

                        XMFLOAT3 Corners[8];
                        boundingBox->m_bounding_box.GetCorners(Corners);
                        for (int i = 0; i < 8; i++) {
                            Corners[i] = Vector3::Subtract(Corners[i], Another_boundingBox->m_bounding_box.Center);
                        }

                        minDepth = FLT_MAX;
                        XMVECTOR vNormal;
                        for (int i = 0; i < 8; i++) {
                            float depth = DistancePointToPlane(minNormal, minNormal, XMLoadFloat3(&Corners[i]));
                            if (depth < minDepth && depth < 0) {
                                minDepth = depth;
                                vNormal = OrthogonalVectorToPlane(minNormal, minNormal, XMLoadFloat3(&Corners[i]));

                            }
                        }

                        vNormal = XMVectorScale(XMVector3Normalize(minNormal), 50.25f * deltaTime);

                        if (true) {
                            
                            velocity->m_velocity = Vector3::Add(velocity->m_velocity, Vector3::XMVectorToFloat3(vNormal));
                        }
                        //object_it = layers["Object"].begin();
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
