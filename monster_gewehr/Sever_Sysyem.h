#pragma once

struct PacketSend_Event {
	CHAR id;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 vel;
	FLOAT yaw;
	UINT State;
	DirectX::XMFLOAT3 c_pos;
	DirectX::XMFLOAT3 c_dir;
	CHAR wepon;
};

struct Login_Event {
	
};

struct Game_Start {

};

class Sever_System : public EntitySystem,
	public EventSubscriber<PacketSend_Event>,
	public EventSubscriber<Login_Event>,
	public EventSubscriber<Game_Start>
{
private:
	bool m_login = false;
	bool m_send = false;
public:

	Sever_System() = default;
	virtual void configure(class World* world);
	virtual void unconfigure(class World* world) {};
	virtual void tick(class World* world, float deltaTime);
	virtual void receive(class World* world, const PacketSend_Event& event);
	virtual void receive(class World* world, const Login_Event& event);
	virtual void receive(class World* world, const Game_Start& event);
	void PacketReassembly(World* world, char* recv_buf, size_t recv_size);
	void ProcessPacket(World* world, char* packet);
};
