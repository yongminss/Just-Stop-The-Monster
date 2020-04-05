#include "stdafx.h"
#include "Scene.h"

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

	m_Viewport = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.f, 1.f };
	m_ScissorRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };

	//m_UI = new UI(Device, CommandList, m_GraphicsRootSignature);
}

void TitleScene::ReleaseObject()
{
	if (m_GraphicsRootSignature) m_GraphicsRootSignature->Release();

	if (m_UI) delete m_UI;
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
	RootParameter[0].Descriptor.ShaderRegister = 1;
	RootParameter[0].Descriptor.RegisterSpace = 0;
	RootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

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

void TitleScene::Render(ID3D12GraphicsCommandList *CommandList)
{
	// 루트 시그너쳐를 설정
	CommandList->SetGraphicsRootSignature(m_GraphicsRootSignature);

	// 뷰포트와 씨저렉트 영역을 설정
	CommandList->RSSetViewports(1, &m_Viewport);
	CommandList->RSSetScissorRects(1, &m_ScissorRect);

	// 씬에 등장할 오브젝트들을 렌더링
	//if (m_UI) m_UI->Render(CommandList);
}


// 싱글 및 멀티와 캐릭터, 함정 등을 선택 후, 게임을 진행하는 게임 씬
GameScene::GameScene()
{

}

GameScene::~GameScene()
{

}

void GameScene::BuildObject(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList)
{
	m_GraphicsRootSignature = CreateGraphicsRootSignature(Device);

	CreateCbvSrvDescriptorHeap(Device, CommandList, 0, 10);

	Material::PrepareShader(Device, CommandList, m_GraphicsRootSignature);

	BuildDefaultLightsAndMaterials();

	m_Player = new Player(Device, CommandList, m_GraphicsRootSignature);
	
	// 함정 오브젝트가 사용할 쉐이더를 생성
	//StandardShader *TrapShader = new StandardShader();
	//TrapShader->CreateShader(Device, CommandList, m_GraphicsRootSignature);

	m_TrapModel = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, m_GraphicsRootSignature, "Model/Trap_Needle.bin", NULL, false);
	m_Trap.emplace_back(new Trap());
	m_Trap.back()->SetChild(m_TrapModel, false);
	m_Trap.back()->SetPostion(XMFLOAT3(0.f, 0.f, 50.f));

	for (int i = 0; i < 5; ++i) {
		m_TrapCover.emplace_back(new TrapCover(Device, CommandList, m_GraphicsRootSignature, 0));
		m_TrapCover.back()->SetPostion(XMFLOAT3(-30.f + (i * 50), 0.f, 100.f + (i * 50)));
	}
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
	m_Lights[1].m_Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f);
	m_Lights[1].m_Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.f);
	m_Lights[1].m_Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.f);
	m_Lights[1].m_Position = XMFLOAT3(-50.f, 20.f, -5.f);
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
	m_Lights[3].m_Theta = (float)cos(XMConvertToRadians(30.f));
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
	RootParameter[1].Constants.Num32BitValues = 33;
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
	m_Player->Update(ElapsedTime);
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

	D3D12_GPU_VIRTUAL_ADDRESS cbLightGpuVirtualAddress = m_cbLight->GetGPUVirtualAddress();
	CommandList->SetGraphicsRootConstantBufferView(12, cbLightGpuVirtualAddress);

	// 게임 씬에 등장할 오브젝트들을 그림

	for (auto iter = m_TrapCover.begin(); iter != m_TrapCover.end(); ++iter)
		(*iter)->Render(CommandList);

	//for (auto iter = m_Trap.begin(); iter != m_Trap.end(); ++iter)
	//	(*iter)->Render(CommandList);
}