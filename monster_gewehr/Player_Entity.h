#pragma once
class CLoadedModelInfo;

enum Player_State {
	IDLE,
	RUN,
	SHOOT,
	HEAL,
	HIT
};



Entity* AddPlayerEntity(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	CLoadedModelInfo* model,
	float x, float y, float z,
	float rx, float ry, float rz,
	float sx, float sy, float sz,
	int n_animation);

Entity* AddAnotherEntity(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, 
	CLoadedModelInfo* model,
	float x, float y, float z, 
	float rx, float ry, float rz, 
	float sx, float sy, float sz, 
	int n_animation);

struct player_Component {

	INT id = -1;
	FLOAT hp = 100;
	FLOAT max_hp = 100;

	FLOAT ammo = 30;

	player_Component() = default;
};



struct ControllAngle_Component {
	XMFLOAT3 m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3 m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	ControllAngle_Component() = default;
	ControllAngle_Component(float R_x, float R_y, float R_z,
		float L_x, float L_y, float L_z,
		float U_x, float U_y, float U_z);
};