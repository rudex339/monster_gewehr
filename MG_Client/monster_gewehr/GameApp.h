#pragma once
#include "common/d3dApp.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace ECS;




class GameApp : public D3DApp
{
public:
    GameApp(HINSTANCE hInstance);
    GameApp(const GameApp& rhs) = delete;
    GameApp& operator=(const GameApp& rhs) = delete;
    ~GameApp();

    virtual bool Initialize()override;

private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;

    void BuildObjects();

protected:
    ECS::World* world = nullptr;

};

