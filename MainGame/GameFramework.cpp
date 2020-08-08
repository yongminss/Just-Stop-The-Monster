#include "stdafx.h"
#include "GameFramework.h"

enum {
	TitleState,
	GameState
};

GameFramework::GameFramework()
{
	// 클래스 멤버 변수 초기화
	m_TitleScene = NULL;

	m_Factory = NULL;
	m_SwapChain = NULL;
	m_Device = NULL;

	for (int i = 0; i < m_nSwapChainBuffer; ++i)
		m_SwapChainBackBuffer[i] = NULL;

	m_CommandAllocator = NULL;
	m_CommandQueue = NULL;
	m_CommandList = NULL;

	m_RtvDescriptorHeap = NULL;
	m_DsvDescriptorHeap = NULL;

	m_nRtvDescriptorIncrementSize = 0;
	m_nDsvDescriptorIncrementSize = 0;

	m_FenceEvent = NULL;
	m_Fence = NULL;
	for (int i = 0; i < m_nSwapChainBuffer; ++i)
		m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	_tcscpy_s(m_FrameRate, _T("Just Stop The Monster ("));

}


GameFramework::~GameFramework()
{

}

bool GameFramework::OnCreate(HINSTANCE hInstance, HWND MainHwnd)
{
	m_hInstance = hInstance;
	m_hwnd = MainHwnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

	BuildObject();

	/*network_manager *net_manager = new network_manager;
	network_manager::GetInst()->test_connect(MainHwnd);*/
	return true;
}

void GameFramework::OnDestroy()
{
	ReleaseObject();

	::CloseHandle(m_FenceEvent);

	if (m_DepthStencilBuffer) m_DepthStencilBuffer->Release();
	if (m_DsvDescriptorHeap) m_DsvDescriptorHeap->Release();

	for (int i = 0; i < m_nSwapChainBuffer; ++i)
		if (m_SwapChainBackBuffer[i]) m_SwapChainBackBuffer[i]->Release();
	if (m_RtvDescriptorHeap) m_RtvDescriptorHeap->Release();

	if (m_CommandAllocator) m_CommandAllocator->Release();
	if (m_CommandQueue) m_CommandQueue->Release();
	if (m_CommandList) m_CommandList->Release();

	if (m_Fence) m_Fence->Release();

	if (m_SwapChain) m_SwapChain->Release();
	if (m_Device) m_Device->Release();
	if (m_Factory) m_Factory->Release();

}

void GameFramework::CreateSwapChain()
{
	RECT Client;
	::GetClientRect(m_hwnd, &Client);
	m_nWndClientWidth = Client.right - Client.left;
	m_nWndClientHeight = Client.bottom - Client.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	::ZeroMemory(&SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	SwapChainDesc.Width = m_nWndClientWidth;
	SwapChainDesc.Height = m_nWndClientHeight;
	SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	SwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = m_nSwapChainBuffer;
	SwapChainDesc.Scaling = DXGI_SCALING_NONE;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC SwapChainFullScreenDesc;
	::ZeroMemory(&SwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	SwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	SwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	SwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_Factory->CreateSwapChainForHwnd(m_CommandQueue, m_hwnd, &SwapChainDesc, &SwapChainFullScreenDesc, NULL, (IDXGISwapChain1 **)&m_SwapChain);

#else
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	::ZeroMemory(&SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	SwapChainDesc.BufferCount = m_nSwapChainBuffer;
	SwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	SwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.OutputWindow = m_hwnd;
	SwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	SwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	SwapChainDesc.Windowed = TRUE;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_Factory->CreateSwapChain(m_CommandQueue, &SwapChainDesc, (IDXGISwapChain **)&m_SwapChain);

#endif
	m_nSwapChainBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	hResult = m_Factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetView();
#endif

}

void GameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nFactoryFlags = 0;

#if defined(_DEBUG)
	ID3D12Debug *DebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&DebugController);

	if (DebugController) {
		DebugController->EnableDebugLayer();
		DebugController->Release();
	}

#endif
	// DXGI 팩토리 생성
	hResult = ::CreateDXGIFactory2(nFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_Factory);

	IDXGIAdapter1 *Adapter = NULL;

	// 하드웨어 어댑터에 대해 특성 레벨 12.0을 지원하는 디바이스를 생성
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_Factory->EnumAdapters1(i, &Adapter); ++i) {

		DXGI_ADAPTER_DESC1 AdapterDesc;

		Adapter->GetDesc1(&AdapterDesc);
		if (AdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(Adapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_Device))) break;
	}
	// 특성 레벨 12.0을 지원하는 하드웨어 디바이스를 생성할 수 없으면 WARP 디바이스를 생성
	if (!Adapter) {
		m_Factory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void**)&Adapter);
		hResult = D3D12CreateDevice(Adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void**)&m_Device);
	}

	// 디바이스가 지원하는 다중 샘플의 품질 수준을 확인
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MsaaQualityLevels;
	MsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	MsaaQualityLevels.SampleCount = 4;
	MsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	MsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &MsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = MsaaQualityLevels.NumQualityLevels;
	// 다중 샘플의 품질 수준이 1보다 크면 다중 샘플링 활성화
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? TRUE : FALSE;

	// 펜스를 생성
	hResult = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_Fence);
	for (UINT i = 0; i < m_nSwapChainBuffer; ++i)
		m_nFenceValues[i] = 0;

	m_FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::nCbvSrvDescriptorIncrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	if (Adapter) Adapter->Release();

}

void GameFramework::CreateCommandQueueAndList()
{
	HRESULT hResult;

	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc;
	::ZeroMemory(&CommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	hResult = m_Device->CreateCommandQueue(&CommandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)&m_CommandQueue);
	hResult = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_CommandAllocator);
	hResult = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_CommandList);

	hResult = m_CommandList->Close();

}

void GameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc;
	::ZeroMemory(&DescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	DescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffer;
	DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_Device->CreateDescriptorHeap(&DescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_RtvDescriptorHeap);
	m_nRtvDescriptorIncrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	DescriptorHeapDesc.NumDescriptors = 1;
	DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_Device->CreateDescriptorHeap(&DescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_DsvDescriptorHeap);
	m_nDsvDescriptorIncrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

}

void GameFramework::CreateRenderTargetView()
{
	D3D12_CPU_DESCRIPTOR_HANDLE RtvCPUDescriptorHandle = m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < m_nSwapChainBuffer; ++i) {
		m_SwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_SwapChainBackBuffer[i]);
		m_Device->CreateRenderTargetView(m_SwapChainBackBuffer[i], NULL, RtvCPUDescriptorHandle);
		RtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}

}

void GameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC ResourceDesc;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	ResourceDesc.Alignment = 0;
	ResourceDesc.Width = m_nWndClientWidth;
	ResourceDesc.Height = m_nWndClientHeight;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	ResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES HeapProperties;
	::ZeroMemory(&HeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProperties.CreationNodeMask = 1;
	HeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ClearValue.DepthStencil.Depth = 1.f;
	ClearValue.DepthStencil.Stencil = 0;

	m_Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &ClearValue, __uuidof(ID3D12Resource), (void **)&m_DepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc;
	::ZeroMemory(&DepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	DepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE DsvCPUDescriptorHandle = m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_Device->CreateDepthStencilView(m_DepthStencilBuffer, &DepthStencilViewDesc, DsvCPUDescriptorHandle);

}

void GameFramework::BuildObject()
{
	m_CommandList->Reset(m_CommandAllocator, NULL);

	m_TitleScene = new TitleScene();
	m_TitleScene->BuildObject(m_Device, m_CommandList);

	m_Timer.Reset();
}

void GameFramework::ReleaseObject()
{

}

void GameFramework::Animate()
{
	float ElapsedTime = m_Timer.GetTimeElapsed();
	
	if (m_GameScene) m_GameScene->ProcessInput(m_hwnd);
	if (m_GameScene) m_GameScene->Animate(ElapsedTime);
}


void GameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_CommandQueue->Signal(m_Fence, nFenceValue);

	if (m_Fence->GetCompletedValue() < nFenceValue) {
		hResult = m_Fence->SetEventOnCompletion(nFenceValue, m_FenceEvent);
		::WaitForSingleObject(m_FenceEvent, INFINITE);
	}

}

void GameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	UINT nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_CommandQueue->Signal(m_Fence, nFenceValue);

	if (m_Fence->GetCompletedValue() < nFenceValue) {
		hResult = m_Fence->SetEventOnCompletion(nFenceValue, m_FenceEvent);
		::WaitForSingleObject(m_FenceEvent, INFINITE);
	}
}

void GameFramework::FrameAdvance()
{
	chrono::system_clock::time_point start = chrono::system_clock::now();

	m_Timer.Tick(0.f);

	Animate();

	HRESULT hResult = m_CommandAllocator->Reset();
	hResult = m_CommandList->Reset(m_CommandAllocator, NULL);

	D3D12_RESOURCE_BARRIER ResourceBarrier;
	::ZeroMemory(&ResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	ResourceBarrier.Transition.pResource = m_SwapChainBackBuffer[m_nSwapChainBufferIndex];
	ResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	ResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_CommandList->ResourceBarrier(1, &ResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE RtvCPUDescriptorHandle = m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	RtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);

	float ClearColor[4] = { 0.2f, 0.2f, 0.2f, 1.f };
	m_CommandList->ClearRenderTargetView(RtvCPUDescriptorHandle, ClearColor, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE DsvCPUDescriptorHandle = m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_CommandList->ClearDepthStencilView(DsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_CommandList->OMSetRenderTargets(1, &RtvCPUDescriptorHandle, TRUE, &DsvCPUDescriptorHandle);

	if (m_TitleScene) {
		bool GameStart = m_TitleScene->IsStartGame();
		if (true == GameStart) {
			SceneState = GameState;
			m_GameStage = m_TitleScene->GetGameStage();
		}
	}

	// Scene을 Rendering 하는 영역
	switch (SceneState)
	{
	case TitleState: // 게임이 시작되기 전
		if (m_GameScene) {
			delete m_GameScene;
			m_GameScene = NULL;
			if (m_TitleScene == NULL) {
				m_TitleScene = new TitleScene();
				m_TitleScene->BuildObject(m_Device, m_CommandList);
			}
		}
		m_TitleScene->Render(m_CommandList);
		break;

	case GameState: // 게임이 시작된 후
		if (m_TitleScene) {
			delete m_TitleScene;
			m_TitleScene = NULL;
			if (m_GameScene == NULL) {
				m_GameScene = new GameScene();
				m_GameScene->SetStageNum(m_GameStage);
				m_GameScene->BuildObject(m_Device, m_CommandList);
				m_Timer.Reset();
				network_manager::GetInst()->send_change_state_packet(PLAYER_STATE_playing_game, m_GameStage);
			}
		}
		// 여기서 패킷을 보냄
		if (m_CheckTime > 0.050) {
			m_CheckTime = 0;
			//network_manager::GetInst()->send_change_state_packet(PLAYER_STATE_playing_game);
			network_manager::GetInst()->send_my_world_pos_packet(m_GameScene->GetPlayerInfo(), m_GameScene->GetPlayerAnimate());
		}
		m_GameScene->Render(m_CommandList);
		break;

	default:
		break;
	}

	ResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	ResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_CommandList->ResourceBarrier(1, &ResourceBarrier);

	hResult = m_CommandList->Close();

	ID3D12CommandList *CommandLists[] = { m_CommandList };
	m_CommandQueue->ExecuteCommandLists(1, CommandLists);

	WaitForGpuComplete();

#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS PresentParameters;
	PresentParameters.DirtyRectsCount = 0;
	PresentParameters.pDirtyRects = NULL;
	PresentParameters.pScrollRect = NULL;
	PresentParameters.pScrollOffset = NULL;
	m_SwapChain->Present1(1, 0, &PresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_SwapChain->Present(1, 0);
#else
	m_SwapChain->Present(0, 0);
#endif
#endif

	MoveToNextFrame();

	m_Timer.GetFrameRate(m_FrameRate + 23, 37);
	size_t Length = _tcslen(m_FrameRate);

	_stprintf_s(m_FrameRate + Length, 70 - Length, _T(" "));
	::SetWindowText(m_hwnd, m_FrameRate);

	chrono::duration<double> sec = chrono::system_clock::now() - start;

	m_CheckTime += sec.count();
}

void GameFramework::OnProcessingMouseMessage(HWND hwnd, UINT MessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_TitleScene)
		m_TitleScene->OnProcessingMouseMessage(hwnd, MessageID, wParam, lParam);

	if (m_GameScene)
		m_GameScene->OnProcessingMouseMessage(hwnd, MessageID, wParam, lParam);

	switch (MessageID) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		break;

	case WM_MOUSEMOVE:
		break;

	default:
		break;
	}

}

void GameFramework::OnProcessingKeyboardMessage(HWND hwnd, UINT MessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_GameScene)
		m_GameScene->OnProcessingKeyboardMessage(hwnd, MessageID, wParam, lParam);

	switch (MessageID) {
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_RETURN:
			/*SceneState = ++SceneState % 2;
			std::cout << SceneState << std::endl;*/
			break;

		default:
			break;
		}
		break;

	case WM_KEYUP:
		switch (wParam) {
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}

LRESULT CALLBACK GameFramework::OnProcessingWindowMessage(HWND hwnd, UINT MessageID, WPARAM wParam, LPARAM lParam)
{
	switch (MessageID) {
	case WM_ACTIVATE:
		break;

	case WM_SIZE:
		m_nWndClientWidth = LOWORD(lParam);
		m_nWndClientHeight = HIWORD(lParam);
		break;
	//case WM_SOCKET:
	//	if (WSAGETASYNCERROR(lParam)) {
	//		closesocket((SOCKET)wParam);
	//	}
	//	switch (WSAGETSELECTEVENT(lParam))
	//	{
	//	case FD_READ:
	//		network_manager::GetInst()->ReadBuffer((SOCKET)wParam);
	//		break;
	//	case FD_CLOSE:
	//		cout << "close \n";
	//		break;
	//	default:
	//		break;
	//	}
	//	//InvalidateRgn(hWnd, NULL, FALSE);
	//	break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hwnd, MessageID, wParam, lParam);
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hwnd, MessageID, wParam, lParam);
		break;

	default:
		break;
	}

	return 0;
}