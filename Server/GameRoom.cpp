#include "GameRoom.h"

GameRoom::GameRoom()
{
	for (auto& id : m_player_ids) {
		id = -1;
	}
	m_state = GameRoomState::G_FREE;
}

BOOL GameRoom::SetPlayerId(int c_id)
{
	m_state_lock.lock();
	if (m_state == G_END) {
		m_state_lock.unlock();
		return false;
	}
	m_state_lock.unlock();

	for (auto& id : m_player_ids) {
		if (id == -1) {
			id = c_id;
			return true;
		}
	}
	return false;
}

BOOL GameRoom::IsPlayerIn(int c_id)
{
	for (auto& id : m_player_ids) {
		if (id == c_id) {
			return true;
		}
	}
	return false;
}

BOOL GameRoom::IsPlayerEmpty()
{
	return false;
}

void GameRoom::DeletePlayerId(int c_id)
{
	for (auto& id : m_player_ids) {
		if (id == c_id) {
			id = -1;
			return;
		}
	}
}

BOOL GameRoom::SetStartGame()
{
	m_state_lock.lock();
	if (m_state == G_FREE) {
		m_state = G_INGAME;
		m_state_lock.unlock();
		return true;
	}
	/*else if (m_state == G_INGAME) {
		m_state_lock.unlock();
		return true;
	}*/
	m_state_lock.unlock();

	return false;
}

void GameRoom::SetEndGame()
{
	m_state_lock.lock();
	m_state = G_END;
	m_state_lock.unlock();

	for (auto& id : m_player_ids) {
		id = -1;
	}
}

void GameRoom::SetFreeRoom()
{
	m_state_lock.lock();
	m_state = G_FREE;
	m_state_lock.unlock();

}
