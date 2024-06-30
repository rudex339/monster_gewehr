#pragma once
#include "stdafx.h"
#include "CAPObject.h"
#include "GameRoom.h"

void ProcessClient(SOCKET sock);
void BossThread();

void PacketReassembly(int id, size_t recv_size);
void ProcessPacket(int id, char* p);


void SendLoginInfo(int id);
void SendLoginFail(int id);
void SendStartGame(int id);
void SendPlayerMove(int id);
void SendAnimaition(int id);
void Disconnect(int id);
void SendHitPlayer(int id);
void SendEndGame(int id);
void SendShoot(int id);
void SendRoomList(int id);
void SendRoomCreate(int room_id);


std::unordered_map<INT, Player> players;
std::array<Monster, MAX_GAME_ROOM> souleaters;

std::array<GameRoom, MAX_GAME_ROOM> gamerooms;

int global_id = 0;