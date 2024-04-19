#include "stdafx.h"
#include "PlayerControl_System.h"
#include "Object_Entity.h"
#include "Render_Sysytem.h"


PlayerControl_System::PlayerControl_System(Entity* Pawn) :m_Pawn(Pawn)
{

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
			ComponentHandle<Rotation_Component> rotation =
				m_Pawn->get<Rotation_Component>();
			ComponentHandle<EulerAngle_Component> eulerangle =
				m_Pawn->get<EulerAngle_Component>();

			rotation->mfYaw += cxDelta;
			rotation->changeRotation(rotation->mfPitch,
				rotation->mfYaw, rotation->mfRoll);

			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&eulerangle->m_xmf3Up),
				XMConvertToRadians(cxDelta));
			eulerangle->m_xmf3Look = Vector3::TransformNormal(eulerangle->m_xmf3Look, xmmtxRotate);
			eulerangle->m_xmf3Right = Vector3::TransformNormal(eulerangle->m_xmf3Right, xmmtxRotate);

			eulerangle->m_xmf3Look = Vector3::Normalize(eulerangle->m_xmf3Look);
			eulerangle->m_xmf3Right = Vector3::CrossProduct(eulerangle->m_xmf3Up, eulerangle->m_xmf3Look, true);
			eulerangle->m_xmf3Up = Vector3::CrossProduct(eulerangle->m_xmf3Look, eulerangle->m_xmf3Right, true);
		}
		else {
			//´Ô rotation ¾øÀ½ ¤µ¤¡
		}
	}


	UCHAR pKeysBuffer[256];
	if (GetKeyboardState(pKeysBuffer)) {
		
		ComponentHandle<EulerAngle_Component> eulerangle =
			m_Pawn->get<EulerAngle_Component>();
		//ComponentHandle<Velocity_Component> velocity =
		//	m_Pawn->get<Velocity_Component>();
		ComponentHandle<Position_Component> position =
			m_Pawn->get<Position_Component>();

		float speed = 12.25f * deltaTime;

		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (pKeysBuffer[0x57] & 0xF0) {
			xmf3Shift = Vector3::Add(xmf3Shift, eulerangle->m_xmf3Look, speed);
		}
		if (pKeysBuffer[0x53] & 0xF0) {
			xmf3Shift = Vector3::Add(xmf3Shift, eulerangle->m_xmf3Look, -speed);
		}
		if (pKeysBuffer[0x41] & 0xF0) {
			xmf3Shift = Vector3::Add(xmf3Shift, eulerangle->m_xmf3Right, -speed);
		}
		if (pKeysBuffer[0x44] & 0xF0) {
			xmf3Shift = Vector3::Add(xmf3Shift, eulerangle->m_xmf3Right, speed);
		}
		
		XMFLOAT3 xm_Velocity = XMFLOAT3(0.f, 0.f, 0.f);
		xm_Velocity = Vector3::Add(xm_Velocity, xmf3Shift);

		position->Position = Vector3::Add(position->Position, xmf3Shift);

		position->m_xmf4x4World._41 += xm_Velocity.x;
		position->m_xmf4x4World._42 += xm_Velocity.y;
		position->m_xmf4x4World._43 += xm_Velocity.z;
	}
	//if(m_Pawn->has())
}

void PlayerControl_System::receive(World* world, const CaptureHWND_Event& event)
{
	Capture = event.capture;
}

void PlayerControl_System::receive(World* world, const CursorPos_Event& event)
{
	m_OldCursorPos = event.m_CursorPos;
}

