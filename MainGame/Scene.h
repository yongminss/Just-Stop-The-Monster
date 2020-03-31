#pragma once

#include "GameObject.h"
#include "Player.h"

class TitleScene
{
public:
	TitleScene();
	~TitleScene();

private:
	ID3D12RootSignature			*m_GraphicsRootSignature = NULL;

	// 타이틀 씬에서 사용할 오브젝트
	UI							*m_UI = NULL;

	D3D12_VIEWPORT				m_Viewport;
	D3D12_RECT					m_ScissorRect;

public:
	void BuildObject(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);
	void ReleaseObject();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *Device);
	
	void Render(ID3D12GraphicsCommandList *CommandList);
};


class GameScene
{
public:
	GameScene();
	~GameScene();

private:
	ID3D12RootSignature			*m_GraphicsRootSignature = NULL;

	static ID3D12DescriptorHeap	*m_CbvSrvDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE m_CbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE m_CbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE m_SrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE m_SrvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE m_CbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE m_CbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE m_SrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE m_SrvGPUDescriptorNextHandle;

	Player						*m_Player = NULL;

	GameObject					*m_TrapModel = NULL;
	list<Trap*>					m_TrapObj;
	list<TrapCover*>			m_TrapCover;

public:
	void BuildObject(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);
	void ReleaseObject();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *Device);
	
	void CreateShaderVariable(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);

	static void CreateCbvSrvDescriptorHeap(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, int nConstantBufferView, int nShaderResourceView);

	void Animate(float ElapsedTime);
	void Render(ID3D12GraphicsCommandList *CommandList);

};