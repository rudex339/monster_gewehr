#include "stdafx.h"
#include "Render_Sysytem.h"



Render_Sysytem::Render_Sysytem(ObjectManager* manager, ID3D12GraphicsCommandList* pd3dCommandList)
{
	SetRootSignANDDescriptorANDCammandlist(manager, pd3dCommandList);
}

void Render_Sysytem::configure(World* world)
{
	world->subscribe<SetCamera_Event>(this);
}

void Render_Sysytem::unconfigure(World* world)
{
}

void Render_Sysytem::tick(World* world, float deltaTime)
{
	//if (m_pd3dGraphicsRootSignature) m_pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	//if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	//m_pCamera->SetViewportsAndScissorRects(m_pd3dCommandList);
	//m_pCamera->UpdateShaderVariables(m_pd3dCommandList);

	

#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif
	//if (m_pPlayer) m_pPlayer->Render(m_pd3dCommandList, m_pCamera);


}

void Render_Sysytem::receive(World* world, const SetCamera_Event& event)
{
	m_pCamera = event.pCamera;
}

void Render_Sysytem::SetRootSignANDDescriptorANDCammandlist(ObjectManager* manager, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dCommandList = pd3dCommandList;
	m_pd3dGraphicsRootSignature = manager->GetGraphicsRootSignature();
	m_pd3dCbvSrvDescriptorHeap = manager->GetCbvSrvDescriptorHeap();
}

void Animate_Sysytem::configure(World* world)
{
}

void Animate_Sysytem::unconfigure(World* world)
{
}

void Animate_Sysytem::tick(World* world, float deltaTime)
{
}
