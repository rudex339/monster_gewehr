#pragma once
#include "stdafx.h"

constexpr short SERVER_PORT = 8000;

constexpr int BUF_SIZE = 500;
constexpr int MAX_CLIENT = 10;
constexpr int MAX_GAME_ROOM = 5;
constexpr int MAX_CLIENT_ROOM = 4;

constexpr char SHOT_GUN = 1;
constexpr char ASSAULT_RIFLE = 2;
constexpr char SNIPER = 3;

constexpr char SC_PACKET_PLAYER_DATA = 2;

enum class S_STATE{LOBBY, SHOP, UPGRADE, ROOM, IN_ROOM, IN_GAME};

#pragma pack(push,1)
struct PLAYER_DATA
{
	
	CHAR id;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 vel;
	FLOAT yaw;
	CHAR wepon;
};

struct MONSTER_DATA
{
	CHAR id;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 vel;
	FLOAT yaw;
	CHAR animaition;
};

struct ITEM_DATA
{
	SHORT bandage;
	SHORT firstaidkit;
	SHORT Adrenaline;
	SHORT grenade;
	SHORT flashbang;

};

// Ŭ�� -> ����

struct CS_LOGIN_PACKET
{
	//UCHAR size;
	//UCHAR type;
	CHAR id[20];
	CHAR weapon;
};

struct CS_PLAYER_PACKET
{
	CHAR id;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 vel;
	FLOAT yaw;
};


// �κ���¿��� �����̳�, ���ӹ濡 ������ ���� ��Ŷ�� ������
struct CS_LOBBY_PACKET
{
	CHAR type;
};

struct CS_SHOP_PACKET
{
	CHAR type;
	CHAR item;
	SHORT amount;
};

struct CS_ROOM_PACKET
{
	CHAR type;
	CHAR room_name[15];
	INT room_num;

};

// �̰� �ϳ��� ��Ŷ���� Ÿ�Ը����� �����ؼ� �����¹�(���� �������� ��Ŷ�� Ŀ��)
struct CS_LOBBY_PACKET2
{
	CHAR type;
	CHAR item;
	CHAR room_name[15];
	INT room_num;
};

// ��Ŷ Ÿ�� ����->Ŭ��

struct SC_LOGIN_OK_PACKET
{
	CHAR is_true;
};

struct SC_PLAYER_PACKET
{
	PLAYER_DATA players[MAX_CLIENT_ROOM];	
};


struct SC_OBJECT_PACKET
{
	PLAYER_DATA players[MAX_CLIENT_ROOM];
	MONSTER_DATA monster;
};
#pragma pack (pop)


