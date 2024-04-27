//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "GameFramework.h"
#include "Object_Entity.h"
#include "Player_Entity.h"
//---------------------------------//
#include "Render_Sysytem.h"
#include "PlayerControl_System.h"
#include "move_System.h"
#include "Sever_Sysyem.h"
#include "Animate_System.h"
//---------------------------------//

CGameFramework::CGameFramework()
{
	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;

	for (int i = 0; i < m_nSwapChainBuffers; i++) m_ppd3dSwapChainBackBuffers[i] = NULL;
	m_nSwapChainBufferIndex = 0;

	m_pd3dCommandList = NULL;

	m_pd3dRtvDescriptorHeap = NULL;
	m_pd3dDsvDescriptorHeap = NULL;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pObjectManager = NULL;
	m_pPlayer = NULL;

	m_pWorld = World::createWorld();

	_tcscpy_s(m_pszFrameRate, _T("MonsterGewehr ( "));
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();

	CreateD3D11On12Device();
	CreateD2DDevice();

	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

	CoInitialize(NULL);

	BuildObjects();

	return(true);
}

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1 **)&m_pdxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue.Get(), &dxgiSwapChainDesc, (IDXGISwapChain**)&m_pdxgiSwapChain);
#endif
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug *pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void **)&m_pdxgiFactory);

	IDXGIAdapter1 *pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)m_pd3dDevice.GetAddressOf()))) break;
	}

	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void **)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)m_pd3dDevice.GetAddressOf());
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pd3dDevice.Get()->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_pd3dDevice.Get()->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **)&m_pd3dFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvDescriptorIncrementSize = m_pd3dDevice.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	HRESULT hResult;

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_pd3dDevice.Get()->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void **)&m_pd3dCommandQueue);

	hResult = m_pd3dDevice.Get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&m_pd3dCommandAllocator);

	hResult = m_pd3dDevice.Get()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator.Get(), NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pd3dCommandList);
	hResult = m_pd3dCommandList->Close();
}

void CGameFramework::CreateD3D11On12Device()
{
	HRESULT hresuld;
	ComPtr<ID3D11Device> d3d11Device;
	hresuld = D3D11On12CreateDevice(
		m_pd3dDevice.Get(),
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		nullptr,
		0,
		reinterpret_cast<IUnknown**>(m_pd3dCommandQueue.GetAddressOf()),
		1,
		0,
		&d3d11Device,
		&m_d3d11DeviceContext,
		nullptr
	);

	// Query the 11On12 device from the 11 device.
	d3d11Device.As(&m_d3d11On12Device);
}

void CGameFramework::CreateD2DDevice()
{
	D2D1_FACTORY_OPTIONS d2dFactoryOptions{};
	D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &m_d2dFactory);
	ComPtr<IDXGIDevice> dxgiDevice;
	m_d3d11On12Device.As(&dxgiDevice);
	m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice);
	m_d2dDevice->CreateDeviceContext(deviceOptions, &m_d2dDeviceContext);
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dwriteFactory);
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	float dpiX;
	float dpiY;
#pragma warning(push)
#pragma warning(disable : 4996) // GetDesktopDpi is deprecated.
	m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
#pragma warning(pop)

	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		dpiX,
		dpiY
	);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void **)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);

		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		m_d3d11On12Device->CreateWrappedResource(
			m_ppd3dSwapChainBackBuffers[i],
			&d3d11Flags,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
			IID_PPV_ARGS(&m_wrappedBackBuffers[i])
		);

		ComPtr<IDXGISurface> surface;
		m_wrappedBackBuffers[i].As(&surface);
		m_d2dDeviceContext->CreateBitmapFromDxgiSurface(
			surface.Get(),
			&bitmapProperties,
			&m_d2dRenderTargets[i]
		);

		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;

		m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pd3dCommandAllocator));
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
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

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
		if (m_pWorld) {
			m_pWorld->emit< CaptureHWND_Event>({ true });
			m_pWorld->emit<CursorPos_Event>({ m_ptOldCursorPos });
		}
		break;
	case WM_LBUTTONUP:
		break;
	case WM_RBUTTONUP:
		::ReleaseCapture();
		if (m_pWorld)m_pWorld->emit< CaptureHWND_Event>({ false });
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pObjectManager) m_pObjectManager->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					::PostQuitMessage(0);
					break;
				case VK_RETURN:
					break;
				case VK_F1:
				case VK_F2:
				case VK_F3:
					//m_pCamera = m_pPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
					break;
				case VK_F9:
					ChangeSwapChainState();
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
				m_GameTimer.Stop();
			else
				m_GameTimer.Start();
			break;
		}
		case WM_SIZE:
			break;
		case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:
			OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
            break;
        case WM_KEYDOWN:
        case WM_KEYUP:
			OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
			break;
	}
	return(0);
}

void CGameFramework::OnDestroy()
{
    ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	m_pWorld->destroyWorld();

	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();

	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
    if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();

#if defined(_DEBUG)
	IDXGIDebug1	*pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void **)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

#define _WITH_TERRAIN_PLAYER

void CGameFramework::BuildObjects()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL);

	m_pObjectManager = new ObjectManager();
	if (m_pObjectManager) m_pObjectManager->BuildObjects(m_pd3dDevice.Get(), m_pd3dCommandList);


	Entity* ent = m_pWorld->create();
	ent->assign<SkyBox_Component>(m_pObjectManager->m_pSkyBox, "default");

	BuildScene((char*)"Scene/Scene.bin");


	ent = AddAnotherEntity(m_pWorld->create(), m_pd3dDevice.Get(), m_pd3dCommandList,
		m_pObjectManager->Get_ModelInfo("Souleater"),
		1014.f, m_pObjectManager->m_pTerrain->GetHeight(1014.f, 1429.f), 1429.0f,
		0.f, 90.f, 0.f,
		20.f, 20.f, 20.f,
		2);
	auto monster_id = ent->get<player_Component>();
	monster_id->id = -2;
	


	m_pPlayer = AddPlayerEntity(m_pWorld->create(), m_pd3dDevice.Get(), m_pd3dCommandList,
		m_pObjectManager->Get_ModelInfo("Soldier"),
		1014.f, m_pObjectManager->m_pTerrain->GetHeight(1014.f, 1429.f)/*2000.f*/, 1429.0f,
		0.f, 0.f, 0.f,
		6.0f, 6.0f, 6.0f,
		3);
	m_pPlayer->assign<ControllAngle_Component>();


	AddAnotherEntity(m_pWorld->create(), m_pd3dDevice.Get(), m_pd3dCommandList,
		m_pObjectManager->Get_ModelInfo("Soldier"),
		310.0f, m_pObjectManager->m_pTerrain->GetHeight(310.0f, 600.0f), 600.0f,
		0.f, 0.f, 0.f,
		6.0f, 6.0f, 6.0f,
		3);

	AddAnotherEntity(m_pWorld->create(), m_pd3dDevice.Get(), m_pd3dCommandList,
		m_pObjectManager->Get_ModelInfo("Soldier"),
		310.0f, m_pObjectManager->m_pTerrain->GetHeight(310.0f, 600.0f), 600.0f,
		0.f, 0.f, 0.f,
		6.0f, 6.0f, 6.0f,
		3);
	AddAnotherEntity(m_pWorld->create(), m_pd3dDevice.Get(), m_pd3dCommandList,
		m_pObjectManager->Get_ModelInfo("Soldier"),
		310.0f, m_pObjectManager->m_pTerrain->GetHeight(310.0f, 600.0f), 600.0f,
		0.f, 0.f, 0.f,
		6.0f, 6.0f, 6.0f,
		3);


	CCamera* temp = new CThirdPersonCamera(m_pCamera);	
	temp->CreateShaderVariables(m_pd3dDevice.Get(), m_pd3dCommandList);
	ComponentHandle<Camera_Component> camera = m_pPlayer->assign<Camera_Component>(temp);
	camera->m_pCamera->SetPosition(XMFLOAT3(310.0f, 
		m_pObjectManager->m_pTerrain->GetHeight(310.0f, 600.0f)+10.f, 600.0f - 30.f));



	//set System
	m_pWorld->registerSystem(new PlayerControl_System());
	m_pWorld->registerSystem(new Move_System());
	m_pWorld->registerSystem(new Sever_System());
	m_pWorld->registerSystem(new Animate_System());
	m_pWorld->registerSystem(new Render_Sysytem(m_pObjectManager, m_pd3dCommandList));


	m_pWorld->emit<GetPlayerPtr_Event>({ m_pPlayer });


	m_pd3dCommandList->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pObjectManager) m_pObjectManager->ReleaseUploadBuffers();

	m_GameTimer.Reset();
#ifdef USE_NETWORK
	InitServer();
#endif
}

void CGameFramework::BuildScene(char* pstrFileName)
{
	FILE* pFile = NULL;
	::fopen_s(&pFile, pstrFileName, "rb");
	::rewind(pFile);

	char pstrToken[256] = { '\0' };
	char pstrGameObjectName[256] = { '\0' };
	UINT nReads = 0;
	BYTE nStrLength = 0, nObjectNameLength = 0;
	int	m_nObjects = 0;
	::ReadStringFromFile(pFile, pstrToken); //"<GameObjects>:"
	nReads = (UINT)::fread(&m_nObjects, sizeof(int), 1, pFile);

	for (int i = 0; i < m_nObjects; i++)
	{
		::ReadStringFromFile(pFile, pstrToken); //"<GameObject>:"
		if (!strcmp(pstrToken, "<GameObject>:")) {
			Entity* ent = m_pWorld->create();
			::ReadStringFromFile(pFile, pstrGameObjectName);

			if (!strcmp(pstrGameObjectName, "Cube.001")) {
				ent->assign<Terrain_Component>(m_pObjectManager->m_pTerrain, "default");
			}
		

			XMFLOAT4X4	xmf4x4World;
			nReads = (UINT)::fread(&xmf4x4World, sizeof(float), 16, pFile); //Transform
			
			
			xmf4x4World = Matrix4x4::Multiply(XMMatrixScaling(
				(4104 / 330),
				(4104 / 330),
				(4104 / 330)), xmf4x4World);
			
			

			xmf4x4World._41 *= (4104 / 330);
			xmf4x4World._42 *= (4104 / 330);
			xmf4x4World._43 *= (4104 / 330);
			xmf4x4World = Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(0.f),
				XMConvertToRadians(0.f),
				XMConvertToRadians(0.f)), xmf4x4World);	

			XMVECTOR translation = XMVectorSet(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43,1.0f);
			translation = XMVector3Transform(translation, XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(0.f),
				XMConvertToRadians(180.f),
				XMConvertToRadians(0.f)));

			xmf4x4World._41 = XMVectorGetX(translation);
			xmf4x4World._42 = XMVectorGetY(translation);
			xmf4x4World._43 = XMVectorGetZ(translation);

			xmf4x4World._41 += 2822.f;
			xmf4x4World._42 += 1024.f;
			xmf4x4World._43 += 1900.f;

			ent->assign<Position_Component>(xmf4x4World);


			ent->assign<Model_Component>(m_pObjectManager->Get_ModelInfo(pstrGameObjectName),
				m_pObjectManager->Get_ModelInfo(pstrGameObjectName)->m_pModelRootObject->m_pstrFrameName);
		}
	}
	::fclose(pFile);
}

void CGameFramework::ReleaseObjects()
{

	if (m_pObjectManager) m_pObjectManager->ReleaseObjects();
	if (m_pObjectManager) delete m_pObjectManager;
}

void CGameFramework::ProcessInput()
{
}

void CGameFramework::AnimateObjects()
{
}

void CGameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

//#define _WITH_PLAYER_TOP

void CGameFramework::FrameAdvance()
{    
	m_GameTimer.Tick(120.0f);
	
	ProcessInput();

    AnimateObjects();

	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor/*Colors::Azure*/, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	//render system
	m_pWorld->tick(m_GameTimer.GetTimeElapsed());
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
	
	hResult = m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	WaitForGpuComplete();


	D2D1_SIZE_F rtSize = m_d2dRenderTargets[m_nSwapChainBufferIndex]->GetSize();
	D2D1_RECT_F textRect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);
	static const WCHAR text[] = L"11On12";

	// Acquire our wrapped render target resource for the current back buffer.
	//m_d3d11On12Device->AcquireWrappedResources(m_wrappedBackBuffers[m_nSwapChainBufferIndex].GetAddressOf(), 1);

	// Render text directly to the back buffer.
	//m_d2dDeviceContext->SetTarget(m_d2dRenderTargets[m_nSwapChainBufferIndex].Get());
	/*m_d2dDeviceContext->BeginDraw();
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
	m_d2dDeviceContext->DrawTextW(
		text,
		_countof(text) - 1,
		m_textFormat.Get(),
		&textRect,
		m_textBrush.Get()
	);*/
	//m_d2dDeviceContext->EndDraw();

	// Release our wrapped render target resource. Releasing 
	// transitions the back buffer resource to the state specified
	// as the OutState when the wrapped resource was created.
	//m_d3d11On12Device->ReleaseWrappedResources(m_wrappedBackBuffers[m_nSwapChainBufferIndex].GetAddressOf(), 1);

	// Flush to submit the 11 command list to the shared command queue.
	//m_d3d11DeviceContext->Flush();

	
	

#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	m_pdxgiSwapChain->Present(0, 0);
#endif
#endif

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	size_t nLength = _tcslen(m_pszFrameRate);
	_stprintf_s(m_pszFrameRate + nLength, 70 - nLength, _T("(DEBUG)"));
	::SetWindowText(m_hWnd, m_pszFrameRate);
}

void CGameFramework::InitServer()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	g_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);

	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP.c_str(), &server_addr.sin_addr);

	connect(g_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	
	unsigned long noblock = 1;
	ioctlsocket(g_socket, FIONBIO, &noblock);

	CS_LOGIN_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_LOGIN;
	int weapon = 0;
	cout << "이름 입력" << endl;
	cin >> packet.name;
	cout << "무기 입력" << endl;
	cin >> weapon;
	packet.weapon = weapon;
	packet.pos = m_pPlayer->get<Position_Component>()->Position;
	packet.yaw = m_pPlayer->get<Rotation_Component>()->mfYaw;
	packet.vel = m_pPlayer->get<Velocity_Component>()->m_velocity;


	int retval = send(g_socket, (char*)&packet, sizeof(packet), 0);

	SC_LOGIN_INFO_PACKET sub_packet;
	while (1) {
		int retval = recv(g_socket, (char*)&sub_packet, sizeof(sub_packet), 0);
		if (retval > 0) {
			if (retval != sizeof(SC_LOGIN_INFO_PACKET)) {
				cout << "ㅈ됬어..." << endl;
			}
			
			break;
		}
		else {
			cout << "못받음" << endl;
		}
	}
	//cout << (int)ply.id << endl;

	ComponentHandle<player_Component> Data = m_pPlayer->get<player_Component>();
	Data->id = (int)sub_packet.id;
	std::cout << "성공했음 일단" << (int)sub_packet.id << std::endl;
	//cout << Data->id << endl;
}