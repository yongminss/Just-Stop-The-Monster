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


// 함정에 사용할 쉐이더 //
D3D12_INPUT_LAYOUT_DESC TrapShader::CreateInputLayout()
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

D3D12_SHADER_BYTECODE TrapShader::CreateVertexShader()
{
	return Shader::CompileShaderFromFile(L"Shaders.hlsl", "VSTrap", "vs_5_1", &m_VertexShaderBlob);
}

D3D12_SHADER_BYTECODE TrapShader::CreatePixelShader()
{
	return Shader::CompileShaderFromFile(L"Shaders.hlsl", "PSPictureColor", "ps_5_1", &m_PixelShaderBlob);
}

void TrapShader::CreateShader(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature)
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
	return Shader::CompileShaderFromFile(L"Shaders.hlsl", "VSTrap", "vs_5_1", &m_VertexShaderBlob);
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


// 3D 오브젝트에 사용할 쉐이더
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

// 애니메이션을 하는 3D 오브젝트에서 사용할 쉐이더
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