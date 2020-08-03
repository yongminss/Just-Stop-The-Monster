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
	UI							*m_Single = NULL;
	UI							*m_Multi = NULL;
	UI							*m_Title = NULL;
	UI							*m_RoomList = NULL;
	UI							*m_MakeRoom = NULL;
	UI							*m_JoinRoom = NULL;
	UI							*m_Room_1 = NULL;
	UI							*m_Room_2 = NULL;
	UI							*m_Room_3 = NULL;
	UI							*m_Room_4 = NULL;
	UI							*m_StageSelect = NULL;
	UI							*m_StageLeft = NULL;
	UI							*m_StageRight = NULL;
	UI							*m_StartButton = NULL;
	UI							*m_PlayerInfo = NULL;
	UI							*m_Player_1 = NULL;
	UI							*m_Player_2 = NULL;
	UI							*m_MyPlayer = NULL;
	UI							*m_BackButton = NULL;
	UI							*m_Number_1 = NULL;
	UI							*m_Number_2 = NULL;
	UI							*m_Number_3 = NULL;
	UI							*m_Number_4 = NULL;

	int							m_StageNumber = 1;

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

	enum {
		Basic,
		Select_Room,
		Wait_Room,
	};

	int		m_state = Basic;
	bool	m_StartGame = false;

public:
	void BuildObject(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);
	void ReleaseObject();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *Device);
	
	static void CreateCbvSrvDescriptorHeap(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, int nConstantBufferView, int nShaderResourceView);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceView(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, Texture *Texture, UINT nRootParameterStartIndex, bool AutoIncrement);

	void Render(ID3D12GraphicsCommandList *CommandList);

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	bool IsStartGame() { return m_StartGame; }

	short GetStageNum() { return (short)m_StageNumber; }

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

	UI							*m_Scope = NULL;

	// Map
	SkyBox						*m_SkyBox[5];
	//GameObject					*m_StageWall = NULL;
	GameObject					*m_StageFloor = NULL;

	GameObject					*m_Stage_01 = NULL;
	GameObject					*m_Stage_02 = NULL;
	GameObject					*m_Stage_03 = NULL;
	GameObject					*m_Stage_04 = NULL;

	bool is_rend_01 = true;
	bool is_rend_02 = false;
	bool is_rend_03 = false;
	bool is_rend_04 = false;

	// Trap Objects
	TrapInstancingShader		*m_Needle = NULL;
	TrapInstancingShader		*m_Fire = NULL;
	TrapInstancingShader		*m_Slow = NULL;
	TrapInstancingShader		*m_Arrow = NULL;

	// Monster Objects
	MonsterInstancingShader		*m_Orc = NULL;
	MonsterInstancingShader		*m_Shaman = NULL;
	MonsterInstancingShader		*m_WolfRider = NULL;

	// Effect
	//EffectShader				*m_FireEffect = NULL;

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
	POINT m_ptOldCursorPos;

	bool	m_bClick = false;

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

	void CheckTile();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	// Player가 가지고 있는 정보를 Framework에 보내주자
	short		GetPlayerAnimate() { return m_Player->GetAnimateState(); }
	XMFLOAT4X4	GetPlayerInfo() { return m_Player->GetTransform(); }

	int AnimateState = 0;
};