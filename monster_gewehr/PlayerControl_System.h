#pragma once
struct CaptureHWND_Event {
	bool capture;
};

struct CursorPos_Event {
	POINT m_CursorPos;
};

class PlayerControl_System : public EntitySystem,
	public EventSubscriber<CaptureHWND_Event>,
	public EventSubscriber<CursorPos_Event>
{
private:
	Entity* m_Pawn;
	POINT m_OldCursorPos;
	bool Capture = false;
public:
	PlayerControl_System() = default;
	PlayerControl_System(Entity* Pawn);

	virtual void configure(class World* world);
	virtual void unconfigure(class World* world);
	virtual void tick(class World* world, float deltaTime);

	virtual void receive(class World* world, const CaptureHWND_Event& event);
	virtual void receive(class World* world, const CursorPos_Event& event);
};

