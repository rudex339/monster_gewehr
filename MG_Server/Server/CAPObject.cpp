#include "CAPObject.h"

void CAPObject::Update(float elapsedTime)
{

}

void CAPObject::Foward(float elapsedTimer)
{
	XMStoreFloat3(&m_pos, XMLoadFloat3(&m_pos) + XMLoadFloat3(&m_front) * m_speed * elapsedTimer);
}

void CAPObject::Backward(float elapsedTimer)
{
	XMStoreFloat3(&m_pos, XMLoadFloat3(&m_pos) + XMLoadFloat3(&m_front) * -m_speed * elapsedTimer);
}

void CAPObject::Left(float elapsedTimer)
{
	XMFLOAT3 rightDirection;
	XMStoreFloat3(&rightDirection, XMVector3Cross(XMLoadFloat3(&m_front), XMVectorSet(0, 1, 0, 0)));

	// 새로운 위치를 계산합니다.
	XMStoreFloat3(&m_pos, XMLoadFloat3(&m_pos) - XMLoadFloat3(&rightDirection) * m_speed * elapsedTimer);
}

void CAPObject::Right(float elapsedTimer)
{
	XMFLOAT3 rightDirection;
	XMStoreFloat3(&rightDirection, XMVector3Cross(XMLoadFloat3(&m_front), XMVectorSet(0, 1, 0, 0)));

	// 새로운 위치를 계산합니다.
	XMStoreFloat3(&m_pos, XMLoadFloat3(&m_pos) + XMLoadFloat3(&rightDirection) * m_speed * elapsedTimer);
}


void CAPObject::updateFront()
{
	XMFLOAT3 front;
	front.x = -sin(m_yaw * XM_PI / 180);
	front.y = 0.0f;
	front.z = -cos(m_yaw * XM_PI / 180);
	XMStoreFloat3(&m_front, XMVector3Normalize(XMLoadFloat3(&front)));
	
}
