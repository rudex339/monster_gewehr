#include "stdafx.h"
#include "PlayerControl_System.h"
#include "Object_Entity.h"
#include "Player_Entity.h"
#include "Render_Sysytem.h"
#include "Sever_Sysyem.h"

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

					model_vector->m_xmf3Look = Vector3::Normalize(model_vector->m_xmf3Look);
					model_vector->m_xmf3Up = Vector3::CrossProduct(model_vector->m_xmf3Look, model_vector->m_xmf3Right, true);
					model_vector->m_xmf3Right = Vector3::CrossProduct(model_vector->m_xmf3Up, model_vector->m_xmf3Look, true);
				}
			}
			else {
				//님 rotation 없음 ㅅㄱ
			}
		}


		UCHAR pKeysBuffer[256];
		if (GetKeyboardState(pKeysBuffer)) {

			float speed = 50.25f * deltaTime;

			bool run_on = false; // 달리기 상태인지 아닌지 확인해 주는거
			static float roll_timer = 0; // 구르기 하는 시간(?)
			static short roll_on = 0; // 구르기상태가 어떤지 0 일때는 안구르고 1일때는 방금 구르기 버튼 눌러서 방향 정해주는거, 2일때는 구르기 중임

			static float shot_cooltime = 0;

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
			if ((pKeysBuffer[VK_SPACE] & 0xF0) && roll_timer == 0 && !player->reload) {
				xmf3Shift_roll = XMFLOAT3(0, 0, 0);
				roll_timer = 0.2f;
				roll_on = 1;
			}
			// 달리기 키는 shift
			if (pKeysBuffer[VK_LSHIFT] & 0xF0 && !roll_on) {
				//cout << "쉬프트 눌림" << endl;
				run_on = true;
				speed *= 1.5;
			}

			if (pKeysBuffer[0x57] & 0xF0) {
				xmf3Shift = Vector3::Add(xmf3Shift, controller_vector->m_xmf3Look, speed);
				if (roll_on == 1) { // 구르기 키를 처음 눌렀으면 어디로 굴러야 하는지 방향을 정해줌
					xmf3Shift_roll = Vector3::Add(xmf3Shift_roll, controller_vector->m_xmf3Look, speed * 3);
					roll_on = 2; // 그리고 다른 키가 구르기 방향을 망치지 않도록 하기 위해 2로 바꿔줌
				}
				if (!player->reload)
					AnimationController->next_State = (UINT)RUN;
			}
			if ((pKeysBuffer[0x53] & 0xF0)) {
				xmf3Shift = Vector3::Add(xmf3Shift, controller_vector->m_xmf3Look, -speed);
				if (roll_on == 1) {
					xmf3Shift_roll = Vector3::Add(xmf3Shift_roll, controller_vector->m_xmf3Look, -speed * 3);
					roll_on = 2;
				}
				if (!player->reload)
					AnimationController->next_State = (UINT)RUN;
			}
			if (pKeysBuffer[0x41] & 0xF0) {
				xmf3Shift = Vector3::Add(xmf3Shift, model_vector->m_xmf3Right, -speed);
				if (roll_on == 1) {
					xmf3Shift_roll = Vector3::Add(xmf3Shift_roll, controller_vector->m_xmf3Right, -speed * 3);
					roll_on = 2;
				}
				if (!player->reload)
					AnimationController->next_State = (UINT)RUN;
			}
			if (pKeysBuffer[0x44] & 0xF0) {
				xmf3Shift = Vector3::Add(xmf3Shift, model_vector->m_xmf3Right, speed);
				if (roll_on == 1) {
					xmf3Shift_roll = Vector3::Add(xmf3Shift_roll, controller_vector->m_xmf3Right, speed * 3);
					roll_on = 2;
				}
				if(!player->reload)
					AnimationController->next_State = (UINT)RUN;
			}

			// 구르기가 아닐때는 그냥 일반 이동을 더하고 구르기 일때는 구르기 전용을 더함
			if (roll_on == 0) {
				velocity->m_velocity = Vector3::Add(velocity->m_velocity, xmf3Shift);
			}
			else {
				velocity->m_velocity = Vector3::Add(velocity->m_velocity, xmf3Shift_roll);
			}

			float fLength = sqrtf(velocity->m_velocity.x * velocity->m_velocity.x + velocity->m_velocity.z * velocity->m_velocity.z);
			if (::IsZero(fLength) && !player->reload)
			{
				AnimationController->next_State = (UINT)IDLE;
			}

			// 구르기 중이거나 달리는 중에는 총발사 금지해놓음
			if ((pKeysBuffer[VK_LBUTTON] & 0xF0) && !run_on && !roll_on && !player->reload) {
				AnimationController->next_State = (UINT)SHOOT;
				if (shot_cooltime <= 0) {
					shot_cooltime = 0.1f;
					world->emit<Shoot_Event>({camera->m_pCamera->GetPosition(), camera->m_pCamera->GetLookVector()});
					player->ammo--;
					if (player->ammo <= 0) {
						AnimationController->next_State = (UINT)RELOAD;
						player->reload = true;
					}
				}
				else {
					shot_cooltime -= deltaTime;
				}
			}
			else {
				shot_cooltime = 0;
			}
			if (R_btn) {
				player->aim_mode = true;
			}
			else {
				player->aim_mode = false;
			}

			if (pKeysBuffer[0x52] & 0xF0 && !player->reload) {
				AnimationController->next_State = (UINT)RELOAD;
				if (!player->reload) {
					player->reload = true;
				}
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