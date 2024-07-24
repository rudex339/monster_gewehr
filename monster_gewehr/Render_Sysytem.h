#pragma once
#include <map>
#include "Timer.h"
#include "Scene_Sysytem.h"
#include "Collision_Sysytem.h"


class ObjectManager;
class CCamera;
class Box;

enum FontType {
	DEFAULT_FONT,
	SMALL_FONT,
	MEDIUM_FONT,
	NEEDLE_FONT,
	NANO_FONT
};

struct SetCamera_Event {
	CCamera* pCamera;
};

struct KeyDown_Event {
	WPARAM key;
};

struct Tab_Event {
	bool tab;
};

struct Mouse_Event {
	POINT cursorPos;
	bool click;
};

struct LIGHTS;

//임시
struct DrawUI_Event {
};

// 로그인 이벤트
struct InputId_Event {

};

// 보급상자 정보 저장
struct Supply
{
	float cooltime = 180.0f;
	float time = 0.0f;
	bool state = true;
	XMFLOAT3 position;
};


class Render_System : public EntitySystem,
	public EventSubscriber<SetCamera_Event>,
	public EventSubscriber<DrawUI_Event>,
	public EventSubscriber<KeyDown_Event>,
	public EventSubscriber<Tab_Event>,
	public EventSubscriber<Mouse_Event>,
	public EventSubscriber<InputId_Event>
{
private:
	CCamera* m_pCamera = NULL;
	ID3D12Device* m_pd3dDevice;
	ID3D12GraphicsCommandList* m_pd3dCommandList = NULL;
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
	ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap = NULL;
	Scene_Sysytem* m_scene;
	

	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource* m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;

	//UI
	ID2D1DeviceContext2* m_d2dDeviceContext;
	IDWriteFactory5* m_dwriteFactory;
	ID2D1Factory3* m_d2dFactory;
	ID2D1Bitmap* m_bitmaps[15];

	//editbox
	IDWriteTextLayout* pTextLayout[2];
	wstring	text[2];
	wstring Invisile_password;
	int textIndex = 0;
	size_t cursorPosition[2] = { 0, 0 };
	wchar_t inputKey;


	// button
	bool clicked;
	int button_event;
	int select_room_num = -1;

	ComPtr<IDWriteTextFormat> m_textFormat;
	ComPtr<IDWriteTextFormat> m_smalltextFormat;
	ComPtr<IDWriteTextFormat> m_verysmalltextFormat;
	ComPtr<IDWriteTextFormat> pTextFormat;
	ComPtr<IDWriteTextFormat> m_ingametextFormat;
	ComPtr<IDWriteTextFormat> m_ingametextFormat2;


	IDWriteFontFile* pFontFile;
	IDWriteFontFace* pFontFace;
	IDWriteFontCollection1* fontCollection;

	/*
		[0] : fontsize 48
		[1] : fontsize 54
	*/
	ComPtr<IDWriteTextFormat> Needleteeth[2];

	ComPtr<ID2D1SolidColorBrush> m_textBrush;

	ComPtr<ID2D1StrokeStyle> m_trokeBrush;

	POINT m_cursorPos;
	
	//boundingbox
	Box* m_pBox;

	// user info (for minimap)
	// key : userID, value : coordinate
	map <int, POINT> UserPositionXZ;

	//// 보급 위치 1(유저-보스 스폰 사이) 2290, 1825
	// 보급 위치 2(도망1~도망2 사이) 1485, 3065
	// 보급 위치 3(도망2~도망3 사이) 385, 2230
	// 보급 위치 4(도망3 구역 구석) 115, 730
	// 보급 위치 5(두갈래길 사이) 865, 2230
	
	// supplys
	XMFLOAT3 supplys[5] = {
		{2290, 0, 1825},
		{1485, 0, 3065},
		{385, 0, 2230},
		{115, 0, 730},
		{865, 0, 2230}
	};

public:
	Render_System() = default;
	Render_System(ObjectManager* manager, ID3D12Device* pd3dDevice,ID3D12GraphicsCommandList* pd3dCommandList, ID2D1DeviceContext2* d2dDeviceContext, ID2D1Factory3* d2dFactory, IDWriteFactory5* dwriteFactory, Scene_Sysytem* scene);
	virtual void configure(class World* world);
	virtual void unconfigure(class World* world);
	virtual void tick(class World* world, float deltaTime);

	virtual void receive(class World* world, const SetCamera_Event& event);
	//인시
	virtual void receive(class World* world, const DrawUI_Event& event);
	virtual void receive(class World* world, const KeyDown_Event& event);
	virtual void receive(class World* world, const Tab_Event& event);
	virtual void receive(class World* world, const Mouse_Event& event);
	virtual void receive(class World* world, const InputId_Event& event);

	void Clicked() { clicked = false; }
	void SetUserInfo(int uid, POINT coordinate);
	map <int, POINT> GetUserInfo() { return UserPositionXZ; }
	void ClearUserInfo() { UserPositionXZ.clear(); }

	float Distance(XMFLOAT3 posA, XMFLOAT3 posB);

	void SetRootSignANDDescriptorANDCammandlist(ObjectManager* manager, ID3D12GraphicsCommandList* pd3dCommandList);
};

