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
	float		gfCurrentTime : packoffset(c4); //테스트용

};

struct CB_TOOBJECTSPACE
{
	matrix		mtxToTexture;
	float4		f4Position;
};

cbuffer cbToLightSpace : register(b6) // ?    
{
	CB_TOOBJECTSPACE gcbToLightSpaces[1];
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
	output.uv = mul(float3(input.uv, 1.0f), (float3x3)(gmtxTexture)).xy;//gmtxTexture
	 // 디버그 스트링 출력

	//output.uv = input.uv;
	return(output);
}
Texture2D gtxtSPRITETexture : register(t17);

float4 PS_SPRITE(VS_SPRITE_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSPRITETexture.Sample(gssWrap, input.uv);
	//cColor = float4(1.0,1.0,1.0,1.0);
	return (cColor);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



VS_SPRITE_OUTPUT VS_SPRITE2(VS_SPRITE_INPUT input)
{
	VS_SPRITE_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = mul(float3(input.uv, 1.0f), (float3x3)(gmtxTexture)).xy;//gmtxTexture
	 // 디버그 스트링 출력

	//output.uv = input.uv;
	return(output);
}

float4 PS_SPRITE2(VS_SPRITE_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSPRITETexture.Sample(gssWrap, input.uv);
	//cColor = float4(1.0,1.0,1.0,1.0);
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


	// 입력 좌표값을 변화시켜 흔들린 선형 형태를 만듭니다.
	float3 newPos = input.position;

	float Distance = distance(newPos, float3(gmtxTexture._11, gmtxTexture._12, gmtxTexture._13));
	float DepthLayer = saturate(Distance / gmtxTexture._23);

	float depthScale = 1.0f + DepthLayer * 0.3;

	newPos.x += 20.0f * DepthLayer * sin(Distance * 40.0f * 3.1415f);
	newPos.z += 20.0f * DepthLayer * cos(Distance * 40.0f * 3.1415f);

	

	newPos *= depthScale;

	// 변화된 좌표값을 모델-뷰-프로젝션 행렬로 변환하여 출력합니다.
	output.position = mul(mul(mul(float4(newPos, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionL = newPos;
	output.uv = input.uv;

	return output;

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


	//임시 거리값 
	float3 Center = float3(gmtxTexture._11, gmtxTexture._12, gmtxTexture._13);
	float3 Length = float3(gmtxTexture._21, gmtxTexture._22, gmtxTexture._23); // Weight, Height, Depth
	float Percent = (input.positionL.z - Center.z) / Length.z;// 0~1 을 가장. 
	float LineState =0.5f;
	float sinvalue = LineState * sin(Percent * 3.1415 * 2);
	
	if(abs(input.positionL.z) % 100 <10.0f)
	{

	
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

	float Distance = distance(input.positionL, Center);
	float DepthLayer = saturate(Distance / Length.z);

	Distance = distance(input.positionL, float3(Center.x, 0.0f, Center.z));
	float normalizedWidth = saturate(Distance / Length.x);
	float normalizedHeight = saturate(Distance / Length.y);
	float2 LengthLayer = float2(normalizedWidth, normalizedHeight);

	float alphaChannel = 1.0f - saturate(DepthLayer * LengthLayer.x * LengthLayer.y);

	//return float4(1.0f, 1.0f, 1.0f, 1.0* DepthLayer);
	return float4(1.0f* DepthLayer, 1.0f* LengthLayer.x, 1.0f* LengthLayer.y, alphaChannel* Percent);
	}
	else {
		return float4(0, 0, 0, 0);
	}
	//return float4(0, 0, 0, 0);
}







//
//Texture2D<float4> noiseTexture1; // 첫 번째 noise 텍스처
//Texture2D<float4> noiseTexture2; // 두 번째 noise 텍스처
//Texture2D<float> depthBufferTexture; // 깊이 버퍼 텍스처
//
//SamplerState samplerLinear
//{
//	Filter = MIN_MAG_LINEAR_MIP_POINT;
//	AddressU = Wrap;
//	AddressV = Wrap;
//};
//
//float4 PSMain(float4 position : SV_POSITION) : SV_TARGET
//{
//	float2 texCoord = position.xy / float2(800, 600); // 화면 크기에 맞게 나눔
//
//	// 텍스처에서 위치 정보 읽어오기
//	float4 location = noiseTexture1.SampleLevel(samplerLinear, texCoord, 0);
//	float4 location2 = noiseTexture2.SampleLevel(samplerLinear, texCoord, 0);
//
//	// 텍스처 좌표를 기준으로 광선의 색 계산
//	float intensity = sin(location.x) + sin(location.y); // Sin 곡선에 따른 강도 계산
//	float3 color = float3(intensity, intensity, intensity); // 흑백으로 표현할 경우
//
//	// 깊이 버퍼에서 해당 픽셀의 깊이 값을 읽어옴
//	float depth = depthBufferTexture.SampleLevel(samplerLinear, texCoord, 0);
//
//	// 그림자 효과 계산
//	float shadowIntensity = 1.0f - depth; // 깊이에 비례한 그림자 강도 계산
//	color *= shadowIntensity; // 색상에 그림자 강도를 곱하여 그림자 효과 적용
//
//	return float4(color, 1.0f);
//}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_PARTICLE_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float4 color : COLOR;
};

struct VS_PARTICLE_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
	float4 color : COLOR;
};

VS_PARTICLE_OUTPUT VS_PARTICLE(VS_PARTICLE_INPUT input)
{
	VS_PARTICLE_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	// 픽셀 쉐이더의 입자 색상
	output.color = input.color;

	return(output);
}

Texture2D gtxtPARTICLETexture : register(t19);

float4 PS_PARTICLE(VS_PARTICLE_OUTPUT input) : SV_TARGET
{
	float4 textureColor;
	float4 finalColor;

	textureColor = gtxtPARTICLETexture.Sample(gssWrap, input.uv);
	finalColor = textureColor * input.color;


	//finalColor = float4(0.0, 0.0, 1.0, 1.0);
	return (finalColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
SamplerState gssFIRE1 : register(s4);
SamplerState gssFIRE2 : register(s5);
cbuffer NoiseBuffer
{
	float frameTime = 0;
	float3 scrollSpeeds = { 1.3f, 2.1f, 2.3f };
	float3 scales = { 1.0f, 2.0f, 3.0f };
	float padding = 0;
};

cbuffer DistortionBuffer
{
	float2 distortion1 = { 0.1f, 0.2f };
	float2 distortion2 = { 0.1f, 0.3f };
	float2 distortion3 = { 0.1f, 0.1f };
	float distortionScale = 0.8f;
	float distortionBias = 0.5f;
};


Texture2D gtxtFIRETexture : register(t20);
Texture2D gtxtAlphaTexture : register(t21);
Texture2D gtxtNoiseTexture : register(t22);

struct VS_FIRE_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD0;
};

struct VS_FIRE_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
	float2 uv2 : TEXCOORD2;
	float2 uv3 : TEXCOORD3;
};

VS_FIRE_OUTPUT VS_FIRE(VS_FIRE_INPUT input)
{
	VS_FIRE_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;


	output.uv1 = (input.uv * scales.x);
	output.uv1.y = output.uv1.y + (gfCurrentTime * scrollSpeeds.x);

	output.uv2 = (input.uv * scales.y);
	output.uv2.y = output.uv2.y + (gfCurrentTime * scrollSpeeds.y);

	output.uv3 = (input.uv * scales.z);
	output.uv3.y = output.uv3.y + (gfCurrentTime * scrollSpeeds.z);

	return output;
}

float4 PS_FIRE(VS_FIRE_OUTPUT input) : SV_TARGET
{
	float4 noise1;
	float4 noise2;
	float4 noise3;
	float4 finalNoise;
	float perturb;
	float2 noiseCoords;
	float4 fireColor;
	float4 alphaColor;

	noise1 = gtxtNoiseTexture.Sample(gssFIRE1, input.uv1);
	noise2 = gtxtNoiseTexture.Sample(gssFIRE1, input.uv2);
	noise3 = gtxtNoiseTexture.Sample(gssFIRE1, input.uv3);

	noise1 = (noise1 - 0.5f) * 2.0f;
	noise2 = (noise2 - 0.5f) * 2.0f;
	noise3 = (noise3 - 0.5f) * 2.0f;

	noise1.xy = noise1.xy * distortion1.xy;
	noise2.xy = noise2.xy * distortion2.xy;
	noise3.xy = noise3.xy * distortion3.xy;

	finalNoise = noise1 + noise2 + noise3;

	perturb = ((1.0f - input.uv.y) * distortionScale) + distortionBias;

	noiseCoords.xy = (finalNoise.xy * perturb) + input.uv.xy;



	fireColor = gtxtFIRETexture.Sample(gssFIRE2, noiseCoords.xy);


	// 불의 투명도
	alphaColor = gtxtAlphaTexture.Sample(gssFIRE2, noiseCoords.xy);

	fireColor.a = alphaColor;

	return fireColor;
	//return float4(1.0f,1.0f,0.0f,1.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

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
};

VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

	return(output);
}

float4 PSLighting(VS_LIGHTING_OUTPUT input) : SV_TARGET //      
{
	input.normalW = normalize(input.normalW);

	return(float4(input.normalW * 0.5f + 0.5f, 1.0f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct PS_DEPTH_OUTPUT
{
	float fzPosition : SV_Target;
	float fDepth : SV_Depth;
};

PS_DEPTH_OUTPUT PSDepthWriteShader(VS_LIGHTING_OUTPUT input)
{
	PS_DEPTH_OUTPUT output;

	output.fzPosition = input.position.z;
	output.fDepth = input.position.z;

	return(output);
}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SHADOW_MAP_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;

	float4 uvs[1] : TEXCOORD0;
};

VS_SHADOW_MAP_OUTPUT VSShadowMapShadow(VS_LIGHTING_INPUT input)
{
	VS_SHADOW_MAP_OUTPUT output = (VS_SHADOW_MAP_OUTPUT)0;

	float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
	output.positionW = positionW.xyz;
	output.position = mul(mul(positionW, gmtxView), gmtxProjection);
	output.normalW = mul(float4(input.normal, 0.0f), gmtxGameObject).xyz;

	for (int i = 0; i < 1; i++)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f) output.uvs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTexture);
	}

	return(output);
}

float4 PSShadowMapShadow(VS_SHADOW_MAP_OUTPUT input) : SV_TARGET
{
	float4 cIllumination = LightingLight(input.positionW, normalize(input.normalW), true, input.uvs);

	return(cIllumination);
}
//
//float4 PSShadowMapShadow(VS_SHADOW_MAP_OUTPUT input) : SV_TARGET
//{
//	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
//	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uvs[0].xy);
//	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
//	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uvs[0].xy);
//	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
//	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uvs[0].xy);
//	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
//	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uvs[0].xy);
//	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
//	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uvs[0].xy);
//
//	float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
//
//	float4 cIllumination = shadowLighting(input.positionW, normalize(input.normalW), true, input.uvs);
//
//	return(lerp(cColor, cIllumination, 0.5f));
//}

///////////////////////////////////////////////////////////////////////////////

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextureToViewport(uint nVertexID : SV_VertexID)
{
	VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;

	if (nVertexID == 0) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	if (nVertexID == 1) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	if (nVertexID == 2) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	if (nVertexID == 3) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	if (nVertexID == 4) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	if (nVertexID == 5) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return(output);
}

float4 GetColorFromDepth(float fDepth)
{
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	if (fDepth < 0.00625f) cColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
	else if (fDepth < 0.0125f) cColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
	else if (fDepth < 0.025f) cColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth < 0.05f) cColor = float4(1.0f, 1.0f, 0.0f, 1.0f);
	else if (fDepth < 0.075f) cColor = float4(0.0f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.1f) cColor = float4(1.0f, 0.5f, 0.5f, 1.0f);
	else if (fDepth < 0.4f) cColor = float4(0.5f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.6f) cColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth < 0.8f) cColor = float4(0.5f, 0.5f, 1.0f, 1.0f);
	else if (fDepth < 0.9f) cColor = float4(0.5f, 1.0f, 0.5f, 1.0f);
	else if (fDepth < 0.95f) cColor = float4(0.5f, 0.0f, 0.5f, 1.0f);
	else if (fDepth < 0.99f) cColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.999f) cColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth == 1.0f) cColor = float4(0.5f, 0.5f, 0.5f, 1.0f);
	else if (fDepth > 1.0f) cColor = float4(0.0f, 0.0f, 0.5f, 1.0f);
	else cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	return(cColor);
}

SamplerState gssBorder : register(s3);

float4 PSTextureToViewport(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float fDepthFromLight0 = gtxtDepthTextures[1].SampleLevel(gssBorder, input.uv, 0).r;

	return((float4)(fDepthFromLight0 * 0.8f));
}
