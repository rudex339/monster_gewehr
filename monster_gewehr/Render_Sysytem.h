#pragma once
#include "Timer.h"


class ObjectManager;
class CCamera;

struct SetCamera_Event {
	CCamera* pCamera;
};

struct LIGHTS;

class Render_Sysytem : public EntitySystem,
	public EventSubscriber<SetCamera_Event>
{
private:
	CCamera* m_pCamera = NULL;
	ID3D12GraphicsCommandList* m_pd3dCommandList = NULL;
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
	ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap = NULL;


	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource* m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;
public:

	Render_Sysytem() = default;
	Render_Sysytem(ObjectManager* manager, ID3D12Device* pd3dDevice,ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void configure(class World* world);
	virtual void unconfigure(class World* world);
	virtual void tick(class World* world, float deltaTime);
	virtual void receive(class World* world, const SetCamera_Event& event);

	void SetRootSignANDDescriptorANDCammandlist(ObjectManager* manager, ID3D12GraphicsCommandList* pd3dCommandList);
};

