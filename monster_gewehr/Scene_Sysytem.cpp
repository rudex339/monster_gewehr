#include "stdafx.h"
#include "Scene_Sysytem.h"
#include "Player_Entity.h"
#include "Object_Entity.h"
#include "ObjectManager.h"
#include "Sever_Sysyem.h"
#include "PlayerControl_System.h"
#include "Render_Sysytem.h"

#define I_BG_WIDTH 1920
#define I_BG_HEIGHT 1152

#define I_TITLE_W 1127
#define I_TITLE_H 676

#define I_LOGIN_W 1000
#define I_LOGIN_H 563

#define I_FACE_W 100
#define I_FACE_H 100

#define I_FRAME_W 1400
#define I_FRAME_H 900

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
	world->subscribe<EnterRoom_Event>(this);
	world->subscribe<LoginCheck_Event>(this);
	world->subscribe<ChoiceRoom_Event>(this);
	world->subscribe<ChoiceItem_Event>(this);
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
				cout << "실행됨22" << endl;
				world->emit<InputId_Event>({});
				if (loginCheck)
					world->emit< ChangeScene_Event>({ LOBBY });
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
			else if (pKeysBuffer[VK_BACK] & 0xF0) {
				world->emit<Quit_Room>({});
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
		initSelect();
		world->reset();
		m_pPawn = NULL;
		world->reset();
		world->emit<SetCamera_Event>({ NULL });

		Entity* ent = world->create();

		D2D1_RECT_F imageRect, screenRect;
		imageRect = { 0, 0, 1125, 675 };
		screenRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };

		ent->assign<ImageUI_Component>(L"image/title.png", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			screenRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

		D2D1_RECT_F sRect[4] = {
					{FRAME_BUFFER_WIDTH / 2 - FRAME_BUFFER_WIDTH / 10 , FRAME_BUFFER_HEIGHT / 2 - FRAME_BUFFER_HEIGHT / 15, FRAME_BUFFER_WIDTH / 2 + FRAME_BUFFER_WIDTH / 10 , FRAME_BUFFER_HEIGHT / 2 - FRAME_BUFFER_HEIGHT / 15 +  FRAME_BUFFER_HEIGHT / 10}};
		float width = FRAME_BUFFER_WIDTH / 10;
		float y1 = FRAME_BUFFER_HEIGHT / 2 - FRAME_BUFFER_HEIGHT / 15;
		float height = FRAME_BUFFER_HEIGHT / 10;
		float offset = FRAME_BUFFER_HEIGHT / 45;

		sRect[0].left = FRAME_BUFFER_WIDTH / 2 - width;
		sRect[0].right = FRAME_BUFFER_WIDTH / 2 + width;
		sRect[0].top = FRAME_BUFFER_HEIGHT / 2;
		sRect[0].bottom = sRect[0].top + height;

		for (int i = 1; i < 4; ++i) {
			sRect[i].left = sRect[i - 1].left;
			sRect[i].right = sRect[i - 1].right;

			sRect[i].top = sRect[i - 1].bottom + offset;
			sRect[i].bottom = sRect[i].top + height;
		}
		
		imageRect = { 0, 0, 340, 70 };

		ent = world->create();
		ent->assign<Button_Component>(GameStartBtn, L"image/null.png", NEEDLE_FONT, L"Connect", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			sRect[0], 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);
		

		ent = world->create();
		ent->assign<Button_Component>(ShopBtn, L"image/null.png", NEEDLE_FONT, L"Shop", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			sRect[1], 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

		ent = world->create();
		ent->assign<Button_Component>(EquipBtn, L"image/null.png", NEEDLE_FONT, L"Equipment", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			sRect[2], 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

		// 버튼 비활성화 테스트 임시로 장비창에 가는 버튼을 비활성화 했다.
		/*ent = world->create();
		Button_Component test = Button_Component(EquipBtn, L"image/null.png", NEEDLE_FONT, L"Equipment", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			sRect[2], 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);
		test.Disable();
		ent->assign<Button_Component>(test);*/


		ent = world->create();
		ent->assign<Button_Component>(ExitBtn, L"image/null.png", NEEDLE_FONT, L"EXIT", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			sRect[3], 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);


	}
	break;

	case ROOMS:
	{
		world->reset();
		Entity* ent = world->create();

		D2D1_RECT_F imageRect, screenRect;
		D2D1_RECT_F sRect;
		imageRect = { 0, 0, 1920, 1152 };
		screenRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };


		ent->assign<TextUI_Component>(NEEDLE_FONT, L"ROOM LIST",
			FRAME_BUFFER_HEIGHT / 20, FRAME_BUFFER_WIDTH / 50,FRAME_BUFFER_HEIGHT / 10, FRAME_BUFFER_WIDTH / 5);

		ent = world->create();
		ent->assign<ImageUI_Component>(L"image/bg.jpg", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			screenRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);
		
		
		// 방 목록
		{
			ent = world->create();
			imageRect = { 0, 0, 1400, 900 };
			sRect = { FRAME_BUFFER_WIDTH / 20, FRAME_BUFFER_HEIGHT / 8, FRAME_BUFFER_WIDTH * 13 / 20, FRAME_BUFFER_HEIGHT * 9 / 10 };
			ent->assign<ImageUI_Component>(L"image/silver_frame.png", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
				sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

		}

		// 방에 대한 설명
		{
			if (m_room_num >= 0) {
				// 방 설명창 프레임
				ent = world->create();
				imageRect = { 0, 0, 1400, 900 };
				sRect = { FRAME_BUFFER_WIDTH * 14 / 20, FRAME_BUFFER_HEIGHT / 8, FRAME_BUFFER_WIDTH * 19 / 20, FRAME_BUFFER_HEIGHT * 7 / 10 };
				ent->assign<ImageUI_Component>(L"image/silver_frame.png", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
					sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

				// 게임모드 이미지
				ent = world->create();
				imageRect = { 0, 0, 1000, 563 };
				float margin = FRAME_BUFFER_WIDTH / 200;
				sRect = { FRAME_BUFFER_WIDTH * 14 / 20 + margin, FRAME_BUFFER_HEIGHT / 8 + margin, FRAME_BUFFER_WIDTH * 19 / 20 - margin, FRAME_BUFFER_HEIGHT * 3.5f / 10 - margin };
				ent->assign<ImageUI_Component>(L"image/monster_hunter_login.png", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
					sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);


				// 게임방 번호 출력
				ent = world->create();
				sRect = { FRAME_BUFFER_WIDTH * 14 / 20 + margin, FRAME_BUFFER_HEIGHT * 3.7f / 10, FRAME_BUFFER_WIDTH * 19 / 20 , FRAME_BUFFER_HEIGHT * 3.9 / 10 };
				TextUI_Component text = TextUI_Component(SMALL_FONT, L"ROOM NO. " + to_wstring(m_room_num), sRect.top, sRect.left, sRect.bottom, sRect.right);

				text.m_paragraph_alignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
				text.m_text_alignment = DWRITE_TEXT_ALIGNMENT_JUSTIFIED;

				ent->assign<TextUI_Component>(text);

				// 유저 정보 출력 (지금은 임시로 출력하지만 나중에 이름이랑 무기를 서버에서 가져와서 출력할 예정)
				sRect = { FRAME_BUFFER_WIDTH * 14 / 20 + margin, FRAME_BUFFER_HEIGHT * 4 / 10, FRAME_BUFFER_WIDTH * 19 / 20, FRAME_BUFFER_HEIGHT * 14 / 30 };
				for (auto& player : RoomPlayers) {
					ent = world->create();
					text = TextUI_Component(SMALL_FONT, player.name , sRect.top, sRect.left, sRect.bottom, sRect.right);
					text.m_paragraph_alignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
					text.m_text_alignment = DWRITE_TEXT_ALIGNMENT_JUSTIFIED;
					ent->assign<TextUI_Component>(text);

					ent = world->create();
					text = TextUI_Component(SMALL_FONT, to_wstring(player.weapon) + L" ", sRect.top, sRect.left, sRect.bottom, sRect.right);
					text.m_paragraph_alignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
					text.m_text_alignment = DWRITE_TEXT_ALIGNMENT_TRAILING;
					ent->assign<TextUI_Component>(text);

					sRect.top = sRect.bottom;
					sRect.bottom += FRAME_BUFFER_HEIGHT / 15;
				}
			}
		}

		// 방생성 & 방입장 버튼
		{
			imageRect = { 0, 0, 1000, 563 };
			sRect = { FRAME_BUFFER_WIDTH * 14 / 20, FRAME_BUFFER_HEIGHT * 15 / 20, FRAME_BUFFER_WIDTH * 19 / 20, FRAME_BUFFER_HEIGHT * 16 / 20 };

			ent = world->create();
			ent->assign<Button_Component>(MakeRoomBtn, L"image/monster_hunter_login.png", DEFAULT_FONT, L"방생성", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
				sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

			sRect = { FRAME_BUFFER_WIDTH * 14 / 20, FRAME_BUFFER_HEIGHT * 17 / 20, FRAME_BUFFER_WIDTH * 19 / 20, FRAME_BUFFER_HEIGHT * 18 / 20 };
			ent = world->create();
			Button_Component joinBtn = Button_Component(JoinRoomBtn, L"image/monster_hunter_login.png", DEFAULT_FONT, L"방입장", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
				sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

			cout << m_room_num << endl;

			if (m_room_num >= 0) {
				joinBtn.Activate();
			}
			else {
				joinBtn.Disable();
			}
			ent->assign<Button_Component>(joinBtn);

		}

		// 생성된 방들만큼 버튼을 추가
		for (auto& Room : Rooms) {
			ent = world->create();
			ent->assign<Button_Component>(Room);
		}
	}
	break;
	
	case SHOP:
	{
		wstring imagefiles[10] = {
				L"image/M4.png",
				L"image/Saiga12.png",
				L"image/M24.png",
				L"image/LightArmor.png",
				L"image/HeavyArmor.png",
				L"image/Grenade.png",
				L"image/Flashbang.png",
				L"image/Bandage.png",
				L"image/FirstAidKit.png",
				L"image/Flashbang.png"
		};

		world->reset();
		Entity* ent = world->create();

		D2D1_RECT_F imageRect, screenRect, sRect, itemRects[2][5];

		imageRect = { 0, 0, 1920, 1152 };
		screenRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };

		ent->assign<ImageUI_Component>(L"image/bg.jpg", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			screenRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);


		ent = world->create();
		ent->assign<TextUI_Component>(NEEDLE_FONT, L"SHOP",
			FRAME_BUFFER_HEIGHT / 20, FRAME_BUFFER_WIDTH / 50, FRAME_BUFFER_HEIGHT / 10, FRAME_BUFFER_WIDTH / 5);

		{
			// 상인 프레임 + 이미지
			ent = world->create();
			imageRect = { 0, 0, 1400, 900 };
			sRect = { FRAME_BUFFER_WIDTH / 20, FRAME_BUFFER_HEIGHT / 8, FRAME_BUFFER_WIDTH * 7 / 20, FRAME_BUFFER_HEIGHT * 7 / 8 };
			ent->assign<ImageUI_Component>(L"image/silver_frame.png", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
				sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

			ent = world->create();
			imageRect = { 0, 0, 700, 1100 };
			sRect = { FRAME_BUFFER_WIDTH / 20 + 3.5, FRAME_BUFFER_HEIGHT / 8 + 5.5f, FRAME_BUFFER_WIDTH * 7 / 20 - 3.5f, FRAME_BUFFER_HEIGHT * 7 / 8 - 5.5f };
			ent->assign<ImageUI_Component>(L"image/Mechanic.png", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
				sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);
		}

		{
			// 무기 및 아이템 프레임 및 이미지
			ent = world->create();
			imageRect = { 0, 0, 1400, 900 };
			sRect = { FRAME_BUFFER_WIDTH * 13/ 20, FRAME_BUFFER_HEIGHT / 10, FRAME_BUFFER_WIDTH * 19 / 20, FRAME_BUFFER_HEIGHT * 9 / 10 };
			ent->assign<ImageUI_Component>(L"image/silver_frame.png", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
				sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);


			float margin = 1.0f;
			float height = FRAME_BUFFER_HEIGHT * 1.6 / 10;
			itemRects[0][0] = { FRAME_BUFFER_WIDTH * 13 / 20 + margin, FRAME_BUFFER_HEIGHT / 10 + margin, FRAME_BUFFER_WIDTH * 16 / 20 - margin, FRAME_BUFFER_HEIGHT * 2.6f / 10 - margin };
			itemRects[1][0] = { FRAME_BUFFER_WIDTH * 16 / 20 + margin, FRAME_BUFFER_HEIGHT / 10 + margin, FRAME_BUFFER_WIDTH * 19 / 20 - margin, FRAME_BUFFER_HEIGHT * 2.6f / 10 - margin };
			for (int i = 1; i < 5; ++i) {
				itemRects[0][i] = itemRects[0][i - 1];
				itemRects[0][i].top = itemRects[0][i - 1].bottom + margin*2;
				itemRects[0][i].bottom = itemRects[0][i].top + height - margin;

				itemRects[1][i] = itemRects[1][i - 1];
				itemRects[1][i].top = itemRects[1][i - 1].bottom + margin * 2;
				itemRects[1][i].bottom = itemRects[1][i].top + height - margin;
			}

			imageRect = { 0, 0, 500, 300 };
			for (int i = 0; i < 2; ++i) {
				for (int j = 0; j < 5; ++j) {
					ent = world->create();
					ent->assign<Button_Component>(ItemBtn, imagefiles[j + 5 * i].c_str(), DEFAULT_FONT, L"", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
						itemRects[i][j], 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect, j + 5*i);

					ent = world->create();
					Button_Component info;
					if(i == 0)
						info = Button_Component(ItemBtn, imagefiles[j + 5 * i].c_str(), SMALL_FONT, L"+" + to_wstring(m_item_info[j + 5 * i]), m_d2dDeviceContext, m_d2dFactory, m_bitmap,
							itemRects[i][j], 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect, j + 5 * i);
					else
						info = Button_Component(ItemBtn, imagefiles[j + 5 * i].c_str(), SMALL_FONT, to_wstring(m_item_info[j + 5 * i]) + L"개", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
							itemRects[i][j], 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect, j + 5 * i);

					info.m_paragraph_alignment = DWRITE_PARAGRAPH_ALIGNMENT_FAR;
					info.m_text_alignment = DWRITE_TEXT_ALIGNMENT_TRAILING;

					ent->assign<Button_Component>(info);
				}
			}
		}

		{
			// 선택된 아이템 및 강화 버튼
			ent = world->create();
			imageRect = { 0, 0, 1400, 900 };
			sRect = { FRAME_BUFFER_WIDTH * 8 / 20, FRAME_BUFFER_HEIGHT * 2 / 8, FRAME_BUFFER_WIDTH * 12 / 20, FRAME_BUFFER_HEIGHT * 7 / 8 };
			ent->assign<ImageUI_Component>(L"image/silver_frame.png", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
				sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);
			
			float margin = 1.0f;
			float height = FRAME_BUFFER_HEIGHT * 3.2 / 15;

			

			if (m_item_num >= 0) {
				//선택한 아이템이 있는 경우만 이미지를 출력한다.
				imageRect = { 0, 0, 500, 300 };
				sRect.left = sRect.left + margin;
				sRect.right = sRect.right - margin;
				sRect.top = sRect.top + margin;
				sRect.bottom = sRect.top + height - margin;

				ent = world->create();
				ent->assign<ImageUI_Component>(imagefiles[m_item_num].c_str(), m_d2dDeviceContext, m_d2dFactory, m_bitmap,
					sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

				
			}

			// 강화 정보 표시
			ent = world->create();
			ent->assign<TextUI_Component>(NEEDLE_FONT, L"SHOP",
				FRAME_BUFFER_HEIGHT / 20, FRAME_BUFFER_WIDTH / 50, FRAME_BUFFER_HEIGHT / 10, FRAME_BUFFER_WIDTH / 5);


			// 구매/강화 버튼
			ent = world->create();

			imageRect = { 0, 0, 1000, 563 };
			sRect = { FRAME_BUFFER_WIDTH * 8 / 20 + margin, FRAME_BUFFER_HEIGHT * 12 / 16 , FRAME_BUFFER_WIDTH * 12 / 20 - margin, FRAME_BUFFER_HEIGHT * 14 / 16 - margin };

			Button_Component buyBtn = Button_Component(BuyBtn, L"image/monster_hunter_login.png", DEFAULT_FONT, L"구매/강화", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
				sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);
			buyBtn.Disable();

			if (m_item_num >= 0) buyBtn.Activate();
			ent->assign<Button_Component>(buyBtn);
		}
	}
	break;

	case EQUIPMENT:
	{
		world->reset();
		Entity* ent = world->create();
		
		D2D1_RECT_F imageRect, screenRect;
		imageRect = { 0, 0, 1920, 1152 };
		screenRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };

		ent->assign<ImageUI_Component>(L"image/bg.jpg", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			screenRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

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
		auto box = m_pPawn->assign<BoundingBox_Component>(2.f, 6.f, 2.f);
		box->m_pMesh = new CBoxMesh(m_pd3dDevice, m_pd3dCommandList, &box->m_bounding_box);


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
		ent->assign<TextUI_Component>(DEFAULT_FONT, L"return lobby, press enter",
			(float)FRAME_BUFFER_HEIGHT / 2 + 200, (float)FRAME_BUFFER_WIDTH / 2 - 400,
			(float)FRAME_BUFFER_HEIGHT / 2 + 280, (float)FRAME_BUFFER_WIDTH / 2 + 400);
		wchar_t m_reportFileName[10];
		ent = world->create();
		ent->assign<TextUI_Component>(DEFAULT_FONT, to_wstring(event.score),
			(float)FRAME_BUFFER_HEIGHT / 2 -20, (float)FRAME_BUFFER_WIDTH / 2 - 400,
			(float)FRAME_BUFFER_HEIGHT / 2 + 60, (float)FRAME_BUFFER_WIDTH / 2 + 400);
		world->emit<GetPlayerPtr_Event>({ NULL });
		break;
	}
	
}

void Scene_Sysytem::receive(World* world, const EnterRoom_Event& event)
{
	m_State = event.State;
	m_room_num = event.room_num;

	world->reset();
	Entity* ent = world->create();
	ent->assign<TextUI_Component>(DEFAULT_FONT, to_wstring(m_room_num) + L"번 방",
		(float)FRAME_BUFFER_HEIGHT / 2 + 200, (float)FRAME_BUFFER_WIDTH / 2 - 400,
		(float)FRAME_BUFFER_HEIGHT / 2 + 280, (float)FRAME_BUFFER_WIDTH / 2 + 400);

}

void Scene_Sysytem::receive(World* world, const LoginCheck_Event& event)
{
	loginCheck = event.logincheck;
}

void Scene_Sysytem::receive(World* world, const ChoiceRoom_Event& event)
{
	m_room_num = event.room_num;
	world->emit<ChangeScene_Event>({ ROOMS });
}

void Scene_Sysytem::receive(World* world, const ChoiceItem_Event& event)
{
	m_item_num = event.item_num;
	world->emit<ChangeScene_Event>({ SHOP });
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

			m_pObjectManager->Get_ModelInfo(pstrGameObjectName)->m_pModelRootObject->m_pMesh->m_xmf3AABBExtents;
			


			if (xmf4x4World._42 < 1054.f) {
				cout << xmf4x4World._42 << "\n";
				auto box = ent->assign<BoundingBox_Component>(
					m_pObjectManager->Get_ModelInfo(pstrGameObjectName)->m_pModelRootObject->m_pMesh->m_xmf3AABBExtents,
					m_pObjectManager->Get_ModelInfo(pstrGameObjectName)->m_pModelRootObject->m_pMesh->m_xmf3AABBCenter);
				// XMFLOAT4X4를 XMMATRIX로 변환
				XMMATRIX worldMatrix = XMLoadFloat4x4(&xmf4x4World);

				// BoundingOrientedBox 변환
				box->m_bounding_box.Transform(box->m_bounding_box, worldMatrix);

				box->m_pMesh = new CBoxMesh(m_pd3dDevice, m_pd3dCommandList, &box->m_bounding_box);
			}
			//box->m_bounding_box.Extents *= m_pObjectManager->Get_ModelInfo(pstrGameObjectName)->m_pModelRootObject->m_pMesh->m_xmf3AABBExtents;

		}
		else
			i -= 1;
	}
	::fclose(pFile);
}

void Scene_Sysytem::AddRoom(int room_num)
{
	int num = room_num;
	D2D1_RECT_F sRect, imageRect;

	// FRAME_BUFFER_WIDTH / 20, FRAME_BUFFER_HEIGHT / 8, FRAME_BUFFER_WIDTH * 13 / 20, FRAME_BUFFER_HEIGHT * 9 / 10
	sRect = { FRAME_BUFFER_WIDTH / 20 + 10.0f, 
		FRAME_BUFFER_HEIGHT / 8 + (float)num * FRAME_BUFFER_HEIGHT / 7 + 20.0f, 
		FRAME_BUFFER_WIDTH * 13 / 20 - 10.0f, 
		FRAME_BUFFER_HEIGHT / 8 + (float)(num+1)*FRAME_BUFFER_HEIGHT / 7 + 10.0f };
	imageRect = { 0, 0, 1000, 563 };

	Rooms.push_back(Button_Component(RoomBtn, L"image/monster_hunter_login.png", DEFAULT_FONT, to_wstring(num) + L"번 방", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
		sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect, num));
}

void Scene_Sysytem::AddRoomPlayers(wstring name, int weapon)
{
	Player_Info info;
	info.name = name;
	info.weapon = weapon;
	if (RoomPlayerNames.find(name) == RoomPlayerNames.end()) {
		RoomPlayerNames.insert(name);
		RoomPlayers.push_back({ name, weapon });
	}
}

void Scene_Sysytem::InitRoomPlayers()
{
	RoomPlayerNames.clear();
	RoomPlayers.clear();
}

void Scene_Sysytem::initSelect()
{
	m_item_num = -1;
	m_item_num = -1;
}
