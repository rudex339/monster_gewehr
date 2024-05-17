#pragma once
#include "Timer.h"


class ObjectManager;
class CCamera;
class Box;

struct SetCamera_Event {
	CCamera* pCamera;
};

struct KeyDown_Event {
	WPARAM key;
};

struct LIGHTS;

//임시
struct DrawUI_Event {
};

class Render_Sysytem : public EntitySystem,
	public EventSubscriber<SetCamera_Event>,
	public EventSubscriber<DrawUI_Event>,
	public EventSubscriber<KeyDown_Event>
{
private:
	CCamera* m_pCamera = NULL;
	ID3D12GraphicsCommandList* m_pd3dCommandList = NULL;
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
	ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap = NULL;

	

	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource* m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;

	//UI
	ID2D1DeviceContext2* m_d2dDeviceContext;
	IDWriteFactory5* m_dwriteFactory;
	ID2D1Factory3* m_d2dFactory;
	ID2D1Bitmap* m_bitmaps[10];

	//editbox
	IDWriteTextLayout* pTextLayout;
	wstring	text;
	size_t cursorPosition = 0;
	wchar_t inputKey;


	ComPtr<IDWriteTextFormat> m_textFormat;
	ComPtr<IDWriteTextFormat> m_smalltextFormat;
	ComPtr<IDWriteTextFormat> pTextFormat;

	ComPtr<ID2D1SolidColorBrush> m_textBrush;

	ComPtr<ID2D1StrokeStyle> m_trokeBrush;
	
	//boundingbox
	Box* m_pBox;

public:

	Render_Sysytem() = default;
	Render_Sysytem(ObjectManager* manager, ID3D12Device* pd3dDevice,ID3D12GraphicsCommandList* pd3dCommandList, ID2D1DeviceContext2* d2dDeviceContext, ID2D1Factory3* d2dFactory, IDWriteFactory5* dwriteFactory);
	virtual void configure(class World* world);
	virtual void unconfigure(class World* world);
	virtual void tick(class World* world, float deltaTime);

	virtual void receive(class World* world, const SetCamera_Event& event);
	//인시
	virtual void receive(class World* world, const DrawUI_Event& event);
	virtual void receive(class World* world, const KeyDown_Event& event);


	void SetRootSignANDDescriptorANDCammandlist(ObjectManager* manager, ID3D12GraphicsCommandList* pd3dCommandList);
};

