#pragma once
#include "ECS.h"
enum {
	LOGIN,
	GAME,
	END
};

struct ChangeScene_Event {
	UINT State;
};

class Scene_Sysytem :
    public EntitySystem,
	public EventSubscriber<ChangeScene_Event>
{
private:
	UINT m_State = 0;
public:
	Scene_Sysytem() = default;

	virtual void configure(class World* world);
	virtual void unconfigure(class World* world);
	virtual void tick(class World* world, float deltaTime);

	virtual void receive(class World* world, const ChangeScene_Event& event);
};

