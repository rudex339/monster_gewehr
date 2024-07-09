#pragma once
#include "Timer.h"
#include "Scene_Sysytem.h"


class ObjectManager;
class CCamera;
class Box;

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
	ID2D1Bitmap* m_bitmaps[10];

	//editbox
	IDWriteTextLayout* pTextLayout[2];
	wstring	text[2];

	int textIndex = 0;
	size_t cursorPosition[2] = { 0, 0 };
	wchar_t inputKey;


	// button
	bool clicked;
	int button_event;

	ComPtr<IDWriteTextFormat> m_textFormat;
	ComPtr<IDWriteTextFormat> m_smalltextFormat;
	ComPtr<IDWriteTextFormat> pTextFormat;

	ComPtr<ID2D1SolidColorBrush> m_textBrush;

	ComPtr<ID2D1StrokeStyle> m_trokeBrush;

	POINT m_cursorPos;
	
	//boundingbox
	Box* m_pBox;


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

	void Clicked() { clicked = false; };

	void SetRootSignANDDescriptorANDCammandlist(ObjectManager* manager, ID3D12GraphicsCommandList* pd3dCommandList);
};

