#pragma once

#include "Timer.h"
#include "Scene.h"
#include "Camera.h"
#include "network_manager.h"

class GameFramework
{
public:
	GameFramework();
	~GameFramework();

private:
	HINSTANCE					m_hInstance;
	HWND						m_hwnd;

	int							m_nWndClientWidth;
	int							m_nWndClientHeight;

	IDXGIFactory4				*m_Factory = NULL;
	IDXGISwapChain3				*m_SwapChain = NULL;
	ID3D12Device				*m_Device = NULL;

	bool						m_bMsaa4xEnable = FALSE;
	UINT						m_nMsaa4xQualityLevels = 0;

	ID3D12CommandAllocator		*m_CommandAllocator = NULL;
	ID3D12CommandQueue			*m_CommandQueue = NULL;
	ID3D12GraphicsCommandList	*m_CommandList = NULL;

	static const UINT			m_nSwapChainBuffer = 2;
	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource				*m_SwapChainBackBuffer[m_nSwapChainBuffer];
	ID3D12DescriptorHeap		*m_RtvDescriptorHeap = NULL;
	UINT						m_nRtvDescriptorIncrementSize;

	ID3D12Resource				*m_DepthStencilBuffer = NULL;
	ID3D12DescriptorHeap		*m_DsvDescriptorHeap = NULL;
	UINT						m_nDsvDescriptorIncrementSize;

	ID3D12Fence					*m_Fence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffer];
	HANDLE						m_FenceEvent;

	_TCHAR						m_FrameRate[70];

	// 프레임워크에서 사용할 클래스 선언
	Timer						m_Timer;
	TitleScene					*m_TitleScene = NULL;
	GameScene					*m_GameScene = NULL;
	
	int							SceneState = 0;

public:
	bool OnCreate(HINSTANCE hInstance, HWND MainHwnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetView();
	void CreateDepthStencilView();

	void BuildObject();
	void ReleaseObject();

	void Animate();
	
	void WaitForGpuComplete();
	void MoveToNextFrame();

	void FrameAdvance();
	
	void OnProcessingMouseMessage(HWND hwnd, UINT MessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hwnd, UINT MessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hwnd, UINT MessageID, WPARAM wParam, LPARAM lParam);

};