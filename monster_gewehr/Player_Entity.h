#pragma once
class CLoadedModelInfo;
class ObjectManager;

enum Player_State {
	IDLE,
	RUN,
	SHOOT,
	HEAL,
	HIT,
	RELOAD,
	AIM,
	GRANADE,
	FASTRUN,
	ROLLING
};

enum OM_Model {
	MONSTER,
	SOLDIER
};

enum Weapon_State {
	RIFLE,
	SHOTGUN,
	SNIPERRIFLE
};

enum Armor_State {
	LIGHT_ARMOR,
	HEAVY_ARMOR
};

Entity* AddPlayerEntity(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	ObjectManager* OM,
	float x, float y, float z,
	float rx, float ry, float rz,
	float sx, float sy, float sz,
	UINT model);

Entity* AddAnotherEntity(Entity* ent, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	ObjectManager* OM,
	float x, float y, float z,
	float rx, float ry, float rz,
	float sx, float sy, float sz,
	UINT model);

struct player_Component {

	INT id = -1;
	FLOAT hp = 100;
	FLOAT max_hp = 100;

	FLOAT stamina = 100;

	INT ammo;
	INT mag;

	BOOL aim_mode = false;

	BOOL reload = false;
	FLOAT reload_coolTime = 3.5f;

	UINT m_weapon = RIFLE;
	UINT m_armor = LIGHT_ARMOR;

	INT heal_item[3];

	FLOAT heal_timer;
	FLOAT heal_all_time;

	// 보급받는 시간에 관여하는 변수
	FLOAT supply_timer;
	FLOAT supply_time = 3.5f;

	// 보급 위치 근처에 있는를 체크
	bool near_supply = false;
	bool is_suppling = false;

	// 보급 가능 시간을 체크(0이하면 보급 가능)
	FLOAT can_supply = 0.0f;

	INT grenade_type;
	INT grenade_amount = 1;

	XMFLOAT3 m_velocity = XMFLOAT3(0, 0, 0);

	player_Component() = default;
};



struct ControllAngle_Component {
	XMFLOAT3 m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3 m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	ControllAngle_Component() = default;
	ControllAngle_Component(float R_x, float R_y, float R_z,
		float L_x, float L_y, float L_z,
		float U_x, float U_y, float U_z);
};