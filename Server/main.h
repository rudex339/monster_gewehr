#pragma once
#include "stdafx.h"
#include "CAPObject.h"
#include "GameRoom.h"
#include "database.h"

void ProcessClient(SOCKET sock);
void BossThread();

void PacketReassembly(int id, size_t recv_size);
void ProcessPacket(int id, char* p);


void SendLoginInfo(int id);
void SendLoginFail(int id);
void SendStartGame(int id);
void SendPlayerMove(int id);
void SendAnimaition(int id);
void SendShot(int id);
void Disconnect(int id);
void SendHitPlayer(int id);
void SendEndGame(int id, bool clear);
void SendRoomList(int id);
void SendRoomCreate(int ply_id, int room_num);
void SendRoomSelect(int id, short room_num);
void SendBreakRoom(int id);
void SendDeleteRoom(short room_num);
void SendRoomJoin(int id);
void SendRoomQuit(int id);
void SendItemInfo(int id);


std::unordered_map<INT, Player> players;
std::array<Monster, MAX_GAME_ROOM> souleaters;

std::array<GameRoom, MAX_GAME_ROOM> gamerooms;

DataBase database;

int global_id = 0;