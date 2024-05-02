#pragma once



#include "Timer.h"
#include "Player.h"
#include "ObjectManager.h"

class GameFramework
{
public:
	GameFramework();
	~GameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateD3D11On12Device();
	void CreateD2DDevice();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

    void BuildObjects();
    void ReleaseObjects();

    void ProcessInput();
    void AnimateObjects();
    void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void InitServer();

private:
	HINSTANCE					m_hInstance;
	HWND						m_hWnd; 

	int							m_nWndClientWidth;
	int							m_nWndClientHeight;
        
	IDXGIFactory4				*m_pdxgiFactory = NULL;
	IDXGISwapChain3				*m_pdxgiSwapChain = NULL;
	ComPtr<ID3D12Device>				m_pd3dDevice;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	static const UINT			m_nSwapChainBuffers = 2;
	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource				*m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap		*m_pd3dRtvDescriptorHeap = NULL;

	ID3D12Resource				*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap		*m_pd3dDsvDescriptorHeap = NULL;

	ComPtr<ID3D12CommandAllocator>		m_pd3dCommandAllocator = NULL;
	ComPtr <ID3D12CommandQueue>		m_pd3dCommandQueue;
	ID3D12GraphicsCommandList	*m_pd3dCommandList = NULL;

	ID3D12Fence					*m_pd3dFence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;

	// Direct3D11on12
	ComPtr<ID3D11On12Device> m_d3d11On12Device;
	ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
	ComPtr<ID3D11Resource> m_wrappedBackBuffers[m_nSwapChainBuffers];

	// Direct2D
	ComPtr<ID2D1Factory3> m_d2dFactory;
	ComPtr<ID2D1Device2> m_d2dDevice;
	ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext;
	ComPtr<ID2D1Bitmap1> m_d2dRenderTargets[m_nSwapChainBuffers];

	// DirectWrite
	ComPtr<IDWriteFactory5>	m_dwriteFactory;


#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;

	World						*m_pWorld = NULL;

	ObjectManager				*m_pObjectManager = NULL;

	Entity						*m_pPlayer = NULL;

	CCamera						*m_pCamera = NULL;

	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[70];
};

