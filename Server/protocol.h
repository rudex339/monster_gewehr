#pragma once
#include "stdafx.h"

constexpr short SERVER_PORT = 8000;

constexpr int BUF_SIZE = 500;
constexpr int MAX_CLIENT = 10;
constexpr int MAX_ID_LENGTH = 20;

constexpr int MAX_GAME_ROOM = 5;
constexpr int MAX_CLIENT_ROOM = 4;

constexpr char SHOT_GUN = 1;
constexpr char ASSAULT_RIFLE = 2;
constexpr char SNIPER = 3;

// 패킷 타입
constexpr char CS_PACKET_LOGIN = 1;
constexpr char CS_PACKET_PLAYER_MOVE = 2;
constexpr char CS_PACKET_CHANGE_ANIMATION = 3;

constexpr char SC_PACKET_LOGIN_INFO = 1;
constexpr char SC_PACKET_ADD_PLAYER = 2;
constexpr char SC_PACKET_REMOVE_PLAYER = 3;
constexpr char SC_PACKET_UPDATE_PLAYER = 4;
constexpr char SC_PACKET_CHANGE_ANIMATION = 5;
constexpr char SC_PACKET_ADD_MONSTER = 6;
constexpr char SC_PACKET_UPDATE_MONSTER = 7;

enum class S_STATE { LOBBY, SHOP, UPGRADE, ROOM, IN_ROOM, IN_GAME, LOG_OUT };

#pragma pack(push,1)
struct PLAYER_DATA
{
	CHAR id;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 vel;
	FLOAT yaw;
	FLOAT hp;
};

struct MONSTER_DATA
{
	CHAR id;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 vel;
	FLOAT yaw;
	FLOAT hp;
};

struct ITEM_DATA
{
	SHORT bandage;
	SHORT firstaidkit;
	SHORT Adrenaline;
	SHORT grenade;
	SHORT flashbang;

};

// 클라 -> 서버

struct CS_LOGIN_PACKET
{
	UCHAR size;
	UCHAR type;
	CHAR name[20];
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 vel;
	FLOAT yaw;
	CHAR weapon;
};

struct CS_PLAYER_MOVE_PACKET
{
	UCHAR size;
	UCHAR type;
	DirectX::XMFLOAT3 pos;
	FLOAT yaw;
};

struct CS_CHANGE_ANIMATION_PACKET
{
	UCHAR size;
	UCHAR type;
	CHAR animation;
};

struct CS_PLAYER_ATTACK_PACKET
{
	UCHAR size;
	UCHAR type;
};

struct CS_RELOAD_PACKET
{
	UCHAR size;
	UCHAR type;
};


// 로비상태에서 상점이나, 게임방에 들어갔을때 보낼 패킷을 나눠둠
struct CS_LOBBY_PACKET
{
	UCHAR size;
	UCHAR type;
};

struct CS_SHOP_PACKET
{
	UCHAR size;
	UCHAR type;
	UCHAR item;
	SHORT amount;
};

struct CS_ROOM_PACKET
{
	UCHAR size;
	UCHAR type;
	UCHAR item;
	CHAR room_name[15];
	INT room_num;

};

// 패킷 타입 서버->클라

struct SC_LOGIN_INFO_PACKET
{
	UCHAR size;
	UCHAR type;
	CHAR id;
};

struct SC_ADD_PLAYER_PACKET
{
	UCHAR size;
	UCHAR type;
	CHAR name[MAX_ID_LENGTH];
	PLAYER_DATA player_data;
	CHAR weapon;
};

struct SC_REMOVE_PLAYER_PACKET
{
	UCHAR size;
	UCHAR type;
	CHAR id;
};

struct SC_UPDATE_PLAYER_PACKET
{
	UCHAR size;
	UCHAR type;
	PLAYER_DATA player_data;
};

struct SC_CHANGE_ANIMATION_PACKET
{
	UCHAR size;
	UCHAR type;
	CHAR animation;
};

struct SC_ADD_MONSTER_PACKET
{
	UCHAR size;
	UCHAR type;
	MONSTER_DATA monster;
};

struct SC_UPDATE_MONSTER_PACKET
{
	UCHAR size;
	UCHAR type;
	MONSTER_DATA monster;
	CHAR animation;
};


struct SC_OBJECT_PACKET
{
	PLAYER_DATA players[MAX_CLIENT_ROOM];
	MONSTER_DATA monster;
};
#pragma pack (pop)


