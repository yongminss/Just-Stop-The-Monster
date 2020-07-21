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

// ���� ���� ��, ������ Ÿ��Ʋ ȭ���� Ŭ����
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

	CreateCbvSrvDescriptorHeap(Device, CommandList, 0, 6);

	m_Viewport = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.f, 1.f };
	m_ScissorRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };

	// TitleScene ���� Redering �� Objects
	m_Background = new UI(Device, CommandList, m_GraphicsRootSignature, 1.f, 1.f, BackGround, 0);
	m_RoomList = new UI(Device, CommandList, m_GraphicsRootSignature, 1.f, 1.f, RoomList, 0);
	m_StageSelect = new UI(Device, CommandList, m_GraphicsRootSignature, 0.45f, 0.2f, Select_Stage, 0);
	m_WeaponSkill = new UI(Device, CommandList, m_GraphicsRootSignature, 0.35f, 0.6f, Select_WeaponAndSkill, 0);
	m_PlayerInfo = new UI(Device, CommandList, m_GraphicsRootSignature, 0.35f, 0.75f, PlayerInfo, 0);
}

void TitleScene::ReleaseObject()
{
	if (m_GraphicsRootSignature) m_GraphicsRootSignature->Release();

	if (m_Background) delete m_Background;

	if (m_RoomList) delete m_RoomList;
}

ID3D12RootSignature *TitleScene::CreateGraphicsRootSignature(ID3D12Device *Device)
{
	ID3D12RootSignature *GraphicsRootSignature = NULL;

	// ��ũ���� ������
	D3D12_DESCRIPTOR_RANGE DescriptorRange;
	DescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	DescriptorRange.NumDescriptors = 1;
	DescriptorRange.BaseShaderRegister = 0;
	DescriptorRange.RegisterSpace = 0;
	DescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ��Ʈ �Ķ����
	D3D12_ROOT_PARAMETER RootParameter[2];
	RootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	RootParameter[0].Descriptor.ShaderRegister = 1; // Camera
	RootParameter[0].Descriptor.RegisterSpace = 0;
	RootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// TitleScene���� �� ��Ʈ �Ķ���͸� ���������
	// Render �ÿ� Root32BitConstants�� �Ѱ��ִ� �� ������ 0�� �Ķ���Ϳ� �ʿ� ���� �� ����
	RootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	RootParameter[1].DescriptorTable.NumDescriptorRanges = 1;
	RootParameter[1].DescriptorTable.pDescriptorRanges = &DescriptorRange;
	RootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// ���÷�
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
	// ��Ʈ �ñ׳��ĸ� ����
	CommandList->SetGraphicsRootSignature(m_GraphicsRootSignature);
	// ��ũ���� �� ����
	CommandList->SetDescriptorHeaps(1, &m_CbvSrvDescriptorHeap);

	// ����Ʈ�� ������Ʈ ������ ����
	CommandList->RSSetViewports(1, &m_Viewport);
	CommandList->RSSetScissorRects(1, &m_ScissorRect);

	// Player�� ���ÿ� ���� �׷��� ���
	switch (m_state) {
	case Basic:
		if (m_Background) m_Background->Render(CommandList);
		break;

	case Select_Room:
		if (m_RoomList) m_RoomList->Render(CommandList);
		if (m_Background) m_Background->Render(CommandList);
		break;

	case Wait_Room:
		if (m_StageSelect) m_StageSelect->Render(CommandList);
		if (m_WeaponSkill) m_WeaponSkill->Render(CommandList);
		if (m_PlayerInfo) m_PlayerInfo->Render(CommandList);

		if (m_Background) m_Background->Render(CommandList);
		break;

	default:
		break;

	}
	// ���� ������ ������Ʈ���� ������
	if (PLAYER_STATE_in_room == network_manager::GetInst()->m_my_info.player_state) {
	}
}

bool TitleScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	POINT MousePos{};
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		MousePos.x = LOWORD(lParam);
		MousePos.y = HIWORD(lParam);
		cout << "x : " << MousePos.x << ", y : " << MousePos.y << endl;

		switch (m_state) {
		case Basic:
			// Single Game ��ư�� Ŭ���ϸ� Game Start
			if (MousePos.x > 290 && MousePos.x < 510 && MousePos.y > 310 && MousePos.y < 360)
				m_StartGame = true;
			// Multi Game ��ư�� Ŭ���ϸ� Make Room
			else if (MousePos.x > 300 && MousePos.x < 510 && MousePos.y > 370 && MousePos.y < 420)
				m_state = Select_Room;
			// Exit ��ư�� ������ Game Over
			else if (MousePos.x > 360 && MousePos.x < 440 && MousePos.y > 430 && MousePos.y < 480)
				::PostQuitMessage(0);
			break;

		case Select_Room:
			// Make Room
			if (MousePos.x > 625 && MousePos.x < 785 && MousePos.y > 175 && MousePos.y < 220) {
				network_manager::GetInst()->send_make_room_packet();
				m_state = Wait_Room;
			}
			// Join Room
			if (MousePos.x > 240 && MousePos.x < 590 && MousePos.y > 70 && MousePos.y < 120)
				if (network_manager::GetInst()->m_vec_gameRoom.size() != NULL) {
					network_manager::GetInst()->send_request_join_room(network_manager::GetInst()->m_vec_gameRoom[0]->room_number);
					m_state = Wait_Room;
				}
			break;

		case Wait_Room:
			m_StartGame = true;
			break;

		default:
			break;
		}

	case WM_LBUTTONUP:
		break;

	default:
		break;
	}

	return false;
}

// �̱� �� ��Ƽ�� ĳ����, ���� ���� ���� ��, ������ �����ϴ� ���� ��
GameScene::GameScene()
{
	m_socket = network_manager::GetInst()->m_serverSocket;
}

GameScene::~GameScene()
{

}

void GameScene::BuildObject(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList)
{
	m_GraphicsRootSignature = CreateGraphicsRootSignature(Device);

	CreateCbvSrvDescriptorHeap(Device, CommandList, 0, 100);

	Material::PrepareShader(Device, CommandList, m_GraphicsRootSignature);

	BuildDefaultLightsAndMaterials();

	m_Player = new Player(Device, CommandList, m_GraphicsRootSignature);

	// UI
	m_CharInfo = new UI(Device, CommandList, m_GraphicsRootSignature, 0.4f, 0.125f, UI_PlayerInfo, 1);
	m_TrapListUi = new UI(Device, CommandList, m_GraphicsRootSignature, 0.3f, 0.125f, UI_TrapList, 1);

	// ��ī�̹ڽ�
	for (int i = 0; i < 5; ++i) m_SkyBox[i] = new SkyBox(Device, CommandList, m_GraphicsRootSignature, i);

	// ���������� ���� �ٴ�
	//m_StageWall = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Stage_Wall4.bin", NULL, false);
	//m_StageWall->SetPostion(XMFLOAT3(820.f, 150.f, -100.f));
	//m_StageWall->SetScale(100.f, 100.f, 100.f);

	//m_StageFloor = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/4.bin", NULL, false);
	//m_StageFloor->SetPostion(XMFLOAT3(2500.f, -50.f, 0.f));
	//m_StageFloor->SetScale(100.f, 100.f, 100.f);

	// ��������
	m_Stage_01 = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Stage01.bin", NULL, false);
	//m_Stage_01->SetRotate(90.0f, 0.0f, 0.0f);
	m_Stage_01->SetPostion(XMFLOAT3(0.f, -50.f, 0.f));
	//m_Stage_01->SetScale(100.f, 100.f, 100.f);

	m_Stage_02 = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Stage02.bin", NULL, false);
	//m_Stage_02->SetRotate(-90.0f, 0.0f,0.0f);
	m_Stage_02->SetPostion(XMFLOAT3(0.f, -50.f, 0.f));
	

	m_Stage_03 = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Stage03.bin", NULL, false);

	m_Stage_03->SetPostion(XMFLOAT3(0.f, -50.f, 0.f));
	
	m_Stage_04 = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Stage04.bin", NULL, false);
	//m_Stage_04->SetPostion(XMFLOAT3(-900.f, -50.f, 500.f));
	m_Stage_04->SetPostion(XMFLOAT3(-0.f, -50.f, 0.f));
	
	// ����
	m_NeedleTrapModel = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Trap_Needle.bin", NULL, true);
	m_NeedleTrapModel->SetScale(1.4f, 1.4f, 1.4f);

	m_FireTrapModel = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Trap_Fire.bin", NULL, false);
	m_FireTrapModel->SetScale(1.4f, 1.4f, 1.4f);

	m_SlowTrapModel = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Trap_Slow.bin", NULL, false);
	m_SlowTrapModel->SetScale(1.4f, 1.4f, 1.4f);
	
	m_ArrowTrapModel = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Trap_Arrow.bin", NULL, false);
	m_ArrowTrapModel->SetScale(1.4f, 1.4f, 1.4f);

	// ���� ��
	m_OrcModel = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Monster_Weak_Infantry.bin", NULL, true);
	m_ShamanModel = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Monster_Shaman.bin", NULL, true);
	m_WolfRiderModel = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Monster_WolfRider.bin", NULL, true);

	// �⺻ ��ũ
	m_Orc.emplace_back(new Monster());
	m_Orc.back()->SetChild(m_OrcModel, false);
	//m_Orc.back()->SetPostion(XMFLOAT3(2200.f, -50.f, -150.f));
	m_Orc.back()->SetPostion(XMFLOAT3(0.f, -50.f, 0.f));

	// ������ ��ũ
	m_Shaman.emplace_back(new Monster());
	m_Shaman.back()->SetChild(m_ShamanModel, true);
	m_Shaman.back()->SetPostion(XMFLOAT3(2200.f, -50.f, -400.f));

	// ���� ��ũ
	m_WolfRider.emplace_back(new Monster());
	m_WolfRider.back()->SetChild(m_WolfRiderModel, true);
	m_WolfRider.back()->SetPostion(XMFLOAT3(2200.f, -50.f, 1020.f));

	m_OtherPlayerModel = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Soldier_Player.bin", NULL, true);
	m_OtherPlayerModel->SetPostion(XMFLOAT3(-1000.f, -15.f, 0.f));

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

	// ���⼺ ����
	m_Lights[0].m_Enable = true;
	m_Lights[0].m_nType = DIRECTIONAL_LIGHT;
	m_Lights[0].m_Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.f);
	m_Lights[0].m_Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.f);
	m_Lights[0].m_Specular = XMFLOAT4(0.6f, 0.4f, 0.4f, 1.f);
	m_Lights[0].m_Direction = XMFLOAT3(1.f, 0.f, 0.f);

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

	// ��ũ���� ������
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

	// ��Ʈ �Ķ����
	D3D12_ROOT_PARAMETER RootParameter[13];
	RootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	RootParameter[0].Descriptor.ShaderRegister = 1; // Camera
	RootParameter[0].Descriptor.RegisterSpace = 0;
	RootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	RootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	RootParameter[1].Constants.Num32BitValues = 33;
	RootParameter[1].Constants.ShaderRegister = 2; // GameObject
	RootParameter[1].Constants.RegisterSpace = 0;
	RootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	RootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	RootParameter[2].DescriptorTable.NumDescriptorRanges = 1;
	RootParameter[2].DescriptorTable.pDescriptorRanges = &DescriptorRange[0]; // Texture
	RootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// bin���Ϸ� �ε��� ������Ʈ�� ���� ����
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

	// �ִϸ��̼ǿ��� ����� ��Ʈ �Ķ����
	RootParameter[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	RootParameter[10].Descriptor.ShaderRegister = 7; // Skinned Bone Offsets
	RootParameter[10].Descriptor.RegisterSpace = 0;
	RootParameter[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	RootParameter[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	RootParameter[11].Descriptor.ShaderRegister = 8; // Skinned Bone Transforms
	RootParameter[11].Descriptor.RegisterSpace = 0;
	RootParameter[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// ����
	RootParameter[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	RootParameter[12].Descriptor.ShaderRegister = 4;
	RootParameter[12].Descriptor.RegisterSpace = 0;
	RootParameter[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// ���÷�
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
		m_Player->UpdateTransform(NULL);
		m_Player->Update(ElapsedTime);
		XMFLOAT3 p_pos = m_Player->GetPosition();
		cout << "x: " << p_pos.x << ", z: " << p_pos.z << endl;
	}

	for (auto iter = m_Trap.begin(); iter != m_Trap.end(); ++iter)
		if (*iter) {
			(*iter)->UpdateTransform(NULL);
			(*iter)->Animate(m_Player, ElapsedTime, NULL);
		}

	for (auto iter = m_Orc.begin(); iter != m_Orc.end(); ++iter)
		if (*iter) {
			if (network_manager::GetInst()->m_monster_pool[0].isLive == false) {
				continue;
			}
			(*iter)->SetTransform(network_manager::GetInst()->m_monster_pool[0].world_pos);
			(*iter)->UpdateTransform(NULL);
			(*iter)->Animate(ElapsedTime, NULL);
			(*iter)->SetAnimateType(3, ANIMATION_TYPE_ONCE);
			cout << "ainm:" << network_manager::GetInst()->m_monster_pool[0].animation_state << endl;
			(*iter)->SetEnable(network_manager::GetInst()->m_monster_pool[0].animation_state);
			/*(*iter)->UpdateTransform(NULL);
			(*iter)->Animate(ElapsedTime, NULL);*/
			/*XMFLOAT3 PlayerPos = m_Player->GetPosition();
			XMFLOAT3 OrcPos = (*iter)->GetPosition();
			float DistanceWithPlayer = Vector3::Distance(PlayerPos, OrcPos);
			if (DistanceWithPlayer > 200.0f) {
				(*iter)->SetLine(ElapsedTime);
				(*iter)->MoveForward(200.f * ElapsedTime);
				(*iter)->SetEnable(2);
			}
			else if (DistanceWithPlayer <= 200.0f && DistanceWithPlayer >= 70.0f) {
				if ((*iter)->nCheckPoint != 0) { (*iter)->nCheckPoint = 0; (*iter)->nInporation = 0.0f; (*iter)->StartLook = (*iter)->GetLook(); }
				PlayerPos.y = -50.f;
				PlayerPos = Vector3::Subtract(PlayerPos, OrcPos);
				(*iter)->SetEnable(2);
				if ((*iter)->GetNowAnimationNum() == 2) {
					(*iter)->SetinterPolation(Vector3::Normalize(PlayerPos));
					(*iter)->MoveForward(200.f * ElapsedTime);
				}
			}
			else {
				(*iter)->SetDirection(PlayerPos);
				(*iter)->SetAnimateType(3, ANIMATION_TYPE_ONCE);
				(*iter)->SetEnable(3);
			}*/
		}

	for (auto iter = m_Shaman.begin(); iter != m_Shaman.end(); ++iter)
		if (*iter) {
			(*iter)->UpdateTransform(NULL);
			(*iter)->Animate(ElapsedTime, NULL);
			XMFLOAT3 PlayerPos = m_Player->GetPosition();
			XMFLOAT3 ShamanPos = (*iter)->GetPosition();
			float DistanceWithPlayer = Vector3::Distance(PlayerPos, ShamanPos);
			if (DistanceWithPlayer > 300.0f) {
				(*iter)->SetLine(ElapsedTime);
				(*iter)->MoveForward(200.f * ElapsedTime);
				(*iter)->SetEnable(2);
			}
			else if (DistanceWithPlayer <= 300.0f && DistanceWithPlayer >= 200.0f) {
				if ((*iter)->nCheckPoint != 0) { (*iter)->nCheckPoint = 0; (*iter)->nInporation = 0.0f; (*iter)->StartLook = (*iter)->GetLook(); }
				PlayerPos.y = -50.f;
				PlayerPos = Vector3::Subtract(PlayerPos, ShamanPos);
				(*iter)->SetEnable(2);
				if ((*iter)->GetNowAnimationNum() == 2) {
					(*iter)->SetinterPolation(Vector3::Normalize(PlayerPos));
					(*iter)->MoveForward(200.f * ElapsedTime);
				}
			}
			else {
				(*iter)->SetDirection(PlayerPos);
				(*iter)->SetAnimateType(3, ANIMATION_TYPE_ONCE);
				(*iter)->SetEnable(3);
			}
		}

	for (auto iter = m_WolfRider.begin(); iter != m_WolfRider.end(); ++iter)
		if (*iter) {
			(*iter)->UpdateTransform(NULL);
			(*iter)->Animate(ElapsedTime, NULL);
			XMFLOAT3 PlayerPos = m_Player->GetPosition();
			XMFLOAT3 WolfPos = (*iter)->GetPosition();
			float DistanceWithPlayer = Vector3::Distance(PlayerPos, WolfPos);
			if (DistanceWithPlayer > 200.0f) {
				(*iter)->SetLine(ElapsedTime);
				(*iter)->MoveForward(200.f * ElapsedTime);
				(*iter)->SetEnable(2);
			}
			else if (DistanceWithPlayer <= 200.0f && DistanceWithPlayer >= 90.0f) {
				if ((*iter)->nCheckPoint != 0) { (*iter)->nCheckPoint = 0; (*iter)->nInporation = 0.0f; (*iter)->StartLook = (*iter)->GetLook(); }
				PlayerPos.y = -50.f;
				PlayerPos = Vector3::Subtract(PlayerPos, WolfPos);
				(*iter)->SetEnable(2);
				if ((*iter)->GetNowAnimationNum() == 2) {
					(*iter)->SetinterPolation(Vector3::Normalize(PlayerPos));
					(*iter)->MoveForward(200.f * ElapsedTime);
				}
			}
			else {
				(*iter)->SetDirection(PlayerPos);
				(*iter)->SetAnimateType(3, ANIMATION_TYPE_ONCE);
				(*iter)->SetEnable(3);
			}
		}

	// Ohter Player
	if (network_manager::GetInst()->IsConnect()) {
		XMFLOAT4X4 Transform = network_manager::GetInst()->m_OtherInfo.Transform;

		AnimateState = int(network_manager::GetInst()->m_OtherInfo.AnimateState);

		m_OtherPlayerModel->SetTransform(Transform);
		m_OtherPlayerModel->SetScale(30.f, 30.f, 30.f);
		m_OtherPlayerModel->SetEnable(AnimateState);
		m_OtherPlayerModel->Animate(ElapsedTime, NULL);
	}
}

void GameScene::Render(ID3D12GraphicsCommandList *CommandList)
{
	// ��Ʈ �ñ׳��ĸ� ����
	CommandList->SetGraphicsRootSignature(m_GraphicsRootSignature);
	// ��ũ���� �� ����
	CommandList->SetDescriptorHeaps(1, &m_CbvSrvDescriptorHeap);

	// ����Ʈ�� ������Ʈ ������ ���� �� ���̴� ���� ����
	m_Player->UpdateCameraSet(CommandList);

	// ������ ����ϱ� ���� �Լ� ȣ��
	UpdateShaderVariable(CommandList);

	// ��ī�̹ڽ� ������
	for (int i = 0; i < 5; ++i) if (m_SkyBox[i]) m_SkyBox[i]->Render(CommandList);

	// �÷��̾� ������
	if (m_Player) m_Player->Render(CommandList);

	// UI
	if (m_CharInfo) m_CharInfo->Render(CommandList);
	if (m_TrapListUi) m_TrapListUi->Render(CommandList);

	// GameScene�� ������ ������Ʈ ������
	/*if (m_StageWall) m_StageWall->Render(CommandList);
	if (m_StageFloor) m_StageFloor->Render(CommandList);*/

	if (is_rend_01) if (m_Stage_01) m_Stage_01->Render(CommandList);
	if (is_rend_02) if (m_Stage_02) m_Stage_02->Render(CommandList);
	if (is_rend_03) if (m_Stage_03) m_Stage_03->Render(CommandList);
	if (is_rend_04) if (m_Stage_04) m_Stage_04->Render(CommandList);

	// Trap Objects
	for (auto iter = m_Trap.begin(); iter != m_Trap.end(); ++iter)
	{
		if (*iter) {
			(*iter)->UpdateTransform(NULL);
			(*iter)->Render(CommandList);
		}
	}
	// Monster Objects
	for (auto iter = m_Orc.begin(); iter != m_Orc.end(); ++iter)
	{
		(*iter)->UpdateTransform(NULL);
		(*iter)->Render(CommandList);
	}
	for (auto iter = m_Shaman.begin(); iter != m_Shaman.end(); ++iter)
		(*iter)->Render(CommandList);

	for (auto iter = m_WolfRider.begin(); iter != m_WolfRider.end(); ++iter)
		(*iter)->Render(CommandList);

	// Ohter Player
	if (network_manager::GetInst()->IsConnect())
		m_OtherPlayerModel->Render(CommandList);
}

void GameScene::CheckTile()
{
	
}

void GameScene::ProcessInput(HWND hWnd)
{
	float xDelta = 0.f, yDelta = 0.f;
	POINT CursourPos;

	if (GetCapture() == hWnd) {
		SetCursor(NULL);
		GetCursorPos(&CursourPos);
		xDelta = (float)(CursourPos.x - m_ptOldCursorPos.x) / 10.0f;
		yDelta = (float)(CursourPos.y - m_ptOldCursorPos.y) / 10.0f;
		SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		m_ptTrapCursorPos = m_ptOldCursorPos;

		if (m_bClick) {
			
			::ScreenToClient(hWnd, &m_ptTrapCursorPos);

			Camera *pCamera = m_Player->GetCamera();
			
			XMFLOAT3 EndPos(400.0f, 300.0f, 1.0f);
			
			EndPos.x /= pCamera->GetProjectionMatrix()._11;
			EndPos.y /= pCamera->GetProjectionMatrix()._22;
			
			XMFLOAT3 xmf3EndProject;
			XMStoreFloat3(&xmf3EndProject, XMVector3TransformCoord(XMLoadFloat3(&EndPos), XMMatrixInverse(NULL, XMLoadFloat4x4(&pCamera->GetViewMatrix()))));
			XMFLOAT3 StartPos = pCamera->GetPosition();

			if (m_Trap.size() != 0 && m_Trap.back()->GetIsBuildTrap()) {
				XMFLOAT3 NormalEnd;
				xmf3EndProject = Vector3::Subtract(xmf3EndProject, StartPos);
				NormalEnd = Vector3::Normalize(xmf3EndProject);
				NormalEnd = m_Player->GetLook();

				if (is_rend_02) {
					GameObject *TileObject = m_Stage_02->CheckTileBound(StartPos, NormalEnd, true);
					if (TileObject != NULL) {
						BoundingBox BoundTile = TileObject->GetMesh()->GetBounds();
						cout << "�ڽ� Up���� x: " << TileObject->GetUp().x << " y: " << TileObject->GetUp().y  << " z: " << TileObject->GetUp().z << endl;
						BoundTile.Transform(BoundTile, XMLoadFloat4x4(&TileObject->m_WorldPos));
						XMFLOAT3 TilePos = BoundTile.Center;
						
						m_Trap.back()->SetPostion(TilePos);
						m_Trap.back()->UpdateTransform(NULL);
					}
					else {
						m_Trap.back()->SetPostion(Vector3::Add(m_Player->GetPosition(), Vector3::ScalarProduct(m_Player->GetLook(), 100)));
						XMFLOAT3 ypos = m_Trap.back()->GetPosition();
						ypos.y = -50.0f;
						m_Trap.back()->SetPostion(ypos);
						m_Trap.back()->UpdateTransform(NULL);
					}
				}
				if (is_rend_01) {

					GameObject *TileObject = new GameObject;
					switch (m_Trap.back()->GetTrapKind()) {
					case TRAP_NEEDLE:
					case TRAP_SLOW:
						TileObject = m_Stage_01->CheckTileBound(StartPos, NormalEnd, true);
						break;
					case TRAP_FIRE:
					case TRAP_ARROW:
						TileObject = m_Stage_01->CheckTileBound(StartPos, NormalEnd, false);
						break;
					}
					if (TileObject != NULL) {
						BoundingBox BoundTile = TileObject->GetMesh()->GetBounds();
						BoundTile.Transform(BoundTile, XMLoadFloat4x4(&TileObject->m_WorldPos));
						XMFLOAT3 TilePos = BoundTile.Center;
					
						if (TRAP_FIRE || TRAP_ARROW) {
							if (BoundTile.Extents.x < BoundTile.Extents.z) {
								if (StartPos.x < TilePos.x) {
									m_Trap.back()->SetLook(XMFLOAT3(0.0f, 0.0f, 1.0f));
									m_Trap.back()->SetRight(XMFLOAT3(0.0f, 1.0f, 0.0f));
									m_Trap.back()->SetUp(XMFLOAT3(-1.0f, 0.0f, 0.0f));
									TilePos.x -= 15.0f;
								}
								else {
									m_Trap.back()->SetLook(XMFLOAT3(0.0f, 0.0f, 1.0f));
									m_Trap.back()->SetRight(XMFLOAT3(0.0f, -1.0f, 0.0f));
									m_Trap.back()->SetUp(XMFLOAT3(1.0f, 0.0f, 0.0f));
									TilePos.x += 15.0f;
								}
							}
							else {
								if (StartPos.z < TilePos.z) {
									m_Trap.back()->SetLook(XMFLOAT3(0.0f, 1.0f, 0.0f));
									m_Trap.back()->SetRight(XMFLOAT3(1.0f, 0.0f, 0.0f));
									m_Trap.back()->SetUp(XMFLOAT3(0.0f, 0.0f, -1.0f));
									TilePos.z -= 15.0f;
								}
								else {
									m_Trap.back()->SetLook(XMFLOAT3(0.0f, -1.0f, 0.0f));
									m_Trap.back()->SetRight(XMFLOAT3(1.0f, 0.0f, 0.0f));
									m_Trap.back()->SetUp(XMFLOAT3(0.0f, 0.0f, 1.0f));
									TilePos.z += 15.0f;
								}
							}
						}
						else {
							TilePos.y += 10.0f;
						}

						m_Trap.back()->SetPostion(TilePos);
						m_Trap.back()->UpdateTransform(NULL);
					}
					else {
						//cout << "tile �ȸ���" << endl;
						m_Trap.back()->SetPostion(Vector3::Add(m_Player->GetPosition(), Vector3::ScalarProduct(m_Player->GetLook(), 100)));
						XMFLOAT3 ypos = m_Trap.back()->GetPosition();
						ypos.y = -50.0f;
						m_Trap.back()->SetPostion(ypos);
						m_Trap.back()->UpdateTransform(NULL);
					}
				}
				if (is_rend_03) {
					//cout << "3����������" << endl;
					GameObject *TileObject = m_Stage_03->CheckTileBound(StartPos, NormalEnd, true);
					if (TileObject != NULL) {
						BoundingBox BoundTile = TileObject->GetMesh()->GetBounds();
						BoundTile.Transform(BoundTile, XMLoadFloat4x4(&TileObject->m_WorldPos));
						XMFLOAT3 TilePos = BoundTile.Center;
						m_Trap.back()->SetPostion(TilePos);
						m_Trap.back()->UpdateTransform(NULL);
					}
					else {
						m_Trap.back()->SetPostion(Vector3::Add(m_Player->GetPosition(), Vector3::ScalarProduct(m_Player->GetLook(), 100)));
						XMFLOAT3 ypos = m_Trap.back()->GetPosition();
						ypos.y = -50.0f;
						m_Trap.back()->SetPostion(ypos);
						m_Trap.back()->UpdateTransform(NULL);
					}
				}

			}
			
		}
	}

	if (xDelta != 0.f || yDelta != 0.f) {
		if (xDelta || yDelta)
			m_Player->SetRotate(0.f, xDelta, 0.f);
	}

}

bool GameScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);

		// �÷��̾ ���� ��ư�� Ŭ������ ��, ���� ��ġ ���̾��ٸ� �� �̻� ������� �ʵ��� ��
		if (m_bClick && m_Trap.size() != 0) {
			if (m_Trap.back()->GetIsBuildTrap() == true) {
				m_Trap.back()->BuildTrap(false);
			}
		}
		else {
			m_ptTrapCursorPos = m_ptOldCursorPos;
			::ScreenToClient(hWnd, &m_ptTrapCursorPos);

			Camera *pCamera = m_Player->GetCamera();

			XMFLOAT3 EndPos((m_ptTrapCursorPos.x * 2.0f) / FRAME_BUFFER_WIDTH - 1.0f, ((-m_ptTrapCursorPos.y * 2.0f) / FRAME_BUFFER_HEIGHT + 1.0f), 1.0f);

			EndPos.x /= pCamera->GetProjectionMatrix()._11;
			EndPos.y /= pCamera->GetProjectionMatrix()._22;
			XMFLOAT3 xmf3EndProject;
			XMStoreFloat3(&xmf3EndProject, XMVector3TransformCoord(XMLoadFloat3(&EndPos), XMMatrixInverse(NULL, XMLoadFloat4x4(&pCamera->GetViewMatrix()))));
			XMFLOAT3 StartPos = pCamera->GetPosition();

			m_Player->SetPlayerAnimateType(ANIMATION_TYPE_SHOOT);
			m_Player->SetEnable(9);
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
			m_Player->SetPlayerAnimateType(ANIMATION_TYPE_RELOAD);
			m_Player->SetEnable(18);
			break;
		// ���� ��ġ �غ�
		case '1':
		{
			if (m_bClick && m_Trap.back()->GetIsBuildTrap() == false) {
				break;
			}
			if (m_bClick && m_Trap.back()->GetIsBuildTrap() == true)
				m_Trap.pop_back();
			else
				m_bClick = true;
			m_Trap.emplace_back(new Trap());
			m_Trap.back()->SetChild(m_NeedleTrapModel, false);
			m_Trap.back()->BuildTrap(true);
			m_Trap.back()->ActiveTrap(true);
			m_Trap.back()->SetEnable(1);
			m_Trap.back()->SetTrapKind(TRAP_NEEDLE);
		}
		break;

		case '2':
		{
			if (m_bClick && m_Trap.back()->GetIsBuildTrap() == false) {
				break;
			}
			if (m_bClick && m_Trap.back()->GetIsBuildTrap() == true)
				m_Trap.pop_back();
			else
				m_bClick = true;

			m_Trap.emplace_back(new Trap());
			m_Trap.back()->SetChild(m_FireTrapModel, false);
			m_Trap.back()->BuildTrap(true);
			m_Trap.back()->SetTrapKind(TRAP_FIRE);
		}
		break;

		case '3':
		{
			if (m_bClick && m_Trap.back()->GetIsBuildTrap() == false) {
				break;
			}
			if (m_bClick && m_Trap.back()->GetIsBuildTrap() == true)
				m_Trap.pop_back();
			else
				m_bClick = true;
			m_Trap.emplace_back(new Trap());
			m_Trap.back()->SetChild(m_SlowTrapModel, false);
			m_Trap.back()->BuildTrap(true);
			m_Trap.back()->SetTrapKind(TRAP_SLOW);
		}
		break;

		case '4':
		{
			if (m_bClick && m_Trap.back()->GetIsBuildTrap() == false) {
				break;
			}
			if (m_bClick && m_Trap.back()->GetIsBuildTrap() == true)
				m_Trap.pop_back();
			else
				m_bClick = true;
			m_Trap.emplace_back(new Trap());
			m_Trap.back()->SetChild(m_ArrowTrapModel, false);
			m_Trap.back()->BuildTrap(true);
			m_Trap.back()->SetTrapKind(TRAP_ARROW);
		}
		break;

		case '7':
		{
			if (is_rend_01) is_rend_01 = false;
			else is_rend_01 = true;
		}
		break;

		case '8':
		{
			if (is_rend_02) is_rend_02 = false;
			else is_rend_02 = true;
		}
		break;

		case '9':
		{
			if (is_rend_03) is_rend_03 = false;
			else is_rend_03 = true;
		}
		break;

		case '0':
		{
			if (is_rend_04) is_rend_04 = false;
			else is_rend_04 = true;
		}
		break;

		case VK_SHIFT:
			m_Player->SetAnimateType(27, ANIMATION_TYPE_ONCE);
			m_Player->SetEnable(27);
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
			if (m_bClick && m_Trap.back()->GetIsBuildTrap() == true) {
				m_Trap.pop_back();
			}
			m_bClick = false;
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