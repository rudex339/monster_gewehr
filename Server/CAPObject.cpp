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

Player::Player(int id, SOCKET socket)
{
	m_id = id;
	m_socket = socket;
	m_hp = 100;
	m_max_hp = 100;
	m_atk = 50;
	m_def = 50;

	m_ammo = 0;
	m_mag = 0;

	m_wepon = 0;
	m_armor = 0;
}


void Player::SetAtkByWeapon(char weapon)
{
	switch (weapon)
	{
	case SHOT_GUN:
		m_atk = 100;
		break;
	case ASSAULT_RIFLE:
		m_atk = 30;
		break;
	case SNIPER:
		m_atk = 100;
		break;	
	}
}

void Player::SetSendBuf(void* buf, size_t size)
{
	/*ZeroMemory(m_send_buf, sizeof(m_send_buf));
	char* c = reinterpret_cast<char*>(buf);
	memcpy(m_send_buf, c, size);*/
}

void Player::RecvLogin()
{
	CS_LOGIN_PACKET lp;
	int retval = recv(m_socket, (char*)&lp, sizeof(CS_LOGIN_PACKET), 0);
	m_name = lp.id;
	m_wepon = lp.weapon;
	SetAtkByWeapon(m_wepon);
}

void Player::SendLogin()
{
	
}

int Player::RecvItemData()
{
	CS_SHOP_PACKET shop;
	int retval = recv(m_socket, (char*)&shop, sizeof(shop), 0);
	if (shop.type == 1) {
		// 여기에 아이템 구매한것
	}
	else {
		// 여기에 아이템 판매한것
	}
	return retval;
}

void Player::SendItemData()
{
}

void Player::RecvPlayerData()
{
	int retval = recv(m_socket, (char*)&cs_player_data, sizeof(cs_player_data), 0);
	if (retval == SOCKET_ERROR) {
		return;
	}
	m_position = cs_player_data.pos;
	m_velocity = cs_player_data.vel;
	m_yaw = cs_player_data.yaw;
	m_id = cs_player_data.id;
	std::cout << m_yaw << std::endl;
}

int Player::SendPlayerData(void* buf, size_t size)
{
	int retval = send(m_socket, (char*)buf, size, 0);
	return retval;
}


