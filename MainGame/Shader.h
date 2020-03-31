#pragma once

#include "GameObject.h"

class Shader
{
public:
	Shader();
	~Shader();

protected:
	int										m_nPipelineState = 0;

	ID3DBlob								*m_VertexShaderBlob = NULL;
	ID3DBlob								*m_PixelShaderBlob = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC		m_PipelineStateDesc;

	ID3D12PipelineState						**m_PipelineStates = NULL;

	ID3D12DescriptorHeap					*m_CbvSrvDescriptorHeap = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE				m_CbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE				m_CbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE				m_SrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE				m_SrvGPUDescriptorStartHandle;

private:
	int										m_nReference = 0;

public:
	void AddRef() { ++m_nReference; }
	void Release() { if (--m_nReference <= 0) delete this; }

	// 렌더링 파이프라인
	virtual D3D12_SHADER_BYTECODE		CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE		CreatePixelShader();

	// 고정 파이프라인
	virtual D3D12_INPUT_LAYOUT_DESC		CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC		CreateRasterizerState();
	virtual D3D12_BLEND_DESC			CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC	CreateDepthStencilState();
	
	D3D12_SHADER_BYTECODE				CompileShaderFromFile(WCHAR *FileName, LPCSTR ShaderName, LPCSTR ShaderProfile, ID3DBlob **ShaderBlob);

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *CommandList, int nPipelineState);
	virtual void CreateShader(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature);

	void CreateCbvSrvDescriptorHeap(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, int nConstantBufferView, int nShaderResourceView);
	void CreateShaderResourceView(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, Texture *Texture, UINT nRootParameterStartIndex, bool AutoIncrement);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return m_CbvGPUDescriptorStartHandle; }

};

class UIShader : public Shader
{
public:
	UIShader() { }
	~UIShader() { }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void CreateShader(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature);
};

class TrapShader : public Shader
{
public:
	TrapShader() { }
	~TrapShader() { }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void CreateShader(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature);
};

class StandardShader : public Shader
{
public:
	StandardShader() { }
	~StandardShader() { }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *CommandList, int nPipelineState);
	virtual void CreateShader(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature);
};