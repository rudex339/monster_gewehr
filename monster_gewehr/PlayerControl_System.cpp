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
	//1 ¿ÜÀû , 
}



PlayerControl_System::PlayerControl_System(Entity* Pawn) :m_Pawn(Pawn)
{
	ComponentHandle<EulerAngle_Component> eulerangle =
		m_Pawn->get<EulerAngle_Component>();

}

void PlayerControl_System::configure(World* world)
{
	world->subscribe<CaptureHWND_Event>(this);
	world->subscribe<CursorPos_Event>(this);

	if(m_Pawn->has<Camera_Component>())
		world->emit<SetCamera_Event>({ m_Pawn->get<Camera_Component>()->m_pCamera });
}

void PlayerControl_System::unconfigure(World* world)
{
}

void PlayerControl_System::tick(World* world, float deltaTime)
{
	ComponentHandle<EulerAngle_Component> eulerangle =
		m_Pawn->get<EulerAngle_Component>();
	ComponentHandle<ControllAngle_Component> controllangle =
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

	float cxDelta = 0.0f, cyDelta = 0.0f;
	if (Capture) {
		


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
			
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&controllangle->m_xmf3Up),
				XMConvertToRadians(cxDelta));
			eulerangle->m_xmf3Look = Vector3::TransformNormal(eulerangle->m_xmf3Look, xmmtxRotate);
			eulerangle->m_xmf3Right = Vector3::TransformNormal(eulerangle->m_xmf3Right, xmmtxRotate);

			eulerangle->m_xmf3Look = Vector3::Normalize(eulerangle->m_xmf3Look);
			eulerangle->m_xmf3Up = Vector3::CrossProduct(eulerangle->m_xmf3Look, eulerangle->m_xmf3Right, true);
			eulerangle->m_xmf3Right = Vector3::CrossProduct(eulerangle->m_xmf3Up, eulerangle->m_xmf3Look, true);
			


			controllangle->m_xmf3Look = Vector3::TransformNormal(controllangle->m_xmf3Look, xmmtxRotate);
			controllangle->m_xmf3Right = Vector3::TransformNormal(controllangle->m_xmf3Right, xmmtxRotate);

			controllangle->m_xmf3Look = Vector3::Normalize(controllangle->m_xmf3Look);
			controllangle->m_xmf3Right = Vector3::CrossProduct(controllangle->m_xmf3Up, controllangle->m_xmf3Look, true);
			controllangle->m_xmf3Up = Vector3::CrossProduct(controllangle->m_xmf3Look, controllangle->m_xmf3Right, true);


			float a = XMVectorGetX(DirectX::XMVector3Dot(XMLoadFloat3(& eulerangle->m_xmf3Right),
				DirectX::XMVector3Cross(XMLoadFloat3(&controllangle->m_xmf3Look), XMLoadFloat3(&Vector3::TransformNormal(eulerangle->m_xmf3Look, xmmtxRotate)))));
			cout << a << endl;

			xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&controllangle->m_xmf3Right),
				XMConvertToRadians(cyDelta));

				if (AngleBetweenVectors(controllangle->m_xmf3Look, Vector3::TransformNormal(eulerangle->m_xmf3Look, xmmtxRotate),a) < 70.f&&
					AngleBetweenVectors(controllangle->m_xmf3Look, Vector3::TransformNormal(eulerangle->m_xmf3Look, xmmtxRotate),a) > -105.f) {

					eulerangle->m_xmf3Look = Vector3::TransformNormal(eulerangle->m_xmf3Look, xmmtxRotate);
					eulerangle->m_xmf3Right = Vector3::TransformNormal(eulerangle->m_xmf3Right, xmmtxRotate);

					eulerangle->m_xmf3Look = Vector3::Normalize(eulerangle->m_xmf3Look);
					eulerangle->m_xmf3Up = Vector3::CrossProduct(eulerangle->m_xmf3Look, eulerangle->m_xmf3Right, true);
					eulerangle->m_xmf3Right = Vector3::CrossProduct(eulerangle->m_xmf3Up, eulerangle->m_xmf3Look, true);
				}
			

			
			
		}
		else {
			//´Ô rotation ¾øÀ½ ¤µ¤¡
		}
	}


	UCHAR pKeysBuffer[256];
	if (GetKeyboardState(pKeysBuffer)) {

		float speed = 50.25f * deltaTime;

		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (pKeysBuffer[0x57] & 0xF0) {
			xmf3Shift = Vector3::Add(xmf3Shift, controllangle->m_xmf3Look, speed);
			//xmf3Shift = Vector3::Add(xmf3Shift, eulerangle->m_xmf3Look, speed);
			AnimationController->next_State = (UINT)RUN;
		}
		if (pKeysBuffer[0x53] & 0xF0) {
			xmf3Shift = Vector3::Add(xmf3Shift, controllangle->m_xmf3Look, -speed);
			//xmf3Shift = Vector3::Add(xmf3Shift, eulerangle->m_xmf3Look, -speed);
			AnimationController->next_State = (UINT)RUN;
		}
		if (pKeysBuffer[0x41] & 0xF0) {
			xmf3Shift = Vector3::Add(xmf3Shift, eulerangle->m_xmf3Right, -speed);
			AnimationController->next_State = (UINT)RUN;
		}
		if (pKeysBuffer[0x44] & 0xF0) {
			xmf3Shift = Vector3::Add(xmf3Shift, eulerangle->m_xmf3Right, speed);
			AnimationController->next_State = (UINT)RUN;
		}

		velocity->m_velocity = Vector3::Add(velocity->m_velocity, xmf3Shift);		
		float fLength =  sqrtf(velocity->m_velocity.x * velocity->m_velocity.x + velocity->m_velocity.z * velocity->m_velocity.z);
		if (::IsZero(fLength))
		{
			AnimationController->next_State = (UINT)IDLE;
		}

		if (pKeysBuffer[VK_LBUTTON] & 0xF0) {
			AnimationController->next_State = (UINT)SHOOT;
		}
	}

	
}

void PlayerControl_System::receive(World* world, const CaptureHWND_Event& event)
{
	Capture = event.capture;
}

void PlayerControl_System::receive(World* world, const CursorPos_Event& event)
{
	m_OldCursorPos = event.m_CursorPos;
}

