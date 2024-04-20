#include "stdafx.h"
#include "Player_Entity.h"
#include "Object_Entity.h"
#include "Camera.h"


Entity* AddPlayerEntity(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CLoadedModelInfo* model, float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz, int n_animation)
{
	AddAnimationMeshObject(ent, pd3dDevice, pd3dCommandList,
		model,
		x, y, z,
		rx, ry, rz,
		sx, sy, sz,
		n_animation);
	ent->assign<Velocity_Component>();
	ent->assign<EulerAngle_Component>();
	ent->assign<player_Component>();

	return ent;
}
