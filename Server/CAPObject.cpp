#include "CAPObject.h"
#include <random>
#include <chrono>
using namespace std::chrono_literals;

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
	m_money = 1000;

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

	death_count = 0;
}

void Player::SetWeapon(char weapon)
{
	m_weapon = weapon;
	switch (weapon)
	{
	case S_RIFLE:
		m_atk = RIFLE_ATK;
		m_ammo = 30;
		m_range = 250.0f;
		break;
	case S_SHOT_GUN:
		m_atk = SHOTGUN_ATK;
		m_ammo = 7;
		m_range = 150.0f;
		break;	
	case S_SNIPER:
		m_atk = SNIPER_ATK;
		m_ammo = 5;
		m_range = 400.0f;
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
	fly_up_speed = 0.1f;

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
	prev_state = curr_state = idle_state;

	home = 0;

	m_bounding_box.Center = m_position;
	m_bounding_box.Center.y += 15.0f;
	m_bounding_box.Extents = XMFLOAT3(30.0f, 30.0f, 30.0f);
	float radian = XMConvertToRadians(m_yaw);
	XMFLOAT4 q{};
	XMStoreFloat4(&q, XMQuaternionRotationRollPitchYaw(0.f, radian, 0.f));
	m_bounding_box.Orientation = q;
}
bool tail_attack_on = false;
// functions

void check_hp(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room)
{
	float hp = monster->GetHp();

	if (hp <= 0) {
		monster->SetState(die_state);
		build_bt(monster, players, room);
	}

	else if (monster->GetState() == runaway_state) return;

	else if (hp <= monster->GetRAHp()) {
		monster->SetState(runaway_state);
		if (hp >= 300.0f) {
			monster->SetRAHp(hp * 0.5f);
		}
		build_bt(monster, players, room);
	}
}

float CalculateAngleBetweenVectors(XMVECTOR vecA, XMVECTOR vecB) {
	// Extract the x and z components and ignore the y component
	XMVECTOR vecAXZ = XMVectorSet(XMVectorGetX(vecA), 0.0f, XMVectorGetZ(vecA), 0.0f);
	XMVECTOR vecBXZ = XMVectorSet(XMVectorGetX(vecB), 0.0f, XMVectorGetZ(vecB), 0.0f);

	// Normalize the vectors
	XMVECTOR vecANormalized = XMVector3Normalize(vecAXZ);
	XMVECTOR vecBNormalized = XMVector3Normalize(vecBXZ);

	// Calculate the dot product
	float dotProduct = XMVectorGetX(XMVector3Dot(vecANormalized, vecBNormalized));

	// Ensure the dot product is within the valid range for arccos
	dotProduct = max(-1.0f, min(1.0f, dotProduct));

	// Calculate the angle in radians
	float angleRadians = acosf(dotProduct);

	// Calculate the cross product
	XMVECTOR crossProduct = XMVector3Cross(vecANormalized, vecBNormalized);

	// Extract the y component of the cross product
	float crossY = XMVectorGetY(crossProduct);

	// Determine the sign of the angle based on the cross product's y component
	if (crossY < 0) {
		angleRadians = -angleRadians;
	}

	// Convert the angle from radians to degrees
	float angleDegrees = XMConvertToDegrees(angleRadians);

	return angleDegrees;
}

bool is_toofar(Monster* monster) {
	if (monster->GetPosition().x <= monster->sector_line[monster->home][0] || (monster->GetPosition().x >= monster->sector_line[monster->home][2]) ||
		monster->GetPosition().z <= monster->sector_line[monster->home][1] || (monster->GetPosition().z >= monster->sector_line[monster->home][3])) {
		return true;
	}
	return false;
}

bool tail_attack_check(Monster* monster) {
	if (Distance(monster->GetPosition(), monster->GetTarget()->GetPosition()) <= 200.0f) {
		return true;
	}
	if (tail_attack_on) {
		return true;
	}

	return false;
}

// 가야하는 방향(타겟)을 향해 쳐다보도록 회전하는 함수
void SetFaceDirection(Monster* monster)
{
	XMFLOAT3 target = monster->GetTargetPos();
	XMFLOAT3 pos = { monster->GetPosition().x, LAND_Y, monster->GetPosition().z };

	XMVECTOR targetPosVec = XMLoadFloat3(&target);
	XMVECTOR currentPosVec = XMLoadFloat3(&pos);

	XMVECTOR frontVec = XMVector3Normalize(XMVectorSubtract(targetPosVec, currentPosVec)); // 봐야하는 방향

	// 회전각 계산후 적용
	XMFLOAT3 Front = monster->GetFront();
	XMVECTOR currentFrontVec = XMLoadFloat3(&Front);
	float angle = CalculateAngleBetweenVectors(currentFrontVec, frontVec);
	monster->SetYaw(monster->GetYaw() + angle);

	// 방향을 적용
	XMFLOAT3 setfront;
	XMStoreFloat3(&setfront, frontVec);
	monster->SetFront(setfront);
}

//-----------------------------------------------------------------------------------------
//// Behaviors Nodes

// idle
Selector idle_selector;
	Leaf check_near_user_node;
	Leaf idle_to_home_node; ConditionChecker idle_to_home_dec;	 // conditional dec 
	Sequence action_seq;
		RandomNode random_action;
			Sequence patrol_seq;
				Leaf set_patrol_pos_node;
				Leaf go_to_pos_node;
			Leaf growling_node;
			Leaf sleep_node;
		Leaf set_idle_ani_node;
		Leaf wait_next_idle_node; TimeLimiter wait_next_idle_dec; // delay dec

	
// alert_state
Selector alert_selector;
	Sequence check_near_user_seq;
		Leaf checking_user_node;
		Leaf to_fight_time_check_node;
		Leaf alert_to_fight_node;
	Sequence alert_to_idle_seq;
		Leaf to_idle_time_check_node;
		Leaf alert_to_idle_node;


// fight
Sequence fight_sequence;
	Sequence find_user_seq;
		Selector check_near_user_sel;
			Leaf find_user_node;
			Leaf fight_to_alert_node;
		RandomNode	choice_target_user_randnode;
			Leaf choice_latest_user_node;
			Leaf choice_near_user_node;
			Leaf choice_far_user_node;
			Leaf choice_random_user_node;
	Selector go2home_or_attack_sel;
		Leaf fight_to_go2home_node; ConditionChecker fight_to_home_dec;
		RandomNode attack_randomnode;
			Leaf fireball_node;
			Sequence dash_seq;
				Leaf change_ani_node;
				Leaf charging_dash_node;	TimeLimiter dash_attack_dec;
				Leaf dash_attack_node;	
			Sequence bite_seq;
				Selector bite_attack_check_sel;
					Sequence bite_timeout_dash_seq;
						Leaf move_to_bite_time_check_node; 
						// 돌진 시퀀스 재사용
					Sequence move_to_bite_seq;
						Leaf move_to_bite_node;
						Leaf bite_node;
			Sequence irontail_seq;	ConditionChecker user_distance_dec;
				// change_ani_node 노드 재활용
				Leaf charging_irontail_node;	TimeLimiter irontail_dec;
				Leaf irontail_node;
	Leaf change_ani_to_idle_node;
	Leaf wait_next_attack_node;	TimeLimiter wait_next_attack_dec;

// runaway
Sequence runaway_sequence;
	Leaf set_runaway_point_node;
	Leaf take_off_node;
	Leaf fly_up_node;
	Leaf fly_to_point_node;
	Leaf fly_down_node;
	Leaf landing_node;
	Leaf runaway_to_idle_node; TimeLimiter wait_change_to_idle_dec;

// go home
Sequence goHome_sequence;
	Leaf move_to_home_node;
	Leaf go2home_to_idle_node; TimeLimiter wait_gohome_to_idle_dec;

// die
Leaf	die_node;

// blind
Sequence blind_sequence;

// runaway points
XMFLOAT3 runaway_point01 = XMFLOAT3(2200.0f, LAND_Y, 3100.0f);
XMFLOAT3 runaway_point02 = XMFLOAT3(410.f, LAND_Y, 3055.f);
XMFLOAT3 runaway_point03 = XMFLOAT3(340.f, LAND_Y, 1450.f);

XMFLOAT3 runaway_point[RUNAWAY_POINT] = { runaway_point01, runaway_point02, runaway_point03 };

//-----------------------------------------------------------------------------------------
//// Behaviors Function
int success() {
	return BehaviorTree::SUCCESS;
}

int fail() {
	return BehaviorTree::FAIL;
}

///////////// idle
int check_near_user_idle(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	bool isTargetExist = false;
	for (int i : room->GetPlyId()) {
		if (i == -1) continue;
		auto playerIter = players->find(i);
		if (playerIter != players->end()) {
			monster->SetUserArround(i, false);
			if (playerIter->second.GetID() != -1) {
				if (Distance(monster->GetPosition(), playerIter->second.GetPosition()) <= IDLE_FIND_USER_DISTANCE) {
					isTargetExist = true;
					monster->SetUserArround(i, true);
				}
			}
		}
	}
	if (isTargetExist) {
		monster->SetAnimation(idle_ani);
		monster->SetState(alert_state);
		build_bt(monster, players, room);

		return BehaviorTree::SUCCESS;
	}

	return BehaviorTree::FAIL;
}

int idle_to_gohome(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	monster->SetAnimation(idle_ani);
	monster->SetState(gohome_state);
	build_bt(monster, players, room);

	return BehaviorTree::SUCCESS;
}

int set_random_pos(Monster* monster) {
	XMFLOAT3 randompos;
	float dir = minus(gen);
	dir = (dir / abs(dir));
	randompos.x = monster->GetPosition().x + urd(gen) * dir;
	randompos.y = monster->GetPosition().y;
	randompos.z = monster->GetPosition().z + urd(gen) * dir;

	monster->SetTargetPos(randompos);

	return BehaviorTree::SUCCESS;
}

int move_to_random_pos(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	monster->SetAnimation(walk_ani);


	if (is_toofar(monster)) {
		monster->SetState(gohome_state);
		build_bt(monster, players, room);
		return BehaviorTree::SUCCESS;
	}

	if (Distance(monster->GetPosition(), monster->GetTargetPos()) <= 5.0f) {
		monster->SetAnimation(idle_ani);
		return BehaviorTree::SUCCESS;
	}
	SetFaceDirection(monster);

	monster->Foward(monster->GetElapsedTime());
	return BehaviorTree::RUNNING;
}




int growling(Monster* monster) {
	monster->SetAnimation(growl_ani);

	static float growlTime = 0.f;
	if (growlTime <= GROWL_ANI_TIME) {
		growlTime += monster->GetElapsedTime();
		return BehaviorTree::RUNNING;
	}
	else {
		growlTime = 0.0f;
		return BehaviorTree::SUCCESS;
	}
}

int sleeping(Monster* monster) {
	monster->SetAnimation(sleep_ani);

	static float sleepTime = 0.f;
	if (sleepTime <= SLEEP_ANI_TIME) {
		sleepTime += monster->GetElapsedTime();
		return BehaviorTree::RUNNING;
	}
	else {
		sleepTime = 0.0f;
		return BehaviorTree::SUCCESS;
	}
}

int setAnimIdle(Monster* monster) {
	monster->SetAnimation(idle_ani);
	return BehaviorTree::SUCCESS;
}

int wait_next_idle(Monster* monster) {
	return BehaviorTree::SUCCESS;
}

///////////// alert
static float a2f_time = 0.0f;
static float a2i_time = 0.0f;

int check_near_user_alert(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	bool isTargetExist = false;
	for (int i : room->GetPlyId()) {
		if (i == -1) continue;
		auto playerIter = players->find(i);
		if (playerIter != players->end()) {
			monster->SetUserArround(i, false);
			if (playerIter->second.GetID() != -1) {
				if (Distance(monster->GetPosition(), playerIter->second.GetPosition()) <= ALERT_FIND_USER_DISTANCE) {
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

int time_check_to_fight(Monster* monster) {
	if (a2f_time >= ALERT_TO_FIGHT_TIME) {
		a2f_time = 0;
		a2i_time = 0;
		return BehaviorTree::SUCCESS;
	}
	a2f_time += monster->GetElapsedTime();
	return BehaviorTree::FAIL;
}

int alert_to_fight(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	monster->SetState(fight_state);
	build_bt(monster, players, room);
	return BehaviorTree::SUCCESS;
}

int time_check_to_idle(Monster* monster) {
	if (a2i_time >= ALERT_TO_IDLE_TIME) {
		a2i_time = 0;
		a2f_time = 0;
		return BehaviorTree::SUCCESS;
	}
	a2i_time += monster->GetElapsedTime();
	return BehaviorTree::FAIL;
}

int alert_to_idle(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	monster->SetState(idle_state);
	build_bt(monster, players, room);
	return BehaviorTree::SUCCESS;
}


/////////// fight
static float check_time = 0.0f;

int check_near_user_fight(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	bool isTargetExist = false;
	for (int i : room->GetPlyId()) {
		if (i == -1) continue;
		auto playerIter = players->find(i);
		if (playerIter != players->end()) {
			monster->SetUserArround(i, false);
			if (playerIter->second.GetID() != -1) {
				if (Distance(monster->GetPosition(), playerIter->second.GetPosition()) <= IDLE_FIND_USER_DISTANCE) {
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

int fight_to_alert(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	monster->SetAnimation(idle_ani);
	monster->SetState(alert_state);
	build_bt(monster, players, room);
	return BehaviorTree::SUCCESS;
}

int choice_latest_user(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	// 공격받은 적이 없다면 실패
	if (monster->GetLatestAttackPlayer() == nullptr) {
		return BehaviorTree::FAIL;
	}

	//거리가 너무 멀면 실패
	if (Distance(monster->GetLatestAttackPlayer()->GetPosition(), monster->GetPosition()) >= FIGHT_FIND_USER_DISTANCE) {
		return BehaviorTree::FAIL;
	}

	if (monster->GetLatestAttackPlayer() != nullptr) {
		monster->SetTarget(monster->GetLatestAttackPlayer());
		monster->SetLatestAttackPlayer(nullptr); //한번 타겟을 설정하고 마지막 공격자 초기화하기
		monster->SetTargetPos(monster->GetTarget()->GetPosition());
		return BehaviorTree::SUCCESS;
	}
	// 최근 공격 유저가 사라지면 실패
	return BehaviorTree::FAIL;
}

int choice_near_user(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	bool targetcheck[MAX_CLIENT_ROOM];
	bool isTargetExist = false;

	float min_distance = 10000000.0f;

	for (int i = 0; i < MAX_CLIENT_ROOM; i++) {
		targetcheck[i] = false;
		if (monster->GetUserArround(i)) {
			isTargetExist = true;
			targetcheck[i] = true;
		}
	}

	for (int i = 0; i < MAX_CLIENT_ROOM; i++) {
		if (targetcheck[i]) {
			auto playerIter = players->find(i);
			if (playerIter != players->end()) {
				float distance = Distance(playerIter->second.GetPosition(), monster->GetPosition());
				if (distance < min_distance) {
					min_distance = distance;
					monster->SetTarget(&playerIter->second);
				}
			}
		}
	}

	if (min_distance >= 100000.0f)
		return BehaviorTree::FAIL;

	monster->SetTargetPos(monster->GetTarget()->GetPosition());
	return BehaviorTree::SUCCESS;
}

int choice_far_user(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	bool targetcheck[MAX_CLIENT_ROOM];
	bool isTargetExist = false;

	float max_distance = -100000000.0f;

	for (int i = 0; i < MAX_CLIENT_ROOM; i++) {
		targetcheck[i] = false;
		if (monster->GetUserArround(i)) {
			isTargetExist = true;
			targetcheck[i] = true;
		}
	}

	for (int i = 0; i < MAX_CLIENT_ROOM; i++) {
		if (targetcheck[i]) {
			auto playerIter = players->find(i);
			if (playerIter != players->end()) {
				float distance = Distance(playerIter->second.GetPosition(), monster->GetPosition());
				if (distance > max_distance) {
					max_distance = distance;
					monster->SetTarget(&playerIter->second);
				}
			}
		}
	}

	if (max_distance <= -100000.0f)
		return BehaviorTree::FAIL;

	monster->SetTargetPos(monster->GetTarget()->GetPosition());
	return BehaviorTree::SUCCESS;
}

int choice_random_user(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
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
				monster->SetTargetPos(monster->GetTarget()->GetPosition());
				return BehaviorTree::SUCCESS;
			}
		}
	}

	return BehaviorTree::FAIL;
}

int fight_to_home(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	monster->SetAnimation(idle_ani);
	monster->SetState(gohome_state);
	build_bt(monster, players, room);
	return BehaviorTree::SUCCESS;
}

int fire_ball(Monster* monster) {
	SetFaceDirection(monster);
	monster->SetAnimation(fireball_ani);
	static float firetime = 0.0f;
	if (firetime >= FIREBALL_ANI_TIME) {
		firetime = 0;
		monster->SetAnimation(idle_ani);
		return BehaviorTree::SUCCESS;
	}
	firetime += monster->GetElapsedTime();
	return BehaviorTree::RUNNING;
}

int change_ani_to_charge(Monster* monster) {
	SetFaceDirection(monster);
	monster->SetAnimation(charging_ani);
	return BehaviorTree::SUCCESS;
}

int charging_dash(Monster* monster) {
	return BehaviorTree::SUCCESS;
}

int dash(Monster* monster) {
	monster->SetAnimation(dash_ani);

	if (Distance(monster->GetPosition(), monster->GetTargetPos()) <= 12.0f) {
		monster->SetAnimation(idle_ani);
		return BehaviorTree::SUCCESS;
	}
	SetFaceDirection(monster);

	monster->Foward(monster->GetElapsedTime() * 3.0f);
	return BehaviorTree::RUNNING;
}

int move_to_bite(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	monster->SetAnimation(walk_ani);

	monster->SetTargetPos(monster->GetTarget()->GetPosition());

	if (is_toofar(monster)) {
		monster->SetAnimation(idle_ani);
		monster->SetState(gohome_state);
		build_bt(monster, players, room);
		return BehaviorTree::SUCCESS;
	}

	if (Distance(monster->GetPosition(), monster->GetTargetPos()) <= 35.0f) {
		return BehaviorTree::SUCCESS;
	}
	SetFaceDirection(monster);

	monster->Foward(monster->GetElapsedTime());
	return BehaviorTree::RUNNING;
}

int bite(Monster* monster) {
	check_time = 0;
	SetFaceDirection(monster);
	monster->SetAnimation(bite_ani);
	static float bitetime = 0.0f;
	//std::cout << "깨물기 재생중 " << TAIL_ANI_TIME - bitetime << "남음" << std::endl;
	if (bitetime >= BITE_ANI_TIME) {
		bitetime = 0.0f;
		return BehaviorTree::SUCCESS;
	}
	bitetime += monster->GetElapsedTime();
	return BehaviorTree::RUNNING;
}

int time_check_to_dash(Monster* monster) {
	if (check_time >= BITE_TO_DASH_TIMEOUT) {
		check_time = 0.0f;
		return BehaviorTree::SUCCESS;
	}
	check_time += monster->GetElapsedTime();
	return BehaviorTree::FAIL;
}

int charging_irontail(Monster* monster) {
	SetFaceDirection(monster);
	return BehaviorTree::SUCCESS;
}

int irontail(Monster* monster) {
	monster->SetAnimation(tail_ani);
	static float tailtime = 0.0f;
	tail_attack_on = true;
	//std::cout << "꼬리치기 재생중 " << TAIL_ANI_TIME - tailtime << "남음" << std::endl;
	if (tailtime >= TAIL_ANI_TIME) {
		tailtime = 0.0f;
		tail_attack_on = false;
		return BehaviorTree::SUCCESS;
	}
	tailtime += monster->GetElapsedTime();
	return BehaviorTree::RUNNING;
}

int change_ani_to_idle(Monster* monster) {
	monster->SetAnimation(idle_ani);
	return BehaviorTree::SUCCESS;
}

int wait_next_attack(Monster* monster) {
	check_time = 0.0f; // 깨물려고 가다가 대쉬하는 시간 체크를 초기화
	return BehaviorTree::SUCCESS;
}


/////////// runaway
int set_runaway_point(Monster* monster) {
	int point = 0;
	while (1) {
		point = rand_runaway_point(gen);
		if (point != monster->home) {
			monster->SetTargetPos(runaway_point[point]);
			monster->home = point;
			break;
		}
	}
	return BehaviorTree::SUCCESS;
}

int take_off(Monster* monster) {
	monster->SetAnimation(flyup_ani);

	static float takeoff_time = 0.f;
	if (takeoff_time <= TAKE_OFF_ANI_TIME) {
		takeoff_time += monster->GetElapsedTime();
		return BehaviorTree::RUNNING;
	}
	else {
		takeoff_time = 0.0f;
		monster->SetPostion(monster->GetPosition().x, monster->GetPosition().y + 30.0f, monster->GetPosition().z);
		return BehaviorTree::SUCCESS;
	}
}

int fly_up(Monster* monster) {
	monster->SetAnimation(flyidle_ani);
	if (monster->GetPosition().y <= 500.f) {

		monster->Up(monster->GetElapsedTime());
		return BehaviorTree::RUNNING;
	}

	return BehaviorTree::SUCCESS;
}

int fly_to_point(Monster* monster) {
	monster->SetAnimation(flying_ani);
	if (Distance(monster->GetPosition(), monster->GetTargetPos()) <= 5.0f) 
		return BehaviorTree::SUCCESS;

	SetFaceDirection(monster);

	monster->Foward(monster->GetElapsedTime()*3.0f);
	return BehaviorTree::RUNNING;
}

int fly_down(Monster* monster) {
	monster->SetAnimation(flyidle_ani);

	if (monster->GetPosition().y >= LAND_Y) {

		monster->Down(monster->GetElapsedTime());
		return BehaviorTree::RUNNING;
	}
	return BehaviorTree::SUCCESS;
}

int landing(Monster* monster) {
	monster->SetAnimation(landing_ani);

	static float landing_time = 0.f;
	if (landing_time <= LANDING_ANI_TIME) {
		landing_time += monster->GetElapsedTime();
		return BehaviorTree::RUNNING;
	}
	else {
		landing_time = 0.0f;
		monster->SetAnimation(idle_ani);
		monster->SetPostion(monster->GetPosition().x, LAND_Y, monster->GetPosition().z);
		return BehaviorTree::SUCCESS;
	}
}

int runaway_to_idle(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	monster->SetAnimation(idle_ani);
	monster->SetState(idle_state);
	build_bt(monster, players, room);
	return BehaviorTree::SUCCESS;
}



/////////// go Home
int move_to_home(Monster* monster) {

	monster->SetAnimation(walk_ani);

	monster->SetTargetPos(runaway_point[monster->home]);

	if (Distance(monster->GetPosition(), monster->GetTargetPos()) <= 5.0f) {
		monster->SetAnimation(idle_ani);
		return BehaviorTree::SUCCESS;
	}
	SetFaceDirection(monster);

	monster->Foward(monster->GetElapsedTime());
	return BehaviorTree::RUNNING;
}

int go2home_to_idle(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room) {
	monster->SetAnimation(idle_ani);
	monster->SetState(idle_state);
	build_bt(monster, players, room);
	return BehaviorTree::SUCCESS;
}

/////////// die
int die(Monster* monster)
{
	monster->SetAnimation(die_ani);

	return BehaviorTree::RUNNING;
}


void build_bt(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room)
{
	// idle
	{
		wait_next_idle_node = Leaf("TimeOut", std::bind(wait_next_idle, monster));
		wait_next_idle_dec = TimeLimiter(&wait_next_idle_node, 2.0s);

		set_patrol_pos_node = Leaf("Set Patrol Pos", std::bind(set_random_pos, monster));
		go_to_pos_node = Leaf("Go To Random Pos", std::bind(move_to_random_pos, monster, players, room));

		patrol_seq = Sequence("Patrol Seq", { &set_patrol_pos_node, &go_to_pos_node });

		growling_node = Leaf("Growling", std::bind(growling, monster));
		sleep_node = Leaf("Sleeping... Zzz", std::bind(sleeping, monster));

		random_action = RandomNode("Random Idle Action", { &patrol_seq, &growling_node, &sleep_node });

		set_idle_ani_node = Leaf("Change Animation to idle", std::bind(setAnimIdle, monster));
		action_seq = Sequence("Action Sequence", { &random_action, &set_idle_ani_node, &wait_next_idle_dec });

		idle_to_home_node = Leaf("GotoHome", std::bind(idle_to_gohome, monster, players, room));
		idle_to_home_dec = ConditionChecker(&idle_to_home_node, [monster]() {
			if (is_toofar(monster))
				return true;
			else
				return false;
			});

		check_near_user_node = Leaf("Check Near User(idle)", std::bind(check_near_user_idle, monster, players, room));

		idle_selector = Selector("IDLE", { &check_near_user_node, &idle_to_home_dec, &action_seq });
	}

	// alert
	{
		alert_to_idle_node = Leaf("Alert to Idle", std::bind(alert_to_idle, monster, players, room));
		to_idle_time_check_node = Leaf("To idle Time Check", std::bind(time_check_to_idle, monster));

		alert_to_idle_seq = Sequence("Alert To Idle Sequence", { &to_idle_time_check_node, &alert_to_idle_node });

		alert_to_fight_node = Leaf("Alert to Fight", std::bind(alert_to_fight, monster, players, room));
		to_fight_time_check_node = Leaf("To idle Time Check", std::bind(time_check_to_fight, monster));
		checking_user_node = Leaf("Check Near User(alert)", std::bind(check_near_user_alert, monster, players, room));

		check_near_user_seq = Sequence("Check Near User Sequence", { &checking_user_node, &to_fight_time_check_node, &alert_to_fight_node });

		alert_selector = Selector("Alert Selector", { &check_near_user_seq, &alert_to_idle_seq });
	}

	// fight
	{
		wait_next_attack_node = Leaf("Next Attack", std::bind(wait_next_attack, monster));
		wait_next_attack_dec = TimeLimiter(&wait_next_attack_node, NEXT_ATTACK_TIME);

		change_ani_to_idle_node = Leaf("Change animation -> idle", std::bind(change_ani_to_idle, monster));
			
		irontail_node = Leaf("IRON TAIL", std::bind(irontail, monster));
		charging_irontail_node = Leaf("charging irontail", std::bind(charging_irontail, monster));
		irontail_dec = TimeLimiter(&charging_irontail_node, CHARGING_IRONTAIL_TIME);
		
		change_ani_node = Leaf("change ani", std::bind(change_ani_to_charge, monster));

		irontail_seq = Sequence("IronTail Sequence", { &change_ani_node, &irontail_dec, &irontail_node });
		user_distance_dec = ConditionChecker(&irontail_seq, [monster]() {
			if (tail_attack_check(monster))
				return true;
			else
				return false;
			});

		dash_attack_node = Leaf("DASH", std::bind(dash, monster));
		charging_dash_node = Leaf("charging irontail", std::bind(charging_dash, monster));
		dash_attack_dec = TimeLimiter(&charging_dash_node, CHARGING_DASH_TIME);

		dash_seq = Sequence("Dash Sequence", { &change_ani_node, &dash_attack_dec, &dash_attack_node });

		fireball_node = Leaf("FIREBALL", std::bind(fire_ball, monster));

		bite_node = Leaf("BITE", std::bind(bite, monster));
		move_to_bite_node = Leaf("move to bite", std::bind(move_to_bite, monster, players, room));

		move_to_bite_seq = Sequence("move_to_bite_Sequence", { &move_to_bite_node, &bite_node });

		move_to_bite_time_check_node = Leaf("move timeout dash now", std::bind(time_check_to_dash, monster));

		bite_timeout_dash_seq = Sequence("bite_timeout_dash Sequence", { &move_to_bite_time_check_node, &dash_seq });

		bite_attack_check_sel = Selector("bite check", { &bite_timeout_dash_seq, &move_to_bite_seq });

		attack_randomnode = RandomNode("Random Attack", { &fireball_node, &dash_seq, &bite_attack_check_sel, &user_distance_dec });

		fight_to_go2home_node = Leaf("go to home", std::bind(fight_to_home, monster, players, room));
		fight_to_home_dec = ConditionChecker(&fight_to_go2home_node, [monster]() {
			if (is_toofar(monster))
				return true;
			else
				return false;
			});

		go2home_or_attack_sel = Selector("go home or attack", { &fight_to_home_dec, &attack_randomnode });

		choice_random_user_node = Leaf("choice random user", std::bind(choice_random_user, monster, players, room));
		choice_far_user_node = Leaf("choice random user", std::bind(choice_far_user, monster, players, room));
		choice_near_user_node = Leaf("choice random user", std::bind(choice_near_user, monster, players, room));
		choice_latest_user_node = Leaf("choice random user", std::bind(choice_latest_user, monster, players, room));

		choice_target_user_randnode = RandomNode("Choice Random User", { &choice_latest_user_node, &choice_near_user_node, &choice_far_user_node, &choice_random_user_node });

		
		fight_to_alert_node = Leaf("fight to alert", std::bind(fight_to_alert, monster, players, room));
		find_user_node = Leaf("search near user", std::bind(check_near_user_fight, monster, players, room));

		check_near_user_sel = Selector("check near user", { &find_user_node, &fight_to_alert_node });

		find_user_seq = Sequence("find user sequence", { &check_near_user_sel, &choice_target_user_randnode });


		fight_sequence = Sequence("fight root sequence", { &find_user_seq, &go2home_or_attack_sel, &change_ani_to_idle_node , &wait_next_attack_dec });
	}

	// runaway
	{
		runaway_to_idle_node = Leaf("runaway to idle", std::bind(runaway_to_idle, monster, players, room));
		wait_change_to_idle_dec = TimeLimiter(&runaway_to_idle_node, 2s);
		landing_node = Leaf("landing", std::bind(landing, monster));
		fly_down_node = Leaf("fly down", std::bind(fly_down, monster));
		fly_to_point_node = Leaf("go to potin", std::bind(fly_to_point, monster));
		fly_up_node = Leaf("fly up", std::bind(fly_up, monster));
		take_off_node = Leaf("take off", std::bind(take_off, monster));

		set_runaway_point_node = Leaf("Set RunAway Point", std::bind(set_runaway_point, monster));

		runaway_sequence = Sequence("RUN AWAY Sequence", { &set_runaway_point_node, &take_off_node, &fly_up_node, &fly_to_point_node,
			&fly_down_node, &landing_node, &wait_change_to_idle_dec });
	}

	// go home
	{
		go2home_to_idle_node = Leaf("go home -> idle", std::bind(go2home_to_idle, monster, players, room));
		wait_gohome_to_idle_dec = TimeLimiter(&go2home_to_idle_node, 0.2s);

		move_to_home_node = Leaf("move to home", std::bind(move_to_home, monster));

		goHome_sequence = Sequence("go home Sequence", { &move_to_home_node, &wait_gohome_to_idle_dec });
	}

	// blind
	{

	}

	// die
	die_node = Leaf("DIE", std::bind(die, monster));

	// root
	{
		if (monster->GetState() == idle_state) 
		{
			monster->BuildBT(&idle_selector);
		}

		else if (monster->GetState() == alert_state)
		{
			monster->BuildBT(&alert_selector);
		}

		else if (monster->GetState() == fight_state)
		{
			monster->BuildBT(&fight_sequence);
		}

		else if (monster->GetState() == runaway_state)
		{
			monster->BuildBT(&runaway_sequence);
		}

		else if (monster->GetState() == gohome_state)
		{
			monster->BuildBT(&goHome_sequence);
		}

		else if (monster->GetState() == die_state)
		{
			monster->BuildBT(&die_node);
		}

		else if (monster->GetState() == blind_state)
		{
			monster->BuildBT(&blind_sequence);
		}
	}
}

void run_bt(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room)
{
	monster->ElapsedTime();
	check_hp(monster, players, room);
	monster->RunBT();
	//std::cout << (int)monster->GetAnimation() << std::endl;
}