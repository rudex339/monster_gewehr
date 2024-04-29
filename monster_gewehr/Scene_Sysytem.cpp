#include "stdafx.h"
#include "Scene_Sysytem.h"
#include "Player_Entity.h"
#include "Object_Entity.h"
#include "ObjectManager.h"
#include "Sever_Sysyem.h"
#include "PlayerControl_System.h"
#include "Render_Sysytem.h"





Scene_Sysytem::Scene_Sysytem(ObjectManager* pObjectManager, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList):
	m_pObjectManager(pObjectManager),
	m_pd3dDevice(pd3dDevice),
	m_pd3dCommandList(pd3dCommandList) {

}

void Scene_Sysytem::configure(World* world)
{
	world->subscribe<ChangeScene_Event>(this);
}

void Scene_Sysytem::unconfigure(World* world)
{
}

void Scene_Sysytem::tick(World* world, float deltaTime)
{
	UCHAR pKeysBuffer[256];
	if (GetKeyboardState(pKeysBuffer)) {
		switch (m_State) {
		case LOGIN:
			if (pKeysBuffer[VK_RETURN] & 0xF0) {
				world->emit< ChangeScene_Event>({LOBBY});
				world->emit<Login_Event>({});
			}
			break;
		case LOBBY:
			if (pKeysBuffer[VK_SPACE] & 0xF0) {
				world->emit< ChangeScene_Event>({ GAME });
				world->emit<Game_Start>({});
			}
			break;
		case GAME:
			break;
		case END:
			if (pKeysBuffer[VK_RETURN] & 0xF0) {
				world->emit< ChangeScene_Event>({ LOBBY });
			}
			break;
		}
	}

}

void Scene_Sysytem::receive(World* world, const ChangeScene_Event& event)
{
	//world->cleanup();
	m_State = event.State;
	switch (m_State) {
	case LOGIN:
		break;
	case LOBBY:
	{
		m_pPawn = NULL;
		world->reset();
		world->emit<SetCamera_Event>({ NULL });
	}
		break;
	case GAME:
	{
		world->reset();

		m_pPawn = AddPlayerEntity(world->create(), m_pd3dDevice, m_pd3dCommandList,
			m_pObjectManager->Get_ModelInfo("Soldier"),
			1014.f, m_pObjectManager->m_pTerrain->GetHeight(1014.f, 1429.f)/*2000.f*/, 1429.0f,
			0.f, 0.f, 0.f,
			6.0f, 6.0f, 6.0f,
			3);


		cout << m_pPawn->get<player_Component>()->id << endl;
		Entity* ent = world->create();
		ent->assign<SkyBox_Component>(m_pObjectManager->m_pSkyBox, "default");

		BuildScene(world, (char*)"Scene/Scene.bin");

		ent = AddAnotherEntity(world->create(), m_pd3dDevice, m_pd3dCommandList,
			m_pObjectManager->Get_ModelInfo("Souleater"),
			1014.f, m_pObjectManager->m_pTerrain->GetHeight(1014.f, 1429.f), 1429.0f,
			0.f, 90.f, 0.f,
			25.f, 25.f, 25.f,
			10);
		auto monster_id = ent->get<player_Component>();
		monster_id->id = -2;

		ent = world->create();
		auto light = ent->assign<Light_Component>();
		light->m_pLight = new LIGHT;
		::ZeroMemory(light->m_pLight, sizeof(LIGHT));
		light->m_pLight->m_bEnable = true;
		light->m_pLight->m_nType = DIRECTIONAL_LIGHT;
		light->m_pLight->m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		light->m_pLight->m_xmf4Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		light->m_pLight->m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
		light->m_pLight->m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);

		AddAnotherEntity(world->create(), m_pd3dDevice, m_pd3dCommandList,
			m_pObjectManager->Get_ModelInfo("Soldier"),
			310.0f, m_pObjectManager->m_pTerrain->GetHeight(310.0f, 600.0f), 600.0f,
			0.f, 0.f, 0.f,
			6.0f, 6.0f, 6.0f,
			3);

		AddAnotherEntity(world->create(), m_pd3dDevice, m_pd3dCommandList,
			m_pObjectManager->Get_ModelInfo("Soldier"),
			310.0f, m_pObjectManager->m_pTerrain->GetHeight(310.0f, 600.0f), 600.0f,
			0.f, 0.f, 0.f,
			6.0f, 6.0f, 6.0f,
			3);
		AddAnotherEntity(world->create(), m_pd3dDevice, m_pd3dCommandList,
			m_pObjectManager->Get_ModelInfo("Soldier"),
			310.0f, m_pObjectManager->m_pTerrain->GetHeight(310.0f, 600.0f), 600.0f,
			0.f, 0.f, 0.f,
			6.0f, 6.0f, 6.0f,
			3);

		CCamera* temp = new CThirdPersonCamera(NULL);
		temp->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);

		ComponentHandle<Camera_Component> camera = m_pPawn->assign<Camera_Component>(temp);
		camera->m_pCamera->SetPosition(XMFLOAT3(310.0f,
			m_pObjectManager->m_pTerrain->GetHeight(310.0f, 600.0f) + 10.f, 600.0f - 30.f));

		world->emit<GetPlayerPtr_Event>({ m_pPawn });

	}
		break;
	case END:
		world->emit<GetPlayerPtr_Event>({ NULL });
		break;
	}
	
}

void Scene_Sysytem::BuildScene(World* world, char* pstrFileName)
{
	FILE* pFile = NULL;
	::fopen_s(&pFile, pstrFileName, "rb");
	::rewind(pFile);

	char pstrToken[256] = { '\0' };
	char pstrGameObjectName[256] = { '\0' };
	UINT nReads = 0;
	BYTE nStrLength = 0, nObjectNameLength = 0;
	int	m_nObjects = 0;
	::ReadStringFromFile(pFile, pstrToken); //"<GameObjects>:"
	nReads = (UINT)::fread(&m_nObjects, sizeof(int), 1, pFile);

	for (int i = 0; i < m_nObjects; i++)
	{
		::ReadStringFromFile(pFile, pstrToken); //"<GameObject>:"			
		if (!strcmp(pstrToken, "<GameObject>:")) {
			Entity* ent = world->create();
			::ReadStringFromFile(pFile, pstrGameObjectName);

			if (!strcmp(pstrGameObjectName, "Cube")) {
				ent->assign<Terrain_Component>(m_pObjectManager->m_pTerrain, "default");
			}


			XMFLOAT4X4	xmf4x4World;
			nReads = (UINT)::fread(&xmf4x4World, sizeof(float), 16, pFile); //Transform


			xmf4x4World = Matrix4x4::Multiply(XMMatrixScaling(
				(4104 / 330),
				(4104 / 330),
				(4104 / 330)), xmf4x4World);



			xmf4x4World._41 *= (4104 / 330);
			xmf4x4World._42 *= (4104 / 330);
			xmf4x4World._43 *= (4104 / 330);
			xmf4x4World = Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(0.f),
				XMConvertToRadians(0.f),
				XMConvertToRadians(0.f)), xmf4x4World);

			XMVECTOR translation = XMVectorSet(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43, 1.0f);
			translation = XMVector3Transform(translation, XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(0.f),
				XMConvertToRadians(180.f),
				XMConvertToRadians(0.f)));

			xmf4x4World._41 = XMVectorGetX(translation);
			xmf4x4World._42 = XMVectorGetY(translation);
			xmf4x4World._43 = XMVectorGetZ(translation);

			xmf4x4World._41 += 2825.f;
			xmf4x4World._42 += 1024.f;
			xmf4x4World._43 += 1918.f;

			ent->assign<Position_Component>(xmf4x4World);


			ent->assign<Model_Component>(m_pObjectManager->Get_ModelInfo(pstrGameObjectName),
				m_pObjectManager->Get_ModelInfo(pstrGameObjectName)->m_pModelRootObject->m_pstrFrameName);
		}
		else
			i -= 1;
	}
	::fclose(pFile);
}
