#include "stdafx.h"
#include "Player_Entity.h"
#include "Object_Entity.h"
#include "ObjectManager.h"
#include "Camera.h"


Entity* AddPlayerEntity(World* world,Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ObjectManager* OM, float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz, UINT model)
{
	switch (model) {
	case MONSTER:
		AddMonsterObject(world, ent, pd3dDevice, pd3dCommandList,
			OM,
			x, y, z,
			rx, ry, rz,
			sx, sy, sz);
		break;
	case SOLDIER:
		AddSoldierObject(world, ent, pd3dDevice, pd3dCommandList,
			OM,
			x, y, z,
			rx, ry, rz,
			sx, sy, sz);
		break;
	}
	
	ent->assign<Velocity_Component>();
	ent->assign<EulerAngle_Component>();
	ent->assign<ControllAngle_Component>();
	ent->assign<player_Component>();

	return ent;
}

Entity* AddAnotherEntity(World* world, Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ObjectManager* OM, float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz, UINT model)
{
	switch (model) {
	case MONSTER:
		AddMonsterObject(world, ent, pd3dDevice, pd3dCommandList,
			OM,
			x, y, z,
			rx, ry, rz,
			sx, sy, sz);
		break;
	case SOLDIER:
		AddSoldierObject(world, ent, pd3dDevice, pd3dCommandList,
			OM,
			x, y, z,
			rx, ry, rz,
			sx, sy, sz);
		break;
	}
	ent->assign<player_Component>();

	return ent;
}

ControllAngle_Component::ControllAngle_Component(float R_x, float R_y, float R_z, float L_x, float L_y, float L_z, float U_x, float U_y, float U_z)
{
	m_xmf3Right = XMFLOAT3(R_x, R_y, R_z);
	m_xmf3Look = XMFLOAT3(L_x, L_y, L_z);
	m_xmf3Up = XMFLOAT3(U_x, U_y, U_z);
}
