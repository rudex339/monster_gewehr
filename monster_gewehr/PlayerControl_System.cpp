#include "stdafx.h"
#include "PlayerControl_System.h"
#include "Object_Entity.h"
#include "Player_Entity.h"
#include "Render_Sysytem.h"
#include "Sever_Sysyem.h"

void ExtractEulerAngles(const XMFLOAT4X4& m_xmf4x4World, float& pitch, float& yaw, float& roll) {
	// 회전 행렬 추출
	XMMATRIX rotationMatrix = XMLoadFloat4x4(&m_xmf4x4World);

	// 회전 행렬의 요소
	float r11 = m_xmf4x4World._11, r12 = m_xmf4x4World._12, r13 = m_xmf4x4World._13;
	float r21 = m_xmf4x4World._21, r22 = m_xmf4x4World._22, r23 = m_xmf4x4World._23;
	float r31 = m_xmf4x4World._31, r32 = m_xmf4x4World._32, r33 = m_xmf4x4World._33;

	// 오일러 각 계산
	pitch = std::asin(-r31);
	if (std::cos(pitch) != 0) {
		yaw = std::atan2(r21, r11);
		roll = std::atan2(r32, r33);
	}
	else {
		yaw = 0;
		roll = std::atan2(-r12, r22);
	}

	// 각도 단위로 변환 (라디안 -> 도)
	pitch = XMConvertToDegrees(pitch);
	yaw = XMConvertToDegrees(yaw);
	roll = XMConvertToDegrees(roll);
}


float AngleBetweenVectors(const XMFLOAT3& vec1, const XMFLOAT3& vec2, float a)
{
	XMVECTOR v1 = XMLoadFloat3(&vec1);
	XMVECTOR v2 = XMLoadFloat3(&vec2);

	// Normalize the vectors
	v1 = XMVector3Normalize(v1);
	v2 = XMVector3Normalize(v2);

	// Calculate the dot product of the two vectors
	float dotProduct = XMVectorGetX(XMVector3Dot(v1, v2));

	// Calculate the cross product to determine the direction of rotation
	XMVECTOR crossProduct = XMVector3Cross(v1, v2);

	// Calculate the angle between the vectors in radians
	float angle = acosf(dotProduct);

	// Determine the sign of the angle based on the direction of rotation
	if (a < 0.0f)
	{
		angle = -angle;
	}

	// Convert the angle from radians to degrees
	angle = XMConvertToDegrees(angle);

	return angle;
	//1 외적 , 
}



void PlayerControl_System::configure(World* world)
{
	world->subscribe<CaptureHWND_Event>(this);
	world->subscribe<CursorPos_Event>(this);
	world->subscribe<GetPlayerPtr_Event>(this);
	
}

void PlayerControl_System::unconfigure(World* world)
{
}

void PlayerControl_System::tick(World* world, float deltaTime)
{
	//cout << deltaTime << endl;
	if (m_Pawn) {
		ComponentHandle<EulerAngle_Component> model_vector =
			m_Pawn->get<EulerAngle_Component>();
		ComponentHandle<ControllAngle_Component> controller_vector =
			m_Pawn->get<ControllAngle_Component>();
		ComponentHandle<Velocity_Component> velocity =
			m_Pawn->get<Velocity_Component>();
		ComponentHandle<Position_Component> position =
			m_Pawn->get<Position_Component>();
		ComponentHandle<Camera_Component> camera =
			m_Pawn->get<Camera_Component>();
		ComponentHandle<Rotation_Component> rotation =
			m_Pawn->get<Rotation_Component>();
		ComponentHandle<AnimationController_Component> AnimationController =
			m_Pawn->get<AnimationController_Component>();
		ComponentHandle<player_Component> player =
			m_Pawn->get< player_Component>();
		ComponentHandle<Model_Component> model =
			m_Pawn->get<Model_Component>();

		float cxDelta = 0.0f, cyDelta = 0.0f;
		if (Capture) {

			//RECT clientRect;
			//GetClientRect(hWnd, &clientRect); // Get the dimensions of the client area

			//int centerX = (clientRect.left + clientRect.right) / 2;
			//int centerY = (clientRect.top + clientRect.bottom) / 2;

			POINT ptCursorPos;
			SetCursor(NULL);
			GetCursorPos(&ptCursorPos);
			cxDelta = (float)(ptCursorPos.x - m_OldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_OldCursorPos.y) / 3.0f;
			SetCursorPos(m_OldCursorPos.x, m_OldCursorPos.y);

			if (m_Pawn->has<Rotation_Component>() &&
				m_Pawn->has<EulerAngle_Component>()) {

				velocity->m_velRotate.x += cxDelta;
				//velocity->m_velRotate.y += cyDelta;

				XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&controller_vector->m_xmf3Up),
					XMConvertToRadians(cxDelta));
				model_vector->m_xmf3Look = Vector3::TransformNormal(model_vector->m_xmf3Look, xmmtxRotate);
				model_vector->m_xmf3Right = Vector3::TransformNormal(model_vector->m_xmf3Right, xmmtxRotate);

				model_vector->m_xmf3Look = Vector3::Normalize(model_vector->m_xmf3Look);
				model_vector->m_xmf3Up = Vector3::CrossProduct(model_vector->m_xmf3Look, model_vector->m_xmf3Right, true);
				model_vector->m_xmf3Right = Vector3::CrossProduct(model_vector->m_xmf3Up, model_vector->m_xmf3Look, true);


				
				controller_vector->m_xmf3Look = Vector3::TransformNormal(controller_vector->m_xmf3Look, xmmtxRotate);
				controller_vector->m_xmf3Right = Vector3::TransformNormal(controller_vector->m_xmf3Right, xmmtxRotate);

				controller_vector->m_xmf3Look = Vector3::Normalize(controller_vector->m_xmf3Look);
				controller_vector->m_xmf3Up = Vector3::CrossProduct(controller_vector->m_xmf3Look, controller_vector->m_xmf3Right, true);
				controller_vector->m_xmf3Right = Vector3::CrossProduct(controller_vector->m_xmf3Up, controller_vector->m_xmf3Look, true);



				float a = XMVectorGetX(DirectX::XMVector3Dot(XMLoadFloat3(&model_vector->m_xmf3Right),
					DirectX::XMVector3Cross(XMLoadFloat3(&controller_vector->m_xmf3Look), XMLoadFloat3(&Vector3::TransformNormal(model_vector->m_xmf3Look, xmmtxRotate)))));
				//cout << a << endl;

				xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&controller_vector->m_xmf3Right),
					XMConvertToRadians(cyDelta));

				if (AngleBetweenVectors(controller_vector->m_xmf3Look, Vector3::TransformNormal(model_vector->m_xmf3Look, xmmtxRotate), a) < 60.f &&
					AngleBetweenVectors(controller_vector->m_xmf3Look, Vector3::TransformNormal(model_vector->m_xmf3Look, xmmtxRotate), a) > -25.f) {

					model_vector->m_xmf3Look = Vector3::TransformNormal(model_vector->m_xmf3Look, xmmtxRotate);
					model_vector->m_xmf3Right = Vector3::TransformNormal(model_vector->m_xmf3Right, xmmtxRotate);
					model_vector->m_xmf3Look = Vector3::Add(model_vector->m_xmf3Look, {0.0f, gun_rebound, 0.0f});

					model_vector->m_xmf3Look = Vector3::Normalize(model_vector->m_xmf3Look);
					model_vector->m_xmf3Up = Vector3::CrossProduct(model_vector->m_xmf3Look, model_vector->m_xmf3Right, true);
					model_vector->m_xmf3Right = Vector3::CrossProduct(model_vector->m_xmf3Up, model_vector->m_xmf3Look, true);
				}
			}
			else {
				//님 rotation 없음 ㅅㄱ
			}
		}

		if (player->reload) {
			if (player->reload_coolTime <= 0) {
				cout << "리로드 완료" << endl;
				if (player->mag + player->ammo >= weapon_ammo[player->m_weapon]) {
					player->mag -= (weapon_ammo[player->m_weapon] - player->ammo);
					player->ammo = weapon_ammo[player->m_weapon];
				}
				else {
					player->ammo += player->mag;
					player->mag = 0;
				}
				player->reload_coolTime = 3.5f;
				player->reload = false;
				
			}
			else {
				player->reload_coolTime -= deltaTime;
			}
		}

		if (heal_on) {
			if (player->heal_timer <= 0) {
				cout << "힐 완료" << endl;
				player->hp += heal_amount[heal_type];	// 이것도 치유한 힐템에 따라서 달라지게
				if (player->hp > 100) {
					player->hp = 100.f;
				}
				heal_on = false;
				world->emit<Heal_Event>({ player->hp, heal_type });
				world->emit<UseItem_Event>({ heal_type });
				player->heal_item[heal_type]--;
				cout << "남은 힐템 : " << player->heal_item[heal_type] << endl;
			}
			else {
				player->heal_timer -= deltaTime;
			}
		}

		player->is_suppling = false;
		if (!player->near_supply) {
			supply_on = false;
		}
		if (supply_on) {
			player->is_suppling = true;
			if (player->supply_timer <= 0) {
				cout << "보급 완료" << endl;
				supply_on = false;
				player->can_supply = 180.0f; // 쿨타임 적용

				player->ammo = weapon_ammo[player->m_weapon];
				player->mag = weapon_mag[player->m_weapon];
				
				// 힐템 추가?
			}
			else {
				player->supply_timer -= deltaTime;
			}
		}
		
		player->can_supply -= deltaTime;


		UCHAR pKeysBuffer[256];
		if (GetKeyboardState(pKeysBuffer)) {
			
			float speed = player_speed[player->m_armor - 3] * deltaTime;
			//float speed = 1050.25f * deltaTime;

			if (player->aim_mode) {
				speed *= 0.5f;
			}

			bool run_on = false; // 달리기 상태인지 아닌지 확인해 주는거
			bool shift_key_press = false;

			XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);

			// 구르기 속도와 방향이 이 틱이 지나도 유지되어야 하기 때문에 static으로 지정함
			static XMFLOAT3 xmf3Shift_roll = XMFLOAT3(0, 0, 0); // 구르기 시스템 전용 속도

			// 구르기 지속중인가 지속이끝나면 구르기 타이머랑 상태 초기화
			if (roll_timer > 0) {
				roll_timer -= deltaTime;
			}
			else {
				roll_timer = 0;
				roll_on = 0;
			}

			// 구르기 키는 space바로 했음
			if ((pKeysBuffer[VK_SPACE] & 0xF0) && roll_timer == 0 && !player->reload && player->stamina >= 25 && !heal_on) {
				xmf3Shift_roll = XMFLOAT3(0, 0, 0);
				roll_timer = 0.2f;
				roll_on = 1;
				player->stamina -= 25;
				Sound_Componet::GetInstance().PlaySound(Sound_Componet::Sound::Dash);
			}
			// 달리기 키는 shift
			if (pKeysBuffer[VK_LSHIFT] & 0xF0 && !roll_on  && !heal_on && !player->reload) {
				shift_key_press = true;
				if (!stamina_empty && player->stamina > 0) {
					run_on = true;
					speed *= 1.5;
					player->stamina -= 0.1;
					AnimationController->next_State = (UINT)FASTRUN;
				}
			}

			// 힐키 (1 : 붕대, 2 : 구상, 3 : 주사기)
			if (pKeysBuffer[0x31] & 0xF0 && !roll_on && !player->reload && player->heal_item[0] > 0 && !heal_on && !supply_on) {
				heal_on = true;
				player->heal_timer = healtime[0];
				player->heal_all_time = healtime[0];
				heal_type = 0;
				AnimationController->next_State = (UINT)HEAL;
			}

			if (pKeysBuffer[0x32] & 0xF0 && !roll_on && !player->reload && player->heal_item[1] > 0 && !heal_on && !supply_on) {
				heal_on = true;
				player->heal_timer = healtime[1];
				player->heal_all_time = healtime[1];
				heal_type = 1;
				AnimationController->next_State = (UINT)HEAL;
			}

			if (pKeysBuffer[0x33] & 0xF0 && !roll_on && !player->reload && player->heal_item[2] > 0 && !heal_on && !supply_on) {
				heal_on = true;
				player->heal_timer = healtime[2];
				player->heal_all_time = healtime[2];
				heal_type = 2;
				AnimationController->next_State = (UINT)HEAL;
			}

			// 상호작용 f키(보급 받는 키 or 회복 취소 키)
			if (pKeysBuffer[0x46] & 0xF0) {

				// 회복취소
				if (heal_on) {
					heal_on = false;
					player->heal_timer = 0;
				}

				if (player->near_supply) {
					// 보급이 가능한지 여부 체크(힐중이거나 보급 시간이 안되면 실행x)
					if (player->can_supply < 0.f && !heal_on) {
						supply_on = true;
						player->supply_timer = player->supply_time;
					}
				}

			}

			// 움직이는키 wsad
			if (pKeysBuffer[0x57] & 0xF0) {
				xmf3Shift = Vector3::Add(xmf3Shift, controller_vector->m_xmf3Look, speed);
				if (roll_on == 1) { // 구르기 키를 처음 눌렀으면 어디로 굴러야 하는지 방향을 정해줌
					xmf3Shift_roll = Vector3::Add(xmf3Shift_roll, controller_vector->m_xmf3Look, speed * 3);					
				}
				if (!player->reload && !run_on && !heal_on && !player->reload && !player->aim_mode)
					AnimationController->next_State = (UINT)RUN;
			}
			if ((pKeysBuffer[0x53] & 0xF0)) {
				xmf3Shift = Vector3::Add(xmf3Shift, controller_vector->m_xmf3Look, -speed);
				if (roll_on == 1) {
					xmf3Shift_roll = Vector3::Add(xmf3Shift_roll, controller_vector->m_xmf3Look, -speed * 3);
				}
				if (!player->reload && !run_on && !heal_on && !player->reload && !player->aim_mode)
					AnimationController->next_State = (UINT)RUN;
			}
			if (pKeysBuffer[0x41] & 0xF0) {
				xmf3Shift = Vector3::Add(xmf3Shift, model_vector->m_xmf3Right, -speed);
				if (roll_on == 1) {
					xmf3Shift_roll = Vector3::Add(xmf3Shift_roll, controller_vector->m_xmf3Right, -speed * 3);
				}
				if (!player->reload && !run_on && !heal_on && !player->reload && !player->aim_mode)
					AnimationController->next_State = (UINT)RUN;
			}
			if (pKeysBuffer[0x44] & 0xF0) {
				xmf3Shift = Vector3::Add(xmf3Shift, model_vector->m_xmf3Right, speed);
				if (roll_on == 1) {
					xmf3Shift_roll = Vector3::Add(xmf3Shift_roll, controller_vector->m_xmf3Right, speed * 3);					
				}
				if(!player->reload && !run_on && !heal_on && !player->reload && !player->aim_mode)
					AnimationController->next_State = (UINT)RUN;
			}

			if (roll_on == 1) {
				roll_on = 2; // 그리고 다른 키가 구르기 방향을 망치지 않도록 하기 위해 2로 바꿔줌
			}

			// 투척 g키
			if (pKeysBuffer[0x47] & 0xF0 && player->grenade_amount) {
				//투척물에 필요한것, 그려아하니까 위치, 회전, scale, 모델, 수류탄 컴포넌트
				
				world->emit<CreateObject_Event>({ granade, 
					XMFLOAT3(model->blankSocketList["Granade"].second._41,
						model->blankSocketList["Granade"].second._42,
						model->blankSocketList["Granade"].second._43),
					XMFLOAT3(rotation->mfPitch,rotation->mfYaw,rotation->mfRoll),
					model_vector->m_xmf3Look});
				//player->grenade_amount -= 1;
			}

			// 구르기가 아닐때는 그냥 일반 이동을 더하고 구르기 일때는 구르기 전용을 더함
			if (roll_on == 0) {
				velocity->m_velocity = Vector3::Add(velocity->m_velocity, xmf3Shift);
			}
			else {
				velocity->m_velocity = Vector3::Add(velocity->m_velocity, xmf3Shift_roll);
			}

			float fLength = sqrtf(velocity->m_velocity.x * velocity->m_velocity.x + velocity->m_velocity.z * velocity->m_velocity.z);
			// 만약 속도가 없으면 idle애니, 아니면 달리기나 걷기소리 내기 속도만 있으면
			if (::IsZero(fLength) && !player->reload && !heal_on && !player->aim_mode)
			{
				AnimationController->next_State = (UINT)IDLE;
			}
			else if (!(::IsZero(fLength)) && !roll_on) {
				if (!run_on)
					Sound_Componet::GetInstance().PlayMoveSound(Sound_Componet::Sound::Walk);
				else
					Sound_Componet::GetInstance().PlayMoveSound(Sound_Componet::Sound::Run);
			}


			if (R_btn) {
				player->aim_mode = true;
			}
			else {
				player->aim_mode = false;
			}

			// 구르기 중이거나 달리는 중에는 총발사 금지해놓음
			if ((pKeysBuffer[VK_LBUTTON] & 0xF0) && !run_on && !roll_on && !player->reload && player->ammo > 0 && !heal_on) {				
				if (shot_cooltime <= 0) {
					AnimationController->next_State = (UINT)SHOOT;
					shot_cooltime = shot_cooltime_list[player->m_weapon];					
					world->emit<ShootGun_Event>({ (int)player->m_weapon, camera->m_pCamera->GetPosition(), camera->m_pCamera->GetLookVector() }); //  이건 콜리전 시스템

					Sound_Componet::GetInstance().PlaySound(player->m_weapon+3);
					player->ammo--;
					if (player->ammo <= 0 && player->mag > 0) {
						AnimationController->next_State = (UINT)RELOAD;
						player->reload = true;
						Sound_Componet::GetInstance().PlaySound(Sound_Componet::Sound::Reload);
					}
				}
				else {
					shot_cooltime -= deltaTime;
				}
			}
			// 우클릭 조준
			else if ((pKeysBuffer[VK_RBUTTON] & 0xF0) && !run_on && !roll_on && !player->reload && !heal_on) {	
				AnimationController->next_State = (UINT)AIM;
				player->aim_mode = true;
				shot_cooltime -= deltaTime;
			}
			else {
				shot_cooltime -= deltaTime;
				player->aim_mode = false;
			}
			
			switch (player->m_weapon)
			{
			case 0:
				if (shot_cooltime > shot_cooltime_list[player->m_weapon] / 2) {
					gun_rebound = 0.02f;
				}
				else {
					gun_rebound = 0.0f;
				}
				break;
			case 1:
				if (shot_cooltime > shot_cooltime_list[player->m_weapon]* 0.8f) {
					gun_rebound = 0.04f;
				}
				else if (shot_cooltime > shot_cooltime_list[player->m_weapon] * 0.7f) {
					gun_rebound = -0.04f;
				}
				else {
					gun_rebound = 0.0f;
				}
				break;
			case 2:
				if (shot_cooltime > shot_cooltime_list[player->m_weapon] * 0.9f) {
					gun_rebound = 0.03f;
				}
				else if(shot_cooltime > shot_cooltime_list[player->m_weapon] * 0.8f ) {
					gun_rebound = -0.03f;
				}
				else {
					gun_rebound = 0.0f;
				}
				break;
			default:
				break;
			}
			

			// r키로 재장전
			if (pKeysBuffer[0x52] & 0xF0 && !player->reload && player->mag > 0 && !roll_on && !heal_on ) {
				AnimationController->next_State = (UINT)RELOAD;
				Sound_Componet::GetInstance().PlaySound(Sound_Componet::Sound::Reload);
				if (!player->reload) {
					player->reload = true;
				}
			}
			if (player->stamina <= 0) {
				stamina_empty = true;
			}
			if (!shift_key_press) {
				stamina_empty = false;
			}
			if (!run_on && !roll_on && player->stamina <= 100) {
				player->stamina += 0.1;
			}


#ifdef DEMO_VER
			if (pKeysBuffer[VK_F1] & 0xF0) {
				world->emit<Demo_Event>({ CS_DEMO_MONSTER_SETPOS });
			}
			if (pKeysBuffer[VK_F2] & 0xF0) {
				world->emit<Demo_Event>({ CS_DEMO_MONSTER_SETHP });
			}
			if (pKeysBuffer[VK_F3] & 0xF0) {
				world->emit<Demo_Event>({ CS_DEMO_MONSTER_BEHAVIOR });
			}
#endif
			m_Pawn->get<player_Component>()->m_velocity = velocity->m_velocity;
			//velocity->m_velocity = Vector3::Add(velocity->m_velocity, XMFLOAT3(0, -65.4f* deltaTime, 0));
			//cout << position->Position.x << " " << position->Position.z << endl;
			//cout << rotation->mfYaw << endl;
		}

	}
}

void PlayerControl_System::receive(World* world, const CaptureHWND_Event& event)
{
	Capture = event.capture;
	R_btn = event.rbd;
}

void PlayerControl_System::receive(World* world, const CursorPos_Event& event)
{
	m_OldCursorPos = event.m_CursorPos;
}

void PlayerControl_System::receive(World* world, const GetPlayerPtr_Event& event)
{
	if (event.enable) {
		world->enableSystem(this);
		m_Pawn = event.Pawn;
		//ComponentHandle<EulerAngle_Component> eulerangle =
		//	m_Pawn->get<EulerAngle_Component>();
		if (m_Pawn) {
			if (m_Pawn->has<Camera_Component>())
				world->emit<SetCamera_Event>({ m_Pawn->get<Camera_Component>()->m_pCamera });
			else {
				world->emit<SetCamera_Event>({ NULL });
			}
		}
	}
	else {
		m_Pawn = NULL;
		world->disableSystem(this);
	}
}