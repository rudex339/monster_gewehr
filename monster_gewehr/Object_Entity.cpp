#include "stdafx.h"
#include "Object_Entity.h"

Entity* AddAnimationMeshObject(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, 
	CLoadedModelInfo* model, 
	float x, float y, float z, 
	float rx, float ry, float rz, 
	float sx, float sy, float sz,
	int n_animation)
{
	ent->assign<Model_Component>(model, model->m_pModelRootObject->m_pstrFrameName);
	auto controller = ent->assign<AnimationController_Component>(
		new CAnimationController(pd3dDevice, pd3dCommandList, n_animation, model),0);
	for (int i = 0; i < n_animation; i++) {
		controller->m_AnimationController->SetTrackAnimationSet(i, i);
		controller->m_AnimationController->SetTrackEnable(i, false);
	}
	controller->m_AnimationController->SetTrackEnable(0, true);
	ent->assign<Position_Component>(x, y, z);
	ent->assign<Rotation_Component>(rx, ry, rz);
	ent->assign<Scale_Component>(sx, sy, sz);
	return ent;
}

Entity* AddSoldierObject(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	CLoadedModelInfo* model,
	float x, float y, float z,
	float rx, float ry, float rz,
	float sx, float sy, float sz,
	int n_animation)
{
	AddAnimationMeshObject(ent, pd3dDevice, pd3dCommandList, model, x, y, z,rx, ry, rz,	sx, sy, sz,	n_animation);

	return ent;
}



EulerAngle_Component::EulerAngle_Component(float R_x, float R_y, float R_z, float L_x, float L_y, float L_z, float U_x, float U_y, float U_z)
{
	m_xmf3Right = XMFLOAT3(R_x, R_y, R_z);
	m_xmf3Look = XMFLOAT3(L_x, L_y, L_z);
	m_xmf3Up = XMFLOAT3(U_x, U_y, U_z);
}

//----------------------------------------------------------------------------------------------
//UI

TextUI_Component::TextUI_Component(const wchar_t* text, float top, float left, float bottom, float right) {
	m_text = text;
	m_Rect = D2D1::RectF(left, top, right, bottom);
}

TextUI_Component::TextUI_Component(wstring text, float top, float left, float bottom, float right)
{
	m_text = text;
	m_Rect = D2D1::RectF(left, top, right, bottom);
}
