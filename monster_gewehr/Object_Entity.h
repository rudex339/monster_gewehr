#pragma once
#include "Meshmodel.h"

struct SkyBox_Component {
	CSkyBox* m_SkyBox;
	string m_name;

	SkyBox_Component() = default;
	SkyBox_Component(CSkyBox* SkyBox, string name) :m_SkyBox(SkyBox), m_name(name) {}
};

struct Terrain_Component {
	CHeightMapTerrain* m_pTerrain;
	string m_name;

	Terrain_Component() = default;
	Terrain_Component(CHeightMapTerrain* pTerrain, string name) :m_pTerrain(pTerrain), m_name(name) {}
};


//----------------------------------------------------------------------------------------------

struct Model_Component {
	CLoadedModelInfo* m_MeshModel;
	string model_name;

	Model_Component() = default;
	Model_Component(CLoadedModelInfo* MeshModel, string name)
		:m_MeshModel(MeshModel), model_name(name) {}
};

struct AnimationController_Component {
	CAnimationController* m_AnimationController;

	UINT cur_State;
	UINT next_State;

	AnimationController_Component() = default;
	AnimationController_Component(CAnimationController* AnimationController,
		UINT curState)
		:m_AnimationController(AnimationController), cur_State(curState), next_State(MAXDWORD){}
};

struct State_Component {
	UINT state=0;

	State_Component() = default;
};




struct Position_Component {
	XMFLOAT4X4 m_xmf4x4World;
	XMFLOAT3 Position;
	Position_Component() : m_xmf4x4World(Matrix4x4::Identity()) {}
	Position_Component(float x, float y, float z) {
		m_xmf4x4World = Matrix4x4::Identity();


		m_xmf4x4World._41 = x;
		m_xmf4x4World._42 = y;
		m_xmf4x4World._43 = z;

		Position = XMFLOAT3(x, y, z);
	}
	Position_Component(XMFLOAT4X4 xmf4x4World) {
		m_xmf4x4World = xmf4x4World;
		Position = XMFLOAT3(m_xmf4x4World._41, 
			m_xmf4x4World._42,
			m_xmf4x4World._43);
	}
};

struct Rotation_Component {
	XMMATRIX mtxRotate;
	float mfPitch, mfYaw, mfRoll;

	Rotation_Component() = default;
	Rotation_Component(float fPitch, float fYaw, float fRoll) {
		mtxRotate = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(fPitch),
			XMConvertToRadians(fYaw),
			XMConvertToRadians(fRoll));
		mfPitch = fPitch;
		mfYaw = fYaw;
		mfRoll = fRoll;
		//Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);
	}
	bool changeRotation(float fPitch, float fYaw, float fRoll) {
		mtxRotate = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(fPitch),
			XMConvertToRadians(fYaw),
			XMConvertToRadians(fRoll));
		mfPitch = fPitch;
		mfYaw = fYaw;
		mfRoll = fRoll;

		return true;
	}
};

struct Scale_Component {
	XMMATRIX mtxScale;
	float mx, my, mz;
	Scale_Component() = default;
	Scale_Component(float x, float y, float z) {
		mtxScale = XMMatrixScaling(x, y, z);
		mx = x;
		my = y;
		mz = z;
		//XMFLOAT4X4 m_xmf4x4ToParent = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParent);
	}
};



Entity* AddAnimationMeshObject(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	CLoadedModelInfo* model,
	float x, float y, float z,
	float rx, float ry, float rz, 
	float sx, float sy, float sz,
	int n_animation);

Entity* AddSoldierObject(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	CLoadedModelInfo* model,
	float x, float y, float z,
	float rx, float ry, float rz,
	float sx, float sy, float sz,
	int n_animation);

//----------------------------------------------------------------------------------------------
//moveable

struct Velocity_Component {
	XMFLOAT3 m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_velRotate = XMFLOAT3(0.0f, 0.0f, 0.0f);


	Velocity_Component() = default;
};

struct EulerAngle_Component {
	XMFLOAT3 m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3 m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	EulerAngle_Component() = default;
	EulerAngle_Component(float R_x, float R_y, float R_z,
		float L_x, float L_y, float L_z,
		float U_x, float U_y, float U_z);
};
//----------------------------------------------------------------------------------------------
//light
#define MAX_LIGHTS						16 

#define POINT_LIGHT						1
#define SPOT_LIGHT						2
#define DIRECTIONAL_LIGHT				3

struct LIGHT
{
	XMFLOAT4							m_xmf4Ambient;
	XMFLOAT4							m_xmf4Diffuse;
	XMFLOAT4							m_xmf4Specular;
	XMFLOAT3							m_xmf3Position;
	float 								m_fFalloff;
	XMFLOAT3							m_xmf3Direction;
	float 								m_fTheta; //cos(m_fTheta)
	XMFLOAT3							m_xmf3Attenuation;
	float								m_fPhi; //cos(m_fPhi)
	bool								m_bEnable;
	int									m_nType;
	float								m_fRange;
	float								padding;
};

struct Light_Component {
	LIGHT *m_pLight = NULL;
	Light_Component() = default;
};

//----------------------------------------------------------------------------------------------
//player

struct BoundingBox_Component {
	DirectX::BoundingOrientedBox m_bounding_box;
	BoundingBox_Component() = default;
};

//----------------------------------------------------------------------------------------------
//UI
struct TextUI_Component {
	wstring m_text;
	D2D1_RECT_F m_Rect;

	TextUI_Component() = default;
	TextUI_Component(const wchar_t* text, float top, float left, float bottom, float right);
	TextUI_Component(wstring text, float top, float left, float bottom, float right);
};