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

// 몬스터 데이터
constexpr float MONSTER_MAX_HP = 1000;

// 패킷 타입
constexpr char CS_PACKET_LOGIN = 1;
constexpr char CS_PACKET_START_GAME = 2;	// 지금은 게임룸 없이 서로 따로따로 게임을 시작하나 나중에 이건 방장이 게임시작 버튼을 눌렀다는 신호를 보내는 패킷으로 사용할것임
constexpr char CS_PACKET_PLAYER_MOVE = 3;
constexpr char CS_PACKET_CHANGE_ANIMATION = 4;
constexpr char CS_PACKET_PLAYER_ATTACK = 5;
constexpr char CS_PACKET_CREATE_ROOM = 6;
constexpr char CS_PACKET_SELECT_ROOM = 7;
constexpr char CS_PACKET_JOIN_ROOM = 8;
constexpr char CS_PACKET_QUIT_ROOM = 9;

// 데모버젼용 패킷
constexpr char CS_DEMO_MONSTER_SETPOS = 100;
constexpr char CS_DEMO_MONSTER_SETHP = 101;
constexpr char CS_DEMO_MONSTER_BEHAVIOR = 102;

constexpr char SC_PACKET_LOGIN_INFO = 1;
constexpr char SC_PACKET_ADD_PLAYER = 2;
constexpr char SC_PACKET_REMOVE_PLAYER = 3;
constexpr char SC_PACKET_UPDATE_PLAYER = 4;
constexpr char SC_PACKET_CHANGE_ANIMATION = 5;
constexpr char SC_PACKET_ADD_MONSTER = 6;
constexpr char SC_PACKET_UPDATE_MONSTER = 7;
constexpr char SC_PACKET_LOGOUT = 8;
constexpr char SC_PACKET_HIT_PLAYER = 9;
constexpr char SC_PACKET_END_GAME = 10;
constexpr char SC_PACKET_LOGIN_FAIL = 11;
constexpr char SC_PACKET_GAME_START = 12; // 게임룸 구현시 방장이 시작했다는 것을 알리는 패킷
constexpr char SC_PACKET_CREATE_ROOM = 13;
constexpr char SC_PACKET_ADD_ROOM = 14;
constexpr char SC_PACKET_SELECT_ROOM = 15;

enum class S_STATE { LOG_IN, LOBBY, SHOP, UPGRADE, ROOM, IN_ROOM, IN_GAME, LOG_OUT };

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
	CHAR weapon;
};

struct CS_START_GAME_PACKET
{
	UCHAR size;
	UCHAR type;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 vel;
	FLOAT yaw;
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
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 dir;
};

struct CS_CREATE_ROOM_PACKET
{
	UCHAR size;
	UCHAR type;
	//CHAR room_name[15];
	SHORT room_num;
};

struct CS_SELECT_ROOM_PACKET
{
	UCHAR size;
	UCHAR type;
	SHORT room_num;
};

struct CS_JOIN_ROOM_PACKET
{
	UCHAR size;
	UCHAR type;
	SHORT room_num;
};

struct CS_QUIT_ROOM_PACKET
{
	UCHAR size;
	UCHAR type;
};

struct CS_DEMO_PACKET
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


// 패킷 타입 서버->클라

struct SC_LOGIN_INFO_PACKET
{
	UCHAR size;
	UCHAR type;
	CHAR id;
};

struct SC_GAME_START_PACKET	// 방장이 게임을 시작했으면 클라이언트들이 add_player를 다 끝내고 이 패킷을 받아 인게임 씬으로 바꿀것임
{
	UCHAR size;
	UCHAR type;
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
	CHAR id;
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

struct SC_LOGOUT_PACKET
{
	UCHAR size;
	UCHAR type;
	CHAR id;
};

struct SC_HIT_PLAYER_PACKET
{
	UCHAR size;
	UCHAR type;
	CHAR id;
	FLOAT hp;
};

struct SC_END_GAME_PACKET
{
	UCHAR size;
	UCHAR type;
	SHORT score;
};

struct SC_CREATE_ROOM_PACKET
{
	UCHAR size;
	UCHAR type;
	//CHAR room_name[15];
	SHORT room_num;
};

struct SC_ADD_ROOM_PACKET
{
	UCHAR size;
	UCHAR type;
	//CHAR room_name[15];
	SHORT room_num;
};

struct SC_SELECT_ROOM_PACKET	// 게임방에 들어가면 어떤 유저가 누가 있는지 준비상태인지 알려주는 패킷
{
	UCHAR size;
	UCHAR type;
	CHAR name[20];
	CHAR weapon;
	CHAR armor;
	BOOL ready;
};
#pragma pack (pop)


