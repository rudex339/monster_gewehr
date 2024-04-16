#pragma once
#include "Timer.h"
#include "Player.h"
#include "ObjectManager.h"

struct SetCamera_Event {
	CCamera* pCamera;
};

class Render_Sysytem : public EntitySystem,
	public EventSubscriber<SetCamera_Event>
{
private:
	CCamera* m_pCamera = NULL;
	ID3D12GraphicsCommandList* m_pd3dCommandList = NULL;
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
	ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap = NULL;
public:

	Render_Sysytem() = default;
	Render_Sysytem(ObjectManager* manager, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void configure(class World* world);
	virtual void unconfigure(class World* world);
	virtual void tick(class World* world, float deltaTime);
	virtual void receive(class World* world, const SetCamera_Event& event);

	void SetRootSignANDDescriptorANDCammandlist(ObjectManager* manager, ID3D12GraphicsCommandList* pd3dCommandList);
};


class Animate_Sysytem : public EntitySystem
{
private:
public:

	Animate_Sysytem() = default;
	virtual void configure(class World* world);
	virtual void unconfigure(class World* world);
	virtual void tick(class World* world, float deltaTime);
};
