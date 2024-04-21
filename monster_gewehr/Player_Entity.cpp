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

ControllAngle_Component::ControllAngle_Component(float R_x, float R_y, float R_z, float L_x, float L_y, float L_z, float U_x, float U_y, float U_z)
{
	m_xmf3Right = XMFLOAT3(R_x, R_y, R_z);
	m_xmf3Look = XMFLOAT3(L_x, L_y, L_z);
	m_xmf3Up = XMFLOAT3(U_x, U_y, U_z);
}
