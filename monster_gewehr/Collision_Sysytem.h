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


class Collision_Sysytem : public EntitySystem,
	public EventSubscriber<AddObjectlayer_Event>,
	public EventSubscriber<Clearlayer_Event>,
	public EventSubscriber<delObjectlayer_Event>
{
private:
	unordered_map<string, vector<Entity*>> layers;
public:
	Collision_Sysytem() = default;
	virtual void configure(class World* world);
	virtual void unconfigure(class World* world) {};
	virtual void tick(class World* world, float deltaTime);

	virtual void receive(class World* world, const AddObjectlayer_Event& event);
	virtual void receive(class World* world, const Clearlayer_Event& event);
	virtual void receive(class World* world, const delObjectlayer_Event& event);
};

