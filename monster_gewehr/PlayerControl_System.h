#pragma once
struct CaptureHWND_Event {
	bool capture;
	bool rbd;
};

struct CursorPos_Event {
	POINT m_CursorPos;
};

struct GetPlayerPtr_Event {
	Entity* Pawn;
	bool enable = true;
};

struct PlayerController_Angle {
	XMFLOAT3 m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3 m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
};

class PlayerControl_System : public EntitySystem,
	public EventSubscriber<CaptureHWND_Event>,
	public EventSubscriber<CursorPos_Event>,
	public EventSubscriber<GetPlayerPtr_Event>
{
private:
	Entity* m_Pawn;
	POINT m_OldCursorPos;
	bool Capture = false;
	bool R_btn = false;

	int weapon_ammo[3] = {30, 12, 7};
	int weapon_mag[3] = {120, 60, 21};

	float roll_timer = 0; // 구르기 하는 시간(?)
	short roll_on = 0; // 구르기상태가 어떤지 0 일때는 안구르고 1일때는 방금 구르기 버튼 눌러서 방향 정해주는거, 2일때는 구르기 중임

	float shot_cooltime = 0;
	float shot_cooltime_list[3] = { 0.1f, 0.5f, 1.0f };

	// 힐 하는 시간
	float heal_timer = 0;
	bool heal_on = false;

	// 힐 하는 시간 정의 0 : 붕대, 1 : 구급상자, 2 : 리젝트
	float healtime[3] = { 10.0f, 20.0f, 2.0f };
	float heal_amount[3] = { 25.f, 100.f, 50.f };

	int heal_type; // 0, 1, 2

public:
	PlayerControl_System() = default;

	virtual void configure(class World* world);
	virtual void unconfigure(class World* world);
	virtual void tick(class World* world, float deltaTime);

	virtual void receive(class World* world, const CaptureHWND_Event& event);
	virtual void receive(class World* world, const CursorPos_Event& event);
	virtual void receive(class World* world, const GetPlayerPtr_Event& event);
};

