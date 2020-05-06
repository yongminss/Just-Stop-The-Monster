#pragma once

class GameObject;

#define VERTEXT_POSITION				0x0001
#define VERTEXT_COLOR					0x0002
#define VERTEXT_NORMAL					0x0004
#define VERTEXT_TANGENT					0x0008
#define VERTEXT_TEXTURE_COORD0			0x0010
#define VERTEXT_TEXTURE_COORD1			0x0020

#define VERTEXT_BONE_INDEX_WEIGHT		0x1000

#define VERTEXT_TEXTURE					(VERTEXT_POSITION | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_DETAIL					(VERTEXT_POSITION | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)
#define VERTEXT_NORMAL_TEXTURE			(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_NORMAL_TANGENT_TEXTURE	(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TANGENT | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_NORMAL_DETAIL			(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)
#define VERTEXT_NORMAL_TANGENT__DETAIL	(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TANGENT | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)


class Vertex
{
public:
	Vertex() { m_Position = XMFLOAT3(0.f, 0.f, 0.f); }
	Vertex(XMFLOAT3 Position) { m_Position = Position; }
	~Vertex() { }

protected:
	XMFLOAT3			m_Position;

};

class TextureVertex : public Vertex
{
public:
	TextureVertex()
	{
		m_Position = XMFLOAT3(0.f, 0.f, 0.f);
		m_TexCoord = XMFLOAT2(0.f, 0.f);
	}
	TextureVertex(XMFLOAT3 Position, XMFLOAT2 TexCoord)
	{
		m_Position = Position;
		m_TexCoord = TexCoord;
	}
	~TextureVertex() { }

private:
	XMFLOAT2			m_TexCoord;

};

class Mesh
{
public:
	Mesh();
	Mesh(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);
	~Mesh();

protected:
	char			m_strMeshName[64] = { 0 };

	XMFLOAT3		m_AABBCenter = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3		m_AABBExtent = XMFLOAT3(0.f, 0.f, 0.f);

	D3D12_PRIMITIVE_TOPOLOGY		m_PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT							m_nSlot = 0;
	UINT							m_nVertices = 0;
	UINT							m_nIndices = 0;
	UINT							m_nStride = 0;
	UINT							m_nOffset = 0;
	UINT							m_nType = 0;

	XMFLOAT3						*m_Position = NULL;

	int								m_nSubMesh = 0;
	int								*m_pnSubSetIndices = NULL;
	UINT							**m_ppnSubSetIndices = NULL;

	ID3D12Resource					*m_VertexBuffer = NULL;
	ID3D12Resource					*m_VertexUploadBuffer = NULL;

	ID3D12Resource					*m_IndexBuffer = NULL;

	D3D12_VERTEX_BUFFER_VIEW		m_VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW			m_IndexBufferView;

	ID3D12Resource					*m_PositionBuffer = NULL;
	ID3D12Resource					*m_PositionUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_PositionBufferView;

	ID3D12Resource					**m_SubSetIndexBuffer = NULL;
	ID3D12Resource					**m_SubSetIndexUploadBuffer = NULL;
	D3D12_INDEX_BUFFER_VIEW			*m_SubSetIndexBufferView = NULL;

private:
	int								m_nReferences = 0;

public:
	void AddRef() { ++m_nReferences; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	UINT GetType() { return m_nType; }
	
	virtual void CreateShaderVariable(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList) { }
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList) { }

	virtual void OnPreRender(ID3D12GraphicsCommandList *CommandList, void *Context) { }
	
	void Render(ID3D12GraphicsCommandList *CommandList);
	void Render(ID3D12GraphicsCommandList *CommandList, int nSubSet);
};

// 텍스쳐 맵핑을 진행할 메쉬
class TextureMesh : public Mesh
{
public:
	TextureMesh(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, float width, float height, float depth, float x, float y, float z, int type, int ImageType = 0);
	~TextureMesh() { }

};

// bin 파일에서 불러오는 오브젝트에 사용할 메쉬
class StandardMesh : public Mesh
{
public:
	StandardMesh(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);
	virtual ~StandardMesh();

protected:
	XMFLOAT4					*m_Color = NULL;
	XMFLOAT2					*m_TextureCoord0 = NULL;
	XMFLOAT2					*m_TextureCoord1 = NULL;
	XMFLOAT3					*m_Normal = NULL;
	XMFLOAT3					*m_Tangent = NULL;
	XMFLOAT3					*m_BiTangent = NULL;

	ID3D12Resource				*m_TextureCoord0Buffer = NULL;
	ID3D12Resource				*m_TextureCoord0UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW	m_TextureCoord0BufferView;

	ID3D12Resource				*m_TextureCoord1Buffer = NULL;
	ID3D12Resource				*m_TextureCoord1UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW	m_TextureCoord1BufferView;

	ID3D12Resource				*m_NormalBuffer = NULL;
	ID3D12Resource				*m_NormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW	m_NormalBufferView;

	ID3D12Resource				*m_TangentBuffer = NULL;
	ID3D12Resource				*m_TangentUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW	m_TangentBufferView;

	ID3D12Resource				*m_BiTangentBuffer = NULL;
	ID3D12Resource				*m_BiTangentUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW	m_BiTangentBufferView;

public:
	void LoadMeshFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, FILE *InFile);

	virtual void OnPreRender(ID3D12GraphicsCommandList *CommandList, void *Context);
};

#define SKINNED_ANIMATION_BONES 128

class SkinnedMesh : public StandardMesh
{
public:
	SkinnedMesh(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);
	virtual ~SkinnedMesh();

private:
	char						m_SkinnedMeshName[64] = { 0 };

	int							m_nBonePerVertex = 4;

	int							 m_nSkinningBone = 0;

	XMFLOAT4X4					*m_BindPoseBoneOffset = NULL;

	XMUINT4						*m_BoneIndice = NULL;
	XMFLOAT4					*m_BoneWeight = NULL;

	ID3D12Resource				*m_BoneIndexBuffer = NULL;
	ID3D12Resource				*m_BoneIndexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW	m_BoneIndexBufferView;

	ID3D12Resource				*m_BoneWeightBuffer = NULL;
	ID3D12Resource				*m_BoneWeightUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW	m_BoneWeightBufferView;

	ID3D12Resource				*m_cbBoneOffset = NULL;
	XMFLOAT4X4					*m_BoneOffsetPos = NULL;

	ID3D12Resource				*m_cbBoneTransform = NULL;
	XMFLOAT4X4					*m_BoneTransformPos = NULL;

public:
	char						(*m_SkinningBoneName)[64];
	GameObject					**m_SkinningBoneFrameCache = NULL;

public:
	int GetSkinningBoneNum() { return m_nSkinningBone; }

	virtual void CreateShaderVariable(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList);

	virtual void OnPreRender(ID3D12GraphicsCommandList *CommandList, void *Context);

	void LoadSkinInfoFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, FILE *InFile);
};
