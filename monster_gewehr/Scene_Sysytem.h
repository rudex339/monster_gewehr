#pragma once

class ObjectManager;



struct ChangeScene_Event {
	UINT State;
};

class Scene_Sysytem :public EntitySystem
{
private:
	UINT m_State = 0;

	ObjectManager* m_pObjectManager = NULL;
	ID3D12Device* m_pd3dDevice;
	ID3D12GraphicsCommandList* m_pd3dCommandList;

	Entity* m_pPawn;
public:
	Scene_Sysytem() = default;
	Scene_Sysytem(ObjectManager* pObjectManager, ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void configure(class World* world);
	virtual void unconfigure(class World* world);
	virtual void tick(class World* world, float deltaTime);

	virtual void changaeScene(class World* world, UINT state);

	void BuildScene(World* world, char* pstrFileName);
};

