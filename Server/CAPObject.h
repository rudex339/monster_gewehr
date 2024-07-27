#pragma once

#include "stdafx.h"
#include "BehaviorTree.h"
#include "BehaviorTreeDefines.h"
#include "GameRoom.h"

class GameRoom;



class CAPObject
{
public:
	CAPObject();
	~CAPObject() = default;	

	void SetPostion(const DirectX::XMFLOAT3& pos) { m_position = pos; }
	void SetPostion(float x, float y, float z);
	void SetVelocity(const DirectX::XMFLOAT3& vel) { m_velocity = vel; }
	void SetYaw(float yaw) { m_yaw = yaw; }
	void SetID(int id)	{ m_id = id; }
	void SetRoomID(short id) { m_room_id = id; }
	void SetBoundingBox(XMFLOAT3 center) { m_bounding_box.Center = center; m_bounding_box.Center.y += 5.0f; }
	void SetBoundingBox() { m_bounding_box.Center = m_position; }

	void RotateBoundingBox(float yaw);
	void RotateBoundingBox();

	DirectX::XMFLOAT3 GetPosition() { return m_position; }
	DirectX::XMFLOAT3 GetVelocity() { return m_velocity; }
	FLOAT GetYaw() { return m_yaw; }
	CHAR GetID() { return m_id; }
	SHORT GetRoomID() { return m_room_id; }
	DirectX::BoundingOrientedBox GetBoundingBox() { return m_bounding_box; }

protected:
	
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_velocity;
	FLOAT m_yaw;
	DirectX::BoundingOrientedBox m_bounding_box;
	CHAR m_id;
	SHORT m_room_id;
};



class Player : public CAPObject
{
public:
	Player();
	~Player() = default; ;
	Player(int id, SOCKET socket);
	void PlayerInit();

	void SetSocket(SOCKET& sock) { m_socket = sock; }
	void SetName(std::string name) { m_name = name; }
	void SetPassword(std::string password) { m_password = password; }
	void SetHp(float hp) { m_hp = hp; }
	void SetMaxHp(float max_hp) { m_max_hp = max_hp; }
	void SetAtk(float atk) { m_atk = atk; }
	void SetDef(float def) { m_def = def; }
	void SetAmmo(float ammo) { m_ammo = ammo; }
	void SetMag(float mag) { m_mag = mag; }
	void SetWeapon(char wepon);
	void SetArmor(char armor);
	void SetThrowWp(char throw_wp) { m_throw_wp = throw_wp; }
	void SetRemainSize(int remain_size) { m_remain_size = remain_size; }
	void SetState(S_STATE state) { m_state = state; }
	void SetAnimaition(char ani) { m_animation = ani; }
	void SetAtkDir(const DirectX::XMFLOAT3& dir) { m_atk_dir = dir; }
	void SetAtkPos(const DirectX::XMFLOAT3& pos) { m_atk_pos = pos; }
	void SetHost(bool is_host) { m_is_host = is_host; }
	void SetReady(bool is_ready) { m_is_ready = is_ready; }
	void SetMoney(int money) { m_money = money; }
	void SetItem(int type, int val) { m_item_info[type] = val; }

	void Lock() { m_lock.lock(); }
	void UnLock() { m_lock.unlock(); }

	int GetRemainSize() { return m_remain_size; }
	char GetWeapon() { return m_weapon; }
	char GetArmor() { return m_armor; }
	char GetThrowWp() { return m_throw_wp; }
	char GetAnimaition() { return m_animation; }
	S_STATE GetState() { return m_state; }
	std::string GetName() { return m_name; }
	std::string GetPassword() { return m_password; }
	PLAYER_DATA GetPlayerData() { return { m_id, m_position, m_velocity, m_yaw, m_hp }; }
	DirectX::XMFLOAT3 GetAtkDir() { return m_atk_dir; }
	DirectX::XMFLOAT3 GetAtkPos() { return m_atk_pos; }
	float GetHp() { return m_hp; }
	float GetRange() { return m_range; }
	bool GetHost() { return m_is_host; }
	bool GetReady() { return m_is_ready; }
	int GetAtk() { return m_atk; }
	int GetMoney() { return m_money; }
	int GetItem(int type) { return m_item_info[type]; }

	int RecvData();
	int DoSend(void* p, size_t size);

	void HitPlayer(int damage) { m_hp -= (damage - m_def); }

	void closesock() { closesocket(m_socket); }

	CHAR m_recv_buf[BUF_SIZE];
	BOOL hit_on = 0;	// 이건 나중에 피격 애니메이션으로 판정할거임 지금은 피격애니메이션 없어서 임시로 사용

	SHORT death_count = 0;	// 이것도 나중에 게임룸 클래스에 넣어서 모든 유저의 죽음 횟수를 세서 게임오버나 점수 깎는걸 만들거임

protected:
	SOCKET m_socket;

	ITEM_DATA items;

	std::string m_name;
	std::string m_password;

	std::mutex m_lock;

	S_STATE m_state;	

	FLOAT m_hp;
	FLOAT m_max_hp;
	FLOAT m_atk;
	FLOAT m_def;
	FLOAT m_range;

	FLOAT m_ammo;	// 현재 총합 총알(소지중 탄약)
	FLOAT m_mag;	// 현재 탄창에 들어있는 탄약

	DirectX::XMFLOAT3 m_atk_dir;
	DirectX::XMFLOAT3 m_atk_pos;

	CHAR m_weapon;	
	CHAR m_armor;
	CHAR m_throw_wp;	// 섬광인지 슈류탄인지
	CHAR m_animation;

	// 데이터 베이스 연동 해줘야함
	INT m_money;

	INT m_item_info[10];	// 아이템 정보들, 0라이플 시작, 강화율이나 소지율
	//------------------------
	INT m_remain_size;

	BOOL m_is_host;

	BOOL m_is_ready;
};


enum MonsterState { idle_state, alert_state, fight_state, runaway_state, gohome_state, die_state, blind_state };
enum MonsterAnimation {
	idle_ani,
	growl_ani, 
	walk_ani,
	flyup_ani, 
	flying_ani, 
	landing_ani,
	bite_ani, 
	dash_ani,
	blind_ani, 
	die_ani,
	tail_ani,
	charging_ani,
	flyidle_ani,
	sleep_ani,
	fireball_ani
};

class Monster : public CAPObject
{
public:
	Monster();
	~Monster() = default;

	// 이동 관련
	float ElapsedTime();
	FLOAT GetElapsedTime() { return elapsedTime; };
	FLOAT GetWaitTime() { return waitTime; }
	void SetWaitTime(FLOAT time) { waitTime = time; }

	void Foward(float elapsedTimer);
	void Back(float elapsedTimer);
	void Left(float elapsedTimer);
	void Right(float elapsedTimer);
	void Up(float elapsedTimer);
	void Down(float elapsedTimer);
	void updateFront();
	void SetPitch(float pitch) { m_pitch = pitch; }

	void SetHp(float hp) { m_hp = hp; }
	void SetRAHp(float runaway_hp) { m_runaway_hp = runaway_hp; }
	float GetHp() { return m_hp; }
	float GetRAHp() { return m_runaway_hp; }

	void SetAtk(float atk) { m_atk = atk; }
	void SetDef(float def) { m_def = def; }

	void SetUserArround(int index, bool b) { isUserArround[index] = b; }
	bool GetUserArround(int index) { return isUserArround[index]; }

	void SetAnimation(CHAR animation) { m_animation = animation; }

	void SetState(CHAR state) {
		prev_state = m_state;
		m_state = state; 
		curr_state = m_state;
	}
	CHAR GetState() { return m_state; }

	void SetChoice(CHAR choice) { m_choose = choice; }
	CHAR GetChoice() { return m_choose; }

	void SetTarget(Player* player) { m_target = player; }
	Player* GetTarget() { return m_target; }

	void SetLatestAttackPlayer(Player* player) { latest_damage_player = player; }
	Player* GetLatestAttackPlayer() { return latest_damage_player; }

	void SetTargetPos(DirectX::XMFLOAT3 target_pos) { m_target_position = target_pos; }
	XMFLOAT3 GetTargetPos() { return m_target_position; }

	char GetAnimation() { return m_animation; }

	void SetFront(XMFLOAT3 front) { m_front = front; }
	XMFLOAT3 GetFront() { return m_front; }

	MONSTER_DATA GetData() { return { m_id, m_position, m_velocity, m_yaw, m_hp }; }	

	void InitMonster();

	void BuildBT(BehaviorTree node) { root = node; }
	void RunBT() { root.run(); }

	CHAR prev_state = idle_state, curr_state = idle_state;

	// [home][index], index0,2 : xmin, xmax | index 1,3 : zmin, zmax 
	float sector_line[3][4] = {
		{1762, 2385, 2492, 3411},
		{125, 2835, 1095, 3275},
		{123, 745, 715, 2082}
	};
	int home = 0;
	int prev_home = 0;
	std::mutex m_lock;
protected:
	bool isUserArround[MAX_CLIENT_ROOM];

	DirectX::XMFLOAT3 m_target_position;
	DirectX::XMFLOAT3 m_front;

	Player* m_target;
	Player* latest_damage_player;

	FLOAT currentTime = 0.f;
	FLOAT elapsedTime = 0.f;
	FLOAT prevTime = 0.f;
	FLOAT waitTime = 0.f;

	FLOAT turnning_speed;
	FLOAT move_speed;
	FLOAT fly_up_speed;

	FLOAT m_pitch;

	FLOAT m_hp;
	FLOAT m_max_hp;
	FLOAT m_runaway_hp;
	FLOAT m_atk;
	FLOAT m_def;

	CHAR m_state;
	CHAR m_animation;
	
	CHAR m_choose;

	BehaviorTree root;
};

void build_bt(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room);
void run_bt(Monster* monster, std::unordered_map<INT, Player>* players, GameRoom* room);

void SetFaceDirection(Monster* monster);
