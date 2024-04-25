#include "CAPObject.h"
#include <random>

#define LAND_Y 1024.f
#define RUNAWAY_POINT 3

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<double> urd(-80.0, 80.0);
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

	m_remain_size = 0;
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
enum MonsterAnimation { idle_ani, 
	growl_ani, walk_ani, 
	flyup_ani, flying_ani, landing_ani, 
	bite_ani, dash_ani, 
	hit_ani, die_ani, 
	sleep_ani };

Monster::Monster()
{
	for (int i = 0; i < MAX_CLIENT_ROOM; i++) {
		isUserArround[i] = false;
	}
	m_id = 5;

	turnning_speed = 0.2f;
	move_speed = 0.1f;
	fly_up_speed = 0.05f;

	m_yaw = 0.0f;

	m_max_hp = 1000;
	m_hp = m_max_hp;
	m_runaway_hp = 990; // m_max_hp * 0.9;
	m_atk = 0;
	m_def = 0;

	/*m_position.x = 1014.f;
	m_position.y = 1024.f;
	m_position.z = 1429.f;*/

	m_position.x = 2289.f;
	m_position.y = 1024.f;
	m_position.z = 895.f;

	m_animation = idle_ani;
	m_state = idle_state;

	m_bounding_box.Center = m_position;
	m_bounding_box.Center.y += 20.0f;
	m_bounding_box.Extents = XMFLOAT3(20.0f, 20.0f, 20.0f);
	m_bounding_box.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
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
	m_yaw += turnning_speed * elapsedTime;
	float radian = XMConvertToRadians(m_yaw);

	XMFLOAT4 q{};
	XMStoreFloat4(&q, XMQuaternionRotationRollPitchYaw(0.f, radian, 0.f));

	m_bounding_box.Orientation = q;
	updateFront();
}

void Monster::Right(float elapsedTime)
{
	m_yaw -= turnning_speed * elapsedTime;
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


//// Behaviors

// idle
#define		FIND_USER_DISTANCE 300.0f
#define		ATTENT_TIME 5000.0f
#define		WAIT_PATROL 3000.0f

//Selector	idle_selector;
Sequence	idle_sequence;
	Sequence	attentive_sequence;
		Leaf		find_near_user_node;
		Leaf		idle_to_fight_node;
	//Leaf		choose_random_action_node;
	Sequence	patrol_sequence;
		Leaf		find_random_pos_node;
		Leaf		move_to_node;
		Leaf		patrol_cool_time_node;

// fight
Sequence	fight_sequence;
	/*Sequence	fight_to_runaway_sequence;
		Leaf	check_hp_node;
		Leaf	fight_to_runaway_node;*/
	Leaf		search_target_node;
	Selector	fight_to_idle_selector;
		Leaf		choose_target_node;
		Leaf		calm_down_node;
		Leaf		fight_to_idle_node;
	Leaf		choose_attack_type_node;
	Sequence	attack_sequence;
		Sequence	move_to_target_sequence;
			Selector	move_or_not_selector;
				Leaf	move_to_target_timeout_node;
				Leaf	move_to_target_node;
			Sequence	dash_sequence;
				Leaf		set_target_node;
				Leaf		charging_dash_node;
				Leaf		dash_attack_node;
				Leaf		dash_cool_time_node;
			/*Leaf		attack_node;
			Leaf		attack_cool_time_node;*/


// runaway
Sequence	runaway_sequence;
	Leaf		set_runaway_location_node;
	Leaf		flyup_node;
	Leaf		runaway_node;
	Leaf		landing_node;
	Leaf		wait_node;


// runaway points
XMFLOAT3 runaway_point01 = XMFLOAT3(2256.0f, LAND_Y, 890.0f);
XMFLOAT3 runaway_point02 = XMFLOAT3(2809.f, LAND_Y, 3329.f);
XMFLOAT3 runaway_point03 = XMFLOAT3(1754.f, LAND_Y, 2795.f);

XMFLOAT3 runaway_point[RUNAWAY_POINT] = { runaway_point01, runaway_point02, runaway_point03 };


//// idle

int find_near_user(Monster* monster, std::unordered_map<INT, Player>* players, float cooltime)
{
	bool isTargetExist = false;

	for (int i = 0; i < MAX_CLIENT_ROOM; i++) {
		auto playerIter = players->find(i);
		if (playerIter != players->end()) {
			if (playerIter->second.GetID() != -1) {
				if (Distance(monster->GetPosition(), playerIter->second.GetPosition()) <= FIND_USER_DISTANCE) {
					isTargetExist = true;
				}
			}
		}
	}

	if (isTargetExist) {
		monster->SetWaitTime(monster->GetWaitTime() + monster->GetElapsedTime());

		if (monster->GetWaitTime() < cooltime)
			return BehaviorTree::RUNNING;

		else {
			monster->SetWaitTime(0.0f);
			return BehaviorTree::SUCCESS;
		}
	}

	return BehaviorTree::FAIL;
}

int to_fight(Monster* monster, std::unordered_map<INT, Player>* players)
{
	monster->SetState(fight_state);
	build_bt(monster, players);


	return BehaviorTree::SUCCESS;
}

//fight
int search_target(Monster* monster, std::unordered_map<INT, Player>* players)
{
	bool isTargetExist = false;

	for (int i = 0; i < MAX_CLIENT; i++) {
		auto playerIter = players->find(i);
		if (playerIter != players->end()) {
			monster->SetUserArround(i, false);
			if (playerIter->second.GetID() != -1) {
				if (Distance(monster->GetPosition(), playerIter->second.GetPosition()) <= FIND_USER_DISTANCE) {
					isTargetExist = true;
					monster->SetUserArround(i, true);
				}
			}
		}
	}

	if (isTargetExist) {
		return BehaviorTree::SUCCESS;
	}

	return BehaviorTree::FAIL;
}

int calm_down(Monster* monster, float cooltime)
{
	monster->SetWaitTime(monster->GetWaitTime() + monster->GetElapsedTime());

	if (monster->GetWaitTime() < cooltime)
		return BehaviorTree::SUCCESS;

	else {
		monster->SetWaitTime(0.0f);
		return BehaviorTree::FAIL;
	}
}

int time_out(Monster* monster, float cooltime)
{
	monster->SetWaitTime(monster->GetWaitTime() + monster->GetElapsedTime());

	if (monster->GetWaitTime() < cooltime)
		return BehaviorTree::FAIL;

	else {
		monster->SetWaitTime(0.0f);
		return BehaviorTree::SUCCESS;
	}
}

int to_idle(Monster* monster, std::unordered_map<INT, Player>* players)
{
	monster->SetState(idle_state);
	build_bt(monster, players);

	return BehaviorTree::SUCCESS;
}

int choose_target(Monster* monster, std::unordered_map<INT, Player>* players)
{
	
	bool targetcheck[MAX_CLIENT_ROOM];
	bool isTargetExist = false;
	for (int i = 0; i < MAX_CLIENT_ROOM; i++) {
		if (monster->GetUserArround(i)) {
			isTargetExist = true;
			if (random_0_to_100(gen) < 50) {
				monster->SetTarget(&players->find(i)->second);
				return BehaviorTree::SUCCESS;
			}
		}
	}

	if (isTargetExist) {
		return BehaviorTree::RUNNING;
	}

	return BehaviorTree::FAIL;
}

int choose_attack_type(Monster* monster)
{
	return BehaviorTree::SUCCESS;
}

int set_target_pos(Monster* monster)
{
	monster->SetTargetPos(monster->GetTarget()->GetPosition());
	
	std::cout << "보스 위치 : " << monster->GetPosition().x << " " << monster->GetPosition().z << std::endl;
	std::cout << "타켓 위치 : " << monster->GetTargetPos().x << " " << monster->GetTargetPos().z << std::endl;
	
	return BehaviorTree::SUCCESS;
}

int charging_dash(Monster* monster)
{
	//monster->SetAnimation(dash_ani);
	// 대쉬 준비하는 애니메이션
	monster->SetWaitTime(monster->GetWaitTime() + monster->GetElapsedTime());
	if (monster->GetWaitTime() < 1500.0f) {
		// 위 시간을 대쉬 준비하는 애니메이션 시간으로 변경
		return BehaviorTree::RUNNING;
	}
	monster->SetWaitTime(0.0f);
	return BehaviorTree::SUCCESS;
}

int dash_attack(Monster* monster)
{
	monster->SetAnimation(dash_ani);

	monster->SetWaitTime(monster->GetWaitTime() + monster->GetElapsedTime());
	if ( monster->GetWaitTime() < 500.0f) {	
		monster->Foward(monster->GetElapsedTime() * 3.0f);
		return BehaviorTree::RUNNING;
	}
	monster->SetWaitTime(0.0f);
	return BehaviorTree::SUCCESS;
}

int bite_attack(Monster* monster)
{
	monster->SetAnimation(bite_ani);
	monster->SetWaitTime(monster->GetWaitTime() + monster->GetElapsedTime());
	if (monster->GetWaitTime() < 300.0f) {
		// 위 시간을 깨무는 애니메이션 시간으로 조정
		return BehaviorTree::RUNNING;
	}
	monster->SetWaitTime(0.0f);
	return BehaviorTree::SUCCESS;
}

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

int move_to_target(Monster* monster, std::unordered_map<INT, Player>* players)
{
	auto playerIter = players->find(monster->GetTarget()->GetID());
	monster->SetTargetPos(playerIter->second.GetPosition());
	
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

void build_bt(Monster* monster, std::unordered_map<INT, Player>* players)
{
	set_target_node = Leaf("Set Target Position", std::bind(set_target_pos, monster));
	charging_dash_node = Leaf("Charging Dash...", std::bind(charging_dash, monster));
	dash_attack_node = Leaf("DASH", std::bind(dash_attack, monster));
	dash_cool_time_node = Leaf("dash cooltime", std::bind(wait_cool_time, monster, 2000.0f)); // 대쉬 쿨타임으로 변경

	dash_sequence = Sequence("Dash Sequence", { &set_target_node, &charging_dash_node, &dash_attack_node, &dash_cool_time_node });
	
	move_to_target_timeout_node = Leaf("Move Timeout Check", std::bind(time_out, monster, 3000.0f)); // 유저를 추적하는 시간
	move_to_target_node = Leaf("Move To Target", std::bind(move_to_target, monster, players));
	move_or_not_selector = Selector("Move or Dash Selector", {&move_to_target_timeout_node, &move_to_target_node});
	move_to_target_sequence = Sequence("Move to Target for Attack Sequence", {&move_or_not_selector, &dash_sequence});

	search_target_node = Leaf("Search Near User", std::bind(search_target, monster, players));
	calm_down_node = Leaf("Calm Down", std::bind(calm_down, monster, 2000.0f));
	fight_to_idle_node = Leaf("Change State(fight->idle)", std::bind(to_idle, monster, players));

	fight_to_idle_selector = Selector("No user here", { &search_target_node, &calm_down_node, &fight_to_idle_node });

	choose_target_node = Leaf("Choose Target", std::bind(choose_target, monster, players));

	attack_sequence = Sequence("Ready to Attack", { &fight_to_idle_selector, &choose_target_node, &move_to_target_sequence });


	// patrol
	find_random_pos_node = Leaf("Find Random Position", std::bind(find_random_pos, monster));
	move_to_node = Leaf("Move", std::bind(move_to, monster));
	patrol_cool_time_node = Leaf("Wait Next Patrol", std::bind(wait_cool_time, monster, WAIT_PATROL));

	patrol_sequence = Sequence("Patrol", { &find_random_pos_node, &move_to_node, &patrol_cool_time_node });


	// runaway
	set_runaway_location_node = Leaf("Set RunAway Point", std::bind(set_runaway_location, monster));
	flyup_node = Leaf("Fly UP", std::bind(fly_up, monster));
	runaway_node = Leaf("Fly to Point", std::bind(move_to, monster));
	landing_node = Leaf("Landing", std::bind(landing, monster));

	runaway_sequence = Sequence("RunAway", { &set_runaway_location_node, &flyup_node, &runaway_node, &landing_node });

	// root
	monster->BuildBT(&attack_sequence);
}

void run_bt(Monster* monster)
{
	monster->ElapsedTime();
	monster->RunBT();
}