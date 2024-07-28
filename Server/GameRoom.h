#pragma once
#include "stdafx.h"
#include "CAPObject.h"

enum GameRoomState { G_FREE, G_CREATE, G_INGAME };

class GameRoom
{
public:
	GameRoom();
	~GameRoom() = default;

	void InitGameRoom();

	BOOL SetPlayerId(int c_id);
	BOOL IsPlayerIn(int c_id);
	BOOL IsRoomEmpty();
	void DeletePlayerId(int c_id);

	BOOL SetStartGame();
	void SetCreateRoom();
	void SetFreeRoom();

	void SetHostName(std::string name) { m_host_name = name; }
	std::string GetHostName() { return m_host_name; }

	std::array<INT, MAX_CLIENT_ROOM> GetPlyId() { return m_player_ids; }

	GameRoomState GetState() { return m_state; }

	int m_all_life;
	std::mutex m_state_lock;

private:
	std::array<INT, MAX_CLIENT_ROOM> m_player_ids;
	GameRoomState m_state;
	std::string m_host_name;

	std::mutex m_player_lock;
	

};

