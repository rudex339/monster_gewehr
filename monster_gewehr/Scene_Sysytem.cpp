#include "stdafx.h"
#include "Scene_Sysytem.h"
#include "Player_Entity.h"
#include "Object_Entity.h"
#include "ObjectManager.h"
#include "Sever_Sysyem.h"
#include "PlayerControl_System.h"

Scene_Sysytem::Scene_Sysytem(ObjectManager* pObjectManager, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID2D1DeviceContext2* deviceContext, ID2D1Factory3* factory):
	m_pObjectManager(pObjectManager),
	m_pd3dDevice(pd3dDevice),
	m_pd3dCommandList(pd3dCommandList),
	m_d2dDeviceContext(deviceContext),
	m_d2dFactory(factory)
	{
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
				/*world->emit< ChangeScene_Event>({ GAME });
				world->emit<Game_Start>({});*/
			}
			break;
		case ROOMS:
			if (pKeysBuffer[VK_SPACE] & 0xF0) {
				world->emit< ChangeScene_Event>({ INROOM });
			}
			else if (pKeysBuffer[VK_ESCAPE] & 0xF0) {
				world->emit< ChangeScene_Event>({ LOBBY });
			}
			break;
		case INROOM:
			if (pKeysBuffer[VK_RETURN] & 0xF0) {
				world->emit< ChangeScene_Event>({ GAME });
				world->emit<Game_Start>({});
			}
			else if (pKeysBuffer[VK_ESCAPE] & 0xF0) {
				world->emit< ChangeScene_Event>({ ROOMS });
			}
			break;
		case SHOP:
			if (pKeysBuffer[VK_ESCAPE] & 0xF0) {
				world->emit< ChangeScene_Event>({ LOBBY });
			}
			break;
		case EQUIPMENT:
			if (pKeysBuffer[VK_ESCAPE] & 0xF0) {
				world->emit< ChangeScene_Event>({ LOBBY });
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
	case LOGIN: {
		Entity* ent = world->create();

		D2D1_RECT_F sRect, imageRect;
		sRect = { 0,0,FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
		imageRect = { 0, 0, 1000, 563 };
		ent->assign<ImageUI_Component>(L"image/monster_hunter_login.png", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

		ent->assign<TextBoxUI_Component>(800.0f, 340.0f, 0);

		ent = world->create();
		ent->assign<TextBoxUI_Component>(800.0f, 380.0f, 1);

	}
	break;

	case LOBBY:
	{
		world->reset();
		m_pPawn = NULL;
		world->reset();
		world->emit<SetCamera_Event>({ NULL });

		Entity* ent = world->create();
		ent->assign<TextUI_Component>(L"press space",
			(float)FRAME_BUFFER_HEIGHT / 2 + 200, (float)FRAME_BUFFER_WIDTH / 2 - 400,
			(float)FRAME_BUFFER_HEIGHT / 2 + 280, (float)FRAME_BUFFER_WIDTH / 2 + 400);

		ent = world->create();
		D2D1_RECT_F imageRect;
		D2D1_RECT_F sRect[4] = {{FRAME_BUFFER_WIDTH / 2 - 50.f , FRAME_BUFFER_HEIGHT / 2 - 15.f, FRAME_BUFFER_WIDTH / 2 + 50.f , FRAME_BUFFER_HEIGHT / 2 + 15.f},
					{FRAME_BUFFER_WIDTH / 2 - 50.f , FRAME_BUFFER_HEIGHT / 2 + 30.f, FRAME_BUFFER_WIDTH / 2 + 50.f , FRAME_BUFFER_HEIGHT / 2 + 60.f},
					{FRAME_BUFFER_WIDTH / 2 - 50.f , FRAME_BUFFER_HEIGHT / 2 + 75.f, FRAME_BUFFER_WIDTH / 2 + 50.f, FRAME_BUFFER_HEIGHT / 2 + 105.f},
					{FRAME_BUFFER_WIDTH / 2 - 50.f , FRAME_BUFFER_HEIGHT / 2 + 130.f, FRAME_BUFFER_WIDTH / 2 + 50.f, FRAME_BUFFER_HEIGHT / 2 + 160.f}};
		
		imageRect = { 0, 0, 1000, 563 };

		ent->assign<Button_Component>(0, L"image/monster_hunter_login.png", L"게임시작", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			sRect[0], 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

		ent = world->create();
		ent->assign<Button_Component>(1, L"image/monster_hunter_login.png", L"상점", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			sRect[1], 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

		ent = world->create();
		ent->assign<Button_Component>(2, L"image/monster_hunter_login.png", L"장비창", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			sRect[2], 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

		ent = world->create();
		ent->assign<Button_Component>(-1, L"image/monster_hunter_login.png", L"게임종료", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			sRect[3], 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);
	}
	break;

	case ROOMS:
	{
		world->reset();
		Entity* ent = world->create();
		ent->assign<TextUI_Component>(L"방들 있는 로비",
			(float)FRAME_BUFFER_HEIGHT / 2 + 200, (float)FRAME_BUFFER_WIDTH / 2 - 400,
			(float)FRAME_BUFFER_HEIGHT / 2 + 280, (float)FRAME_BUFFER_WIDTH / 2 + 400);
	}
	break;

	case INROOM:
	{
		world->reset();
		Entity* ent = world->create();
		ent->assign<TextUI_Component>(L"방",
			(float)FRAME_BUFFER_HEIGHT / 2 + 200, (float)FRAME_BUFFER_WIDTH / 2 - 400,
			(float)FRAME_BUFFER_HEIGHT / 2 + 280, (float)FRAME_BUFFER_WIDTH / 2 + 400);
	} 
	break;
	
	case SHOP:
	{
		world->reset();
		Entity* ent = world->create();
		ent->assign<TextUI_Component>(L"상점",
			(float)FRAME_BUFFER_HEIGHT / 2 + 200, (float)FRAME_BUFFER_WIDTH / 2 - 400,
			(float)FRAME_BUFFER_HEIGHT / 2 + 280, (float)FRAME_BUFFER_WIDTH / 2 + 400);
	}
	break;

	case EQUIPMENT:
	{
		world->reset();
		Entity* ent = world->create();
		ent->assign<TextUI_Component>(L"장비창",
			(float)FRAME_BUFFER_HEIGHT / 2 + 200, (float)FRAME_BUFFER_WIDTH / 2 - 400,
			(float)FRAME_BUFFER_HEIGHT / 2 + 280, (float)FRAME_BUFFER_WIDTH / 2 + 400);
	}
	break;



	case GAME:
	{
		world->reset();

		m_pPawn = AddPlayerEntity(world->create(), m_pd3dDevice, m_pd3dCommandList,
			m_pObjectManager,
			1014.f, m_pObjectManager->m_pTerrain->GetHeight(1014.f, 1429.f)/*2000.f*/, 1429.0f,
			0.f, 0.f, 0.f,
			6.0f, 6.0f, 6.0f,
			SOLDIER);
		m_pPawn->assign<BoundingBox_Component>(5.f, 10.f, 5.f);


		cout << m_pPawn->get<player_Component>()->id << endl;
		Entity* ent = world->create();
		ent->assign<SkyBox_Component>(m_pObjectManager->m_pSkyBox, "default");

		BuildScene(world, (char*)"Scene/Scene.bin");

		ent = AddAnotherEntity(world->create(), m_pd3dDevice, m_pd3dCommandList,
			m_pObjectManager,
			1014.f, m_pObjectManager->m_pTerrain->GetHeight(1014.f, 1429.f), 1429.0f,
			0.f, 90.f, 0.f,
			25.f, 25.f, 25.f,
			MONSTER);
		auto monster_id = ent->get<player_Component>();
		ent->assign<BoundingBox_Component>(30.f, 30.f, 30.f);

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
			m_pObjectManager,
			310.0f, m_pObjectManager->m_pTerrain->GetHeight(310.0f, 600.0f), 600.0f,
			0.f, 0.f, 0.f,
			6.0f, 6.0f, 6.0f,
			SOLDIER);

		AddAnotherEntity(world->create(), m_pd3dDevice, m_pd3dCommandList,
			m_pObjectManager,
			310.0f, m_pObjectManager->m_pTerrain->GetHeight(310.0f, 600.0f), 600.0f,
			0.f, 0.f, 0.f,
			6.0f, 6.0f, 6.0f,
			SOLDIER);
		AddAnotherEntity(world->create(), m_pd3dDevice, m_pd3dCommandList,
			m_pObjectManager,
			310.0f, m_pObjectManager->m_pTerrain->GetHeight(310.0f, 600.0f), 600.0f,
			0.f, 0.f, 0.f,
			6.0f, 6.0f, 6.0f,
			SOLDIER);

		CCamera* temp = new CThirdPersonCamera(NULL);
		temp->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);

		ComponentHandle<Camera_Component> camera = m_pPawn->assign<Camera_Component>(temp);
		camera->m_pCamera->SetPosition(XMFLOAT3(310.0f,
			m_pObjectManager->m_pTerrain->GetHeight(310.0f, 600.0f) + 10.f, 600.0f - 30.f));

		world->emit<GetPlayerPtr_Event>({ m_pPawn });

	}
		break;
	case END:
		Entity* ent = world->create();
		ent->assign<TextUI_Component>(L"return lobby, press enter",
			(float)FRAME_BUFFER_HEIGHT / 2 + 200, (float)FRAME_BUFFER_WIDTH / 2 - 400,
			(float)FRAME_BUFFER_HEIGHT / 2 + 280, (float)FRAME_BUFFER_WIDTH / 2 + 400);
		wchar_t m_reportFileName[10];
		ent = world->create();
		ent->assign<TextUI_Component>(to_wstring(event.score),
			(float)FRAME_BUFFER_HEIGHT / 2 -20, (float)FRAME_BUFFER_WIDTH / 2 - 400,
			(float)FRAME_BUFFER_HEIGHT / 2 + 60, (float)FRAME_BUFFER_WIDTH / 2 + 400);
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

			xmf4x4World._41 += 2783.f;
			xmf4x4World._42 += 1024.f;
			xmf4x4World._43 += 1852.f;

			ent->assign<Position_Component>(xmf4x4World);


			ent->assign<Model_Component>(m_pObjectManager->Get_ModelInfo(pstrGameObjectName),
				m_pObjectManager->Get_ModelInfo(pstrGameObjectName)->m_pModelRootObject->m_pstrFrameName);
		}
		else
			i -= 1;
	}
	::fclose(pFile);
}
