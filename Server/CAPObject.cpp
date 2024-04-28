#include "CAPObject.h"
#include <random>

#define LAND_Y 1024.f
#define RUNAWAY_POINT 3

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<double> urd(-30.0, 30.0);
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
	m_state = S_STATE::LOG_IN;

	m_bounding_box.Center = m_position;
	m_bounding_box.Extents = XMFLOAT3(5.f, 10.f, 5.f);
	m_bounding_box.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
}

void Player::PlayerInit()
{
	m_position = XMFLOAT3(0.f, 0.f, 0.f);
	m_hp = 100;
	m_max_hp = 100;

	m_ammo = 0;
	m_mag = 0;

	m_state = S_STATE::LOBBY;

	m_bounding_box.Center = m_position;
	m_bounding_box.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
}

void Player::SetAtkByWeapon(char weapon)
{
	switch (weapon)
	{
	case SHOT_GUN:
		m_atk = 25;
		m_ammo = 7;
		m_mag = 56;
		break;
	case ASSAULT_RIFLE:
		m_atk = 10;
		m_ammo = 30;
		m_mag = 300;
		break;
	case SNIPER:
		m_atk = 100;
		m_ammo = 5;
		m_mag = 40;
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
	move_speed = 0.08f;
	fly_up_speed = 0.05f;

	m_yaw = 0.0f;

	m_max_hp = 10;
	m_hp = m_max_hp;
	m_runaway_hp = -600; // m_max_hp * 0.9;
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
	m_bounding_box.Center.y += 10.0f;
	m_bounding_box.Extents = XMFLOAT3(18.0f, 15.0f, 20.0f);
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

	m_max_hp = 10;
	m_hp = m_max_hp;
	m_runaway_hp = -600; // m_max_hp * 0.9;
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
	m_bounding_box.Center.y += 10.0f;
	m_bounding_box.Extents = XMFLOAT3(18.0f, 15.0f, 20.0f);
	float radian = XMConvertToRadians(m_yaw);
	XMFLOAT4 q{};
	XMStoreFloat4(&q, XMQuaternionRotationRollPitchYaw(0.f, radian, 0.f));
	m_bounding_box.Orientation = q;
}

void check_hp(Monster* monster, std::unordered_map<INT, Player>* players)
{
	float hp = monster->GetHp();

	if (hp <= 0) {
		monster->SetState(die_state);
		build_bt(monster, players);
	}

	else if (hp <= monster->GetRAHp()) {
		monster->SetState(runaway_state);
		if (hp >= 300.0f) {
			monster->SetRAHp(hp * 0.5f);
		}
		build_bt(monster, players);
	}
}


//// Behaviors

// idle
Selector	idle_selector;
	Sequence	idle_sequence;
		Sequence	attentive_sequence;
			Leaf		find_near_user_node;
			Leaf		idle_to_fight_node;
	Leaf		choose_random_action_node;
	Selector	choose_action_selector;
		Selector	go_to_home_selector;
			Leaf		check_far_node;
			Leaf		go_to_home_node;
			//Leaf		wait
		Leaf		growling_node;
		Leaf		do_nothing_node;
		Sequence	patrol_sequence;
			Leaf		find_random_pos_node;
			Leaf		move_to_node;
			Leaf		patrol_cool_time_node;
	

// fight
Sequence fight_sequence;
	Selector	search_user_selector;
		Leaf		search_user_node;
		Leaf		search_time_out_node;
		Leaf		fight_to_idle_node;
	Selector	choose_user_selector;
		Leaf		choose_user_time_out_node;
		Leaf		choose_user_node;
	Selector	attack_selector;
		Sequence	dash_or_move_sequence;
			Leaf	dash_time_out_node;
			Sequence	dash_sequence;
				Leaf        set_target_node;
				Leaf        charging_dash_node;
				Leaf        dash_attack_node;
				Leaf        dash_cool_time_node;
		Sequence	move_to_bite_sequence;
			Leaf		move_to_user_node;
			Sequence	bite_attack_sequence;
				Leaf		bite_attack_node;
				Leaf		bite_cooltime_node;


// runaway
Sequence	runaway_sequence;
	Leaf		set_runaway_location_node;
	Leaf		flyup_node;
	Leaf		runaway_node;
	Leaf		landing_node;
	Leaf		runaway_to_idle_node;

// die
Leaf	die_node;

// runaway points
XMFLOAT3 runaway_point01 = XMFLOAT3(2256.0f, LAND_Y, 890.0f);
XMFLOAT3 runaway_point02 = XMFLOAT3(2809.f, LAND_Y, 3329.f);
XMFLOAT3 runaway_point03 = XMFLOAT3(1754.f, LAND_Y, 2795.f);

XMFLOAT3 runaway_point[RUNAWAY_POINT] = { runaway_point01, runaway_point02, runaway_point03 };

static float bite_attack_time = 0.0f;
static float dash_attack_time = 0.0f;
static float charging_dash_time = 0.0f;
static float do_nothing_time = 0.0f;

static float time_out_search = 0.0f;
static float time_out_dash = 0.0f;
static float time_out_choose = 100000.0f;

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

int time_out(Monster* monster, float& timeout, float cooltime)
{
	timeout += monster->GetElapsedTime();
	if (timeout < cooltime)
		return BehaviorTree::SUCCESS;

	else {
		timeout = 0.0f;
		return BehaviorTree::FAIL;
	}
}

int time_out_seq(Monster* monster, float& timeout, float cooltime)
{
	timeout += monster->GetElapsedTime();

	if (timeout < cooltime)
		return BehaviorTree::FAIL;

	else {
		timeout = 0.0f;
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
		targetcheck[i] = false;
		if (monster->GetUserArround(i)) {
			isTargetExist = true;
			targetcheck[i] = true;
		}
	}

	if (isTargetExist) {
		while (1) {
			int target = random_0_to_100(gen) % 4;
			if (targetcheck[target]) {
				monster->SetTarget(&players->find(target)->second);
				return BehaviorTree::SUCCESS;
			}
		}
	}

	return BehaviorTree::FAIL;
}

int charging_dash(Monster* monster)
{
	//monster->SetAnimation(dash_ani);
	// 대쉬 준비하는 애니메이션

	charging_dash_time += monster->GetElapsedTime();
	
	std::cout << charging_dash_time << std::endl;

	if (charging_dash_time < DASH_CHARGING_TIME) {
		return BehaviorTree::RUNNING;
	}
	charging_dash_time = 0.0f;
	return BehaviorTree::SUCCESS;
}

int dash_attack(Monster* monster)
{
	monster->SetAnimation(dash_ani);

	dash_attack_time += monster->GetElapsedTime();
	if (dash_attack_time < DASH_TIME) {
		monster->Foward(monster->GetElapsedTime() * DASH_SPEED);
		return BehaviorTree::RUNNING;
	}
	dash_attack_time = 0.0f;
	return BehaviorTree::SUCCESS;
}

int bite_attack(Monster* monster)
{
	monster->SetAnimation(bite_ani);
	bite_attack_time += monster->GetElapsedTime();
	if (bite_attack_time < BITE_ANIMATION) {
		return BehaviorTree::RUNNING;
	}
	bite_attack_time = 0.0f;
	charging_dash_time = 0.0f;
	return BehaviorTree::SUCCESS;
}

//// idle
int attent(Monster* monster, std::unordered_map<INT, Player>* players, float cooltime)
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
		monster->SetWaitTime(monster->GetWaitTime() + monster->GetElapsedTime());
		if (monster->GetWaitTime() < cooltime) {
			return BehaviorTree::RUNNING;
		}
		else {
			monster->SetWaitTime(0.0f);
			return BehaviorTree::SUCCESS;
		}
	}

	return BehaviorTree::FAIL;

}

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

int choose_action(Monster* monster)
{
	monster->SetChoice(random_0_to_100(gen));

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

int growling(Monster* monster)
{
	if (monster->GetChoice() <= 10) {
		monster->SetAnimation(growl_ani);
		monster->SetWaitTime(monster->GetWaitTime() + monster->GetElapsedTime());
		if (monster->GetWaitTime() < 5200.0f) {
			// 위 시간을 포효하는 애니메이션 시간으로 조정
			return BehaviorTree::RUNNING;
		}
		monster->SetWaitTime(0.0f);
		return BehaviorTree::SUCCESS;
	}
	return BehaviorTree::FAIL;
}

int do_nothing(Monster* monster, float time)
{
	if (monster->GetChoice() <= 40) {
		monster->SetAnimation(idle_ani);
		do_nothing_time += monster->GetElapsedTime();

		if (do_nothing_time < time)
			return BehaviorTree::RUNNING;

		else {
			do_nothing_time = 0.0f;
			return BehaviorTree::SUCCESS;
		}
	}
	return BehaviorTree::FAIL;
}

// patrol

int find_random_pos(Monster* monster)
{
	XMFLOAT3 randompos;
	randompos.x = monster->GetPosition().x + 50.0f + urd(gen);
	randompos.y = monster->GetPosition().y;
	randompos.z = monster->GetPosition().z + 50.0f + urd(gen);

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

	angle = angle * XMVectorGetX(XMVector3Dot(upVec, XMVector3Cross(FrontVec, frontVec)));

	if (angle < -0.01f) {
		monster->Left(monster->GetElapsedTime());
	}
	else if (angle > 0.01f) {
		monster->Right(monster->GetElapsedTime());
	}
	else {
		XMFLOAT3 setfront;
		XMStoreFloat3(&setfront, frontVec);
		monster->SetFront(setfront);
	}
	monster->Foward(monster->GetElapsedTime());
	return BehaviorTree::RUNNING;
}

int move_to_user(Monster* monster, std::unordered_map<INT, Player>* players)
{
	monster->SetAnimation(walk_ani);

	auto playerIter = players->find(monster->GetTarget()->GetID());
	monster->SetTargetPos(playerIter->second.GetPosition());

	if (Distance(monster->GetPosition(), monster->GetTargetPos()) <= 5.0f)
		return BehaviorTree::SUCCESS;

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


	angle = angle * XMVectorGetX(XMVector3Dot(upVec, XMVector3Cross(FrontVec, frontVec)));

	if (angle < -0.01f) {
		monster->Left(monster->GetElapsedTime());
	}
	else if (angle > 0.01f) {
		monster->Right(monster->GetElapsedTime());
	}
	else {
		XMFLOAT3 setfront;
		XMStoreFloat3(&setfront, frontVec);
		monster->SetFront(setfront);
	}
	monster->Foward(monster->GetElapsedTime());

	return BehaviorTree::FAIL;
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

// die

int die(Monster* monster)
{
	monster->SetAnimation(die_ani);

	return BehaviorTree::RUNNING;
}

void build_bt(Monster* monster, std::unordered_map<INT, Player>* players)
{
	//fight
	bite_cooltime_node = Leaf("CoolTime(bite)", std::bind(wait_cool_time, monster, BITE_COOLTIME));
	bite_attack_node = Leaf("BITE!!", std::bind(bite_attack, monster));
	bite_attack_sequence = Sequence("bite sequence", { &bite_attack_node, &bite_cooltime_node });

	move_to_user_node = Leaf("Move to Target User", std::bind(move_to_user, monster, players));
	move_to_bite_sequence = Sequence("go_to_bite", { &move_to_user_node, &bite_attack_sequence });

	dash_cool_time_node = Leaf("CoolTime(Dash)", std::bind(wait_cool_time, monster, DASH_COOLTIME));
	dash_attack_node = Leaf("DASH!!", std::bind(dash_attack, monster));
	charging_dash_node = Leaf("Charging Dash...", std::bind(charging_dash, monster));
	set_target_node = Leaf("Set Target", std::bind(choose_target, monster, players));
	dash_sequence = Sequence("dash sequence", { &set_target_node, &charging_dash_node, &dash_attack_node, &dash_cool_time_node });

	dash_time_out_node = Leaf("Timeout check(dash)", std::bind(time_out_seq, monster, time_out_dash, CHASE_USER_TIME));
	dash_or_move_sequence = Sequence("dash or move seq", { &dash_time_out_node, &dash_sequence });
	attack_selector = Selector("attack selector", { &dash_or_move_sequence, &move_to_bite_sequence });

	choose_user_node = Leaf("choose user", std::bind(choose_target, monster, players));
	choose_user_time_out_node = Leaf("Timeout check(choose user)", std::bind(time_out, monster, time_out_choose, CHANGE_TARGET_USER_TIME));
	choose_user_selector = Selector("choose user selector", { &choose_user_time_out_node, &choose_user_node });

	fight_to_idle_node = Leaf("State Change(fight->idle)", std::bind(to_idle, monster, players));
	search_time_out_node = Leaf("Timeout check(Search)", std::bind(time_out, monster, time_out_search, CALMDOWN_TIME));
	search_user_node = Leaf("Search Users", std::bind(search_target, monster, players));

	search_user_selector = Selector("search user selector", { &search_user_node, &search_time_out_node, &fight_to_idle_node });

	fight_sequence = Sequence("fight sequence(root)", { &search_user_selector, &choose_user_selector, &attack_selector });

	//idle


	find_near_user_node = Leaf("Search Near User", std::bind(attent, monster, players, 3000.0f));
	idle_to_fight_node = Leaf("Change State(idle->fight)", std::bind(to_fight, monster, players));


	attentive_sequence = Sequence("Attent Sequence", { &find_near_user_node, &idle_to_fight_node });


	choose_random_action_node = Leaf("Choose action", std::bind(choose_action, monster));

	growling_node = Leaf("Growling", std::bind(growling, monster));
	do_nothing_node = Leaf("Do Nothing", std::bind(do_nothing, monster, 2000.0f));

	choose_action_selector = Selector("Do Action Selector", { &growling_node, &do_nothing_node, &patrol_sequence });



	find_random_pos_node = Leaf("Find Random Position", std::bind(find_random_pos, monster));
	move_to_node = Leaf("Move", std::bind(move_to, monster));
	patrol_cool_time_node = Leaf("Wait Next Patrol", std::bind(wait_cool_time, monster, WAIT_PATROL));

	patrol_sequence = Sequence("Patrol", { &find_random_pos_node, &move_to_node, &patrol_cool_time_node });

	idle_sequence = Sequence("Idle", { &choose_random_action_node, &choose_action_selector });

	idle_selector = Selector("", { &attentive_sequence, &idle_sequence });

	// runaway
	set_runaway_location_node = Leaf("Set RunAway Point", std::bind(set_runaway_location, monster));
	flyup_node = Leaf("Fly UP", std::bind(fly_up, monster));
	runaway_node = Leaf("Fly to Point", std::bind(move_to, monster));
	landing_node = Leaf("Landing", std::bind(landing, monster));
	runaway_to_idle_node = Leaf("Change State(runaway->idle)", std::bind(to_idle, monster, players));

	runaway_sequence = Sequence("RunAway", { &set_runaway_location_node, &flyup_node, &runaway_node, &landing_node, &runaway_to_idle_node });

	// die
	die_node = Leaf("DIE", std::bind(die, monster));

	// root
	if (monster->GetState() == idle_state)
		monster->BuildBT(&idle_selector);

	else if (monster->GetState() == fight_state)
		monster->BuildBT(&fight_sequence);

	else if (monster->GetState() == runaway_state)
		monster->BuildBT(&runaway_sequence);

	else if (monster->GetState() == die_state)
		monster->BuildBT(&die_node);
}

void run_bt(Monster* monster, std::unordered_map<INT, Player>* players)
{
	monster->ElapsedTime();
	check_hp(monster, players);
	monster->RunBT();
}