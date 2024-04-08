#pragma once
#include "stdafx.h"

constexpr short SERVER_PORT = 8000;

constexpr int BUF_SIZE = 500;
constexpr int MAX_CLIENT = 10;
constexpr int MAX_GAME_ROOM = 5;
constexpr int MAX_CLIENT_ROOM = 4;

// ��Ŷ Ÿ�� ����->Ŭ��
constexpr char SC_PACKET_PLAYER_DATA = 2;

struct SC_PLAYER_DATA
{
	UCHAR size;
	UCHAR type;
	INT id;
	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT yaw;
};


struct CS_PLAYER_DATA
{
	UCHAR size;
	UCHAR type;
	INT id;
	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT yaw;
};
