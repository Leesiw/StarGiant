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
	uint					gnTexturesMask : packoffset(c8);
};

cbuffer cbPlusInfo : register(b9)//b2
{
	matrix		gmtxTexture : packoffset(c0);
	//float		gfCurrentTime : packoffset(c4); //테스트용

};

#include "Light.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#define _WITH_VERTEX_LIGHTING

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
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

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
	return(lerp(cColor, cIllumination, 0.5f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
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
	int4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	//output.positionW = float3(0.0f, 0.0f, 0.0f);
	//output.normalW = float3(0.0f, 0.0f, 0.0f);
	//output.tangentW = float3(0.0f, 0.0f, 0.0f);
	//output.bitangentW = float3(0.0f, 0.0f, 0.0f);
	//matrix mtxVertexToBoneWorld;
	//for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	//{
	//	mtxVertexToBoneWorld = mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
	//	output.positionW += input.weights[i] * mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
	//	output.normalW += input.weights[i] * mul(input.normal, (float3x3)mtxVertexToBoneWorld);
	//	output.tangentW += input.weights[i] * mul(input.tangent, (float3x3)mtxVertexToBoneWorld);
	//	output.bitangentW += input.weights[i] * mul(input.bitangent, (float3x3)mtxVertexToBoneWorld);
	//}
	float4x4 mtxVertexToBoneWorld = (float4x4)0.0f;
	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	{
//		mtxVertexToBoneWorld += input.weights[i] * gpmtxBoneTransforms[input.indices[i]];
		mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
	}
	output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
	output.normalW = mul(input.normal, (float3x3)mtxVertexToBoneWorld).xyz;
	output.tangentW = mul(input.tangent, (float3x3)mtxVertexToBoneWorld).xyz;
	output.bitangentW = mul(input.bitangent, (float3x3)mtxVertexToBoneWorld).xyz;

//	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTerrainBaseTexture : register(t1);
Texture2D gtxtTerrainDetailTexture : register(t2);

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
	float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);
//	float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
	float4 cColor = input.color * saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionL = input.position;

	return(output);
}

TextureCube gtxtSkyCubeTexture : register(t13);
SamplerState gssClamp : register(s1);

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_UI_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_UI_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

VS_UI_OUTPUT VS_UI(VS_UI_INPUT input)
{
	VS_UI_OUTPUT output;

output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
output.uv = input.uv;

return(output);
}

Texture2D gtxtUITexture : register(t14);

float4 PS_UI(VS_UI_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtUITexture.Sample(gssWrap, input.uv);
	return (cColor);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct VS_SPRITE_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;

};

struct VS_SPRITE_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};


//스프라이트 VS
VS_SPRITE_OUTPUT VS_SPRITE(VS_SPRITE_INPUT input)
{
	VS_SPRITE_OUTPUT output;
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = mul(float3(input.uv, 1.0f), (float3x3)(gmtxTexture)).xy;
	//input.position.y += test.y * 10000;
	//output.uv = input.uv;
	return(output);
}
Texture2D gtxtSPRITETexture : register(t17);

float4 PS_SPRITE(VS_SPRITE_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSPRITETexture.Sample(gssWrap, input.uv);
	return (cColor);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VS_GOD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_GOD_OUTPUT
{
	float3	positionL : POSITION;
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
	//float4 ChannelMask :COLOR0;
};

VS_GOD_OUTPUT VS_GODMain(VS_GOD_INPUT input)
{
	/*VS_UI_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

 float4					m_cAmbient;
float4					m_cDiffuse;
float4					m_cSpecular; //a = power
float4					m_cEmissive;
	return(output);*/
	VS_GOD_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionL = input.position;
	output.uv = input.uv;

	return(output);

}

Texture2D gtxtGODTexture1 : register(t15);
Texture2D gtxtGODTexture2 : register(t16);


float4 PS_GODMain(VS_GOD_OUTPUT input) : SV_TARGET
{
	/*float compositeNoise = 0.015f;
	float shadow = 1.0f;
	float4 cookie = (1.0f, 1.0f, 1.0f, 1.0f);

	float shadowMapDepth;
	float4 output;

	if (bCookie) {
		cookie = tex2Dproj(CookieSampler, tcProj);
	}
	if (bScrollingNoise) {
		float4 noise1 = tex2Dproj(ScrollingNosieSampler, tcProjScro111);
		float4 noise2 = tex2Dproj(ScrollingNosieSampler, tcProjScro112);

		compositeNoise = noise1.r * noise2.g * 0.05f;
	}

	shadowMapDepth = tex2Dproj(ShadowMapSampler, tcProj);

	if (bShadowMapping) {
		if (IsPos_depth.w < shadowMapDepth)
			shadow = 1.0f;
		else
			shadow = 0.0f;
	}
*/

	float compositeNoise = 0.15f;

	float4 noise1 = gtxtGODTexture1.Sample(gssWrap, input.uv);
	float4 noise2 = gtxtGODTexture2.Sample(gssWrap, input.uv);
	compositeNoise = lerp(noise1, noise2, 0.5f);
	/*output.rgb = compositeNoise * cookie.rgb * lightColor * scale * atten * shadow * ChannelMask;
		*/

	float4 cookie = float4(1.0f, 1.0f, 1.0f, 1.0f); // 조명 빌보드 텍스처색 (그려지는 조명색)
	float4 lightColor = float4(0.85f, 0.98f, 1.0f, 1.0f); //진짜 조명색 
	//float scale = 9.0f / fFractionOfMaxShells;
	//float atten = 0.25f + 20000.0f / dot(IsPos_depth.xyz, IsPos_Depthxyz);

	float4 cColor = compositeNoise * float4(cookie.r, cookie.g, cookie.b, 0.0f) * lightColor;

	cColor.a = saturate(dot(float3(cColor.a, cColor.g, cColor.b), float3(1.0f, 1.0f, 1.0f)));
	if (cColor.a > 0.8) cColor.rgb *= 1.4f + (cColor.a - 0.8);
	else if (cColor.a >0.6) cColor.rgb *= 1.3f + (cColor.a - 0.6);
	else if (cColor.a >0.4)cColor.rgb *= 1.1f + (cColor.a - 0.4);
	else cColor.rgb *= 1.f + (cColor.a);

	return(cColor);

}


//// 두 번째 버텍스 쉐이더
//struct VS_Line_OUTPUT {
//	float4 position : SV_POSITION; // 정점 좌표
//	float2 uv : TEXCOORD0; // 텍스처 좌표
//};
//
//VS_Line_OUTPUT VS_Line(float4 input) {
//	VS_Line_OUTPUT output;
//	output.position = input.position;
//	output.uv = output.position.xy;
//	return output;
//}
//
//// 두 번째 픽셀 쉐이더
//struct PS_Line_INPUT {
//	float4 position : SV_POSITION; // 정점 좌표
//	float2 uv : TEXCOORD0; // 텍스처 좌표
//};
//
//float4 PS_Line(PS_Line_INPUT input) : SV_TARGET1{
//	if (input.uv.y >= 0.5) { // 픽셀 좌표가 윗쪽 반절에 위치한 경우
//		return float4(1.0, 0.0, 0.0, 1.0); // 빨간색으로 설정
//	}
//	else { // 픽셀 좌표가 아랫쪽 반절에 위치한 경우
//		return float4(0.0, 1.0, 0.0, 1.0); // 초록색으로 설정
//	}
//}
//
//
//float4 Combine(float4 color1 : SV_TARGET, float4 color2 : SV_TARGET1) : SV_TARGET
//{
//	// 두 개의 픽셀 색상 값을 합쳐서 출력
//	return color1 + color2;
//}

/*
struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
};

VS_OUTPUT VSMain(float4 position : POSITION, float2 texCoord : TEXCOORD0)
{
	VS_OUTPUT output;
	output.Position = position;
	output.TexCoord = texCoord;
	return output;
}

float4 PSMain(float2 texCoord : TEXCOORD0) : SV_TARGET
{
	// 텍스처 샘플링해서 화면에 그리기
	float4 color = tex.Sample(sampler, texCoord);
	return color;
}

float4 PSMain2(float2 texCoord : TEXCOORD0) : SV_TARGET1
{
	// 투명하지 않은 픽셀의 위치 구하기
	float4 color = tex2.Sample(sampler2, texCoord);
	if (color.a > 0.5)
	{
		return float4(texCoord, 0.0, 1.0);
	}
	else
	{
		return float4(0.0, 0.0, 0.0, 0.0);
	}
}

float4 Combine(float4 color1 : SV_TARGET, float4 color2 : SV_TARGET1) : SV_TARGET
{
	// 두 개의 픽셀 색상 값을 합쳐서 출력
	return color1 + color2;
}*/