#pragma once

#include "GameObject.h"
#include "Player.h"
#include "Shader.h"

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

#define MAX_LIGHTS			16

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

struct LIGHT
{
	XMFLOAT4		m_Ambient;
	XMFLOAT4		m_Diffuse;
	XMFLOAT4		m_Specular;
	XMFLOAT3		m_Position;
	float			m_Falloff;
	XMFLOAT3		m_Direction;
	float			m_Theta;
	XMFLOAT3		m_Attenuation;
	float			m_Phi;
	bool			m_Enable;
	int				m_nType;
	float			m_Range;
	float			padding;
};

struct LIGHTS
{
	LIGHT			m_Lights[MAX_LIGHTS];
	XMFLOAT4		m_GlobalAmbient;
	int				m_nLights;
};


class GameScene
{
public:
	GameScene();
	~GameScene();

private:
	ID3D12RootSignature			*m_GraphicsRootSignature = NULL;

	LIGHT						*m_Lights = NULL;
	int							m_nLights = 0;

	XMFLOAT4					m_GlobalAmbient;

	ID3D12Resource				*m_cbLight = NULL;
	LIGHTS						*m_cbMappedLight = NULL;

	Player						*m_Player = NULL;

	GameObject					*m_TrapModel = NULL;
	list<Trap*>					m_Trap;

	GameObject					*m_MonsterModel = NULL;
	list<Trap*>					m_Moster;

	list<TrapCover*>			m_TrapCover;

	// 디스크립터 힙을 게임 씬에서 만듬
	static ID3D12DescriptorHeap	*m_CbvSrvDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE m_CbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE m_CbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE m_SrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE m_SrvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE m_CbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE m_CbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE m_SrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE m_SrvGPUDescriptorNextHandle;

public:
	void BuildObject(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);
	void ReleaseObject();

	void BuildDefaultLightsAndMaterials();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *Device);

	void CreateShaderVariable(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);
	void UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList);

	static void CreateCbvSrvDescriptorHeap(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, int nConstantBufferView, int nShaderResourceView);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceView(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, Texture *Texture, UINT nRootParameterStartIndex, bool AutoIncrement);

	void Animate(float ElapsedTime);
	void Render(ID3D12GraphicsCommandList *CommandList);

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};