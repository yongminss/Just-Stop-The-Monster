struct MATERIAL
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
};

cbuffer cbCameraInfo : register(b1)
{
	matrix					gmtxView : packoffset(c0);
	matrix					gmtxProjection : packoffset(c4);
	float3					gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix					gmtxGameObject : packoffset(c0);
	MATERIAL				gMaterial : packoffset(c4);
    uint                    gnTexturesMask : packoffset(c8.x);
	uint					gbRed : packoffset(c8.y);
};

#include "Light.hlsl"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define _WITH_VERTEX_LIGHTING

struct VS_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
#ifdef _WITH_VERTEX_LIGHTING
	float4 color : COLOR;
#endif
};

VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
#ifdef _WITH_VERTEX_LIGHTING
	output.normalW = normalize(output.normalW);
	output.color = Lighting(output.positionW, output.normalW);
#endif
	return(output);
}

float4 PSLighting(VS_LIGHTING_OUTPUT input) : SV_TARGET
{
#ifdef _WITH_VERTEX_LIGHTING
	return(input.color);
#else
	input.normalW = normalize(input.normalW);
	float4 color = Lighting(input.positionW, input.normalW);

	return(color);
#endif
}

Texture2D gtxtTexture : register(t0);
SamplerState gSamplerState : register(s0);

struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : UV;
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : UV;
};


// VS에 넣어줄 위치와 UV 좌표 //
// 1. UI
VS_TEXTURED_OUTPUT VSUserInterface(VS_TEXTURED_INPUT input) 
{
    VS_TEXTURED_OUTPUT output;
    output.position = float4(input.position, 1.f);
    output.uv = input.uv;
    
    return output;
}
// 2. 함정
VS_TEXTURED_OUTPUT VSTrap(VS_TEXTURED_INPUT input)
{
    VS_TEXTURED_OUTPUT output;
    output.position = mul(mul(mul(float4(input.position, 1.f), gmtxGameObject), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return output;
}

// PS에 넣어줄 색상 정보 //
// 1. dds 파일의 이미지 그대로 출력
float4 PSPictureColor(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
    float4 Color = gtxtTexture.Sample(gSamplerState, input.uv);

    //clip(Color.a - 0.f);
    
    return Color;
}

// 3D 오브젝트에 사용할 쉐이더 코드
#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);

SamplerState gssWrap : register(s0);

struct VS_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct VS_INSTANCING_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    
    float4x4 Transform : WORLDMATRIX;
};

struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;

    output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
    output.normalW = mul(input.normal, (float3x3) gmtxGameObject);
    output.tangentW = mul(input.tangent, (float3x3) gmtxGameObject);
    output.bitangentW = mul(input.bitangent, (float3x3) gmtxGameObject);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);    
    output.uv = input.uv;

    return (output);
}

VS_STANDARD_OUTPUT VSInstancingStandard(VS_INSTANCING_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;

    output.positionW = mul(float4(input.position, 1.0f), input.Transform).xyz;
    output.normalW = input.normal; //mul(input.normal, (float3x3) input.Transform);
    output.tangentW = input.tangent; //mul(input.tangent, (float3x3) input.Transform);
    output.bitangentW = input.bitangent; //mul(input.bitangent, (float3x3) input.Transform);
    
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return (output);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
        cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
    float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_SPECULAR_MAP)
        cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
    float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
        cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
    float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_METALLIC_MAP)
        cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
    float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_EMISSION_MAP)
        cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

    float3 normalW;
    float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
        float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
        normalW = normalize(mul(vNormal, TBN));
    }
    else
    {
        normalW = normalize(input.normalW);
    }
    float4 cIllumination = Lighting(input.positionW, normalW);
    
	if (gbRed & 0x01) {
		cColor.x = 1.0f;
		cColor.w = 0.3f;
	}
    return (lerp(cColor, cIllumination, 0.5f));

}

#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			128

cbuffer cbBoneOffsets : register(b7)
{
    float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b8)
{
    float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    uint4 indices : BONEINDEX;
    float4 weights : BONEWEIGHT;
};

struct VS_INSTANCING_SKINNED_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    uint4 indices : BONEINDEX;
    float4 weights : BONEWEIGHT;
    
    float4x4 Transform : WORLDMATRIX;
};

VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;

    output.positionW = float3(0.0f, 0.0f, 0.0f);
    output.normalW = float3(0.0f, 0.0f, 0.0f);
    output.tangentW = float3(0.0f, 0.0f, 0.0f);
    output.bitangentW = float3(0.0f, 0.0f, 0.0f);
    matrix mtxVertexToBoneWorld;
    
    for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
    {
        mtxVertexToBoneWorld = mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
        output.positionW += input.weights[i] * mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
        output.normalW += input.weights[i] * mul(input.normal, (float3x3) mtxVertexToBoneWorld);
        output.tangentW += input.weights[i] * mul(input.tangent, (float3x3) mtxVertexToBoneWorld);
        output.bitangentW += input.weights[i] * mul(input.bitangent, (float3x3) mtxVertexToBoneWorld);
    }

    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return (output);
}

//VS_STANDARD_OUTPUT SC_VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
//{
//    VS_STANDARD_OUTPUT output;

//    float4x4 mtxVertexToBoneWorld = (float4x4) 0.0f;
//    for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
//    {
//        mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gsbBoneTransforms[(input.instanceID * SKINNED_ANIMATION_BONES) + input.indices[i]]);
//    }
//    output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
//    output.normalW = mul(input.normal, (float3x3) mtxVertexToBoneWorld).xyz;
//    output.tangentW = mul(input.tangent, (float3x3) mtxVertexToBoneWorld).xyz;
//    output.bitangentW = mul(input.bitangent, (float3x3) mtxVertexToBoneWorld).xyz;


//    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
//    output.uv = input.uv;

//    return (output);
//}

VS_STANDARD_OUTPUT VSSkinnedAnimation_InstancingStandard(VS_INSTANCING_SKINNED_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;

    output.positionW = float3(0.0f, 0.0f, 0.0f);
    output.normalW = float3(0.0f, 0.0f, 0.0f);
    output.tangentW = float3(0.0f, 0.0f, 0.0f);
    output.bitangentW = float3(0.0f, 0.0f, 0.0f);
    
    matrix mtxVertexToBoneWorld = (float4x4) 0.f;
    
    for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
    {
        //mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[(input.instanceID * SKINNED_ANIMATION_BONES) + input.indices[i]]);
        mtxVertexToBoneWorld = input.Transform; //mul(input.Transform, mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]));
    }    
    output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
    output.normalW = mul(input.normal, (float3x3) mtxVertexToBoneWorld).xyz;
    output.tangentW = mul(input.tangent, (float3x3) mtxVertexToBoneWorld).xyz;
    output.bitangentW = mul(input.bitangent, (float3x3) mtxVertexToBoneWorld).xyz;
 
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return (output);
}