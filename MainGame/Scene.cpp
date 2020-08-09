#include "stdafx.h"
#include "Scene.h"
class network_manager;

ID3D12DescriptorHeap *TitleScene::m_CbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE TitleScene::m_CbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE TitleScene::m_CbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE TitleScene::m_SrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE TitleScene::m_SrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE TitleScene::m_CbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE TitleScene::m_CbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE TitleScene::m_SrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE TitleScene::m_SrvGPUDescriptorNextHandle;

// 게임 시작 시, 등장할 타이틀 화면의 클래스
TitleScene::TitleScene()
{

}

TitleScene::~TitleScene()
{
	ReleaseObject();
}

void TitleScene::BuildObject(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList)
{
	m_GraphicsRootSignature = CreateGraphicsRootSignature(Device);

	CreateCbvSrvDescriptorHeap(Device, CommandList, 0, 30);

	m_Viewport = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.f, 1.f };
	m_ScissorRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };

	// TitleScene 에서 Redering 될 Objects
	m_Background = new UI(Device, CommandList, m_GraphicsRootSignature, 1.0f, 1.0f, BackGround, 0);
	m_Title = new UI(Device, CommandList, m_GraphicsRootSignature, 0.7, 0.5f, Title, 0);
	m_Single = new UI(Device, CommandList, m_GraphicsRootSignature, 0.5f, 0.5f, Single, 0);
	m_Multi = new UI(Device, CommandList, m_GraphicsRootSignature, 0.5f, 0.5f, Multi, 0);
	m_Room_1 = new UI(Device, CommandList, m_GraphicsRootSignature, 0.4f, 0.4f, Room_1, 0);
	m_Room_2 = new UI(Device, CommandList, m_GraphicsRootSignature, 0.4f, 0.4f, Room_2, 0);
	m_Room_3 = new UI(Device, CommandList, m_GraphicsRootSignature, 0.4f, 0.4f, Room_3, 0);
	m_Room_4 = new UI(Device, CommandList, m_GraphicsRootSignature, 0.4f, 0.4f, Room_4, 0);
	m_RoomList = new UI(Device, CommandList, m_GraphicsRootSignature, 1.f, 1.f, RoomList, 0);
	m_JoinRoom = new UI(Device, CommandList, m_GraphicsRootSignature, 1.f, 1.f, JoinRoom, 0);
	m_MakeRoom = new UI(Device, CommandList, m_GraphicsRootSignature, 1.f, 1.f, MakeRoom, 0);
	m_StageSelect = new UI(Device, CommandList, m_GraphicsRootSignature, 0.45f, 0.2f, Select_Stage, 0);
	m_StageLeft = new UI(Device, CommandList, m_GraphicsRootSignature, 0.45f, 0.2f, Stage_Left, 0);
	m_StageRight = new UI(Device, CommandList, m_GraphicsRootSignature, 0.45f, 0.2f, Stage_Right, 0);
	m_StartButton = new UI(Device, CommandList, m_GraphicsRootSignature, 0.25f, 0.15f, Start_Button, 0);
	m_PlayerInfo = new UI(Device, CommandList, m_GraphicsRootSignature, 0.35f, 0.8f, PlayerInfo, 0);
	m_Player_1 = new UI(Device, CommandList, m_GraphicsRootSignature, 0.2f, 0.2f, Player_1, 0);
	m_Player_2 = new UI(Device, CommandList, m_GraphicsRootSignature, 0.2f, 0.2f, Player_2, 0);
	m_Player_3 = new UI(Device, CommandList, m_GraphicsRootSignature, 0.2f, 0.2f, Player_3, 0);
	m_Player_4 = new UI(Device, CommandList, m_GraphicsRootSignature, 0.2f, 0.2f, Player_4, 0);
	m_BackButton = new UI(Device, CommandList, m_GraphicsRootSignature, 0.1f, 0.1f, Back_Button, 0);
	m_Number_1 = new UI(Device, CommandList, m_GraphicsRootSignature, 0.03f, 0.07f, StageNum_1, 0);
	m_Number_2 = new UI(Device, CommandList, m_GraphicsRootSignature, 0.03f, 0.07f, StageNum_2, 0);
	m_Number_3 = new UI(Device, CommandList, m_GraphicsRootSignature, 0.03f, 0.07f, StageNum_3, 0);
}

void TitleScene::ReleaseObject()
{
	if (m_GraphicsRootSignature) m_GraphicsRootSignature->Release();

	if (m_Background) delete m_Background;
	if (m_RoomList) delete m_RoomList;
	if (m_Title) delete m_Title;
	if (m_Single) delete m_Single;
	if (m_Multi) delete m_Multi;
	if (m_Room_1) delete m_Room_1;
	if (m_Room_2) delete m_Room_2;
	if (m_Room_3) delete m_Room_3;
	if (m_Room_4) delete m_Room_4;
	if (m_JoinRoom) delete m_JoinRoom;
	if (m_MakeRoom) delete m_MakeRoom;
	if (m_StageSelect) delete m_StageSelect;
	if (m_StageLeft) delete m_StageLeft;
	if (m_StageRight) delete m_StageRight;
	if (m_StartButton) delete m_StartButton;
	if (m_PlayerInfo) delete m_PlayerInfo;
	if (m_Player_1) delete m_Player_1;
	if (m_Player_2) delete m_Player_2;
	if (m_BackButton) delete m_BackButton;
	if (m_Number_1) delete m_Number_1;
	if (m_Number_2) delete m_Number_2;
	if (m_Number_3) delete m_Number_3;
}

ID3D12RootSignature *TitleScene::CreateGraphicsRootSignature(ID3D12Device *Device)
{
	ID3D12RootSignature *GraphicsRootSignature = NULL;

	// 디스크립터 레인지
	D3D12_DESCRIPTOR_RANGE DescriptorRange;
	DescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	DescriptorRange.NumDescriptors = 1;
	DescriptorRange.BaseShaderRegister = 0;
	DescriptorRange.RegisterSpace = 0;
	DescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// 루트 파라미터
	D3D12_ROOT_PARAMETER RootParameter[2];
	RootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	RootParameter[0].Descriptor.ShaderRegister = 1; // Camera
	RootParameter[0].Descriptor.RegisterSpace = 0;
	RootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// TitleScene에선 이 루트 파라미터만 사용하지만
	// Render 시에 Root32BitConstants에 넘겨주는 값 때문에 0번 파라미터에 필요 없는 값 생성
	RootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	RootParameter[1].DescriptorTable.NumDescriptorRanges = 1;
	RootParameter[1].DescriptorTable.pDescriptorRanges = &DescriptorRange;
	RootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// 샘플러
	D3D12_STATIC_SAMPLER_DESC SamplerDesc;
	::ZeroMemory(&SamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.MipLODBias = 0;
	SamplerDesc.MaxAnisotropy = 1;
	SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	SamplerDesc.ShaderRegister = 0;
	SamplerDesc.RegisterSpace = 0;
	SamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS RootSignatureFlag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc;
	::ZeroMemory(&RootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	RootSignatureDesc.NumParameters = _countof(RootParameter);
	RootSignatureDesc.pParameters = RootParameter;
	RootSignatureDesc.NumStaticSamplers = 1;
	RootSignatureDesc.pStaticSamplers = &SamplerDesc;
	RootSignatureDesc.Flags = RootSignatureFlag;

	ID3DBlob *SignatureBlob = NULL;
	ID3DBlob *ErrorBlob = NULL;
	D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &SignatureBlob, &ErrorBlob);
	Device->CreateRootSignature(0, SignatureBlob->GetBufferPointer(), SignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&GraphicsRootSignature);

	if (SignatureBlob) SignatureBlob->Release();
	if (ErrorBlob) ErrorBlob->Release();

	return GraphicsRootSignature;
}

void TitleScene::CreateCbvSrvDescriptorHeap(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, int nConstantBufferView, int nShaderResourceView)
{
	D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc;
	DescriptorHeapDesc.NumDescriptors = nConstantBufferView + nShaderResourceView;
	DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DescriptorHeapDesc.NodeMask = 0;
	Device->CreateDescriptorHeap(&DescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_CbvSrvDescriptorHeap);

	m_CbvCPUDescriptorNextHandle = m_CbvCPUDescriptorStartHandle = m_CbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_CbvGPUDescriptorNextHandle = m_CbvGPUDescriptorStartHandle = m_CbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_SrvCPUDescriptorNextHandle.ptr = m_SrvCPUDescriptorStartHandle.ptr = m_CbvCPUDescriptorStartHandle.ptr + (::nCbvSrvDescriptorIncrementSize * nConstantBufferView);
	m_SrvGPUDescriptorNextHandle.ptr = m_SrvGPUDescriptorStartHandle.ptr = m_CbvGPUDescriptorStartHandle.ptr + (::nCbvSrvDescriptorIncrementSize * nConstantBufferView);
}

D3D12_SHADER_RESOURCE_VIEW_DESC GetTitleSceneShaderResourceViewDesc(D3D12_RESOURCE_DESC ResourceDesc, UINT nTextureType)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc;
	ShaderResourceViewDesc.Format = ResourceDesc.Format;
	ShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D:
	case RESOURCE_TEXTURE2D_ARRAY:
		ShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		ShaderResourceViewDesc.Texture2D.MipLevels = -1;
		ShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		ShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		ShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;

	case RESOURCE_TEXTURE2DARRAY:
		ShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		ShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		ShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		ShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		ShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		ShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		ShaderResourceViewDesc.Texture2DArray.ArraySize = ResourceDesc.DepthOrArraySize;
		break;

	case RESOURCE_TEXTURE_CUBE:
		ShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		ShaderResourceViewDesc.TextureCube.MipLevels = -1;
		ShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		ShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;

	case RESOURCE_BUFFER:
		ShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		ShaderResourceViewDesc.Buffer.FirstElement = 0;
		ShaderResourceViewDesc.Buffer.NumElements = 0;
		ShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		ShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return ShaderResourceViewDesc;
}

D3D12_GPU_DESCRIPTOR_HANDLE TitleScene::CreateShaderResourceView(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, Texture *Texture, UINT nRootParameterStartIndex, bool AutoIncrement)
{
	if (Texture) {
		int nTexture = Texture->GetTextureNum();
		int nTextureType = Texture->GetTextureType();
		for (int i = 0; i < nTexture; ++i) {
			ID3D12Resource *ShaderResource = Texture->GetTexture(i);
			D3D12_RESOURCE_DESC ResourceDesc = ShaderResource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc = GetTitleSceneShaderResourceViewDesc(ResourceDesc, nTextureType);
			Device->CreateShaderResourceView(ShaderResource, &ShaderResourceViewDesc, m_SrvCPUDescriptorNextHandle);
			m_SrvCPUDescriptorNextHandle.ptr += ::nCbvSrvDescriptorIncrementSize;

			Texture->SetRootArgument(i, (AutoIncrement) ? (nRootParameterStartIndex + i) : nRootParameterStartIndex, m_SrvGPUDescriptorNextHandle);
			m_SrvGPUDescriptorNextHandle.ptr += ::nCbvSrvDescriptorIncrementSize;
		}
	}
	return m_SrvGPUDescriptorNextHandle;
}

void TitleScene::Render(ID3D12GraphicsCommandList *CommandList)
{
	// 루트 시그너쳐를 설정
	CommandList->SetGraphicsRootSignature(m_GraphicsRootSignature);
	// 디스크립터 힙 설정
	CommandList->SetDescriptorHeaps(1, &m_CbvSrvDescriptorHeap);

	// 뷰포트와 씨저렉트 영역을 설정
	CommandList->RSSetViewports(1, &m_Viewport);
	CommandList->RSSetScissorRects(1, &m_ScissorRect);

	// Player의 선택에 따라 그려질 방법
	int cnt = 0;
	switch (m_state) {
	case Basic:
		if (m_Title) m_Title->Render(CommandList);
		if (m_Single) m_Single->Render(CommandList);
		if (m_Multi) m_Multi->Render(CommandList);
		if (m_Background) m_Background->Render(CommandList);
		break;

	case Select_Room:
		if (m_BackButton) m_BackButton->Render(CommandList);

		for (GAME_ROOM_C* a : network_manager::GetInst()->m_vec_gameRoom) {
			if (cnt == 0) m_Room_1->Render(CommandList);
			if (cnt == 1) m_Room_2->Render(CommandList);
			if (cnt == 2) m_Room_3->Render(CommandList);
			if (cnt == 3) m_Room_4->Render(CommandList);
			cnt++;
		}
		if (m_JoinRoom) m_JoinRoom->Render(CommandList);
		if (m_MakeRoom) m_MakeRoom->Render(CommandList);
		if (m_RoomList) m_RoomList->Render(CommandList);
		if (m_Background) m_Background->Render(CommandList);
		break;

	case Wait_Room:
		if (m_BackButton) m_BackButton->Render(CommandList);

		if (m_StageNumber == 1 && m_Number_1) m_Number_1->Render(CommandList);
		else if (m_StageNumber == 2 && m_Number_2) m_Number_2->Render(CommandList);
		else if (m_StageNumber == 3 && m_Number_3) m_Number_3->Render(CommandList);
		
		if (m_StageLeft) m_StageLeft->Render(CommandList);
		if (m_StageRight) m_StageRight->Render(CommandList);
		if (m_StageSelect) m_StageSelect->Render(CommandList);
		if (m_StartButton) m_StartButton->Render(CommandList);
		
		if (m_NumID_0) m_NumID_0->Render(CommandList);

		for (int i = 0; i < 4; i++) {
			if (network_manager::GetInst()->m_myRoomInfo.players_id[i] != -1) {
				if (i == 0 && m_Player_1) m_Player_1->Render(CommandList);
				else if (i == 1 && m_Player_2) m_Player_2->Render(CommandList);
				else if (i == 2 && m_Player_3) m_Player_3->Render(CommandList);
				else if (i == 3 && m_Player_4) m_Player_4->Render(CommandList);
			}
		}
		if (m_PlayerInfo) m_PlayerInfo->Render(CommandList);
		if (m_Background) m_Background->Render(CommandList);
		break;

	default:
		break;

	}
	// 씬에 등장할 오브젝트들을 렌더링
	if (PLAYER_STATE_in_room == network_manager::GetInst()->m_my_info.player_state) {
	}
}

bool TitleScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	POINT MousePos{};

	MousePos.x = LOWORD(lParam);
	MousePos.y = HIWORD(lParam);

	switch (nMessageID)
	{

	case WM_LBUTTONDOWN:
		MousePos.x = LOWORD(lParam);
		MousePos.y = HIWORD(lParam);
		/*cout << "x : " << MousePos.x << ", y : " << MousePos.y << endl;*/
		switch (m_state) {
		case Basic:
			// Single Game 버튼을 클릭하면 Game Start
			if (MousePos.x > 200 && MousePos.x < 600 && MousePos.y > 300 && MousePos.y < 400)
				m_StartGame = true;
			// Multi Game 버튼을 클릭하면 Make Room
			else if (MousePos.x > 200 && MousePos.x < 600 && MousePos.y > 430 && MousePos.y < 500)
				m_state = Select_Room;
			break;
		case Select_Room:
			// Back Button
			if (MousePos.x > 80 && MousePos.x < 160 && MousePos.y > 90 && MousePos.y < 150) {
				m_state = Basic;
			}
			// Make Room
			if (MousePos.x > 625 && MousePos.x < 785 && MousePos.y > 175 && MousePos.y < 220) {
				network_manager::GetInst()->send_make_room_packet();
				m_state = Wait_Room;
			}
			// Join Room
			if (MousePos.x > 260 && MousePos.x < 590 && MousePos.y > 110 && MousePos.y < 194)
				if (network_manager::GetInst()->m_vec_gameRoom.size() == 1) {
					network_manager::GetInst()->send_request_join_room(network_manager::GetInst()->m_vec_gameRoom[0]->room_number);
					m_state = Wait_Room;

				}
			if (MousePos.x > 260 && MousePos.x < 590 && MousePos.y > 210 && MousePos.y < 300)
				if (network_manager::GetInst()->m_vec_gameRoom.size() == 2) {
					network_manager::GetInst()->send_request_join_room(network_manager::GetInst()->m_vec_gameRoom[1]->room_number);
					m_state = Wait_Room;

				}
			if (MousePos.x > 260 && MousePos.x < 590 && MousePos.y > 320 && MousePos.y < 400)
				if (network_manager::GetInst()->m_vec_gameRoom.size() == 2) {
					network_manager::GetInst()->send_request_join_room(network_manager::GetInst()->m_vec_gameRoom[2]->room_number);
					m_state = Wait_Room;

				}
			if (MousePos.x > 260 && MousePos.x < 590 && MousePos.y > 425 && MousePos.y < 505)
				if (network_manager::GetInst()->m_vec_gameRoom.size() == 2) {
					network_manager::GetInst()->send_request_join_room(network_manager::GetInst()->m_vec_gameRoom[3]->room_number);
					m_state = Wait_Room;

				}
			break;

		case Wait_Room:
			// Back Button
			if (MousePos.x > 80 && MousePos.x < 160 && MousePos.y > 60 && MousePos.y < 120) {
				network_manager::GetInst()->send_leaveRoom();
				m_state = Select_Room;
			}
			//Start Button
			if (MousePos.x > 100 && MousePos.x < 300 && MousePos.y >370 && MousePos.y < 470) {
				m_StartGame = true;
			}
			// Stage Select Before
			if (MousePos.x > 90 && MousePos.x < 145 && MousePos.y > 215 && MousePos.y < 255) {
				if (m_StageNumber > 1) m_StageNumber -= 1;
				//cout << "StageNumber Down " << m_StageNumber << endl;
			}
			// Stage Select Next
			if (MousePos.x > 250 && MousePos.x < 300 && MousePos.y > 215 && MousePos.y < 255) {
				if (m_StageNumber < 3) m_StageNumber += 1;
				//cout << "StageNumber UP " << m_StageNumber << endl;
			}

			break;

		default:
			break;
		}

	case WM_LBUTTONUP:
		break;

	default:
		//cout << "x: " << MousePos.x << " y: " << MousePos.y << endl;
		switch (m_state) {
		case Basic:
			break;
		case Select_Room:
			// Back Button
			if (MousePos.x > 80 && MousePos.x < 160 && MousePos.y > 60 && MousePos.y < 120) {
				//m_BackButton->SetRed(0x01);

			}
			else {
				//m_BackButton->SetRed(0x00);
			}
			break;
		case Wait_Room:
			// Back Button
			if (MousePos.x > 80 && MousePos.x < 160 && MousePos.y > 60 && MousePos.y < 120) {
				//m_BackButton->SetRed(0x01);
			}
			else {
				//m_BackButton->SetRed(0x00);
			}
			// Stage Select Before
			if (MousePos.x > 90 && MousePos.x < 145 && MousePos.y > 215 && MousePos.y < 255) {
				//색상바꾸기
			}
			// Stage Select Next
			if (MousePos.x > 250 && MousePos.x < 145 && MousePos.y > 300 && MousePos.y < 255) {
				//색상바꾸기
			}
			break;
		default:
			break;
		}
		break;
	}

	return false;
}

// 싱글 및 멀티와 캐릭터, 함정 등을 선택 후, 게임을 진행하는 게임 씬
GameScene::GameScene()
{
	m_socket = network_manager::GetInst()->m_serverSocket;
}

GameScene::~GameScene()
{
	// Direct 삭제
	if (m_GraphicsRootSignature) m_GraphicsRootSignature->Release();
	if (m_cbLight) m_cbLight->Release();

	// GameObject 삭제
	if (m_Player) m_Player->Release();

	if (m_Stage_02) m_Stage_02->Release();

	m_Trap.clear();
	vector<Trap*>().swap(m_Trap);

	m_Orc.clear();
	vector<Monster*>().swap(m_Orc);

	m_StrongOrc.clear();
	vector<Monster*>().swap(m_StrongOrc);

	m_WolfRider.clear();
	vector<Monster*>().swap(m_WolfRider);
}

void GameScene::BuildObject(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList)
{
	m_GraphicsRootSignature = CreateGraphicsRootSignature(Device);

	CreateCbvSrvDescriptorHeap(Device, CommandList, 0, 1000);

	Material::PrepareShader(Device, CommandList, m_GraphicsRootSignature);

	BuildDefaultLightsAndMaterials();

	m_Player = new Player(Device, CommandList, m_GraphicsRootSignature);

	// UI
	for (int i = 0; i < 10; i++) {
		m_HpBar[i] = new UI(Device, CommandList, m_GraphicsRootSignature, 0.04f, 0.06f, UI_HpBar + i, 1);
	}

	m_CharInfo = new UI(Device, CommandList, m_GraphicsRootSignature, 0.4f, 0.125f, UI_PlayerInfo, 1);
	m_TrapListUi = new UI(Device, CommandList, m_GraphicsRootSignature, 0.3f, 0.125f, UI_TrapList, 1);
	m_Scope = new UI(Device, CommandList, m_GraphicsRootSignature, 0.03f, 0.0365f, UI_SCOPE, 1);

	for (int i = 0; i < 10; i++) {
		m_Bullet[i] = new UI(Device, CommandList, m_GraphicsRootSignature, 0.1f, 0.15f, UI_Bullet + i, 1);
	}
	m_victory = new UI(Device, CommandList, m_GraphicsRootSignature, 0.5f, 0.2f, UI_Victory, 1);
	m_gameover = new UI(Device, CommandList, m_GraphicsRootSignature, 0.5f, 0.175f, UI_Gameover, 1);

	
	for (int i = 0; i < 10; ++i) {
		m_wave[i] = new UI(Device, CommandList, m_GraphicsRootSignature, 0.25f, 0.25f, UI_wave01 + i, 1);
	}
	// 스카이박스
	for (int i = 0; i < 5; ++i) {
		m_SkyBox[i] = new SkyBox(Device, CommandList, m_GraphicsRootSignature, i);
		if (i == 4) m_SkyBox[i]->SetRotate(0.f, -90.f, 0.f);
	}

	switch (m_MapNum) {
	case 1:
		m_Stage_02 = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Stage02.bin", NULL, false);
		m_Stage_02->SetPostion(XMFLOAT3(0.f, -50.f, 0.f));
		break;
	case 2:
		m_Stage_03 = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Stage03.bin", NULL, false);
		m_Stage_03->SetPostion(XMFLOAT3(0.f, -50.f, 0.f));
		break;
	case 3:
		m_Stage_04 = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Stage04.bin", NULL, false);
		m_Stage_04->SetPostion(XMFLOAT3(-0.f, -50.f, 0.f));
		break;
	}

	// Trap Objects
	for (int i = 0; i < MAX_TRAP; ++i) {
		m_Trap.emplace_back(new Trap());
		GameObject *TrapObj = NULL;
		if (i < MAX_TRAP / 4) {
			TrapObj = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Trap_Needle.bin", NULL, true);
			m_Trap.back()->m_nTrapKind = TRAP_NEEDLE;
		}
		else if (i < MAX_TRAP / 2) {
			TrapObj = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Trap_Fire.bin", NULL, false);
			m_Trap.back()->m_nTrapKind = TRAP_FIRE;

		}
		else if (i < MAX_TRAP - (MAX_TRAP / 4)) {
			TrapObj = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Trap_Slow.bin", NULL, false);
			m_Trap.back()->m_nTrapKind = TRAP_SLOW;
		}
		else {
			TrapObj = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Trap_Arrow.bin", NULL, false);
			m_Trap.back()->m_nTrapKind = TRAP_ARROW;
			GameObject *SpearObj = NULL;
			SpearObj = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Trap_Spear.bin", NULL, false);
			m_Trap.back()->SetChild(SpearObj, false);
		}
		m_Trap.back()->SetChild(TrapObj, false);
		m_Trap.back()->m_id = i;
		m_Trap.back()->SetEnable(1);
		m_Trap.back()->SetPostion(XMFLOAT3(0, -1000.f, 0.f));
	}
	/*m_Needle = new TrapInstancingShader();
	m_Needle->CreateShader(Device, CommandList, m_GraphicsRootSignature);
	m_Needle->BuildObject(Device, CommandList, m_GraphicsRootSignature, 0);

	m_Fire = new TrapInstancingShader();
	m_Fire->CreateShader(Device, CommandList, m_GraphicsRootSignature);
	m_Fire->BuildObject(Device, CommandList, m_GraphicsRootSignature, 1);

	m_Slow = new TrapInstancingShader();
	m_Slow->CreateShader(Device, CommandList, m_GraphicsRootSignature);
	m_Slow->BuildObject(Device, CommandList, m_GraphicsRootSignature, 2);

	m_Arrow = new TrapInstancingShader();
	m_Arrow->CreateShader(Device, CommandList, m_GraphicsRootSignature);
	m_Arrow->BuildObject(Device, CommandList, m_GraphicsRootSignature, 3);*/

	// Monster Object
	for (int i = 0; i < 32; ++i) {
		GameObject *OrcObj = NULL;
		OrcObj = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Monster_Weak_Infantry.bin", NULL, true);
		m_Orc.emplace_back(new Monster());
		m_Orc.back()->SetChild(OrcObj, true);
		m_Orc.back()->SetMonsterType(TYPE_ORC);
		m_Orc.back()->SetEnable(2);
		m_Orc.back()->SetPostion(XMFLOAT3(0.f, -1000.f, 0.f));
	}
	for (int i = 0; i < 24; ++i) { // Strong Orc
		GameObject *OrcObj = NULL;
		OrcObj = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Monster_Strong_Infantry.bin", NULL, true);
		m_StrongOrc.emplace_back(new Monster());
		m_StrongOrc.back()->SetChild(OrcObj, true);
		m_StrongOrc.back()->SetMonsterType(TYPE_STRONGORC);
		m_StrongOrc.back()->SetEnable(2);
		m_StrongOrc.back()->SetPostion(XMFLOAT3(0.f, -1000.f, 0.f));
	}
	for (int i = 0; i < 16; ++i) { // Wolf Rider
		GameObject *OrcObj = NULL;
		OrcObj = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Monster_WolfRider.bin", NULL, true);
		m_WolfRider.emplace_back(new Monster());
		m_WolfRider.back()->SetChild(OrcObj, true);
		m_WolfRider.back()->SetMonsterType(TYPE_RIDER);
		m_WolfRider.back()->SetEnable(2);
		m_WolfRider.back()->SetPostion(XMFLOAT3(0.f, -1000.f, 0.f));
	}
	/*m_Orc = new MonsterInstancingShader();
	m_Orc->CreateShader(Device, CommandList, m_GraphicsRootSignature);
	m_Orc->BuildObject(Device, CommandList, m_GraphicsRootSignature, TYPE_ORC);

	m_Shaman = new MonsterInstancingShader();
	m_Shaman->CreateShader(Device, CommandList, m_GraphicsRootSignature);
	m_Shaman->BuildObject(Device, CommandList, m_GraphicsRootSignature, TYPE_SHAMAN);

	m_StrongOrc = new MonsterInstancingShader();
	m_StrongOrc->CreateShader(Device, CommandList, m_GraphicsRootSignature);
	m_StrongOrc->BuildObject(Device, CommandList, m_GraphicsRootSignature, TYPE_STRONGORC);

	m_WolfRider = new MonsterInstancingShader();
	m_WolfRider->CreateShader(Device, CommandList, m_GraphicsRootSignature);
	m_WolfRider->BuildObject(Device, CommandList, m_GraphicsRootSignature, TYPE_RIDER);*/

	// Multi-Player Object
	m_OtherPlayerModel = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Soldier_Player.bin", NULL, true);
	m_OtherPlayerModel->SetPostion(XMFLOAT3(-1000.f, -15.f, 0.f));

	m_BulletEffect = new Effect(Device, CommandList, m_GraphicsRootSignature, 0);
	m_BulletEffect->SetPostion(XMFLOAT3(0.f, -1000.f, 0.f));

	/*for (int i = 0; i < 5 * 25; ++i) {
		m_Fire[i] = new Effect(Device, CommandList, m_GraphicsRootSignature, 1);
		m_Fire[i]->SetPostion(XMFLOAT3(0.f, -1000.f, 0.f));
	}*/

	m_Portal = new Effect(Device, CommandList, m_GraphicsRootSignature, 2);

	switch (m_MapNum) {
	case 1:
	{
		m_Portal->SetRotate(180.f, 0.f, 0.f);
		m_Portal->SetPostion(XMFLOAT3(0.f, 140.f, 120.f));
	}
	break;

	case 2:
	{
		m_Portal->SetRotate(0.f, -90.f, 0.f);
		m_Portal->SetPostion(XMFLOAT3(-170.f, 150.f, 60.f));
	}
	break;

	case 3:
	{
		m_Portal->SetRotate(0.f, -90.f, 0.f);
		m_Portal->SetPostion(XMFLOAT3(-170.f, 150.f, 60.f));
	}
	break;
	}

	// Effect
	//m_FireEffect = new EffectShader();
	//m_FireEffect->CreateShader(Device, CommandList, m_GraphicsRootSignature);
	//m_FireEffect->BuildObject(Device, CommandList, m_GraphicsRootSignature);

	CreateShaderVariable(Device, CommandList);
}

void GameScene::ReleaseObject()
{
	if (m_GraphicsRootSignature) m_GraphicsRootSignature->Release();
}

void GameScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 4;
	m_Lights = new LIGHT[m_nLights];
	::ZeroMemory(m_Lights, sizeof(LIGHT) * m_nLights);

	m_GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.f);

	// 방향성 조명
	m_Lights[0].m_Enable = true;
	m_Lights[0].m_nType = DIRECTIONAL_LIGHT;
	m_Lights[0].m_Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.f);
	m_Lights[0].m_Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.f);
	m_Lights[0].m_Specular = XMFLOAT4(0.6f, 0.4f, 0.4f, 1.f);
	m_Lights[0].m_Direction = XMFLOAT3(0.f, 0.f, 1.f);

	/*
	m_Lights[0].m_Enable = true;
	m_Lights[0].m_nType = POINT_LIGHT;
	m_Lights[0].m_Range = 1000.f;
	m_Lights[0].m_Ambient = XMFLOAT4(0.1f, 0.f, 0.f, 1.f);
	m_Lights[0].m_Diffuse = XMFLOAT4(0.8f, 0.f, 0.f, 1.f);
	m_Lights[0].m_Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.f);
	m_Lights[0].m_Position = XMFLOAT3(30.f, 30.f, 30.f);
	m_Lights[0].m_Direction = XMFLOAT3(0.f, 0.f, 0.f);
	m_Lights[0].m_Attenuation = XMFLOAT3(1.f, 0.001f, 0.0001f);

	m_Lights[1].m_Enable = true;
	m_Lights[1].m_nType = SPOT_LIGHT;
	m_Lights[1].m_Range = 500.f;
	m_Lights[1].m_Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.f);
	m_Lights[1].m_Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.f);
	m_Lights[1].m_Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.f);
	m_Lights[1].m_Position = XMFLOAT3(0.f, 20.f, -200.f);
	m_Lights[1].m_Direction = XMFLOAT3(0.f, 0.f, 1.f);
	m_Lights[1].m_Attenuation = XMFLOAT3(1.f, 0.001f, 0.0001f);
	m_Lights[1].m_Falloff = 8.f;
	m_Lights[1].m_Phi = (float)cos(XMConvertToRadians(40.f));
	m_Lights[1].m_Theta = (float)cos(XMConvertToRadians(20.f));

	m_Lights[2].m_Enable = true;
	m_Lights[2].m_nType = DIRECTIONAL_LIGHT;
	m_Lights[2].m_Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.f);
	m_Lights[2].m_Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.f);
	m_Lights[2].m_Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.f);
	m_Lights[2].m_Direction = XMFLOAT3(1.f, 0.f, 0.f);

	m_Lights[3].m_Enable = true;
	m_Lights[3].m_nType = SPOT_LIGHT;
	m_Lights[3].m_Range = 600.f;
	m_Lights[3].m_Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.f);
	m_Lights[3].m_Diffuse = XMFLOAT4(0.3f, 0.7f, 0.f, 1.f);
	m_Lights[3].m_Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.f);
	m_Lights[3].m_Position = XMFLOAT3(50.f, 30.f, 30.f);
	m_Lights[3].m_Direction = XMFLOAT3(0.f, 1.f, 1.f);
	m_Lights[3].m_Attenuation = XMFLOAT3(1.f, 0.01f, 0.0001f);
	m_Lights[3].m_Falloff = 8.f;
	m_Lights[3].m_Phi = (float)cos(XMConvertToRadians(40.f));
	m_Lights[3].m_Theta = (float)cos(XMConvertToRadians(30.f));*/
}

ID3D12RootSignature *GameScene::CreateGraphicsRootSignature(ID3D12Device *Device)
{
	ID3D12RootSignature *GraphicsRootSignature = NULL;

	// 디스크립터 레인지
	D3D12_DESCRIPTOR_RANGE DescriptorRange[8];
	DescriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	DescriptorRange[0].NumDescriptors = 1;
	DescriptorRange[0].BaseShaderRegister = 0; // Texture
	DescriptorRange[0].RegisterSpace = 0;
	DescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	DescriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	DescriptorRange[1].NumDescriptors = 1;
	DescriptorRange[1].BaseShaderRegister = 6; // AlbedoTexture
	DescriptorRange[1].RegisterSpace = 0;
	DescriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	DescriptorRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	DescriptorRange[2].NumDescriptors = 1;
	DescriptorRange[2].BaseShaderRegister = 7; // SpecularTexture
	DescriptorRange[2].RegisterSpace = 0;
	DescriptorRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	DescriptorRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	DescriptorRange[3].NumDescriptors = 1;
	DescriptorRange[3].BaseShaderRegister = 8; // NormalTexture
	DescriptorRange[3].RegisterSpace = 0;
	DescriptorRange[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	DescriptorRange[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	DescriptorRange[4].NumDescriptors = 1;
	DescriptorRange[4].BaseShaderRegister = 9; // MetallicTexture
	DescriptorRange[4].RegisterSpace = 0;
	DescriptorRange[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	DescriptorRange[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	DescriptorRange[5].NumDescriptors = 1;
	DescriptorRange[5].BaseShaderRegister = 10; // EmissionTexture
	DescriptorRange[5].RegisterSpace = 0;
	DescriptorRange[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	DescriptorRange[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	DescriptorRange[6].NumDescriptors = 1;
	DescriptorRange[6].BaseShaderRegister = 11; // DetailAlbedoTexture
	DescriptorRange[6].RegisterSpace = 0;
	DescriptorRange[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	DescriptorRange[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	DescriptorRange[7].NumDescriptors = 1;
	DescriptorRange[7].BaseShaderRegister = 12; // DetailNormalTexture
	DescriptorRange[7].RegisterSpace = 0;
	DescriptorRange[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// 루트 파라미터
	D3D12_ROOT_PARAMETER RootParameter[13];
	RootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	RootParameter[0].Descriptor.ShaderRegister = 1; // Camera
	RootParameter[0].Descriptor.RegisterSpace = 0;
	RootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	RootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	RootParameter[1].Constants.Num32BitValues = 34;
	RootParameter[1].Constants.ShaderRegister = 2; // GameObject
	RootParameter[1].Constants.RegisterSpace = 0;
	RootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	RootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	RootParameter[2].DescriptorTable.NumDescriptorRanges = 1;
	RootParameter[2].DescriptorTable.pDescriptorRanges = &DescriptorRange[0]; // Texture
	RootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// bin파일로 로드할 오브젝트의 재질 설정
	RootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	RootParameter[3].DescriptorTable.NumDescriptorRanges = 1;
	RootParameter[3].DescriptorTable.pDescriptorRanges = &DescriptorRange[1]; // AlbedoTexture
	RootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	RootParameter[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	RootParameter[4].DescriptorTable.NumDescriptorRanges = 1;
	RootParameter[4].DescriptorTable.pDescriptorRanges = &DescriptorRange[2]; // SpecularTexture
	RootParameter[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	RootParameter[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	RootParameter[5].DescriptorTable.NumDescriptorRanges = 1;
	RootParameter[5].DescriptorTable.pDescriptorRanges = &(DescriptorRange[3]); // NormalTexture
	RootParameter[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	RootParameter[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	RootParameter[6].DescriptorTable.NumDescriptorRanges = 1;
	RootParameter[6].DescriptorTable.pDescriptorRanges = &(DescriptorRange[4]); // MetallicTexture
	RootParameter[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	RootParameter[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	RootParameter[7].DescriptorTable.NumDescriptorRanges = 1;
	RootParameter[7].DescriptorTable.pDescriptorRanges = &(DescriptorRange[5]); // EmissionTexture
	RootParameter[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	RootParameter[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	RootParameter[8].DescriptorTable.NumDescriptorRanges = 1;
	RootParameter[8].DescriptorTable.pDescriptorRanges = &(DescriptorRange[6]); // DetailAlbedoTexture
	RootParameter[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	RootParameter[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	RootParameter[9].DescriptorTable.NumDescriptorRanges = 1;
	RootParameter[9].DescriptorTable.pDescriptorRanges = &(DescriptorRange[7]); // DetailNormalTexture
	RootParameter[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// 애니메이션에서 사용할 루트 파라미터
	RootParameter[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	RootParameter[10].Descriptor.ShaderRegister = 7; // Skinned Bone Offsets
	RootParameter[10].Descriptor.RegisterSpace = 0;
	RootParameter[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	RootParameter[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	RootParameter[11].Descriptor.ShaderRegister = 8; // Skinned Bone Transforms
	RootParameter[11].Descriptor.RegisterSpace = 0;
	RootParameter[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// 조명
	RootParameter[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	RootParameter[12].Descriptor.ShaderRegister = 4;
	RootParameter[12].Descriptor.RegisterSpace = 0;
	RootParameter[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// 샘플러
	D3D12_STATIC_SAMPLER_DESC SamplerDesc;
	::ZeroMemory(&SamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.MipLODBias = 0;
	SamplerDesc.MaxAnisotropy = 1;
	SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	SamplerDesc.ShaderRegister = 0;
	SamplerDesc.RegisterSpace = 0;
	SamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS RootSignatureFlag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc;
	::ZeroMemory(&RootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	RootSignatureDesc.NumParameters = _countof(RootParameter);
	RootSignatureDesc.pParameters = RootParameter;
	RootSignatureDesc.NumStaticSamplers = 1;
	RootSignatureDesc.pStaticSamplers = &SamplerDesc;
	RootSignatureDesc.Flags = RootSignatureFlag;

	ID3DBlob *SignatureBlob = NULL;
	ID3DBlob *ErrorBlob = NULL;
	D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &SignatureBlob, &ErrorBlob);
	Device->CreateRootSignature(0, SignatureBlob->GetBufferPointer(), SignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&GraphicsRootSignature);

	if (SignatureBlob) SignatureBlob->Release();
	if (ErrorBlob) ErrorBlob->Release();

	return GraphicsRootSignature;
}

void GameScene::CreateShaderVariable(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList)
{
	UINT ncbElementByte = ((sizeof(LIGHT) + 255) & ~255);
	m_cbLight = ::CreateBufferResource(Device, CommandList, NULL, ncbElementByte, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cbLight->Map(0, NULL, (void**)&m_cbMappedLight);
}

void GameScene::UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList)
{
	::memcpy(m_cbMappedLight->m_Lights, m_Lights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_cbMappedLight->m_GlobalAmbient, &m_GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_cbMappedLight->m_nLights, &m_nLights, sizeof(int));

	D3D12_GPU_VIRTUAL_ADDRESS cbLightGpuVirtualAddress = m_cbLight->GetGPUVirtualAddress();
	CommandList->SetGraphicsRootConstantBufferView(12, cbLightGpuVirtualAddress);
}

ID3D12DescriptorHeap *GameScene::m_CbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE GameScene::m_CbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE GameScene::m_CbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE GameScene::m_SrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE GameScene::m_SrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE GameScene::m_CbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE GameScene::m_CbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE GameScene::m_SrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE GameScene::m_SrvGPUDescriptorNextHandle;

void GameScene::CreateCbvSrvDescriptorHeap(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, int nConstantBufferView, int nShaderResourceView)
{
	D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc;
	DescriptorHeapDesc.NumDescriptors = nConstantBufferView + nShaderResourceView;
	DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DescriptorHeapDesc.NodeMask = 0;
	Device->CreateDescriptorHeap(&DescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_CbvSrvDescriptorHeap);

	m_CbvCPUDescriptorNextHandle = m_CbvCPUDescriptorStartHandle = m_CbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_CbvGPUDescriptorNextHandle = m_CbvGPUDescriptorStartHandle = m_CbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_SrvCPUDescriptorNextHandle.ptr = m_SrvCPUDescriptorStartHandle.ptr = m_CbvCPUDescriptorStartHandle.ptr + (::nCbvSrvDescriptorIncrementSize * nConstantBufferView);
	m_SrvGPUDescriptorNextHandle.ptr = m_SrvGPUDescriptorStartHandle.ptr = m_CbvGPUDescriptorStartHandle.ptr + (::nCbvSrvDescriptorIncrementSize * nConstantBufferView);
}

D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC ResourceDesc, UINT nTextureType)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc;
	ShaderResourceViewDesc.Format = ResourceDesc.Format;
	ShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D:
	case RESOURCE_TEXTURE2D_ARRAY:
		ShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		ShaderResourceViewDesc.Texture2D.MipLevels = -1;
		ShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		ShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		ShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;

	case RESOURCE_TEXTURE2DARRAY:
		ShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		ShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		ShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		ShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		ShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		ShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		ShaderResourceViewDesc.Texture2DArray.ArraySize = ResourceDesc.DepthOrArraySize;
		break;

	case RESOURCE_TEXTURE_CUBE:
		ShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		ShaderResourceViewDesc.TextureCube.MipLevels = -1;
		ShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		ShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;

	case RESOURCE_BUFFER:
		ShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		ShaderResourceViewDesc.Buffer.FirstElement = 0;
		ShaderResourceViewDesc.Buffer.NumElements = 0;
		ShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		ShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return ShaderResourceViewDesc;
}

D3D12_GPU_DESCRIPTOR_HANDLE GameScene::CreateShaderResourceView(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, Texture *Texture, UINT nRootParameterStartIndex, bool AutoIncrement)
{
	if (Texture) {
		int nTexture = Texture->GetTextureNum();
		int nTextureType = Texture->GetTextureType();
		for (int i = 0; i < nTexture; ++i) {
			ID3D12Resource *ShaderResource = Texture->GetTexture(i);
			D3D12_RESOURCE_DESC ResourceDesc = ShaderResource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc = GetShaderResourceViewDesc(ResourceDesc, nTextureType);
			Device->CreateShaderResourceView(ShaderResource, &ShaderResourceViewDesc, m_SrvCPUDescriptorNextHandle);
			m_SrvCPUDescriptorNextHandle.ptr += ::nCbvSrvDescriptorIncrementSize;

			Texture->SetRootArgument(i, (AutoIncrement) ? (nRootParameterStartIndex + i) : nRootParameterStartIndex, m_SrvGPUDescriptorNextHandle);
			m_SrvGPUDescriptorNextHandle.ptr += ::nCbvSrvDescriptorIncrementSize;
		}
	}
	return m_SrvGPUDescriptorNextHandle;
}

void GameScene::Animate(float ElapsedTime)
{
	for (int i = 0; i < 5; ++i) if (m_SkyBox[i]) m_SkyBox[i]->Animate(m_Player->GetPosition());

	if (m_Player) {
		if (m_Player->GetPlayerLife() != network_manager::GetInst()->m_my_info.hp) {
			m_Player->SetPlayerLife(network_manager::GetInst()->m_my_info.hp);
			m_Player->SetAnimateType(28, ANIMATION_TYPE_ONCE);
			m_Player->SetEnable(28);
		}

		if (m_Player->GetPlayerLife() == 0) {
			m_Player->SetAnimateType(29, ANIMATION_TYPE_ONCE);
			m_Player->SetEnable(29);
		}

		//벽과 충돌검사
		m_Player->UpdateTransform(NULL);
		if (m_Player->GetMoveInfo()) {
			GameObject *TileObject = new GameObject;
			BoundingBox BoundPlayer = m_Player->GetBodyBounding();
			switch (m_MapNum) {
			case 1:
				TileObject = m_Stage_02->IsStageIntersect(BoundPlayer);
				break;
			case 2:
				TileObject = m_Stage_03->IsStageIntersect(BoundPlayer);
				break;
			case 3:
				TileObject = m_Stage_04->IsStageIntersect(BoundPlayer);
				break;
			}
			if (TileObject != NULL) {
				BoundingBox BoundTile = TileObject->GetMesh()->GetBounds();
				BoundTile.Transform(BoundTile, XMLoadFloat4x4(&TileObject->m_WorldPos));
				
				XMFLOAT3 BoundDistance = Vector3::Subtract(BoundTile.Center, BoundPlayer.Center);
				if (BoundTile.Extents.x < BoundTile.Extents.z) { // 세로벽
					if (BoundDistance.x < 0.f) { //왼쪽 벽 충돌체크
						m_Player->SetmPosition(Vector3::Add(XMFLOAT3(BoundTile.Extents.x + BoundPlayer.Extents.x + BoundDistance.x, 0.0f, 0.0f), m_Player->GetPosition()));
					}
					else {						//오른쪽 벽 충돌체크
						m_Player->SetmPosition(Vector3::Add(XMFLOAT3(-BoundTile.Extents.x - BoundPlayer.Extents.x + BoundDistance.x, 0.0f, 0.0f), m_Player->GetPosition()));
					}
					//cout << "x 벽 충돌중" << endl;
				}
				else {	//가로벽
					if (BoundDistance.z < 0.f) { //후방 벽 충돌체크
						m_Player->SetmPosition(Vector3::Add(XMFLOAT3(0.0f, 0.0f, BoundTile.Extents.z + BoundPlayer.Extents.z + BoundDistance.z), m_Player->GetPosition()));
					}
					else {						//전방 벽 충돌체크
						m_Player->SetmPosition(Vector3::Add(XMFLOAT3(0.0f, 0.0f, -BoundTile.Extents.z - BoundPlayer.Extents.z + BoundDistance.z), m_Player->GetPosition()));
					}
				}
			}
		}

		//m_Player->GetCamera()->SetOffset(XMFLOAT3(0.0f, 70.0f, -150.0f));

		//Camera *pCamera =;

		//if (m_Player->GetCamera()) {
		//	m_Player->GetCamera()->Update(m_Player->GetPosition(), ElapsedTime);
		//	m_Player->GetCamera()->RegenerateViewMatrix();
		//}
		XMFLOAT3 StartPos = m_Player->GetCamera()->GetPosition();
		//cout << "Cam Pos x: " << StartPos.x  << " y: "<< StartPos.y << " z: " << StartPos.z << endl;
		XMFLOAT3 EndPos;
		EndPos = Vector3::Normalize(Vector3::Subtract(m_Player->GetPosition(), StartPos));


		GameObject *TileObject = new GameObject;
		BoundingBox BoundCam = m_Player->GetCamera()->GetCamBound();
		BoundCam.Transform(BoundCam, XMLoadFloat4x4(&Matrix4x4::Inverse(m_Player->GetCamera()->GetViewMatrix())));

		

		switch (m_MapNum) {
		case 1:
			TileObject = m_Stage_02->CheckCamBlock(StartPos, EndPos);//m_Stage_02->IsStageIntersect(BoundCam);
			break;
		case 2:
			TileObject = m_Stage_03->CheckCamBlock(StartPos, EndPos);
			break;
		case 3:
			TileObject = m_Stage_04->CheckCamBlock(StartPos, EndPos);
			break;
		}
		//if (TileObject != NULL) {
		//
		//	BoundingBox BoundTile = TileObject->GetMesh()->GetBounds();
		//	BoundTile.Transform(BoundTile, XMLoadFloat4x4(&TileObject->m_WorldPos));
		//
		//	XMFLOAT3 BoundCamDistance = Vector3::Subtract(BoundTile.Center, BoundCam.Center);
		//
		//	m_Player->GetCamera()->SetOffset(XMFLOAT3(0.0f, 70.0f, +30.0f));
		//	
		//	if (BoundTile.Extents.x < BoundTile.Extents.z) { // 세로벽
		//		
		//		if(StartPos.x <=  BoundTile.Center.x + BoundTile.Extents.x && BoundTile.Center.x - BoundTile.Extents.x <= m_Player->GetPosition().x)
		//			m_Player->GetCamera()->SetOffset(XMFLOAT3(0.0f, 70.0f, +30.0f));
		//
		//		if (StartPos.x >= BoundTile.Center.x - BoundTile.Extents.x && BoundTile.Center.x + BoundTile.Extents.x >= m_Player->GetPosition().x)
		//			m_Player->GetCamera()->SetOffset(XMFLOAT3(0.0f, 70.0f, +30.0f));
		//		if (BoundCamDistance.x < 0.f) { //왼쪽 벽 충돌체크
		//			//m_Player->GetCamera()->SetPosition(Vector3::Add(XMFLOAT3(0.0f, 0.0f, BoundTile.Extents.z + BoundCam.Extents.z - BoundCamDistance.z ), m_Player->GetCamera()->GetPosition()));
		//			cout << "왼벽 충돌" << endl;
		//			//cout << "BoundDistance x: " << BoundCamDistance.x << " y: " << BoundCamDistance.y << " z: " << BoundCamDistance.z << endl;
		//			//cout << "cam pos x: " << m_Player->GetCamera()->GetPosition().x << " y: " << m_Player->GetCamera()->GetPosition().y << " z: " << m_Player->GetCamera()->GetPosition().z << endl;
		//		}
		//		else {						//오른쪽 벽 충돌체크
		//			//m_Player->GetCamera()->SetPosition(Vector3::Add(XMFLOAT3(-BoundTile.Extents.x - BoundCam.Extents.x + BoundCamDistance.x, 0.0f, 0.0f), m_Player->GetCamera()->GetPosition()));
		//			cout << "오른벽 충돌" << endl;
		//		}
		//		//cout << "x 벽 충돌중" << endl;
		//	}
		//	else {	//가로벽
		//		if (StartPos.z <= BoundTile.Center.z + BoundTile.Extents.z && BoundTile.Center.z + BoundTile.Extents.z <= m_Player->GetPosition().z)
		//			m_Player->GetCamera()->SetOffset(XMFLOAT3(0.0f, 70.0f, +30.0f));
		//		
		//		if (StartPos.z >= BoundTile.Center.z - BoundTile.Extents.z && BoundTile.Center.z + BoundTile.Extents.z >= m_Player->GetPosition().z)
		//			m_Player->GetCamera()->SetOffset(XMFLOAT3(0.0f, 70.0f, +30.0f));
		//
		//		if (BoundCamDistance.z < 0.f) { //후방 벽 충돌체크
		//			//m_Player->GetCamera()->SetPosition(Vector3::Add(XMFLOAT3(0.0f, 0.0f, BoundTile.Extents.z + BoundCam.Extents.z + BoundCamDistance.z), m_Player->GetCamera()->GetPosition()));
		//			cout << "후벽 충돌" << endl;
		//		}
		//		else {						//전방 벽 충돌체크
		//			//m_Player->GetCamera()->SetPosition(Vector3::Add(XMFLOAT3(0.0f, 0.0f, -BoundTile.Extents.z - BoundCam.Extents.z + BoundCamDistance.z), m_Player->GetCamera()->GetPosition()));
		//			cout << "전방벽 충돌" << endl;
		//		}
		//	}
		//}
		//else {
		//	cout << "충돌안된 Cam Pos x: " << StartPos.x << " y: " << StartPos.y << " z: " << StartPos.z << endl;
		//	//cout << "Offset x: " << m_Player->GetCamera()->GetOffset().x << " y: " << m_Player->GetCamera()->GetOffset().y << " z: " << m_Player->GetCamera()->GetOffset().z << endl;
		//	//cout << "충돌처리 안됨" << endl;
		//}
		
		XMFLOAT3 position = m_Player->GetPosition();

		switch (m_MapNum) {
		case 1:
		{
			if (position.z < -10.f)
				m_Player->SetmPosition(Vector3::Add(XMFLOAT3(0.f, 0.f, 500.f * m_ElapsedTime), m_Player->GetPosition()));
		}
		break;

		case 2:
		{
			if (position.x < -320.f && position.z >= 0.f && position.z <= 130.f)
				m_Player->SetmPosition(Vector3::Add(XMFLOAT3(500.f * m_ElapsedTime, 0.f, 0.f), m_Player->GetPosition()));
		}
		break;
		}
		m_Player->Update(ElapsedTime);
		
		XMFLOAT3 p_pos = m_Player->GetPosition();
	}

	/*for (int i = 0; i < MAX_TRAP; ++i) {
		if (m_Trap[i]->m_nTrapKind == TRAP_ARROW && m_Trap[i]->is_active == true) {
			XMFLOAT3 TrapRange1 = m_Trap[i]->GetPosition();
			TrapRange1 = Vector3::Add(TrapRange1, Vector3::ScalarProduct(m_Trap[i]->GetUp(), 300.f));
			XMFLOAT3 TrapRange2 = Vector3::Add(TrapRange1, Vector3::ScalarProduct(m_Trap[i]->GetLook(), 50.f));
			if (TrapRange1.x < TrapRange2.x) {
	
			}
			GameObject *SprObj = NULL;
			SprObj = m_Trap[i]->FindFrame("Trap_Spear");
			SprObj->SetPostion(Vector3::Add(SprObj->GetPosition(), Vector3::ScalarProduct(SprObj->GetUp(), 10.f)));
			if ((SprObj->GetPosition().y - m_Trap[i]->GetPosition().y) > 300.0f) {
				SprObj->SetPostion(XMFLOAT3(0.0f, 0.0f, 0.0f));
			}
		}
	}*/


	// Ohter Player
	if (network_manager::GetInst()->IsConnect()) {
		XMFLOAT4X4 Transform = network_manager::GetInst()->m_OtherInfo.Transform;

		AnimateState = network_manager::GetInst()->m_OtherInfo.AnimateState;

		m_OtherPlayerModel->SetTransform(Transform);
		m_OtherPlayerModel->SetScale(30.f, 30.f, 30.f);
		if (AnimateState == 27) m_OtherPlayerModel->SetAnimateType(27, ANIMATION_TYPE_ONCE);
		m_OtherPlayerModel->SetEnable(AnimateState);
		m_OtherPlayerModel->Animate(ElapsedTime, NULL);
	}

	if (m_target) CheckBuildTrap();

	m_ElapsedTime = ElapsedTime;
}

void GameScene::Render(ID3D12GraphicsCommandList *CommandList)
{
	// 루트 시그너쳐를 설정
	CommandList->SetGraphicsRootSignature(m_GraphicsRootSignature);
	// 디스크립터 힙 설정
	CommandList->SetDescriptorHeaps(1, &m_CbvSrvDescriptorHeap);

	// 뷰포트와 씨저렉트 영역을 설정 및 쉐이더 변수 갱신
	m_Player->UpdateCameraSet(CommandList);

	// 조명을 사용하기 위한 함수 호출
	UpdateShaderVariable(CommandList);

	// 스카이박스 렌더링
	for (int i = 0; i < 5; ++i) if (m_SkyBox[i]) m_SkyBox[i]->Render(CommandList);

	// UI
	for (int i = m_Player->GetPlayerLife() / 20 - 1; i >= 0; i--) {
		if (m_HpBar[i]) m_HpBar[i]->Render(CommandList);
	}
	if (m_Player->GetPlayerLife() == 10) m_HpBar[0]->Render(CommandList);


	for (int i = m_Player->GetPlayerBullet(); i >= 0; i--) {
		if (m_Bullet[i]) m_Bullet[i]->Render(CommandList);
	}

	// 플레이어 렌더링
	if (m_Player) m_Player->Render(CommandList);
	//cout << "x: " << m_Player->GetPosition().x << ", z: " << m_Player->GetPosition().z << endl;

	// UI
	if (network_manager::GetInst()->game_end == true) if (m_victory) m_victory->Render(CommandList);
	if (m_Player->GetPlayerLife() <= 0) if (m_gameover) m_gameover->Render(CommandList);
	if (m_CharInfo) m_CharInfo->Render(CommandList);
	if (m_Scope) m_Scope->Render(CommandList);
	if (m_TrapListUi) m_TrapListUi->Render(CommandList);


	if (m_wave[network_manager::GetInst()->m_myRoomInfo.wave_count - 1]) 
		m_wave[network_manager::GetInst()->m_myRoomInfo.wave_count - 1]->Render(CommandList);

	// GameScene에 등장할 오브젝트 렌더링
	switch (m_MapNum) {
	case 1:
		if (m_Stage_02) m_Stage_02->Render(CommandList);
		break;
	case 2:
		if (m_Stage_03) m_Stage_03->Render(CommandList);
		break;
	case 3:
		if (m_Stage_04) m_Stage_04->Render(CommandList);
		break;
	}

	// Fire Effect
	//for (int i = 0; i < 5 * 25; ++i) {
	//	if (m_Fire[i]) {
	//		if (m_Fire[i]->is_set == true) {
	//			m_Fire[i]->Animate(m_ElapsedTime, m_Player->GetCamera()->GetPosition());

	//			XMFLOAT3 position = m_Fire[i]->GetPosition();

	//			if (m_Fire[i]->flag == 0) {

	//				int id = m_Fire[i]->m_flagID;

	//				if (m_Fire[id]->m_dir == 1) position.x -= m_ElapsedTime * 200.f;
	//				else if (m_Fire[id]->m_dir == 2) position.x += m_ElapsedTime * 200.f;
	//				else if (m_Fire[id]->m_dir == 3) position.z -= m_ElapsedTime * 200.f;
	//				else if (m_Fire[id]->m_dir == 4) position.z += m_ElapsedTime * 200.f;

	//				if (Vector3::Distance(position, m_Trap[m_Fire[i]->trap_id]->GetPosition()) > 250.f) m_Fire[i]->flag = 1;
	//			}

	//			else {
	//				m_Fire[i]->flag = 0;

	//				int id = m_Fire[i]->m_flagID;

	//				position = m_Trap[m_Fire[i]->trap_id]->GetPosition();
	//				/*if (m_Fire[id]->m_dir == 1) position.x -= 50.f;
	//				else if (m_Fire[id]->m_dir == 2) position.x += 50.f;
	//				else if (m_Fire[id]->m_dir == 3) position.z -= 50.f;
	//				else position.z += 50.f;*/
	//				position.y += -15.f + rand() % 30;
	//			}
	//			m_Fire[i]->SetPostion(position);
	//			m_Fire[i]->Render(CommandList);
	//		}
	//	}
	//}

	// Trap Objects
	for (int i = 0; i < MAX_TRAP; ++i) {
		if (network_manager::GetInst()->m_trap_pool[i].enable == false) continue;
		// 0 - 11 Needle (12)
		// 12 - 24 Fire (13)
		// 25 - 37 Slow (13)
		// 38 - 49 Arrow (12)

		int trap_id = m_Trap[network_manager::GetInst()->m_trap_pool[i].id]->m_id;
		
		if (m_Trap[trap_id]->GetIsBuildTrap() == false) {
			XMFLOAT4X4 world = network_manager::GetInst()->m_trap_pool[i].trap4x4pos;
			
			m_Trap[trap_id]->is_active = true;
			m_Trap[trap_id]->SetRight(XMFLOAT3(world._11, world._12, world._13));
			m_Trap[trap_id]->SetUp(XMFLOAT3(world._21, world._22, world._23));
			m_Trap[trap_id]->SetLook(XMFLOAT3(world._31, world._32, world._33));
			m_Trap[trap_id]->SetPostion(XMFLOAT3(world._41, world._42, world._43));

			if (m_Trap[trap_id]->m_nTrapKind == TRAP_ARROW) {

					if (m_Trap[trap_id]->is_ArrowShoot != true) {

						if (network_manager::GetInst()->m_trap_pool[i].wallTrapOn == true) {
							m_Trap[trap_id]->is_ArrowShoot = true;

						}
						else
							continue;
					}
					//cout << "i 번째 화살 애니메이션" << endl;
					GameObject *SprObj = NULL;
					SprObj = m_Trap[trap_id]->FindFrame("Trap_Spear");
					SprObj->SetPostion(Vector3::Add(SprObj->GetPosition(), Vector3::ScalarProduct(SprObj->GetUp(), 800.f*m_ElapsedTime)));
					if ((SprObj->GetPosition().y - m_Trap[trap_id]->GetPosition().y) > 300.0f) {
						SprObj->SetPostion(XMFLOAT3(0.0f, 0.0f, 0.0f));
						m_Trap[trap_id]->m_Arrowstack++;
						if (m_Trap[trap_id]->m_Arrowstack > 4) {
							m_Trap[trap_id]->is_ArrowShoot = false;
							m_Trap[trap_id]->m_Arrowstack = 0;
							network_manager::GetInst()->m_trap_pool[i].wallTrapOn = false;
						}
					}
				}


		}
	}
	// Trap Object
	for (int i = 0; i < MAX_TRAP; ++i) {
		if (m_Trap[i]->is_active == false) continue;
		if (m_Trap[i]) {
			m_Trap[i]->Animate(m_ElapsedTime, NULL);
			m_Trap[i]->UpdateTransform(NULL);
			m_Trap[i]->Render(CommandList);
		}
	}
	// Monster Object
	Monster_Function(CommandList, m_Orc);
	Monster_Function(CommandList, m_StrongOrc);
	Monster_Function(CommandList, m_WolfRider);

	/*if (m_Needle) m_Needle->Render(CommandList);
	if (m_Fire) m_Fire->Render(CommandList);
	if (m_Slow) m_Slow->Render(CommandList);
	if (m_Arrow) m_Arrow->Render(CommandList);*/

	/*if (network_manager::GetInst()->is_wave == true) {
		for (int i = 0; i < MAX_MONSTER; ++i)
			m_Orc[i]->is_active = false;
		network_manager::GetInst()->is_wave = false;
	}*/

	// Monster Objects
//	for (int i = 0; i < MAX_MONSTER; ++i) { // 활성화 시킬 Monster Object를 선택
//
//		if (network_manager::GetInst()->m_monster_pool[i].isLive == false) continue;
//
//		if (network_manager::GetInst()->is_wave == false) {
//
//			if (network_manager::GetInst()->m_monster_pool[i].type == m_Orc[i]->GetType()) {
//				m_Orc[i]->is_active = true;
//				m_Orc[i]->m_id = i;/*network_manager::GetInst()->m_monster_pool[i].id;*/
//			}
//			else {
//				char type = network_manager::GetInst()->m_monster_pool[i].type;
//				for (int j = i; j < MAX_MONSTER; ++j) {
//					if (m_Orc[j]->GetType() != type) continue;
//
//					if (m_Orc[j]->is_active == false) {
//						m_Orc[j]->is_active = true;
//						m_Orc[j]->m_id = i;/*network_manager::GetInst()->m_monster_pool[i].id;*/
//						break;
//					}
//
//				}
//			}
//		}
//	}
//
//	for (int i = 0; i < MAX_MONSTER; ++i) {
//		//if (network_manager::GetInst()->m_monster_pool[i].isLive == false) continue;
//		if (m_Orc[i]->is_active == false) continue;
//
//		int server_num = m_Orc[i]->m_id;
//
//		if (server_num == -1) break;
//
//		int animation_temp = m_Orc[i]->GetNowAnimationNum();
//
//		int server_animation = network_manager::GetInst()->m_monster_pool[server_num].animation_state;
//
//		if (server_animation <= 0 && server_animation > 30) server_animation = 0;
//
//		if (animation_temp != server_animation || m_Orc[i]->GetNowAnimationNum() == 0) {
//
//			if (server_animation == M_ANIM_DEATH /*server_animation == M_ANIM_ATT*/) {
//				m_Orc[i]->SetAnimateType(server_animation, ANIMATION_TYPE_ONCE);
//				m_Orc[i]->SetEnable(server_animation);
//			}
//			else
//				m_Orc[i]->SetEnable(server_animation);
//		}
//
//		/*if (network_manager::GetInst()->m_monster_pool[server_num].animation_state != 0) {
//			m_Monster[i]->SetEnable(network_manager::GetInst()->m_monster_pool[server_num].animation_state);
//		}*/
//		
//		XMFLOAT4X4 world = network_manager::GetInst()->m_monster_pool[server_num].world_pos;
//
//		m_Orc[i]->SetRight(XMFLOAT3(world._11, world._12, world._13));
//		m_Orc[i]->SetUp(XMFLOAT3(world._21, world._22, world._23));
//		m_Orc[i]->SetLook(XMFLOAT3(world._31, world._32, world._33));
//		m_Orc[i]->SetPostion(XMFLOAT3(world._41, world._42, world._43));
//
//		m_Orc[i]->Animate(m_ElapsedTime, NULL);
//		m_Orc[i]->UpdateTransform(NULL);
//		m_Orc[i]->Render(CommandList);
//
//		/*if (m_Monster[i]->is_active == false) continue;
//
//		cout << i << endl;
//*/
//		/*int server_num = m_Monster[i]->m_id;
//
//		XMFLOAT4X4 world = network_manager::GetInst()->m_monster_pool[i].world_pos;
//
//		m_Monster[server_num]->SetRight(XMFLOAT3(world._11, world._12, world._13));
//		m_Monster[server_num]->SetUp(XMFLOAT3(world._21, world._22, world._23));
//		m_Monster[server_num]->SetLook(XMFLOAT3(world._31, world._32, world._33));
//		m_Monster[server_num]->SetPostion(XMFLOAT3(world._41, world._42, world._43));
//
//		m_Monster[server_num]->Animate(m_ElapsedTime, NULL);
//		m_Monster[server_num]->UpdateTransform(NULL);
//		m_Monster[server_num]->Render(CommandList);*/
//
//
//	}




		/*if (network_manager::GetInst()->m_monster_pool[i].animation_state != 0)
			m_Monster[m_Monster[i]->m_id]->SetEnable(network_manager::GetInst()->m_monster_pool[i].animation_state);*/

			/*	int server_id = m_Monster[i]->m_id;

				cout << i << " : " << server_id << endl;*/

				//XMFLOAT4X4 world = network_manager::GetInst()->m_monster_pool[i].world_pos;

				/*m_Monster[server_id]->SetRight(XMFLOAT3(world._11, world._12, world._13));
				m_Monster[server_id]->SetUp(XMFLOAT3(world._21, world._22, world._23));
				m_Monster[server_id]->SetLook(XMFLOAT3(world._31, world._32, world._33));
				m_Monster[server_id]->SetPostion(XMFLOAT3(world._41, world._42, world._43));

				m_Monster[server_id]->Animate(m_ElapsedTime, NULL);
				m_Monster[server_id]->UpdateTransform(NULL);
				m_Monster[server_id]->Render(CommandList);*/


				/*m_Monster[i]->m_id = network_manager::GetInst()->m_monster_pool[i].id;

				int server_id = m_Monster[i]->m_id;*/

				/*if (network_manager::GetInst()->m_monster_pool[i].animation_state != 0)
					m_Monster[m_Monster[i]->m_id]->SetEnable(network_manager::GetInst()->m_monster_pool[i].animation_state);*/

					//XMFLOAT4X4 world = network_manager::GetInst()->m_monster_pool[i].world_pos;

					/*m_Monster[server_id]->SetRight(XMFLOAT3(world._11, world._12, world._13));
					m_Monster[server_id]->SetUp(XMFLOAT3(world._21, world._22, world._23));
					m_Monster[server_id]->SetLook(XMFLOAT3(world._31, world._32, world._33));
					m_Monster[server_id]->SetPostion(XMFLOAT3(world._41, world._42, world._43));

					m_Monster[server_id]->Animate(m_ElapsedTime, NULL);
					m_Monster[server_id]->UpdateTransform(NULL);
					m_Monster[server_id]->Render(CommandList);*/

					/*if (m_Orc) m_Orc->Render(CommandList);
					if (m_StrongOrc) m_StrongOrc->Render(CommandList);
					if (m_Shaman) m_Shaman->Render(CommandList);
					if (m_WolfRider) m_WolfRider->Render(CommandList);*/



	// Ohter Player
	if (network_manager::GetInst()->IsConnect()) {
		m_OtherPlayerModel->Render(CommandList);
	}

	// Effect
	shoot_time += m_ElapsedTime * 10;
	if (m_BulletEffect) {
		if (is_shoot == 0 || is_shoot == 2 || is_shoot == 4) {
			GameObject *WeaponObj= m_Player->GetPlayerWeapon();
			
			if (WeaponObj != NULL) {
				XMFLOAT4X4 Respos = WeaponObj->m_TransformPos;
				//cout << WeaponObj->GetFrameName() <<endl;
				XMFLOAT4X4 pos = Matrix4x4::Multiply(Respos, m_Player->m_WorldPos);
				m_BulletEffect->SetRight(XMFLOAT3(pos._11, pos._12, pos._13));
				m_BulletEffect->SetUp(XMFLOAT3(pos._21, pos._22, pos._23));
				m_BulletEffect->SetLook(XMFLOAT3(pos._31, pos._22, pos._33));
				XMFLOAT3 position = Vector3::Add(XMFLOAT3(pos._41, pos._42, pos._43), Vector3::ScalarProduct(m_BulletEffect->GetRight(), 8));
				position = Vector3::Add(position, Vector3::ScalarProduct(m_BulletEffect->GetLook(), 15));
				m_BulletEffect->SetPostion(position);
	
				m_BulletEffect->Render(CommandList);
			}
		}
	}
	if (shoot_time > 0.5f && is_shoot > -1) {
		shoot_time = 0.f;
		is_shoot -= 1;
	}

	if (is_Reload == 0 || is_Reload == 2) {
		if (18 <= m_Player->GetNowAnimationNum() && m_Player->GetNowAnimationNum() <= 26) {
			int bul = m_Player->GetPlayerBullet();
			if (bul > 3) {
				m_Player->SetPlayerBullet(9);
				is_Reload -= 1;
			}
			else {
				m_Player->SetPlayerBullet(bul + 5);
				is_Reload -= 1;
			}
		}
	}
	if (is_Reload > -1) {
		Reload_time += m_ElapsedTime * 10;
		if (Reload_time > 10.0f) {
			Reload_time = 0.0f;
			is_Reload -= 1;
		}
	}

	// Portal Rendering
	if (m_Portal) {
		m_Portal->SetRotate(0.f, 0.f, 10.f * m_ElapsedTime);
		m_Portal->Render(CommandList);
	}
}

void GameScene::CheckTile()
{

}

void GameScene::CheckBuildTrap()
{
	if (m_bClick) {
		Camera *pCamera = m_Player->GetCamera();
		XMFLOAT3 StartPos = pCamera->GetPosition();

		XMFLOAT3 EndPos;
		EndPos = Vector3::Normalize(pCamera->GetLook());

		if (m_target->GetIsBuildTrap()) {
			GameObject *TileObject = new GameObject();
			switch (m_MapNum) {
			case 1:
				switch (m_target->m_nTrapKind) {
				case TRAP_NEEDLE:
				case TRAP_SLOW:
					TileObject = m_Stage_02->CheckTileBound(StartPos, EndPos, true);
					break;
				case TRAP_FIRE:
				case TRAP_ARROW:
					TileObject = m_Stage_02->CheckTileBound(StartPos, EndPos, false);
					break;
				}
				break;
			case 2:
				switch (m_target->m_nTrapKind) {
				case TRAP_NEEDLE:
				case TRAP_SLOW:
					TileObject = m_Stage_03->CheckTileBound(StartPos, EndPos, true);
					break;
				case TRAP_FIRE:
				case TRAP_ARROW:
					TileObject = m_Stage_03->CheckTileBound(StartPos, EndPos, false);
					break;
				}
				break;
			case 3:
				switch (m_target->m_nTrapKind) {
				case TRAP_NEEDLE:
				case TRAP_SLOW:
					TileObject = m_Stage_04->CheckTileBound(StartPos, EndPos, true);
					break;
				case TRAP_FIRE:
				case TRAP_ARROW:
					TileObject = m_Stage_04->CheckTileBound(StartPos, EndPos, false);
					break;
				}
				break;
			}

			if (TileObject != NULL) {
				m_target->AccessTrap(true);
				BoundingBox BoundTile = TileObject->GetMesh()->GetBounds();
				BoundTile.Transform(BoundTile, XMLoadFloat4x4(&TileObject->m_WorldPos));
				XMFLOAT3 TilePos = BoundTile.Center;
				bool IsTrapPlaced = false;

				if (m_target->m_nTrapKind == TRAP_FIRE || m_target->m_nTrapKind == TRAP_ARROW) { // 벽타일
					if (BoundTile.Extents.x < BoundTile.Extents.z) {
						if (StartPos.x < TilePos.x) {   //플레이어스폰기준 우측에 놓을때. 함정이 -x축을 바라봄
							m_target->SetLook(XMFLOAT3(0.0f, 0.0f, 1.0f));
							m_target->SetRight(XMFLOAT3(0.0f, 1.0f, 0.0f));
							m_target->SetUp(XMFLOAT3(-1.0f, 0.0f, 0.0f));
							TilePos.x -= 15.0f;
						}
						else {                      // 플레이어스폰기준 좌측에 놓을때. 함정이 +x축을 바라봄
							m_target->SetLook(XMFLOAT3(0.0f, 0.0f, 1.0f));
							m_target->SetRight(XMFLOAT3(0.0f, -1.0f, 0.0f));
							m_target->SetUp(XMFLOAT3(1.0f, 0.0f, 0.0f));
							TilePos.x += 15.0f;
						}
					}
					else {
						if (StartPos.z < TilePos.z) {   // 플레이어스폰기준 정면에 놓을때. 함정이 -z축을 바라봄
							m_target->SetLook(XMFLOAT3(0.0f, 1.0f, 0.0f));
							m_target->SetRight(XMFLOAT3(1.0f, 0.0f, 0.0f));
							m_target->SetUp(XMFLOAT3(0.0f, 0.0f, -1.0f));
							TilePos.z -= 15.0f;
						}
						else {                     // 플레이어스폰기준 후방에 놓을때. +z축을 바라봄
							m_target->SetLook(XMFLOAT3(0.0f, -1.0f, 0.0f));
							m_target->SetRight(XMFLOAT3(1.0f, 0.0f, 0.0f));
							m_target->SetUp(XMFLOAT3(0.0f, 0.0f, 1.0f));
							TilePos.z += 15.0f;
						}
					}
				}
				else if (m_target->m_nTrapKind == TRAP_NEEDLE || m_target->m_nTrapKind == TRAP_SLOW) {
					TilePos.y += 10.0f;
				}
				m_target->SetPostion(TilePos);

				for (auto iter = m_Trap.begin(); iter != m_Trap.end(); ++iter) {
					if ((*iter)->is_active == false) continue;
					if ((*iter) != m_target) {
						if (XMVector3Equal(XMLoadFloat3(&(*iter)->GetPosition()), XMLoadFloat3(&TilePos))) {
							m_target->MoveUp(5.f);
							m_target->is_collision = true;
							IsTrapPlaced = true;
							break;
						}
					}
				}
				m_target->UpdateTransform(NULL);
				if (IsTrapPlaced == true) {
					m_target->SetRed(0x01);
				}
				else {
					m_target->SetRed(0x00);
					m_target->is_collision = false;
				}
			}
			else {
				m_target->AccessTrap(false);
				m_target->SetPostion(XMFLOAT3(0.0f, -1000.0f, 0.0f));
			}
		}
	}
}

void GameScene::ProcessInput(HWND hWnd)
{
	if (m_Player->GetPlayerLife() == 0) return;

	float xDelta = 0.f, yDelta = 0.f;
	POINT CursourPos;

	if (GetCapture() == hWnd) {
		SetCursor(NULL);
		GetCursorPos(&CursourPos);
		xDelta = (float)(CursourPos.x - m_ptOldCursorPos.x) / 10.0f;
		yDelta = (float)(CursourPos.y - m_ptOldCursorPos.y) / 10.0f;
		SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
	}

	if (xDelta != 0.f || yDelta != 0.f) {
		if (xDelta || yDelta)
			m_Player->PRotate(yDelta, xDelta, 0.f);
	}
}

bool GameScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_Player->GetPlayerLife() == 0) return 0;

	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
		// 플레이어가 공격 버튼을 클릭했을 때, 함정 설치 중이었다면 더 이상 따라오지 않도록 함
		if (m_bClick) {
			if (m_target != NULL)
				if (m_target->GetIsTrapAccess() == true && m_target->GetIsBuildTrap() == true && m_target->is_collision == false) {

					network_manager::GetInst()->send_install_trap(m_target->m_id, m_target->m_nTrapKind, m_target->m_WorldPos);

					//if (m_target->m_nTrapKind == TRAP_FIRE) {

					//	int mul = 0;

					//	for (int i = 0; i < 10 * 25; i += 10) {
					//		if (m_Fire[i]->is_set == false) break;
					//		++mul;
					//	}
					//	
					//	int fire_id = mul * 10;

					//	XMFLOAT3 right = m_target->GetRight();

					//	// 함정이 -x축을 바라볼 때
					//	if (right.y == 1) {
					//		m_Fire[fire_id]->m_dir = 1;
					//	}
					//	// 함정이 +x축을 바라볼 때
					//	else if (right.y == -1) {
					//		m_Fire[fire_id]->m_dir = 2;
					//	}
					//	else
					//	{
					//		XMFLOAT3 Up = m_target->GetUp();
					//		// 함정이 -z축을 바라볼 때
					//		if (Up.z == -1) m_Fire[fire_id]->m_dir = 3;
					//		else m_Fire[fire_id]->m_dir = 4;
					//	}

					//	int range = 0;

					//	for (int i = fire_id; i < fire_id + 5; ++i) {
					//		m_Fire[i]->is_set = true;
					//		m_Fire[i]->m_flagID = fire_id;
					//		m_Fire[i]->m_dir = m_Fire[fire_id]->m_dir;
					//		m_Fire[i]->trap_id = m_target->m_id;
					//		XMFLOAT3 position = m_target->GetPosition();
					//		if (m_Fire[fire_id]->m_dir == 1 || m_Fire[fire_id]->m_dir == 2) {
					//			position.x -= 200.f - (range * 50);
					//		}
					//		else {
					//			position.z -= 200.f - (range * 50);
					//		}
					//		position.y = 0.f + rand() % 30;
					//		m_Fire[i]->SetPostion(position);

					//		++range;
					//	}
					//}

					m_target->BuildTrap(false);
					m_target = NULL;
				}
			/*if (m_Needle->m_Trap.back()->GetIsTrapAccess() == true && m_Needle->m_Trap.back()->GetIsBuildTrap() == true)
				m_Needle->m_Trap.back()->BuildTrap(false);
			break;
		case TRAP_FIRE:
			if (m_Fire->m_Trap.back()->GetIsTrapAccess() == true && m_Fire->m_Trap.back()->GetIsBuildTrap() == true)
				m_Fire->m_Trap.back()->BuildTrap(false);
			break;
		case TRAP_SLOW:
			if (m_Slow->m_Trap.back()->GetIsTrapAccess() == true && m_Slow->m_Trap.back()->GetIsBuildTrap() == true)
				m_Slow->m_Trap.back()->BuildTrap(false);
			break;
		case TRAP_ARROW:
			if (m_Arrow->m_Trap.back()->GetIsTrapAccess() == true && m_Arrow->m_Trap.back()->GetIsBuildTrap() == true)
				m_Arrow->m_Trap.back()->BuildTrap(false);
			break;*/

			//m_TrapType = -1;
		}
		else {
			int bul = m_Player->GetPlayerBullet();
			if (m_Player->GetNowAnimationNum() < 18 && m_Player->GetNextAnimationNum() < 18 && bul > -1) {
				is_shoot = 4;
				m_Player->SetPlayerAnimateType(ANIMATION_TYPE_SHOOT);
				m_Player->SetEnable(9);
				m_Player->SetPlayerBullet(--bul);

				Camera *pCamera = m_Player->GetCamera();
				XMFLOAT3 StartPos = pCamera->GetPosition();
				XMFLOAT3 EndPos;
				EndPos = Vector3::Normalize(pCamera->GetLook());

				float ResultDistance = 10000;
				int HitIndex = 0;

				GameObject *MonObj = new GameObject();
				GameObject *ResultObj = new GameObject();

				for (int i = 0; i < m_Orc.size(); ++i) {
					MonObj = m_Orc[i]->CheckMonster(StartPos, EndPos);
					if (MonObj != NULL) {
						if (ResultObj == NULL || MonObj->GetMesh()->m_fDistance < ResultDistance) {
							ResultObj = MonObj;
							ResultDistance = ResultObj->GetMesh()->m_fDistance;
							HitIndex = i;
						}
					}
				}
				MonObj = NULL;
				for (int i = 0; i < m_StrongOrc.size(); ++i) {
					MonObj = m_StrongOrc[i]->CheckMonster(StartPos, EndPos);
					if (MonObj != NULL) {
						if (ResultObj == NULL || MonObj->GetMesh()->m_fDistance < ResultDistance) {
							ResultObj = MonObj;
							ResultDistance = ResultObj->GetMesh()->m_fDistance;
							HitIndex = i;
						}
					}
				}
				MonObj = NULL;
				for (int i = 0; i < m_WolfRider.size(); ++i) {
					MonObj = m_WolfRider[i]->CheckMonster(StartPos, EndPos);
					if (MonObj != NULL) {
						if (ResultObj == NULL || MonObj->GetMesh()->m_fDistance < ResultDistance) {
							ResultObj = MonObj;
							ResultDistance = ResultObj->GetMesh()->m_fDistance;
							HitIndex = i;
						}
					}
				}

				GameObject *TileObj = new GameObject();

				switch (m_MapNum) {
				case 1:
					TileObj = m_Stage_02->CheckTileBound(StartPos, EndPos, false);
					break;
				case 2:
					TileObj = m_Stage_03->CheckTileBound(StartPos, EndPos, false);
					break;
				case 3:
					TileObj = m_Stage_04->CheckTileBound(StartPos, EndPos, false);
					break;
				}

				if ((TileObj == NULL && ResultObj != NULL) || (TileObj != NULL && ResultObj != NULL && TileObj->GetMesh()->m_fDistance > ResultDistance)) {
					if (strstr(ResultObj->GetFrameName(), "Head")) {
						switch (ResultObj->m_Type) {
						case TYPE_ORC:
						{
							m_Orc[HitIndex]->m_RedHit = true;
							network_manager::GetInst()->send_shoot(m_Orc[HitIndex]->m_id, true);
						}
						break;
						case TYPE_STRONGORC:
						{
							m_StrongOrc[HitIndex]->m_RedHit = true;
							network_manager::GetInst()->send_shoot(m_StrongOrc[HitIndex]->m_id, true);
						}
						break;
						case TYPE_RIDER:
						{
							m_WolfRider[HitIndex]->m_RedHit = true;
							network_manager::GetInst()->send_shoot(m_WolfRider[HitIndex]->m_id, true);
						}
						break;
						}
					}
					else {
						switch (ResultObj->m_Type) {
						case TYPE_ORC:
						{
							m_Orc[HitIndex]->m_RedHit = true;
							network_manager::GetInst()->send_shoot(m_Orc[HitIndex]->m_id, false);
						}
						break;
						case TYPE_STRONGORC:
						{
							m_StrongOrc[HitIndex]->m_RedHit = true;
							network_manager::GetInst()->send_shoot(m_StrongOrc[HitIndex]->m_id, false);
						}
						break;
						case TYPE_RIDER:
						{
							m_WolfRider[HitIndex]->m_RedHit = true;
							network_manager::GetInst()->send_shoot(m_WolfRider[HitIndex]->m_id, false);
						}
						break;
						}
					}
				}
			}
			else if (m_Player->GetNowAnimationNum() < 9 && m_Player->GetNextAnimationNum() < 9 && bul == -1) {
				is_Reload = 4;
				m_Player->SetPlayerAnimateType(ANIMATION_TYPE_RELOAD);
				m_Player->SetEnable(18);
			}
		}
		break;

	case WM_LBUTTONUP:
		//m_Player->SetEnable(0);
		//::ReleaseCapture();
		break;

	default:
		break;
	}

	return false;
}

bool GameScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_Player->GetPlayerLife() == 0) return 0;

	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'w':
		case 'W':
		{
			m_Player->SetMoveUp(true);
			break;
		}
		case 's':
		case 'S':
		{
			m_Player->SetMoveDown(true);
			break;
		}
		case 'a':
		case 'A':
		{
			m_Player->SetMoveLeft(true);
		}
		break;
		case 'd':
		case 'D':
		{
			m_Player->SetMoveRight(true);
		}
		break;
		case 'r':
		case 'R':
			if (m_Player->GetPlayerBullet() < 9 && m_Player->GetNextAnimationNum() < 9 && m_Player->GetNowAnimationNum() < 9) {
				is_Reload = 4;
				m_Player->SetPlayerAnimateType(ANIMATION_TYPE_RELOAD);
				m_Player->SetEnable(18);
			}
			break;

			// 함정 설치
		case '1':
		{
			if (m_bClick) break;

			m_bClick = true;

			for (int i = 0; i < MAX_TRAP; ++i) {
				if (m_Trap[i]->is_active == false && m_Trap[i]->m_nTrapKind == TRAP_NEEDLE) {
					m_Trap[i]->is_active = true;
					m_Trap[i]->BuildTrap(true);
					m_target = m_Trap[i];
					break;
				}
			}
			//if (m_bClick) {
			//	if (m_TrapType == TRAP_NEEDLE) {
			//		break; //가시 함정이 이미 만들어진 상태
			//	}
			//	else {
			//		// 다른 함정이 이미 만들어진 상태 -> 삭제하고 가시함정 생성
			//		switch (m_TrapType) {
			//		case TRAP_FIRE:
			//			if (m_Fire->m_Trap.back()->GetIsBuildTrap() == true) {
			//				m_Fire->m_Trap.pop_back();
			//				m_Needle->BuildTrap(TRAP_NEEDLE);
			//			}
			//			break;
			//		case TRAP_SLOW:
			//			if (m_Slow->m_Trap.back()->GetIsBuildTrap() == true) {
			//				m_Slow->m_Trap.pop_back();
			//				m_Needle->BuildTrap(TRAP_NEEDLE);
			//			}
			//			break;
			//		case TRAP_ARROW:
			//			if (m_Arrow->m_Trap.back()->GetIsBuildTrap() == true) {
			//				m_Arrow->m_Trap.pop_back();
			//				m_Needle->BuildTrap(TRAP_NEEDLE);
			//			}
			//			break;
			//		case -1:
			//			//m_Needle->BuildTrap(TRAP_NEEDLE);
			//			break;
			//		}
			//		m_TrapType = TRAP_NEEDLE;
			//		break;
			//	}
			//}
			//else {
			//	m_bClick = true;
			//	m_Needle->BuildTrap(TRAP_NEEDLE);
			//	m_TrapType = TRAP_NEEDLE;
			//}
		}
		break;

		case '2':
		{
			if (m_bClick) break;

			m_bClick = true;

			for (int i = 0; i < MAX_TRAP; ++i) {
				if (m_Trap[i]->is_active == false && m_Trap[i]->m_nTrapKind == TRAP_FIRE) {
					m_Trap[i]->is_active = true;
					m_Trap[i]->BuildTrap(true);
					m_target = m_Trap[i];
					break;
				}
			}
			//if (m_bClick) {
			//	if (m_TrapType == TRAP_FIRE) {
			//		break; //불 함정이 이미 만들어진 상태
			//	}
			//	else {
			//		// 다른 함정이 이미 만들어진 상태 -> 삭제하고 불함정 생성
			//		switch (m_TrapType) {
			//		case TRAP_NEEDLE:
			//			if (m_Needle->m_Trap.back()->GetIsBuildTrap() == true) {
			//				m_Needle->m_Trap.pop_back();
			//				m_Fire->BuildTrap(TRAP_FIRE);
			//			}
			//			break;
			//		case TRAP_SLOW:
			//			if (m_Slow->m_Trap.back()->GetIsBuildTrap() == true) {
			//				m_Slow->m_Trap.pop_back();
			//				m_Fire->BuildTrap(TRAP_FIRE);
			//			}
			//			break;
			//		case TRAP_ARROW:
			//			if (m_Arrow->m_Trap.back()->GetIsBuildTrap() == true) {
			//				m_Arrow->m_Trap.pop_back();
			//				m_Fire->BuildTrap(TRAP_FIRE);
			//			}
			//			break;
			//		case -1:
			//			m_Fire->BuildTrap(TRAP_FIRE);
			//			break;
			//		}
			//		m_TrapType = TRAP_FIRE;
			//		break;
			//	}
			//}
			//else {
			//	m_bClick = true;
			//	m_Fire->BuildTrap(TRAP_FIRE);
			//	m_TrapType = TRAP_FIRE;
			//}
		}
		break;

		case '3':
		{
			if (m_bClick) break;

			m_bClick = true;

			for (int i = 0; i < MAX_TRAP; ++i) {
				if (m_Trap[i]->is_active == false && m_Trap[i]->m_nTrapKind == TRAP_SLOW) {
					m_Trap[i]->is_active = true;
					m_Trap[i]->BuildTrap(true);
					m_target = m_Trap[i];
					break;
				}
			}
			//if (m_bClick) {
			//	if (m_TrapType == TRAP_SLOW) {
			//		break; //불 함정이 이미 만들어진 상태
			//	}
			//	else {
			//		// 다른 함정이 이미 만들어진 상태 -> 삭제하고 불함정 생성
			//		switch (m_TrapType) {
			//		case TRAP_NEEDLE:
			//			if (m_Needle->m_Trap.back()->GetIsBuildTrap() == true) {
			//				m_Needle->m_Trap.pop_back();
			//				m_Slow->BuildTrap(TRAP_SLOW);
			//			}
			//			break;
			//		case TRAP_FIRE:
			//			if (m_Fire->m_Trap.back()->GetIsBuildTrap() == true) {
			//				m_Fire->m_Trap.pop_back();
			//				m_Slow->BuildTrap(TRAP_SLOW);
			//			}
			//			break;
			//		case TRAP_ARROW:
			//			if (m_Arrow->m_Trap.back()->GetIsBuildTrap() == true) {
			//				m_Arrow->m_Trap.pop_back();
			//				m_Slow->BuildTrap(TRAP_SLOW);
			//			}
			//			break;
			//		case -1:
			//			m_Slow->BuildTrap(TRAP_SLOW);
			//			break;
			//		}
			//		m_TrapType = TRAP_SLOW;
			//		break;
			//	}
			//}
			//else {
			//	m_bClick = true;
			//	m_Slow->BuildTrap(TRAP_SLOW);
			//	m_TrapType = TRAP_SLOW;
			//}
		}
		break;

		case '4':
		{
			if (m_bClick) break;

			m_bClick = true;

			for (int i = 0; i < MAX_TRAP; ++i) {
				if (m_Trap[i]->is_active == false && m_Trap[i]->m_nTrapKind == TRAP_ARROW) {
					m_Trap[i]->is_active = true;
					m_Trap[i]->BuildTrap(true);
					m_target = m_Trap[i];
					break;
				}
			}
			//if (m_bClick) {
			//	if (m_TrapType == TRAP_ARROW) {
			//		break; //불 함정이 이미 만들어진 상태
			//	}
			//	else {
			//		// 다른 함정이 이미 만들어진 상태 -> 삭제하고 불함정 생성
			//		switch (m_TrapType) {
			//		case TRAP_NEEDLE:
			//			if (m_Needle->m_Trap.back()->GetIsBuildTrap() == true) {
			//				m_Needle->m_Trap.pop_back();
			//				m_Arrow->BuildTrap(TRAP_ARROW);
			//			}
			//			break;
			//		case TRAP_FIRE:
			//			if (m_Fire->m_Trap.back()->GetIsBuildTrap() == true) {
			//				m_Fire->m_Trap.pop_back();
			//				m_Arrow->BuildTrap(TRAP_ARROW);
			//			}
			//			break;
			//		case TRAP_SLOW:
			//			if (m_Slow->m_Trap.back()->GetIsBuildTrap() == true) {
			//				m_Slow->m_Trap.pop_back();
			//				m_Arrow->BuildTrap(TRAP_ARROW);
			//			}
			//			break;
			//		case -1:
			//			m_Arrow->BuildTrap(TRAP_ARROW);
			//			break;
			//		}
			//		m_TrapType = TRAP_ARROW;
			//		break;
//	}
//}
//else {
//	m_bClick = true;
//	m_Arrow->BuildTrap(TRAP_ARROW);
//	m_TrapType = TRAP_ARROW;
//}
		}
		break;

		case '5':
		{
			int hp = m_Player->GetPlayerLife();
			if (hp > 0)
				m_Player->SetPlayerLife(--hp);
			int bul = m_Player->GetPlayerBullet();
			if (bul > 0)
				m_Player->SetPlayerBullet(--bul);
		}
		break;


		case '6':
		{
			int hp = m_Player->GetPlayerLife();
			if (hp < 9)
				m_Player->SetPlayerLife(++hp);
			int bul = m_Player->GetPlayerBullet();
			if (bul < 9)
				m_Player->SetPlayerBullet(++bul);
		}
		break;

		case VK_SHIFT:
			if (m_Player->GetForwardInfo()) {
				//m_Player->SetMoveRoll(true);
				m_Player->SetAnimateType(27, ANIMATION_TYPE_ONCE);
				m_Player->SetEnable(27);
				network_manager::GetInst()->send_my_world_pos_packet(m_Player->m_TransformPos, 27);
			}
			break;
		default:
			break;
		}
		break;

	case WM_KEYUP:
		switch (wParam) {
		case '1':
		case '2':
		case '3':
		case '4':
			if (m_bClick) {
				m_bClick = false;
				if (m_target != NULL) {
					m_target->is_active = false;
					m_target = NULL;
				}
			}
			//if (m_bClick) {
			//	if (m_Needle->m_Trap.back()->GetIsBuildTrap() == true) {
			//		m_Needle->m_Trap.pop_back();
			//		m_TrapType = -1;
			//		m_bClick = false;
			//		cout << "trap back 삭제 size: " << m_Needle->m_Trap.size() << endl;
			//	}
			//	else {
			//		// 함정 배치가 완료된 경우
			//		if (m_TrapType == TRAP_NEEDLE) {
			//			m_TrapType = -1;
			//			m_bClick = false;
			//		}
			//		else if (m_TrapType == -1) {
			//			m_bClick = false;
			//		}
			//	}
			//}
			break;

		case 'w':
		case 'W':
			m_Player->SetMoveUp(false);
			break;

		case 's':
		case 'S':
			m_Player->SetMoveDown(false);
			break;

		case 'a':
		case 'A':
			m_Player->SetMoveLeft(false);
			break;

		case 'd':
		case 'D':
			m_Player->SetMoveRight(false);
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
	return false;
}

void GameScene::Monster_Function(ID3D12GraphicsCommandList *CommandList, vector<Monster*> Orc)
{
	for (int i = 0; i < Orc.size(); ++i) {

		switch (Orc[i]->m_Type) {
		case TYPE_ORC:
			if (network_manager::GetInst()->m_orcPool[i].isLive == false) continue;
			break;

		case TYPE_STRONGORC:
			if (network_manager::GetInst()->m_strongorcPool[i].isLive == false) continue;
			break;

		case TYPE_RIDER:
			if (network_manager::GetInst()->m_riderPool[i].isLive == false) continue;
			break;
		}

		// Monster Animation Set
		int server_anim = 0;
		switch (Orc[i]->m_Type) {
		case TYPE_ORC:
			server_anim = network_manager::GetInst()->m_orcPool[i].animation_state;
			break;

		case TYPE_STRONGORC:
			server_anim = network_manager::GetInst()->m_strongorcPool[i].animation_state;
			break;

		case TYPE_RIDER:
			server_anim = network_manager::GetInst()->m_riderPool[i].animation_state;
			break;
		}

		if (server_anim <= 0 && server_anim > 30) server_anim = 0;

		if (server_anim != 0 && server_anim != Orc[i]->GetNowAnimationNum()) {
			if (server_anim == M_ANIM_DEATH) {
				Orc[i]->SetAnimateType(server_anim, ANIMATION_TYPE_ONCE);
				Orc[i]->SetEnable(server_anim);
			}
			else
				Orc[i]->SetEnable(server_anim);
		}

		// Monster Pos Set
		XMFLOAT4X4 world{};

		int monster_id = 0;

		switch (Orc[i]->m_Type) {
		case TYPE_ORC:
		{
			Orc[i]->m_id = network_manager::GetInst()->m_orcPool[i].id;
			world = network_manager::GetInst()->m_orcPool[i].world_pos;
		}
		break;

		case TYPE_STRONGORC:
		{
			Orc[i]->m_id = network_manager::GetInst()->m_strongorcPool[i].id;
			world = network_manager::GetInst()->m_strongorcPool[i].world_pos;
		}
		break;

		case TYPE_RIDER:
		{
			Orc[i]->m_id = network_manager::GetInst()->m_riderPool[i].id;
			world = network_manager::GetInst()->m_riderPool[i].world_pos;
		}
		break;
		}
		if (Orc[i]->m_RedHit == true) Orc[i]->Monster_Hit(m_ElapsedTime);

		Orc[i]->SetRight(XMFLOAT3(world._11, world._12, world._13));
		Orc[i]->SetUp(XMFLOAT3(world._21, world._22, world._23));
		Orc[i]->SetLook(XMFLOAT3(world._31, world._32, world._33));
		Orc[i]->SetPostion(XMFLOAT3(world._41, world._42, world._43));

		Orc[i]->Animate(m_ElapsedTime, NULL);
		Orc[i]->UpdateTransform(NULL);
		Orc[i]->Render(CommandList);
	}
}