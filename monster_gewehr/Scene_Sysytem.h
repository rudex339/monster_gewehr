#pragma once

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

struct ChangeScene_Event {
	UINT State;
	short score = 0;
};





class Scene_Sysytem :public EntitySystem,
	public EventSubscriber<ChangeScene_Event>
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
public:
	Scene_Sysytem() = default;
	Scene_Sysytem(ObjectManager* pObjectManager, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID2D1DeviceContext2* deviceContext, ID2D1Factory3* factory);

	virtual void configure(class World* world);
	virtual void unconfigure(class World* world);
	virtual void tick(class World* world, float deltaTime);

	virtual void receive(class World* world, const ChangeScene_Event& event);
	

	void BuildScene(World* world, char* pstrFileName);
};

