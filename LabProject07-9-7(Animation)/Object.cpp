﻿//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include "Scene.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers, int nRootParameters)
{
	m_nTextureType = nTextureType;
	m_nTextures = nTextures;

	if (m_nTextures > 0)
	{
		m_pRootArgumentInfos = new SRVROOTARGUMENTINFO[m_nTextures];
		m_ppd3dTextureUploadBuffers = new ID3D12Resource * [m_nTextures];
		m_ppd3dTextures = new ID3D12Resource * [m_nTextures];
		m_pnResourceTypes = new UINT[m_nTextures];

		for (int i = 0; i < m_nTextures; i++) m_pnResourceTypes[i] = 0;
		m_pdxgiBufferFormats = new DXGI_FORMAT[m_nTextures];
		for (int i = 0; i < m_nTextures; i++) m_pnResourceTypes[i] = DXGI_FORMAT_UNKNOWN;
		m_pnBufferElements = new int[m_nTextures];
		for (int i = 0; i < m_nTextures; i++) m_pnBufferElements[i] = 0;
	}


	m_nRootParameters = nRootParameters;
	if (nRootParameters > 0) {
		m_pnRootParameterIndices = new int[nRootParameters];

		m_pd3dSrvGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nTextures];
		for (int i = 0; i < m_nTextures; i++) m_pd3dSrvGpuDescriptorHandles[i].ptr = NULL;
	}

	for (int i = 0; i < m_nRootParameters; i++) m_pnRootParameterIndices[i] = -1;


	m_nSamplers = nSamplers;
	if (m_nSamplers > 0) m_pd3dSamplerGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSamplers];
}

CTexture::~CTexture()
{
	if (m_ppd3dTextures)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextures[i]) m_ppd3dTextures[i]->Release();
		delete[] m_ppd3dTextures;
	}

	if (m_pRootArgumentInfos)
	{
		delete[] m_pRootArgumentInfos;
	}
	if (m_pdxgiBufferFormats) delete[] m_pdxgiBufferFormats;
	if (m_pnBufferElements) delete[] m_pnBufferElements;
	if (m_pd3dSrvGpuDescriptorHandles) delete[] m_pd3dSrvGpuDescriptorHandles;

	if (m_pd3dSamplerGpuDescriptorHandles) delete[] m_pd3dSamplerGpuDescriptorHandles;
}

void CTexture::SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_pRootArgumentInfos[nIndex].m_nRootParameterIndex = nRootParameterIndex;
	m_pRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle = d3dSrvGpuDescriptorHandle;
}

void CTexture::SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle)
{
	m_pd3dSamplerGpuDescriptorHandles[nIndex] = d3dSamplerGpuDescriptorHandle;
}

void CTexture::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_nTextureType == RESOURCE_TEXTURE2D_ARRAY)
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[0].m_nRootParameterIndex, m_pRootArgumentInfos[0].m_d3dSrvGpuDescriptorHandle);
	}
	else
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[i].m_nRootParameterIndex, m_pRootArgumentInfos[i].m_d3dSrvGpuDescriptorHandle);
		}
	}
}

void CTexture::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[nIndex].m_nRootParameterIndex, m_pRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle);
}
D3D12_SHADER_RESOURCE_VIEW_DESC CTexture::GetShaderResourceViewDesc(int nIndex)
{
	ID3D12Resource* pShaderResource = GetTexture(nIndex);
	D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	int nTextureType = GetTextureType(nIndex);
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY: //[]
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		if (d3dResourceDesc.Format == DXGI_FORMAT_D32_FLOAT) d3dShaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.Format = m_pdxgiBufferFormats[nIndex];
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = m_pnBufferElements[nIndex];
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}




void CTexture::ReleaseUploadBuffers()
{
	if (m_ppd3dTextureUploadBuffers)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextureUploadBuffers[i]) m_ppd3dTextureUploadBuffers[i]->Release();
		delete[] m_ppd3dTextureUploadBuffers;
		m_ppd3dTextureUploadBuffers = NULL;
	}
}

void CTexture::ReleaseShaderVariables()
{
}

void CTexture::LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex, bool bIsDDSFile)
{
	if (bIsDDSFile)
		m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromDDSFile(pd3dDevice, pd3dCommandList, pszFileName, &(m_ppd3dTextureUploadBuffers[nIndex]), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	else
		m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromWICFile(pd3dDevice, pd3dCommandList, pszFileName, &(m_ppd3dTextureUploadBuffers[nIndex]), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}
ID3D12Resource* CTexture::CreateTexture(ID3D12Device* pd3dDevice, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue, UINT nResourceType, UINT nIndex)
{
	//m_pnResourceTypes[nIndex] = nResourceType;
	m_ppd3dTextures[nIndex] = ::CreateTexture2DResource(pd3dDevice, nWidth, nHeight, 1, 0, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	return(m_ppd3dTextures[nIndex]);
}
void CTexture::SetSrvGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_pd3dSrvGpuDescriptorHandles[nIndex] = d3dSrvGpuDescriptorHandle;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMaterial::CMaterial(int nTextures)
{
	m_nTextures = nTextures;

	m_ppTextures = new CTexture*[m_nTextures];
	m_ppstrTextureNames = new _TCHAR[m_nTextures][64];
	for (int i = 0; i < m_nTextures; i++) m_ppTextures[i] = NULL;
	for (int i = 0; i < m_nTextures; i++) m_ppstrTextureNames[i][0] = '\0';
}

CMaterial::~CMaterial()
{
	if (m_pShader) m_pShader->Release();

	if (m_nTextures > 0)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppTextures[i]) m_ppTextures[i]->Release();
		delete[] m_ppTextures;

		if (m_ppstrTextureNames) delete[] m_ppstrTextureNames;
	}
}

void CMaterial::SetShader(CShader *pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CMaterial::SetTexture(CTexture *pTexture, UINT nTexture) 
{ 
	if (m_ppTextures[nTexture]) m_ppTextures[nTexture]->Release();
	m_ppTextures[nTexture] = pTexture; 
	if (m_ppTextures[nTexture]) m_ppTextures[nTexture]->AddRef();  
}

void CMaterial::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i]) m_ppTextures[i]->ReleaseUploadBuffers();
	}
}

CShader *CMaterial::m_pSkinnedAnimationShader = NULL;
CShader *CMaterial::m_pStandardShader = NULL;

void CMaterial::PrepareShaders(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	m_pStandardShader = new CStandardShader();
	m_pStandardShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pStandardShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pSkinnedAnimationShader = new CSkinnedAnimationStandardShader();
	m_pSkinnedAnimationShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pSkinnedAnimationShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CMaterial::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4AmbientColor, 16);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4AlbedoColor, 20);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4SpecularColor, 24);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4EmissiveColor, 28);

	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_nType, 32);

	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i]) m_ppTextures[i]->UpdateShaderVariable(pd3dCommandList, 0);
	}
}

void CMaterial::LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR *pwstrTextureName, CTexture **ppTexture, CGameObject *pParent, FILE *pInFile, CShader *pShader)
{
	char pstrTextureName[64] = { '\0' };

	BYTE nStrLength = ::ReadStringFromFile(pInFile, pstrTextureName); 

	bool bDuplicated = false;
	if (strcmp(pstrTextureName, "null"))
	{
		SetMaterialType(nType);

		char pstrFilePath[64] = { '\0' };
		strcpy_s(pstrFilePath, 64, "Model/Textures/");
		
		bDuplicated = (pstrTextureName[0] == '@');
		strcpy_s(pstrFilePath + 15, 64 - 15, (bDuplicated) ? (pstrTextureName+1) : pstrTextureName);
		strcpy_s(pstrFilePath + 15 + ((bDuplicated) ? (nStrLength - 1) : nStrLength), 64 - 15 - ((bDuplicated) ? (nStrLength - 1) : nStrLength), ".dds");

		size_t nConverted = 0;
		mbstowcs_s(&nConverted, pwstrTextureName, 64, pstrFilePath, _TRUNCATE);

//#define _WITH_DISPLAY_TEXTURE_NAME

#ifdef _WITH_DISPLAY_TEXTURE_NAME
		static int nTextures = 0, nRepeatedTextures = 0;
		TCHAR pstrDebug[256] = { 0 };
		_stprintf_s(pstrDebug, 256, _T("Texture Name: %d %c %s\n"), (pstrTextureName[0] == '@') ? nRepeatedTextures++ : nTextures++, (pstrTextureName[0] == '@') ? '@' : ' ', pwstrTextureName);
		OutputDebugString(pstrDebug);
#endif
		if (!bDuplicated)
		{
			*ppTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
			(*ppTexture)->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pwstrTextureName, 0, true);
			if (*ppTexture) (*ppTexture)->AddRef();

			CScene::CreateShaderResourceViews(pd3dDevice, *ppTexture, nRootParameter, false);
		}
		else
		{
			if (pParent)
			{
				while (pParent)
				{
					if (!pParent->m_pParent) break;
					pParent = pParent->m_pParent;
				}
				CGameObject *pRootGameObject = pParent;
				*ppTexture = pRootGameObject->FindReplicatedTexture(pwstrTextureName);
				if (*ppTexture) (*ppTexture)->AddRef();
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationSet::CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrames, int nAnimatedBones, char *pstrName)
{
	m_fLength = fLength;
	m_nFramesPerSecond = nFramesPerSecond;
	m_nKeyFrames = nKeyFrames;

	strcpy_s(m_pstrAnimationSetName, 64, pstrName);

	m_pfKeyFrameTimes = new float[nKeyFrames];
	m_ppxmf4x4KeyFrameTransforms = new XMFLOAT4X4*[nKeyFrames];
	for (int i = 0; i < nKeyFrames; i++) m_ppxmf4x4KeyFrameTransforms[i] = new XMFLOAT4X4[nAnimatedBones];
}

CAnimationSet::~CAnimationSet()
{
	if (m_pfKeyFrameTimes) delete[] m_pfKeyFrameTimes;
	for (int j = 0; j < m_nKeyFrames; j++) if (m_ppxmf4x4KeyFrameTransforms[j]) delete[] m_ppxmf4x4KeyFrameTransforms[j];
	if (m_ppxmf4x4KeyFrameTransforms) delete[] m_ppxmf4x4KeyFrameTransforms;
	
	if (m_pCallbackKeys) delete[] m_pCallbackKeys;
	if (m_pAnimationCallbackHandler) delete m_pAnimationCallbackHandler;
}

void CAnimationSet::HandleCallback()
{
	if (m_pAnimationCallbackHandler)
	{
		for (int i = 0; i < m_nCallbackKeys; i++)
		{
			if (::IsEqual(m_pCallbackKeys[i].m_fTime, m_fPosition, ANIMATION_CALLBACK_EPSILON))
			{
				if (m_pCallbackKeys[i].m_pCallbackData) m_pAnimationCallbackHandler->HandleCallback(m_pCallbackKeys[i].m_pCallbackData, m_fPosition);
				break;
			}
		}
	}
}

void CAnimationSet::SetPosition(float fElapsedPosition)
{
	switch (m_nType)
	{
		case ANIMATION_TYPE_LOOP:
		{
			m_fPosition += fElapsedPosition;
			if (m_fPosition >= m_fLength) m_fPosition = 0.0f;
//			m_fPosition = fmod(fTrackPosition, m_pfKeyFrameTimes[m_nKeyFrames-1]); // m_fPosition = fTrackPosition - int(fTrackPosition / m_pfKeyFrameTimes[m_nKeyFrames-1]) * m_pfKeyFrameTimes[m_nKeyFrames-1];
//			m_fPosition = fmod(fTrackPosition, m_fLength); //if (m_fPosition < 0) m_fPosition += m_fLength;
//			m_fPosition = fTrackPosition - int(fTrackPosition / m_fLength) * m_fLength;
			break;
		}
		case ANIMATION_TYPE_ONCE:
			break;
		case ANIMATION_TYPE_PINGPONG:
			break;
	}
}

XMFLOAT4X4 CAnimationSet::GetSRT(int nBone)
{
	XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Identity();
#ifdef _WITH_ANIMATION_SRT
	XMVECTOR S, R, T;
	for (int i = 0; i < (m_nKeyFrameTranslations - 1); i++)
	{
		if ((m_pfKeyFrameTranslationTimes[i] <= m_fPosition) && (m_fPosition <= m_pfKeyFrameTranslationTimes[i+1]))
		{
			float t = (m_fPosition - m_pfKeyFrameTranslationTimes[i]) / (m_pfKeyFrameTranslationTimes[i+1] - m_pfKeyFrameTranslationTimes[i]);
			T = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i][nBone]), XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i+1][nBone]), t);
			break;
		}
	}
	for (UINT i = 0; i < (m_nKeyFrameScales - 1); i++)
	{
		if ((m_pfKeyFrameScaleTimes[i] <= m_fPosition) && (m_fPosition <= m_pfKeyFrameScaleTimes[i+1]))
		{
			float t = (m_fPosition - m_pfKeyFrameScaleTimes[i]) / (m_pfKeyFrameScaleTimes[i+1] - m_pfKeyFrameScaleTimes[i]);
			S = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameScales[i][nBone]), XMLoadFloat3(&m_ppxmf3KeyFrameScales[i+1][nBone]), t);
			break;
		}
	}
	for (UINT i = 0; i < (m_nKeyFrameRotations - 1); i++)
	{
		if ((m_pfKeyFrameRotationTimes[i] <= m_fPosition) && (m_fPosition <= m_pfKeyFrameRotationTimes[i+1]))
		{
			float t = (m_fPosition - m_pfKeyFrameRotationTimes[i]) / (m_pfKeyFrameRotationTimes[i+1] - m_pfKeyFrameRotationTimes[i]);
			R = XMQuaternionSlerp(XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i][nBone]), XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i+1][nBone]), t);
			break;
		}
	}

	XMStoreFloat4x4(&xmf4x4Transform, XMMatrixAffineTransformation(S, NULL, R, T));
#else   
	for (int i = 0; i < (m_nKeyFrames - 1); i++) 
	{
		if ((m_pfKeyFrameTimes[i] <= m_fPosition) && (m_fPosition < m_pfKeyFrameTimes[i+1]))
		{
			float t = (m_fPosition - m_pfKeyFrameTimes[i]) / (m_pfKeyFrameTimes[i+1] - m_pfKeyFrameTimes[i]);
			xmf4x4Transform = Matrix4x4::Interpolate(m_ppxmf4x4KeyFrameTransforms[i][nBone], m_ppxmf4x4KeyFrameTransforms[i+1][nBone], t);
			break;
		}
	}
	if (m_fPosition >= m_pfKeyFrameTimes[m_nKeyFrames-1]) xmf4x4Transform = m_ppxmf4x4KeyFrameTransforms[m_nKeyFrames-1][nBone];

#endif
	return(xmf4x4Transform);
}

void CAnimationSet::SetCallbackKeys(int nCallbackKeys)
{
	m_nCallbackKeys = nCallbackKeys;
	m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
}

void CAnimationSet::SetCallbackKey(int nKeyIndex, float fKeyTime, void *pData)
{
	m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_pCallbackKeys[nKeyIndex].m_pCallbackData = pData;
}

void CAnimationSet::SetAnimationCallbackHandler(CAnimationCallbackHandler *pCallbackHandler)
{
	m_pAnimationCallbackHandler = pCallbackHandler;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationSets::CAnimationSets(int nAnimationSets)
{
	m_nAnimationSets = nAnimationSets;
	m_pAnimationSets = new CAnimationSet*[nAnimationSets];
}

CAnimationSets::~CAnimationSets()
{
	for (int i = 0; i < m_nAnimationSets; i++) if (m_pAnimationSets[i]) delete m_pAnimationSets[i];
	if (m_pAnimationSets) delete[] m_pAnimationSets;

	if (m_ppAnimatedBoneFrameCaches) delete[] m_ppAnimatedBoneFrameCaches;
}

void CAnimationSets::SetCallbackKeys(int nAnimationSet, int nCallbackKeys)
{
	m_pAnimationSets[nAnimationSet]->m_nCallbackKeys = nCallbackKeys;
	m_pAnimationSets[nAnimationSet]->m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
}

void CAnimationSets::SetCallbackKey(int nAnimationSet, int nKeyIndex, float fKeyTime, void *pData)
{
	m_pAnimationSets[nAnimationSet]->m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_pAnimationSets[nAnimationSet]->m_pCallbackKeys[nKeyIndex].m_pCallbackData = pData;
}

void CAnimationSets::SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler *pCallbackHandler)
{
	m_pAnimationSets[nAnimationSet]->SetAnimationCallbackHandler(pCallbackHandler);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationController::CAnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nAnimationTracks, CLoadedModelInfo *pModel)
{
	m_nAnimationTracks = nAnimationTracks;
    m_pAnimationTracks = new CAnimationTrack[nAnimationTracks];

	m_pAnimationSets = pModel->m_pAnimationSets;
	m_pAnimationSets->AddRef();

	m_nSkinnedMeshes = pModel->m_nSkinnedMeshes;
	m_ppSkinnedMeshes = new CSkinnedMesh*[m_nSkinnedMeshes];
	for (int i = 0; i < m_nSkinnedMeshes; i++) m_ppSkinnedMeshes[i] = pModel->m_ppSkinnedMeshes[i];

	m_ppd3dcbSkinningBoneTransforms = new ID3D12Resource*[m_nSkinnedMeshes];
	m_ppcbxmf4x4MappedSkinningBoneTransforms = new XMFLOAT4X4*[m_nSkinnedMeshes];

	UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppd3dcbSkinningBoneTransforms[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
		m_ppd3dcbSkinningBoneTransforms[i]->Map(0, NULL, (void **)&m_ppcbxmf4x4MappedSkinningBoneTransforms[i]);
	}
}

CAnimationController::~CAnimationController()
{
	if (m_pAnimationTracks) delete[] m_pAnimationTracks;

	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppd3dcbSkinningBoneTransforms[i]->Unmap(0, NULL);
		m_ppd3dcbSkinningBoneTransforms[i]->Release();
	}
	if (m_ppd3dcbSkinningBoneTransforms) delete[] m_ppd3dcbSkinningBoneTransforms;
	if (m_ppcbxmf4x4MappedSkinningBoneTransforms) delete[] m_ppcbxmf4x4MappedSkinningBoneTransforms;

	if (m_pAnimationSets) m_pAnimationSets->Release();

	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;
}

void CAnimationController::SetCallbackKeys(int nAnimationSet, int nCallbackKeys)
{
	if (m_pAnimationSets) m_pAnimationSets->SetCallbackKeys(nAnimationSet, nCallbackKeys);
}

void CAnimationController::SetCallbackKey(int nAnimationSet, int nKeyIndex, float fKeyTime, void *pData)
{
	if (m_pAnimationSets) m_pAnimationSets->SetCallbackKey(nAnimationSet, nKeyIndex, fKeyTime, pData);
}

void CAnimationController::SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler *pCallbackHandler)
{
	if (m_pAnimationSets) m_pAnimationSets->SetAnimationCallbackHandler(nAnimationSet, pCallbackHandler);
}

void CAnimationController::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pAnimationTracks)
	{
		m_pAnimationTracks[nAnimationTrack].m_nAnimationSet = nAnimationSet;
//		m_pAnimationTracks[nAnimationTrack].m_fPosition = 0.0f;
//		if (m_pAnimationSets) m_pAnimationSets->m_pAnimationSets[nAnimationSet]->m_fPosition = 0.0f;
	}
}

void CAnimationController::SetTrackEnable(int nAnimationTrack, bool bEnable)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetEnable(bEnable);
}

void CAnimationController::SetTrackPosition(int nAnimationTrack, float fPosition)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetPosition(fPosition);
}

void CAnimationController::SetTrackSpeed(int nAnimationTrack, float fSpeed)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetSpeed(fSpeed);
}

void CAnimationController::SetTrackWeight(int nAnimationTrack, float fWeight)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetWeight(fWeight);
}

void CAnimationController::SetTrackAllUnable(int Cnt)
{
	if (m_pAnimationTracks)
		for (int i = 0; i < Cnt; ++i) {
			m_pAnimationTracks[Cnt].SetEnable(false);
		}	
}

void CAnimationController::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppSkinnedMeshes[i]->m_pd3dcbSkinningBoneTransforms = m_ppd3dcbSkinningBoneTransforms[i];
		m_ppSkinnedMeshes[i]->m_pcbxmf4x4MappedSkinningBoneTransforms = m_ppcbxmf4x4MappedSkinningBoneTransforms[i];
	}
}

void CAnimationController::AdvanceTime(float fTimeElapsed, CGameObject *pRootGameObject) 
{
	m_fTime += fTimeElapsed; 
	if (m_pAnimationTracks)
	{
//		for (int k = 0; k < m_nAnimationTracks; k++) m_pAnimationTracks[k].m_fPosition += (fTimeElapsed * m_pAnimationTracks[k].m_fSpeed);
		for (int k = 0; k < m_nAnimationTracks; k++) m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet]->SetPosition(fTimeElapsed * m_pAnimationTracks[k].m_fSpeed);

		for (int j = 0; j < m_pAnimationSets->m_nAnimatedBoneFrames; j++)
		{
			XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Zero();
			for (int k = 0; k < m_nAnimationTracks; k++)
			{
				if (m_pAnimationTracks[k].m_bEnable)
				{
					CAnimationSet *pAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet];
					XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j);
					xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, Matrix4x4::Scale(xmf4x4TrackTransform, m_pAnimationTracks[k].m_fWeight));
				}
			}
			m_pAnimationSets->m_ppAnimatedBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
		}

		pRootGameObject->UpdateTransform(NULL);

		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_bEnable) m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet]->HandleCallback();
		}
	}
} 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CLoadedModelInfo::~CLoadedModelInfo()
{
	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;
}

void CLoadedModelInfo::PrepareSkinning()
{
	int nSkinnedMesh = 0;
	m_ppSkinnedMeshes = new CSkinnedMesh*[m_nSkinnedMeshes];
	m_pModelRootObject->FindAndSetSkinnedMesh(m_ppSkinnedMeshes, &nSkinnedMesh);

	for (int i = 0; i < m_nSkinnedMeshes; i++) m_ppSkinnedMeshes[i]->PrepareSkinning(m_pModelRootObject);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject::CGameObject()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
	m_xmf4x4Texture = Matrix4x4::Identity();
}

CGameObject::CGameObject(int nMaterials) : CGameObject()
{
	m_nMaterials = nMaterials;
	if (m_nMaterials > 0)
	{
		m_ppMaterials = new CMaterial*[m_nMaterials];
		for(int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;
	}
}

CGameObject::~CGameObject()
{
	if (m_pMesh) m_pMesh->Release();

	if (m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i]) m_ppMaterials[i]->Release();
		}
	}
	if (m_ppMaterials) delete[] m_ppMaterials;

	if (m_pSkinnedAnimationController) delete m_pSkinnedAnimationController;
}

void CGameObject::AddRef() 
{ 
	m_nReferences++; 

	if (m_pSibling) m_pSibling->AddRef();
	if (m_pChild) m_pChild->AddRef();
}

void CGameObject::Release() 
{ 
	if (m_pChild) m_pChild->Release();
	if (m_pSibling) m_pSibling->Release();

	if (--m_nReferences <= 0) delete this;  
}

void CGameObject::SetChild(CGameObject *pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();

	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;

	}
	else
	{
		m_pChild = pChild;
		
	}

}

void CGameObject::SetMesh(CMesh *pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void CGameObject::SetShader(CShader *pShader)
{
	m_nMaterials = 1;
	m_ppMaterials = new CMaterial*[m_nMaterials];
	m_ppMaterials[0] = new CMaterial(0);
	m_ppMaterials[0]->SetShader(pShader);
}

void CGameObject::SetShader(int nMaterial, CShader *pShader)
{
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->SetShader(pShader);
}

void CGameObject::SetMaterial(int nMaterial, CMaterial *pMaterial)
{
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->Release();
	m_ppMaterials[nMaterial] = pMaterial;
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->AddRef();
}

void CGameObject::SetTexture(CTexture* texture)
{
	if (m_ppMaterials != NULL)
	{
		//m_nMaterials = 1;
		//m_ppMaterials = new CMaterial * [m_nMaterials];
		CMaterial* pMaterial = new CMaterial(1); //
		pMaterial->SetTexture(texture);
		m_ppMaterials[0] = pMaterial;
	}
}

CSkinnedMesh *CGameObject::FindSkinnedMesh(char *pstrSkinnedMeshName)
{
	CSkinnedMesh *pSkinnedMesh = NULL;
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT)) 
	{
		pSkinnedMesh = (CSkinnedMesh *)m_pMesh;
		if(!strncmp(pSkinnedMesh->m_pstrMeshName, pstrSkinnedMeshName, strlen(pstrSkinnedMeshName))) return(pSkinnedMesh);
	}

	if (m_pSibling) if (pSkinnedMesh = m_pSibling->FindSkinnedMesh(pstrSkinnedMeshName)) return(pSkinnedMesh);
	if (m_pChild) if (pSkinnedMesh = m_pChild->FindSkinnedMesh(pstrSkinnedMeshName)) return(pSkinnedMesh);

	return(NULL);
}

void CGameObject::FindAndSetSkinnedMesh(CSkinnedMesh **ppSkinnedMeshes, int *pnSkinnedMesh)
{
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT)) ppSkinnedMeshes[(*pnSkinnedMesh)++] = (CSkinnedMesh *)m_pMesh;

	if (m_pSibling) m_pSibling->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
	if (m_pChild) m_pChild->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
}

CGameObject *CGameObject::FindFrame(char *pstrFrameName)
{
	CGameObject *pFrameObject = NULL;
	if (!strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

void CGameObject::UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);

}

void CGameObject::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackAnimationSet(nAnimationTrack, nAnimationSet);
}

void CGameObject::SetTrackAnimationPosition(int nAnimationTrack, float fPosition)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackPosition(nAnimationTrack, fPosition);
}

float CGameObject::GetTrackAnimationPosition(int nAnimationTrack)
{
	return 0.0f;
}

void CGameObject::Animate(float fTimeElapsed)
{
	//OnPrepareRender();

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);

	if (m_pSibling) m_pSibling->Animate(fTimeElapsed);
	if (m_pChild) m_pChild->Animate(fTimeElapsed);
}

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	if (m_pMesh)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

		if (m_nMaterials > 0)
		{
			for (int i = 0; i < m_nMaterials; i++)
			{
				if (m_ppMaterials[i])
				{
					if (m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
					m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);
				}

				m_pMesh->Render(pd3dCommandList, i);
			}
		}
	}

	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera);
}
void CGameObject::ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	if (m_pMesh)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

		if (m_nMaterials > 0)
		{
			for (int i = 0; i < m_nMaterials; i++)
			{
				if (m_ppMaterials[i])
				{
					//if (m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
					m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);
				}
				m_pMesh->Render(pd3dCommandList, i);
				//m_pMesh->ShadowRender(pd3dCommandList, i);
			}
		}
	}

	if (m_pSibling) m_pSibling->ShadowRender(pd3dCommandList, pCamera);
	if (m_pChild) m_pChild->ShadowRender(pd3dCommandList, pCamera);
}
void CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, CMaterial *pMaterial)
{
}

void CGameObject::ReleaseShaderVariables()
{
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
	}

	if (m_pSibling!=NULL) 
		m_pSibling->ReleaseUploadBuffers();
	if (m_pChild != NULL)
		m_pChild->ReleaseUploadBuffers();
}

void CGameObject::ReleaseUploadBuffers2()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
	}
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4ToParent._41 = x;
	m_xmf4x4ToParent._42 = y;
	m_xmf4x4ToParent._43 = z;

	UpdateTransform(NULL);
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf3Scale.x = x;
	m_xmf3Scale.y = y;
	m_xmf3Scale.z = z;
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void CGameObject::SetLookAt(XMFLOAT3 xmf3Target, XMFLOAT3 xmf3Up)
{
	XMFLOAT3 xmf3Position(m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43);
	XMFLOAT4X4 mtxLookAt = Matrix4x4::LookAtLH(xmf3Position, xmf3Target, xmf3Up);
	m_xmf4x4ToParent._11 = mtxLookAt._11; m_xmf4x4ToParent._12 = mtxLookAt._21; m_xmf4x4ToParent._13 = mtxLookAt._31;
	m_xmf4x4ToParent._21 = mtxLookAt._12; m_xmf4x4ToParent._22 = mtxLookAt._22; m_xmf4x4ToParent._23 = mtxLookAt._32;
	m_xmf4x4ToParent._31 = mtxLookAt._13; m_xmf4x4ToParent._32 = mtxLookAt._23; m_xmf4x4ToParent._33 = mtxLookAt._33;
}

void CGameObject::SetQuaternion(XMFLOAT4 Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(&Quaternion));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, Matrix4x4::Identity());
	m_xmf4x4ToParent = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4ToParent);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT4 *pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

//#define _WITH_DEBUG_FRAME_HIERARCHY

CTexture *CGameObject::FindReplicatedTexture(_TCHAR *pstrTextureName)
{
	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i])
		{
			for (int j = 0; j < m_ppMaterials[i]->m_nTextures; j++)
			{
				if (m_ppMaterials[i]->m_ppTextures[j])
				{
					if (!_tcsncmp(m_ppMaterials[i]->m_ppstrTextureNames[j], pstrTextureName, _tcslen(pstrTextureName))) return(m_ppMaterials[i]->m_ppTextures[j]);
				}
			}
		}
	}
	CTexture *pTexture = NULL;
	if (m_pSibling) if (pTexture = m_pSibling->FindReplicatedTexture(pstrTextureName)) return(pTexture);
	if (m_pChild) if (pTexture = m_pChild->FindReplicatedTexture(pstrTextureName)) return(pTexture);

	return(NULL);
}

void CGameObject::ResetScale()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
}

int ReadIntegerFromFile(FILE *pInFile)
{
	int nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile); 
	return(nValue);
}

float ReadFloatFromFile(FILE *pInFile)
{
	float fValue = 0;
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile); 
	return(fValue);
}

BYTE ReadStringFromFile(FILE *pInFile, char *pstrToken)
{
	BYTE nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile); 
	pstrToken[nStrLength] = '\0';

	return(nStrLength);
}

void CGameObject::LoadMaterialsFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject *pParent, FILE *pInFile, CShader *pShader)
{
	char pstrToken[64] = { '\0' };
	int nMaterial = 0;
	UINT nReads = 0;

	m_nMaterials = ReadIntegerFromFile(pInFile);

	m_ppMaterials = new CMaterial*[m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;

	CMaterial *pMaterial = NULL;

	for ( ; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Material>:"))
		{
			nMaterial = ReadIntegerFromFile(pInFile);

			pMaterial = new CMaterial(7); //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

			if (!pShader)
			{
				UINT nMeshType = GetMeshType();
				if (nMeshType & VERTEXT_NORMAL_TANGENT_TEXTURE)
				{
					if (nMeshType & VERTEXT_BONE_INDEX_WEIGHT)
					{
						pMaterial->SetSkinnedAnimationShader();
					}
					else
					{
						pMaterial->SetStandardShader();
					}
				}
			}
			SetMaterial(nMaterial, pMaterial);
		}
		else if (!strcmp(pstrToken, "<AlbedoColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4AlbedoColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<EmissiveColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4EmissiveColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4SpecularColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<Glossiness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossiness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Smoothness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSmoothness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Metallic>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSpecularHighlight), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularHighlight>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fMetallic), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<GlossyReflection>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossyReflection), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<AlbedoMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_ALBEDO_MAP, 3, pMaterial->m_ppstrTextureNames[0], &(pMaterial->m_ppTextures[0]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<SpecularMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_SPECULAR_MAP, 4, pMaterial->m_ppstrTextureNames[1], &(pMaterial->m_ppTextures[1]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<NormalMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_NORMAL_MAP, 5, pMaterial->m_ppstrTextureNames[2], &(pMaterial->m_ppTextures[2]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<MetallicMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_METALLIC_MAP, 6, pMaterial->m_ppstrTextureNames[3], &(pMaterial->m_ppTextures[3]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<EmissionMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_EMISSION_MAP, 7, pMaterial->m_ppstrTextureNames[4], &(pMaterial->m_ppTextures[4]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<DetailAlbedoMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_DETAIL_ALBEDO_MAP, 8, pMaterial->m_ppstrTextureNames[5], &(pMaterial->m_ppTextures[5]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<DetailNormalMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_DETAIL_NORMAL_MAP, 9, pMaterial->m_ppstrTextureNames[6], &(pMaterial->m_ppTextures[6]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "</Materials>"))
		{
			break;
		}
	}
}

CGameObject *CGameObject::LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CGameObject *pParent, FILE *pInFile, CShader *pShader, int *pnSkinnedMeshes)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	CGameObject *pGameObject = new CGameObject();

	for ( ; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Frame>:"))
		{
			nFrame = ::ReadIntegerFromFile(pInFile);
			nTextures = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
		}
		else if (!strcmp(pstrToken, "<Transform>:"))
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4ToParent, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			CStandardMesh *pMesh = new CStandardMesh(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pGameObject->SetMesh(pMesh);
		}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			if (pnSkinnedMeshes) (*pnSkinnedMeshes)++;

			CSkinnedMesh *pSkinnedMesh = new CSkinnedMesh(pd3dDevice, pd3dCommandList);
			pSkinnedMesh->LoadSkinInfoFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			::ReadStringFromFile(pInFile, pstrToken); //<Mesh>:
			if (!strcmp(pstrToken, "<Mesh>:")) pSkinnedMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);

			pGameObject->SetMesh(pSkinnedMesh);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					CGameObject *pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pShader, pnSkinnedMeshes);
					if (pChild) pGameObject->SetChild(pChild);
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
					TCHAR pstrDebug[256] = { 0 };
					_stprintf_s(pstrDebug, 256, "(Frame: %p) (Parent: %p)\n"), pChild, pGameObject);
					OutputDebugString(pstrDebug);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	return(pGameObject);
}

void CGameObject::PrintFrameInfo(CGameObject *pGameObject, CGameObject *pParent)
{
	TCHAR pstrDebug[256] = { 0 };

	_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	OutputDebugString(pstrDebug);

	if (pGameObject->m_pSibling) CGameObject::PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) CGameObject::PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}

void CGameObject::LoadAnimationFromFile(FILE *pInFile, CLoadedModelInfo *pLoadedModel)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nAnimationSets = 0;

	for ( ; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<AnimationSets>:"))
		{
			nAnimationSets = ::ReadIntegerFromFile(pInFile);
			pLoadedModel->m_pAnimationSets = new CAnimationSets(nAnimationSets);
		}
		else if (!strcmp(pstrToken, "<FrameNames>:"))
		{
			pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames = ::ReadIntegerFromFile(pInFile); 
			pLoadedModel->m_pAnimationSets->m_ppAnimatedBoneFrameCaches = new CGameObject*[pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames];

			for (int j = 0; j < pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames; j++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				pLoadedModel->m_pAnimationSets->m_ppAnimatedBoneFrameCaches[j] = pLoadedModel->m_pModelRootObject->FindFrame(pstrToken);

#ifdef _WITH_DEBUG_SKINNING_BONE
				TCHAR pstrDebug[256] = { 0 };
				TCHAR pwstrAnimationBoneName[64] = { 0 };
				TCHAR pwstrBoneCacheName[64] = { 0 };
				size_t nConverted = 0;
				mbstowcs_s(&nConverted, pwstrAnimationBoneName, 64, pstrToken, _TRUNCATE);
				mbstowcs_s(&nConverted, pwstrBoneCacheName, 64, pLoadedModel->m_ppAnimatedBoneFrameCaches[j]->m_pstrFrameName, _TRUNCATE);
				_stprintf_s(pstrDebug, 256, _T("AnimationBoneFrame:: Cache(%s) AnimationBone(%s)\n"), pwstrBoneCacheName, pwstrAnimationBoneName);
				OutputDebugString(pstrDebug);
#endif
			}
		}
		else if (!strcmp(pstrToken, "<AnimationSet>:"))
		{
			int nAnimationSet = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pstrToken); //Animation Set Name

			float fLength = ::ReadFloatFromFile(pInFile);
			int nFramesPerSecond = ::ReadIntegerFromFile(pInFile);
			int nKeyFrames = ::ReadIntegerFromFile(pInFile);

			pLoadedModel->m_pAnimationSets->m_pAnimationSets[nAnimationSet] = new CAnimationSet(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames, pstrToken);

			for (int i = 0; i < nKeyFrames; i++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				if (!strcmp(pstrToken, "<Transforms>:"))
				{
					int nKey = ::ReadIntegerFromFile(pInFile); //i
					float fKeyTime = ::ReadFloatFromFile(pInFile);

					CAnimationSet *pAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[nAnimationSet];
					pAnimationSet->m_pfKeyFrameTimes[i] = fKeyTime;
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i], sizeof(XMFLOAT4X4), pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames, pInFile);
				}
			}
		}
		else if (!strcmp(pstrToken, "</AnimationSets>"))
		{
			break;
		}
	}
}

CLoadedModelInfo *CGameObject::LoadGeometryAndAnimationFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, char *pstrFileName, CShader *pShader)
{
	FILE *pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	CLoadedModelInfo *pLoadedModel = new CLoadedModelInfo();

	char pstrToken[64] = { '\0' };

	for ( ; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>:"))
			{
				pLoadedModel->m_pModelRootObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader, &pLoadedModel->m_nSkinnedMeshes);
				::ReadStringFromFile(pInFile, pstrToken); //"</Hierarchy>"
			}
			else if (!strcmp(pstrToken, "<Animation>:"))
			{
				CGameObject::LoadAnimationFromFile(pInFile, pLoadedModel);
				pLoadedModel->PrepareSkinning();
			}
			else if (!strcmp(pstrToken, "</Animation>:"))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, "Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	CGameObject::PrintFrameInfo(pGameObject, NULL);
#endif

	return(pLoadedModel);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color) : CGameObject(1)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	//CHeightMapGridMesh *pMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, 0, 0, nWidth, nLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
	//SetMesh(pMesh);

	//CreateShaderVariables(pd3dDevice, pd3dCommandList);

	///*CTexture *pTerrainBaseTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	//pTerrainBaseTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Terrain/Base_Texture.dds", 0);

	//CTexture *pTerrainDetailTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	//pTerrainDetailTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Terrain/Detail_Texture_7.dds", 0);*/

	//CTerrainShader *pTerrainShader = new CTerrainShader();
	//pTerrainShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	//pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//CScene::CreateShaderResourceViews(pd3dDevice, pTerrainBaseTexture, 13, false);
	//CScene::CreateShaderResourceViews(pd3dDevice, pTerrainDetailTexture, 14, false);

	//CMaterial *pTerrainMaterial = new CMaterial(0);
	////pTerrainMaterial->SetTexture(pTerrainBaseTexture, 0);
	////pTerrainMaterial->SetTexture(pTerrainDetailTexture, 1);
	//pTerrainMaterial->SetShader(pTerrainShader);

	//SetMaterial(0, pTerrainMaterial);
}

CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBox::CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) : CGameObject(1)
{
	CSkyBoxMesh *pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 2.0f);
	SetMesh(pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture *pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0);
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"SkyBox/Texture1.dds", 0);

	CSkyBoxShader *pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pSkyBoxTexture, 10, false);

	CMaterial *pSkyBoxMaterial = new CMaterial(1);
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture);
	pSkyBoxMaterial->SetShader(pSkyBoxShader);

	SetMaterial(0, pSkyBoxMaterial);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	CGameObject::Render(pd3dCommandList, pCamera);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSuperCobraObject::CSuperCobraObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
}

CSuperCobraObject::~CSuperCobraObject()
{
}

void CSuperCobraObject::OnPrepareAnimate()
{
	m_pMainRotorFrame = FindFrame("MainRotor");
	m_pTailRotorFrame = FindFrame("TailRotor");
}

void CSuperCobraObject::Animate(float fTimeElapsed)
{
	if (m_pMainRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4ToParent);
	}
	if (m_pTailRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4ToParent);
	}

	CGameObject::Animate(fTimeElapsed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGunshipObject::CGunshipObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
}

CGunshipObject::~CGunshipObject()
{
}

void CGunshipObject::OnPrepareAnimate()
{
	m_pMainRotorFrame = FindFrame("Rotor");
	m_pTailRotorFrame = FindFrame("Back_Rotor");
}

void CGunshipObject::Animate(float fTimeElapsed)
{
	if (m_pMainRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4ToParent);
	}
	if (m_pTailRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4ToParent);
	}

	CGameObject::Animate(fTimeElapsed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMi24Object::CMi24Object(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
}

CMi24Object::~CMi24Object()
{
}

void CMi24Object::OnPrepareAnimate()
{
	m_pMainRotorFrame = FindFrame("Top_Rotor");
	m_pTailRotorFrame = FindFrame("Tail_Rotor");
}

void CMi24Object::Animate(float fTimeElapsed)
{
	if (m_pMainRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4ToParent);
	}
	if (m_pTailRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4ToParent);
	}

	CGameObject::Animate(fTimeElapsed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAngrybotObject::CAngrybotObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks)
{
	CLoadedModelInfo *pAngrybotModel = pModel;
	if (!pAngrybotModel) pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Player.bin", NULL);

	SetChild(pAngrybotModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pAngrybotModel);

}

CAngrybotObject::~CAngrybotObject()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMonsterObject::CMonsterObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks)
{
	CLoadedModelInfo *pMonsterModel = pModel;
	if (!pMonsterModel) pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Monster.bin", NULL);

	SetChild(pMonsterModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pMonsterModel);

}

CMonsterObject::~CMonsterObject()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHumanoidObject::CHumanoidObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks)
{
	CLoadedModelInfo *pHumanoidModel = pModel;
	if (!pHumanoidModel) pHumanoidModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Humanoid.bin", NULL);

	SetChild(pHumanoidModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pHumanoidModel);


}

CHumanoidObject::~CHumanoidObject()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CEthanObject::CEthanObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks)
{
	CLoadedModelInfo *pEthanModel = pModel;
	if (!pEthanModel) pEthanModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Ethan.bin", NULL);

	SetChild(pEthanModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pEthanModel);


}

CEthanObject::~CEthanObject()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CLionObject::CLionObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks)
{
	CLoadedModelInfo *pLionModel = pModel;
	if (!pLionModel) pLionModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/arrow.bin", NULL);

	SetChild(pLionModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pLionModel);


}

CLionObject::~CLionObject()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CZebraObject::CZebraObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks)
{
	CLoadedModelInfo *pZebraModel = pModel;
	if (!pZebraModel) pZebraModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Red.bin", NULL);

	SetChild(pZebraModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pZebraModel);


}

CZebraObject::~CZebraObject()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CEagleObject::CEagleObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks)
{
	CLoadedModelInfo *pEagleModel = pModel;
	if (!pEagleModel) pEagleModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Eagle.bin", NULL);

	SetChild(pEagleModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pEagleModel);


}

CEagleObject::~CEagleObject()
{
}

//============================================================================================
CMeteorObject::CMeteorObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	CLoadedModelInfo* pMeteorModel = pModel;
	if (!pMeteorModel) pMeteorModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/meteo.bin", NULL);

	SetChild(pMeteorModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pMeteorModel);


}

CMeteorObject::~CMeteorObject()
{
}

void CMeteorObject::Animate(float fTimeElapsed)
{
	XMFLOAT3 xmf3Position = GetPosition();
	xmf3Position = Vector3::Add(xmf3Position, m_xmf3MovingDirection, 50.0 * fTimeElapsed);
	//SetPosition(xmf3Position);

	CGameObject::Animate(fTimeElapsed);
}

//============================================================================================
CEnemyObject::CEnemyObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	isAlive = false;
	Maxhp = hp;

	CLoadedModelInfo* pEnemyModel = pModel;
	if (!pEnemyModel) pEnemyModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/AlienDestroyer.bin", NULL);


	SetChild(pEnemyModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pEnemyModel);

}

CEnemyObject::~CEnemyObject()
{
}

void CEnemyObject::VelocityUpdate(float fTimeElapsed, XMFLOAT3& player_look)
{
	float fLength = Vector3::Length(m_xmf3Velocity);
	float fMaxVelocity = 200.f;
	if (fLength > fMaxVelocity)
	{
		m_xmf3Velocity.x *= (fMaxVelocity / fLength) * fTimeElapsed * 30;
		m_xmf3Velocity.y *= (fMaxVelocity / fLength) * fTimeElapsed * 30;
		m_xmf3Velocity.z *= (fMaxVelocity / fLength) * fTimeElapsed * 30;
	}

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);

//	XMFLOAT3 LookVelocity = Vector3::ScalarProduct(player_look, fTimeElapsed * 50.f, false);
//	xmf3Velocity = Vector3::Add(LookVelocity, xmf3Velocity);
	XMFLOAT3 xmf3Position = Vector3::Add(GetPosition(), xmf3Velocity);
	SetPosition(xmf3Position);

	float fDeceleration = (100.0f * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	if (Vector3::Length(m_xmf3Velocity) > 0.00001f) {
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
	}
}

void CEnemyObject::Reset()
{
	m_xmf4x4World = Matrix4x4::Identity();
	m_xmf4x4ToParent = Matrix4x4::Identity();
	SetScale(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z);
}


void CEnemyObject::LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos)
{

	XMFLOAT3 new_pos = pos;
	XMMATRIX inv_mat = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World));	// 역행렬

	new_pos = Vector3::TransformCoord(new_pos, inv_mat); // 타겟의 위치를 적 자체의 좌표계로 변환
	if (Vector3::Length(new_pos) > 0.0001f) {
		new_pos = Vector3::Normalize(new_pos);
	}

	float pitch = XMConvertToDegrees(asin(-new_pos.y));
	float yaw = XMConvertToDegrees(atan2(new_pos.x, new_pos.z));

	float rotate_angle = fTimeElapsed * 90.f;

	XMFLOAT3 p_y_r{ pitch, yaw, 0.f };
	if (Vector3::Length(p_y_r) > rotate_angle) {
		p_y_r = Vector3::Normalize(p_y_r);
		Rotate(p_y_r.x * rotate_angle, p_y_r.y * rotate_angle, 0.f);
	}
	else {
		Rotate(pitch, yaw, 0.f);
	}

	UpdateTransform(NULL);
}


void CEnemyObject::AI(float fTimeElapsed, XMFLOAT3& pl_pos)
{
	switch (state)
	{
	case EnemyState::AIMING:	// 플레이어 방향을 바라보도록 한다
		AimingAI(fTimeElapsed, pl_pos);
		break;
	case EnemyState::MOVE:
		MoveAI(fTimeElapsed, pl_pos);
		break;
	}
}

void CEnemyObject::MoveAI(float fTimeElapsed, XMFLOAT3& pl_pos)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 player_pos = pl_pos;
	XMFLOAT3 destination = Vector3::Add(m_xmf3Destination, player_pos);

	//LookAtPosition(fTimeElapsed, destination);
}

void CEnemyObject::AimingAI(float fTimeElapsed, XMFLOAT3& pl_pos)
{
	XMFLOAT3 player_pos = pl_pos;
	LookAtPosition(fTimeElapsed, player_pos);	// 플레이어를 보도록 회전
	UpdateTransform();
}


//============================================================================================
CInsideShipObject::CInsideShipObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	CLoadedModelInfo* pMeteorModel = pModel;
	if (!pMeteorModel) pMeteorModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/InsideShip.bin", NULL);

	SetChild(pMeteorModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 1, pMeteorModel);
	//m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	//m_pSkinnedAnimationController->SetCallbackKeys(0, 1);

}

CInsideShipObject::~CInsideShipObject()
{
}

//============================================================================================
CBulletObject::CBulletObject(float fEffectiveRange)
{
	m_fBulletEffectiveRange = fEffectiveRange;
}

CBulletObject::CBulletObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	CLoadedModelInfo* pBulletModel = pModel;
	if (!pBulletModel) pBulletModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/cube.bin", NULL);

	/*CcolorShader* pBulletShader = new CcolorShader();
	pBulletShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pBulletShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CMaterial* pBulletMaterial = new CMaterial(1);
	pBulletMaterial->SetShader(pBulletShader);

	pBulletMaterial->m_xmf4AlbedoColor = XMFLOAT4(0.8, 0.8, 0.8, 1);
	pBulletMaterial->m_xmf4EmissiveColor = XMFLOAT4(0.2, 0.2, 0.2, 1);
	pBulletMaterial->m_xmf4AmbientColor = XMFLOAT4(0.8, 0.8, 0.8, 1);

	pBulletModel->m_pModelRootObject->SetMaterial(0, pBulletMaterial);*/

	if (pBulletModel->m_pModelRootObject->m_ppMaterials[0])
	{
		//pBulletModel->m_pModelRootObject->m_ppMaterials[0]->m_xmf4AlbedoColor = XMFLOAT4(1, 1, 1, 1);
		pBulletModel->m_pModelRootObject->m_ppMaterials[0]->m_xmf4EmissiveColor = XMFLOAT4(1,1,1,1);
		pBulletModel->m_pModelRootObject->m_ppMaterials[0]->m_xmf4AmbientColor = XMFLOAT4(1,1,1, 1); 
		pBulletModel->m_pModelRootObject->m_ppMaterials[0]->m_xmf4SpecularColor = XMFLOAT4(1,1,1, 1); 
	}

	SetChild(pBulletModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pBulletModel);

}

void CBulletObject::Animate(float fElapsedTime)
{
	m_fElapsedTimeAfterFire += fElapsedTime;

	float fDistance = m_fMovingSpeed * fElapsedTime;

	if ((m_fElapsedTimeAfterFire > m_fLockingDelayTime))
	{
		m_xmf3Position.x = m_xmf4x4ToParent._41;
		m_xmf3Position.y = m_xmf4x4ToParent._42;
		m_xmf3Position.z = m_xmf4x4ToParent._43;
		
	}
	m_xmf3Position.x = m_xmf4x4ToParent._41;
	m_xmf3Position.y = m_xmf4x4ToParent._42;
	m_xmf3Position.z = m_xmf4x4ToParent._43;

	m_fMovingDistance += fDistance;

	Move(DIR_FORWARD, fDistance);

//	cout << GetPosition().z << endl;
	if ((m_fMovingDistance > m_fBulletEffectiveRange) || (m_fElapsedTimeAfterFire > m_fLockingTime)) Reset();


	CGameObject::Animate(fElapsedTime);
}


void CBulletObject::SetEnemyFire4x4(XMFLOAT3 Player_Position)
{
	//m_fMovingDistance  기반으로 UP, Right, directt 설정해주기 
	/*XMFLOAT3 xmf3Position(m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43);
	XMFLOAT4X4 mtxLookAt = Matrix4x4::LookAtLH(xmf3Position, Player_Position, m_xmf3Up);*/

	m_xmf3Look = Vector3::Normalize(m_xmf3MovingDirection);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
}

void CBulletObject::Move(DWORD dwDirection, float fDistance)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (is_enemy_fire) 
		{
			//cout << m_fPitch<< "   " << m_fYaw << "   " << m_fRoll << endl;
			//cout << m_xmf3Right.x<< "   " << m_xmf3Right.y << "   " << m_xmf3Right.z << endl;
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3MovingDirection, fDistance);
		}
		else
		{
			if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
			if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
			if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
			if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
			if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
			if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);
		}
		Move(xmf3Shift);
	}
}


void CBulletObject::Move(const XMFLOAT3& xmf3Shift)
{
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
	SetPosition(m_xmf3Position);
}

void CBulletObject::Reset()
{
	Rotate(-m_fPitch, -m_fYaw, -m_fRoll);
	m_fPitch = 0; m_fYaw = 0; m_fRoll = 0;
	m_pLockedObject = NULL;
	m_fElapsedTimeAfterFire = 0;
	m_fMovingDistance = 0;
	m_fRotationAngle = 0.0f;

	m_bActive = false;
}

//=======================================

CMissileObject::CMissileObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	CLoadedModelInfo* pBulletModel = pModel;
	if (!pBulletModel) pBulletModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/cube.bin", NULL);

	SetChild(pBulletModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pBulletModel);

}

void CMissileObject::LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos)
{
	XMFLOAT3 new_pos = pos;

	XMMATRIX inv_mat = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World));	// 역행렬

	new_pos = Vector3::TransformCoord(new_pos, inv_mat); // 타겟의 위치를 적 자체의 좌표계로 변환
	if (Vector3::Length(new_pos) > 0.0001f) {
		new_pos = Vector3::Normalize(new_pos);
	}

	float pitch = XMConvertToDegrees(asin(-new_pos.y));
	float yaw = XMConvertToDegrees(atan2(new_pos.x, new_pos.z));

	float rotate_angle = fTimeElapsed * 180.f;

	XMFLOAT3 p_y_r{ pitch, yaw, 0.f };
	if (Vector3::Length(p_y_r) > rotate_angle) {
		p_y_r = Vector3::Normalize(p_y_r);
		Rotate(p_y_r.x * rotate_angle, p_y_r.y * rotate_angle, 0.f);
	}
	else {
		Rotate(pitch, yaw, 0.f);
	}

	UpdateTransform(NULL);
}

void CMissileObject::ResetRotate()
{
	m_xmf4x4World = Matrix4x4::Identity();
	m_xmf4x4ToParent = Matrix4x4::Identity();
}

//================================================

CUIObject::CUIObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
	CSkyBoxMesh* pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 0.0f);
	SetMesh(pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0);
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/crosshair.dds", 0);

	CSkyBoxShader* pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pSkyBoxTexture, 10, false);

	CMaterial* pSkyBoxMaterial = new CMaterial(1);
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture);
	pSkyBoxMaterial->SetShader(pSkyBoxShader);

	SetMaterial(0, pSkyBoxMaterial);



}

void CUIObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	CGameObject::Render(pd3dCommandList, pCamera);
}

//============================================================================================================================

CSpriteObject::CSpriteObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, XMFLOAT3 Center, XMFLOAT3 Size, int type) : CGameObject(1)
{

	CTexturedRectMesh* SpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 50.0f, 50.0f, 0.0f);//Size.x, Size.y, 0.0f);
	SetMesh(SpriteMesh);

	CTexture* pSpriteTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pSpriteTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Model/Textures/Explode_8x8.dds", 0);//Explode_8x8

	CSpriteShader* CSpriteObjectShader = new CSpriteShader();
	
	CSpriteObjectShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	CSpriteObjectShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pSpriteTexture, 18, false); //PS를 UI  18

	CMaterial* pStriteMaterial = new CMaterial(1);
	pStriteMaterial->SetTexture(pSpriteTexture);
	pStriteMaterial->SetShader(CSpriteObjectShader);

	SetRowColumn(8, 8);
	SetSpeed(3.0f / (8 * 8));
	Animate(0.0f);

	SetMaterial(0, pStriteMaterial);
	SpriteMode = type;
	is_Alive = false;

}

void CSpriteObject::Animate(float fElapsedTime)
{
		m_fTime += fElapsedTime * 0.5f;
		if (m_fTime >= m_fSpeed) {
			m_fTime = 0.0f;
		}

		m_xmf4x4Texture._11 = 1.0f / float(m_nRows);
		m_xmf4x4Texture._22 = 1.0f / float(m_nCols);
		m_xmf4x4Texture._31 = float(m_nRow) / float(m_nRows);
		m_xmf4x4Texture._32 = float(m_nCol) / float(m_nCols);
		if (m_fTime == 0.0f)
		{
			if (++m_nCol == m_nCols) { m_nRow++; m_nCol = 0; }
			if (m_nRow == m_nRows) { m_nRow = 0;}// , FullAnimated = true; 
		}
		CGameObject::Animate(fElapsedTime);
}


void CSpriteObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	//cout << m_ppMaterials[0]->m_ppTextures[0]->m_pRootArgumentInfos[0].m_nRootParameterIndex<<endl;
	switch (SpriteMode) {
		case static_cast<int>(SpriteType::Ship):
			//CGameObject::Render(pd3dCommandList, pCamera);
			SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
			break;

		case static_cast<int>(SpriteType::EnemyBoom):
			//CGameObject::Render(pd3dCommandList, pCamera);
			SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f)); 
			//시간 제한 코드 
			CountDiedTime(1.0f);
			break;
		default: break;
	}
	CGameObject::UpdateTransform(NULL);
	CGameObject::Render(pd3dCommandList, pCamera);	
}

void CSpriteObject::CountDiedTime(float dieTime)
{
	if (is_Alive) {
		m_fCntTime += m_fTime;
		if (m_fCntTime >= dieTime) {
			is_Alive = false; //삭제용 
			m_fCntTime = 0.0f;
			TargetNum = -1;
		}
	}

	//cout << m_fCntTime << endl;
}



void CSpriteObject::SetfollowPosition(XMFLOAT3 Target, XMFLOAT3 Distance,XMFLOAT3 LookAt)
{
	//주인공 객체로부터 떨어져야 하는 위치 벡터
	XMFLOAT3 offset = XMFLOAT3(-LookAt.x * Distance.x, -LookAt.y * Distance.y, -LookAt.z * Distance.z);

	// 주인공 객체의 위치 벡터와 계산된 위치 벡터를 더하기
	XMFLOAT3 spaceshipPosition = XMFLOAT3(Target.x + offset.x, Target.y + offset.y, Target.z + offset.z);
	SetPosition(spaceshipPosition);
	//왜이러냐 진짜 
}

void CSpriteObject::SetNewTexture(ID3D12Device* pd3dDevice,CTexture* pSpriteTexture)
{
	//CScene::CreateShaderResourceViews(pd3dDevice, pSpriteTexture, 20, false); //PS를 UI  18
}


void CSpriteObject::CreateShaderVariable(ID3D12Device* pd3dDevice,ID3D12GraphicsCommandList* pd3dCommandList)
{ 
	//only do once time
	m_pcbplusShaderVariable = NULL;
	UINT ncbElementBytes = ((sizeof(CB_PLUS_INFO) + 255) & ~255); //256의 배수
	m_pcbplusShaderVariable = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

	m_pcbplusShaderVariable->Map(0, NULL, (void**)&m_pcbPlusInfo);
}



void CSpriteObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource* m_pd3dcbPlusInfo)
{
	//float fCurrentTime = m_GameTimer.GetTotalTime();
	////std::cout << fCurrentTime << std::endl;
	//m_pd3dCommandList->SetGraphicsRoot32BitConstants(13, 1, &fCurrentTime, 0);

 	//m_pcbPlusInfo->Texture_size = m_xmf4x4Texture;
	XMStoreFloat4x4(&m_pcbPlusInfo->gmtxTexture, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4Texture)));

	//m_pcbPlusInfo->gfCurrentTime = m_fTime;

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbPlusInfo->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(19, d3dGpuVirtualAddress);

}

CMascotObject::CMascotObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	CLoadedModelInfo* pMascotModel = pModel;
	if (!pMascotModel) pMascotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Doggy_V0.bin", NULL);

	SetChild(pMascotModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pMascotModel);

}

CMascotObject::~CMascotObject()
{
}

CBlackHole::CBlackHole(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nWidth, UINT nHeight, UINT nDepth) : CGameObject(1)
{
	CTexturedRectMesh* pUIMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, nWidth, nHeight, nDepth);
	SetMesh(pUIMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_blackholeTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_blackholeTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/blackhole.dds", 0);

	CUIShader* m_blackholeShader;
	CMaterial* m_blackholeMaterial;


	m_blackholeShader = new CUIShader(); //ui쉐이더 쓰고있음 바꾸는거 권장...

	m_blackholeShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_blackholeShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, m_blackholeTexture, 15, false);

	m_blackholeMaterial = new CMaterial(1);
	m_blackholeMaterial->SetTexture(m_blackholeTexture, 0);
	m_blackholeMaterial->SetShader(m_blackholeShader);
	SetMaterial(0, m_blackholeMaterial);
}

void CBlackHole::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}

CBlackHoleMeteorObject::CBlackHoleMeteorObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	CLoadedModelInfo* pMeteorModel = pModel;
	if (!pMeteorModel) pMeteorModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/meteo.bin", NULL);

	SetChild(pMeteorModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pMeteorModel);

	// m_fDistanceFromCenter를 10과 50 사이의 랜덤 값
	m_fDistanceFromCenter = 20.0f + static_cast<float>(rand()) / (RAND_MAX / (150.0f - 20.0f));

	m_fRotationSpeed = 1.0f + static_cast<float>(rand()) / (RAND_MAX / (3.0f - 1.0f));


	// m_xmf3MovingDirection를 랜덤한 방향으로 - 
	float randomAngle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * XM_PI;
	m_xmf3MovingDirection = XMFLOAT3(cosf(randomAngle), 0.0f, sinf(randomAngle));
	m_xmf3MovingDirection = Vector3::Normalize(m_xmf3MovingDirection);
}



void CBlackHoleMeteorObject::Animate(float fTimeElapsed, XMFLOAT3 cPos)
{
	// 공전 중심으로부터의 거리와 방향을 계산합니다.
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Position, cPos);
	float fDistance = m_fDistanceFromCenter;

	// 방향 벡터를 정규화합니다.
	xmf3Direction = Vector3::Normalize(xmf3Direction);

	ffffTimeElapsed += fTimeElapsed;
	// 누적된 fTimeElapsed가 3 이상인 경우 공전 방향을 다시 랜덤으로 변경합니다.
	if (ffffTimeElapsed >= 3.0f)
	{
		float randomAngle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * XM_PI;
		m_xmf3MovingDirection = XMFLOAT3(cosf(randomAngle), 0.0f, sinf(randomAngle));
		m_xmf3MovingDirection = Vector3::Normalize(m_xmf3MovingDirection);
		ffffTimeElapsed = 0.0f;
	}

	// 회전 각도를 계산합니다.
	float fRotationAngle = m_fRotationSpeed * fTimeElapsed;

	// 회전 행렬을 생성합니다.
	XMMATRIX xmRotation = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3MovingDirection), fRotationAngle);

	// 방향 벡터를 회전시킵니다.
	XMVECTOR xmPosition = XMLoadFloat3(&xmf3Direction);
	xmPosition = XMVector3TransformCoord(xmPosition, xmRotation);

	// 회전된 방향 벡터를 거리만큼 곱하여 새 위치를 계산합니다.
	xmPosition = XMVectorScale(xmPosition, fDistance);

	// 중심 위치를 더하여 최종 위치를 얻습니다.
	xmPosition = XMVectorAdd(xmPosition, XMLoadFloat3(&cPos));

	// XMVECTOR를 XMFLOAT3로 변환합니다.
	XMStoreFloat3(&xmf3Position, xmPosition);

	// 위치를 설정합니다.
	SetPosition(xmf3Position);

	CGameObject::Animate(fTimeElapsed);
}
//회전방향randx
//void CBlackHoleMeteorObject::Animate(float fTimeElapsed, XMFLOAT3 cPos)
//{
//	// 공전 중심으로부터의 거리와 방향을 계산합니다.
//	XMFLOAT3 xmf3Position = GetPosition();
//	XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Position, cPos);
//	float fDistance = m_fDistanceFromCenter;
//
//	// 방향 벡터를 정규화합니다.
//	xmf3Direction = Vector3::Normalize(xmf3Direction);
//
//
//	ffffTimeElapsed += fTimeElapsed;
//	// 누적된 fTimeElapsed가 5 이상인 경우 공전 방향을 다시 랜덤으로 변경합니다.
//	if (ffffTimeElapsed >= 3.0f)
//	{
//		float randomAngle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * XM_PI;
//		m_xmf3MovingDirection = XMFLOAT3(cosf(randomAngle), 0.0f, sinf(randomAngle));
//		m_xmf3MovingDirection = Vector3::Normalize(m_xmf3MovingDirection);
//		ffffTimeElapsed = 0.0f;
//	}
//
//	// 회전 각도를 계산합니다.
//	float fRotationAngle = m_fRotationSpeed * fTimeElapsed;
//
//	// 회전 행렬을 생성합니다.
//	XMMATRIX xmRotation = XMMatrixRotationAxis(XMLoadFloat3(&XMFLOAT3(0.0f, 1.0f, 0.0f)), fRotationAngle);
//
//	// 방향 벡터를 회전시킵니다.
//	XMVECTOR xmPosition = XMLoadFloat3(&xmf3Direction);
//	xmPosition = XMVector3TransformCoord(xmPosition, xmRotation);
//
//	// 회전된 방향 벡터를 거리만큼 곱하여 새 위치를 계산합니다.
//	xmPosition = XMVectorScale(xmPosition, fDistance);
//
//	// 중심 위치를 더하여 최종 위치를 얻습니다.
//	xmPosition = XMVectorAdd(xmPosition, XMLoadFloat3(&cPos));
//
//	// XMVECTOR를 XMFLOAT3로 변환합니다.
//	XMStoreFloat3(&xmf3Position, xmPosition);
//
//	// 위치를 설정합니다.
//	SetPosition(xmf3Position);
//
//	CGameObject::Animate(fTimeElapsed);
//}

CJewelObject::CJewelObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	CLoadedModelInfo* pJewelModel = pModel;
	if (!pJewelModel) pJewelModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/SoftStar.bin", NULL);

	SetChild(pJewelModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pJewelModel);

}

void CJewelObject::endingMove(float fElapsedTime, XMFLOAT3 tarpos)
{
	anitime += fElapsedTime;
	if (anitime < 5) {

		m_xmf4x4ToParent._41 += fElapsedTime * 0;
		m_xmf4x4ToParent._42 += fElapsedTime * 50;
		m_xmf4x4ToParent._43 += fElapsedTime * 0;
	}

	if (tarpos.z - m_xmf4x4ToParent._43 > 0) {
		if (anitime >= 5) {

			m_xmf4x4ToParent._41 += fElapsedTime * 0;
			m_xmf4x4ToParent._42 += fElapsedTime * 0;
			m_xmf4x4ToParent._43 -= anitime * 10;
		}
	}
	else {
		if (anitime >= 5) {

			m_xmf4x4ToParent._41 += fElapsedTime * 0;
			m_xmf4x4ToParent._42 += fElapsedTime * 0;
			m_xmf4x4ToParent._43 += anitime * 10;
		}
	}

	Animate(fElapsedTime);
}

//================================
CParticleObject::CParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
	CParticleMesh* pParticleMesh = new CParticleMesh(pd3dDevice, pd3dCommandList, 10.0f, 10.0f, 0.0f);
	SetMesh(pParticleMesh);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pParticleTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pParticleTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/star.dds", 0); //star


	CParticleShader* pParticleShader = new CParticleShader(); //CParticleShader CUIShader
	pParticleShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pParticleShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pParticleTexture, 21, false);

	CMaterial* pParticleMaterial = new CMaterial(1);
	pParticleMaterial->SetTexture(pParticleTexture, 0);
	pParticleMaterial->SetShader(pParticleShader);

	SetMaterial(0, pParticleMaterial);

	velocity = (float)(rand() % 5);

	angleX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;
	angleY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;
	angleZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;

	position.x = GetPosition().x;
	position.y = GetPosition().y;
	position.z = GetPosition().z;
}

void CParticleObject::Animate(float fElapsedTime)
{
	ffTimeElapsed += fElapsedTime;
	if (ffTimeElapsed > lifeTime) {
		isLive = false;
		ffTimeElapsed = 0.f;
	}

	// 랜덤한 방향을 설정하기 위한 각도 계산

	// velocity에 따라 파티클 이동
	position.x += velocity * cosf(angleX) * fElapsedTime * 100;
	position.y += velocity * sinf(angleY) * fElapsedTime * 100;
	position.z += velocity * sinf(angleZ) * fElapsedTime * 100;

	SetPosition(position);

	CGameObject::Animate(fElapsedTime);
}

void CParticleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}

void CParticleObject::setPos(XMFLOAT3 pos)
{
	position.x = pos.x;
	position.y = pos.y;
	position.z = pos.z;
}


//======================
CFireObject::CFireObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
	CFireMesh* pFireMesh = new CFireMesh(pd3dDevice, pd3dCommandList, 20.0f, 30.0f, 0.0f);
	//CTexturedRectMesh* pFireMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 10.0f, 10.0f, 0.0f);
	pFireMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	SetMesh(pFireMesh);

	//CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CFireShader* pFireShader = new CFireShader();
	pFireShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pFireShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);


	CTexture* pfireTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pfireTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"data/fire02.dds", 0); // fire mix color

	CTexture* pnoiseTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pnoiseTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"data/noise01.dds", 0);//fre material texture

	CTexture* palphaTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	palphaTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"data/alpha01.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, pfireTexture, 22, false);
	CScene::CreateShaderResourceViews(pd3dDevice, pnoiseTexture, 23, false);
	CScene::CreateShaderResourceViews(pd3dDevice, palphaTexture, 24, false);




	CMaterial* pFireMaterial = new CMaterial(3);
	pFireMaterial->SetTexture(pfireTexture, 0);
	pFireMaterial->SetTexture(pnoiseTexture, 1);
	pFireMaterial->SetTexture(palphaTexture, 2);



	pFireMaterial->SetShader(pFireShader);


	SetMaterial(0, pFireMaterial);
	m_GameTimer.Start();
	m_GameTimer.Tick(30.0f);
}

void CFireObject::Animate(float fElapsedTime)
{
	CGameObject::Animate(fElapsedTime);
}

void CFireObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	UpdateShaderVariables(pd3dCommandList, m_pcbplusShaderVariable);
	//CGameObject::Render(pd3dCommandList, pCamera);
}

void CFireObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	//only do once time
	m_pcbplusShaderVariable = NULL;
	UINT ncbElementBytes = ((sizeof(CB_PLUS_INFO) + 255) & ~255); //256�� ���
	m_pcbplusShaderVariable = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

	m_pcbplusShaderVariable->Map(0, NULL, (void**)&m_pcbPlusInfo);
}

void CFireObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource* m_pd3dcbPlusInfo)
{
	time +=  m_GameTimer.GetTimeElapsed();
	m_pcbPlusInfo->gfCurrentTime = time;
	if (time >= 1000.0f) time = 0;
	//XMStoreFloat4x4(&m_pcbPlusInfo->gfCurrentTime, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4Texture)));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbPlusInfo->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(19, d3dGpuVirtualAddress);

}

void CFireObject::SetLookAt(XMFLOAT3& xmf3TargetPosition, XMFLOAT3& xmf3Up)
{
	XMFLOAT3 xmf3Position(m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43);
	XMFLOAT4X4 mtxLookAt = Matrix4x4::LookAtLH(xmf3Position, xmf3TargetPosition, xmf3Up);
	m_xmf4x4ToParent._11 = mtxLookAt._11; m_xmf4x4ToParent._12 = mtxLookAt._21; m_xmf4x4ToParent._13 = mtxLookAt._31;
	m_xmf4x4ToParent._21 = mtxLookAt._12; m_xmf4x4ToParent._22 = mtxLookAt._22; m_xmf4x4ToParent._23 = mtxLookAt._32;
	m_xmf4x4ToParent._31 = mtxLookAt._13; m_xmf4x4ToParent._32 = mtxLookAt._23; m_xmf4x4ToParent._33 = mtxLookAt._33;

}

void CFireObject::Rotate180Degrees()
{
	m_xmf4x4ToParent._11 = -m_xmf4x4ToParent._11;
	m_xmf4x4ToParent._22 = -m_xmf4x4ToParent._22;
	m_xmf4x4ToParent._33 = -m_xmf4x4ToParent._33;
}







CFlameParticleObject::CFlameParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
	CParticleMesh* pParticleMesh = new CParticleMesh(pd3dDevice, pd3dCommandList, 150.0f, 150.0f, 0.0f);
	SetMesh(pParticleMesh);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pParticleTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pParticleTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/bluecircle.dds", 0); //star


	CParticleShader* pParticleShader = new CParticleShader(); //CParticleShader CUIShader
	pParticleShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pParticleShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pParticleTexture, 21, false);

	CMaterial* pParticleMaterial = new CMaterial(1);
	pParticleMaterial->SetTexture(pParticleTexture, 0);
	pParticleMaterial->SetShader(pParticleShader);

	SetMaterial(0, pParticleMaterial);

	
	velocity = 10;
	//velocity = (float)(rand() % 5);

	angleX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;
	angleY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;
	angleZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;

	//intervalX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 50.0f;
	//intervalY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 50.0f;
	intervalZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 150.0f;


	position.x = GetPosition().x + intervalX;
	position.y = GetPosition().y + intervalY;
	position.z = GetPosition().z + intervalZ;

	direction = XMVectorSubtract(XMLoadFloat3(&TargetPos), XMLoadFloat3(&position));
	direction = XMVector3Normalize(direction);
}

void CFlameParticleObject::Animate(float fElapsedTime)
{
	ffTimeElapsed += fElapsedTime;
	if (ffTimeElapsed > lifeTime) {
		isLive = false;
		ffTimeElapsed = 0.f;
	}

	// 처음 한 번만 TargetPos로 향하는 방향 벡터 계산
	if (XMVector3Equal(targetDirection, XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f)))
	{
		targetDirection = XMVectorSubtract(XMLoadFloat3(&TargetPos), XMLoadFloat3(&position));
		targetDirection = XMVector3Normalize(targetDirection);
	}

	// velocity에 따라 파티클 이동
	XMVECTOR velocityVector = XMVectorScale(targetDirection, velocity * fElapsedTime * 10);
	XMFLOAT3 velocityResult;
	XMStoreFloat3(&velocityResult, velocityVector);

	position.x += velocityResult.x * 10;
	position.y += velocityResult.y * 10;
	position.z += velocityResult.z * 10;


	SetPosition(position);

	CGameObject::Animate(fElapsedTime);
}

void CFlameParticleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}

void CFlameParticleObject::setPos(XMFLOAT3 pos)
{
	position.x = pos.x + intervalX;
	position.y = pos.y + intervalY;
	position.z = pos.z + intervalZ;
}

ChealParticleObject::ChealParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
	ChealParticleMesh* pParticleMesh = new ChealParticleMesh(pd3dDevice, pd3dCommandList, 150.0f, 150.0f, 0.0f);
	SetMesh(pParticleMesh);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pParticleTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pParticleTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/heal.dds", 0); //star


	CParticleShader* pParticleShader = new CParticleShader(); //CParticleShader CUIShader
	pParticleShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pParticleShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pParticleTexture, 21, false);

	CMaterial* pParticleMaterial = new CMaterial(1);
	pParticleMaterial->SetTexture(pParticleTexture, 0);
	pParticleMaterial->SetShader(pParticleShader);

	SetMaterial(0, pParticleMaterial);

	velocity = (float)(rand() % 5);

	angleX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;
	angleY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;
	angleZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;

	position.x = GetPosition().x;
	position.y = GetPosition().y;
	position.z = GetPosition().z;

}

void ChealParticleObject::Animate(float fElapsedTime)
{
	ffTimeElapsed += fElapsedTime;
	if (ffTimeElapsed > lifeTime) {
		isLive = false;
		ffTimeElapsed = 0.f;
	}

	// 랜덤한 방향을 설정하기 위한 각도 계산

	// velocity에 따라 파티클 이동
	position.x += velocity * cosf(angleX) * fElapsedTime * 200;
	position.y += velocity * sinf(angleY) * fElapsedTime * 200;
	position.z += velocity * sinf(angleZ) * fElapsedTime * 200;

	SetPosition(position);

	CGameObject::Animate(fElapsedTime);
}

void ChealParticleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}

void ChealParticleObject::setPos(XMFLOAT3 pos)
{
	position.x = pos.x;
	position.y = pos.y;
	position.z = pos.z;
}


//===============================
CSkullObject::CSkullObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
	CParticleMesh* pParticleMesh = new CParticleMesh(pd3dDevice, pd3dCommandList, 150.0f, 150.0f, 0.0f);
	SetMesh(pParticleMesh);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pParticleTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pParticleTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/skull.dds", 0); //star


	CParticleShader* pParticleShader = new CParticleShader(); //CParticleShader CUIShader
	pParticleShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pParticleShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pParticleTexture, 21, false);

	CMaterial* pParticleMaterial = new CMaterial(1);
	pParticleMaterial->SetTexture(pParticleTexture, 0);
	pParticleMaterial->SetShader(pParticleShader);

	SetMaterial(0, pParticleMaterial);


	velocity = 10;
	//velocity = (float)(rand() % 5);

	angleX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;
	angleY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;
	angleZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;

	intervalX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 50.0f;
	intervalY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 50.0f;
	intervalZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 150.0f;


	position.x = GetPosition().x + intervalX;
	position.y = GetPosition().y + intervalY;
	position.z = GetPosition().z + intervalZ;

	direction = XMVectorSubtract(XMLoadFloat3(&TargetPos), XMLoadFloat3(&position));
	direction = XMVector3Normalize(direction);
}

void CSkullObject::Animate(float fElapsedTime)
{
	ffTimeElapsed += fElapsedTime;
	if (ffTimeElapsed > lifeTime) {
		isLive = false;
		ffTimeElapsed = 0.f;
	}

	// 처음 한 번만 TargetPos로 향하는 방향 벡터 계산
	if (XMVector3Equal(targetDirection, XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f)))
	{
		targetDirection = XMVectorSubtract(XMLoadFloat3(&TargetPos), XMLoadFloat3(&position));
		targetDirection = XMVector3Normalize(targetDirection);
	}

	// velocity에 따라 파티클 이동
	XMVECTOR velocityVector = XMVectorScale(targetDirection, velocity * fElapsedTime * 10);
	XMFLOAT3 velocityResult;
	XMStoreFloat3(&velocityResult, velocityVector);

	position.x += velocityResult.x * 10;
	position.y += velocityResult.y * 10;
	position.z += velocityResult.z * 10;


	SetPosition(position);

	CGameObject::Animate(fElapsedTime);
}
void CSkullObject::setPos(XMFLOAT3 pos)
{
	position.x = pos.x + intervalX;
	position.y = pos.y + intervalY;
	position.z = pos.z + intervalZ;
}

void CSkullObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}

//===============================
CLineObject::CLineObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
	CParticleMesh* pParticleMesh = new CParticleMesh(pd3dDevice, pd3dCommandList, 150.0f, 150.0f, 0.0f);
	SetMesh(pParticleMesh);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pParticleTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pParticleTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/line.dds", 0); //star


	CParticleShader* pParticleShader = new CParticleShader(); //CParticleShader CUIShader
	pParticleShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pParticleShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pParticleTexture, 21, false);

	CMaterial* pParticleMaterial = new CMaterial(1);
	pParticleMaterial->SetTexture(pParticleTexture, 0);
	pParticleMaterial->SetShader(pParticleShader);

	SetMaterial(0, pParticleMaterial);


	velocity = 10;
	//velocity = (float)(rand() % 5);

	angleX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;
	angleY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;
	angleZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;

	//intervalX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 50.0f;
	//intervalY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 50.0f;
	intervalZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 150.0f;


	position.x = GetPosition().x + intervalX;
	position.y = GetPosition().y + intervalY;
	position.z = GetPosition().z + intervalZ;

	direction = XMVectorSubtract(XMLoadFloat3(&TargetPos), XMLoadFloat3(&position));
	direction = XMVector3Normalize(direction);
}

void CLineObject::Animate(float fElapsedTime)
{
	ffTimeElapsed += fElapsedTime;
	if (ffTimeElapsed > lifeTime) {
		isLive = false;
		ffTimeElapsed = 0.f;
	}



	//if (XMVector3Equal(targetDirection, XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f)))
	//{
	//	targetDirection = XMVectorSubtract(XMLoadFloat3(&TargetPos), XMLoadFloat3(&position));
	//	targetDirection = XMVector3Normalize(targetDirection);
	//}

	//타겟을 향해 계속
	targetDirection = XMVectorSubtract(XMLoadFloat3(&TargetPos), XMLoadFloat3(&position));
	targetDirection = XMVector3Normalize(targetDirection);

	// velocity에 따라 파티클 이동
	XMVECTOR velocityVector = XMVectorScale(targetDirection, velocity * fElapsedTime * 10);
	XMFLOAT3 velocityResult;
	XMStoreFloat3(&velocityResult, velocityVector);

	position.x += velocityResult.x * 10;
	position.y += velocityResult.y * 10;
	position.z += velocityResult.z * 10;


	SetPosition(position);

	CGameObject::Animate(fElapsedTime);
}
void CLineObject::setPos(XMFLOAT3 pos)
{
	position.x = pos.x + intervalX;
	position.y = pos.y + intervalY;
	position.z = pos.z + intervalZ;
}

void CLineObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}


//==============

CMagicCircleObject::CMagicCircleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int cnt, int num) : CGameObject(1)
{
	CMagicCircleMesh* pParticleMesh;

	if(cnt == 0)
		pParticleMesh = new CMagicCircleMesh(pd3dDevice, pd3dCommandList, 300.0f, 300.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	else if (cnt == 1)
		pParticleMesh = new CMagicCircleMesh(pd3dDevice, pd3dCommandList, 300.0f, 300.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	else if (cnt == 2)
		pParticleMesh = new CMagicCircleMesh(pd3dDevice, pd3dCommandList, 300.0f, 300.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	else
		pParticleMesh = new CMagicCircleMesh(pd3dDevice, pd3dCommandList, 300.0f, 300.0f, 0.0f, 1.0f, 0.0f, 1.0f);

	SetMesh(pParticleMesh);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CTexture* pParticleTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	if (num==0)
		pParticleTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/magiccircle.dds", 0); //star
	else
		pParticleTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/magiccircle2.dds", 0); //star



	CParticleShader* pParticleShader = new CParticleShader(); //CParticleShader CUIShader
	pParticleShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pParticleShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pParticleTexture, 21, false);

	CMaterial* pParticleMaterial = new CMaterial(1);
	pParticleMaterial->SetTexture(pParticleTexture, 0);
	pParticleMaterial->SetShader(pParticleShader);

	SetMaterial(0, pParticleMaterial);


	velocity = 10;
	//velocity = (float)(rand() % 5);

	angleX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;
	angleY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;
	angleZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * DirectX::XM_PI;

	//intervalX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 50.0f;
	//intervalY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 50.0f;
	//intervalZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 150.0f;


	position.x = GetPosition().x + intervalX;
	position.y = GetPosition().y + intervalY;
	position.z = GetPosition().z + intervalZ;

	direction = XMVectorSubtract(XMLoadFloat3(&TargetPos), XMLoadFloat3(&position));
	direction = XMVector3Normalize(direction);
}

void CMagicCircleObject::Animate(float fElapsedTime)
{
	ffTimeElapsed += fElapsedTime;
	if (ffTimeElapsed > lifeTime) {
		isLive = false;
		ffTimeElapsed = 0.f;
	}

	if (XMVector3Equal(targetDirection, XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f)))
	{
		targetDirection = XMVectorSubtract(XMLoadFloat3(&TargetPos), XMLoadFloat3(&position));
		targetDirection = XMVector3Normalize(targetDirection);
	}

	// velocity에 따라 파티클 이동
	XMVECTOR velocityVector = XMVectorScale(targetDirection, velocity * fElapsedTime * 10);
	XMFLOAT3 velocityResult;
	XMStoreFloat3(&velocityResult, velocityVector);

	//position.x += velocityResult.x * 10;
	//position.y += velocityResult.y * 10;
	//position.z += velocityResult.z * 10;


	SetPosition(position);

	CGameObject::Animate(fElapsedTime);
}
void CMagicCircleObject::setPos(XMFLOAT3 pos)
{
	position.x = pos.x + intervalX;
	position.y = pos.y + intervalY;
	position.z = pos.z + intervalZ;
}

void CMagicCircleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}