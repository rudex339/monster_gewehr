#include "stdafx.h"
#include "Render_Sysytem.h"
#include "Object_Entity.h"
#include "ObjectManager.h"
#include "Player_Entity.h"
#include "Sever_Sysyem.h"
#include "Scene_Sysytem.h"

struct LIGHTS
{
	LIGHT								m_pLights[MAX_LIGHTS];
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights;
};

bool is_camera_behind(const DirectX::XMVECTOR& camera_pos, const DirectX::XMVECTOR& camera_look, const DirectX::XMVECTOR& object_pos) {
	DirectX::XMVECTOR camera_to_object = DirectX::XMVectorSubtract(object_pos, camera_pos);

	float dot_product;
	DirectX::XMStoreFloat(&dot_product, DirectX::XMVector3Dot(camera_look, camera_to_object));

	if (dot_product < -100) {
		return true;
	}

	return false;
}

bool is_camera_far(const DirectX::XMVECTOR& camera_pos, const DirectX::XMVECTOR& object_pos) {
	float distance;
	DirectX::XMStoreFloat(&distance, DirectX::XMVector3Length(DirectX::XMVectorSubtract(object_pos, camera_pos)));

	if (distance > 700.0f) {
		return true;
	}

	return false;
}

bool should_render(const DirectX::XMVECTOR& camera_pos, const DirectX::XMVECTOR& camera_look, const DirectX::XMVECTOR& object_pos) {
	return is_camera_behind(camera_pos, camera_look, object_pos) || is_camera_far(camera_pos, object_pos);
}

HRESULT LoadBitmapFromFile(const wchar_t* imagePath, ID2D1DeviceContext2* d2dDeviceContext, ID2D1Factory3* d2dFactory, ID2D1Bitmap** ppBitmap)
{
	IWICImagingFactory* pWICFactory = NULL;
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pFrame = NULL;
	IWICFormatConverter* pConverter = NULL;

	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICFactory));
	if (FAILED(hr))
		return hr;

	hr = pWICFactory->CreateDecoderFromFilename(imagePath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);
	if (FAILED(hr))
	{
		pWICFactory->Release();
		return hr;
	}

	hr = pDecoder->GetFrame(0, &pFrame);
	if (FAILED(hr))
	{
		pDecoder->Release();
		pWICFactory->Release();
		return hr;
	}

	hr = pWICFactory->CreateFormatConverter(&pConverter);
	if (FAILED(hr))
	{
		pFrame->Release();
		pDecoder->Release();
		pWICFactory->Release();
		return hr;
	}

	hr = pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeMedianCut);
	if (FAILED(hr))
	{
		pConverter->Release();
		pFrame->Release();
		pDecoder->Release();
		pWICFactory->Release();
		return hr;
	}

	hr = d2dDeviceContext->CreateBitmapFromWicBitmap(pConverter, NULL, ppBitmap);
	if (FAILED(hr))
	{
		pConverter->Release();
		pFrame->Release();
		pDecoder->Release();
		pWICFactory->Release();
		return hr;
	}

	pConverter->Release();
	pFrame->Release();
	pDecoder->Release();
	pWICFactory->Release();

	return S_OK;
}

Render_System::Render_System(ObjectManager* manager, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID2D1DeviceContext2* d2dDeviceContext, ID2D1Factory3* d2dFactory, IDWriteFactory5* dwriteFactory, Scene_Sysytem* scene)
{
	SetRootSignANDDescriptorANDCammandlist(manager, pd3dCommandList);

	m_d2dDeviceContext = d2dDeviceContext;
	m_dwriteFactory = dwriteFactory;
	m_d2dFactory = d2dFactory;
	m_scene = scene;

	m_xmf4GlobalAmbient = XMFLOAT4(0.50f, 0.50f, 0.50f, 1.0f);

	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);

	//boundingbox
	m_pBox = NULL;
	////////////////////바운딩 박스를 그리지 않으려면 이부분만 주석처리하면 됨
	if (manager->m_pBox != NULL) {
		m_pBox = manager->m_pBox;
	}
	////////////////////

	m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Wheat), &m_textBrush);
	m_dwriteFactory->CreateTextFormat(
		L"Verdana",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		50,
		L"en-us",
		&m_textFormat
	);

	m_dwriteFactory->CreateTextFormat(
		L"Verdana",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		18,
		L"en-us",
		&m_smalltextFormat
	);

	m_dwriteFactory->CreateTextFormat(
		L"Garamond",               
		NULL,                   
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		26,                      
		L"en-us",
		&pTextFormat
	);

	// custom font
	IDWriteFactory3* pDWriteFactory;
	HRESULT hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory5),
		reinterpret_cast<IUnknown**>(&pDWriteFactory)
	);
	IDWriteFontSetBuilder* pFontSetBuilder = nullptr;
	if (SUCCEEDED(hr))
	{
		hr = pDWriteFactory->CreateFontSetBuilder(&pFontSetBuilder);
	}
	IDWriteFontFile* pFontFile = nullptr;
	if (SUCCEEDED(hr))
	{
		hr = pDWriteFactory->CreateFontFileReference(L"font/Needleteeth Regular.ttf", nullptr, &pFontFile);
	}
	BOOL isSupported;
	DWRITE_FONT_FILE_TYPE fileType;
	UINT32 numberOfFonts;
	hr = pFontFile->Analyze(&isSupported, &fileType, /* face type */ nullptr, &numberOfFonts);

	for (uint32_t fontIndex = 0; fontIndex < numberOfFonts; fontIndex++)
	{
		IDWriteFontFaceReference* pFontFaceReference;
		hr = pDWriteFactory->CreateFontFaceReference(pFontFile, fontIndex, DWRITE_FONT_SIMULATIONS_NONE, &pFontFaceReference);

		if (SUCCEEDED(hr))
		{
			hr = pFontSetBuilder->AddFontFaceReference(pFontFaceReference);
		}
	}
	IDWriteFontSet* pFontSet;
	hr = pFontSetBuilder->CreateFontSet(&pFontSet);

	pDWriteFactory->CreateFontCollectionFromFontSet(pFontSet, &fontCollection);

	m_dwriteFactory->CreateTextFormat(
		L"Needleteeth Regular",
		fontCollection,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		48,
		L"en-us",
		&Needleteeth[0]
	);

	m_dwriteFactory->CreateTextFormat(
		L"Needleteeth Regular",
		fontCollection,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		54,
		L"en-us",
		&Needleteeth[1]
	);


	LoadBitmapFromFile(L"image/soldierFace.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[0]);

	float dashes[] = { 1.0f, 2.0f, 2.0f, 3.0f, 2.0f, 2.0f };
	//m_d2dFactory->CreateStrokeStyle(
	//	D2D1::StrokeStyleProperties(
	//		D2D1_CAP_STYLE_FLAT,
	//		D2D1_CAP_STYLE_FLAT,
	//		D2D1_CAP_STYLE_ROUND,
	//		D2D1_LINE_JOIN_MITER,
	//		10.0f,
	//		D2D1_DASH_STYLE_CUSTOM,
	//		0.0f),
	//	dashes,
	//	ARRAYSIZE(dashes),
	//	&m_strokeBrush
	//);

	m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	
}

void Render_System::configure(World* world)
{
	world->subscribe<SetCamera_Event>(this);
	world->subscribe<DrawUI_Event>(this);
	world->subscribe<KeyDown_Event>(this);
	world->subscribe<Tab_Event>(this);
	world->subscribe<Mouse_Event>(this);
	world->subscribe<InputId_Event>(this);
}

void Render_System::unconfigure(World* world)
{
	world->unsubscribeAll(this);
}

void Render_System::tick(World* world, float deltaTime)
{
	if (m_pCamera) {
		if (m_pd3dGraphicsRootSignature) m_pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
		if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

		m_pCamera->SetViewportsAndScissorRects(m_pd3dCommandList);
		m_pCamera->UpdateShaderVariables(m_pd3dCommandList);

		//UpdateShaderVariables(pd3dCommandList);
		int cur_point = 0;
		world->each<Light_Component>([&](
		Entity* ent,
			ComponentHandle<Light_Component> light
			) -> void {
				if (cur_point < MAX_LIGHTS) {
					::memcpy(&m_pcbMappedLights->m_pLights[cur_point], light->m_pLight, sizeof(LIGHT));
					cur_point++;
				}
			});

		::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
		::memcpy(&m_pcbMappedLights->m_nLights, &cur_point, sizeof(int));

		D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
		m_pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights


		world->each<SkyBox_Component>([&](
			Entity* ent,
			ComponentHandle<SkyBox_Component> SkyBox
			) -> void {
				SkyBox->m_SkyBox->Render(m_pd3dCommandList, m_pCamera);
			});

		/*world->each<Terrain_Component>([&](
			Entity* ent,
			ComponentHandle<Terrain_Component> Terrain
			) -> void {
				Terrain->m_pTerrain->Render(m_pd3dCommandList, m_pCamera);
			});*/
		world->each< Model_Component, Position_Component>([&](
			Entity* ent,
			ComponentHandle<Model_Component> Model,
			ComponentHandle<Position_Component> pos
			) -> void {
				if (ent->has<Terrain_Component>()) {
					Model->m_MeshModel->m_pModelRootObject->UpdateTransform(&pos->m_xmf4x4World);
					Model->m_MeshModel->m_pModelRootObject->Render(	m_pd3dCommandList, m_pCamera);
				}
				else if (ent->has<AnimationController_Component>() &&
					ent->has<Rotation_Component>() &&
					ent->has<Scale_Component>()) {
					if (ent->has<player_Component>()) {
						ComponentHandle<player_Component> data = ent->get<player_Component>();
						if (data->id == -1)
							return;
					}
					ComponentHandle<AnimationController_Component> AnimationController = ent->get<AnimationController_Component>();
					ComponentHandle<Rotation_Component> rotation = ent->get<Rotation_Component>();
					ComponentHandle<Scale_Component> Scale = ent->get<Scale_Component>();

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

					//test
					if (m_pBox) {
						if (ent->has<BoundingBox_Component>()) {
							ComponentHandle<BoundingBox_Component> box = ent->get<BoundingBox_Component>();

							box->m_bounding_box.Center = pos->Position;
							box->m_bounding_box.Center.y += box->m_bounding_box.Extents.y / 2 + 5.f;

							//m_pBox->UpdateTransform(&Matrix4x4::Identity());
							if (box->m_pMesh) {
								m_pBox->m_pMesh = box->m_pMesh;
								m_pBox->SetPosition(box->m_bounding_box.Center);
								//cout << box->m_bounding_box.Center.x << " " << box->m_bounding_box.Center.z << endl;
								//cout << pos->Position.x << " " << pos->Position.y << " " << pos->Position.z << endl;
								m_pBox->Render(m_pd3dCommandList, &box->m_bounding_box);
								m_pBox->SetPosition(0.f, 0.f, 0.f);
							}
						}
					}


					Model->m_MeshModel->m_pModelRootObject->UpdateTransform(&xmf4x4World);

					AnimationController->m_AnimationController->UpdateShaderVariables();
					if(Model->draw)
						Model->m_MeshModel->m_pModelRootObject->Render(	m_pd3dCommandList, m_pCamera);

					for (auto child : Model->m_pchildObjects) {
						if (child->socket) {
							child->m_MeshModel->m_pModelRootObject->UpdateTransform(&child->socket->m_xmf4x4World);
						}
						else {
							child->m_MeshModel->m_pModelRootObject->UpdateTransform(&xmf4x4World);
						}
						if (child->draw)
							child->m_MeshModel->m_pModelRootObject->Render(m_pd3dCommandList, m_pCamera);
					}


				}
				else{
					Model->m_MeshModel->m_pModelRootObject->UpdateTransform(&pos->m_xmf4x4World);
					if (!should_render(XMLoadFloat3(&m_pCamera->GetPosition()), XMLoadFloat3(&m_pCamera->GetLookVector()), XMLoadFloat3(&pos->Position))) {						
						if (Model->draw){
							Model->m_MeshModel->m_pModelRootObject->Render(m_pd3dCommandList, m_pCamera);
						}
						//test
						if (m_pBox) {
							if (ent->has<BoundingBox_Component>()) {
								ComponentHandle<BoundingBox_Component> box = ent->get<BoundingBox_Component>();
								//box->m_bounding_box.Center = pos->Position;
								//box->m_bounding_box.Center.y += box->m_bounding_box.Extents.y / 2;
								
								m_pBox->m_pMesh = box->m_pMesh;
								m_pBox->Render(m_pd3dCommandList, &box->m_bounding_box);
							}
						}
					}
				}
			});
	}
}

void Render_System::receive(World* world, const SetCamera_Event& event)
{
	m_pCamera = event.pCamera;
}

void Render_System::receive(World* world, const DrawUI_Event& event)
{
	
	world->each<ImageUI_Component>([&](
		Entity* ent,
		ComponentHandle<ImageUI_Component> imageUI
		) -> void {
			m_d2dDeviceContext->DrawBitmap(
				imageUI->m_bitmap,
				imageUI->m_Rect,
				imageUI->m_opacity,
				imageUI->m_mode,
				imageUI->m_imageRect
			);
		}
	);

	m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::White));
	world->each<TextUI_Component>([&](
		Entity* ent,
		ComponentHandle<TextUI_Component> textUI
		) -> void {
			ComPtr<IDWriteTextFormat> textformat;
			switch (textUI->m_fontType)
			{
			case GARMULI_FONT:
				textformat = m_smalltextFormat;
				break;
			case NEEDLE_FONT:
				textformat = Needleteeth[0];
				break;
			case SMALL_FONT:
				textformat = m_smalltextFormat;
				break;
			case DEFAULT_FONT:
			default:
				textformat = m_textFormat;
				break;
			}
			
			textformat.Get()->SetParagraphAlignment(textUI->m_paragraph_alignment);
			textformat.Get()->SetTextAlignment(textUI->m_text_alignment);

			m_textBrush->SetOpacity(1.0f);
			m_d2dDeviceContext->DrawTextW(
				textUI->m_text.data(),
				textUI->m_text.size(),
				textformat.Get(),
				&textUI->m_Rect,
				m_textBrush.Get()
			);
		}
	);

	world->each<TextBoxUI_Component>([&](
		Entity* ent,
		ComponentHandle<TextBoxUI_Component> editBox
		)-> void {

			// 텍스트 입력 박스
			m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::White));
			m_d2dDeviceContext->FillRectangle(
				{ editBox->x, editBox->y + 3.0f, editBox->x + 400.f, editBox->y + 35.0f },
				m_textBrush.Get()
			);
			// 텍스트 레이아웃
			m_dwriteFactory->CreateTextLayout(
				text[editBox->index].c_str(),
				static_cast<UINT32>(text[editBox->index].length()),
				pTextFormat.Get(),
				500,
				200,
				&pTextLayout[editBox->index]
			);

			

			// 텍스트
			m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Black));

			m_d2dDeviceContext->DrawTextLayout(
				D2D1::Point2F(editBox->x, editBox->y),
				pTextLayout[editBox->index],
				m_textBrush.Get(),
				D2D1_DRAW_TEXT_OPTIONS_NONE
			);


			// 커서 
			if (cursorPosition[textIndex] <= text[textIndex].length() && textIndex == editBox->index)
			{
				DWRITE_TEXT_METRICS textMetrics;
				pTextLayout[textIndex]->GetMetrics(&textMetrics);
				float cursorX = textMetrics.widthIncludingTrailingWhitespace + editBox->x + 1.0f;

				m_d2dDeviceContext->DrawLine(
					D2D1::Point2F(cursorX, editBox->y + 4.0f),
					D2D1::Point2F(cursorX, editBox->y + textMetrics.height - 1.0f),
					m_textBrush.Get(),
					1.0f
				);
			}
		}
	);

	world->each<Button_Component>([&](
		Entity* ent,
		ComponentHandle<Button_Component> button
		)-> void {
			{
				button->m_textBrush = m_textBrush;
				ComPtr<IDWriteTextFormat> textformat[2];
				switch (button->m_fontType)
				{
				case GARMULI_FONT:
					textformat[0] = m_smalltextFormat;
					textformat[1] = m_textFormat;
					break;
				case NEEDLE_FONT:
					textformat[0] = Needleteeth[0];
					textformat[1] = Needleteeth[1];
					break;
				case DEFAULT_FONT:
				default:
					textformat[0] = m_smalltextFormat;
					textformat[1] = m_textFormat;
					break;
				}
				button->m_textFormat = textformat[0];

				button->m_textFormats[0] = textformat[0];
				button->m_textFormats[1] = textformat[1];
			}

			{
				button->CursorOn(m_cursorPos, button->m_textFormats[1], button->m_textFormats[0]);
				m_d2dDeviceContext->DrawBitmap(
					button->m_bitmap,
					button->m_Rect,
					button->m_opacity, 
					button->m_mode,
					button->m_imageRect
				);
			}

			{
				button->m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::WhiteSmoke));
				button->m_textBrush.Get()->SetOpacity(button->m_opacity);

				button->m_textFormat.Get()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);	// 텍스트를 상하의 가운데에 위치
				button->m_textFormat.Get()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);			// 텍스트를 좌우의 가운데에 위치

				m_d2dDeviceContext->DrawTextW(
					button->m_text.data(),
					button->m_text.size(),
					button->m_textFormat.Get(),
					&button->m_Rect,
					button->m_textBrush.Get()
				);
			}


			if (button->cursor_on && clicked && button->activate) {
				switch (button->button_id)
				{
				case ExitBtn:
					exit(0);
					break;
				case GameStartBtn:
					cout << "0번 눌림 게임시작" << endl;
					world->emit< ChangeScene_Event>({ ROOMS });
					break;
				case ShopBtn:
					cout << "상점 눌림" << endl;
					world->emit< ChangeScene_Event>({ SHOP });
					break;
				case EquipBtn:
					cout << "장비창 눌림" << endl;
					world->emit< ChangeScene_Event>({ EQUIPMENT });
					break;
				case MakeRoomBtn:
					cout << "방 생성" << endl;
					//m_scene->AddRoom();
					//world->emit< ChangeScene_Event>({ ROOMS });
					world->emit<Create_Room>({});
					break;
				case RoomBtn:
					world->emit<ChoiceRoom_Event>({ button->m_room_num });
					select_room_num = button->m_room_num;
					m_scene->InitRoomPlayers();
					world->emit<Select_Room>({ (SHORT)select_room_num });					
					break;
				case JoinRoomBtn:
					world->emit<EnterRoom_Event>({ INROOM, select_room_num });
					world->emit<Join_Room>({ (SHORT)select_room_num });
					break;
				default:
					cout << "디폴트" << endl;
					break;
				}
				Clicked(); // 버튼이 눌리고 나면 clicked를 false로 변경(원래 마우스를 움직여야만 false로 바뀌기 때문에 생기는 버그 해결용)
			}
		}
	);

	D2D1_RECT_F textRect = D2D1::RectF(FRAME_BUFFER_WIDTH-300, FRAME_BUFFER_HEIGHT - 100, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	D2D1_RECT_F imageRect = D2D1::RectF(10, 5, 90, 90);
	D2D1_ELLIPSE ellipse = D2D1::Ellipse({ FRAME_BUFFER_WIDTH/2, FRAME_BUFFER_HEIGHT/2 }, 4.0f, 4.0f);
	world->each< player_Component, Camera_Component>([&](
		Entity* ent,
		ComponentHandle<player_Component> player,
		ComponentHandle<Camera_Component> camera
		) -> void {
			{
				textRect = D2D1::RectF(0, 0, 390, 100);
				m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::White));
				m_d2dDeviceContext->DrawRectangle(&textRect, m_textBrush.Get());
			}

			{
				m_d2dDeviceContext->DrawBitmap(m_bitmaps[0], imageRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, {0,0,100,100});
				//m_d2dDeviceContext->DrawBitmap(m_bitmaps[1], {400, 400, 500, 600}, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, {0,0,100,100});
			}

			{
				wstring text = std::to_wstring((int)player->ammo);
				text += std::wstring(" / 30", " / 30" + strlen(" / 30"));

				D2D1_RECT_F textRect = D2D1::RectF(FRAME_BUFFER_WIDTH - 300, FRAME_BUFFER_HEIGHT - 100, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
				m_d2dDeviceContext->DrawTextW(
					text.data(),
					text.size(),
					m_textFormat.Get(),
					&textRect,
					m_textBrush.Get()
				);
			}
			{
				wstring text = std::wstring("HP ", "HP " + strlen("HP "));
				text += std::to_wstring((int)player->hp);
				textRect = D2D1::RectF(100, 10, 170, 50);
				m_d2dDeviceContext->DrawTextW(
					text.data(),
					text.size(),
					m_smalltextFormat.Get(),
					&textRect,
					m_textBrush.Get()
				);

				textRect = D2D1::RectF(170, 15, 370, 30);
				m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
				m_d2dDeviceContext->FillRectangle(&textRect, m_textBrush.Get());

				textRect = D2D1::RectF(170, 15, (int)player->hp * 2 + 170, 30);
				m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
				m_d2dDeviceContext->FillRectangle(&textRect, m_textBrush.Get());
			}

			{
				wstring text = std::wstring("SP 100", "SP 100" + strlen("SP 100"));

				m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::White));
				textRect = D2D1::RectF(100, 40, 170, 80);
				m_d2dDeviceContext->DrawTextW(
					text.data(),
					text.size(),
					m_smalltextFormat.Get(),
					&textRect,
					m_textBrush.Get()
				);
				textRect = D2D1::RectF(170, 45, 370, 60);
				m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
				m_d2dDeviceContext->FillRectangle(&textRect, m_textBrush.Get());

				textRect = D2D1::RectF(170, 45, 370, 60);
				m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::YellowGreen));
				m_d2dDeviceContext->FillRectangle(&textRect, m_textBrush.Get());


			}

			{
				if (player->aim_mode) {
					m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Blue));
					m_d2dDeviceContext->FillEllipse(&ellipse, m_textBrush.Get());

				}
			}
		}
	);
}

void Render_System::SetRootSignANDDescriptorANDCammandlist(ObjectManager* manager, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dCommandList = pd3dCommandList;
	m_pd3dGraphicsRootSignature = manager->GetGraphicsRootSignature();
	m_pd3dCbvSrvDescriptorHeap = manager->GetCbvSrvDescriptorHeap();
}

void Render_System::receive(World* world, const KeyDown_Event& event)
{
	bool isShiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	bool isCapsLockOn = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
	wchar_t key = event.key;

	if ((isShiftPressed && !isCapsLockOn) || (!isShiftPressed && isCapsLockOn))
	{
		key = toupper(key);
	}
	else
	{
		key = tolower(key);
	}

	if (event.key == VK_BACK && cursorPosition[textIndex] > 0)
	{
		text[textIndex].erase(text[textIndex].begin() + cursorPosition[textIndex] - 1);
		cursorPosition[textIndex]--;
	}
	else if (event.key == VK_LEFT && cursorPosition[textIndex] > 0)
	{
		cursorPosition[textIndex]--;
	}
	else if (event.key == VK_RIGHT && cursorPosition[textIndex] < text[textIndex].length())
	{
		cursorPosition[textIndex]++;
	}

	else if(0x30 <= event.key && 0x39 >= event.key ||
			0x41 <= event.key && 0x5A >= event.key) {
		text[textIndex].insert(text[textIndex].begin() + cursorPosition[textIndex], static_cast<wchar_t>(key));
		cursorPosition[textIndex]++;
	}
}

void Render_System::receive(World* world, const Tab_Event& event)
{
	textIndex = (textIndex + 1) % 2;
}

void Render_System::receive(World* world, const Mouse_Event& event)
{
	m_cursorPos = event.cursorPos;
	clicked = event.click;
}

void Render_System::receive(World* world, const InputId_Event& event)
{
	string id;

	id.assign(text[0].begin(), text[0].end());
	cout << id.c_str() << endl;
	world->emit<Login_Event>({ id });
}
