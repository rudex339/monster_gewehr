#include "CAPObject.h"
#include <random>

#define LAND_Y 1024.f
#define RUNAWAY_POINT 3

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<double> urd(-80.0, 80.0);
std::uniform_int_distribution<int> rand_runaway_point(0, RUNAWAY_POINT-1);


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

	m_wepon = 0;
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

	m_wepon = 0;
	m_armor = 0;

	m_state = S_STATE::LOBBY;
}


void Player::SetAtkByWeapon(char weapon)
{
	switch (weapon)
	{
	case SHOT_GUN:
		m_atk = 100;
		break;
	case ASSAULT_RIFLE:
		m_atk = 30;
		break;
	case SNIPER:
		m_atk = 100;
		break;	
	}
}



int Player::RecvData(char* p)
{
	int retval = recv(m_socket, m_recv_buf + m_remain_size, BUF_SIZE - m_remain_size, 0);
	if (retval <= 0) {
		return 0;
	}
	else {
		p = m_recv_buf;
		return retval;
		
	}
}

void Player::Process_Packet(char* p)
{
	int retval;
	switch (p[1])
	{
	case CS_PACKET_LOGIN: {
		CS_LOGIN_PACKET* packet = reinterpret_cast<CS_LOGIN_PACKET*>(p);
		m_name = packet->name;
		m_wepon = packet->weapon;
		m_state = S_STATE::IN_GAME;
	}
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


enum MonsterState { idle_state, fight_state, runaway_state };
enum MonsterAnimation { idle_ani, growl_ani, walk_ani, flyup_ani, flying_ani, landing_ani, bite_ani, dash_ani, hit_ani, die_ani, sleep_ani };

Monster::Monster()
{
	for (int i = 0; i < MAX_CLIENT_ROOM; i++) {
		isUserArrround[i] = false;
	}

	turnning_speed = 0.1f;
	move_speed = 0.1f;
	fly_up_speed = 0.05f;

	m_yaw = 0.0f;

	m_max_hp = 1000;
	m_hp = m_max_hp;
	m_runaway_hp = 990; // m_max_hp * 0.9;
	m_atk = 0;
	m_def = 0;

	m_position.x = 1014.f;
	m_position.y = 1024.f;
	m_position.z = 1429.f;

	/*m_position.x = 2289.f;
	m_position.y = 1024.f;
	m_position.z = 895.f;*/

	m_animation = idle_ani;
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

void Monster::Foward(float elapsedTimer)
{
	m_position.x += m_front.x * move_speed * elapsedTime;
	m_position.z += m_front.z * move_speed * elapsedTime;
	

	updateFront();
}

void Monster::Back(float elapsedTimer)
{
	m_position.x -= m_front.x * move_speed * elapsedTime;
	m_position.z -= m_front.z * move_speed * elapsedTime;

	updateFront();
}

void Monster::Left(float elapsedTimer)
{
	m_yaw += turnning_speed * elapsedTime;
	std::cout << m_position.x << std::endl;

	updateFront();
}

void Monster::Right(float elapsedTimer)
{
	m_yaw -= turnning_speed * elapsedTime;

	updateFront();
}

void Monster::Up(float elapsedTimer)
{
	m_position.y += fly_up_speed * elapsedTime;
}

void Monster::Down(float elapsedTimer)
{
	m_position.y -= fly_up_speed * elapsedTime;
}

void Monster::updateFront()
{
	XMVECTOR front;
	front = XMVectorSet(-sin(XMConvertToRadians(m_yaw)), 0.0f, -cos(XMConvertToRadians(m_yaw)), 0.0f);
	XMVECTOR normalizedFront = XMVector3Normalize(front);
	XMStoreFloat3(&m_front, -normalizedFront);
}


// test behavior (just move)
Leaf		find_random_pos_node;
Leaf		move_to_node;
Leaf		patrol_cool_time_node;
#define		WAIT_PATROL 1000.0f

Sequence	patrol_sequence;

Leaf		set_runaway_location_node;
Leaf		flyup_node;
Leaf		runaway_node;
Leaf		landing_node;
Leaf		wait_node;

Sequence	runaway_sequence;

XMFLOAT3 runaway_point01 = XMFLOAT3(2256.0f, LAND_Y, 890.0f);
XMFLOAT3 runaway_point02 = XMFLOAT3(2809.f, LAND_Y, 3329.f);
XMFLOAT3 runaway_point03 = XMFLOAT3(1754.f, LAND_Y, 2795.f);

XMFLOAT3 runaway_point[RUNAWAY_POINT] = { runaway_point01, runaway_point02, runaway_point03 };



// patrol

int find_random_pos(Monster* monster)
{
	XMFLOAT3 randompos;
	randompos.x = monster->GetPosition().x + urd(gen);
	randompos.y = monster->GetPosition().y;
	randompos.z = monster->GetPosition().z + urd(gen);

	monster->SetTargetPos(randompos);

	return BehaviorTree::SUCCESS;
}

int move_to(Monster* monster)
{
	monster->SetAnimation(walk_ani);

	if (monster->GetPosition().y >= LAND_Y + 100.f) {
		monster->SetAnimation(flying_ani);
	}

	if (Distance(monster->GetPosition(), monster->GetTargetPos()) <= 1.0f) return BehaviorTree::SUCCESS;

	XMFLOAT3 target = monster->GetTargetPos();
	XMFLOAT3 pos = monster->GetPosition();

	XMVECTOR targetPosVec = XMLoadFloat3(&target);
	XMVECTOR currentPosVec = XMLoadFloat3(&pos);

	XMVECTOR frontVec = XMVector3Normalize(XMVectorSubtract(targetPosVec, currentPosVec)); // 봐야하는 방향

	XMVECTOR upVec = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMFLOAT3 front = monster->GetFront();
	XMVECTOR FrontVec = XMLoadFloat3(&front); // 몬스터가 현재 보는 방향

	bool isleft = XMVectorGetX(XMVector3Dot(upVec, XMVector3Cross(FrontVec, frontVec))) > 0;

	float angle;
	XMVECTOR angleVec = XMVector3AngleBetweenNormals(FrontVec, frontVec);
	XMStoreFloat(&angle, angleVec);


	if (angle > 0.1f) {
		if (isleft) {
			monster->Left(monster->GetElapsedTime());
		}
		else {
			monster->Right(monster->GetElapsedTime());
		}
		monster->Foward(monster->GetElapsedTime());
		return BehaviorTree::RUNNING;
	}

	else {
		monster->Foward(monster->GetElapsedTime());
		return BehaviorTree::RUNNING;
	}

	return BehaviorTree::SUCCESS;
}

int wait_cool_time(Monster* monster, FLOAT cooltime)
{
	monster->SetAnimation(idle_ani);
	monster->SetWaitTime(monster->GetWaitTime() + monster->GetElapsedTime());

	if (monster->GetWaitTime() < cooltime)
		return BehaviorTree::RUNNING;

	else {
		monster->SetWaitTime(0.0f);
		return BehaviorTree::SUCCESS;
	}
}


// run away
int set_runaway_location(Monster* monster)
{
	int point = 0;
	while (1) {
		point = rand_runaway_point(gen);
		if (Distance(monster->GetPosition(), runaway_point[point]) > 300.f) {
			monster->SetTargetPos(runaway_point[point]);
			break;
		}
	}

	return BehaviorTree::SUCCESS;
}

int fly_up(Monster* monster)
{
	monster->SetAnimation(flyup_ani);

	if (monster->GetPosition().y <= 1200.f) {

		monster->Up(monster->GetElapsedTime());
		return BehaviorTree::RUNNING;
	}

	return BehaviorTree::SUCCESS;
}

int landing(Monster* monster)
{
	monster->SetAnimation(landing_ani);

	if (monster->GetPosition().y >= LAND_Y) {
		monster->Down(monster->GetElapsedTime());
		return BehaviorTree::RUNNING;
	}

	XMFLOAT3 pos = monster->GetPosition();
	pos.y = LAND_Y;
	monster->SetPostion(pos);

	return BehaviorTree::SUCCESS;
}

void build_bt(Monster* monster)
{
	find_random_pos_node = Leaf("Find Random Position", std::bind(find_random_pos, monster));
	move_to_node = Leaf("Move", std::bind(move_to, monster));
	patrol_cool_time_node = Leaf("Wait Next Patrol", std::bind(wait_cool_time, monster, WAIT_PATROL));

	patrol_sequence = Sequence("Patrol", { &find_random_pos_node, &move_to_node, &patrol_cool_time_node });


	set_runaway_location_node = Leaf("Set RunAway Point", std::bind(set_runaway_location, monster));
	flyup_node = Leaf("Fly UP", std::bind(fly_up, monster));
	runaway_node = Leaf("Fly to Point", std::bind(move_to, monster));
	landing_node = Leaf("Landing", std::bind(landing, monster));

	runaway_sequence = Sequence("RunAway", { &set_runaway_location_node, &flyup_node, &runaway_node, &landing_node });

	monster->BuildBT(&runaway_sequence);
}

void run_bt(Monster* monster)
{
	monster->ElapsedTime();
	monster->RunBT();
}