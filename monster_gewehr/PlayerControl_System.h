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

public:
	PlayerControl_System() = default;

	virtual void configure(class World* world);
	virtual void unconfigure(class World* world);
	virtual void tick(class World* world, float deltaTime);

	virtual void receive(class World* world, const CaptureHWND_Event& event);
	virtual void receive(class World* world, const CursorPos_Event& event);
	virtual void receive(class World* world, const GetPlayerPtr_Event& event);
};

