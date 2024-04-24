#pragma once
#include "stdafx.h"
#include "CAPObject.h"

void ProcessClient(SOCKET sock);
void PacketReassembly(int id, char* r_buf, size_t recv_size);
void ProcessPacket(int id, char* p);
void SendLoginInfo(int id);
void SendPlayerMove(int id);
void SendAnimaition(int id);

std::unordered_map<INT, Player> players;
Monster souleater;


int global_id = 0;