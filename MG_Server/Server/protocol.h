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

#pragma pack(push,1)
struct PLAYER_DATA
{
	CHAR id = -1;
	XMFLOAT3 pos;
	XMFLOAT3 vel;
	FLOAT yaw;
};

struct MONSTER_DATA
{
	CHAR id;
	XMFLOAT3 pos;
	XMFLOAT3 vel;
	FLOAT yaw;
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
	XMFLOAT3 pos;
	XMFLOAT3 vel;
	FLOAT yaw;
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


