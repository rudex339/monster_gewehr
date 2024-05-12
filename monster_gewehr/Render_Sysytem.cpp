#include "stdafx.h"
#include "Render_Sysytem.h"
#include "Object_Entity.h"
#include "ObjectManager.h"
#include "Player_Entity.h"

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

	if (distance > 2000.0f) {
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


Render_Sysytem::Render_Sysytem(ObjectManager* manager, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID2D1DeviceContext2* d2dDeviceContext, ID2D1Factory3* d2dFactory, IDWriteFactory5* dwriteFactory, ID2D1Bitmap* bitmap)
{
	SetRootSignANDDescriptorANDCammandlist(manager, pd3dCommandList);

	m_d2dDeviceContext = d2dDeviceContext;
	m_dwriteFactory = dwriteFactory;
	m_d2dFactory = d2dFactory;
	m_bitmaps[0] = bitmap;
		
	m_xmf4GlobalAmbient = XMFLOAT4(0.50f, 0.50f, 0.50f, 1.0f);

	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);



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

	LoadBitmapFromFile(L"image/soldierFace.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[0]);
	//LoadBitmapFromFile(L"image/16317fbf7667c044.png", m_d2dDeviceContext, m_d2dFactory, &m_bitmaps[1]);

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

void Render_Sysytem::configure(World* world)
{
	world->subscribe<SetCamera_Event>(this);
	world->subscribe<DrawUI_Event>(this);
}

void Render_Sysytem::unconfigure(World* world)
{
	world->unsubscribeAll(this);
}

void Render_Sysytem::tick(World* world, float deltaTime)
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
					Model->m_MeshModel->m_pModelRootObject->Render(
						m_pd3dCommandList, m_pCamera);
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


					Model->m_MeshModel->m_pModelRootObject->UpdateTransform(&xmf4x4World);

					AnimationController->m_AnimationController->UpdateShaderVariables();

					Model->m_MeshModel->m_pModelRootObject->Render(
						m_pd3dCommandList, m_pCamera);

				}
				else if (!should_render(XMLoadFloat3(&m_pCamera->GetPosition()), XMLoadFloat3(&m_pCamera->GetLookVector()), XMLoadFloat3(&pos->Position))) {
					Model->m_MeshModel->m_pModelRootObject->UpdateTransform(&pos->m_xmf4x4World);
					Model->m_MeshModel->m_pModelRootObject->Render(
						m_pd3dCommandList, m_pCamera);
				}
			});
	}
}

void Render_Sysytem::receive(World* world, const SetCamera_Event& event)
{
	m_pCamera = event.pCamera;
}

void Render_Sysytem::receive(World* world, const DrawUI_Event& event)
{
	m_textBrush.Get()->SetColor(D2D1::ColorF(D2D1::ColorF::White));
	world->each<TextUI_Component>([&](
		Entity* ent,
		ComponentHandle<TextUI_Component> textUI
		) -> void {			
			m_d2dDeviceContext->DrawTextW(
				textUI->m_text.data(),
				textUI->m_text.size(),
				m_textFormat.Get(),
				&textUI->m_Rect,
				m_textBrush.Get()
			);
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

void Render_Sysytem::SetRootSignANDDescriptorANDCammandlist(ObjectManager* manager, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dCommandList = pd3dCommandList;
	m_pd3dGraphicsRootSignature = manager->GetGraphicsRootSignature();
	m_pd3dCbvSrvDescriptorHeap = manager->GetCbvSrvDescriptorHeap();
}
