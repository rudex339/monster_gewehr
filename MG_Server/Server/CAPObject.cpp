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

void Player::Recv_Player_move()
{
	int retval = recv(m_socket, (char*)&sc_player_data, sizeof(sc_player_data), 0);

}
