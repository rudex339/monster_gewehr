#pragma once

#include "stdafx.h"


class CAPObject
{
public:
	CAPObject();
	~CAPObject();	

	void SetPostion(const XMFLOAT3& pos);
	void SetPostion(float x, float y, float z);
	void SetYaw(float yaw) { m_yaw = yaw; }
	void SetID(int id)	{ m_id = id; }

	XMFLOAT3 GetPosition() { return m_position; }
	FLOAT GetYaw() { return m_yaw; }
	int GetID(int id) { return m_id; }

protected:
	
	XMFLOAT3 m_position;
	float m_yaw;
	BoundingOrientedBox m_bounding_box;
	int m_id;
};

class Player : CAPObject
{
public:
	Player();
	~Player();



protected:
	SOCKET m_socket;

	int m_hp;
	int m_atk;
	int m_def;

	char wepon_type;

};

