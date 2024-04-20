#include "stdafx.h"
#include "Render_Sysytem.h"
#include "Object_Entity.h"
#include "ObjectManager.h"
#include "Player_Entity.h"


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
	world->unsubscribeAll(this);
}

void Render_Sysytem::tick(World* world, float deltaTime)
{
	if (m_pd3dGraphicsRootSignature) m_pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	

	m_pCamera->SetViewportsAndScissorRects(m_pd3dCommandList);
	m_pCamera->UpdateShaderVariables(m_pd3dCommandList);

	//UpdateShaderVariables(pd3dCommandList);
	//D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	//pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights


	world->each<SkyBox_Component>([&](
		Entity* ent,
		ComponentHandle<SkyBox_Component> SkyBox
		) -> void {
			SkyBox->m_SkyBox->Render(m_pd3dCommandList, m_pCamera);
		});

	world->each<Terrain_Component>([&](
		Entity* ent,
		ComponentHandle<Terrain_Component> Terrain
		) -> void {
			Terrain->m_pTerrain->Render(m_pd3dCommandList, m_pCamera);
		});


	world->each<AnimationController_Component, Model_Component, Position_Component, Rotation_Component, Scale_Component>([&](
		Entity* ent,
		ComponentHandle<AnimationController_Component> AnimationController,
		ComponentHandle<Model_Component> Model,
		ComponentHandle<Position_Component> pos,
		ComponentHandle<Rotation_Component> rotation,
		ComponentHandle<Scale_Component> Scale
		) -> void {

			if (ent->has<player_Component>()) {
				ComponentHandle<player_Component> data = ent->get<player_Component>();
				if (data->id == -1)
					return;
		}
		AnimationController->m_AnimationController->AdvanceTime(deltaTime, Model->m_MeshModel->m_pModelRootObject);
		Model->m_MeshModel->m_pModelRootObject->Animate(deltaTime);

		XMFLOAT4X4 xmf4x4World = Matrix4x4::Identity();
		xmf4x4World = Matrix4x4::Multiply(XMMatrixScaling(Scale->mx, 
			Scale->my, 
			Scale->mz), xmf4x4World);
		xmf4x4World = Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(rotation->mfPitch),
			XMConvertToRadians(rotation->mfYaw),
			XMConvertToRadians(rotation->mfRoll)), xmf4x4World);
		xmf4x4World._41 = pos->Position.x;
		xmf4x4World._42 = pos->Position.y;
		xmf4x4World._43 = pos->Position.z;
		

		Model->m_MeshModel->m_pModelRootObject->UpdateTransform(&xmf4x4World);

		AnimationController->m_AnimationController->UpdateShaderVariables(m_pd3dCommandList);

		Model->m_MeshModel->m_pModelRootObject->Render(
			m_pd3dCommandList, m_pCamera);
	});
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
