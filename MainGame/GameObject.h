#pragma once

#include "Mesh.h"

class Shader;
class Texture;

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

struct SRVROOTARGUMENTINFO
{
	UINT							m_nRootParameterIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_SrvGpuDescriptorHandle;
};

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4						m_WorldPos;
};

class Texture
{
public:
	Texture(int nTextureResource, UINT ResourceType, int nSampler);
	virtual ~Texture();

private:
	int								m_nReference = 0;

	UINT							m_nTextureType = RESOURCE_TEXTURE2D;
	int								m_nTexture = 0;

	ID3D12Resource					**m_Texture = NULL;
	ID3D12Resource					**m_TextureUploadBuffer;

	SRVROOTARGUMENTINFO				*m_RootArgumentInfo = NULL;

	int								m_nSampler = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		*m_SamplerGpuDescriptorHandle = NULL;

public:
	void AddRef() { ++m_nReference; }
	void Release() { if (--m_nReference <= 0) delete this; }

	void SetRootArgument(int Index, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE SrvGpuDescriptorHandle);

	void UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList);
	void UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList, int Index);

	void LoadTextureFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, wchar_t *FileName, UINT Index);

	int GetTextureNum() { return m_nTexture; }
	UINT GetTextureType() { return m_nTextureType; }
	ID3D12Resource *GetTexture(int Index) { return m_Texture[Index]; }

};


struct MATERIALLOADINFO
{
	XMFLOAT4			m_AlbedoColor = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	XMFLOAT4			m_EmissiveColor = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
	XMFLOAT4			m_SpecularColor = XMFLOAT4(0.f, 0.f, 0.f, 1.f);

	float				m_Glossiness = 0.f;
	float				m_Smoothness = 0.f;
	float				m_SpecularHighlight = 0.f;
	float				m_Metallic = 0.f;
	float				m_GlossyReflection = 0.f;

	UINT				m_nType = 0x00;
};

struct MATERIALSLOADINFO
{
	int					m_nMaterial = 0;
	MATERIALLOADINFO	*m_Material = NULL;
};

class Material
{
public:
	Material(int nTexture);
	~Material();

private:

	int				m_nTexture = 0;

	int				m_nType = 0x00;

	int				m_bRed = 0x00;

	static Shader	*m_IlluminatedShader;
	static Shader	*m_StandardShader;
	static Shader	*m_SkinnedAnimationShader;

	int				m_nReference = 0;

public:
	Texture * *m_Texture = NULL;
	Shader			*m_Shader = NULL;
	_TCHAR(*m_TextureName)[64] = NULL;

public:
	void AddRef() { ++m_nReference; }
	void Release() { if (--m_nReference <= 0) delete this; }

	void SetTexture(Texture *Texture, UINT nTexture = 0);
	void SetShader(Shader *Shader);
	void SetType(UINT nType) { m_nType |= nType; }

	void SetRed(UINT bRed) { m_bRed = bRed; }

	void SetIlluminatedShader() { SetShader(m_IlluminatedShader); }
	void SetStandardShader() { SetShader(m_StandardShader); }
	void SetSkinnedAnimationShader() { Material::SetShader(m_SkinnedAnimationShader); }

	static void PrepareShader(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature);
	void UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList);

	void LoadTexutreFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, UINT nType, UINT nRootParameter, _TCHAR *TextureName, Texture **Texture, FILE *InFile, GameObject *Parent, Shader *Shader, Material *ObjMaterial);

	XMFLOAT4 m_Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f);
	XMFLOAT4 m_Albedo = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
	XMFLOAT4 m_Emissive = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
	XMFLOAT4 m_Specular = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
	float	 m_Glossiness = 0.f;
	float	 m_Smoothness = 0.f;
	float	 m_Metallic = 0.f;
	float	 m_SpecularHighlight = 0.f;
	float	 m_GlossyReflection = 0.f;

	int GetTextureNum() { return m_nTexture; }
};


// �ִϸ��̼�
#define ANIMATION_TYPE_ONCE			0
#define ANIMATION_TYPE_LOOP			1
#define ANIMATION_TYPE_SHOOT		2
#define ANIMATION_TYPE_RELOAD		3

#define _WITH_ANIMATION_INTERPOLATION

class AnimationSet
{
public:
	AnimationSet() { }
	~AnimationSet();

public:
	char			m_strName[64];

	float			m_Length = 0.f;
	int				m_nFramePerSecond = 0;

	int				m_nKeyFrameTransform = 0;
	float			*m_KeyFrameTransformTime = NULL;
	XMFLOAT4X4		**m_KeyFrameTransform = NULL;

	float			m_Speed = 1.f;
	float			m_Position = 0.f;

	float			m_StartPosition = -1.0f;
	float			m_TranslatePosition = 0.0f;

	bool			m_bAnimateChange = false;

	float			m_ReloadPosition = -1.0f; // reload, ��� �Ҷ� �̵��� �����뵵
	bool			m_bReloadEnd = false;

	int				m_nType = ANIMATION_TYPE_LOOP;

	int				m_nCurrentKey = -1;

public:
	XMFLOAT4X4 GetSRT(int nFrame, float Position);

	float GetPosition(float Position);
	void *GetCallback(float Position) { return NULL; }
};

class AnimationTrack
{
public:
	AnimationTrack() { }
	~AnimationTrack() { }

public:
	AnimationSet * m_AnimationSet = NULL;

	float			m_Speed = 1.f;
	float			m_Position = 0.f;
	float			m_Weight = 1.f;
};

class AnimationCallbackHandler
{
public:
	virtual void HandleCallback(void *CallbackData) { }
};

class AnimationController
{
public:
	AnimationController(int nAnimationTrack = 1);
	~AnimationController();

public:
	float				m_Time = 0.f;

	int					m_nAnimationTrack = 0;
	AnimationTrack		*m_AnimationTrack = NULL;

	int					m_nAnimationSet = 0;
	AnimationSet		*m_AnimationSet = NULL;

	int					m_nAnimationBoneFrame = 0;
	GameObject			**m_AnimationBoneFrameCache = NULL;

	int					m_nNowAnimation = 0;

	int					m_nNextAnimation = -1;

public:
	void SetAnimationSet(int nAnimationSet);
	void SetAnimationEnable(int nAnimationSet);
	void SetAnimateControlType(int nAnimationSet, int nType);
	void SetPlayerAnimateType(int nType);
	void AdvanceTime(float ElapsedTime, AnimationCallbackHandler *CallbackHandler);
};


class GameObject
{
public:
	GameObject();
	GameObject(int nMaterial);
	~GameObject();

protected:
	int								m_nReference = 0;

	Mesh							*m_Mesh = NULL;

	Material						**m_Material = NULL;
	int								m_nMaterial = 0;

public:
	XMFLOAT4X4						m_WorldPos;
	XMFLOAT4X4						m_TransformPos;

protected:
	D3D12_GPU_DESCRIPTOR_HANDLE		m_CbvGPUDescriptorHandle;

	char							m_FrameName[64];

	GameObject						*m_Child = NULL;
	GameObject						*m_Sibling = NULL;

	// ������Ʈ�� Right, Up, Look, Position ����
	XMFLOAT3						m_Right;
	XMFLOAT3						m_Up;
	XMFLOAT3						m_Look;
	XMFLOAT3						m_Position;

	XMFLOAT3						m_CamRight;
	XMFLOAT3						m_CamUp;
	XMFLOAT3						m_CamLook;

	ID3D12Resource					*m_cbGameObject = NULL;
	CB_GAMEOBJECT_INFO				*m_cbMappedGameObject = NULL;

	short							m_AnimateState = 0;

public:
	GameObject * m_Parent = NULL;

	AnimationController				*m_AnimationController = NULL;

public:

	void SetRed(UINT bRed) {
		if (m_nMaterial > 0) {
			for (int i = 0; i < m_nMaterial; ++i) {
				if (m_Material[i]) {
					m_Material[i]->SetRed(bRed);
				}
			}
		}
		if (m_Sibling) m_Sibling->SetRed(bRed);
		if (m_Child) m_Child->SetRed(bRed);
	}

	float GetMeshDistance() { return m_Mesh->m_fDistance; }

	void AddRef();
	void Release();

	void SetMesh(Mesh *Mesh);
	void SetShader(int nMaterial, Shader *Shader);
	void SetMaterial(int nMaterial, Material *Material);

	void SetChild(GameObject *Child, bool ReferenceUpdate = false);
	void SetObject(GameObject *Obj);

	void UpdateTransform(XMFLOAT4X4 *Parent);

	void SetRight(XMFLOAT3 Right);
	void SetUp(XMFLOAT3 Up);
	void SetLook(XMFLOAT3 Look);
	void SetPostion(XMFLOAT3 Position);

	void SetmPosition(XMFLOAT3 Position);

	void SetTransform(XMFLOAT4X4 Transform);

	void SetScale(float x, float y, float z);
	virtual void SetRotate(float Pitch, float Yaw, float Roll);

	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE CbvGPUDescriptorHandle) { m_CbvGPUDescriptorHandle = CbvGPUDescriptorHandle; }

	UINT GetMeshType() { return (m_Mesh) ? m_Mesh->GetType() : 0; }

	XMFLOAT3 GetRight() { return XMFLOAT3(m_TransformPos._11, m_TransformPos._12, m_TransformPos._13); }
	XMFLOAT3 GetUp() { return XMFLOAT3(m_TransformPos._21, m_TransformPos._22, m_TransformPos._23); }
	XMFLOAT3 GetLook() { return XMFLOAT3(m_TransformPos._31, m_TransformPos._32, m_TransformPos._33); }
	XMFLOAT3 GetPosition() { return XMFLOAT3(m_TransformPos._41, m_TransformPos._42, m_TransformPos._43); }
	XMFLOAT4X4 GetTransform() { return m_TransformPos; }

	XMFLOAT3 GetCamUp() { return m_CamUp; }
	XMFLOAT3 GetCamRight() { return m_CamRight; }
	XMFLOAT3 GetCamLook() { return m_CamLook; }

	Mesh *GetMesh() { return m_Mesh; }


	virtual void MoveUp(float Distance);
	virtual void MoveForward(float Distance);
	virtual void MoveRight(float Distance);

	void LoadMaterialInfoFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, FILE *InFile, GameObject *Parent, Shader *Shader);

	static GameObject *LoadFrameHierarchyFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature, FILE *InFile, GameObject *Parent, Shader *Shader);
	static GameObject *LoadGeometryAndAnimationFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature, char *FileName, Shader *Shader, bool Animation);

	GameObject *FindFrame(char *FrameName);

	BoundingBox GetBodyBounding();

	GameObject *IsStageIntersect(BoundingBox BodyBound);

	int CheckMonster(XMFLOAT3 startpos, XMFLOAT3 endpos);

	GameObject *CheckTileBound(XMFLOAT3 startpos, XMFLOAT3 endpos, bool IsFloor);

	Texture *FindReplicatedTexture(_TCHAR *TextureName);

	virtual void CreateShaderVariable(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList, XMFLOAT4X4 *WorldPos);

	void SetEnable(int nAnimationSet);
	void SetAnimateType(int nAnimationSet, int nType);
	void SetPlayerAnimateType(int nType);

	short GetAnimateState() { return m_AnimateState; }

	int GetNowAnimationNum() {
		if (m_AnimationController) return m_AnimationController->m_nNowAnimation;
		if (m_Sibling) return m_Sibling->GetNowAnimationNum();
		if (m_Child) return m_Child->GetNowAnimationNum();
		 }

	int GetNextAnimationNum() {
		if (m_AnimationController) return m_AnimationController->m_nNextAnimation;
		if (m_Sibling) return m_Sibling->GetNextAnimationNum();
		if (m_Child) return m_Child->GetNextAnimationNum();
	}

	virtual void OnPrepareRender() { }

	virtual void Animate(float ElapsedTime, XMFLOAT4X4 *Parent = NULL);
	virtual void Render(ID3D12GraphicsCommandList *CommandList);
	void Render(ID3D12GraphicsCommandList *CommandList, UINT InstanceNum, D3D12_VERTEX_BUFFER_VIEW InstanceBufferView);

	// �ִϸ��̼�
	void CacheSkinningBoneFrame(GameObject *RootFrame);
	void LoadAnimationFromFile(FILE *InFile);
};

class UI : public GameObject
{
public:
	UI(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature, float x, float y, int type, int SceneNum);
	~UI();

	virtual void Animate(float ElapsedTime, XMFLOAT4X4 *Parent = NULL);
	virtual void Render(ID3D12GraphicsCommandList *CommandList);

	void SetColor(UINT color) {}
};

class SkyBox : public GameObject
{
public:
	SkyBox(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature, int type);
	~SkyBox();

	virtual void Animate(XMFLOAT3 PlayerPosition);
	virtual void Render(ID3D12GraphicsCommandList *CommandList);
};

class Effect : public GameObject
{
public:
	Effect() {}
	Effect(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature, UINT type);
	~Effect() {}

private:
	float flag = 1.f;
	float scale = 1.f;

public:
	virtual void Animate(float ElapsedTime, XMFLOAT4X4 *Parent = NULL);
	virtual void Render(ID3D12GraphicsCommandList *CommandList);
};

#define TRAP_NEEDLE		0
#define TRAP_FIRE		1
#define TRAP_SLOW		2
#define TRAP_ARROW		3

// 3D ��
class Trap : public GameObject
{
public:
	Trap() { }
	~Trap() { }

private:
	bool IsBuildTrap = false;
	bool IsActive = false;

	int m_nTrapKind;

public:
	void BuildTrap(bool Input) { IsBuildTrap = Input; }
	void ActiveTrap(bool Input) { IsActive = Input; }

	bool GetIsBuildTrap() { return IsBuildTrap; }

	void Animate(XMFLOAT3 Position, float ElapsedTime, XMFLOAT4X4 *Parent);

	void SetTrapKind(int nKindnum) { m_nTrapKind = nKindnum; }

	int GetTrapKind() { return m_nTrapKind; }
};

class Monster : public GameObject
{
public:
	Monster() { }
	~Monster() { }

	bool IsClose = false;
	float nInporation = 0.0f;
	int nCheckPoint = 0;
	XMFLOAT3 StartLook = XMFLOAT3(0.0f, 0.0f, 0.0f);
public:
	void SetDirection(XMFLOAT3 Position);
	void SetLookDirection(XMFLOAT3 Look);
	void SetLine(float ElapsedTime);
	void SetinterPolation(XMFLOAT3 DesLook);
};