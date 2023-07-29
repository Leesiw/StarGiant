#pragma once
#include "Shader.h"

struct TOOBJECTSPACEINFO;
struct TOLIGHTSPACES;



class CBaseShader : public CShader
{
public:
	CBaseShader(CPlayer** pPlayer, LIGHT* pLights);
	virtual ~CBaseShader();

public:
	CPlayer** m_ppRenderObjects = 0;
	LIGHT							* m_pLights = NULL;
	CTexture						* m_pRenderTexture = NULL;

	TOLIGHTSPACES					* m_pToLightSpaces;
	ID3D12Resource					* m_pd3dcbToLightSpaces = NULL;
	TOLIGHTSPACES* m_pcbMappedToLightSpaces = NULL;
	
	XMMATRIX						m_xmProjectionToTexture;
	CCamera* m_ppRenderCamera;
	ID3D12Resource* m_pd3dRenderBuffer = NULL;

	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dRtvCPUDescriptorHandle;

	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap = NULL;	
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvDescriptorCPUHandle;

public:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	CTexture* GetDepthTexture() { return(m_pRenderTexture); }
	ID3D12Resource* GetDepthTextureResource(UINT nIndex) { return(m_pRenderTexture->GetTexture(nIndex)); }

	virtual void ReleaseObjects();

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
};

class CSceneRenderShader : public CBaseShader
{
public:
	CSceneRenderShader(CPlayer** pPlayer, LIGHT* pLights);
	virtual ~CSceneRenderShader();

	void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat);

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext = NULL);
	
	void PrepareShadowMap(ID3D12GraphicsCommandList* pd3dCommandList, CGameObject** Map, CPlayer** Player);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, CGameObject** Map, CPlayer** Player);

};

/*class CDepthRenderShader : public CBaseShader
{
public:
	CDepthRenderShader(CGameObject** pObjects, LIGHT* pLights);
	virtual ~CDepthRenderShader();

	void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat);

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext = NULL);

	void PrepareShadowMap(ID3D12GraphicsCommandList* pd3dCommandList, CGameObject** Map, CPlayer** Player);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, CGameObject** Map, CPlayer** Player);

protected:

};*/
//
class CSceneMapShader : public CShader
{
public:
	CSceneMapShader(CPlayer** pPlayer);
	virtual ~CSceneMapShader();

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed) { }
	virtual void ReleaseObjects();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, CGameObject** Map, CPlayer** Player);
	CTexture* GetDepthTexture() { return(m_pRenderTexture); }

public:
	CPlayer** m_pRenderObjects = NULL;

	CTexture* m_pRenderTexture = NULL;
};





//float4 ps_main(float4 toProj : TEXCOORD0, float4 toProjScrolll : TEXCORRD1,
//    float4 toProjScroll2 : TEXCOORD2, float4 IsPos_dept : TEXCOORD3, float4 ChannelMask : COLOR0,
//    uniform bool bScrollingNoise,
//    uniform bool bShadowMapping,
//    uniform bool bCookie) :COLOR
//{
//    float compositeNoise = 0.015f;
//float shadow = 1.0f;
//float4 cookie = (1.0f, 1.0f, 1.0f, 1.0f);
//
//float shadowMapDepth;
//float4 output; 
//
//if (bCookie) {
//    cookie = tex2Dproj(CookieSampler, toProj);
//}
//
//if (bScrollingNosie) {
//    float4 noise1 = tex2Dproj(ScrollingNoiseSampler, toProjScroll1);
//    float4 noise2 = tex2Dproj(ScrollingNoiseSampler, toProjScroll2);
//
//    compositeNoise = noise1.r * noise2.g * 0.05f;
//    
//}
//
//shadowMapDepth = tex2Dproj(ShadowMapSampler, tcProj);
//
//if (bShadowMapping) {
//    if (IsPos_depth.w < shadowMapDepth)
//        shadow = 1.0f;
//    else
//        shadow = 0.0f;
//}
//float atten = 0.25f + 20000.f / dot(IsPos_depth.xyz, IsPos_depth.xyz); 
//float scale = 9.0f / fFractionOfMaxShells; 
//
//output.rgb = compositeNoise * cookie.rgb * lightColor * scale * atten * shadow * ChannelMask;
//return output;
//}