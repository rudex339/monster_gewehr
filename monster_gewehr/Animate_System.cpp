#include "stdafx.h"
#include "Animate_System.h"
#include "Object_Entity.h"
#include "Player_Entity.h"

void Animate_System::configure(World* world)
{
}

void Animate_System::unconfigure(World* world)
{
}

void Animate_System::tick(World* world, float deltaTime)
{
	world->each<AnimationController_Component, Model_Component>([&](
		Entity* ent,
		ComponentHandle<AnimationController_Component> AnimationController,
		ComponentHandle<Model_Component> Model
		) -> void {
			if (AnimationController->next_State != MAXDWORD && AnimationController->next_State != AnimationController->cur_State) {
				AnimationController->m_AnimationController->SetTrackEnable(AnimationController->cur_State, false);
				AnimationController->m_AnimationController->SetTrackEnable(AnimationController->next_State, true);
				AnimationController->cur_State = AnimationController->next_State;
				AnimationController->next_State = MAXDWORD;
			}

		});
}
