#pragma once

#include "stdafx.h"


class CAPObject
{
public:
	CAPObject();
	~CAPObject() = default;	

	void SetPostion(const XMFLOAT3& pos) { m_position = pos; }
	void SetPostion(float x, float y, float z);
	void SetYaw(float yaw) { m_yaw = yaw; }
	void SetID(int id)	{ m_id = id; }

	XMFLOAT3 GetPosition() { return m_position; }
	FLOAT GetYaw() { return m_yaw; }
	int GetID() { return m_id; }

protected:
	
	XMFLOAT3 m_position;
	float m_yaw;
	BoundingOrientedBox m_bounding_box;
	int m_id;
};

class Player : public CAPObject
{
public:
	Player();
	~Player();

	void SetSocket(SOCKET& sock) { m_socket = sock; }
	void SetName(std::wstring name) { m_name = name; }
	void SetLock() { m_lock.lock(); }
	void SetUnLock() { m_lock.unlock(); }
	void SetHp(float hp) { m_hp = hp; }
	void SetMaxHp(float max_hp) { m_max_hp = max_hp; }
	void SetAtk(float atk) { m_atk = atk; }
	void SetDef(float def) { m_def = def; }
	void SetAmmo(float ammo) { m_ammo = ammo; }
	void SetMag(float mag) { m_mag = mag; }
	void SetWepon(char wepon) { m_wepon = wepon; }
	void SetArmor(char armor) { m_armor = armor; }

	void Recv_Player_move();

protected:
	SOCKET m_socket;
	SC_PLAYER_DATA sc_player_data;

	std::wstring m_name;

	std::mutex m_lock;

	float m_hp;
	float m_max_hp;
	float m_atk;
	float m_def;

	float m_ammo;	// ÇöÀç ÃÑÇÕ ÃÑ¾Ë(¼ÒÁöÁß Åº¾à)
	float m_mag;	// ÇöÀç ÅºÃ¢¿¡ µé¾îÀÖ´Â Åº¾à

	char m_wepon;
	char m_armor;


};

