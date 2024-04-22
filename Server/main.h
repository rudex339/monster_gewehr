#pragma once
#include "stdafx.h"
#include "CAPObject.h"

void ProcessClient(SOCKET sock);

std::unordered_map<INT, Player> players;
Monster souleater;

SC_OBJECT_PACKET send_players;

int global_id = 0;