#include "CAPObject.h"

CAPObject::CAPObject()
{
	m_position = { 0.f, 0.f, 0.f };
	m_yaw = 0.f;
	m_id = -1;
}

void CAPObject::SetPostion(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
}

Player::Player()
{
	m_socket = {};

	m_name = {};

	m_hp = 0;
	m_max_hp = 0;
	m_atk = 0;
	m_def = 0;

	m_ammo = 0;
	m_mag = 0;

	m_wepon = 0;
	m_armor = 0;
}

void Player::Player_Init(int id)
{
	m_id = id;
	m_hp = 100;
	m_max_hp = 100;
}

void Player::SetSendBuf(void* buf, size_t size)
{
	ZeroMemory(m_send_buf, sizeof(m_send_buf));
	char* c = reinterpret_cast<char*>(buf);
	memcpy(m_send_buf, c, size);
}

void Player::Recv_Player_Data()
{

	int retval = recv(m_socket, (char*)&cs_player_data, sizeof(cs_player_data), 0);
	m_position.x = cs_player_data.x;
	m_position.y = cs_player_data.y;
	m_position.z = cs_player_data.z;
	m_yaw = cs_player_data.yaw;
}

void Player::Send_Player_Data(void* buf, size_t size)
{
	int retval = send(m_socket, (char*)buf, size, 0);
}


