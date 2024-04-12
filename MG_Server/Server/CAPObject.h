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
	XMFLOAT3 GetVelocity() { return m_velocity; }
	FLOAT GetYaw() { return m_yaw; }
	int GetID() { return m_id; }
	PLAYER_DATA GetData() { return { m_id, m_position, m_velocity, m_yaw }; }

protected:
	
	XMFLOAT3 m_position;
	XMFLOAT3 m_velocity;
	FLOAT m_yaw;
	BoundingOrientedBox m_bounding_box;
	CHAR m_id;
};

class Player : public CAPObject
{
public:
	Player();
	~Player() = default; ;
	Player(int id, SOCKET socket);

	void SetSocket(SOCKET& sock) { m_socket = sock; }
	void SetName(std::string name) { m_name = name; }
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
	void SetAtkByWeapon(char aromor);


	void SetSendBuf(void* buf, size_t size);

	void RecvLogin();
	void SendLogin();

	void RecvPlayerData();
	void SendPlayerData(void* buf, size_t size);

	//char m_send_buf[BUF_SIZE];
	char m_recv_buf[BUF_SIZE];
protected:
	SOCKET m_socket;
	SC_PLAYER_PACKET sc_player_data;
	CS_PLAYER_PACKET cs_player_data;

	

	std::string m_name;

	std::mutex m_lock;

	FLOAT m_hp;
	FLOAT m_max_hp;
	FLOAT m_atk;
	FLOAT m_def;

	FLOAT m_ammo;	// ÇöÀç ÃÑÇÕ ÃÑ¾Ë(¼ÒÁöÁß Åº¾à)
	FLOAT m_mag;	// ÇöÀç ÅºÃ¢¿¡ µé¾îÀÖ´Â Åº¾à

	CHAR m_wepon;
	CHAR m_armor;


};

