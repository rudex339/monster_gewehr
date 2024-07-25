#include "CAPObject.h"
#include <random>

#define LAND_Y 0.f
#define RUNAWAY_POINT 3

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<double> urd(50.0f, 100.0f);
std::uniform_real_distribution<double> minus(-1.0f, 1.0f);
std::uniform_int_distribution<int> rand_runaway_point(0, RUNAWAY_POINT-1);
std::uniform_int_distribution<int> random_0_to_100(0, 100);



CAPObject::CAPObject()
{
	m_position = { 0.f, 1000.f, 0.f };
	m_yaw = 0.f;
	m_id = -1;
}

void CAPObject::SetPostion(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
}

void CAPObject::RotateBoundingBox(float yaw)
{
	float radian = XMConvertToRadians(yaw);

	XMFLOAT4 q{};
	XMStoreFloat4(&q, XMQuaternionRotationRollPitchYaw(0.f, radian, 0.f));

	m_bounding_box.Orientation = q;
}

void CAPObject::RotateBoundingBox()
{
	float radian = XMConvertToRadians(m_yaw);

	XMFLOAT4 q{};
	XMStoreFloat4(&q, XMQuaternionRotationRollPitchYaw(0.f, radian, 0.f));

	m_bounding_box.Orientation = q;
}

//-------------------------------------------------------------------------------------------------
// Player
//-------------------------------------------------------------------------------------------------

Player::Player()
{
	m_socket = {};

	m_name = {};

	m_hp = 0;
	m_max_hp = 0;
	m_atk = 0;
	m_def = 0;

	m_ammo = 0;
	m_mag = 0;

	m_weapon = 0;
	m_armor = 0;
}

Player::Player(int id, SOCKET socket)
{
	m_id = id;
	m_socket = socket;
	m_hp = 100;
	m_max_hp = 100;
	m_atk = 50;
	m_def = 50;

	m_ammo = 0;
	m_mag = 0;

	m_weapon = 0;
	m_armor = 0;
	m_throw_wp = 0;

	m_remain_size = 0;
	m_state = S_STATE::LOG_IN;

	m_bounding_box.Center = m_position;
	m_bounding_box.Extents = XMFLOAT3(5.f, 10.f, 5.f);
	m_bounding_box.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);

	m_is_host = false;
	m_is_ready = false;

	// 나중에 데이터베이스 연동하면 삭제해라
	m_item_info[0] = 0;
	m_item_info[1] = 0;
	m_item_info[2] = 0;

	m_item_info[3] = 0;
	m_item_info[4] = 0;

	m_item_info[5] = 0;
	m_item_info[6] = 0;

	m_item_info[7] = 10;
	m_item_info[8] = 10;
	m_item_info[9] = 10;
}

void Player::PlayerInit()
{
	m_position = XMFLOAT3(2465.f, 0.f, 826.f);
	m_yaw = 0.f;
	m_hp = 100;
	m_max_hp = 100;

	m_ammo = 0;
	m_mag = 0;

	m_state = S_STATE::LOBBY;

	m_bounding_box.Center = m_position;
	m_bounding_box.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);

	m_is_host = false;
	m_is_ready = false;
}

void Player::SetWeapon(char weapon)
{
	m_weapon = weapon;
	switch (weapon)
	{
	case S_RIFLE:
		m_atk = 10;
		m_ammo = 30;
		m_range = 250.0f;
		break;
	case S_SHOT_GUN:
		m_atk = 25;
		m_ammo = 7;
		m_range = 100.0f;
		break;	
	case S_SNIPER:
		m_atk = 100;
		m_ammo = 5;
		m_range = 500.0f;
		break;
	}
}



int Player::RecvData()
{
	int retval = recv(m_socket, m_recv_buf + m_remain_size, BUF_SIZE - m_remain_size, 0);
	if (retval <= 0) {
		if (WSAGetLastError() == WSAETIMEDOUT)
			return 0;
		else
			return -1;
	}
	else {
		return retval;

	}
}

int Player::DoSend(void* p, size_t size)
{
	int retval = send(m_socket, (char*)p, size, 0);
	return retval;
}

//-------------------------------------------------------------------------------------------------
// Monster
//-------------------------------------------------------------------------------------------------

float Distance(XMFLOAT3 pos1, XMFLOAT3 pos2) {
	return sqrt((pos1.x - pos2.x) * (pos1.x - pos2.x) + (pos1.z - pos2.z) * (pos1.z - pos2.z));
}


Monster::Monster()
{
	for (int i = 0; i < MAX_CLIENT_ROOM; i++) {
		isUserArround[i] = false;
	}
	m_id = -2;

	turnning_speed = 0.3f;
	move_speed = 0.07f;
	fly_up_speed = 0.05f;

	m_yaw = 0.0f;

	m_max_hp = MONSTER_MAX_HP;
	m_hp = m_max_hp;
	m_runaway_hp = 900; // m_max_hp * 0.9;
	m_atk = 0;
	m_def = 0;

	/*m_position.x = 1014.f;
	m_position.y = 1024.f;
	m_position.z = 1429.f;*/

	m_position.x = 2200.f;
	m_position.y = 0.0f;
	m_position.z = 3100.f;

	m_animation = idle_ani;
	m_state = idle_state;

	m_bounding_box.Center = m_position;
	m_bounding_box.Center.y += 15.0f;
	m_bounding_box.Extents = XMFLOAT3(30.0f, 30.0f, 30.0f);
	float radian = XMConvertToRadians(m_yaw);
	XMFLOAT4 q{};
	XMStoreFloat4(&q, XMQuaternionRotationRollPitchYaw(0.f, radian, 0.f));
	m_bounding_box.Orientation = q;
}

float Monster::ElapsedTime()
{
	currentTime = clock();
	if (prevTime == 0) {
		elapsedTime = currentTime - clock();
	}
	else {
		elapsedTime = currentTime - prevTime;
	}
	prevTime = currentTime;
	return elapsedTime / 10.0f;
}

void Monster::Foward(float elapsedTime)
{
	m_position.x += m_front.x * move_speed * elapsedTime;
	m_position.z += m_front.z * move_speed * elapsedTime;
	m_bounding_box.Center.x = m_position.x;
	m_bounding_box.Center.z = m_position.z;

	updateFront();
}

void Monster::Back(float elapsedTime)
{
	m_position.x -= m_front.x * move_speed * elapsedTime;
	m_position.z -= m_front.z * move_speed * elapsedTime;
	m_bounding_box.Center.x = m_position.x;
	m_bounding_box.Center.z = m_position.z;

	updateFront();
}

void Monster::Left(float elapsedTime)
{
	m_yaw -= turnning_speed * elapsedTime;
	float radian = XMConvertToRadians(m_yaw);

	XMFLOAT4 q{};
	XMStoreFloat4(&q, XMQuaternionRotationRollPitchYaw(0.f, radian, 0.f));

	m_bounding_box.Orientation = q;

	updateFront();
}

void Monster::Right(float elapsedTime)
{
	m_yaw += turnning_speed * elapsedTime;
	float radian = XMConvertToRadians(m_yaw);

	XMFLOAT4 q{};
	XMStoreFloat4(&q, XMQuaternionRotationRollPitchYaw(0.f, radian, 0.f));

	m_bounding_box.Orientation = q;
	updateFront();

}

void Monster::Up(float elapsedTime)
{
	m_position.y += fly_up_speed * elapsedTime;
	m_bounding_box.Center.y = m_position.y + 5.0f;
}

void Monster::Down(float elapsedTime)
{
	m_position.y -= fly_up_speed * elapsedTime;
	m_bounding_box.Center.y = m_position.y + 5.0f;
}

void Monster::updateFront()
{
	XMVECTOR front;
	front = XMVectorSet(-sin(XMConvertToRadians(m_yaw)), 0.0f, -cos(XMConvertToRadians(m_yaw)), 0.0f);
	XMVECTOR normalizedFront = XMVector3Normalize(front);
	XMStoreFloat3(&m_front, -normalizedFront);
}

void Monster::InitMonster()
{
	for (int i = 0; i < MAX_CLIENT_ROOM; i++) {
		isUserArround[i] = false;
	}

	turnning_speed = 0.3f;
	move_speed = 0.08f;
	fly_up_speed = 0.05f;

	m_yaw = 0.0f;

	m_max_hp = MONSTER_MAX_HP;
	m_hp = m_max_hp;
	m_runaway_hp = 600; // m_max_hp * 0.9;
	m_atk = 0;
	m_def = 0;

	/*m_position.x = 1014.f;
	m_position.y = 1024.f;
	m_position.z = 1429.f;*/

	m_position.x = 2200.f;
	m_position.y = 0.f;
	m_position.z = 3100.f;

	m_animation = idle_ani;
	m_state = idle_state;

	m_bounding_box.Center = m_position;
	m_bounding_box.Center.y += 15.0f;
	m_bounding_box.Extents = XMFLOAT3(30.0f, 30.0f, 30.0f);
	float radian = XMConvertToRadians(m_yaw);
	XMFLOAT4 q{};
	XMStoreFloat4(&q, XMQuaternionRotationRollPitchYaw(0.f, radian, 0.f));
	m_bounding_box.Orientation = q;
}



void check_hp(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room)
{
	float hp = monster->GetHp();

	if (hp <= 0) {
		monster->SetState(die_state);
		build_bt(monster, players, room);
	}

	else if (hp <= monster->GetRAHp()) {
		monster->SetState(runaway_state);
		if (hp >= 300.0f) {
			monster->SetRAHp(hp * 0.5f);
		}
		build_bt(monster, players, room);
	}
}


//// Behaviors

// idle
Sequence idle_sequence;
	
// alert_state
Sequence alert_sequence;

// fight
Sequence fight_sequence;

// runaway
Sequence runaway_sequence;

// go home
Sequence goHome_sequence;

// die
Leaf	die_node;

// blind
Sequence blind_sequence;

// runaway points
XMFLOAT3 runaway_point01 = XMFLOAT3(2200.0f, LAND_Y, 3100.0f);
XMFLOAT3 runaway_point02 = XMFLOAT3(410.f, LAND_Y, 3055.f);
XMFLOAT3 runaway_point03 = XMFLOAT3(340.f, LAND_Y, 1450.f);

XMFLOAT3 runaway_point[RUNAWAY_POINT] = { runaway_point01, runaway_point02, runaway_point03 };


// die

int die(Monster* monster)
{
	monster->SetAnimation(die_ani);

	return BehaviorTree::RUNNING;
}


void build_bt(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room)
{

	// die
	die_node = Leaf("DIE", std::bind(die, monster));

	// root
	if (monster->GetState() == idle_state)
		monster->BuildBT(&idle_sequence);

	else if (monster->GetState() == alert_state)
		monster->BuildBT(&alert_sequence);

	else if (monster->GetState() == fight_state)
		monster->BuildBT(&fight_sequence);

	else if (monster->GetState() == runaway_state)
		monster->BuildBT(&runaway_sequence);

	else if (monster->GetState() == gohome_state)
		monster->BuildBT(&goHome_sequence);

	else if (monster->GetState() == die_state)
		monster->BuildBT(&die_node);

	else if (monster->GetState() == blind_state)
		monster->BuildBT(&blind_sequence);
}

void run_bt(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room)
{
	monster->ElapsedTime();
	check_hp(monster, players, room);
	monster->RunBT();
}