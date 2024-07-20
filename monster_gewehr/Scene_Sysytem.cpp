#include "stdafx.h"
#include "Scene_Sysytem.h"
#include "Player_Entity.h"
#include "Object_Entity.h"
#include "ObjectManager.h"
#include "Sever_Sysyem.h"
#include "Collision_Sysytem.h"
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
	world->subscribe<Refresh_Scene>(this);
	world->subscribe<ChoiceEquip_Event>(this);
	world->subscribe<CreateObject_Event>(this);
	world->subscribe<StartRoom_Event>(this);
	world->subscribe<Ready_Event>(this);
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
				//world->emit< ChangeScene_Event>({ GAME });
				//world->emit<Game_Start>({});
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
				// 나갈때 내가 어떤 장비를 선택했는지 서버에 알려줄거임
				world->emit<Set_Equipment>({ (char)equipments[0], (char)equipments[1], (char)equipments[2] });
				world->emit< ChangeScene_Event>({ LOBBY });
			}
			break;
		case GAME:
			break;
		case END:
			if (pKeysBuffer[VK_RETURN] & 0xF0) {
				world->emit<Clearlayer_Event>({});
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
		ent->assign<Button_Component>(RoomBtn, L"image/null.png", NEEDLE_FONT, L"Connect", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
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
		// 방에 들어가 있을때 가지고 있던 정보들 다 초기화
		m_join_room = -1;
		m_is_host = false;
		InitInRoomPlayers();
		//----------------------------------

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


			if (m_room_num >= 0 && RoomPlayers.size() < 4) {
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
	case INROOM:
	{
		world->emit<EnterRoom_Event>({ INROOM, m_room_num, m_is_host });
	}
	break;
	case SHOP:
	{
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

				// 강화와 관련된 정보 표시 
				// 비용이나 텍스트는 추후 추가 예정
				ent = world->create();
				TextUI_Component text = TextUI_Component(SMALL_FONT, L"비용 : " + to_wstring(100), FRAME_BUFFER_HEIGHT * 2 / 8 + height - margin, FRAME_BUFFER_WIDTH * 8 / 20, FRAME_BUFFER_WIDTH * 8 / 20 + margin, FRAME_BUFFER_WIDTH * 12 / 20);
				text.m_paragraph_alignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
				text.m_text_alignment = DWRITE_TEXT_ALIGNMENT_CENTER;
				ent->assign<TextUI_Component>(text);

			}

			


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

		D2D1_RECT_F imageRect, sRect;
		int selected[2] = { 0, 0 };

		ent = world->create();
		ent->assign<TextUI_Component>(NEEDLE_FONT, L"EQUIPMENT",
			FRAME_BUFFER_HEIGHT / 20, FRAME_BUFFER_WIDTH / 50, FRAME_BUFFER_HEIGHT / 10, FRAME_BUFFER_WIDTH / 5);

		{ // 플레이어를 그린다

			ent = AddAnotherEntity(world->create(), m_pd3dDevice, m_pd3dCommandList,
				m_pObjectManager,
				-12.f, -8.0f, 20.0f,
				0.f, 145.f, 0.f,
				6.0f, 6.0f, 6.0f,
				SOLDIER);

			ent->get<player_Component>()->id = 1;

			auto& weapon = ent->get<Model_Component>().get().m_pchildObjects.begin(); 
			for (int i = 0; i < 3; ++i) {
				if (i == equipments[0]) {
					weapon[i]->draw = true;
				}
				else weapon[i]->draw = false;
			}
			

			CCamera* temp = new CThirdPersonCamera(NULL);
			temp->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);

			ComponentHandle<Camera_Component> camera = ent->assign<Camera_Component>(temp);

			world->emit<SetCamera_Event>({ ent->get<Camera_Component>().get().m_pCamera });

		}

		{// 무기 및 방어구 선택창

			imageRect = { 0, 0, 500, 300 };
			sRect = { FRAME_BUFFER_WIDTH * 24 / 40, FRAME_BUFFER_HEIGHT * 2 / 32 , FRAME_BUFFER_WIDTH * 30 / 40, FRAME_BUFFER_HEIGHT * 7 / 32 };

			float height = FRAME_BUFFER_HEIGHT * 5 / 32;
			float y_margin = FRAME_BUFFER_HEIGHT * 2 / 32;
			float x_margin = FRAME_BUFFER_WIDTH / 40;

			float button_width = FRAME_BUFFER_WIDTH * 2 / 40;

			for (int i = 0; i < 4; ++i) {
				D2D1_RECT_F Rect;
				// 중앙 이미지
				ent = world->create();
				ent->assign<ImageUI_Component>(imagefiles[equipments[i]].c_str(), m_d2dDeviceContext, m_d2dFactory, m_bitmap,
					sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

				// 왼쪽 버튼
				ent = world->create();
				Rect = { sRect.left - x_margin - button_width, sRect.top, sRect.left - x_margin, sRect.bottom };
				ent->assign<Button_Component>(EquipLeftBtn, L"image/monster_hunter_login.png", DEFAULT_FONT, L"", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
					Rect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect, i);

				// 오른쪽 버튼
				ent = world->create();
				Rect = { sRect.right + x_margin, sRect.top, sRect.right + x_margin + button_width, sRect.bottom };
				ent->assign<Button_Component>(EquipRightBtn, L"image/monster_hunter_login.png", DEFAULT_FONT, L"", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
					Rect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect, i);

				if (i == 3) break;
				sRect = { sRect.left , sRect.bottom + y_margin, sRect.right, sRect.bottom + y_margin + height };
			}

			// 힐 아이템 정보 및 추가/제거
			ent = world->create();
			TextUI_Component healtem = TextUI_Component(SMALL_FONT, to_wstring(equipHealItems[choicedHealItem]) , sRect.top, sRect.left, sRect.bottom, sRect.right);

			healtem.m_paragraph_alignment = DWRITE_PARAGRAPH_ALIGNMENT_FAR;
			healtem.m_text_alignment = DWRITE_TEXT_ALIGNMENT_TRAILING;
			ent->assign<TextUI_Component>(healtem);

			sRect = { sRect.left , sRect.bottom + x_margin / 4, sRect.right, sRect.bottom + x_margin / 4 + height / 4 };
			D2D1_RECT_F Rect = { sRect.left, sRect.top, sRect.right - ((sRect.right - sRect.left) / 2), sRect.bottom };


			ent = world->create();
			Button_Component downBtn = Button_Component(EquipDownBtn, L"image/null.png", DEFAULT_FONT, L"ㅡ", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
				Rect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);
			
			if (equipHealItems[choicedHealItem] == 0) {
				downBtn.Disable();
			}
			else {
				downBtn.Activate();
			}
			ent->assign<Button_Component>(downBtn);


			sRect = { Rect.right, sRect.top, sRect.right, sRect.bottom};
			ent = world->create();
			Button_Component upBtn = Button_Component(EquipUpBtn, L"image/null.png", DEFAULT_FONT, L"+", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
				sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);

			

			switch (choicedHealItem)
			{
			case 0:
				upBtn.Activate();
				if (equipHealItems[0] == 9) {
					upBtn.Disable();
				}
				break;
			case 1:
				upBtn.Activate();
				if (equipHealItems[1] == 4) {
					upBtn.Disable();
				}
				break;
			case 2:
				upBtn.Activate();
				if (equipHealItems[2] == 2) {
					upBtn.Disable();
				}
				break;
			default:
				break;
			}

			ent->assign<Button_Component>(upBtn);
		}

	}
	break;

	case GAME:
	{
		world->reset();

		m_pPawn = AddPlayerEntity(world->create(), m_pd3dDevice, m_pd3dCommandList,
			m_pObjectManager,
			1014.f, m_pObjectManager->m_pTerrain->GetHeight(1014.f, 1429.f) + 10.f/*2000.f*/ - 8.0f , 1429.0f,
			0.f, 0.f, 0.f,
			6.0f, 6.0f, 6.0f,
			SOLDIER);
		auto box = m_pPawn->assign<BoundingBox_Component>(2.f, 6.f, 2.f);
		box->m_pMesh = new CBoxMesh(m_pd3dDevice, m_pd3dCommandList, &box->m_bounding_box);
		world->emit<AddObjectlayer_Event>({"Player",m_pPawn});

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
		box->m_pMesh = new CBoxMesh(m_pd3dDevice, m_pd3dCommandList, &box->m_bounding_box);
		world->emit<AddObjectlayer_Event>({ "Monster",ent });

		monster_id->id = -2;

		ent = world->create();
		auto light = ent->assign<Light_Component>();
		light->m_pLight = new LIGHT;
		::ZeroMemory(light->m_pLight, sizeof(LIGHT));
		light->m_pLight->m_bEnable = true;
		light->m_pLight->m_nType = DIRECTIONAL_LIGHT;
		light->m_pLight->m_xmf4Ambient = XMFLOAT4(0.3f, 0.2f, 0.1f, 0.5f);
		light->m_pLight->m_xmf4Diffuse = XMFLOAT4(0.8f, 0.7f, 0.6f, 1.0f);
		light->m_pLight->m_xmf4Specular = XMFLOAT4(1.0f, 0.9f, 0.8f, 0.0f);
		light->m_pLight->m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);

		//ent = world->create();
		//light = ent->assign<Light_Component>();
		//light->m_pLight = new LIGHT;
		//light->m_pLight->m_bEnable = true;
		//light->m_pLight->m_nType = SPOT_LIGHT;
		//light->m_pLight->m_fRange = 400.0f;
		//light->m_pLight->m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		//light->m_pLight->m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
		//light->m_pLight->m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
		//light->m_pLight->m_xmf3Position = XMFLOAT3(1646.0f, 1164.0f, 2246.0f);
		//light->m_pLight->m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
		//light->m_pLight->m_xmf3Attenuation = XMFLOAT3(0.5f, 0.01f, 0.0001f);
		//light->m_pLight->m_fFalloff = 8.0f;
		//light->m_pLight->m_fPhi = (float)cos(XMConvertToRadians(40.0f));
		//light->m_pLight->m_fTheta = (float)cos(XMConvertToRadians(20.0f));



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
			m_pObjectManager->m_pTerrain->GetHeight(310.0f, 600.0f) , 600.0f - 30.f));

		world->emit<GetPlayerPtr_Event>({ m_pPawn, true });

	}
		break;
	case END:
		::ReleaseCapture();
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
	m_join_room = m_room_num;
	m_is_host = event.is_host;

	world->reset();
	Entity* ent = world->create();

	D2D1_RECT_F imageRect;
	imageRect = { 0, 0, 340, 70 };

	D2D1_RECT_F sRect;

	float width = FRAME_BUFFER_WIDTH / 10;
	float height = FRAME_BUFFER_HEIGHT / 10;
	float offset = FRAME_BUFFER_HEIGHT / 45;

	sRect.left = FRAME_BUFFER_WIDTH / 2 - width;
	sRect.right = FRAME_BUFFER_WIDTH / 2 + width;
	sRect.top = FRAME_BUFFER_HEIGHT / 2;
	sRect.bottom = sRect.top + height;

	//world->reset();
	//Entity* ent = world->create();

	//D2D1_RECT_F imageRect, sRect;
	int selected[2] = { 0, 0 };

	ent = world->create();
	ent->assign<TextUI_Component>(NEEDLE_FONT, L"ROOM NO. " + to_wstring(m_room_num),
		FRAME_BUFFER_HEIGHT / 20, FRAME_BUFFER_WIDTH / 50, FRAME_BUFFER_HEIGHT / 10, FRAME_BUFFER_WIDTH / 5);

	{ // 플레이어를 그린다
		// 나 자신 그리기
		ent = AddAnotherEntity(world->create(), m_pd3dDevice, m_pd3dCommandList,
			m_pObjectManager,
			-12.f, -8.0f, 20.0f,
			0.f, 145.f, 0.f,
			6.0f, 6.0f, 6.0f,
			SOLDIER);

		ent->get<player_Component>()->id = m_id;

		auto& my_weapon = ent->get<Model_Component>().get().m_pchildObjects.begin();
		for (int i = 0; i < 3; ++i) {
			if (i == equipments[0]) {
				my_weapon[i]->draw = true;
			}
			else my_weapon[i]->draw = false;
		}

		// // 이름 및 레디 상태 출력
		// 방장이면 MASTER가, 아니라면 READY or 아무것도 안그리기
		if (m_is_host) {
			Entity* ent = world->create();
			TextUI_Component text = TextUI_Component(MEDIUM_FONT, L" HOST",
				FRAME_BUFFER_HEIGHT * 9 / 10, FRAME_BUFFER_WIDTH / 6,
				FRAME_BUFFER_HEIGHT , FRAME_BUFFER_WIDTH * 2 / 6);
			text.m_paragraph_alignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
			text.m_text_alignment = DWRITE_TEXT_ALIGNMENT_JUSTIFIED;

			ent->assign<TextUI_Component>(text);
		}
		else {
			if (m_ready) {
				ent = world->create();
				TextUI_Component text = TextUI_Component(MEDIUM_FONT, L" READY",
					FRAME_BUFFER_HEIGHT * 9 / 10, FRAME_BUFFER_WIDTH / 6,
					FRAME_BUFFER_HEIGHT, FRAME_BUFFER_WIDTH * 2 / 6);
				text.m_paragraph_alignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
				text.m_text_alignment = DWRITE_TEXT_ALIGNMENT_JUSTIFIED;
				ent->assign<TextUI_Component>(text);
			}
		}

		for (int i = 0; i < InRoomPlayers.size(); i++) {
			ent = AddAnotherEntity(world->create(), m_pd3dDevice, m_pd3dCommandList,
				m_pObjectManager,
				roomPlayerAngle[i][0], roomPlayerAngle[i][1], roomPlayerAngle[i][2],
				0.f, roomPlayerSet[i], 0.f,
				6.0f, 6.0f, 6.0f,
				SOLDIER);
			ent->get<player_Component>()->id = InRoomPlayers[i].id;

			auto& weapon = ent->get<Model_Component>().get().m_pchildObjects.begin();
			for (int j = 0; j < 3; ++j) {
				if (j == InRoomPlayers[i].weapon) {
					weapon[j]->draw = true;
				}
				else weapon[j]->draw = false;
			}

			if (InRoomPlayers[i].host) {
				ent = world->create();
				TextUI_Component text = TextUI_Component(MEDIUM_FONT, L"HOST",
					FRAME_BUFFER_HEIGHT * 8 / 10, FRAME_BUFFER_WIDTH * (1.8 + i * 1.4) / 6,
					FRAME_BUFFER_HEIGHT * 9 / 10, FRAME_BUFFER_WIDTH * (3.0 + i * 1.2) / 6);
				ent->assign<TextUI_Component>(text);
			}
			else {
				if (InRoomPlayers[i].ready) {
					ent = world->create();
					TextUI_Component text = TextUI_Component(MEDIUM_FONT, L"READY",
						FRAME_BUFFER_HEIGHT * 8 / 10, FRAME_BUFFER_WIDTH * (1.8 + i * 1.4) / 6,
						FRAME_BUFFER_HEIGHT * 9 / 10, FRAME_BUFFER_WIDTH * (3.0 + i * 1.2) / 6);
					ent->assign<TextUI_Component>(text);
				}
			}
		}

		CCamera* temp = new CThirdPersonCamera(NULL);
		temp->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);

		ComponentHandle<Camera_Component> camera = ent->assign<Camera_Component>(temp);

		world->emit<SetCamera_Event>({ ent->get<Camera_Component>().get().m_pCamera });

	}

	if (event.is_host) {
		ent = world->create();
		ent->assign<Button_Component>(GameStartBtn, L"image/null.png", NEEDLE_FONT, L"GameStart", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);
	}
	else {
		ent = world->create();
		ent->assign<Button_Component>(GameReadyBtn, L"image/null.png", NEEDLE_FONT, L"Ready", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
			sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);
	}
	

}

void Scene_Sysytem::receive(World* world, const LoginCheck_Event& event)
{
	loginCheck = event.logincheck;
	m_id = event.id;
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

void Scene_Sysytem::receive(World* world, const CreateObject_Event& event)
{
	switch (event.object) {
	case GRANADE:
	case FLASHBANG:
		Entity* ent = world->create();
		ent->assign<Position_Component>(event.Position.x, event.Position.y, event.Position.z);
		ent->assign<Rotation_Component>(event.Rotate.x, 90.f, event.Rotate.z);
		ent->assign<Scale_Component>(1.f, 1.f, 1.f);
		ComponentHandle<Velocity_Component> vel = ent->assign<Velocity_Component>();
		DirectX::XMVECTOR rotationVec = DirectX::XMLoadFloat3(&event.Rotate);

		// Convert rotation (in degrees) to radians
		DirectX::XMVECTOR rotationRad = DirectX::XMVectorScale(rotationVec, DirectX::XM_PI / 180.0f);

		// Calculate the forward direction based on rotation
		DirectX::XMVECTOR forward = DirectX::XMVectorSet(0.f, 0.f, 1.f, 0.f); // Assuming forward is along the z-axis
		DirectX::XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(event.Rotate.x - 20.f),
			XMConvertToRadians(event.Rotate.y),
			XMConvertToRadians(event.Rotate.z));

		DirectX::XMVECTOR direction = DirectX::XMVector3TransformNormal(forward, rotationMatrix);

		// Scale the direction by speed
		DirectX::XMVECTOR velocity = DirectX::XMVectorScale(direction, 30.f);
		DirectX::XMStoreFloat3(&vel->m_velocity, velocity);
		string pstrGameObjectName = "BP_building60_SM_wall2_StaticMeshComponent0";
		ent->assign<Model_Component>(m_pObjectManager->Get_ModelInfo(pstrGameObjectName),
			m_pObjectManager->Get_ModelInfo(pstrGameObjectName)->m_pModelRootObject->m_pstrFrameName);
		ComponentHandle<BoundingBox_Component> box = ent->assign<BoundingBox_Component>(
			m_pObjectManager->Get_ModelInfo(pstrGameObjectName)->m_pModelRootObject->m_pMesh->m_xmf3AABBExtents,
			m_pObjectManager->Get_ModelInfo(pstrGameObjectName)->m_pModelRootObject->m_pMesh->m_xmf3AABBCenter);
		ent->assign<Grande_Component>(event.object,20.f,100.f)->coolTime=100.f;

		world->emit<AddObjectlayer_Event>({ "Granade", ent });
		break;
	}
}

void Scene_Sysytem::receive(World* world, const Refresh_Scene& event)
{
	if(m_State == event.State)
		world->emit<ChangeScene_Event>({ m_State });
}

void Scene_Sysytem::receive(World* world, const ChoiceEquip_Event& event)
{
	switch (event.btnType)
	{
	case EquipLeftBtn:
		equipments[event.equipType]--;
		break;
	case EquipRightBtn:
		equipments[event.equipType]++;
		break;
	case EquipUpBtn:
		equipHealItems[choicedHealItem]++;
		// 최대 보유량과 비교해서 보유량 -1 하는 코드 필요
		break;
	case EquipDownBtn:
		equipHealItems[choicedHealItem]--;
		// 최대 보유량에 1 추가하는 코드 필요
		break;

	default:
		break;
	}

	if (equipments[0] < 0) equipments[0] = 2;
	equipments[0] = equipments[0] % 3;

	if (equipments[1] < 3) equipments[1] = 4;
	if (equipments[1] > 4) equipments[1] = 3;
	
	if (equipments[2] < 5) equipments[2] = 6;
	if (equipments[2] > 6) equipments[2] = 5;

	if (equipments[3] < 7) equipments[3] = 9;
	if (equipments[3] > 9) equipments[3] = 7;
	choicedHealItem = equipments[3] - 7;

	world->emit<ChangeScene_Event>({ EQUIPMENT });
}

void Scene_Sysytem::receive(World* world, const StartRoom_Event& event)
{
	int room_num = event.room_num;

	if (room_num == m_join_room) {
		world->emit< ChangeScene_Event>({ GAME });
		world->each<player_Component, Position_Component, Velocity_Component, Rotation_Component>(
			[&](Entity* ent,
				ComponentHandle<player_Component> Player,
				ComponentHandle<Position_Component> Position,
				ComponentHandle< Velocity_Component> Velocity,
				ComponentHandle<Rotation_Component> Rotation) ->
			void {
				if (ent->has<Camera_Component>()) {
					Player->id = event.ply_id;
					auto& weapon = ent->get<Model_Component>().get().m_pchildObjects.begin();
					for (int i = 0; i < 3; ++i) {
						if (i == equipments[0]) {
							weapon[i]->draw = true;
						}
						else weapon[i]->draw = false;
					}
				}
			});
	}
	else {
		auto room = find_if(Rooms.begin(), Rooms.end(), [room_num](const Button_Component& b) {
			return b.m_room_num == room_num;
			});
		if (room != Rooms.end()) {
			room->Disable();
			if (room_num == m_room_num) {
				initSelect();
			}
		}
		world->emit<Refresh_Scene>({ROOMS});
	}
}

void Scene_Sysytem::receive(World* world, const Ready_Event& event)
{
	m_ready = !m_ready;
	world->emit<Refresh_Scene>({ INROOM });
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
				//cout << xmf4x4World._42 << "\n";
				auto box = ent->assign<BoundingBox_Component>(
					m_pObjectManager->Get_ModelInfo(pstrGameObjectName)->m_pModelRootObject->m_pMesh->m_xmf3AABBExtents,
					m_pObjectManager->Get_ModelInfo(pstrGameObjectName)->m_pModelRootObject->m_pMesh->m_xmf3AABBCenter);
				// XMFLOAT4X4를 XMMATRIX로 변환
				if (!strcmp(pstrGameObjectName, "Cube")) {
					xmf4x4World._42 += 2.f;
				}
				
				XMMATRIX worldMatrix = XMLoadFloat4x4(&xmf4x4World);

				// BoundingOrientedBox 변환
				box->m_bounding_box.Transform(box->m_bounding_box, worldMatrix);
				world->emit<AddObjectlayer_Event>({ "Object",ent });

				box->m_pMesh = new CBoxMesh(m_pd3dDevice, m_pd3dCommandList, &box->m_bounding_box);
			}
			//box->m_bounding_box.Extents *= m_pObjectManager->Get_ModelInfo(pstrGameObjectName)->m_pModelRootObject->m_pMesh->m_xmf3AABBExtents;

		}
		else
			i -= 1;
	}
	::fclose(pFile);
}

void Scene_Sysytem::AddRoom(int room_num, bool disable)
{
	int num = room_num;
	D2D1_RECT_F sRect, imageRect;

	// FRAME_BUFFER_WIDTH / 20, FRAME_BUFFER_HEIGHT / 8, FRAME_BUFFER_WIDTH * 13 / 20, FRAME_BUFFER_HEIGHT * 9 / 10
	sRect = { FRAME_BUFFER_WIDTH / 20 + 10.0f, 
		FRAME_BUFFER_HEIGHT / 8 + (float)num * FRAME_BUFFER_HEIGHT / 7 + 20.0f,
		FRAME_BUFFER_WIDTH * 13 / 20 - 10.0f, 
		FRAME_BUFFER_HEIGHT / 8 + (float)(num+1)*FRAME_BUFFER_HEIGHT / 7 + 10.0f };
	imageRect = { 0, 0, 1000, 563 };
	Button_Component room_btn = Button_Component(SelectRoomBtn, L"image/monster_hunter_login.png", DEFAULT_FONT, to_wstring(num) + L"번 방", m_d2dDeviceContext, m_d2dFactory, m_bitmap,
		sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect, num);
	if (disable) {
		room_btn.Disable();
	}	
	Rooms.push_back(room_btn);
}

void Scene_Sysytem::DeleteRoom(int room_num)
{
	Rooms.erase(remove_if(Rooms.begin(), Rooms.end(), [room_num](const Button_Component& b) {
		return b.m_room_num == room_num;
		}), Rooms.end());

	if (m_room_num == room_num) {
		m_room_num = -1;
	}
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
	m_room_num = -1;
	m_join_room = -1;
}

void Scene_Sysytem::AddInRoomPlayers(RoomPlayer_Info info)
{
	InRoomPlayers.push_back(info);
}

void Scene_Sysytem::RemoveInRoomPlayers(int id)
{
	InRoomPlayers.erase(remove_if(InRoomPlayers.begin(), InRoomPlayers.end(), [id](const RoomPlayer_Info& p) {
		return p.id == id;
		}), InRoomPlayers.end());
}

void Scene_Sysytem::InitInRoomPlayers() 
{
	InRoomPlayers.clear();
	m_ready = false;
}

void Scene_Sysytem::ReadyCheck(int id, bool ready)
{
	for (int i = 0; i < InRoomPlayers.size(); i++) {
		if (InRoomPlayers[i].id == id) {
			InRoomPlayers[i].ready = !InRoomPlayers[i].ready;
			break;
		}
	}
}
