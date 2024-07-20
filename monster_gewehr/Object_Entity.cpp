#include "stdafx.h"
#include "Object_Entity.h"
#include "ObjectManager.h"


Entity* AddSoldierObject(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	ObjectManager* OM,
	float x, float y, float z,
	float rx, float ry, float rz,
	float sx, float sy, float sz)
{
	CLoadedModelInfo* model = OM->Get_ModelInfo("Soldier");
	auto Mcomponent = ent->assign<Model_Component>(model->m_pModelRootObject, model->m_pModelRootObject->m_pstrFrameName);

	Model_Component* temp_mComponet = new Model_Component(OM->Get_ModelInfo("M4A1")->m_pModelRootObject,
		OM->Get_ModelInfo("M4A1")->m_pModelRootObject->m_pstrFrameName);
	temp_mComponet->SetSocket(model->m_pModelRootObject, "Bip001_R_Hand");
	Mcomponent.get().addChildComponent(temp_mComponet);

	temp_mComponet = new Model_Component(OM->Get_ModelInfo("benelliM4")->m_pModelRootObject,
		OM->Get_ModelInfo("benelliM4")->m_pModelRootObject->m_pstrFrameName);
	temp_mComponet->SetSocket(model->m_pModelRootObject, "Bip001_R_Hand");
	Mcomponent.get().addChildComponent(temp_mComponet);
	temp_mComponet->draw = false;

	temp_mComponet = new Model_Component(OM->Get_ModelInfo("M110")->m_pModelRootObject,
		OM->Get_ModelInfo("M110")->m_pModelRootObject->m_pstrFrameName);
	temp_mComponet->SetSocket(model->m_pModelRootObject, "Bip001_R_Hand");
	Mcomponent.get().addChildComponent(temp_mComponet);
	temp_mComponet->draw = false;

	auto controller = ent->assign<AnimationController_Component>(
		new CAnimationController(pd3dDevice, pd3dCommandList, 6, model), 0);
	for (int i = 0; i < 6; i++) {
		controller->m_AnimationController->SetTrackAnimationSet(i, i);
		controller->m_AnimationController->SetTrackEnable(i, false);
		//controller->m_AnimationController->SetTrackType(i, ANIMATION_TYPE_ONCE);
	}
	controller->m_AnimationController->SetTrackEnable(0, true);
	controller->m_AnimationController->SetTrackWeight(0,1.f);
	ent->assign<Position_Component>(x, y, z);
	ent->assign<Rotation_Component>(rx, ry, rz);
	ent->assign<Scale_Component>(sx, sy, sz);

	return ent;
}

Entity* AddMonsterObject(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	ObjectManager* OM,
	float x, float y, float z,
	float rx, float ry, float rz,
	float sx, float sy, float sz)
{
	CLoadedModelInfo* model = OM->Get_ModelInfo("Souleater");
	ent->assign<Model_Component>(model->m_pModelRootObject, model->m_pModelRootObject->m_pstrFrameName);
	auto controller = ent->assign<AnimationController_Component>(
		new CAnimationController(pd3dDevice, pd3dCommandList, 10, model), 0);
	for (int i = 0; i < 10; i++) {
		controller->m_AnimationController->SetTrackAnimationSet(i, i);
		controller->m_AnimationController->SetTrackEnable(i, false);
		//controller->m_AnimationController->SetTrackType(i, ANIMATION_TYPE_ONCE);
	}
	controller->m_AnimationController->SetTrackType(9, ANIMATION_TYPE_ONCE);

	ent->assign<Position_Component>(x, y, z);
	ent->assign<Rotation_Component>(rx, ry, rz);
	ent->assign<Scale_Component>(sx, sy, sz);

	return ent;
}


EulerAngle_Component::EulerAngle_Component(float R_x, float R_y, float R_z, float L_x, float L_y, float L_z, float U_x, float U_y, float U_z)
{
	m_xmf3Right = XMFLOAT3(R_x, R_y, R_z);
	m_xmf3Look = XMFLOAT3(L_x, L_y, L_z);
	m_xmf3Up = XMFLOAT3(U_x, U_y, U_z);
}

//----------------------------------------------------------------------------------------------
//UI

HRESULT LoadBitmapFromFiles(const wchar_t* imagePath, ID2D1DeviceContext2* d2dDeviceContext, ID2D1Factory3* d2dFactory, ID2D1Bitmap** ppBitmap)
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

TextUI_Component::TextUI_Component(int fontType, const wchar_t* text, float top, float left, float bottom, float right) {
	m_text = text;
	m_Rect = D2D1::RectF(left, top, right, bottom);
	m_fontType = fontType;
	m_paragraph_alignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
	m_text_alignment = DWRITE_TEXT_ALIGNMENT_CENTER;
}

TextUI_Component::TextUI_Component(int fontType, wstring text, float top, float left, float bottom, float right)
{
	m_text = text;
	m_Rect = D2D1::RectF(left, top, right, bottom);
	m_fontType = fontType;
}

ImageUI_Component::ImageUI_Component(const wchar_t* imagePath, ID2D1DeviceContext2* deviceContext, ID2D1Factory3* factory, ID2D1Bitmap* bitmap, D2D1_RECT_F posrect, float opacity, D2D1_INTERPOLATION_MODE mode, D2D1_RECT_F imagerect)
{
	m_d2dDeviceContext = deviceContext;
	m_d2dFactory = factory;
	m_bitmap = bitmap;
	m_Rect = posrect;
	m_imageRect = imagerect;
	m_mode = mode;
	m_opacity = opacity;

	LoadBitmapFromFiles(imagePath, m_d2dDeviceContext, m_d2dFactory, &m_bitmap);
}

TextBoxUI_Component::TextBoxUI_Component(float layoutX, float layoutY, int num)
{
	x = layoutX;
	y = layoutY;
	index = num;
}

Button_Component::Button_Component(int id, const wchar_t* imagePath, int fontType, wstring text, ID2D1DeviceContext2* deviceContext, ID2D1Factory3* factory, ID2D1Bitmap* bitmap, D2D1_RECT_F posrect, float opacity, D2D1_INTERPOLATION_MODE mode, D2D1_RECT_F imagerect, int num)
{
	button_id = id;
	m_text = text;
	m_d2dDeviceContext = deviceContext;
	m_d2dFactory = factory;
	m_bitmap = bitmap;
	m_Rectsaved = m_Rect = posrect;
	m_imageRect = imagerect;
	m_mode = mode;
	m_opacity = opacity;
	cursor_on = false;
	
	m_fontType = fontType;

	activate = true;

	m_paragraph_alignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
	m_text_alignment = DWRITE_TEXT_ALIGNMENT_CENTER;

	if (button_id == SelectRoomBtn) {
		m_room_num = num;
	}
	else if (button_id == ItemBtn || button_id == EquipLeftBtn || button_id == EquipRightBtn) {
		item_num = num;
	}

	if (imagePath != NULL) {
		LoadBitmapFromFiles(imagePath, m_d2dDeviceContext, m_d2dFactory, &m_bitmap);
	}
}

void Button_Component::CursorOn(POINT cursor, ComPtr<IDWriteTextFormat> big_font, ComPtr<IDWriteTextFormat> small_font)
{
	if (!activate) {
		return;
	}
	if (cursor.x > m_Rectsaved.left && cursor.x < m_Rectsaved.right && cursor.y > m_Rectsaved.top && cursor.y < m_Rectsaved.bottom) {
		m_Rect.bottom = m_Rectsaved.bottom + (m_Rectsaved.bottom - m_Rectsaved.top) * 0.05;
		m_Rect.right = m_Rectsaved.right + (m_Rectsaved.right - m_Rectsaved.left) * 0.05;
		m_Rect.left = m_Rectsaved.left - (m_Rectsaved.right - m_Rectsaved.left) * 0.05;
		m_Rect.top = m_Rectsaved.top - (m_Rectsaved.bottom - m_Rectsaved.top) * 0.05;
		cursor_on = true;
		m_textFormat = big_font;
	}
	else {
		m_Rect = m_Rectsaved;
		cursor_on = false;
		m_textFormat = small_font;
	}
}

void Button_Component::Disable()
{
	activate = false;
	m_opacity = 0.5f;
}

void Button_Component::Activate()
{
	activate = true;
	m_opacity = 1.0f;
}

void Model_Component::addChildComponent(Model_Component* child)
{
	child->m_pParentObject = this;
	m_pchildObjects.push_back(child);
}

void Model_Component::SetSocket(GameObjectModel* rootModel, char* name)
{
	socket = rootModel->FindFrame(name);
}
