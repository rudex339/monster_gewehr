#pragma once

#include "CAPGlobal.h"


class CAPObject
{
public:
	CAPObject();
	~CAPObject();

	void SetSize(float x, float y, float z);
	void SetPos(float x, float y, float z);
	void GetPos(float* x, float* y, float* z);
	void SetMass(float mass);
	float GetMass();
	void SetVel(float x, float y, float z);
	void GetVel(float* x, float* y, float* z);
	float GetVelMag();
	void SetAcc(float x, float y, float z);
	void SetForce(float x, float y, float z);
	void SetType(int type);
	int GetType();

	void SetID(int id);
	int GetID(int id);
	void SetParent(int id);
	bool isAncestor(int id);

	void SetHP(float hp);
	float GetHP();

	void Update(float elapsedTime);

	void Foward(float elapsedTimer);
	void Backward(float elapsedTimer);
	void Left(float elapsedTimer);
	void Right(float elapsedTimer);
	void updateFront();

	void AddForce(float x, float y, float z, float elapsedTime);

private:
	// �������� ��ǥ��� ��������
	XMFLOAT3 m_pos;
	XMFLOAT3 m_front;
	XMFLOAT3 m_vel;
	XMFLOAT3 m_acc;
	XMFLOAT3 m_forceX;
	float m_mass = 0.f;
	float m_yaw;
	float m_speed;

	// Ÿ��(ĳ��������, ��������, �Ѿ�����)
	int m_type = TYPE_DEFAULT;

	// ���� ID
	int m_ID = -1;
	int m_parentID = -1;

	// ü��
	float m_HP = 0;

	// �÷��̾�� �κ����� �ΰ������� ����, ���ʹ� ���¸ӽſ� ����Ҽ� ����?����
	int m_state = STATE_DEFAULT;

};

