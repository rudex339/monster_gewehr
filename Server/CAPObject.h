#pragma once

#include "stdafx.h"
#include "BehaviorTree.h"


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
	void SetRoomID(int id) { m_room_id = id; }
	void SetBoundingBox(XMFLOAT3 center) { m_bounding_box.Center = center; m_bounding_box.Center.y += 5.0f; }
	void RotateBoundingBox(XMFLOAT4& orientaion) { m_bounding_box.Orientation = orientaion; }

	DirectX::XMFLOAT3 GetPosition() { return m_position; }
	DirectX::XMFLOAT3 GetVelocity() { return m_velocity; }
	FLOAT GetYaw() { return m_yaw; }
	CHAR GetID() { return m_id; }
	CHAR GetRoomID() { return m_room_id; }
	DirectX::BoundingOrientedBox GetBoundingBox() { return m_bounding_box; }
	

protected:
	
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_velocity;
	FLOAT m_yaw;
	DirectX::BoundingOrientedBox m_bounding_box;
	CHAR m_id;
	CHAR m_room_id;
};

class Player : public CAPObject
{
public:
	Player();
	~Player() = default; ;
	Player(int id, SOCKET socket);

	void SetSocket(SOCKET& sock) { m_socket = sock; }
	void SetName(std::string name) { m_name = name; }	
	void SetHp(float hp) { m_hp = hp; }
	void SetMaxHp(float max_hp) { m_max_hp = max_hp; }
	void SetAtk(float atk) { m_atk = atk; }
	void SetDef(float def) { m_def = def; }
	void SetAmmo(float ammo) { m_ammo = ammo; }
	void SetMag(float mag) { m_mag = mag; }
	void SetWepon(char wepon) { m_wepon = wepon; }
	void SetArmor(char armor) { m_armor = armor; }
	void SetAtkByWeapon(char aromor);
	void SetRemainSize(int remain_size) { m_remain_size = remain_size; }
	void SetState(S_STATE state) { m_state = state; }
	void SetAnimaition(char ani) { m_animation = ani; }
	void SetAtkDir(const DirectX::XMFLOAT3& dir) { m_atk_dir = dir; }
	void SetAtkPos(const DirectX::XMFLOAT3& pos) { m_atk_pos = pos; }

	void Lock() { m_lock.lock(); }
	void UnLock() { m_lock.unlock(); }

	int GetRemainSize() { return m_remain_size; }
	char GetWeapon() { return m_wepon; }
	char GetAnimaition() { return m_animation; }
	S_STATE GetState() { return m_state; }
	std::string GetName() { return m_name; }
	PLAYER_DATA GetPlayerData() { return { m_id, m_position, m_velocity, m_yaw, m_hp }; }
	DirectX::XMFLOAT3 GetAtkDir() { return m_atk_dir; }
	DirectX::XMFLOAT3 GetAtkPos() { return m_atk_pos; }
	

	int RecvData();
	int DoSend(void* p, size_t size);

	void Process_Packet(char* p);

	char m_recv_buf[BUF_SIZE];
protected:
	SOCKET m_socket;
	SC_OBJECT_PACKET sc_player_data;

	ITEM_DATA items;

	std::string m_name;

	std::mutex m_lock;

	S_STATE m_state;

	FLOAT m_hp;
	FLOAT m_max_hp;
	FLOAT m_atk;
	FLOAT m_def;

	FLOAT m_ammo;	// ÇöÀç ÃÑÇÕ ÃÑ¾Ë(¼ÒÁöÁß Åº¾à)
	FLOAT m_mag;	// ÇöÀç ÅºÃ¢¿¡ µé¾îÀÖ´Â Åº¾à

	DirectX::XMFLOAT3 m_atk_dir;
	DirectX::XMFLOAT3 m_atk_pos;


	CHAR m_wepon;
	CHAR m_armor;	
	CHAR m_animation;

	INT m_remain_size;
};

class Monster : public CAPObject
{
public:
	Monster();
	~Monster() = default;

	// ÀÌµ¿ °ü·Ã
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
	void SetMaxHp(float max_hp) { m_max_hp = max_hp; }
	void SetRAHp(float runaway_hp) { m_runaway_hp = runaway_hp; }
	void SetAtk(float atk) { m_atk = atk; }
	void SetDef(float def) { m_def = def; }

	void SetUserArround(int index, bool b) { isUserArround[index] = b; }
	bool GetUserArround(int index) { return isUserArround[index]; }


	void SetAnimation(CHAR animation) { m_animation = animation; }

	void SetState(CHAR state) { m_state = state; }
	CHAR GetState() { return m_state; }

	void SetChoice(CHAR choice) { m_choose = choice; }
	CHAR GetChoice() { return m_choose; }

	void SetTarget(Player* player) { m_target = player; }
	Player* GetTarget() { return m_target; }

	void SetTargetPos(DirectX::XMFLOAT3 target_pos) { m_target_position = target_pos; }
	XMFLOAT3 GetTargetPos() { return m_target_position; }

	char GetAnimation() { return m_animation; }
	XMFLOAT3 GetFront() { return m_front; }

	MONSTER_DATA GetData() { return { m_room_id, m_position, m_velocity, m_yaw, m_hp }; }

	float GetHp() { return m_hp; }

	void BuildBT(BehaviorTree node) { root = node; }
	void RunBT() { root.run(); }

	std::mutex m_lock;
protected:
	bool isUserArround[MAX_CLIENT_ROOM];

	DirectX::XMFLOAT3 m_target_position;
	DirectX::XMFLOAT3 m_front;

	Player* m_target;

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

void build_bt(Monster* monster, std::unordered_map<INT, Player>* players);
void run_bt(Monster* monster);