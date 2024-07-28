#include "stdafx.h"
#include "Object_Entity.h"
#include "ObjectManager.h"
#include "Player_Entity.h"


Entity* AddSoldierObject(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	ObjectManager* OM,
	float x, float y, float z,
	float rx, float ry, float rz,
	float sx, float sy, float sz)
{
	CLoadedModelInfo* model = OM->Get_ModelInfo("Soldier");
	auto Mcomponent = ent->assign<Model_Component>(model->m_pModelRootObject, model->m_pModelRootObject->m_pstrFrameName);
	Mcomponent->SetSocket("Granade", "Bip001_R_Hand");

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
		new SoldierAnimationController(pd3dDevice, pd3dCommandList, 9, model, ent), 0);
	for (int i = 0; i < 9; i++) {
		controller->m_AnimationController->SetTrackAnimationSet(i, i);
		//controller->m_AnimationController->SetTrackEnable(i, false);
		//controller->m_AnimationController->SetTrackType(i, ANIMATION_TYPE_ONCE);
	}
	//controller->m_AnimationController->SetTrackWeight(8, 0.f);
	controller->m_AnimationController->SetTrackEnable(0, true);
	controller->m_AnimationController->SetTrackWeight(0, 1.f);
	controller->m_AnimationController->SetBlendingSpeed(2, 8.0f);
	controller->m_AnimationController->SetBlendingSpeed(6, 8.0f);
	
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
		new CAnimationController(pd3dDevice, pd3dCommandList, 15, model), 0);
	for (int i = 0; i < 15; i++) {
		controller->m_AnimationController->SetTrackAnimationSet(i, i);
		controller->m_AnimationController->SetTrackEnable(i, false);

		//controller->m_AnimationController->SetTrackType(i, ANIMATION_TYPE_ONCE);
	}

	// 한번만 재생하도록 수정
	controller->m_AnimationController->SetTrackType(1, ANIMATION_TYPE_ONCE);
	controller->m_AnimationController->SetTrackType(6, ANIMATION_TYPE_ONCE);
	controller->m_AnimationController->SetTrackType(9, ANIMATION_TYPE_ONCE);
	controller->m_AnimationController->SetTrackType(10, ANIMATION_TYPE_ONCE);
	controller->m_AnimationController->SetTrackType(14, ANIMATION_TYPE_ONCE);

	controller->m_AnimationController->SetBlendingSpeed(0, 100.0f);
	/*
	0 IDLE,
	1 GROW,
	2 WALK,
	3 FLYUP,
	4 FLYING,
	5 LANDING,
	6 BITE,
	7 RUN,
	8 HIT,
	9 DIE,
	10 TailAttack,
	11 Blind(차징),
	12 FLYIDLE,
	13 SLEEP,
	14 FIREBALL 
	*/

	// 몬스터 애니메이션 속도 조절
	controller->m_AnimationController->SetTrackSpeed(1, 0.5f);
	controller->m_AnimationController->SetTrackSpeed(3, 0.7f);
	controller->m_AnimationController->SetTrackSpeed(5, 0.7f);
	controller->m_AnimationController->SetTrackSpeed(7, 2.0f);
	controller->m_AnimationController->SetTrackSpeed(10, 1.0f);
	controller->m_AnimationController->SetTrackSpeed(11, 0.05f);
	controller->m_AnimationController->SetTrackSpeed(14, 0.5f);

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
	m_paragraph_alignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
	m_text_alignment = DWRITE_TEXT_ALIGNMENT_CENTER;
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

TextBoxUI_Component::TextBoxUI_Component(float layoutX, float layoutY, float width, float height, int num)
{
	x = layoutX;
	y = layoutY;
	m_width = width;
	m_height = height;
	index = num;

}

bool TextBoxUI_Component::CursorInBox(POINT cursor)
{
	if (cursor.x > x && cursor.x < x + m_width && cursor.y > y && cursor.y < y + m_height) {
		return true;
	}

	return false;
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
		if (!cursor_on) {
			Sound_Componet::GetInstance().PlaySound(Sound_Componet::Sound::B_On);
		}
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

void Model_Component::SetSocket(string socketName, string frameName)
{
	blankSocketList[socketName] = make_pair(frameName, Matrix4x4::Identity());
}

Sound_Componet::Sound_Componet()
{
	m_result = FMOD::System_Create(&m_system);
	m_result = m_system->init(32, FMOD_INIT_NORMAL, nullptr);
	m_result = m_system->set3DSettings(1.0f, 1.0f, 1.0f);

	m_result = m_system->createSound("Sound/Music/title.mp3", FMOD_LOOP_NORMAL, 0, &m_music[Music::Title]);
	m_result = m_system->createSound("Sound/Music/ingame3.mp3", FMOD_LOOP_NORMAL, 0, &m_music[Music::Ingame]);
	m_result = m_system->createSound("Sound/Music/gameclear.mp3", FMOD_LOOP_OFF, 0, &m_music[Music::GameClear]);
	m_result = m_system->createSound("Sound/Music/gamefail.mp3", FMOD_LOOP_OFF, 0, &m_music[Music::GameFail]);
	
	m_result = m_system->createSound("Sound/Effect/button_on.mp3", FMOD_LOOP_OFF, 0, &m_sound[Sound::B_On]);
	m_result = m_system->createSound("Sound/Effect/button_push.mp3", FMOD_LOOP_OFF, 0, &m_sound[Sound::B_Push]);
	m_result = m_system->createSound("Sound/Effect/purchase.mp3", FMOD_LOOP_OFF, 0, &m_sound[Sound::Purchase]);
	m_result = m_system->createSound("Sound/Effect/rifle.mp3", FMOD_LOOP_OFF, 0, &m_sound[Sound::Rifle]);
	m_result = m_system->createSound("Sound/Effect/shotgun.mp3", FMOD_LOOP_OFF, 0, &m_sound[Sound::ShotGun]);	
	m_result = m_system->createSound("Sound/Effect/sniper.mp3", FMOD_LOOP_OFF, 0, &m_sound[Sound::Sniper]);
	m_result = m_system->createSound("Sound/Effect/walk.mp3", FMOD_LOOP_OFF, 0, &m_sound[Sound::Walk]);
	m_result = m_system->createSound("Sound/Effect/run.mp3", FMOD_LOOP_OFF, 0, &m_sound[Sound::Run]);
	m_result = m_system->createSound("Sound/Effect/reload.mp3", FMOD_LOOP_OFF, 0, &m_sound[Sound::Reload]);
	m_result = m_system->createSound("Sound/Effect/dash.mp3", FMOD_LOOP_OFF, 0, &m_sound[Sound::Dash]);
	m_result = m_system->createSound("Sound/Effect/hurt.mp3", FMOD_LOOP_OFF, 0, &m_sound[Sound::Hurt]);

	m_result = m_system->createSound("Sound/Effect/rifle.mp3", FMOD_3D, 0, &m_3dsound[TDSound::TDRifle]);
	m_result = m_3dsound[TDSound::TDRifle]->set3DMinMaxDistance(100.f, 5000.f);
	m_result = m_3dsound[TDSound::TDRifle]->setMode(FMOD_LOOP_OFF);

	m_result = m_system->createSound("Sound/Effect/shotgun.mp3", FMOD_3D, 0, &m_3dsound[TDSound::TDShotGun]);
	m_result = m_3dsound[TDSound::TDShotGun]->set3DMinMaxDistance(100.f, 5000.f);
	m_result = m_3dsound[TDSound::TDShotGun]->setMode(FMOD_LOOP_OFF);

	m_result = m_system->createSound("Sound/Effect/sniper.mp3", FMOD_3D, 0, &m_3dsound[TDSound::TDSniper]);
	m_result = m_3dsound[TDSound::TDSniper]->set3DMinMaxDistance(100.f, 5000.f);
	m_result = m_3dsound[TDSound::TDSniper]->setMode(FMOD_LOOP_OFF);
	
	m_musicChannel->setVolume(3.0f);
}

Sound_Componet::~Sound_Componet()
{
	m_system->release();
}

void Sound_Componet::PlayMusic(Music tag)
{
	bool playing;
	m_result = m_musicChannel->isPlaying(&playing);
	if (playing) m_musicChannel->stop();
	m_result = m_system->playSound(m_music[tag], 0, false, &m_musicChannel);
}

void Sound_Componet::StopMusic()
{
	bool playing;
	m_result = m_musicChannel->isPlaying(&playing);
	if (playing) m_musicChannel->stop();
}

void Sound_Componet::PlaySound(Sound tag)
{
	bool playing;
	for (auto& channel : m_soundChannel) {
		m_result = channel->isPlaying(&playing);
		if (!playing) {
			m_result = m_system->playSound(m_sound[tag], 0, false, &channel);
			break;
		}
	}
}

void Sound_Componet::PlaySound(int type)
{
	bool playing;
	for (auto& channel : m_soundChannel) {
		m_result = channel->isPlaying(&playing);
		if (!playing) {
			m_result = m_system->playSound(m_sound[type], 0, false, &channel);
			break;
		}
	}
}

void Sound_Componet::PlayMoveSound(Sound tag)
{
	bool playing;
	m_result = m_movesoundChannel[tag - Sound::Walk]->isPlaying(&playing);
	if (!playing) {
		m_result = m_system->playSound(m_sound[tag], 0, false, &m_movesoundChannel[tag - Sound::Walk]);
	}
}

// 이밑부터는 다른사람 소리 들을때 3d 효과 넣는 함수들
void Sound_Componet::Play3DSound(XMFLOAT3 sound, TDSound tag)
{
	FMOD_VECTOR soundPos = { sound.x, sound.y, sound.z };
	FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f };
	
	bool playing;
	for (auto& channel : m_soundChannel) {
		m_result = channel->isPlaying(&playing);
		if (!playing) {	
			m_result = m_system->playSound(m_3dsound[tag], 0, true, &channel);
			m_result = channel->set3DAttributes(&soundPos, &velocity);
			m_result = channel->setPaused(false);
			break;
		}
	}
}

void Sound_Componet::ListenerUpdate(XMFLOAT3 pos, XMFLOAT3 vel, XMFLOAT3 front, XMFLOAT3 up)
{	
	FMOD_VECTOR listenerPos = { pos.x, pos.y, pos.z };
	FMOD_VECTOR velocity = { vel.x, vel.y, vel.z };
	FMOD_VECTOR forward = { front.x, front.y, front.z };
	FMOD_VECTOR sound_up = { up.x, up.y, up.z };

	m_result = m_system->set3DListenerAttributes(0, &listenerPos, &velocity, &forward, &sound_up);
	if (m_result != FMOD_OK) {
		std::cerr << "Failed to set listener attributes: " << "\n";
	}

	m_result = m_system->update();
	if (m_result != FMOD_OK) {
		std::cerr << "Failed to update system: " << "\n";
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoldierAnimationController::SoldierAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel, Entity* owner) :
	CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pModel), m_owner(owner)
{
	weight_Under[0] = 1.f;
}

SoldierAnimationController::~SoldierAnimationController()
{
}

void SoldierAnimationController::AdvanceTime(float fTimeElapsed, GameObjectModel* pRootGameObject)
{
	m_fTime += fTimeElapsed;
	XMFLOAT3 velocity = m_owner->get<player_Component>()->m_velocity;
	velocityXZ = Vector3::Length(velocity);
	//float player_speed[2] = { 50.25f, 40.25f };
	//ent�� xz �ӵ��� ���ѵ� �� �ӵ��� 0�̸� idle 0�̻��̸� run run �ӵ� �̻��̸� fastrun

	//���� �ӵ��� 40.25*deltatime �̻��ΰ�? run�� fastrun
	for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++) m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = Matrix4x4::Zero();

	if (velocityXZ > 45.f * fTimeElapsed) {
		weight_Under[0] = 0.f;

		weight_Under[8] += m_pAnimationTracks[8].m_blendingSpeed * fTimeElapsed;
		weight_Under[1] = 1.0 - weight_Under[8];
	}
	//else���� �ӵ��� 0 �̻��ΰ�? idle�� run
	else if (velocityXZ > 0)
	{
		weight_Under[8] = 0.f;

		weight_Under[1] += m_pAnimationTracks[1].m_blendingSpeed * fTimeElapsed;
		weight_Under[0] = 1.0 - weight_Under[1];
	}
	else {
		weight_Under[8] = 0.f;

		weight_Under[0] += m_pAnimationTracks[0].m_blendingSpeed * fTimeElapsed;
		weight_Under[1] = 1.0 - weight_Under[0];

		// weight_Under ������ 0�� 1 ���̷� ����

	}
	for (int i = 0; i < 10; ++i) {
		if (weight_Under[i] < 0.0f) {
			weight_Under[i] = 0.0f;
		}
		else if (weight_Under[i] > 1.0f) {
			weight_Under[i] = 1.0f;
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (m_pAnimationTracks)
	{
		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++) m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = Matrix4x4::Zero();
		float blendingSpeed = 0.f;
		for (int k = 0; k < m_nAnimationTracks; k++) {
			if (m_pAnimationTracks[k].m_bEnable) {
				blendingSpeed = m_pAnimationTracks[k].m_blendingSpeed;
			}
		}
		for (int k = 0; k < m_nAnimationTracks; k++) {
			if (m_pAnimationTracks[k].m_bEnable) {
				m_pAnimationTracks[k].m_fWeight += blendingSpeed * fTimeElapsed;
				if (m_pAnimationTracks[k].m_fWeight > 1.f) {
					m_pAnimationTracks[k].m_fWeight = 1.f;
				}
			}
			else {
				m_pAnimationTracks[k].m_fWeight -= blendingSpeed * fTimeElapsed;
				if (m_pAnimationTracks[k].m_fWeight < 0.f) {
					m_pAnimationTracks[k].m_fWeight = 0.f;
				}
			}
		}

		for (int k = 0; k < m_nAnimationTracks; k++)
		{

			if (m_pAnimationTracks[k].m_fWeight > 0.f || weight_Under[k] > 0.f)
			{
				bool up = false;
				CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet];
				float fPosition = m_pAnimationTracks[k].UpdatePosition(m_pAnimationTracks[k].m_fPosition, fTimeElapsed, pAnimationSet->m_fLength);
				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)//���ϸ��̼� ���� �� ������ ��ȯ ����� ������
				{
					XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent;
					XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);//���⼭ ���� �������� ��ȯ ���� ������
					if (up) {
						xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, Matrix4x4::Scale(xmf4x4TrackTransform, m_pAnimationTracks[k].m_fWeight));
					}
					else {
						xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, Matrix4x4::Scale(xmf4x4TrackTransform, weight_Under[k]));
						if (!strcmp(m_pAnimationSets->m_ppBoneFrameCaches[j]->m_pstrFrameName, "Bip001_Spine")) {
							up = true;
						}
					}
					m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
				}
				m_pAnimationTracks[k].HandleCallback();
			}
		}

		pRootGameObject->UpdateTransform(NULL);

		OnRootMotion(pRootGameObject);
		OnAnimationIK(pRootGameObject);
	}
}

void SoldierAnimationController::Animate(float fElapsedTime)
{
	if (m_owner->has<Velocity_Component>()) {
		XMFLOAT3 velocity = m_owner->get<Velocity_Component>()->m_velocity;
		velocityXZ = Vector3::Length(velocity);
	}
	else {
		XMFLOAT3 velocity = m_owner->get<player_Component>()->m_velocity;
		velocityXZ = Vector3::Length(velocity);
	}
}