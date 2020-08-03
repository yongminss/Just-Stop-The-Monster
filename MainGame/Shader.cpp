#include "stdafx.h"
#include "Shader.h"

Shader::Shader()
{

}

Shader::~Shader()
{
	if (m_CbvSrvDescriptorHeap) m_CbvSrvDescriptorHeap->Release();
	if (m_PipelineStates) {
		for (int i = 0; i < m_nPipelineState; ++i)
			if (m_PipelineStates[i]) m_PipelineStates[i]->Release();
		delete m_PipelineStates;
	}
}

D3D12_SHADER_BYTECODE Shader::CreateVertexShader()
{
	D3D12_SHADER_BYTECODE ShaderByteCode;
	ShaderByteCode.BytecodeLength = 0;
	ShaderByteCode.pShaderBytecode = NULL;

	return ShaderByteCode;
}

D3D12_SHADER_BYTECODE Shader::CreatePixelShader()
{
	D3D12_SHADER_BYTECODE ShaderByteCode;
	ShaderByteCode.BytecodeLength = 0;
	ShaderByteCode.pShaderBytecode = NULL;

	return ShaderByteCode;
}

D3D12_INPUT_LAYOUT_DESC Shader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc;
	InputLayoutDesc.pInputElementDescs = NULL;
	InputLayoutDesc.NumElements = 0;

	return InputLayoutDesc;

}

D3D12_RASTERIZER_DESC Shader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC RasterizerDesc;
	
	::ZeroMemory(&RasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	RasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	RasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	RasterizerDesc.FrontCounterClockwise = FALSE;
	RasterizerDesc.DepthBias = 0;
	RasterizerDesc.DepthBiasClamp = 0.0f;
	RasterizerDesc.SlopeScaledDepthBias = 0.0f;
	RasterizerDesc.DepthClipEnable = TRUE;
	RasterizerDesc.MultisampleEnable = FALSE;
	RasterizerDesc.AntialiasedLineEnable = FALSE;
	RasterizerDesc.ForcedSampleCount = 0;
	RasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;


	return RasterizerDesc;
}

D3D12_BLEND_DESC Shader::CreateBlendState()
{
	D3D12_BLEND_DESC BlendDesc;

	::ZeroMemory(&BlendDesc, sizeof(D3D12_BLEND_DESC));
	//BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.AlphaToCoverageEnable = TRUE;
	BlendDesc.IndependentBlendEnable = FALSE;
	//BlendDesc.RenderTarget[0].BlendEnable = FALSE;
	BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	BlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	//BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	//BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return BlendDesc;
}

D3D12_DEPTH_STENCIL_DESC Shader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC DepthStencilDesc;

	::ZeroMemory(&DepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	DepthStencilDesc.DepthEnable = TRUE;
	DepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	DepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	DepthStencilDesc.StencilEnable = FALSE;
	DepthStencilDesc.StencilReadMask = 0x00;
	DepthStencilDesc.StencilWriteMask = 0x00;
	DepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	DepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	DepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	DepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return DepthStencilDesc;
}

D3D12_SHADER_BYTECODE Shader::CompileShaderFromFile(WCHAR *FileName, LPCSTR ShaderName, LPCSTR ShaderProfile, ID3DBlob **ShaderBlob)
{
	UINT nCompileFlag = 0;
#if defined(_DEBUG)
	nCompileFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ID3DBlob *ErrorBlob = NULL;
	HRESULT hResult = ::D3DCompileFromFile(FileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, ShaderName, ShaderProfile, nCompileFlag, 0, ShaderBlob, &ErrorBlob);
	char *ErrorString = NULL;
	if (ErrorBlob) ErrorString = (char*)ErrorBlob->GetBufferPointer();

	D3D12_SHADER_BYTECODE ShaderByteCode;
	ShaderByteCode.BytecodeLength = (*ShaderBlob)->GetBufferSize();
	ShaderByteCode.pShaderBytecode = (*ShaderBlob)->GetBufferPointer();

	return ShaderByteCode;
}

void Shader::OnPrepareRender(ID3D12GraphicsCommandList *CommandList, int nPipelineState)
{
	if (m_PipelineStates)
		CommandList->SetPipelineState(m_PipelineStates[nPipelineState]);
}

void Shader::CreateShader(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature)
{
	HRESULT hResult;

	::ZeroMemory(&m_PipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	m_PipelineStateDesc.pRootSignature = GraphicsRootSignature;
	m_PipelineStateDesc.VS = CreateVertexShader();
	m_PipelineStateDesc.PS = CreatePixelShader();
	m_PipelineStateDesc.InputLayout = CreateInputLayout();
	m_PipelineStateDesc.RasterizerState = CreateRasterizerState();
	m_PipelineStateDesc.BlendState = CreateBlendState();
	m_PipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	m_PipelineStateDesc.SampleMask = UINT_MAX;
	m_PipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_PipelineStateDesc.NumRenderTargets = 1;
	m_PipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_PipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_PipelineStateDesc.SampleDesc.Count = 1;
	m_PipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	hResult = Device->CreateGraphicsPipelineState(&m_PipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_PipelineStates[0]);
}


// UI에서 사용할 쉐이더 //
D3D12_INPUT_LAYOUT_DESC UIShader::CreateInputLayout()
{
	UINT nInputElementDesc = 2;
	D3D12_INPUT_ELEMENT_DESC *InputElementDesc = new D3D12_INPUT_ELEMENT_DESC[nInputElementDesc];

	InputElementDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	InputElementDesc[1] = { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc;
	InputLayoutDesc.pInputElementDescs = InputElementDesc;
	InputLayoutDesc.NumElements = nInputElementDesc;

	return InputLayoutDesc;
}

D3D12_SHADER_BYTECODE UIShader::CreateVertexShader()
{
	return Shader::CompileShaderFromFile(L"Shaders.hlsl", "VSUserInterface", "vs_5_1", &m_VertexShaderBlob);
}

D3D12_SHADER_BYTECODE UIShader::CreatePixelShader()
{
	return Shader::CompileShaderFromFile(L"Shaders.hlsl", "PSPictureColor", "ps_5_1", &m_PixelShaderBlob);
}

void UIShader::CreateShader(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature)
{
	m_nPipelineState = 1;
	m_PipelineStates = new ID3D12PipelineState*[m_nPipelineState];
	
	Shader::CreateShader(Device, CommandList, GraphicsRootSignature);
}


// 스카이박스에 사용할 쉐이더 //
D3D12_INPUT_LAYOUT_DESC SkyBoxShader::CreateInputLayout()
{
	UINT nInputElementDesc = 2;
	D3D12_INPUT_ELEMENT_DESC *InputElementDesc = new D3D12_INPUT_ELEMENT_DESC[nInputElementDesc];

	InputElementDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	InputElementDesc[1] = { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc;
	InputLayoutDesc.pInputElementDescs = InputElementDesc;
	InputLayoutDesc.NumElements = nInputElementDesc;

	return InputLayoutDesc;
}

D3D12_SHADER_BYTECODE SkyBoxShader::CreateVertexShader()
{
	return Shader::CompileShaderFromFile(L"Shaders.hlsl", "VSTexture", "vs_5_1", &m_VertexShaderBlob);
}

D3D12_SHADER_BYTECODE SkyBoxShader::CreatePixelShader()
{
	return Shader::CompileShaderFromFile(L"Shaders.hlsl", "PSPictureColor", "ps_5_1", &m_PixelShaderBlob);
}

D3D12_DEPTH_STENCIL_DESC SkyBoxShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC DepthStencilDesc;
	::ZeroMemory(&DepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	DepthStencilDesc.DepthEnable = FALSE;
	DepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	DepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	DepthStencilDesc.StencilEnable = FALSE;
	DepthStencilDesc.StencilReadMask = 0x00;
	DepthStencilDesc.StencilWriteMask = 0x00;
	DepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	DepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	DepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	DepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return DepthStencilDesc;
}

void SkyBoxShader::CreateShader(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature)
{
	m_nPipelineState = 1;
	m_PipelineStates = new ID3D12PipelineState*[m_nPipelineState];

	Shader::CreateShader(Device, CommandList, GraphicsRootSignature);
}


D3D12_INPUT_LAYOUT_DESC EffectShader::CreateInputLayout()
{
	UINT nInputElementDesc = 2;
	D3D12_INPUT_ELEMENT_DESC *InputElementDesc = new D3D12_INPUT_ELEMENT_DESC[nInputElementDesc];

	InputElementDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	InputElementDesc[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc;
	InputLayoutDesc.pInputElementDescs = InputElementDesc;
	InputLayoutDesc.NumElements = nInputElementDesc;

	return InputLayoutDesc;
}

D3D12_SHADER_BYTECODE EffectShader::CreateVertexShader()
{
	return Shader::CompileShaderFromFile(L"Shaders.hlsl", "VSEffect", "vs_5_1", &m_VertexShaderBlob);
}

D3D12_SHADER_BYTECODE EffectShader::CreatePixelShader()
{
	return Shader::CompileShaderFromFile(L"Shaders.hlsl", "PSEffect", "ps_5_1", &m_PixelShaderBlob);
}

D3D12_BLEND_DESC EffectShader::CreateBlendState()
{
	D3D12_BLEND_DESC BlendDesc;

	::ZeroMemory(&BlendDesc, sizeof(D3D12_BLEND_DESC));
	//BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.AlphaToCoverageEnable = TRUE;
	BlendDesc.IndependentBlendEnable = FALSE;
	//BlendDesc.RenderTarget[0].BlendEnable = FALSE;
	BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	BlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	//BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	//BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return BlendDesc;
}

void EffectShader::CreateShader(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature)
{
	m_nPipelineState = 1;
	m_PipelineStates = new ID3D12PipelineState*[m_nPipelineState];

	Shader::CreateShader(Device, CommandList, GraphicsRootSignature);
}

void EffectShader::BuildObject(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature)
{
	m_Effect = new Effect();

	CubeMesh *ObjMesh = NULL;
	ObjMesh = new CubeMesh(Device, CommandList);
	m_Effect->SetMesh(ObjMesh);

	m_Effect->SetPostion(XMFLOAT3(0.f, 0.f, 0.f));
}

void EffectShader::Render(ID3D12GraphicsCommandList *CommandList)
{
	Shader::OnPrepareRender(CommandList, 0);

	if (m_Effect) m_Effect->Render(CommandList);
}

// Standard & Skinned Animation Shader
D3D12_INPUT_LAYOUT_DESC StandardShader::CreateInputLayout()
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE StandardShader::CreateVertexShader()
{
	return(Shader::CompileShaderFromFile(L"Shaders.hlsl", "VSStandard", "vs_5_1", &m_VertexShaderBlob));
}

D3D12_SHADER_BYTECODE StandardShader::CreatePixelShader()
{
	return(Shader::CompileShaderFromFile(L"Shaders.hlsl", "PSStandard", "ps_5_1", &m_PixelShaderBlob));
}

void StandardShader::OnPrepareRender(ID3D12GraphicsCommandList *CommandList, int nPipelineState)
{
	if (m_PipelineStates)
		CommandList->SetPipelineState(m_PipelineStates[nPipelineState]);
}

void StandardShader::CreateShader(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature)
{
	m_nPipelineState = 1;
	m_PipelineStates = new ID3D12PipelineState*[m_nPipelineState];

	Shader::CreateShader(Device, CommandList, GraphicsRootSignature);
}


D3D12_INPUT_LAYOUT_DESC SkinnedAnimationShader::CreateInputLayout()
{
	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return d3dInputLayoutDesc;
}

D3D12_SHADER_BYTECODE SkinnedAnimationShader::CreateVertexShader()
{
	return(Shader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkinnedAnimationStandard", "vs_5_1", &m_VertexShaderBlob));
}

// Instancing Object Shader (Trap, Monster)
D3D12_INPUT_LAYOUT_DESC TrapInstancingShader::CreateInputLayout()
{
	UINT nInputElementDescs = 9;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	pd3dInputElementDescs[5] = { "WORLDMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[6] = { "WORLDMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[7] = { "WORLDMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[8] = { "WORLDMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE TrapInstancingShader::CreateVertexShader()
{
	return(Shader::CompileShaderFromFile(L"Shaders.hlsl", "VSInstancingStandard", "vs_5_1", &m_VertexShaderBlob));
}

D3D12_SHADER_BYTECODE TrapInstancingShader::CreatePixelShader()
{
	return(Shader::CompileShaderFromFile(L"Shaders.hlsl", "PSStandard", "ps_5_1", &m_PixelShaderBlob));
}

void TrapInstancingShader::OnPrepareRender(ID3D12GraphicsCommandList *CommandList, int nPipelineState)
{
	if (m_PipelineStates)
		CommandList->SetPipelineState(m_PipelineStates[nPipelineState]);
}

void TrapInstancingShader::CreateShader(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature)
{
	m_nPipelineState = 1;
	m_PipelineStates = new ID3D12PipelineState*[m_nPipelineState];

	Shader::CreateShader(Device, CommandList, GraphicsRootSignature);
}

void TrapInstancingShader::BuildObject(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature, UINT Type)
{
	m_Trap.reserve(INSTANCE_NUM);

	switch (Type) {
	case 0:
		Model = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, GraphicsRootSignature, "Model/Trap_Needle.bin", NULL, true);
		break;

	case 1:
		Model = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, GraphicsRootSignature, "Model/Trap_Fire.bin", NULL, false);
		break;

	case 2:
		Model = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, GraphicsRootSignature, "Model/Trap_Slow.bin", NULL, false);
		break;

	case 3:
		Model = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, GraphicsRootSignature, "Model/Trap_Arrow.bin", NULL, false);
		break;

	default:
		cout << "Trap`s Type Error!" << endl;
		break;
	}

	CreateShaderVariable(Device, CommandList);
}

void TrapInstancingShader::CreateShaderVariable(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList)
{
	m_cbGameObject = ::CreateBufferResource(Device, CommandList, NULL, sizeof(VS_VB_INSTANCE) * INSTANCE_NUM, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cbGameObject->Map(0, NULL, (void**)&m_MappedGameObject);
	m_InstanceBufferView.BufferLocation = m_cbGameObject->GetGPUVirtualAddress();
	m_InstanceBufferView.StrideInBytes = sizeof(VS_VB_INSTANCE);
	m_InstanceBufferView.SizeInBytes = sizeof(VS_VB_INSTANCE) * INSTANCE_NUM;
}

void TrapInstancingShader::UpdateShaderVariable()
{
	for (int i = 0; i < m_Trap.size(); ++i) {
		XMStoreFloat4x4(&m_MappedGameObject[i].m_Transform, XMMatrixTranspose(XMLoadFloat4x4(&m_Trap[i]->m_WorldPos)));
	}
}

void TrapInstancingShader::Animate(float ElapsedTime, XMFLOAT3 Position)
{
	for (int i = 0; i < m_Trap.size(); ++i) {
		m_Trap[i]->UpdateTransform(NULL);
		m_Trap[i]->Animate(Position, ElapsedTime, NULL);
	}
}

void TrapInstancingShader::Render(ID3D12GraphicsCommandList *CommandList)
{
	if (m_Trap.size() != 0) {
		Shader::OnPrepareRender(CommandList, 0);

		UpdateShaderVariable();

		m_Trap[0]->Render(CommandList, INSTANCE_NUM, m_InstanceBufferView);
	}
}

void TrapInstancingShader::BuildTrap()
{
	Trap *Obj = NULL;

	if (m_Trap.size() < INSTANCE_NUM) {
		Obj = new Trap();
		Obj->SetChild(Model, false);
		Obj->SetScale(100.f, 100.f, 100.f);
		Obj->BuildTrap(true);
		Obj->ActiveTrap(true);
		Obj->SetEnable(1);
		Obj->SetTrapKind(TRAP_NEEDLE);
		m_Trap.emplace_back(Obj);
	}
}

D3D12_INPUT_LAYOUT_DESC MonsterInstancingShader::CreateInputLayout()
{
	UINT nInputElementDescs = 11;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	pd3dInputElementDescs[7] = { "WORLDMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 7, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[8] = { "WORLDMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 7, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[9] = { "WORLDMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 7, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[10] = { "WORLDMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 7, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return d3dInputLayoutDesc;
}

D3D12_SHADER_BYTECODE MonsterInstancingShader::CreateVertexShader()
{
	return(Shader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkinnedAnimation_InstancingStandard", "vs_5_1", &m_VertexShaderBlob));
}

void MonsterInstancingShader::BuildObject(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature, UINT Type)
{
	m_Monster.reserve(INSTANCE_NUM);

	switch (Type) {
	case 0:
		Model = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, GraphicsRootSignature, "Model/Monster_Weak_Infantry.bin", NULL, true);
		break;

	case 1:
		Model = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, GraphicsRootSignature, "Model/Monster_Shaman.bin", NULL, true);
		break;

	case 2:
		Model = GameObject::LoadGeometryAndAnimationFromFile(Device, CommandList, GraphicsRootSignature, "Model/Monster_WolfRider.bin", NULL, true);
		break;

	default:
		cout << "Monster`s Type Error!" << endl;
		break;
	}
	Monster *Orc = NULL;

	for (int i = 0; i < INSTANCE_NUM; ++i) {
		Orc = new Monster();
		Orc->SetChild(Model, true);
		Orc->SetScale(50.f, 50.f, 50.f);
		Orc->SetRotate(-90.f, 0.f, 0.f);
		Orc->SetPostion(XMFLOAT3(0.f + (i * 100), -50.f, -250.f - (Type * 50)));
		m_Monster.emplace_back(Orc);
	}
	CreateShaderVariable(Device, CommandList);
}

void MonsterInstancingShader::UpdateShaderVariable()
{
	for (int i = 0; i < m_Monster.size(); ++i) {
		XMStoreFloat4x4(&m_MappedGameObject[i].m_Transform, XMMatrixTranspose(XMLoadFloat4x4(&m_Monster[i]->m_WorldPos)));
	}
}

void MonsterInstancingShader::Animate(float ElapsedTime)
{
	for (int i = 0; i < m_Monster.size(); ++i) {
		m_Monster[i]->UpdateTransform(NULL);
		m_Monster[i]->Animate(ElapsedTime, NULL);
	}
}

void MonsterInstancingShader::Render(ID3D12GraphicsCommandList *CommandList)
{
	Shader::OnPrepareRender(CommandList, 0);

	UpdateShaderVariable();

	/*XMFLOAT3 position = XMFLOAT3(network_manager::GetInst()->m_monster_pool->world_pos._41, network_manager::GetInst()->m_monster_pool->world_pos._42, network_manager::GetInst()->m_monster_pool->world_pos._43);
	m_Monster[0]->SetPostion(position);*/

	m_Monster[0]->Render(CommandList, INSTANCE_NUM, m_InstanceBufferView);
	//cout << m_Monster[0]->GetPosition().x << " " << m_Monster[0]->GetPosition().y << " " << m_Monster[0]->GetPosition().z << endl;
}