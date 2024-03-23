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
	// 기초적인 좌표들과 물리량들
	XMFLOAT3 m_pos;
	XMFLOAT3 m_front;
	XMFLOAT3 m_vel;
	XMFLOAT3 m_acc;
	XMFLOAT3 m_forceX;
	float m_mass = 0.f;
	float m_yaw;
	float m_speed;

	// 타입(캐릭터인지, 몬스터인지, 총알인지)
	int m_type = TYPE_DEFAULT;

	// 고유 ID
	int m_ID = -1;
	int m_parentID = -1;

	// 체력
	float m_HP = 0;

	// 플레이어는 로비인지 인게임인지 감지, 몬스터는 상태머신에 사용할수 있을?지도
	int m_state = STATE_DEFAULT;

};

