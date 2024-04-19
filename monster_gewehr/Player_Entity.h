#pragma once
class CLoadedModelInfo;

Entity* AddPlayerEntity(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	CLoadedModelInfo* model,
	float x, float y, float z,
	float rx, float ry, float rz,
	float sx, float sy, float sz,
	int n_animation);