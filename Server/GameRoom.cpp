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
		std::cout << "�̴ַ� id : " << id << std::endl;
		id = -1;
	}
	SetFreeRoom();
}

BOOL GameRoom::SetPlayerId(int c_id)
{
	for (auto& id : m_player_ids) {
		std::cout << "�·� �÷��̾� id : " << id << std::endl;
		if (id == -1) {
			id = c_id;
			std::cout << "�·� �Ϸ� id : " << id << std::endl;
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
	std::cout << "�ΰ��� ���� �غ�" << std::endl;
	if (m_state == G_CREATE) {
		m_state = G_INGAME;
		return true;
	}
	std::cout << "�ΰ��� ���� �Ϸ�" << std::endl;
	return false;
}

void GameRoom::SetCreateRoom()
{
	/*std::cout << "�游��� ��" << std::endl;
	{
		std::lock_guard<std::mutex> lock{ m_state_lock };
		m_state = G_CREATE;
	}
	std::cout << "�游��� ���" << std::endl;*/
	std::cout << "create �غ�" << std::endl;
	m_state = G_CREATE;
	std::cout << "create �Ϸ�" << std::endl;
	for (auto& id : m_player_ids) {
		id = -1;
	}
}


void GameRoom::SetFreeRoom()
{
	/*std::cout << "���� FREE ��" << std::endl;
	{
		std::lock_guard<std::mutex> lock{ m_state_lock };
		m_state = G_FREE;
	}
	std::cout << "���� FREE ���111" << std::endl;*/

	std::cout << "free �غ�" << std::endl;
	m_state = G_FREE;
	std::cout << "free �Ϸ�" << std::endl;
	
}
