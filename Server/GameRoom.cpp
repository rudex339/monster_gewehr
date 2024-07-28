#include "GameRoom.h"

GameRoom::GameRoom()
{
	m_all_life = MAX_LIFE;
	for (auto& id : m_player_ids) {
		id = -1;
	}
	m_state = GameRoomState::G_FREE;
}

void GameRoom::InitGameRoom()
{
	m_all_life = MAX_LIFE;
	for (auto& id : m_player_ids) {
		std::cout << "이닛룸 id : " << id << std::endl;
		id = -1;
	}
	SetFreeRoom();
}

BOOL GameRoom::SetPlayerId(int c_id)
{
	for (auto& id : m_player_ids) {
		std::cout << "셋룸 플레이어 id : " << id << std::endl;
		if (id == -1) {
			id = c_id;
			std::cout << "셋룸 완료 id : " << id << std::endl;
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

BOOL GameRoom::IsRoomEmpty()
{
	for (auto& id : m_player_ids) {
		if (id != -1) {
			return false;
		}
	}
	return true;
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
	/*{
		std::lock_guard<std::mutex> lock{ m_state_lock };
		if (m_state == G_CREATE) {
			m_state = G_INGAME;
			return true;
		}
	}*/
	std::cout << "인게임 상태 준비" << std::endl;
	if (m_state == G_CREATE) {
		m_state = G_INGAME;
		return true;
	}
	std::cout << "인게임 상태 완료" << std::endl;
	return false;
}

void GameRoom::SetCreateRoom()
{
	/*std::cout << "방만들기 락" << std::endl;
	{
		std::lock_guard<std::mutex> lock{ m_state_lock };
		m_state = G_CREATE;
	}
	std::cout << "방만들기 언락" << std::endl;*/
	std::cout << "create 준비" << std::endl;
	m_state = G_CREATE;
	std::cout << "create 완료" << std::endl;
	for (auto& id : m_player_ids) {
		id = -1;
	}
}


void GameRoom::SetFreeRoom()
{
	/*std::cout << "게임 FREE 락" << std::endl;
	{
		std::lock_guard<std::mutex> lock{ m_state_lock };
		m_state = G_FREE;
	}
	std::cout << "게임 FREE 언락111" << std::endl;*/

	std::cout << "free 준비" << std::endl;
	m_state = G_FREE;
	std::cout << "free 완료" << std::endl;
	
}
