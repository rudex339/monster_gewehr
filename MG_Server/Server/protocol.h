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

enum class S_STATE{SHOP, UPGRADE, ROOM, IN_ROOM, IN_GAME};

#pragma pack(push,1)
struct PLAYER_DATA
{
	
	CHAR id = -1;
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

// 클라 -> 서버

struct CS_LOGIN_PACKET
{
	UCHAR size;
	UCHAR type;
	CHAR name[15];
	CHAR wepon;
};

struct CS_PLAYER_PACKET
{
	CHAR id;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 vel;
	FLOAT yaw;
};

struct CS_LOBBY_PACKET
{
	CHAR type;
};

struct CS_SHOP_PACKET
{
	CHAR type;
	CHAR item;
};

struct CS_ROOM_PACKET
{
	CHAR type;
	CHAR room_name[15];
	INT room_num;

};

// 패킷 타입 서버->클라

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


