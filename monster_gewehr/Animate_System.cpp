#include "stdafx.h"
#include "Animate_System.h"
#include "Object_Entity.h"
#include "Player_Entity.h"
#include "Scene_Sysytem.h"
#include "Collision_Sysytem.h"

void Animate_System::configure(World* world)
{
}

void Animate_System::unconfigure(World* world)
{
}

void Animate_System::tick(World* world, float deltaTime)
{
	world->each<Model_Component>([&](
		Entity* ent,		
		ComponentHandle<Model_Component> Model
		) -> void {
			if (ent->has<AnimationController_Component>()) {
				ComponentHandle<AnimationController_Component> AnimationController = ent->get<AnimationController_Component>();
				if (AnimationController->next_State != MAXDWORD && AnimationController->next_State != AnimationController->cur_State) {
					AnimationController->m_AnimationController->SetTrackEnable(AnimationController->cur_State, false);
					AnimationController->m_AnimationController->SetTrackEnable(AnimationController->next_State, true);
					AnimationController->cur_State = AnimationController->next_State;
					AnimationController->next_State = MAXDWORD;
				}
			}
			else if (ent->has<Emitter_Componet>()) {
				ComponentHandle<Emitter_Componet> emiiter = ent->get<Emitter_Componet>();
				emiiter->fTime += deltaTime;
				emiiter->coolTime -= deltaTime;
				if (emiiter->coolTime < 0) {
					world->emit<delObjectlayer_Event>({ "Granade",ent});
					world->destroy(ent);
				}
				if (emiiter->fTime >= emiiter->speed) emiiter->fTime = 0.0f;
				if (emiiter->fTime == 0.0f)
				{
					if (++emiiter->m_nCol == emiiter->m_nCols) { emiiter->m_nRow++; emiiter->m_nCol = 0; }
					if (emiiter->m_nRow == emiiter->m_nRows) emiiter->m_nRow = 0;
				}
			}

		});
}
