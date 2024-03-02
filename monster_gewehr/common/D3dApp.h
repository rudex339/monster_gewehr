
#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif


#include "GameTimer.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class D3DApp
{
protected:

    D3DApp(HINSTANCE hInstance);
    D3DApp(const D3DApp& rhs) = delete;
    D3DApp& operator=(const D3DApp& rhs) = delete;
    virtual ~D3DApp();

public:

    static D3DApp* GetApp();

    HINSTANCE AppInst()const;
    HWND      MainWnd()const;
    float     AspectRatio()const;

    bool Get4xMsaaState()const;
    void Set4xMsaaState(bool value);

    int Run();

    virtual bool Initialize();
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
    virtual void CreateRtvAndDsvDescriptorHeaps();
    virtual void OnResize();
    virtual void Update(const GameTimer& gt) = 0;
    virtual void Draw(const GameTimer& gt) = 0;

    // Convenience overrides for handling mouse input.
    virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
    virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
    virtual void OnMouseMove(WPARAM btnState, int x, int y) { }

protected:

    bool InitMainWindow();
    bool InitDirect3D();
    void CreateCommandObjects();
    void CreateSwapChain();
    void CreateDepthStencilView()
    {
        D3D12_RESOURCE_DESC d3dResourceDesc;
        d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        d3dResourceDesc.Alignment = 0;
        d3dResourceDesc.Width = mClientWidth;
        d3dResourceDesc.Height = mClientHeight;
        d3dResourceDesc.DepthOrArraySize = 1;
        d3dResourceDesc.MipLevels = 1;
        d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        d3dResourceDesc.SampleDesc.Count = (m4xMsaaState) ? 4 : 1;
        d3dResourceDesc.SampleDesc.Quality = (m4xMsaaState) ? (m4xMsaaQuality - 1) : 0;
        d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_HEAP_PROPERTIES d3dHeapProperties;
        ::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
        d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        d3dHeapProperties.CreationNodeMask = 1;
        d3dHeapProperties.VisibleNodeMask = 1;

        D3D12_CLEAR_VALUE d3dClearValue;
        d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        d3dClearValue.DepthStencil.Depth = 1.0f;
        d3dClearValue.DepthStencil.Stencil = 0;

        md3dDevice.Get()->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)mDepthStencilBuffer.Get());

        D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
        ::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
        d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

        D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = mDsvHeap.Get()->GetCPUDescriptorHandleForHeapStart();
        md3dDevice.Get()->CreateDepthStencilView(mDepthStencilBuffer.Get(), &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
    }

    void FlushCommandQueue();

    ID3D12Resource* CurrentBackBuffer()const;
    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;

    void CalculateFrameStats();

    void LogAdapters();
    void LogAdapterOutputs(IDXGIAdapter* adapter);
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

protected:

    static D3DApp* mApp;

    HINSTANCE mhAppInst = nullptr; // application instance handle=InitDirect3DApp
    HWND      mhMainWnd = nullptr; // main window handle=m_hWnd
    bool      mAppPaused = false;  // is the application paused?
    bool      mMinimized = false;  // is the application minimized?
    bool      mMaximized = false;  // is the application maximized?
    bool      mResizing = false;   // are the resize bars being dragged?
    bool      mFullscreenState = false;// fullscreen enabled

    // Set true to use 4X MSAA (?.1.8).  The default is false.
    bool      m4xMsaaState = false;    // 4X MSAA enabled=m_bMsaa4xEnable
    UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA=m_nMsaa4xQualityLevels

    // Used to keep track of the “delta-time?and game time (?.4).
    GameTimer mTimer;//=m_GameTimer

    Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;//=m_pdxgiFactory
    Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;//=m_pdxgiSwapChain
    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;//m_pd3dDevice

    Microsoft::WRL::ComPtr<ID3D12Fence> mFence;//m_pd3dFence
    UINT64 mCurrentFence = 0;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;//m_pd3dCommandQueue
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;//m_pd3dCommandAllocator
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;//m_pd3dCommandList

    static const int SwapChainBufferCount = 2;
    int mCurrBackBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];//m_ppd3dSwapChainBackBuffers
    Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;//m_pd3dDepthStencilBuffer

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;//m_pd3dRtvDescriptorHeap
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;//m_pd3dDsvDescriptorHeap

    D3D12_VIEWPORT mScreenViewport;
    D3D12_RECT mScissorRect;

    UINT mRtvDescriptorSize = 0;
    UINT mDsvDescriptorSize = 0;
    UINT mCbvSrvUavDescriptorSize = 0;

    // Derived class should set these in derived constructor to customize starting values.
    std::wstring mMainWndCaption = L"Game App";
    D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    int mClientWidth = 800;//
    int mClientHeight = 600;//
};

