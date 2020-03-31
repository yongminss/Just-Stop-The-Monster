#include "stdafx.h"
#include "GameObject.h"
#include "Shader.h"

Texture::Texture(int nTextureResource, UINT ResourceType, int nSampler)
{
	m_nTextureType = ResourceType;
	m_nTexture = nTextureResource;

	if (m_nTexture > 0) {
		m_RootArgumentInfo = new SRVROOTARGUMENTINFO[m_nTexture];
		m_TextureUploadBuffer = new ID3D12Resource*[m_nTexture];
		m_Texture = new ID3D12Resource*[m_nTexture];
	}
	m_nSampler = nSampler;

	if (m_nSampler > 0)
		m_SamplerGpuDescriptorHandle = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSampler];

}

Texture::~Texture()
{
	if (m_Texture)
		for (int i = 0; i < m_nTexture; ++i)
			if (m_Texture[i]) m_Texture[i]->Release();

	if (m_RootArgumentInfo)
		delete[] m_RootArgumentInfo;

	if (m_SamplerGpuDescriptorHandle)
		delete[] m_SamplerGpuDescriptorHandle;
}

void Texture::SetRootArgument(int Index, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE SrvGpuDescriptorHandle)
{
	m_RootArgumentInfo[Index].m_nRootParameterIndex = nRootParameterIndex;
	m_RootArgumentInfo[Index].m_SrvGpuDescriptorHandle = SrvGpuDescriptorHandle;
}

void Texture::UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList)
{
	if (m_nTextureType == RESOURCE_TEXTURE2D_ARRAY) {
		CommandList->SetGraphicsRootDescriptorTable(m_RootArgumentInfo[0].m_nRootParameterIndex, m_RootArgumentInfo[0].m_SrvGpuDescriptorHandle);
	}
	else {
#ifdef SEPARATE_DESCRIPTOR_RANGE
		for (int i = 0; i < m_nTexture; ++i)
			CommandList->SetGraphicsRootDescriptorTable(m_RootArgumentInfo[i].m_nRootParameterIndex, m_RootArgumentInfo[i].m_SrvGpuDescriptorHandle);
#else
		CommandList->SetGraphicsRootDescriptorTable(m_RootArgumentInfo[0].m_nRootParameterIndex, m_RootArgumentInfo[0].m_SrvGpuDescriptorHandle);
#endif
	}
}

void Texture::UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList, int Index)
{
	CommandList->SetGraphicsRootDescriptorTable(m_RootArgumentInfo[Index].m_nRootParameterIndex, m_RootArgumentInfo[Index].m_SrvGpuDescriptorHandle);
}

void Texture::LoadTextureFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, wchar_t *FileName, UINT Index)
{
	m_Texture[Index] = ::CreateTextureResourceFromFile(Device, CommandList, FileName, &m_TextureUploadBuffer[Index], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}


MaterialColor::MaterialColor(MATERIALLOADINFO *MaterialInfo)
{
	m_Diffuse = MaterialInfo->m_AlbedoColor;
	m_Specular = MaterialInfo->m_SpecularColor;
	m_Specular.w = MaterialInfo->m_Glossiness*255.f;
	m_Emissive = MaterialInfo->m_EmissiveColor;
}

MaterialColor::~MaterialColor()
{

}

Shader *Material::m_IlluminatedShader = NULL;
Shader *Material::m_StandardShader = NULL;
Shader *Material::m_SkinnedAnimationShader = NULL;

Material::Material(int nTexture)
{
	m_nTexture = nTexture;

	m_Texture = new Texture*[m_nTexture];
	m_TextureName = new _TCHAR[m_nTexture][64];

	for (int i = 0; i < m_nTexture; ++i)
		m_Texture[i] = NULL;
	for (int i = 0; i < m_nTexture; ++i)
		m_TextureName[i][0] = '\0';
}

Material::~Material()
{

}

void Material::SetTexture(Texture *Texture, UINT nTexture)
{
	if (m_Texture[nTexture]) m_Texture[nTexture]->Release();
	m_Texture[nTexture] = Texture;
	if (m_Texture[nTexture]) m_Texture[nTexture]->AddRef();
}

void Material::SetShader(Shader *Shader)
{
	if (m_Shader) m_Shader->Release();
	m_Shader = Shader;
	if (m_Shader) m_Shader->AddRef();
}

void Material::SetMaterialColor(MaterialColor *MaterialColor)
{
	if (m_MaterialColor) m_MaterialColor->Release();
	m_MaterialColor = MaterialColor;
	if (m_MaterialColor) m_MaterialColor->AddRef();
}

void Material::PrepareShader(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature)
{
	m_StandardShader = new StandardShader();
	m_StandardShader->CreateShader(Device, CommandList, GraphicsRootSignature);

}

void Material::UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList)
{
	CommandList->SetGraphicsRoot32BitConstants(1, 4, &(m_MaterialColor), 16);
	CommandList->SetGraphicsRoot32BitConstants(1, 4, &(m_MaterialColor), 20);
	CommandList->SetGraphicsRoot32BitConstants(1, 4, &(m_MaterialColor), 24);
	CommandList->SetGraphicsRoot32BitConstants(1, 4, &(m_MaterialColor), 28);

	for (int i = 0; i < m_nTexture; ++i)
		if (m_Texture[i]) m_Texture[i]->UpdateShaderVariable(CommandList, 0);
}

void Material::LoadTexutreFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, UINT nType, UINT nRootParameter, _TCHAR *TextureName, Texture **ObjTexture, FILE *InFile, GameObject *Parent, Shader *Shader)
{
	char strTextureName[64] = { '\0' };

	BYTE nStrLength = 64;
	UINT nRead = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, InFile);
	nRead = (UINT)::fread(strTextureName, sizeof(char), nStrLength, InFile);
	strTextureName[nStrLength] = '\0';

	bool bDuplicated = false;
	if (strcmp(strTextureName, "null")) {

		SetType(nType);

		char FilePath[64] = { '\0' };
		strcpy_s(FilePath, 64, "Model/Textures/");

		bDuplicated = (strTextureName[0] == '@');
		strcpy_s(FilePath + 15, 64 - 15, (bDuplicated) ? (strTextureName + 1) : strTextureName);
		strcpy_s(FilePath + 15 + ((bDuplicated) ? (nStrLength - 1) : nStrLength), 64 - 15 - ((bDuplicated) ? (nStrLength - 1) : nStrLength), ".dds");

		size_t nConverted = 0;
		mbstowcs_s(&nConverted, TextureName, 64, FilePath, _TRUNCATE);

#ifdef _WITH_DISPLAY_TEXTURE_NAME
		static int nTextures = 0, nRepeatedTextures = 0;
		TCHAR pstrDebug[256] = { 0 };
		_stprintf_s(pstrDebug, 256, _T("Texture Name: %d %c %s\n"), (strTextureName[0] == '@') ? nRepeatedTextures++ : nTextures++, (strTextureName[0] == '@') ? '@' : ' ', TextureName);
		OutputDebugString(pstrDebug);
#endif
		if (!bDuplicated) {
			*ObjTexture = new Texture(1, RESOURCE_TEXTURE2D, 0);
			(*ObjTexture)->LoadTextureFromFile(Device, CommandList, TextureName, 0);
			if (*ObjTexture) (*ObjTexture)->AddRef();
			//GameScene
		}
		else {
			if (Parent) {
				while (Parent) {
					if (!Parent->m_Parent) break;
					Parent = Parent->m_Parent;
				}
				GameObject *RootGameObj = Parent;
				*ObjTexture = RootGameObj->FindReplicatedTexture(TextureName);
				if (*ObjTexture) (*ObjTexture)->AddRef();
			}
		}
	}
}

// Game Obejct //
GameObject::GameObject()
{
	m_TransformPos = Matrix4x4::Identity();
	m_WorldPos = Matrix4x4::Identity();
}

GameObject::~GameObject()
{

}

void GameObject::AddRef()
{
	++m_nReference;

	if (m_Sibling) m_Sibling->AddRef();
	if (m_Child) m_Child->AddRef();
}

void GameObject::Release()
{
	if (m_Child) m_Child->Release();
	if (m_Sibling) m_Sibling->Release();
}

void GameObject::SetMesh(Mesh *Mesh)
{
	if (m_Mesh) m_Mesh->Release();
	m_Mesh = Mesh;
	if (m_Mesh) m_Mesh->AddRef();
}

void GameObject::SetShader(int nMaterial, Shader *Shader)
{
	if (m_Material[nMaterial]) m_Material[nMaterial]->SetShader(Shader);
}

void GameObject::SetMaterial(int nMaterial, Material *Material)
{
	if (m_Material[nMaterial]) m_Material[nMaterial]->Release();
	m_Material[nMaterial] = Material;
	if (m_Material[nMaterial]) m_Material[nMaterial]->AddRef();
}

void GameObject::SetChild(GameObject *Child, bool ReferenceUpdate)
{
	if (Child) {
		Child->m_Parent = this;
		if (ReferenceUpdate) Child->AddRef();
	}
	if (m_Child) {
		if (Child) Child->m_Sibling = m_Child->m_Sibling;
		m_Child->m_Sibling = Child;
	}
	else
		m_Child = Child;
}

void GameObject::UpdateTransform(XMFLOAT4X4 *Parent)
{
	m_WorldPos = (Parent) ? Matrix4x4::Multiply(m_TransformPos, *Parent) : m_TransformPos;

	if (m_Sibling) m_Sibling->UpdateTransform(Parent);
	if (m_Child) m_Child->UpdateTransform(&m_WorldPos);
}

void GameObject::SetPostion(XMFLOAT3 Position)
{
	m_TransformPos._41 = Position.x;
	m_TransformPos._42 = Position.y;
	m_TransformPos._43 = Position.z;
}

// bin 파일을 읽기 위한 코드 //

int ReadIntegerFromFile(FILE* File)
{
	int nValue = 0;
	UINT nRead = (UINT)::fread(&nValue, sizeof(int), 1, File);

	return nValue;
}

BYTE ReadStringFromFile(FILE *File, char* Token)
{
	BYTE nStrLength = 0;
	UINT nRead = 0;

	nRead = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, File);
	nRead = (UINT)::fread(Token, sizeof(char), nStrLength, File);
	Token[nStrLength] = '\0';

	return nStrLength;
}

// ------------------------- //

MeshLoadInfo *GameObject::LoadMeshInfoFromFile(FILE *InFile)
{
	char Token[64] = { '\0' };
	UINT nRead = 0;

	int nPosition = 0, nColor = 0, nNormal = 0, nIndices = 0, nSubMeshes = 0, nSubIndices = 0;

	MeshLoadInfo *MeshInfo = new MeshLoadInfo;

	MeshInfo->m_nVertices = ::ReadIntegerFromFile(InFile);
	::ReadStringFromFile(InFile, MeshInfo->m_MeshName);

	for (; ;) {
		::ReadStringFromFile(InFile, Token);

		if (!strcmp(Token, "<Bounds>:")) {
			nRead = (UINT)::fread(&(MeshInfo->m_AABBCenter), sizeof(XMFLOAT3), 1, InFile);
			nRead = (UINT)::fread(&(MeshInfo->m_AABBExtent), sizeof(XMFLOAT3), 1, InFile);
		}
		else if (!strcmp(Token, "<Positions>:")) {
			nPosition = ::ReadIntegerFromFile(InFile);
			if (nPosition > 0) {
				MeshInfo->m_nType |= VERTEXT_POSITION;
				MeshInfo->m_Position = new XMFLOAT3[nPosition];
				nRead = (UINT)::fread(MeshInfo->m_Position, sizeof(XMFLOAT3), nPosition, InFile);
			}
		}
		else if (!strcmp(Token, "<Colors>:")) {
			nColor = ::ReadIntegerFromFile(InFile);
			if (nColor > 0) {
				MeshInfo->m_nType |= VERTEXT_COLOR;
				MeshInfo->m_Color = new XMFLOAT4[nColor];
				nRead = (UINT)::fread(MeshInfo->m_Color, sizeof(XMFLOAT4), nColor, InFile);
			}
		}
		else if (!strcmp(Token, "<Normals>:")) {
			nNormal = ::ReadIntegerFromFile(InFile);
			if (nNormal > 0) {
				MeshInfo->m_nType |= VERTEXT_NORMAL;
				MeshInfo->m_Normal = new XMFLOAT3[nNormal];
				nRead = (UINT)::fread(MeshInfo->m_Normal, sizeof(XMFLOAT3), nNormal, InFile);
			}
		}
		else if (!strcmp(Token, "<Indices>:")) {
			nIndices = ::ReadIntegerFromFile(InFile);
			if (nIndices > 0) {
				MeshInfo->m_pnIndices = new UINT[nIndices];
				nRead = (UINT)::fread(MeshInfo->m_pnIndices, sizeof(UINT), nIndices, InFile);
			}
		}
		else if (!strcmp(Token, "<SubMeshes>:")) {
			MeshInfo->m_nSubMeshes = ::ReadIntegerFromFile(InFile);

			if (MeshInfo->m_nSubMeshes > 0) {
				MeshInfo->m_nSubSetIndices = new int[MeshInfo->m_nSubMeshes];
				MeshInfo->m_pnSubSetIndices = new UINT*[MeshInfo->m_nSubMeshes];

				for (int i = 0; i < MeshInfo->m_nSubMeshes; ++i) {
					::ReadStringFromFile(InFile, Token);

					if (!strcmp(Token, "<SubMesh>:")) {
						int nIndex = ::ReadIntegerFromFile(InFile);
						MeshInfo->m_nSubSetIndices[i] = ::ReadIntegerFromFile(InFile);

						if (MeshInfo->m_nSubSetIndices[i] > 0) {
							MeshInfo->m_pnSubSetIndices[i] = new UINT[MeshInfo->m_nSubSetIndices[i]];
							nRead = (UINT)::fread(MeshInfo->m_pnSubSetIndices[i], sizeof(int), MeshInfo->m_nSubSetIndices[i], InFile);
						}
					}
				}
			}
		}
		else if (!strcmp(Token, "</Mesh>"))
			break;
	}

	return MeshInfo;
}

void GameObject::LoadMaterialInfoFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, FILE *InFile, GameObject *Parent, Shader *Shader)
{
	char Token[64] = { '\0' };
	UINT nRead = 0;
	int nMaterial = 0;

	m_nMaterial = ::ReadIntegerFromFile(InFile);

	m_Material = new Material*[m_nMaterial];
	for (int i = 0; i < m_nMaterial; ++i) m_Material[i] = NULL;

	Material *ObjMaterial = NULL;

	for (; ;) {
		::ReadStringFromFile(InFile, Token);

		if (!strcmp(Token, "<Material>:")) {
			nMaterial = ::ReadIntegerFromFile(InFile);

			ObjMaterial = new Material(7);

			if (!Shader) {
				UINT nMeshType = GetMeshType();
				if (nMeshType & VERTEXT_NORMAL_TANGENT_TEXTURE)
					if (nMeshType & VERTEXT_BONE_INDEX_WEIGHT)
						ObjMaterial->SetSkinnedAnimationShader();
					else
						ObjMaterial->SetStandardShader();
			}
			SetMaterial(nMaterial, ObjMaterial);
		}
		else if (!strcmp(Token, "<AlbedoColor>:"))
			nRead = (UINT)::fread(&(ObjMaterial->m_Albedo), sizeof(float), 4, InFile);
		else if (!strcmp(Token, "<EmissiveColor>:"))
			nRead = (UINT)::fread(&(ObjMaterial->m_Emissive), sizeof(float), 4, InFile);
		else if (!strcmp(Token, "<SpecularColor>:"))
			nRead = (UINT)::fread(&(ObjMaterial->m_Specular), sizeof(float), 4, InFile);
		else if (!strcmp(Token, "<Glossiness>:"))
			nRead = (UINT)::fread(&(ObjMaterial->m_Glossiness), sizeof(float), 1, InFile);
		else if (!strcmp(Token, "<Smoothness>:"))
			nRead = (UINT)::fread(&(ObjMaterial->m_Smoothness), sizeof(float), 1, InFile);
		else if (!strcmp(Token, "<Metallic>:"))
			nRead = (UINT)::fread(&(ObjMaterial->m_Metallic), sizeof(float), 1, InFile);
		else if (!strcmp(Token, "<SpecularHighlight>:"))
			nRead = (UINT)::fread(&(ObjMaterial->m_SpecularHighlight), sizeof(float), 1, InFile);
		else if (!strcmp(Token, "<GlossyReflection>:"))
			nRead = (UINT)::fread(&(ObjMaterial->m_GlossyReflection), sizeof(float), 1, InFile);
		else if (!strcmp(Token, "<AlbedoMap>:"))
			m_Material[nMaterial]->LoadTexutreFromFile(Device, CommandList, MATERIAL_ALBEDO_MAP, 3, ObjMaterial->m_TextureName[0], &(ObjMaterial->m_Texture[0]), InFile, Parent, Shader);
		else if (!strcmp(Token, "<SpecularMap>:"))
			m_Material[nMaterial]->LoadTexutreFromFile(Device, CommandList, MATERIAL_SPECULAR_MAP, 4, ObjMaterial->m_TextureName[1], &(ObjMaterial->m_Texture[1]), InFile, Parent, Shader);
		else if (!strcmp(Token, "<NormalMap>:"))
			m_Material[nMaterial]->LoadTexutreFromFile(Device, CommandList, MATERIAL_ALBEDO_MAP, 5, ObjMaterial->m_TextureName[2], &(ObjMaterial->m_Texture[2]), InFile, Parent, Shader);
		else if (!strcmp(Token, "<MetallicMap>:"))
			m_Material[nMaterial]->LoadTexutreFromFile(Device, CommandList, MATERIAL_ALBEDO_MAP, 6, ObjMaterial->m_TextureName[3], &(ObjMaterial->m_Texture[3]), InFile, Parent, Shader);
		else if (!strcmp(Token, "<EmissionMap>:"))
			m_Material[nMaterial]->LoadTexutreFromFile(Device, CommandList, MATERIAL_ALBEDO_MAP, 7, ObjMaterial->m_TextureName[4], &(ObjMaterial->m_Texture[4]), InFile, Parent, Shader);
		else if (!strcmp(Token, "<DetailAlbedoMap>:"))
			m_Material[nMaterial]->LoadTexutreFromFile(Device, CommandList, MATERIAL_ALBEDO_MAP, 8, ObjMaterial->m_TextureName[5], &(ObjMaterial->m_Texture[5]), InFile, Parent, Shader);
		else if (!strcmp(Token, "<DetailNormalMap>:"))
			m_Material[nMaterial]->LoadTexutreFromFile(Device, CommandList, MATERIAL_ALBEDO_MAP, 9, ObjMaterial->m_TextureName[6], &(ObjMaterial->m_Texture[6]), InFile, Parent, Shader);
		else if (!strcmp(Token, "</Materials>"))
			break;
	}
}

GameObject *GameObject::LoadFrameHierarchyFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature, FILE *InFile, GameObject *Parent, Shader *Shader)
{
	char Token[64] = { '\0' };
	UINT nRead = 0;
	int nFrame = 0, nTexture = 0;

	GameObject *GameObj = NULL;

	for (; ;) {
		::ReadStringFromFile(InFile, Token);
		if (!strcmp(Token, "<Frame>:")) {

			GameObj = new GameObject();

			nRead = (UINT)::fread(&nFrame, sizeof(int), 1, InFile);
			nRead = (UINT)::fread(&nTexture, sizeof(int), 1, InFile);

			::ReadStringFromFile(InFile, GameObj->m_FrameName);
		}
		else if (!strcmp(Token, "<Transform>:")) {
			XMFLOAT3 Position, Rotation, Scale;
			XMFLOAT4 Quaternion;
			nRead = (UINT)::fread(&Position, sizeof(float), 3, InFile);
			nRead = (UINT)::fread(&Rotation, sizeof(float), 3, InFile);
			nRead = (UINT)::fread(&Scale, sizeof(float), 3, InFile);
			nRead = (UINT)::fread(&Quaternion, sizeof(float), 4, InFile);
		}
		else if (!strcmp(Token, "<TransformMatrix>:"))
			nRead = (UINT)::fread(&GameObj->m_TransformPos, sizeof(float), 16, InFile);
		else if (!strcmp(Token, "<Mesh>:")) {
			StandardMesh *ObjMesh = new StandardMesh(Device, CommandList);
			ObjMesh->LoadMeshFromFile(Device, CommandList, InFile);
			if (ObjMesh)
				GameObj->SetMesh(ObjMesh);
		}
		else if (!strcmp(Token, "<SkinningInfo>:")) {
			SkinnedMesh *ObjMesh = new SkinnedMesh(Device, CommandList);
			ObjMesh->CreateShaderVariable(Device, CommandList);

			ObjMesh->LoadSkinInfoFromFile(Device, CommandList, InFile);

			::ReadStringFromFile(InFile, Token);
			ObjMesh->LoadMeshFromFile(Device, CommandList, InFile);
			
			GameObj->SetMesh(ObjMesh);
		}
		else if (!strcmp(Token, "<Materials>:")) {
			GameObj->LoadMaterialInfoFromFile(Device, CommandList, InFile, Parent, Shader);
		}
		else if (!strcmp(Token, "<Children>:")) {
			int nChild = ::ReadIntegerFromFile(InFile);
			if (nChild > 0)
				for (int i = 0; i < nChild; ++i) {
					GameObject *Child = GameObject::LoadFrameHierarchyFromFile(Device, CommandList, GraphicsRootSignature, InFile, GameObj, Shader);
					if (Child)
						GameObj->SetChild(Child);
				}
		}
		else if (!strcmp(Token, "</Frame>"))
			break;
	}
	return GameObj;
}

GameObject *GameObject::LoadGeometryAndAnimationFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature, char *FileName, Shader *Shader, bool Animation)
{
	FILE *File = NULL;
	::fopen_s(&File, FileName, "rb");
	::rewind(File);

	GameObject *GameObj = GameObject::LoadFrameHierarchyFromFile(Device, CommandList, GraphicsRootSignature, File, NULL, Shader);

	//GameObj->CacheSkinningBoneFrame(GameObj);

	if (Animation) {

	}

	return GameObj;
}

Texture *GameObject::FindReplicatedTexture(_TCHAR *TextureName)
{
	for (int i = 0; i < m_nMaterial; ++i) {
		if (m_Material[i]) {
			for (int j = 0; j < m_Material[i]->GetTextureNum(); ++j) {
				if (m_Material[i]->m_Texture[j]) {
					if (!_tcsncmp(m_Material[i]->m_TextureName[j], TextureName, _tcslen(TextureName)))
						return m_Material[i]->m_Texture[j];
				}
			}
		}
	}
	Texture *ObjTexture = NULL;

	if (m_Sibling) if (ObjTexture = m_Sibling->FindReplicatedTexture(TextureName))
		return ObjTexture;
	if (m_Child) if (ObjTexture = m_Child->FindReplicatedTexture(TextureName))
		return ObjTexture;
}

void GameObject::CreateShaderVariable(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList)
{
	UINT ncbElementByte = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);
	m_cbGameObject = ::CreateBufferResource(Device, CommandList, NULL, ncbElementByte, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cbGameObject->Map(0, NULL, (void**)&m_cbMappedGameObject);
}

void GameObject::UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList, XMFLOAT4X4 *WorldPos)
{
	XMFLOAT4X4 World;
	XMStoreFloat4x4(&World, XMMatrixTranspose(XMLoadFloat4x4(WorldPos)));
	CommandList->SetGraphicsRoot32BitConstants(1, 16, &World, 0);
}

void GameObject::Animate(float ElapsedTime, XMFLOAT3 Position)
{
	XMFLOAT3 Look = Vector3::Subtract(m_Position, Position);
	Look = Vector3::Normalize(Look);

	m_TransformPos._31 = Look.x;
	m_TransformPos._32 = Look.y;
	m_TransformPos._33 = Look.z;

	m_TransformPos._21 = 0.f;
	m_TransformPos._22 = 1.f;
	m_TransformPos._23 = 0.f;

	XMFLOAT3 Right = Vector3::CrossProduct(m_Up, Look, true);
	m_TransformPos._11 = Right.x;
	m_TransformPos._12 = Right.y;
	m_TransformPos._13 = Right.z;
}

void GameObject::Render(ID3D12GraphicsCommandList *CommandList)
{
	UpdateShaderVariable(CommandList, &m_WorldPos);
	
	if (m_nMaterial > 0) {
		for (int i = 0; i < m_nMaterial; ++i) {
			if (m_Material[i]) {
				if (m_Material[i]->m_Shader)
					m_Material[i]->m_Shader->OnPrepareRender(CommandList, 0);
				m_Material[i]->UpdateShaderVariable(CommandList);
			}
			m_Mesh->Render(CommandList, i);
		}
	}
	if (m_Sibling) m_Sibling->Render(CommandList);
	if (m_Child) m_Child->Render(CommandList);
}

void GameObject::CacheSkinningBoneFrame(GameObject *RootFrame)
{
	if (m_Mesh && (m_Mesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT)) {
		
		SkinnedMesh *ObjMesh = (SkinnedMesh *)m_Mesh;
		
		for (int i = 0; i < ObjMesh->GetSkinningBoneNum(); ++i) {

		}
	}
}


// UI
UI::UI(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature)
{
	TextureMesh *ObjMesh = new TextureMesh(Device, CommandList, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f);
	SetMesh(ObjMesh);

	Texture *ObjTexture = new Texture(1, RESOURCE_TEXTURE2D, 0);
	ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/JSTM_Title.dds", 0);

	UIShader *ObjShader = new UIShader();
	ObjShader->CreateShader(Device, CommandList, GraphicsRootSignature);
	ObjShader->CreateCbvSrvDescriptorHeap(Device, CommandList, 0, 1);
	ObjShader->CreateShaderResourceView(Device, CommandList, ObjTexture, 1, false);

	Material *ObjMaterial = new Material(1);
	ObjMaterial->SetTexture(ObjTexture);
	MATERIALLOADINFO *ObjMaterialInfo = new MATERIALLOADINFO();
	MaterialColor *ObjMaterialColor = new MaterialColor(ObjMaterialInfo);
	ObjMaterial->SetMaterialColor(ObjMaterialColor);
	m_nMaterial = 1;

	m_Material = new Material*();
	m_Material[0] = NULL;
	SetMaterial(0, ObjMaterial);
	SetCbvGPUDescriptorHandle(ObjShader->GetGPUCbvDescriptorStartHandle());
	SetShader(0, ObjShader);
	
	ObjMaterial = NULL;

}

UI::~UI()
{

}

void UI::Render(ID3D12GraphicsCommandList *CommandList)
{
	if (m_nMaterial > 0) {
		for (int i = 0; i < m_nMaterial; ++i) {
			if (m_Material[i]->m_Shader)
				m_Material[i]->m_Shader->OnPrepareRender(CommandList, 0);
			m_Material[i]->UpdateShaderVariable(CommandList);
			if (m_Mesh)
				m_Mesh->Render(CommandList);
		}
	}
}


// 함정
void Trap::Render(ID3D12GraphicsCommandList *CommandList)
{

}


// 함정 윗부분
TrapCover::TrapCover(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature, int Type)
{
	TextureMesh *ObjMesh = new TextureMesh(Device, CommandList, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f);
	SetMesh(ObjMesh);

	//CreateShaderVariable(Device, CommandList);

	Texture *ObjTexture = new Texture(1, RESOURCE_TEXTURE2D, 0);
	
	// 함정 윗부분의 타입에 맞는 이미지를 로드
	switch (Type)
	{
	case 0:
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/JSTM_Title.dds", 0);
		break;

	default:
		break;
	}

	TrapShader *ObjShader = new TrapShader();
	ObjShader->CreateShader(Device, CommandList, GraphicsRootSignature);
	//ObjShader->CreateCbvSrvDescriptorHeap(Device, CommandList, 0, 1);
	ObjShader->CreateShaderResourceView(Device, CommandList, ObjTexture, 2, false);

	Material *ObjMaterial = new Material(1);
	ObjMaterial->SetTexture(ObjTexture);
	MATERIALLOADINFO *ObjMaterialInfo = new MATERIALLOADINFO();
	MaterialColor *ObjMaterialColor = new MaterialColor(ObjMaterialInfo);
	ObjMaterial->SetMaterialColor(ObjMaterialColor);
	m_nMaterial = 1;

	m_Material = new Material*();
	m_Material[0] = NULL;
	SetMaterial(0, ObjMaterial);
	SetCbvGPUDescriptorHandle(ObjShader->GetGPUCbvDescriptorStartHandle());
	SetShader(0, ObjShader);

	ObjMaterial = NULL;
}

TrapCover::~TrapCover()
{

}

void TrapCover::Render(ID3D12GraphicsCommandList *CommandList)
{
	UpdateShaderVariable(CommandList, &m_WorldPos);

	if (m_nMaterial > 0) {
		for (int i = 0; i < m_nMaterial; ++i) {
			if (m_Material[i]->m_Shader)
				m_Material[i]->m_Shader->OnPrepareRender(CommandList, 0);
			m_Material[i]->UpdateShaderVariable(CommandList);
			if (m_Mesh)
				m_Mesh->Render(CommandList);
		}
	}
}