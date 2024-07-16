#pragma once
#include <queue>
#include <unordered_set>
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
	GameStartBtn,
	ShopBtn,
	EquipBtn,
	MakeRoomBtn,
	RoomBtn,
	JoinRoomBtn,
	ItemBtn,
	BuyBtn,
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
};

struct LoginCheck_Event {
	bool logincheck = false;
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

	XMFLOAT3 Position, Rotate;
};



class Scene_Sysytem :public EntitySystem,
	public EventSubscriber<ChangeScene_Event>,
	public EventSubscriber<EnterRoom_Event>,
	public EventSubscriber<LoginCheck_Event>,
	public EventSubscriber<ChoiceRoom_Event>,
	public EventSubscriber<ChoiceItem_Event>,
	public EventSubscriber<CreateObject_Event>
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

	// 规 格废
	vector<Button_Component> Rooms;
	queue<int> Room_ids;

	vector<Player_Info> RoomPlayers;
	unordered_set<wstring> RoomPlayerNames;

	int m_room_num = -1;
	bool loginCheck = false;
	int m_item_num = -1;
	int m_item_info[10];

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
	virtual void receive(class World* world, const CreateObject_Event& event);
	
	void BuildScene(World* world, char* pstrFileName);

	// 规 积己 窃荐
	void AddRoom(int room_num);
	void AddRoomPlayers(wstring name, int weapon);
	void InitRoomPlayers();
	void initSelect();
};

