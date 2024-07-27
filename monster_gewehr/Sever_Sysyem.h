#pragma once
#include "Scene_Sysytem.h"

struct PacketSend_Event {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 vel;
	FLOAT yaw;
	UINT State;
	CHAR wepon;
};

struct Shoot_Event {
	short hit_count;
};

struct Login_Event {
	string id;
	string password;
};

struct Game_Start {

};

struct Create_Room {

};

struct Join_Room {
	SHORT room_num;
};

struct Quit_Room {

};

struct Select_Room {
	SHORT room_num;
};

struct Ready_Room {
	
};

struct Set_Equipment{
	CHAR weapon;
	CHAR armor;
	CHAR grenade;
};

struct Heal_Event {
	FLOAT hp;
	INT item_type;
};

struct Buy_Item {
	INT money;
	INT item_type;
};

struct ThrowWeapon_Event {
	CHAR type;
	DirectX::XMFLOAT3 pos;
};

struct Demo_Event {
	CHAR type;
};

class Sever_System : public EntitySystem,
	public EventSubscriber<PacketSend_Event>,
	public EventSubscriber<Shoot_Event>,
	public EventSubscriber<Login_Event>,
	public EventSubscriber<Game_Start>,
	public EventSubscriber<Create_Room>,
	public EventSubscriber<Join_Room>,
	public EventSubscriber<Quit_Room>,
	public EventSubscriber<Select_Room>,
	public EventSubscriber<Ready_Room>,
	public EventSubscriber<Set_Equipment>,
	public EventSubscriber<Heal_Event>,
	public EventSubscriber<Buy_Item>,
	public EventSubscriber<ThrowWeapon_Event>,
	public EventSubscriber<Demo_Event>
{
private:
	bool m_login = false;
	bool m_send = false;

	UINT m_id;
	Scene_Sysytem* m_scene;

public:

	Sever_System() = default;
	Sever_System(Scene_Sysytem* scene) { m_scene = scene; };
	virtual void configure(class World* world);
	virtual void unconfigure(class World* world) {};
	virtual void tick(class World* world, float deltaTime);
	virtual void receive(class World* world, const PacketSend_Event& event);
	virtual void receive(class World* world, const Shoot_Event& event);
	virtual void receive(class World* world, const Login_Event& event);
	virtual void receive(class World* world, const Game_Start& event);
	virtual void receive(class World* world, const Create_Room& event);
	virtual void receive(class World* world, const Join_Room& event);
	virtual void receive(class World* world, const Quit_Room& event);
	virtual void receive(class World* world, const Select_Room& event);
	virtual void receive(class World* world, const Ready_Room& event);
	virtual void receive(class World* world, const Set_Equipment& event);
	virtual void receive(class World* world, const Heal_Event& event);
	virtual void receive(class World* world, const Buy_Item& event);
	virtual void receive(class World* world, const ThrowWeapon_Event& event);
	virtual void receive(class World* world, const Demo_Event& event);
	void PacketReassembly(World* world, char* recv_buf, size_t recv_size);
	void ProcessPacket(World* world, char* packet);
};
