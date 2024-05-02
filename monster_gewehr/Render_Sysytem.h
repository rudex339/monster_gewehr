#pragma once
#include "Timer.h"


class ObjectManager;
class CCamera;

struct SetCamera_Event {
	CCamera* pCamera;
};

struct LIGHTS;

//임시
struct DrawUI_Event {
};

class Render_Sysytem : public EntitySystem,
	public EventSubscriber<SetCamera_Event>,
	public EventSubscriber<DrawUI_Event>
{
private:
	CCamera* m_pCamera = NULL;
	ID3D12GraphicsCommandList* m_pd3dCommandList = NULL;
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
	ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap = NULL;

	//UI
	ID2D1DeviceContext2* m_d2dDeviceContext;
	IDWriteFactory5* m_dwriteFactory;

	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource* m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;
	ComPtr<IDWriteTextFormat> m_textFormat;
	ComPtr<ID2D1SolidColorBrush> m_textBrush;

public:

	Render_Sysytem() = default;
	Render_Sysytem(ObjectManager* manager, ID3D12Device* pd3dDevice,ID3D12GraphicsCommandList* pd3dCommandList, ID2D1DeviceContext2* d2dDeviceContext, IDWriteFactory5* dwriteFactory);
	virtual void configure(class World* world);
	virtual void unconfigure(class World* world);
	virtual void tick(class World* world, float deltaTime);

	virtual void receive(class World* world, const SetCamera_Event& event);
	//인시
	virtual void receive(class World* world, const DrawUI_Event& event);

	void SetRootSignANDDescriptorANDCammandlist(ObjectManager* manager, ID3D12GraphicsCommandList* pd3dCommandList);
};

