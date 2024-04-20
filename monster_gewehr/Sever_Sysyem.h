#pragma once

struct PacketSend_Event {
	CHAR id;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 vel;
	FLOAT yaw;
	CHAR wepon;
};
class Sever_System : public EntitySystem,
	public EventSubscriber<PacketSend_Event>
{
private:

public:

	Sever_System() = default;
	virtual void configure(class World* world);
	virtual void unconfigure(class World* world) {};
	virtual void tick(class World* world, float deltaTime);
	virtual void receive(class World* world, const PacketSend_Event& event);
};
