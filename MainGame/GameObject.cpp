#include "stdafx.h"
#include "GameObject.h"
#include "Shader.h"
#include "Scene.h"

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
	m_Texture[Index] = ::CreateTextureResourceFromFile(Device, CommandList, FileName, &(m_TextureUploadBuffer[Index]), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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

void Material::PrepareShader(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature)
{
	m_StandardShader = new StandardShader();
	m_StandardShader->CreateShader(Device, CommandList, GraphicsRootSignature);

	m_SkinnedAnimationShader = new SkinnedAnimationShader();
	m_SkinnedAnimationShader->CreateShader(Device, CommandList, GraphicsRootSignature);
}

void Material::UpdateShaderVariable(ID3D12GraphicsCommandList *CommandList)
{
	CommandList->SetGraphicsRoot32BitConstants(1, 4, &m_Ambient, 16);
	CommandList->SetGraphicsRoot32BitConstants(1, 4, &m_Albedo, 20);
	CommandList->SetGraphicsRoot32BitConstants(1, 4, &m_Specular, 24);
	CommandList->SetGraphicsRoot32BitConstants(1, 4, &m_Emissive, 28);

	CommandList->SetGraphicsRoot32BitConstants(1, 1, &m_nType, 32);

	CommandList->SetGraphicsRoot32BitConstants(1, 1, &m_bRed, 33);


	for (int i = 0; i < m_nTexture; ++i)
		if (m_Texture[i]) m_Texture[i]->UpdateShaderVariable(CommandList, i);
}

void Material::LoadTexutreFromFile(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, UINT nType, UINT nRootParameter, _TCHAR *TextureName, Texture **ObjTexture, FILE *InFile, GameObject *Parent, Shader *Shader, Material *ObjMaterial)
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

			GameScene::CreateShaderResourceView(Device, CommandList, *ObjTexture, nRootParameter, false);
		}
		else {
			if (Parent) {
				while (Parent) {
					if (!Parent->m_Parent) break;
					Parent = Parent->m_Parent;
				}
				if (*ObjTexture) (*ObjTexture)->AddRef();
				GameObject *RootGameObj = Parent;
				*ObjTexture = RootGameObj->FindReplicatedTexture(TextureName);
			}
		}
	}
}


// Animation //

AnimationSet::~AnimationSet()
{
	if (m_KeyFrameTransformTime) delete[] m_KeyFrameTransformTime;
	for (int j = 0; j < m_nKeyFrameTransform; j++) if (m_KeyFrameTransform[j]) delete[] m_KeyFrameTransform[j];
	if (m_KeyFrameTransform) delete[] m_KeyFrameTransform;
}

float AnimationSet::GetPosition(float Position)
{
	float GetPosition = Position;

	switch (m_nType) {
	case ANIMATION_TYPE_LOOP:
		if (m_bAnimateChange == false) {
			GetPosition = Position - int(Position / m_KeyFrameTransformTime[m_nKeyFrameTransform - 1]) * m_KeyFrameTransformTime[m_nKeyFrameTransform - 1];
		}
		else {
			if (m_StartPosition == -1.0f) {
				m_StartPosition = Position;
			}
			GetPosition = Position - int(Position / m_KeyFrameTransformTime[m_nKeyFrameTransform - 1]) * m_KeyFrameTransformTime[m_nKeyFrameTransform - 1];
			if ((Position - m_StartPosition) / m_KeyFrameTransformTime[m_nKeyFrameTransform - 1] >= 0.1f) {
				m_StartPosition = -1.0f;
				m_bAnimateChange = false;
			}
		}
		break;

	case ANIMATION_TYPE_ONCE:
		if (m_StartPosition == -1.0f) {
			m_StartPosition = Position;
		}
		if (m_bAnimateChange == false) {
			GetPosition = (Position - m_StartPosition) - int((Position - m_StartPosition) / m_KeyFrameTransformTime[m_nKeyFrameTransform - 1]) * (m_KeyFrameTransformTime[m_nKeyFrameTransform - 1]);

			if ((Position - m_StartPosition) / m_KeyFrameTransformTime[m_nKeyFrameTransform - 1] >= 0.9f) {
				m_bAnimateChange = true;
				m_TranslatePosition = Position - m_StartPosition;
			}
		}
		else {
			GetPosition = (Position - m_StartPosition) - int((Position - m_StartPosition) / m_KeyFrameTransformTime[m_nKeyFrameTransform - 1]) * (m_KeyFrameTransformTime[m_nKeyFrameTransform - 1]);

			if ((Position - m_StartPosition) / m_KeyFrameTransformTime[m_nKeyFrameTransform - 1] >= 1.0f) {
				m_StartPosition = -1.0f;
				m_TranslatePosition = 0.0f;
				m_bAnimateChange = false;
			}
		}
		break;

	case ANIMATION_TYPE_SHOOT:
	case ANIMATION_TYPE_RELOAD:
		if (m_StartPosition == -1.0f) {
			m_StartPosition = Position;
		}
		if (m_bAnimateChange == false) {
			GetPosition = (Position - m_StartPosition) - int((Position - m_StartPosition) / m_KeyFrameTransformTime[m_nKeyFrameTransform - 1]) * (m_KeyFrameTransformTime[m_nKeyFrameTransform - 1]);

			if ((Position - m_StartPosition) / m_KeyFrameTransformTime[m_nKeyFrameTransform - 1] >= 0.9f) {
				m_bAnimateChange = true;
				m_bReloadEnd = true; // true이면 이제 SHOOT 애니메이션으로 부르지 않음.
				m_TranslatePosition = Position - m_StartPosition;
			}
		}
		else {
			if (m_bReloadEnd == true) { // 애니메이션이 90프로 끝났을때
				GetPosition = (Position - m_StartPosition) - int((Position - m_StartPosition) / m_KeyFrameTransformTime[m_nKeyFrameTransform - 1]) * (m_KeyFrameTransformTime[m_nKeyFrameTransform - 1]);

				if ((Position - m_StartPosition) / m_KeyFrameTransformTime[m_nKeyFrameTransform - 1] >= 1.0f) {
					m_StartPosition = -1.0f;
					m_TranslatePosition = 0.0f;
					m_bAnimateChange = false;
					m_ReloadPosition = -1.0f;
					m_bReloadEnd = false;
				}
			}
			else { // 애니메이션이 중도 변환된 상태일때
				if (m_ReloadPosition == -1.0f) {
					m_ReloadPosition = Position;
				}
				GetPosition = (Position - m_StartPosition) - int((Position - m_StartPosition) / m_KeyFrameTransformTime[m_nKeyFrameTransform - 1]) * (m_KeyFrameTransformTime[m_nKeyFrameTransform - 1]);
				if ((Position - m_ReloadPosition) / m_KeyFrameTransformTime[m_nKeyFrameTransform - 1] >= 0.1f) {
					m_StartPosition = -1.0f;
					m_TranslatePosition = 0.0f;
					m_bAnimateChange = false;
					m_ReloadPosition = -1.0f;
					m_bReloadEnd = false;
				}
			}
		}
		break;
	}
	return GetPosition;
}

XMFLOAT4X4 AnimationSet::GetSRT(int nFrame, float Position)
{
	XMFLOAT4X4 Transform = Matrix4x4::Identity();

	for (int i = 0; i < m_nKeyFrameTransform - 1; ++i) {
		if ((m_KeyFrameTransformTime[i] <= Position) && (Position <= m_KeyFrameTransformTime[i + 1])) {
			float t = (Position - m_KeyFrameTransformTime[i]) / (m_KeyFrameTransformTime[i + 1] - m_KeyFrameTransformTime[i]);
			XMVECTOR S0, R0, T0, S1, R1, T1;
			XMMatrixDecompose(&S0, &R0, &T0, XMLoadFloat4x4(&m_KeyFrameTransform[i][nFrame]));
			XMMatrixDecompose(&S1, &R1, &T1, XMLoadFloat4x4(&m_KeyFrameTransform[i + 1][nFrame]));
			XMVECTOR S = XMVectorLerp(S0, S1, t);
			XMVECTOR T = XMVectorLerp(T0, T1, t);
			XMVECTOR R = XMQuaternionSlerp(R0, R1, t);
			XMStoreFloat4x4(&Transform, XMMatrixAffineTransformation(S, XMVectorZero(), R, T));
			break;
		}
	}

	return Transform;
}

AnimationController::AnimationController(int nAnimationTrack)
{
	m_nAnimationTrack = nAnimationTrack;
	m_AnimationTrack = new AnimationTrack[nAnimationTrack];
}

AnimationController::~AnimationController()
{
	if (m_AnimationSet) delete[] m_AnimationSet;
	if (m_AnimationBoneFrameCache) delete[] m_AnimationBoneFrameCache;
	if (m_AnimationTrack) delete[] m_AnimationTrack;
}

void AnimationController::SetAnimationSet(int nAnimationSet)
{
	if (m_AnimationSet) {
		for (int i = 0; i < m_nAnimationTrack; ++i)
			m_AnimationTrack[i].m_AnimationSet = &m_AnimationSet[i];
	}
	m_nNowAnimation = nAnimationSet;
}

void AnimationController::SetAnimationEnable(int nAnimationSet)
{
	if (m_AnimationTrack) {
		if (m_nNowAnimation != nAnimationSet && m_nNextAnimation == -1)
		{
			if (m_AnimationTrack[m_nNowAnimation].m_AnimationSet->m_nType == ANIMATION_TYPE_RELOAD)
			{
				if (m_AnimationTrack[m_nNowAnimation].m_AnimationSet->m_bReloadEnd == false)
				{
					if (0 <= nAnimationSet && nAnimationSet <= 8)
					{
						nAnimationSet += 18;
						if (nAnimationSet != m_nNowAnimation)
							m_nNextAnimation = nAnimationSet;
						else
							m_nNextAnimation = -1;
					}
					else
					{
						m_nNextAnimation = -1;
					}
				}
				else
				{
					m_nNextAnimation = nAnimationSet;
				}
			}
			else if (m_AnimationTrack[m_nNowAnimation].m_AnimationSet->m_nType == ANIMATION_TYPE_SHOOT)
			{
				if (m_AnimationTrack[m_nNowAnimation].m_AnimationSet->m_bReloadEnd == false)
				{
					if (0 <= nAnimationSet && nAnimationSet <= 8)
					{
						nAnimationSet += 9;
						if (nAnimationSet != m_nNowAnimation)
							m_nNextAnimation = nAnimationSet;
						else
							m_nNextAnimation = -1;
					}
					else
					{
						m_nNextAnimation = -1;
					}
				}
				else
				{
					m_nNextAnimation = nAnimationSet;
				}
			}
			// LOOP(IDLE/RUN) 애니메이션일때 RELOAD/SHOOT 실행시 LOOP진행중인 것으로 이어지도록
			else if (m_AnimationTrack[m_nNowAnimation].m_AnimationSet->m_nType == ANIMATION_TYPE_LOOP)
			{
				if (nAnimationSet == 18 || nAnimationSet == 9) // reload
				{
					nAnimationSet += m_nNowAnimation;
					m_nNextAnimation = nAnimationSet;
				}
				else
				{
					m_nNextAnimation = nAnimationSet;
					//cout << "anim: " << nAnimationSet << endl;
				}
			}
			else
			{
				m_nNextAnimation = nAnimationSet;
			}
		}
	}
}

void AnimationController::SetAnimateControlType(int nAnimationSet, int nType)
{
	if (m_AnimationTrack) {
		m_AnimationTrack[nAnimationSet].m_AnimationSet->m_nType = nType;
	}
}

void AnimationController::SetPlayerAnimateType(int nType)
{
	if (m_AnimationTrack) {
		switch (nType) {
		case ANIMATION_TYPE_SHOOT:
			this->SetAnimateControlType(9, ANIMATION_TYPE_SHOOT);
			this->SetAnimateControlType(10, ANIMATION_TYPE_SHOOT);
			this->SetAnimateControlType(11, ANIMATION_TYPE_SHOOT);
			this->SetAnimateControlType(12, ANIMATION_TYPE_SHOOT);
			this->SetAnimateControlType(13, ANIMATION_TYPE_SHOOT);
			this->SetAnimateControlType(14, ANIMATION_TYPE_SHOOT);
			this->SetAnimateControlType(15, ANIMATION_TYPE_SHOOT);
			this->SetAnimateControlType(16, ANIMATION_TYPE_SHOOT);
			this->SetAnimateControlType(17, ANIMATION_TYPE_SHOOT);
			break;
		case ANIMATION_TYPE_RELOAD:
			this->SetAnimateControlType(18, ANIMATION_TYPE_RELOAD);
			this->SetAnimateControlType(19, ANIMATION_TYPE_RELOAD);
			this->SetAnimateControlType(20, ANIMATION_TYPE_RELOAD);
			this->SetAnimateControlType(21, ANIMATION_TYPE_RELOAD);
			this->SetAnimateControlType(22, ANIMATION_TYPE_RELOAD);
			this->SetAnimateControlType(23, ANIMATION_TYPE_RELOAD);
			this->SetAnimateControlType(24, ANIMATION_TYPE_RELOAD);
			this->SetAnimateControlType(25, ANIMATION_TYPE_RELOAD);
			this->SetAnimateControlType(26, ANIMATION_TYPE_RELOAD);
		}
	}
}

void AnimationController::AdvanceTime(float ElapsedTime, AnimationCallbackHandler *CallbackHandler)
{
	m_Time += ElapsedTime;

	if (m_AnimationSet) {
		if (m_nNowAnimation < 0 && m_nNowAnimation > 29)
			return;
		m_AnimationTrack[m_nNowAnimation].m_Position += (ElapsedTime * m_AnimationTrack[m_nNowAnimation].m_Speed);
		AnimationSet *pAnimationSet = m_AnimationTrack[m_nNowAnimation].m_AnimationSet;
		pAnimationSet->m_Position += (ElapsedTime * pAnimationSet->m_Speed);

		if (!(pAnimationSet->m_bAnimateChange)) {
			if (m_nNextAnimation >= 0 && m_nNextAnimation <= 29 && pAnimationSet->m_nType == ANIMATION_TYPE_LOOP) {
				if (m_AnimationTrack[m_nNextAnimation].m_AnimationSet->m_nType == ANIMATION_TYPE_LOOP) {
					pAnimationSet->m_bAnimateChange = true;
					m_AnimationTrack[m_nNextAnimation].m_Position = pAnimationSet->m_Position;
					m_AnimationTrack[m_nNextAnimation].m_AnimationSet->m_Position = pAnimationSet->m_Position;
				}
				else if (m_AnimationTrack[m_nNextAnimation].m_AnimationSet->m_nType == ANIMATION_TYPE_ONCE ||
					m_AnimationTrack[m_nNextAnimation].m_AnimationSet->m_nType == ANIMATION_TYPE_SHOOT ||
					m_AnimationTrack[m_nNextAnimation].m_AnimationSet->m_nType == ANIMATION_TYPE_RELOAD) {
					pAnimationSet->m_bAnimateChange = true;
				}
			}
			else if (m_nNextAnimation >= 0 && m_nNextAnimation <= 29 && pAnimationSet->m_nType == ANIMATION_TYPE_RELOAD) {
				if (m_AnimationTrack[m_nNextAnimation].m_AnimationSet->m_nType == ANIMATION_TYPE_RELOAD) {
					pAnimationSet->m_bAnimateChange = true;
					m_AnimationTrack[m_nNextAnimation].m_Position = pAnimationSet->m_Position;
					m_AnimationTrack[m_nNextAnimation].m_AnimationSet->m_Position = pAnimationSet->m_Position;
					m_AnimationTrack[m_nNextAnimation].m_AnimationSet->m_StartPosition = pAnimationSet->m_StartPosition;
				}
			}
			else if (m_nNextAnimation >= 0 && m_nNextAnimation <= 29 && pAnimationSet->m_nType == ANIMATION_TYPE_SHOOT) {
				if (m_AnimationTrack[m_nNextAnimation].m_AnimationSet->m_nType == ANIMATION_TYPE_SHOOT) {
					pAnimationSet->m_bAnimateChange = true;
					m_AnimationTrack[m_nNextAnimation].m_Position = pAnimationSet->m_Position;
					m_AnimationTrack[m_nNextAnimation].m_AnimationSet->m_Position = pAnimationSet->m_Position;
					m_AnimationTrack[m_nNextAnimation].m_AnimationSet->m_StartPosition = pAnimationSet->m_StartPosition;
				}
			}
			float fPositon = pAnimationSet->GetPosition(pAnimationSet->m_Position);
			for (int i = 0; i < m_nAnimationBoneFrame; i++)
			{
				m_AnimationBoneFrameCache[i]->m_TransformPos = pAnimationSet->GetSRT(i, fPositon);
				//애니메이션에서 두 프레임 사이를 보간해준다.
			}
		}
		else if (pAnimationSet->m_bAnimateChange) {
			// ONCE 타입의 애니메이션 실행 후 다음 애니메이션이 -1이면, NextAnimation은 Idle
			if ((m_nNextAnimation == -1 && pAnimationSet->m_nType == ANIMATION_TYPE_ONCE)) {
				m_nNextAnimation = 0;
			}
			// RELOAD 타입이 끝까지 애니메이션 실행 후 다음 애니메이션이 -1이면, NextAnimation은 idle
			if (m_nNextAnimation == -1 && pAnimationSet->m_nType == ANIMATION_TYPE_RELOAD && pAnimationSet->m_bReloadEnd == true) {
				m_nNextAnimation = 0;
			}
			// SHOOT 타입이 끝까지 애니메이션 실행 후 다음 애니메이션이 -1이면, NextAnimation은 idle
			if (m_nNextAnimation == -1 && pAnimationSet->m_nType == ANIMATION_TYPE_SHOOT && pAnimationSet->m_bReloadEnd == true) {
				m_nNextAnimation = 0;
			}

			m_AnimationTrack[m_nNextAnimation].m_Position += (ElapsedTime * m_AnimationTrack[m_nNextAnimation].m_Speed);
			AnimationSet *pNewAnimationSet = m_AnimationTrack[m_nNextAnimation].m_AnimationSet;
			pNewAnimationSet->m_Position += (ElapsedTime * pNewAnimationSet->m_Speed);

			//if (pAnimationSet->m_fStartPosition == -1.0f) {
			//	pAnimationSet->m_fStartPosition = pAnimationSet->m_fPosition;
			//}

			float fPosition = pAnimationSet->GetPosition(pAnimationSet->m_Position);


			float fInterpolPosition;
			if (pAnimationSet->m_nType == ANIMATION_TYPE_LOOP) {
				fInterpolPosition = ((pAnimationSet->m_Position - pAnimationSet->m_StartPosition)) / ((pAnimationSet->m_KeyFrameTransformTime[pAnimationSet->m_nKeyFrameTransform - 1]) * 0.1f);
			}
			else if ((pAnimationSet->m_nType == ANIMATION_TYPE_RELOAD && pAnimationSet->m_bReloadEnd == false) ||
				(pAnimationSet->m_nType == ANIMATION_TYPE_SHOOT && pAnimationSet->m_bReloadEnd == false)) {
				fInterpolPosition = ((pAnimationSet->m_Position) - (pAnimationSet->m_ReloadPosition)) / ((pAnimationSet->m_KeyFrameTransformTime[pAnimationSet->m_nKeyFrameTransform - 1]) * 0.1f);
			}
			else {
				fInterpolPosition = ((pAnimationSet->m_Position - pAnimationSet->m_StartPosition) - (pAnimationSet->m_TranslatePosition)) / ((pAnimationSet->m_KeyFrameTransformTime[pAnimationSet->m_nKeyFrameTransform - 1]) - pAnimationSet->m_TranslatePosition);
			}

			if (!(pAnimationSet->m_bAnimateChange)) {
				if (pAnimationSet->m_nType == ANIMATION_TYPE_RELOAD && pNewAnimationSet->m_nType == ANIMATION_TYPE_RELOAD && pAnimationSet->m_bReloadEnd == true) {
					m_nNextAnimation -= 18;
					pNewAnimationSet->m_bReloadEnd = false;
					pNewAnimationSet->m_bAnimateChange = false;
					pNewAnimationSet->m_ReloadPosition = -1.0f;
					pNewAnimationSet->m_StartPosition = -1.0f;
					pNewAnimationSet->m_TranslatePosition = 0.0f;
				}
				else if (pAnimationSet->m_nType == ANIMATION_TYPE_SHOOT && pNewAnimationSet->m_nType == ANIMATION_TYPE_SHOOT && pAnimationSet->m_bReloadEnd == true) {
					m_nNextAnimation -= 9;
					pNewAnimationSet->m_bReloadEnd = false;
					pNewAnimationSet->m_bAnimateChange = false;
					pNewAnimationSet->m_ReloadPosition = -1.0f;
					pNewAnimationSet->m_StartPosition = -1.0f;
					pNewAnimationSet->m_TranslatePosition = 0.0f;
				}
				m_nNowAnimation = m_nNextAnimation;
				m_nNextAnimation = -1;
				return;
			}

			float fNewPosition = pNewAnimationSet->GetPosition(pNewAnimationSet->m_Position);

			for (int i = 0; i < m_nAnimationBoneFrame; i++)
			{
				m_AnimationBoneFrameCache[i]->m_TransformPos = Matrix4x4::Interpolate(pAnimationSet->GetSRT(i, fPosition), pNewAnimationSet->GetSRT(i, fNewPosition), fInterpolPosition);
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

GameObject::GameObject(int nMaterial) : GameObject()
{
	m_nMaterial = nMaterial;
	if (m_nMaterial > 0) {
		m_Material = new Material*[m_nMaterial];
		for (int i = 0; i < m_nMaterial; ++i) m_Material[i] = NULL;
	}
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

	if (--m_nReference <= 0) delete this;
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

void GameObject::SetRight(XMFLOAT3 Right)
{
	m_TransformPos._11 = Right.x;
	m_TransformPos._12 = Right.y;
	m_TransformPos._13 = Right.z;

	UpdateTransform(NULL);
}

void GameObject::SetUp(XMFLOAT3 Up)
{
	m_TransformPos._21 = Up.x;
	m_TransformPos._22 = Up.y;
	m_TransformPos._23 = Up.z;

	UpdateTransform(NULL);
}

void GameObject::SetLook(XMFLOAT3 Look)
{
	m_TransformPos._31 = Look.x;
	m_TransformPos._32 = Look.y;
	m_TransformPos._33 = Look.z;

	UpdateTransform(NULL);
}

void GameObject::SetmPosition(XMFLOAT3 Position)
{
	m_Position.x = Position.x;
	m_Position.y = Position.y;
	m_Position.z = Position.z;
}

void GameObject::SetPostion(XMFLOAT3 Position)
{
	m_TransformPos._41 = Position.x;
	m_TransformPos._42 = Position.y;
	m_TransformPos._43 = Position.z;

	UpdateTransform(NULL);
}

void GameObject::SetTransform(XMFLOAT4X4 Transform)
{
	m_TransformPos._11 = Transform._11, m_TransformPos._12 = Transform._12, m_TransformPos._13 = Transform._13, m_TransformPos._14 = Transform._14;
	m_TransformPos._21 = Transform._21, m_TransformPos._22 = Transform._22, m_TransformPos._23 = Transform._23, m_TransformPos._14 = Transform._24;
	m_TransformPos._31 = Transform._31, m_TransformPos._32 = Transform._32, m_TransformPos._33 = Transform._33, m_TransformPos._14 = Transform._34;
	m_TransformPos._41 = Transform._41, m_TransformPos._42 = -55.f, m_TransformPos._43 = Transform._43, m_TransformPos._14 = Transform._44;
}

void GameObject::SetScale(float x, float y, float z)
{
	XMMATRIX Scale = XMMatrixScaling(x, y, z);
	m_TransformPos = Matrix4x4::Multiply(Scale, m_TransformPos);

	UpdateTransform(NULL);
}

void GameObject::SetRotate(float Pitch, float Yaw, float Roll)
{
	XMMATRIX Rotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(Pitch), XMConvertToRadians(Yaw), XMConvertToRadians(Roll));
	m_TransformPos = Matrix4x4::Multiply(Rotate, m_TransformPos);

	UpdateTransform(NULL);
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

void GameObject::MoveUp(float Distance)
{
	XMFLOAT3 Position = GetPosition();
	XMFLOAT3 Up = GetUp();
	Position = Vector3::Add(Position, Up, Distance);
	GameObject::SetPostion(Position);
}

void GameObject::MoveForward(float Distance)
{
	XMFLOAT3 Position = GetPosition();
	XMFLOAT3 Look = GetLook();
	Position = Vector3::Add(Position, Look, Distance);
	GameObject::SetPostion(Position);

	m_TransformPos._42 = -50.f;
}

void GameObject::MoveRight(float Distance)
{
	XMFLOAT3 Position = GetPosition();
	XMFLOAT3 Right = GetRight();
	Position = Vector3::Add(Position, Right, Distance);
	GameObject::SetPostion(Position);
}

// ------------------------- //
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
			else
				SetShader(0, Shader);

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
			m_Material[nMaterial]->LoadTexutreFromFile(Device, CommandList, MATERIAL_ALBEDO_MAP, 3, ObjMaterial->m_TextureName[0], &(ObjMaterial->m_Texture[0]), InFile, Parent, Shader, ObjMaterial);
		else if (!strcmp(Token, "<SpecularMap>:"))
			m_Material[nMaterial]->LoadTexutreFromFile(Device, CommandList, MATERIAL_SPECULAR_MAP, 4, ObjMaterial->m_TextureName[1], &(ObjMaterial->m_Texture[1]), InFile, Parent, Shader, ObjMaterial);
		else if (!strcmp(Token, "<NormalMap>:"))
			m_Material[nMaterial]->LoadTexutreFromFile(Device, CommandList, MATERIAL_NORMAL_MAP, 5, ObjMaterial->m_TextureName[2], &(ObjMaterial->m_Texture[2]), InFile, Parent, Shader, ObjMaterial);
		else if (!strcmp(Token, "<MetallicMap>:"))
			m_Material[nMaterial]->LoadTexutreFromFile(Device, CommandList, MATERIAL_METALLIC_MAP, 6, ObjMaterial->m_TextureName[3], &(ObjMaterial->m_Texture[3]), InFile, Parent, Shader, ObjMaterial);
		else if (!strcmp(Token, "<EmissionMap>:"))
			m_Material[nMaterial]->LoadTexutreFromFile(Device, CommandList, MATERIAL_EMISSION_MAP, 7, ObjMaterial->m_TextureName[4], &(ObjMaterial->m_Texture[4]), InFile, Parent, Shader, ObjMaterial);
		else if (!strcmp(Token, "<DetailAlbedoMap>:"))
			m_Material[nMaterial]->LoadTexutreFromFile(Device, CommandList, MATERIAL_DETAIL_ALBEDO_MAP, 8, ObjMaterial->m_TextureName[5], &(ObjMaterial->m_Texture[5]), InFile, Parent, Shader, ObjMaterial);
		else if (!strcmp(Token, "<DetailNormalMap>:"))
			m_Material[nMaterial]->LoadTexutreFromFile(Device, CommandList, MATERIAL_DETAIL_NORMAL_MAP, 9, ObjMaterial->m_TextureName[6], &(ObjMaterial->m_Texture[6]), InFile, Parent, Shader, ObjMaterial);
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

	GameObj->CacheSkinningBoneFrame(GameObj);

	if (Animation) {
		GameObj->m_AnimationController = new AnimationController(30);
		GameObj->LoadAnimationFromFile(File);
		GameObj->m_AnimationController->SetAnimationSet(0);
	}

	return GameObj;
}

GameObject *GameObject::FindFrame(char *FrameName)
{
	GameObject *FrameObject = NULL;

	if (!strncmp(m_FrameName, FrameName, strlen(FrameName))) return this;

	if (m_Sibling) if (FrameObject = m_Sibling->FindFrame(FrameName)) return FrameObject;
	if (m_Child) if (FrameObject = m_Child->FindFrame(FrameName)) return FrameObject;

	return NULL;
}

BoundingBox GameObject::GetBodyBounding()
{
	if (m_Mesh)
	{
		if (strstr(m_FrameName, "Body"))
		{
			BoundingBox BodyBound = m_Mesh->GetBounds();
			BodyBound.Transform(BodyBound, XMLoadFloat4x4(&m_WorldPos));
			return BodyBound;
		}
	}
	if (m_Sibling)
		return m_Sibling->GetBodyBounding();
	if (m_Child)
		return m_Child->GetBodyBounding();
}

GameObject *GameObject::IsStageIntersect(BoundingBox BodyBound)
{
	GameObject *TileObject = NULL;
	GameObject *TileSibling = NULL;
	GameObject *TileChild = NULL;
	if (m_Mesh)
	{
		if (strstr(m_FrameName, "Normal") || strstr(m_FrameName, "Doorway"))
		{
			BoundingBox TileBound = m_Mesh->GetBounds();
			TileBound.Transform(TileBound, XMLoadFloat4x4(&m_WorldPos));
			if (TileBound.Intersects(BodyBound)) {
				TileObject = this;
			}
		}
	}
	if (m_Sibling)
		TileSibling = m_Sibling->IsStageIntersect(BodyBound);
	if (m_Child)
		TileChild = m_Child->IsStageIntersect(BodyBound);

	if (TileObject) {
		if (TileSibling) {
			if (Vector3::Distance(TileObject->GetPosition(),BodyBound.Center) > Vector3::Distance(TileSibling->GetPosition(), BodyBound.Center))
				TileObject = TileSibling;
		}
		if (TileChild) {
			if (Vector3::Distance(TileObject->GetPosition(), BodyBound.Center) > Vector3::Distance(TileChild->GetPosition(), BodyBound.Center))
				TileObject = TileChild;
		}
	}
	else {
		if (TileSibling && TileChild) {
			if (Vector3::Distance(TileSibling->GetPosition(), BodyBound.Center) > Vector3::Distance(TileChild->GetPosition(), BodyBound.Center))
				TileObject = TileChild;
			else
				TileObject = TileSibling;
		}
		else if (TileSibling)
			TileObject = TileSibling;
		else if (TileChild)
			TileObject = TileChild;
	}
	
	if (TileObject)
		return TileObject;

	return NULL;
}

int GameObject::CheckMonster(XMFLOAT3 startpos, XMFLOAT3 endpos)
{
	if (m_Mesh) {
		// 몬스터 부위
		BoundingBox MonsterBound = m_Mesh->GetBounds();
		MonsterBound.Transform(MonsterBound, XMLoadFloat4x4(&m_WorldPos));
		if (MonsterBound.Intersects(XMLoadFloat3(&startpos), XMLoadFloat3(&endpos), m_Mesh->m_fDistance) == true) {
			if (strstr(m_FrameName, "Head"))
				return 2;
			else
				return 1;
		}
	}
	if (m_Sibling)
		return m_Sibling->CheckMonster(startpos, endpos);
	if (m_Child)
		return m_Child->CheckMonster(startpos, endpos);
	
	return 0;
}

GameObject *GameObject::CheckTileBound(XMFLOAT3 startpos, XMFLOAT3 endpos, bool IsFloor)
{
	GameObject *TileObject = NULL;
	GameObject *TileSibling = NULL;
	GameObject *TileChild = NULL;
	if (m_Mesh) {
		if (IsFloor) {
			// 바닥 타일
			if (strstr(m_FrameName, "Floor")) {
				BoundingBox TileBound = m_Mesh->GetBounds();
				TileBound.Transform(TileBound, XMLoadFloat4x4(&m_WorldPos));
				if (TileBound.Intersects(XMLoadFloat3(&startpos), XMLoadFloat3(&endpos), m_Mesh->m_fDistance) == true) {
					TileObject = this;
				}
			}
		}
		else {
			// 벽 타일
			if (strstr(m_FrameName, "Normal")) {
				BoundingBox TileBound = m_Mesh->GetBounds();
				TileBound.Transform(TileBound, XMLoadFloat4x4(&m_WorldPos));
				if (TileBound.Intersects(XMLoadFloat3(&startpos), XMLoadFloat3(&endpos), m_Mesh->m_fDistance) == true) {
					TileObject = this;
				}
			}
		}
	}
	if (m_Sibling)
		TileSibling = m_Sibling->CheckTileBound(startpos, endpos, IsFloor);
	if (m_Child)
		TileChild = m_Child->CheckTileBound(startpos, endpos, IsFloor);

	if (TileObject) {
		if (TileSibling) {
			if (TileObject->m_Mesh->m_fDistance > TileSibling->m_Mesh->m_fDistance)
				TileObject = TileSibling;
		}
		if (TileChild) {
			if (TileObject->m_Mesh->m_fDistance > TileChild->m_Mesh->m_fDistance)
				TileObject = TileChild;
		}
	}
	else {
		if (TileSibling && TileChild) {
			if (TileSibling->m_Mesh->m_fDistance > TileChild->m_Mesh->m_fDistance)
				TileObject = TileChild;
			else
				TileObject = TileSibling;
		}
		else if (TileSibling)
			TileObject = TileSibling;
		else if (TileChild)
			TileObject = TileChild;
	}

	if (TileObject)
		return TileObject; 

	return NULL;
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

	return NULL;
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

void GameObject::SetEnable(int nAnimationSet)
{
	if (nAnimationSet > 29  && nAnimationSet < 0) return;
	m_AnimateState = short(nAnimationSet);

	if (m_AnimationController) m_AnimationController->SetAnimationEnable(nAnimationSet);

	if (m_Sibling) m_Sibling->SetEnable(nAnimationSet);
	if (m_Child) m_Child->SetEnable(nAnimationSet);
}

void GameObject::SetAnimateType(int nAnimationSet, int nType)
{
	if (m_AnimationController) m_AnimationController->SetAnimateControlType(nAnimationSet, nType);

	if (m_Sibling) m_Sibling->SetAnimateType(nAnimationSet, nType);
	if (m_Child) m_Child->SetAnimateType(nAnimationSet, nType);
}

void GameObject::SetPlayerAnimateType(int nType)
{
	if (m_AnimationController) m_AnimationController->SetPlayerAnimateType(nType);

	if (m_Sibling) m_Sibling->SetPlayerAnimateType(nType);
	if (m_Child) m_Child->SetPlayerAnimateType(nType);
}

void GameObject::Animate(float ElapsedTime, XMFLOAT4X4 *Parent)
{
	if (m_AnimationController) m_AnimationController->AdvanceTime(ElapsedTime, NULL);

	if (m_Sibling) m_Sibling->Animate(ElapsedTime, Parent);
	if (m_Child) m_Child->Animate(ElapsedTime, &m_WorldPos);
}

void GameObject::Render(ID3D12GraphicsCommandList *CommandList)
{
	OnPrepareRender();
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

		for (int i = 0; i < ObjMesh->GetSkinningBoneNum(); ++i)
			ObjMesh->m_SkinningBoneFrameCache[i] = RootFrame->FindFrame(ObjMesh->m_SkinningBoneName[i]);
	}

	if (m_Sibling) m_Sibling->CacheSkinningBoneFrame(RootFrame);
	if (m_Child) m_Child->CacheSkinningBoneFrame(RootFrame);
}

void GameObject::LoadAnimationFromFile(FILE *InFile)
{
	char Token[64] = { '\0' };

	BYTE nStrLength = 0;
	UINT nRead = 0;

	for (; ;) {
		nRead = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, InFile);
		nRead = (UINT)::fread(Token, sizeof(char), nStrLength, InFile);
		Token[nStrLength] = '\0';

		if (!strcmp(Token, "<AnimationSets>:")) {
			nRead = (UINT)::fread(&m_AnimationController->m_nAnimationSet, sizeof(int), 1, InFile);
			m_AnimationController->m_AnimationSet = new AnimationSet[m_AnimationController->m_nAnimationSet];
		}
		else if (!strcmp(Token, "<FrameNames>:")) {
			nRead = (UINT)::fread(&m_AnimationController->m_nAnimationBoneFrame, sizeof(int), 1, InFile);
			m_AnimationController->m_AnimationBoneFrameCache = new GameObject*[m_AnimationController->m_nAnimationBoneFrame];

			for (int i = 0; i < m_AnimationController->m_nAnimationBoneFrame; ++i) {
				nRead = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, InFile);
				nRead = (UINT)::fread(Token, sizeof(char), nStrLength, InFile);
				Token[nStrLength] = '\0';

				m_AnimationController->m_AnimationBoneFrameCache[i] = FindFrame(Token);
			}
		}
		else if (!strcmp(Token, "<AnimationSet>:")) {
			int nAnimationSet = 0;
			nRead = (UINT)::fread(&nAnimationSet, sizeof(int), 1, InFile);
			AnimationSet *ObjAnimationSet = &m_AnimationController->m_AnimationSet[nAnimationSet];

			nRead = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, InFile);
			nRead = (UINT)::fread(ObjAnimationSet->m_strName, sizeof(char), nStrLength, InFile);
			ObjAnimationSet->m_strName[nStrLength] = '\0';

			nRead = (UINT)::fread(&ObjAnimationSet->m_Length, sizeof(float), 1, InFile);
			nRead = (UINT)::fread(&ObjAnimationSet->m_nFramePerSecond, sizeof(int), 1, InFile);

			nRead = (UINT)::fread(&ObjAnimationSet->m_nKeyFrameTransform, sizeof(int), 1, InFile);
			ObjAnimationSet->m_KeyFrameTransformTime = new float[ObjAnimationSet->m_nKeyFrameTransform];
			ObjAnimationSet->m_KeyFrameTransform = new XMFLOAT4X4*[ObjAnimationSet->m_nKeyFrameTransform];
			for (int i = 0; i < ObjAnimationSet->m_nKeyFrameTransform; ++i)
				ObjAnimationSet->m_KeyFrameTransform[i] = new XMFLOAT4X4[m_AnimationController->m_nAnimationBoneFrame];

			for (int i = 0; i < ObjAnimationSet->m_nKeyFrameTransform; ++i) {
				nRead = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, InFile);
				nRead = (UINT)::fread(Token, sizeof(char), nStrLength, InFile);
				Token[nStrLength] = '\0';

				if (!strcmp(Token, "<Transforms>:")) {
					int nKeyFrame = 0;
					nRead = (UINT)::fread(&nKeyFrame, sizeof(int), 1, InFile);

					nRead = (UINT)::fread(&ObjAnimationSet->m_KeyFrameTransformTime[i], sizeof(float), 1, InFile);
					nRead = (UINT)::fread(ObjAnimationSet->m_KeyFrameTransform[i], sizeof(float), 16 * m_AnimationController->m_nAnimationBoneFrame, InFile);
				}
			}
		}
		else if (!strcmp(Token, "</AnimationSets>"))
			break;
	}
}


// UI
UI::UI(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature, float x, float y, int Type, int SceneNum)
{
	TextureMesh *ObjMesh = new TextureMesh(Device, CommandList, x, y, 0.f, 0.f, 0.f, 0.f, 0, Type);
	SetMesh(ObjMesh);

	Texture *ObjTexture = new Texture(1, RESOURCE_TEXTURE2D, 0);

	switch (Type) {
	case BackGround:
		// Title Background
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/JSTM_Title.dds", 0);
		break;
	case Title:
		// Title Logo
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Main_Title_Logo.dds", 0);
		break;
	case RoomList:
		// Room List
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Select_Room_List.dds", 0);
		break;
	case MakeRoom:
		// Make Room
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Select_Make_Room.dds", 0);
		break;
	case JoinRoom:
		// Join Room
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Select_Join_Room.dds", 0);
		break;
	case Select_Stage:
		// Stage Select
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Stage_Box.dds", 0);
		break;
	case Stage_Left:
		// Stage Left
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Stage_Left.dds", 0);
		break;
	case Stage_Right:
		// Stage Right
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Stage_Right.dds", 0);
		break;
	case Start_Button:
		// Weapon and Skill Select
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/UI_StartButton.dds", 0);
		break;
	case Back_Button:
		//Back Button
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Back_Button.dds", 0);
		break;
	case PlayerInfo:
		// Player Info
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/PlayerBox.dds", 0);
		break;
	case Player_1:
		// Player Info
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Player1.dds", 0);
		break;
	case Player_2:
		// Player Info
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Player2.dds", 0);
		break;
	case MyPlayer:
		// Player Info
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/MyPlayer.dds", 0);
		break;
	case Num_1:
		//Number 1
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Number_1.dds", 0);
		break;
	case Num_2:
		//Number 2
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Number_2.dds", 0);
		break;
	case Num_3:
		//Number 3
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Number_3.dds", 0);
		break;
	case Num_4:
		//Number 4
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Number_4.dds", 0);
		break;
	case UI_PlayerInfo:
		// Charactor Information
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/charinfo.dds", 0);
		break;
	case UI_TrapList:
		// Trap List UI
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/traplist.dds", 0);
		break;
	case UI_SCOPE:
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/Scope.dds", 0);
		break;
	default:
		break;
	}
	UIShader *ObjShader = new UIShader();
	ObjShader->CreateShader(Device, CommandList, GraphicsRootSignature);
	switch (SceneNum) {
	case 0:
		TitleScene::CreateShaderResourceView(Device, CommandList, ObjTexture, 1, false);
		break;

	case 1:
		GameScene::CreateShaderResourceView(Device, CommandList, ObjTexture, 2, false);
		break;
	}
	Material *ObjMaterial = new Material(1);
	ObjMaterial->SetTexture(ObjTexture);
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

void UI::Animate(float ElapsedTime, XMFLOAT4X4 *Parent)
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


// InGame에 사용할 이미지 파일
SkyBox::SkyBox(ID3D12Device *Device, ID3D12GraphicsCommandList *CommandList, ID3D12RootSignature *GraphicsRootSignature, int type)
{
	TextureMesh *ObjMesh = NULL;

	switch (type) {
	case 0: // 정면
		ObjMesh = new TextureMesh(Device, CommandList, 1000.f, 1000.f, 1000.f, 0.f, 0.f, 0.f, 0);
		break;

	case 1: // 후면
		ObjMesh = new TextureMesh(Device, CommandList, 1000.f, 1000.f, -1000.f, 0.f, 0.f, 0.f, 1);
		break;

	case 2: // 좌측
		ObjMesh = new TextureMesh(Device, CommandList, -1000.f, 1000.f, 1000.f, 0.f, 0.f, 0.f, 2);
		break;

	case 3: // 우측
		ObjMesh = new TextureMesh(Device, CommandList, 1000.f, 1000.f, 1000.f, 0.f, 0.f, 0.f, 3);
		break;

	case 4: // 천장
		ObjMesh = new TextureMesh(Device, CommandList, 1000.f, 1000.f, 1000.f, 0.f, 0.f, 0.f, 4);
		break;

	default:
		break;
	}
	SetMesh(ObjMesh);

	Texture *ObjTexture = new Texture(1, RESOURCE_TEXTURE2D, 0);
	// 함정 윗부분의 타입에 맞는 이미지를 로드
	switch (type)
	{
	case 0:
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/skybox_front.dds", 0);
		break;

	case 1:
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/skybox_back.dds", 0);
		break;

	case 2:
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/skybox_right.dds", 0);
		break;

	case 3:
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/skybox_left.dds", 0);
		break;

	case 4:
		ObjTexture->LoadTextureFromFile(Device, CommandList, L"Image/skybox_up.dds", 0);
		break;

	default:
		break;
	}

	SkyBoxShader *ObjShader = new SkyBoxShader();
	ObjShader->CreateShader(Device, CommandList, GraphicsRootSignature);
	GameScene::CreateShaderResourceView(Device, CommandList, ObjTexture, 2, false);

	Material *ObjMaterial = new Material(1);
	ObjMaterial->SetTexture(ObjTexture);
	m_nMaterial = 1;

	m_Material = new Material*();
	m_Material[0] = NULL;
	SetMaterial(0, ObjMaterial);
	SetCbvGPUDescriptorHandle(ObjShader->GetGPUCbvDescriptorStartHandle());
	SetShader(0, ObjShader);

	ObjMaterial = NULL;
}

SkyBox::~SkyBox()
{

}

void SkyBox::Animate(XMFLOAT3 PlayerPosition)
{
	// SkyBox는 플레이어를 쫓아감
	SetPostion(PlayerPosition);
}

void SkyBox::Render(ID3D12GraphicsCommandList *CommandList)
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


void Trap::Animate(GameObject *Player, float ElapsedTime, XMFLOAT4X4 *Parent)
{
	// 함정의 위치를 설정하는 함수
	//if (IsBuildTrap) {
	//	SetPostion(Vector3::Add(Player->GetPosition(), Vector3::ScalarProduct(Player->GetLook(), 100)));
	//	m_TransformPos._42 = -50.f;
	//
	//	UpdateTransform(NULL);
	//}

	// 함정 활성화
	if (IsActive) {
		if (m_AnimationController) m_AnimationController->AdvanceTime(ElapsedTime, NULL);

		if (m_Sibling) m_Sibling->Animate(ElapsedTime, Parent);
		if (m_Child) m_Child->Animate(ElapsedTime, &m_WorldPos);
	}
}

// Monster
void Monster::SetDirection(XMFLOAT3 Position)
{
	// 플레이어를 바라보도록 설정
	Position.y = -50.f;

	XMFLOAT3 Look = Vector3::Subtract(GetPosition(), Position);
	Look = Vector3::Normalize(Look);
	m_TransformPos._31 = -Look.x;
	m_TransformPos._32 = -Look.y;
	m_TransformPos._33 = -Look.z;

	m_TransformPos._21 = 0.f;
	m_TransformPos._22 = 1.f;
	m_TransformPos._23 = 0.f;

	XMFLOAT3 Right = Vector3::CrossProduct(GetUp(), Look, true);
	m_TransformPos._11 = -Right.x;
	m_TransformPos._12 = -Right.y;
	m_TransformPos._13 = -Right.z;
}

void Monster::SetLookDirection(XMFLOAT3 Look)
{
	Look = Vector3::Normalize(Look);
	m_TransformPos._31 = Look.x;
	m_TransformPos._32 = Look.y;
	m_TransformPos._33 = Look.z;

	m_TransformPos._21 = 0.f;
	m_TransformPos._22 = 1.f;
	m_TransformPos._23 = 0.f;

	XMFLOAT3 Right = Vector3::CrossProduct(GetUp(), Look, true);
	m_TransformPos._11 = -Right.x;
	m_TransformPos._12 = -Right.y;
	m_TransformPos._13 = -Right.z;
}
void Monster::SetinterPolation(XMFLOAT3 DesLook)
{
	XMFLOAT3 NowLook = StartLook;
	float alpha = XMScalarACos(Vector3::DotProduct(NowLook, DesLook));
	//cout << nCheckPoint << " 각도: " << alpha * nInporation << endl;
	if (!XMVector3Equal(XMLoadFloat3(&NowLook), XMLoadFloat3(&DesLook))) {
		XMMATRIX Rotate = XMMatrixRotationAxis(XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&NowLook), XMLoadFloat3(&DesLook))), alpha*nInporation);
		NowLook = Vector3::TransformNormal(NowLook, Rotate);
	}
	else
		return;
	//SetRotate(0.0f, alpha * (1.0f - nInporation), 0.0f);
	//XMMATRIX Rotate = XMMatrixRotationRollPitchYaw(0.0f, XMConvertToRadians(alpha * nInporation), 0.0f);
	SetLookDirection(NowLook);
}

void Monster::SetLine(float ElapsedTime)
{
	XMFLOAT3 pos = GetPosition();
	if (880.0f < pos.x && pos.x <= 2200.0f) {
		if (nCheckPoint != 1) { nCheckPoint = 1; nInporation = 0.0f; StartLook = GetLook(); }
		if (nInporation < 1.0f) {
			nInporation += 0.02f;
			SetinterPolation(XMFLOAT3(-1.0f, 0.0f, 0.0f));
		}
		else {
			SetLookDirection(XMFLOAT3(-1.0f, 0.0f, 0.0f));
		}
	}
	else if (360.0f < pos.x && pos.x <= 880.0f) {
		if (nCheckPoint != 2) { nCheckPoint = 2; nInporation = 0.0f; StartLook = GetLook(); }
		if (pos.z < 280.0f) {
			if (nInporation < 1.0f) {
				nInporation += 0.02f;
				XMFLOAT3 Look = Vector3::Subtract(XMFLOAT3(-400.0f, -50.0f, 800.0f), pos);
				SetinterPolation(Vector3::Normalize(Look));
			}
		}
		else {
			if (nInporation < 1.0f) {
				nInporation += 0.02f;
				XMFLOAT3 Look = Vector3::Subtract(XMFLOAT3(-400.0f, -50.0f, -500.0f), pos);
				SetinterPolation(Vector3::Normalize(Look));
			}
		}
	}
	else {
		if (nCheckPoint != 3) { nCheckPoint = 3; nInporation = 0.0f; StartLook = GetLook(); }
		if (nInporation < 1.0f) {
			nInporation += 0.02f;
			SetinterPolation(XMFLOAT3(-1.0f, 0.0f, 0.0f));
		}
	}
}