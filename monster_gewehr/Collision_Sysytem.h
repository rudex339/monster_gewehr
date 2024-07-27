#pragma once
struct AddObjectlayer_Event {
	string layer;
	Entity* ent;
};
struct Clearlayer_Event {

};
struct delObjectlayer_Event {
	string layer;
	Entity* ent;
};
struct ShootGun_Event {
	int weapon_type;
	XMFLOAT3 pos;
	XMFLOAT3 dir;

};


class Collision_Sysytem : public EntitySystem,
	public EventSubscriber<AddObjectlayer_Event>,
	public EventSubscriber<Clearlayer_Event>,
	public EventSubscriber<delObjectlayer_Event>,
	public EventSubscriber<ShootGun_Event>
{
private:
	unordered_map<string, vector<Entity*>> layers;
	uniform_real_distribution<float> urd{ -0.1f, 0.1f };

	float m_shot_range[3] = { 250.0f, 150.0f, 400.0f };
public:
	Collision_Sysytem() = default;
	virtual void configure(class World* world);
	virtual void unconfigure(class World* world) {};
	virtual void tick(class World* world, float deltaTime);

	virtual void receive(class World* world, const AddObjectlayer_Event& event);
	virtual void receive(class World* world, const Clearlayer_Event& event);
	virtual void receive(class World* world, const delObjectlayer_Event& event);
	virtual void receive(class World* world, const ShootGun_Event& event);
};

