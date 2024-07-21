#pragma once
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include "Object_Entity.h"

class ObjectManager;

enum {
	LOGIN,
	LOBBY,
	ROOMS,
	INROOM,
	EQUIPMENT,
	SHOP,
	GAME,
	END
};

enum {
	ExitBtn = -1,
	LoginBtn,
	RegisterBtn,
	ChangeSceneBtn,
	MakeRoomBtn,
	SelectRoomBtn,
	JoinRoomBtn,
	ItemBtn,
	BuyBtn,
	GameReadyBtn,
	GameStartBtn,
	EquipLeftBtn,
	EquipRightBtn,
	EquipUpBtn,
	EquipDownBtn
};

enum Items {
	M4,
	SAIGA,
	M24,
	L_ARMOR,
	H_ARMOR,
	GRANADE,
	FLASHBANG,
	BANDAGE,
	FAK,
	INJECTOR
};

struct ChangeScene_Event {
	UINT State;
	short score = 0;
};

struct ChoiceRoom_Event {
	int room_num = -1 ;
};

struct EnterRoom_Event {
	UINT State;
	int room_num = 0;
	BOOL is_host;
};

struct LoginCheck_Event {
	bool logincheck = false;
	int id;
};

struct Player_Info {
	wstring name;
	int weapon;

};

struct ChoiceItem_Event {
	int item_num = -1;
};

struct CreateObject_Event {
	int object = 0;

	XMFLOAT3 Position, Rotate, direct;
};
struct Refresh_Scene {
	UINT State;
};

struct ChoiceEquip_Event {
	int btnType;
	int equipType;
};

struct StartRoom_Event {
	UINT ply_id;
	int room_num;
};

struct RoomPlayer_Info {
	int id;
	wstring name;
	int weapon;
	int armor;
	bool ready = false;
	bool host = false;
};

struct Ready_Event {
};

struct LoginButton_Event {
	int len;
	int index;
};


class Scene_Sysytem :public EntitySystem,
	public EventSubscriber<ChangeScene_Event>,
	public EventSubscriber<EnterRoom_Event>,
	public EventSubscriber<LoginCheck_Event>,
	public EventSubscriber<ChoiceRoom_Event>,
	public EventSubscriber<CreateObject_Event>,
	public EventSubscriber<ChoiceItem_Event>,
	public EventSubscriber<Refresh_Scene>,
	public EventSubscriber<ChoiceEquip_Event>,
	public EventSubscriber<StartRoom_Event>,
	public EventSubscriber<Ready_Event>,
	public EventSubscriber<LoginButton_Event>
{
private:
	UINT m_State = 0;

	ObjectManager* m_pObjectManager = NULL;
	ID3D12Device* m_pd3dDevice;
	ID3D12GraphicsCommandList* m_pd3dCommandList;

	ID2D1DeviceContext2* m_d2dDeviceContext;
	ID2D1Factory3* m_d2dFactory;
	ID2D1Bitmap* m_bitmap;

	Entity* m_pPawn;

	short m_score = 0;

	int textboxlen[2] = { 0,0 };

	int m_id = -1;
	bool m_ready = false;

	// 방 목록
	vector<Button_Component> Rooms;
	queue<int> Room_ids;

	vector<Player_Info> RoomPlayers;
	unordered_set<wstring> RoomPlayerNames;

	vector<RoomPlayer_Info> InRoomPlayers;

	wstring imagefiles[10] = {
				L"image/M4.png",
				L"image/Saiga12.png",
				L"image/M24.png",
				L"image/LightArmor.png",
				L"image/HeavyArmor.png",
				L"image/Grenade.png",
				L"image/Flashbang.png",
				L"image/Bandage.png",
				L"image/FirstAidKit.png",
				L"image/Injector.png"
	};

	int m_room_num = -1;
	bool loginCheck = false;
	int m_item_num = -1;
	int m_item_info[10] = { 0, 0, 0, 0, 0, 0, 0, 10, 1, 10 };
	int m_join_room = -1;
	bool m_is_host = false;

	int money = 1000;
	int cost = 100;

	//0: 붕대, 1: FAK, 2: 주사기
	int costs[3] = { 100, 250, 500 };

	int equipments[4] = { 0, 3, 5, 7 };
	int choicedHealItem = 0;
	int haveHealItems[3];
	int equipHealItems[3] = { 0, 0, 0 };

	float roomPlayerSet[3] = { 160, 180, 190 };
	float roomPlayerAngle[3][3] = { {-5.f, -8.0f, 25.0f}, {6.f, -8.0f, 25.0f}, {17.f, -8.0f, 25.0f} };

	int weapon_ammo[3] = { 30, 12, 7 };
	int weapon_mag[3] = { 120, 60, 21 };

public:

	Scene_Sysytem() = default;
	Scene_Sysytem(ObjectManager* pObjectManager, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID2D1DeviceContext2* deviceContext, ID2D1Factory3* factory);

	virtual void configure(class World* world);
	virtual void unconfigure(class World* world);
	virtual void tick(class World* world, float deltaTime);

	virtual void receive(class World* world, const ChangeScene_Event& event);
	virtual void receive(class World* world, const EnterRoom_Event& event);
	virtual void receive(class World* world, const LoginCheck_Event& event);
	virtual void receive(class World* world, const ChoiceRoom_Event& event);
	virtual void receive(class World* world, const ChoiceItem_Event& event);
	virtual void receive(class World* world, const Refresh_Scene& event);
	virtual void receive(class World* world, const ChoiceEquip_Event& event);
	virtual void receive(class World* world, const CreateObject_Event& event);
	virtual void receive(class World* world, const StartRoom_Event& event);
	virtual void receive(class World* world, const Ready_Event& event);
	virtual void receive(class World* world, const LoginButton_Event& event);

	void BuildScene(World* world, char* pstrFileName);

	// 방 생성 함수
	void AddRoom(int room_num, bool disable);
	void DeleteRoom(int room_num);
	void AddRoomPlayers(wstring name, int weapon);
	void InitRoomPlayers();
	void initSelect();
	void AddInRoomPlayers(RoomPlayer_Info info);
	void RemoveInRoomPlayers(int id);
	void InitInRoomPlayers();
	void ReadyCheck(int id, bool ready);
	void Purchase();
};

