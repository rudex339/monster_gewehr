#pragma once
#include "stdafx.h"
#include "CAPObject.h"
#include "GameRoom.h"
#include "database.h"

// tcp¿« ¿‹¿ÁµÈ
//void ProcessClient(SOCKET sock);
//void BossThread();
//void PacketReassembly(int id, size_t recv_size);

enum EVENT_TYPE
{
	EV_HIT, EV_BITE, EV_TAIL, EV_DASH
};
struct TIMER_EVENT
{
	std::chrono::system_clock::time_point time_point;
	EVENT_TYPE type;
	int id;

	constexpr bool operator<(const TIMER_EVENT& rhs) const
	{
		return time_point > rhs.time_point;
	}
};

void WorkerThread();
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
void SendRegisterSucc(int id);
void SendRegisterFail(int id);

void TimerThread();
void ProcessEvent(TIMER_EVENT& event);

SOCKET listen_sock;
HANDLE iocp_handle;

std::unordered_map<INT, Player> players;
std::array<Monster, MAX_GAME_ROOM> souleaters;

std::array<GameRoom, MAX_GAME_ROOM> gamerooms;

concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;

DataBase database;

int global_id = 0;