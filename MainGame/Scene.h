#pragma once

#include "GameObject.h"
#include "Player.h"
#include "Shader.h"
#include "network_manager.h"

class TitleScene
{
public:
	TitleScene();
	~TitleScene();

private:
	ID3D12RootSignature			*m_GraphicsRootSignature = NULL;

	// 타이틀 씬에서 사용할 오브젝트
	UI							*m_Background = NULL;
	UI							*m_Select = NULL;

	D3D12_VIEWPORT				m_Viewport;
	D3D12_RECT					m_ScissorRect;

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

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *Device);
	
	static void CreateCbvSrvDescriptorHeap(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, int nConstantBufferView, int nShaderResourceView);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceView(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, Texture *Texture, UINT nRootParameterStartIndex, bool AutoIncrement);

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

	SOCKET m_socket;

private:
	ID3D12RootSignature			*m_GraphicsRootSignature = NULL;

	// Light
	LIGHT						*m_Lights = NULL;
	int							m_nLights = 0;

	XMFLOAT4					m_GlobalAmbient;

	ID3D12Resource				*m_cbLight = NULL;
	LIGHTS						*m_cbMappedLight = NULL;

	Player						*m_Player = NULL;

	// UI
	UI							*m_CharInfo = NULL;
	UI							*m_TrapListUi = NULL;

	// Map
	SkyBox						*m_SkyBox[5];
	GameObject					*m_StageWall = NULL;
	GameObject					*m_StageFloor = NULL;

	// Trap
	GameObject					*m_NeedleTrapModel = NULL;
	list<Trap*>					m_NeedleTrap;

	// Monster Model
	GameObject					*m_OrcModel = NULL;
	list<Monster*>				m_Orc;

	GameObject					*m_ShamanModel = NULL;
	list<Monster*>				m_Shaman;

	GameObject					*m_WolfRiderModel = NULL;
	list<Monster*>				m_WolfRider;

	// Other Player
	GameObject					*m_OtherPlayerModel = NULL;

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

	// 마우스 입력
	POINT m_OldCursourPos;

public:
	bool temp = false;

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

	void ProcessInput(HWND hWnd);

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	// Player가 가지고 있는 정보를 Framework에 보내주자
	short		GetPlayerAnimate() { return m_Player->GetAnimateState(); }
	XMFLOAT4X4	GetPlayerInfo() { return m_Player->GetTransform(); }

	int AnimateState = 0;
};