#pragma once
#include "Meshmodel.h"

class ObjectManager;


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
	GameObjectModel* m_MeshModel;
	
	string model_name;

	GameObjectModel* socket = NULL;

	Model_Component* m_pParentObject = NULL;
	vector<Model_Component*> m_pchildObjects;

	bool draw = true;
	Model_Component() = default;
	Model_Component(GameObjectModel* MeshModel, string name)
		:m_MeshModel(MeshModel), model_name(name) {}

	void addChildComponent(Model_Component* child);

	void SetSocket(GameObjectModel* rootModel, char* name);
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


	Position_Component() : m_xmf4x4World(Matrix4x4::Identity()){}
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



Entity* AddSoldierObject(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	ObjectManager* OM,
	float x, float y, float z,
	float rx, float ry, float rz,
	float sx, float sy, float sz);

Entity* AddMonsterObject(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	ObjectManager* OM,
	float x, float y, float z,
	float rx, float ry, float rz,
	float sx, float sy, float sz);

//----------------------------------------------------------------------------------------------
//moveable

struct Velocity_Component {
	XMFLOAT3 m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_velRotate = XMFLOAT3(0.0f, 0.0f, 0.0f);

	bool gravity = true;

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
	//test
	CMesh* m_pMesh = NULL;


	unordered_map<string, DirectX::BoundingOrientedBox*> m_BoundingBoxList;
	BoundingBox_Component(float x, float y, float z) {
		m_bounding_box.Extents = XMFLOAT3(x,y,z);
	}
	BoundingBox_Component(XMFLOAT3 extent, XMFLOAT3 center = XMFLOAT3(0.f, 0.f, 0.f)) {
		m_bounding_box.Center = center;
		m_bounding_box.Extents = extent;
	}
	void add(string bone, DirectX::BoundingOrientedBox* boundingbox) {
		m_BoundingBoxList.insert({ bone, boundingbox });
	}
};

//----------------------------------------------------------------------------------------------
//UI
struct TextUI_Component {
	wstring m_text;
	D2D1_RECT_F m_Rect;
	int m_fontType;
	
	DWRITE_PARAGRAPH_ALIGNMENT m_paragraph_alignment;
	DWRITE_TEXT_ALIGNMENT m_text_alignment;

	TextUI_Component() = default;
	TextUI_Component(int fontType, const wchar_t* text, float top, float left, float bottom, float right);
	TextUI_Component(int fontType, wstring text, float top, float left, float bottom, float right);
};

struct TextBoxUI_Component {
	float x, y;
	float m_width, m_height;
	int index;

	TextBoxUI_Component() = default;
	TextBoxUI_Component(float layoutX, float layoutY, float width, float height, int num);
	bool CursorInBox(POINT cursor);

};

struct ImageUI_Component {
	
	ID2D1DeviceContext2* m_d2dDeviceContext;
	ID2D1Factory3* m_d2dFactory;
	ID2D1Bitmap* m_bitmap;

	float m_opacity;
	D2D1_INTERPOLATION_MODE m_mode;

	D2D1_RECT_F m_Rect, m_imageRect;

	ImageUI_Component() = default;
	ImageUI_Component(const wchar_t* imagePath, ID2D1DeviceContext2* deviceContext, ID2D1Factory3* factory, ID2D1Bitmap* bitmap, D2D1_RECT_F posrect, float opacity, D2D1_INTERPOLATION_MODE mode, D2D1_RECT_F imagerect);

};

struct Button_Component : public ImageUI_Component, TextUI_Component {
	D2D1_RECT_F m_Rect, m_Rectsaved;
	ComPtr<IDWriteTextFormat> m_textFormats[2];
	ComPtr<IDWriteTextFormat> m_textFormat;
	ComPtr<ID2D1SolidColorBrush> m_textBrush;

	int button_id;
	bool cursor_on;
	int m_room_num;
	int item_num;

	bool activate;

	UINT Next_Scene;
	UINT Curr_Scene;

	Button_Component() = default;
	Button_Component(int id, const wchar_t* imagePath, int fontType, wstring m_text, ID2D1DeviceContext2* deviceContext, ID2D1Factory3* factory, ID2D1Bitmap* bitmap, D2D1_RECT_F posrect, float opacity, D2D1_INTERPOLATION_MODE mode, D2D1_RECT_F imagerect, int num = -1);

	void CursorOn(POINT cursor, ComPtr<IDWriteTextFormat> pTextFormat, ComPtr<IDWriteTextFormat> small_font);
	void Disable();
	void Activate();
};

//grande component
struct Grande_Component {
	UINT property = 0;
	DirectX::BoundingSphere sphere;
	float damage;
	float range;
	bool Boom = false;
	float coolTime = 0.f;
	Grande_Component() = default;

	Grande_Component(const UINT& property,float range, float damage)
		: property(property), range(range), damage(damage)
	{
		sphere.Radius = range;
	}
};

struct Emitter_Componet {
	float fTime=0.f;
	float speed = 0.f;
	float coolTime ;
	int m_nRow = 0;
	int m_nCol = 0;
	int m_nRows = 1;
	int m_nCols = 1;

	Emitter_Componet() = default;

	Emitter_Componet(float coolTime = 100.f, float speed=0.1f, int rows = 1, int cols = 1)
		: coolTime(coolTime), m_nRows(rows),m_nCols(cols),speed(speed)
	{
	}
};

constexpr int MAX_PLAY_SOUND = 32;

struct Sound_Componet {
	enum Music
	{
		Title,
		Ingame,
		GameClear,
		GameFail,
		MusicCount
	};
	enum Sound
	{
		B_On,
		B_Push,
		Purchase,
		Rifle,
		ShotGun,
		Sniper,
		Walk,
		Run,
		Reload,
		Dash,
		Hurt,
		SoundCount
	};
	enum TDSound
	{
		TDRifle,
		TDShotGun,
		TDSniper,
		TDSoundCount
	};

	Sound_Componet(const Sound_Componet& rhs) = delete;
	Sound_Componet(Sound_Componet&& rhs) = delete;
	void operator=(const Sound_Componet& rhs) = delete;
	void operator=(Sound_Componet&&) = delete;

	static Sound_Componet& GetInstance()
	{
		static Sound_Componet instance;
		return instance;
	}

	FMOD::System* m_system;

	array<FMOD::Sound*, Music::MusicCount> m_music;
	FMOD::Channel* m_musicChannel;

	array<FMOD::Sound*, Sound::SoundCount>	m_sound;
	array<FMOD::Sound*, TDSound::TDSoundCount>	m_3dsound;	// 3d 효과음을 위한 사운드 어레이
	array<FMOD::Channel*, MAX_PLAY_SOUND>	m_soundChannel; // 효과음 출력을 위한 채널

	array<FMOD::Channel*, 2> m_movesoundChannel;

	FMOD_RESULT m_result;

	Sound_Componet();
	~Sound_Componet();

	void PlayMusic(Music tag);
	void StopMusic();

	void PlaySound(Sound tag);
	void PlaySound(int type);
	void PlayMoveSound(Sound tag);

	// 이밑부터는 다른사람 소리 들을때 3d 효과 넣는 함수들
	void Play3DSound(XMFLOAT3 sound, TDSound tag);

	void ListenerUpdate(XMFLOAT3 pos, XMFLOAT3 vel, XMFLOAT3 front, XMFLOAT3 up);

};

class SoldierAnimationController : public CAnimationController {
private:
	Entity* m_owner;
	float  velocityXZ = 10.f;
	float weight_Under[10] = { 0, };
public:

	SoldierAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel, Entity* owner);
	~SoldierAnimationController();

	//virtual void SetTrackEnable(int nAnimationTrack, bool bEnable);
	virtual void AdvanceTime(float fElapsedTime, GameObjectModel* pRootGameObject);
	virtual void Animate(float fElapsedTime);
};