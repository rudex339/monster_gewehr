#pragma once
class Animate_System : public EntitySystem
{
private:
public:

	Animate_System() = default;
	virtual void configure(class World* world);
	virtual void unconfigure(class World* world);
	virtual void tick(class World* world, float deltaTime);
};