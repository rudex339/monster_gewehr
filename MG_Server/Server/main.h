#pragma once
#include "stdafx.h"
#include "CAPObject.h"

DWORD WINAPI Calculate(LPVOID arg);
DWORD WINAPI ProcessClient(LPVOID arg);

std::unordered_map<INT, Player> players;
SC_PLAYER_PACKET send_players;

int global_id = 0;