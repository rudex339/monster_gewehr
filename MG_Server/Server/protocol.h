#pragma once
#include "stdafx.h"

constexpr short SERVER_PORT = 8000;

constexpr int BUF_SIZE = 500;
constexpr int MAX_CLIENT = 10;
constexpr int MAX_GAME_ROOM = 5;
constexpr int MAX_CLIENT_ROOM = 4;

struct SC_PLAYER_DATA
{
	int id;
	float x;
	float y;
	float z;
	float yaw;

	float hp;
	float m_max_hp;

	char wepon;
	char armor;
};
