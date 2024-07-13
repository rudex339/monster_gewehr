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

// ���� ������
constexpr float MONSTER_MAX_HP = 1000;

// ��Ŷ Ÿ��
constexpr char CS_PACKET_LOGIN = 1;
constexpr char CS_PACKET_START_GAME = 2;	// ������ ���ӷ� ���� ���� ���ε��� ������ �����ϳ� ���߿� �̰� ������ ���ӽ��� ��ư�� �����ٴ� ��ȣ�� ������ ��Ŷ���� ����Ұ���
constexpr char CS_PACKET_PLAYER_MOVE = 3;
constexpr char CS_PACKET_CHANGE_ANIMATION = 4;
constexpr char CS_PACKET_PLAYER_ATTACK = 5;
constexpr char CS_PACKET_CREATE_ROOM = 6;
constexpr char CS_PACKET_SELECT_ROOM = 7;
constexpr char CS_PACKET_JOIN_ROOM = 8;
constexpr char CS_PACKET_QUIT_ROOM = 9;

// ��������� ��Ŷ
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
constexpr char SC_PACKET_GAME_START = 12; // ���ӷ� ������ ������ �����ߴٴ� ���� �˸��� ��Ŷ
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

// Ŭ�� -> ����

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

// �κ���¿��� �����̳�, ���ӹ濡 ������ ���� ��Ŷ�� ������
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


// ��Ŷ Ÿ�� ����->Ŭ��

struct SC_LOGIN_INFO_PACKET
{
	UCHAR size;
	UCHAR type;
	CHAR id;
};

struct SC_GAME_START_PACKET	// ������ ������ ���������� Ŭ���̾�Ʈ���� add_player�� �� ������ �� ��Ŷ�� �޾� �ΰ��� ������ �ٲܰ���
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

struct SC_SELECT_ROOM_PACKET	// ���ӹ濡 ���� � ������ ���� �ִ��� �غ�������� �˷��ִ� ��Ŷ
{
	UCHAR size;
	UCHAR type;
	CHAR name[20];
	CHAR weapon;
	CHAR armor;
	BOOL ready;
};
#pragma pack (pop)


