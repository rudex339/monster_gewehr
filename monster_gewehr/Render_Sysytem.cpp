#include "stdafx.h"
#include "Render_Sysytem.h"
#include "Object_Entity.h"
#include "ObjectManager.h"
#include "Player_Entity.h"
#include "Sever_Sysyem.h"
#include "Scene_Sysytem.h"
#include <math.h>
#include "BlurFilter.h"

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

	if (distance > 1500.0f) {
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
	m_pd3dDevice = pd3dDevice;
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
		FRAME_BUFFER_HEIGHT * FRAME_BUFFER_WIDTH / 18432,
		L"en-us",
		&m_textFormat
	);

	m_dwriteFactory->CreateTextFormat(
		L"Verdana",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		FRAME_BUFFER_HEIGHT * FRAME_BUFFER_WIDTH / 51200,
		L"en-us",
		&m_smalltextFormat
	);

	m_dwriteFactory->CreateTextFormat(
		L"Verdana",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		FRAME_BUFFER_HEIGHT * FRAME_BUFFER_WIDTH / 102400,
		L"en-us",
		&m_verysmalltextFormat
	);

	m_dwriteFactory->CreateTextFormat(
		L"Verdana",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		FRAME_BUFFER_HEIGHT * FRAME_BUFFER_WIDTH / 76800,
		L"en-us",
		&m_ingametextFormat
	);

	m_dwriteFactory->CreateTextFormat(
		L"Garamond",
		NULL,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		FRAME_BUFFER_HEIGHT * FRAME_BUFFER_WIDTH / 35446,
		L"en-us",
		&pTextFormat
	);

	m_dwriteFactory->CreateTextFormat(
		L"Verdana",
		NULL,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		FRAME_BUFFER_HEIGHT * FRAME_BUFFER_WIDTH / 35446,
		L"en-us",
		&m_ingametextFormat2
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
		FRAME_BUFFER_HEIGHT * FRAME_BUFFER_WIDTH / 19200,
		L"en-us",
		&Needleteeth[0]
	);

	m_dwriteFactory->CreateTextFormat(
		L"Needleteeth Regular",
		fontCollection,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		FRAME_BUFFER_HEIGHT * FRAME_BUFFER_WIDTH / 17066,
		L"en-us",
		&Needleteeth[1]
	);


	LoadBitmapFromFile(L"image/soldierFace.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[0]);
	LoadBitmapFromFile(L"image/icons/m4.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[2]);
	LoadBitmapFromFile(L"image/icons/Benelli.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[3]);
	LoadBitmapFromFile(L"image/icons/sr.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[4]);
	LoadBitmapFromFile(L"image/icons/band.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[5]);
	LoadBitmapFromFile(L"image/icons/fak.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[6]);
	LoadBitmapFromFile(L"image/icons/injector.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[7]);
	LoadBitmapFromFile(L"image/icons/grenade.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[8]);
	LoadBitmapFromFile(L"image/icons/flashbang.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[9]);
	LoadBitmapFromFile(L"image/icons/key1.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[10]);
	LoadBitmapFromFile(L"image/icons/key2.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[11]);
	LoadBitmapFromFile(L"image/icons/key3.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[12]);





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

	m_pBlurFilter = new BlurFilter(pd3dDevice, pd3dCommandList);

}

void Render_System::configure(World* world)
{
	world->subscribe<SetCamera_Event>(this);
	world->subscribe<DrawUI_Event>(this);
	world->subscribe<KeyDown_Event>(this);
	world->subscribe<Tab_Event>(this);
	world->subscribe<Mouse_Event>(this);
	world->subscribe<InputId_Event>(this); 
	world->subscribe<DrawComputeShader_Event>(this);
	world->subscribe<SetBlur_Event>(this);
}

void Render_System::unconfigure(World* world)
{
	world->unsubscribeAll(this);
}

void Render_System::tick(World* world, float deltaTime)
{
	if (m_pBlurFilter->m_Strength > 0) {
		m_pBlurFilter->m_Strength -= deltaTime*5;
	}

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

		world->each<Terrain_Component>([&](
			Entity* ent,
			ComponentHandle<Terrain_Component> Terrain
			) -> void {
				Terrain->m_pTerrain->Render(m_pd3dCommandList, m_pCamera);
			});
		world->each< Model_Component, Position_Component>([&](
			Entity* ent,
			ComponentHandle<Model_Component> Model,
			ComponentHandle<Position_Component> pos
			) -> void {
				if (ent->has<Terrain_Component>()) {
					Model->m_MeshModel->UpdateTransform(&pos->m_xmf4x4World);
					Model->m_MeshModel->Render(	m_pd3dCommandList, m_pCamera);
				}
				else if (ent->has<Rotation_Component>() &&
					ent->has<Scale_Component>()) {
					if (ent->has<player_Component>()) {
						ComponentHandle<player_Component> data = ent->get<player_Component>();
						if (data->id == -1)
							return;
					}


					ComponentHandle<Rotation_Component> rotation = ent->get<Rotation_Component>();
					ComponentHandle<Scale_Component> Scale = ent->get<Scale_Component>();
					if (ent->has<AnimationController_Component>()) {
						// 애니메이션 실행 시키는 부분
						ComponentHandle<AnimationController_Component> AnimationController = ent->get<AnimationController_Component>();
						AnimationController->m_AnimationController->AdvanceTime(deltaTime, Model->m_MeshModel);
						Model->m_MeshModel->Animate(deltaTime);
					}

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

							float radian = XMConvertToRadians(rotation->mfYaw);
							XMFLOAT4 q{};
							XMStoreFloat4(&q, XMQuaternionRotationRollPitchYaw(0.f, radian, 0.f));

							box->m_bounding_box.Orientation = q;

							//box->m_bounding_box.Orientation.x = rotation->mfPitch;
							//box->m_bounding_box.Orientation.y = rotation->mfYaw;
							//box->m_bounding_box.Orientation.z = rotation->mfRoll;


							m_pBox->UpdateTransform(&Matrix4x4::Identity());
							if (box->m_pMesh) {
								m_pBox->m_pMesh = box->m_pMesh;
								m_pBox->SetPosition(box->m_bounding_box.Center);
								//m_pBox->SetRotation(rotation->mfPitch, rotation->mfYaw, rotation->mfRoll);
								//cout << box->m_bounding_box.Center.x << " " << box->m_bounding_box.Center.z << endl;
								//cout << pos->Position.x << " " << pos->Position.y << " " << pos->Position.z << endl;
								m_pBox->Render(m_pd3dCommandList, &box->m_bounding_box);
								m_pBox->SetPosition(0.f, 0.f, 0.f);
							}
						}
					}


					Model->m_MeshModel->UpdateTransform(&xmf4x4World);

					if (ent->has<AnimationController_Component>()) {
						ComponentHandle<AnimationController_Component> AnimationController = ent->get<AnimationController_Component>();
						AnimationController->m_AnimationController->UpdateShaderVariables();
					}
					if (ent->has<Emitter_Componet>()) {
						ComponentHandle<Emitter_Componet> emiiter = ent->get<Emitter_Componet>();
						((TextureRectMesh*)Model->m_MeshModel->m_pMesh)->changeRowCol(emiiter->m_nRow, emiiter->m_nCol, emiiter->m_nRows, emiiter->m_nCols);

						
					}
					
					if (Model->draw) {
						Model->m_MeshModel->Animate(deltaTime);
						Model->m_MeshModel->Render(m_pd3dCommandList, m_pCamera);
					}

					for (auto child : Model->m_pchildObjects) {
						if (child->socket) {
							child->m_MeshModel->UpdateTransform(&child->socket->m_xmf4x4World);
						}
						else {
							child->m_MeshModel->UpdateTransform(&xmf4x4World);
						}
						if (child->draw)
							child->m_MeshModel->Render(m_pd3dCommandList, m_pCamera);
					}


				}
				else{
					Model->m_MeshModel->UpdateTransform(&pos->m_xmf4x4World);
					if (!should_render(XMLoadFloat3(&m_pCamera->GetPosition()), XMLoadFloat3(&m_pCamera->GetLookVector()), XMLoadFloat3(&pos->Position))) {		
					//if (true) {
						if (Model->draw){
							Model->m_MeshModel->Render(m_pd3dCommandList, m_pCamera);
						}
						//test
						if (m_pBox) {
							if (ent->has<BoundingBox_Component>()) {

								ComponentHandle<BoundingBox_Component> box = ent->get<BoundingBox_Component>();
								//box->m_bounding_box.Center = pos->Position;
								//box->m_bounding_box.Center.y += box->m_bounding_box.Extents.y / 2;
								if (box->m_pMesh) {
									m_pBox->m_pMesh = box->m_pMesh;
									m_pBox->Render(m_pd3dCommandList, &box->m_bounding_box);
								}
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
			case MEDIUM_FONT:
				textformat = pTextFormat;
				break;
			case NEEDLE_FONT:
				textformat = Needleteeth[0];
				break;
			case SMALL_FONT:
				textformat = m_smalltextFormat;
				break;
			case NANO_FONT:
				textformat = m_verysmalltextFormat;
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

			if (editBox->CursorInBox(m_cursorPos) && clicked) {
				textIndex = editBox->index;
			}

			// 텍스트 입력 박스
			m_textBrush.Get()->SetOpacity(1.0f);
			m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::White));
			m_d2dDeviceContext->FillRectangle(
				{ editBox->x, editBox->y, editBox->x + editBox->m_width, editBox->y + editBox->m_height },
				m_textBrush.Get()
			);

			if (editBox->index == 1) {
				Invisile_password = text[editBox->index];
				for (int i = 0; i < Invisile_password.length(); ++i) {
					Invisile_password[i] = '*';
				}

				// 텍스트 레이아웃
				m_dwriteFactory->CreateTextLayout(
					Invisile_password.c_str(),
					static_cast<UINT32>(Invisile_password.length()),
					pTextFormat.Get(),
					editBox->m_width,
					editBox->m_height,
					&pTextLayout[editBox->index]
				);
			}
			else {
				// 텍스트 레이아웃
				m_dwriteFactory->CreateTextLayout(
					text[editBox->index].c_str(),
					static_cast<UINT32>(text[editBox->index].length()),
					pTextFormat.Get(),
					editBox->m_width,
					editBox->m_height,
					&pTextLayout[editBox->index]
				);
			}


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
				// 커서 위치를 기준으로 텍스트 폭 계산
				DWRITE_HIT_TEST_METRICS hitTestMetrics;
				float cursorX, cursorY;

				pTextLayout[textIndex]->HitTestTextPosition(
					static_cast<UINT32>(cursorPosition[textIndex]), // 커서 위치 인덱스
					FALSE, // 커서가 문자 앞에 있는지 여부
					&cursorX, // 커서의 X 좌표
					&cursorY, // 커서의 Y 좌표
					&hitTestMetrics // 커서 위치의 텍스트 메트릭스
				);

				cursorX += editBox->x; // editBox의 x 좌표를 더하여 커서의 절대 좌표 계산

				// 커서를 계산된 위치에 그리기
				m_d2dDeviceContext->DrawLine(
					D2D1::Point2F(cursorX, editBox->y + 4.0f),
					D2D1::Point2F(cursorX, editBox->y + hitTestMetrics.height - 1.0f),
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
				case MEDIUM_FONT:
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

				button->m_textFormat.Get()->SetParagraphAlignment(button->m_paragraph_alignment);	// 텍스트를 상하의 가운데에 위치
				button->m_textFormat.Get()->SetTextAlignment(button->m_text_alignment);			// 텍스트를 좌우의 가운데에 위치

				m_d2dDeviceContext->DrawTextW(
					button->m_text.data(),
					button->m_text.size(),
					button->m_textFormat.Get(),
					&button->m_Rect,
					button->m_textBrush.Get()
				);
			}


			if (button->cursor_on && clicked && button->activate) {
				Sound_Componet::GetInstance().PlaySound(Sound_Componet::Sound::B_Push);
				switch (button->button_id)
				{
				case ExitBtn:
					exit(0);
					break;

				case LoginBtn:
					// 여기서 아이디 비교하고, 성공하면 로비로 이동
					m_scene->SetName(text[0]);
					world->emit<InputId_Event>({});
					break;

				case RegisterBtn:
					// 여기서 아이디랑 비밀번호를 DB에 저장 및 로비로 이동
					break;

				case ChangeSceneBtn:
					if (button->Next_Scene == ROOMS && button->Curr_Scene == INROOM) {
						world->emit<Quit_Room>({});
					}
					else if (button->Next_Scene == LOBBY && button->Curr_Scene == EQUIPMENT) {
						world->emit<Set_Equipment>({ (char)m_scene->GetEquipments()[0], (char)m_scene->GetEquipments()[1], (char)m_scene->GetEquipments()[2] });
					}
					world->emit<ChangeScene_Event>({ button->Next_Scene });
					break;
				case MakeRoomBtn:
					world->emit<Create_Room>({});
					break;
				case SelectRoomBtn:
					m_scene->InitRoomPlayers();
					select_room_num = button->m_room_num;
					world->emit<Select_Room>({ (SHORT)select_room_num });
					world->emit<ChoiceRoom_Event>({ button->m_room_num });
					break;
				case JoinRoomBtn:
					//world->emit<EnterRoom_Event>({ INROOM, select_room_num, false });
					world->emit<Join_Room>({ (SHORT)select_room_num });
					break;
				case ItemBtn:
					world->emit<ChoiceItem_Event>({ button->item_num });
					break;
				case BuyBtn:
					cout << "구매/강화" << endl;
					world->emit<Purchase_Event>({});
					world->emit<Refresh_Scene>({ SHOP });
					Sound_Componet::GetInstance().PlaySound(Sound_Componet::Sound::Purchase);
					break;
				case GameReadyBtn:
					world->emit<Ready_Room>({});
					world->emit<Ready_Event>({});

					break;
				case GameStartBtn:
					//world->emit< ChangeScene_Event>({ GAME });
					world->emit<Game_Start>({});
					break;
				case EquipLeftBtn:
					world->emit<ChoiceEquip_Event>({ EquipLeftBtn, button->item_num });
					break;
				case EquipRightBtn:
					world->emit<ChoiceEquip_Event>({ EquipRightBtn, button->item_num });
					break;
				case EquipUpBtn:
					world->emit<ChoiceEquip_Event>({ EquipUpBtn, button->item_num });
					break;
				case EquipDownBtn:
					world->emit<ChoiceEquip_Event>({ EquipDownBtn, button->item_num });
					break;
				case EndGameBtn:
					world->emit<Clearlayer_Event>({});
					world->emit<ChangeScene_Event>({ LOBBY });
					Sound_Componet::GetInstance().PlayMusic(Sound_Componet::Music::Title);
					break;
				default:
					cout << "디폴트" << endl;
					break;
				}
				Clicked(); // 버튼이 눌리고 나면 clicked를 false로 변경(원래 마우스를 움직여야만 false로 바뀌기 때문에 생기는 버그 해결용)
			}
		}
	);

	world->each<player_Component, Position_Component>([&](
		Entity* ent,
		ComponentHandle<player_Component> player,
		ComponentHandle<Position_Component> position
		) -> void {
			POINT PosXZ = { position->Position.x, position->Position.z };
			SetUserInfo(player->id, PosXZ);
		}
	);

	D2D1_RECT_F textRect = D2D1::RectF(FRAME_BUFFER_WIDTH - 300, FRAME_BUFFER_HEIGHT - 100, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	D2D1_RECT_F imageRect = D2D1::RectF(10, 5, 90, 90);
	D2D1_ELLIPSE ellipse = D2D1::Ellipse({ FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT / 2 }, 4.0f, 4.0f);
	D2D1_ARC_SEGMENT arcSegment = D2D1::ArcSegment({ FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT / 2 }, {10.f, 10.f}, 10.f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL);
	world->each<player_Component, Camera_Component, ControllAngle_Component, Position_Component>([&](
		Entity* ent,
		ComponentHandle<player_Component> player,
		ComponentHandle<Camera_Component> camera,
		ComponentHandle<ControllAngle_Component> cc,
		ComponentHandle<Position_Component> position
		) -> void {
			{
				if (m_scene->GetState() == END) { return; }
				// 플레이어 정보 UI
				// 테두리
				textRect = D2D1::RectF(0, 0, FRAME_BUFFER_WIDTH / 3.2 + 20, FRAME_BUFFER_HEIGHT / 7 + 20);
				m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::White));
				m_d2dDeviceContext->DrawRectangle(&textRect, m_textBrush.Get());

				// 초상화
				D2D1_RECT_F sRect = { 10, 10, FRAME_BUFFER_WIDTH / 13, FRAME_BUFFER_HEIGHT / 7 };
				ImageUI_Component image = ImageUI_Component(L"image/soldierFace.png", m_d2dDeviceContext, m_d2dFactory, m_bitmaps[0], sRect, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);
				m_d2dDeviceContext->DrawBitmap(
					m_bitmaps[0],
					image.m_Rect,
					image.m_opacity,
					image.m_mode,
					image.m_imageRect
				);

				// HP 텍스트
				TextUI_Component hp = TextUI_Component(DEFAULT_FONT, L"HP " + to_wstring((int)player->hp),
					FRAME_BUFFER_HEIGHT / 35, FRAME_BUFFER_WIDTH / 13 + 5, FRAME_BUFFER_HEIGHT * 2 / 35 ,FRAME_BUFFER_WIDTH / 3.2);

				m_d2dDeviceContext->DrawTextW(
					hp.m_text.c_str(),
					hp.m_text.size(),
					m_ingametextFormat.Get(),
					hp.m_Rect,
					m_textBrush.Get()
				);

				// HP 바
				textRect = D2D1::RectF(FRAME_BUFFER_WIDTH * 5 / 36, FRAME_BUFFER_HEIGHT / 35, FRAME_BUFFER_WIDTH / 3.2, FRAME_BUFFER_HEIGHT * 2 / 35);
				m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
				m_d2dDeviceContext->FillRectangle(&textRect, m_textBrush.Get());

				float width = (FRAME_BUFFER_WIDTH / 3.2 - FRAME_BUFFER_WIDTH * 5 / 36) / 100;

				textRect = D2D1::RectF(FRAME_BUFFER_WIDTH * 5 / 36, FRAME_BUFFER_HEIGHT / 35, FRAME_BUFFER_WIDTH * 5 / 36 + player->hp * width, FRAME_BUFFER_HEIGHT * 2 / 35);
				m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
				m_d2dDeviceContext->FillRectangle(&textRect, m_textBrush.Get());

				// SP 텍스트
				m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::White));
				TextUI_Component sp = TextUI_Component(DEFAULT_FONT, L"SP " + to_wstring((int)player->stamina),
					FRAME_BUFFER_HEIGHT * 3 / 35, FRAME_BUFFER_WIDTH / 13 + 5, FRAME_BUFFER_HEIGHT * 4 / 35, FRAME_BUFFER_WIDTH / 3.2);

				m_d2dDeviceContext->DrawTextW(
					sp.m_text.c_str(),
					sp.m_text.size(),
					m_ingametextFormat.Get(),
					sp.m_Rect,
					m_textBrush.Get()
				);

				// SP 바
				textRect = D2D1::RectF(FRAME_BUFFER_WIDTH * 5 / 36, FRAME_BUFFER_HEIGHT * 3 / 35, FRAME_BUFFER_WIDTH / 3.2, FRAME_BUFFER_HEIGHT * 4 / 35);
				m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
				m_d2dDeviceContext->FillRectangle(&textRect, m_textBrush.Get());

				textRect = D2D1::RectF(FRAME_BUFFER_WIDTH * 5 / 36, FRAME_BUFFER_HEIGHT * 3 / 35, FRAME_BUFFER_WIDTH * 5 / 36 + player->stamina * width, FRAME_BUFFER_HEIGHT * 4 / 35);
				m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::YellowGreen));
				m_d2dDeviceContext->FillRectangle(&textRect, m_textBrush.Get());

			}

			// 무기 종류와 탄창 표시
			{
				// 무기 아이콘
				D2D1_RECT_F sRect = {  FRAME_BUFFER_WIDTH * 25 / 30, FRAME_BUFFER_HEIGHT * 19 / 24, FRAME_BUFFER_WIDTH * 29 / 30, FRAME_BUFFER_HEIGHT * 21 / 24 };
				imageRect = {0,0,450,150};
				m_d2dDeviceContext->DrawBitmap(
					m_bitmaps[m_scene->GetEquipments()[0]+2],
					sRect,
					1.0f,
					D2D1_INTERPOLATION_MODE_LINEAR,
					imageRect
				);

				// 탄창
				TextUI_Component ammo = TextUI_Component(DEFAULT_FONT, to_wstring((int)player->ammo) + L"/" + to_wstring((int)player->mag),
					FRAME_BUFFER_HEIGHT * 21 / 24, FRAME_BUFFER_WIDTH * 13 / 15, FRAME_BUFFER_HEIGHT, FRAME_BUFFER_WIDTH);
				m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::White));

				m_d2dDeviceContext->DrawTextW(
					ammo.m_text.c_str(),
					ammo.m_text.size(),
					m_ingametextFormat2.Get(),
					&ammo.m_Rect,
					m_textBrush.Get()
				);
			}

			// 힐 아이템 아이콘 및 보유량
			{
				D2D1_RECT_F sRect = { FRAME_BUFFER_WIDTH * 25 / 30, FRAME_BUFFER_HEIGHT * 19 / 24, FRAME_BUFFER_WIDTH * 29 / 30, FRAME_BUFFER_HEIGHT * 21 / 24 };
				imageRect = { 0,0,200,200 };

				for (int i = 0; i < 4; ++i) {
					if (i < 3) {
						float opacity = m_scene->GetHealItems()[i] > 0 ? 1.0f : 0.4f;
						// 아이템 사용 키 아이콘 출력
						sRect = { FRAME_BUFFER_WIDTH * 26.7 / 30, FRAME_BUFFER_HEIGHT * (16.5f - i * 1.5f) / 24, FRAME_BUFFER_WIDTH * 27.7 / 30, FRAME_BUFFER_HEIGHT * (18 - i * 1.5f) / 24 };
						m_d2dDeviceContext->DrawBitmap(
							m_bitmaps[10+i],
							sRect,
							opacity,
							D2D1_INTERPOLATION_MODE_LINEAR,
							imageRect
						);

						// 아이템 아이콘 출력
						sRect = { FRAME_BUFFER_WIDTH * 28 / 30, FRAME_BUFFER_HEIGHT * (16.5f - i * 1.5f) / 24, FRAME_BUFFER_WIDTH * 29 / 30, FRAME_BUFFER_HEIGHT * (18 - i * 1.5f) / 24 };
						m_d2dDeviceContext->DrawBitmap(
							m_bitmaps[i + 5],
							sRect,
							opacity,
							D2D1_INTERPOLATION_MODE_LINEAR,
							imageRect
						);

						// 아이템 보유 개수 출력
						TextUI_Component heal_text = TextUI_Component(DEFAULT_FONT, to_wstring(m_scene->GetHealItems()[i]),
							FRAME_BUFFER_HEIGHT * (16.5f - i * 1.5f) / 24, FRAME_BUFFER_WIDTH * 29.2 / 30, FRAME_BUFFER_HEIGHT * (18 - i * 1.5f) / 24, FRAME_BUFFER_WIDTH * 30 / 30);
						m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::White));

						m_verysmalltextFormat.Get()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
						m_d2dDeviceContext->DrawTextW(
							heal_text.m_text.c_str(),
							heal_text.m_text.size(),
							m_verysmalltextFormat.Get(),
							&heal_text.m_Rect,
							m_textBrush.Get()
						);
					}

					else {
						// if() 수류탄 보유 중일때만 출력하도록
						// 투척무기 아이콘 출력
						sRect = { FRAME_BUFFER_WIDTH * 28 / 30, FRAME_BUFFER_HEIGHT * (16.5f - i * 1.5f) / 24, FRAME_BUFFER_WIDTH * 29 / 30, FRAME_BUFFER_HEIGHT * (18 - i * 1.5f) / 24 };
						m_d2dDeviceContext->DrawBitmap(
							m_bitmaps[m_scene->GetEquipments()[2] + 3],
							sRect,
							1.0f,
							D2D1_INTERPOLATION_MODE_LINEAR,
							imageRect
						);
					}
				}
			}

			// 회복 표시
			{
				if (player->heal_timer > 0) {
					TextUI_Component heal_text = TextUI_Component(DEFAULT_FONT, L"회복중",
						FRAME_BUFFER_HEIGHT * 20 / 24, FRAME_BUFFER_WIDTH * 47 / 100, FRAME_BUFFER_HEIGHT * 21 / 24, FRAME_BUFFER_WIDTH * 58 / 100);
					m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::White));

					m_d2dDeviceContext->DrawTextW(
						heal_text.m_text.c_str(),
						heal_text.m_text.size(),
						m_ingametextFormat2.Get(),
						&heal_text.m_Rect,
						m_textBrush.Get()
					);

					float width = (FRAME_BUFFER_WIDTH * 70 / 100 - FRAME_BUFFER_WIDTH * 30 / 100) / 100 ;

					// 회복 진행 바
					textRect = D2D1::RectF(FRAME_BUFFER_WIDTH * 30 / 100, FRAME_BUFFER_HEIGHT * 43 / 48, FRAME_BUFFER_WIDTH * 70 / 100, FRAME_BUFFER_HEIGHT * 45 / 48);
					m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Gray));
					m_d2dDeviceContext->FillRectangle(&textRect, m_textBrush.Get());

					float time_per = 100.f - player->heal_timer * 100.f / player->heal_all_time;

					textRect = D2D1::RectF(FRAME_BUFFER_WIDTH * 30 / 100, FRAME_BUFFER_HEIGHT * 43 / 48, FRAME_BUFFER_WIDTH * 30 / 100 + time_per * width, FRAME_BUFFER_HEIGHT * 45 / 48);
					m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Green));
					m_d2dDeviceContext->FillRectangle(&textRect, m_textBrush.Get());

				}
			}


			// 보급상자 근처에 가면 보급 가능 여부 및 남은 시간 출력
			player->near_supply = false;
			{
				for (int i = 0; i < 5; ++i) {
					if (Distance(position->Position, supplys[i]) <= 30.0f) {
						player->near_supply = true;
						
						
						// 보급 중일때
						if (player->is_suppling) {
							TextUI_Component heal_text = TextUI_Component(DEFAULT_FONT, L"보급중",
								FRAME_BUFFER_HEIGHT * 20 / 24, FRAME_BUFFER_WIDTH * 47 / 100, FRAME_BUFFER_HEIGHT * 21 / 24, FRAME_BUFFER_WIDTH * 58 / 100);
							m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::White));

							m_d2dDeviceContext->DrawTextW(
								heal_text.m_text.c_str(),
								heal_text.m_text.size(),
								m_ingametextFormat2.Get(),
								&heal_text.m_Rect,
								m_textBrush.Get()
							);

							float width = (FRAME_BUFFER_WIDTH * 70 / 100 - FRAME_BUFFER_WIDTH * 30 / 100) / 100;

							// 보급 진행 바
							textRect = D2D1::RectF(FRAME_BUFFER_WIDTH * 30 / 100, FRAME_BUFFER_HEIGHT * 43 / 48, FRAME_BUFFER_WIDTH * 70 / 100, FRAME_BUFFER_HEIGHT * 45 / 48);
							m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Gray));
							m_d2dDeviceContext->FillRectangle(&textRect, m_textBrush.Get());

							float time_per = 100.f - player->supply_timer * 100.f / player->supply_time;

							textRect = D2D1::RectF(FRAME_BUFFER_WIDTH * 30 / 100, FRAME_BUFFER_HEIGHT * 43 / 48, FRAME_BUFFER_WIDTH * 30 / 100 + time_per * width, FRAME_BUFFER_HEIGHT * 45 / 48);
							m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Green));
							m_d2dDeviceContext->FillRectangle(&textRect, m_textBrush.Get());
						}

						// 보급 중이 아닐때 보급하라는 표시 
						else {
							// 보급 가능 여부 체크
							if (player->can_supply < 0.f) {
								TextUI_Component heal_text = TextUI_Component(DEFAULT_FONT, L"F키를 눌러 보급",
									FRAME_BUFFER_HEIGHT * 15 / 24, FRAME_BUFFER_WIDTH * 47 / 100, FRAME_BUFFER_HEIGHT * 16 / 24, FRAME_BUFFER_WIDTH * 58 / 100);
								m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::White));

								m_d2dDeviceContext->DrawTextW(
									heal_text.m_text.c_str(),
									heal_text.m_text.size(),
									m_ingametextFormat.Get(),
									&heal_text.m_Rect,
									m_textBrush.Get()
								);
							}

							else {
								TextUI_Component heal_text = TextUI_Component(DEFAULT_FONT, L"보급 쿨타임 중 (" + to_wstring((int)player->can_supply) + L"초)",
									FRAME_BUFFER_HEIGHT * 15 / 24, FRAME_BUFFER_WIDTH * 47 / 100, FRAME_BUFFER_HEIGHT * 16 / 24, FRAME_BUFFER_WIDTH * 58 / 100);
								m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::White));

								m_d2dDeviceContext->DrawTextW(
									heal_text.m_text.c_str(),
									heal_text.m_text.size(),
									m_ingametextFormat.Get(),
									&heal_text.m_Rect,
									m_textBrush.Get()
								);
							}
						}
						

					}
				}
			}

			// 미니맵
			{
				// 오른쪽가면 z증가 앞으로 가면 x감소
				float mapscale = 3960 / 396;
				int margin = 54 / 2;
				float x = margin + (int)(position->Position.z / mapscale);
				float y = margin + 50 + (int)(position->Position.x / mapscale);

				// cout << "x: " << position->Position.x << ",  z : " << position->Position.z << endl;
				D2D1_RECT_F sRect = { FRAME_BUFFER_WIDTH * 18 / 20 - margin, margin, FRAME_BUFFER_WIDTH - margin, FRAME_BUFFER_WIDTH * 2 / 20 + margin };
				imageRect = { x - FRAME_BUFFER_WIDTH / 20, y - FRAME_BUFFER_WIDTH / 20 , x + FRAME_BUFFER_WIDTH / 20, y + FRAME_BUFFER_WIDTH / 20 };

				ImageUI_Component image = ImageUI_Component(L"image/minimap.png", m_d2dDeviceContext, m_d2dFactory, m_bitmaps[1], sRect, 0.8f, D2D1_INTERPOLATION_MODE_LINEAR, imageRect);
				m_d2dDeviceContext->DrawBitmap(
					image.m_bitmap,
					image.m_Rect,
					image.m_opacity,
					image.m_mode,
					image.m_imageRect
				);

				float MapX, MapZ;

				for (auto& pos : GetUserInfo()) {
					float Xdiff = (pos.second.y - position->Position.z) / mapscale;
					float Zdiff = (pos.second.x - position->Position.x) / mapscale;

					if (Xdiff <= 0) {
						MapX = max(-FRAME_BUFFER_WIDTH / 20, Xdiff);
					}
					else {
						MapX = min(FRAME_BUFFER_WIDTH / 20, Xdiff);
					}
					if (Zdiff <= 0) {
						MapZ = max(-FRAME_BUFFER_WIDTH / 20, Zdiff);
					}
					else {
						MapZ = min(FRAME_BUFFER_WIDTH / 20, Zdiff);
					}


					D2D1_ELLIPSE playerPos = { {FRAME_BUFFER_WIDTH * 19 / 20 - margin + MapX, FRAME_BUFFER_WIDTH / 20 + margin + MapZ}, 2.0f, 2.0f };

					if (pos.first == -1) continue;
					if (pos.first == -2) {
						m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
						m_d2dDeviceContext->FillRectangle({ playerPos.point.x - playerPos.radiusX, playerPos.point.y - playerPos.radiusY , playerPos.point.x + playerPos.radiusX, playerPos.point.y + playerPos.radiusY }, m_textBrush.Get());
					}
					else if (pos.first == m_scene->getID()) {
						playerPos.radiusX += 2.0f;
						playerPos.radiusY += 2.0f;
						m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::GreenYellow));
						m_d2dDeviceContext->FillRectangle({ playerPos.point.x - playerPos.radiusX, playerPos.point.y - playerPos.radiusY , playerPos.point.x + playerPos.radiusX, playerPos.point.y + playerPos.radiusY }, m_textBrush.Get());
					}
					else {
						m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Green));
						m_d2dDeviceContext->FillEllipse(&playerPos, m_textBrush.Get());
					}
				}
			}


			{
				// 조준 크로스헤어
				if (player->aim_mode) {
					m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::Blue));
					m_d2dDeviceContext->FillEllipse(&ellipse, m_textBrush.Get());

				}
			}
			ClearUserInfo(); // 방의 유저들 좌표를 저장하던 map 초기화
		}
	);

	// 무기 발사속도 지정
	world->each<player_Component, AnimationController_Component>([&](
		Entity* ent,
		ComponentHandle<player_Component> player,
		ComponentHandle<AnimationController_Component> animation
		) -> void {
			if (player->id >= 0) {
				if (player->m_weapon == 0)
					animation->m_AnimationController->SetTrackSpeed(2, 5.0f);
				else if (player->m_weapon == 1)
					animation->m_AnimationController->SetTrackSpeed(2, 1.0f);
				else if (player->m_weapon == 2)
					animation->m_AnimationController->SetTrackSpeed(2, 0.5f);
			}
		}
	);

}

void Render_System::receive(class World* world, const  DrawComputeShader_Event& event) {
	if (m_pBlurFilter) {
		m_pBlurFilter->Execute(event.cmdList, event.input);
	}
}


float Render_System::Distance(XMFLOAT3 posA, XMFLOAT3 posB)
{
	return sqrt(pow(posB.x - posA.x, 2) + pow(posB.y - posA.y, 2) + pow(posB.z - posA.z, 2));
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

	else if (0x30 <= event.key && 0x39 >= event.key ||
		0x41 <= event.key && 0x5A >= event.key) {
		if (text[textIndex].size() >= 20) return;
		text[textIndex].insert(text[textIndex].begin() + cursorPosition[textIndex], static_cast<wchar_t>(key));
		cursorPosition[textIndex]++;
	}
	world->emit<LoginButton_Event>({ (int)text[textIndex].length() , textIndex });
	world->emit<Refresh_Scene>({ LOGIN });
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
	string password;

	id.assign(text[0].begin(), text[0].end());
	password.assign(text[1].begin(), text[1].end());

	world->emit<Login_Event>({ id, password });
}

void Render_System::receive(World* world, const SetBlur_Event& event) {
	
	switch (event.blur) {
	case 0:
		m_pBlurFilter->m_Strength = 5.0f;
		break;
	}
}

void Render_System::SetUserInfo(int uid, POINT coordinate)
{
	auto& user = UserPositionXZ.find(uid);
	if (user == UserPositionXZ.end()) {
		UserPositionXZ.insert(make_pair(uid, coordinate));
	}
	else {
		user->second = coordinate;
	}
}
