#include "stdafx.h"
#include "Scene_Sysytem.h"

void Scene_Sysytem::configure(World* world)
{
	world->subscribe<ChangeScene_Event>(this);
}

void Scene_Sysytem::unconfigure(World* world)
{
}

void Scene_Sysytem::tick(World* world, float deltaTime)
{
}

void Scene_Sysytem::receive(World* world, const ChangeScene_Event& event)
{
	world->cleanup();
	
}
