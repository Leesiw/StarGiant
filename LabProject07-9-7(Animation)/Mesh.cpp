//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Mesh.h"
#include "Object.h"

CMesh::CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

CMesh::~CMesh()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();

	if (m_nSubMeshes > 0)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexBuffers[i]) m_ppd3dSubSetIndexBuffers[i]->Release();
			if (m_ppnSubSetIndices[i]) delete[] m_ppnSubSetIndices[i];
		}
		if (m_ppd3dSubSetIndexBuffers) delete[] m_ppd3dSubSetIndexBuffers;
		if (m_pd3dSubSetIndexBufferViews) delete[] m_pd3dSubSetIndexBufferViews;

		if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;
		if (m_ppnSubSetIndices) delete[] m_ppnSubSetIndices;
	}

	if (m_pxmf3Positions) delete[] m_pxmf3Positions;
}

void CMesh::ReleaseUploadBuffers()
{
	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	m_pd3dPositionUploadBuffer = NULL;

	if ((m_nSubMeshes > 0) && m_ppd3dSubSetIndexUploadBuffers)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexUploadBuffers[i]) m_ppd3dSubSetIndexUploadBuffers[i]->Release();
		}
		if (m_ppd3dSubSetIndexUploadBuffers) delete[] m_ppd3dSubSetIndexUploadBuffers;
		m_ppd3dSubSetIndexUploadBuffers = NULL;
	}
}

void CMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
}



void CMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet)
{
	UpdateShaderVariables(pd3dCommandList);

	OnPreRender(pd3dCommandList, NULL);

	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	if((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

void CMesh::ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	UpdateShaderVariables(pd3dCommandList);

	OnPreShadowRender(pd3dCommandList, NULL);

	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

void CMesh::OnPostRender(ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	BYTE *pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapPixels[x + ((m_nLength - 1 - y)*m_nWidth)] = pHeightMapPixels[x + (y*m_nWidth)];
		}
	}

	if (pHeightMapPixels) delete[] pHeightMapPixels;
}

CHeightMapImage::~CHeightMapImage()
{
	if (m_pHeightMapPixels) delete[] m_pHeightMapPixels;
	m_pHeightMapPixels = NULL;
}

XMFLOAT3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;
	float y1 = (float)m_pHeightMapPixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return(xmf3Normal);
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER

float CHeightMapImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / m_xmf3Scale.x;
	fz = fz / m_xmf3Scale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pHeightMapPixels[x + (z*m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z*m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1)*m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1)*m_nWidth)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}

CHeightMapGridMesh::CHeightMapGridMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, void *pContext) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = nWidth * nLength;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf4Colors = new XMFLOAT4[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];
	m_pxmf2TextureCoords1 = new XMFLOAT2[m_nVertices];

	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;
	int cxHeightMap = pHeightMapImage->GetHeightMapWidth();
	int czHeightMap = pHeightMapImage->GetHeightMapLength();

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			fHeight = OnGetHeight(x, z, pContext);
			m_pxmf3Positions[i] = XMFLOAT3((x*m_xmf3Scale.x), fHeight, (z*m_xmf3Scale.z));
			m_pxmf4Colors[i] = Vector4::Add(OnGetColor(x, z, pContext), xmf4Color);
			m_pxmf2TextureCoords0[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
			m_pxmf2TextureCoords1[i] = XMFLOAT2(float(x) / float(m_xmf3Scale.x*0.5f), float(z) / float(m_xmf3Scale.z*0.5f));
			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
		}
	}

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dColorBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4Colors, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dColorUploadBuffer);

	m_d3dColorBufferView.BufferLocation = m_pd3dColorBuffer->GetGPUVirtualAddress();
	m_d3dColorBufferView.StrideInBytes = sizeof(XMFLOAT4);
	m_d3dColorBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;

	m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_pd3dTextureCoord1Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords1, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord1UploadBuffer);

	m_d3dTextureCoord1BufferView.BufferLocation = m_pd3dTextureCoord1Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_nSubMeshes = 1;
	m_pnSubSetIndices = new int[m_nSubMeshes];
	m_ppnSubSetIndices = new UINT*[m_nSubMeshes];

	m_ppd3dSubSetIndexBuffers = new ID3D12Resource*[m_nSubMeshes];
	m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource*[m_nSubMeshes];
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

	m_pnSubSetIndices[0] = ((nWidth * 2)*(nLength - 1)) + ((nLength - 1) - 1);
	m_ppnSubSetIndices[0] = new UINT[m_pnSubSetIndices[0]];

	for (int j = 0, z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			for (int x = 0; x < nWidth; x++)
			{
				if ((x == 0) && (z > 0)) m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
		else
		{
			for (int x = nWidth - 1; x >= 0; x--)
			{
				if (x == (nWidth - 1)) m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
	}

	m_ppd3dSubSetIndexBuffers[0] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[0], sizeof(UINT) * m_pnSubSetIndices[0], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[0]);

	m_pd3dSubSetIndexBufferViews[0].BufferLocation = m_ppd3dSubSetIndexBuffers[0]->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[0];
}

CHeightMapGridMesh::~CHeightMapGridMesh()
{
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
	if (m_pd3dTextureCoord0Buffer) m_pd3dTextureCoord0Buffer->Release();
	if (m_pd3dTextureCoord1Buffer) m_pd3dTextureCoord1Buffer->Release();

	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
	if (m_pxmf2TextureCoords0) delete[] m_pxmf2TextureCoords0;
	if (m_pxmf2TextureCoords1) delete[] m_pxmf2TextureCoords1;
}

void CHeightMapGridMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dColorUploadBuffer) m_pd3dColorUploadBuffer->Release();
	m_pd3dColorUploadBuffer = NULL;

	if (m_pd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
	m_pd3dTextureCoord0UploadBuffer = NULL;

	if (m_pd3dTextureCoord1UploadBuffer) m_pd3dTextureCoord1UploadBuffer->Release();
	m_pd3dTextureCoord1UploadBuffer = NULL;
}


float CHeightMapGridMesh::OnGetHeight(int x, int z, void *pContext)
{
	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;
	BYTE *pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	int nWidth = pHeightMapImage->GetHeightMapWidth();
	float fHeight = pHeightMapPixels[x + (z*nWidth)] * xmf3Scale.y;
	return(fHeight);
}

XMFLOAT4 CHeightMapGridMesh::OnGetColor(int x, int z, void *pContext)
{
	XMFLOAT3 xmf3LightDirection = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	xmf3LightDirection = Vector3::Normalize(xmf3LightDirection);
	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	XMFLOAT4 xmf4IncidentLightColor(0.9f, 0.8f, 0.4f, 1.0f);
	float fScale = Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z + 1), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z + 1), xmf3LightDirection);
	fScale = (fScale / 4.0f) + 0.05f;
	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;
	XMFLOAT4 xmf4Color = Vector4::Multiply(fScale, xmf4IncidentLightColor);
	return(xmf4Color);
}

void CHeightMapGridMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[4] = { m_d3dPositionBufferView, m_d3dColorBufferView, m_d3dTextureCoord0BufferView, m_d3dTextureCoord1BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 4, pVertexBufferViews);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkyBoxMesh::CSkyBoxMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 36;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	// Front Quad (quads point inward)
	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[1] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[2] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[3] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[4] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[5] = XMFLOAT3(+fx, -fx, +fx);
	// Back Quad										
	m_pxmf3Positions[6] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[7] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[8] = XMFLOAT3(+fx, -fx, -fx);
	m_pxmf3Positions[9] = XMFLOAT3(+fx, -fx, -fx);
	m_pxmf3Positions[10] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[11] = XMFLOAT3(-fx, -fx, -fx);
	// Left Quad										
	m_pxmf3Positions[12] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[13] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[14] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[15] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[16] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[17] = XMFLOAT3(-fx, -fx, +fx);
	// Right Quad										
	m_pxmf3Positions[18] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[19] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[20] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[21] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[22] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[23] = XMFLOAT3(+fx, -fx, -fx);
	// Top Quad											
	m_pxmf3Positions[24] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[25] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[26] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[27] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[28] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[29] = XMFLOAT3(+fx, +fx, +fx);
	// Bottom Quad										
	m_pxmf3Positions[30] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[31] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[32] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[33] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[34] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[35] = XMFLOAT3(+fx, -fx, -fx);

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
}

CSkyBoxMesh::~CSkyBoxMesh()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
CStandardMesh::CStandardMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
}

CStandardMesh::~CStandardMesh()
{
	if (m_pd3dTextureCoord0Buffer) m_pd3dTextureCoord0Buffer->Release();
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
	if (m_pd3dTangentBuffer) m_pd3dTangentBuffer->Release();
	if (m_pd3dBiTangentBuffer) m_pd3dBiTangentBuffer->Release();

	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
	if (m_pxmf3Normals) delete[] m_pxmf3Normals;
	if (m_pxmf3Tangents) delete[] m_pxmf3Tangents;
	if (m_pxmf3BiTangents) delete[] m_pxmf3BiTangents;
	if (m_pxmf2TextureCoords0) delete[] m_pxmf2TextureCoords0;
	if (m_pxmf2TextureCoords1) delete[] m_pxmf2TextureCoords1;
}

void CStandardMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
	m_pd3dTextureCoord0UploadBuffer = NULL;

	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	m_pd3dNormalUploadBuffer = NULL;

	if (m_pd3dTangentUploadBuffer) m_pd3dTangentUploadBuffer->Release();
	m_pd3dTangentUploadBuffer = NULL;

	if (m_pd3dBiTangentUploadBuffer) m_pd3dBiTangentUploadBuffer->Release();
	m_pd3dBiTangentUploadBuffer = NULL;
}

void CStandardMesh::LoadMeshFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pInFile)
{
	char pstrToken[64] = { '\0' };
	int nPositions = 0, nColors = 0, nNormals = 0, nTangents = 0, nBiTangents = 0, nTextureCoords = 0, nIndices = 0, nSubMeshes = 0, nSubIndices = 0;

	UINT nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pInFile);

	::ReadStringFromFile(pInFile, m_pstrMeshName);

	for ( ; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);

			m_xmOOBB = BoundingOrientedBox(m_xmf3AABBCenter, m_xmf3AABBExtents, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); //바운딩 박스
		}
		else if (!strcmp(pstrToken, "<Positions>:"))
		{
			nReads = (UINT)::fread(&nPositions, sizeof(int), 1, pInFile);
			if (nPositions > 0)
			{
				m_nType |= VERTEXT_POSITION;
				m_pxmf3Positions = new XMFLOAT3[nPositions];
				nReads = (UINT)::fread(m_pxmf3Positions, sizeof(XMFLOAT3), nPositions, pInFile);

				m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

				m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
				m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Colors>:"))
		{
			nReads = (UINT)::fread(&nColors, sizeof(int), 1, pInFile);
			if (nColors > 0)
			{
				m_nType |= VERTEXT_COLOR;
				m_pxmf4Colors = new XMFLOAT4[nColors];
				nReads = (UINT)::fread(m_pxmf4Colors, sizeof(XMFLOAT4), nColors, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords0>:"))
		{
			nReads = (UINT)::fread(&nTextureCoords, sizeof(int), 1, pInFile);
			if (nTextureCoords > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD0;
				m_pxmf2TextureCoords0 = new XMFLOAT2[nTextureCoords];
				nReads = (UINT)::fread(m_pxmf2TextureCoords0, sizeof(XMFLOAT2), nTextureCoords, pInFile);

				m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

				m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
				m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords1>:"))
		{
			nReads = (UINT)::fread(&nTextureCoords, sizeof(int), 1, pInFile);
			if (nTextureCoords > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD1;
				m_pxmf2TextureCoords1 = new XMFLOAT2[nTextureCoords];
				nReads = (UINT)::fread(m_pxmf2TextureCoords1, sizeof(XMFLOAT2), nTextureCoords, pInFile);

				m_pd3dTextureCoord1Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords1, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord1UploadBuffer);

				m_d3dTextureCoord1BufferView.BufferLocation = m_pd3dTextureCoord1Buffer->GetGPUVirtualAddress();
				m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			nReads = (UINT)::fread(&nNormals, sizeof(int), 1, pInFile);
			if (nNormals > 0)
			{
				m_nType |= VERTEXT_NORMAL;
				m_pxmf3Normals = new XMFLOAT3[nNormals];
				nReads = (UINT)::fread(m_pxmf3Normals, sizeof(XMFLOAT3), nNormals, pInFile);

				m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

				m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
				m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Tangents>:"))
		{
			nReads = (UINT)::fread(&nTangents, sizeof(int), 1, pInFile);
			if (nTangents > 0)
			{
				m_nType |= VERTEXT_TANGENT;
				m_pxmf3Tangents = new XMFLOAT3[nTangents];
				nReads = (UINT)::fread(m_pxmf3Tangents, sizeof(XMFLOAT3), nTangents, pInFile);

				m_pd3dTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Tangents, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTangentUploadBuffer);

				m_d3dTangentBufferView.BufferLocation = m_pd3dTangentBuffer->GetGPUVirtualAddress();
				m_d3dTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<BiTangents>:"))
		{
			nReads = (UINT)::fread(&nBiTangents, sizeof(int), 1, pInFile);
			if (nBiTangents > 0)
			{
				m_pxmf3BiTangents = new XMFLOAT3[nBiTangents];
				nReads = (UINT)::fread(m_pxmf3BiTangents, sizeof(XMFLOAT3), nBiTangents, pInFile);

				m_pd3dBiTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3BiTangents, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBiTangentUploadBuffer);

				m_d3dBiTangentBufferView.BufferLocation = m_pd3dBiTangentBuffer->GetGPUVirtualAddress();
				m_d3dBiTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dBiTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			nReads = (UINT)::fread(&(m_nSubMeshes), sizeof(int), 1, pInFile);
			if (m_nSubMeshes > 0)
			{
				m_pnSubSetIndices = new int[m_nSubMeshes];
				m_ppnSubSetIndices = new UINT*[m_nSubMeshes];

				m_ppd3dSubSetIndexBuffers = new ID3D12Resource*[m_nSubMeshes];
				m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource*[m_nSubMeshes];
				m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

				for (int i = 0; i < m_nSubMeshes; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<SubMesh>:"))
					{
						int nIndex = 0;
						nReads = (UINT)::fread(&nIndex, sizeof(int), 1, pInFile); //i
						nReads = (UINT)::fread(&(m_pnSubSetIndices[i]), sizeof(int), 1, pInFile);
						if (m_pnSubSetIndices[i] > 0)
						{
							m_ppnSubSetIndices[i] = new UINT[m_pnSubSetIndices[i]];
							nReads = (UINT)::fread(m_ppnSubSetIndices[i], sizeof(UINT), m_pnSubSetIndices[i], pInFile);

							m_ppd3dSubSetIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[i]);

							m_pd3dSubSetIndexBufferViews[i].BufferLocation = m_ppd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
							m_pd3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
							m_pd3dSubSetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[i];
						}
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "</Mesh>"))
		{
			break;
		}
	}
}

void CStandardMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[5] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView, m_d3dNormalBufferView, m_d3dTangentBufferView, m_d3dBiTangentBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 5, pVertexBufferViews);
}

void CStandardMesh::OnPreShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dNormalBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkinnedMesh::CSkinnedMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : CStandardMesh(pd3dDevice, pd3dCommandList)
{
}

CSkinnedMesh::~CSkinnedMesh()
{
	if (m_pxmn4BoneIndices) delete[] m_pxmn4BoneIndices;
	if (m_pxmf4BoneWeights) delete[] m_pxmf4BoneWeights;

	if (m_ppSkinningBoneFrameCaches) delete[] m_ppSkinningBoneFrameCaches;
	if (m_ppstrSkinningBoneNames) delete[] m_ppstrSkinningBoneNames;

	if (m_pxmf4x4BindPoseBoneOffsets) delete[] m_pxmf4x4BindPoseBoneOffsets;
	if (m_pd3dcbBindPoseBoneOffsets) m_pd3dcbBindPoseBoneOffsets->Release();

	if (m_pd3dBoneIndexBuffer) m_pd3dBoneIndexBuffer->Release();
	if (m_pd3dBoneWeightBuffer) m_pd3dBoneWeightBuffer->Release();

	ReleaseShaderVariables();
}

void CSkinnedMesh::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CSkinnedMesh::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pd3dcbBindPoseBoneOffsets)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneOffsetsGpuVirtualAddress = m_pd3dcbBindPoseBoneOffsets->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(11, d3dcbBoneOffsetsGpuVirtualAddress); //Skinned Bone Offsets
	}

	if (m_pd3dcbSkinningBoneTransforms)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneTransformsGpuVirtualAddress = m_pd3dcbSkinningBoneTransforms->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(12, d3dcbBoneTransformsGpuVirtualAddress); //Skinned Bone Transforms

		for (int j = 0; j < m_nSkinningBones; j++)
		{
			XMStoreFloat4x4(&m_pcbxmf4x4MappedSkinningBoneTransforms[j], XMMatrixTranspose(XMLoadFloat4x4(&m_ppSkinningBoneFrameCaches[j]->m_xmf4x4World)));
		}
	}
}

void CSkinnedMesh::ReleaseShaderVariables()
{
}

void CSkinnedMesh::ReleaseUploadBuffers()
{
	CStandardMesh::ReleaseUploadBuffers();

	if (m_pd3dBoneIndexUploadBuffer) m_pd3dBoneIndexUploadBuffer->Release();
	m_pd3dBoneIndexUploadBuffer = NULL;

	if (m_pd3dBoneWeightUploadBuffer) m_pd3dBoneWeightUploadBuffer->Release();
	m_pd3dBoneWeightUploadBuffer = NULL;
}

void CSkinnedMesh::PrepareSkinning(CGameObject *pModelRootObject)
{
	for (int j = 0; j < m_nSkinningBones; j++)
	{
		m_ppSkinningBoneFrameCaches[j] = pModelRootObject->FindFrame(m_ppstrSkinningBoneNames[j]);
	}
}

void CSkinnedMesh::LoadSkinInfoFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pInFile)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	::ReadStringFromFile(pInFile, m_pstrMeshName); 

	for ( ; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<BonesPerVertex>:"))
		{
			m_nBonesPerVertex = ::ReadIntegerFromFile(pInFile);
		}
		else if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<BoneNames>:"))
		{
			m_nSkinningBones = ::ReadIntegerFromFile(pInFile);
			if (m_nSkinningBones > 0) 
			{
				m_ppstrSkinningBoneNames = new char[m_nSkinningBones][64];
				m_ppSkinningBoneFrameCaches = new CGameObject*[m_nSkinningBones];
				for (int i = 0; i < m_nSkinningBones; i++)
				{
					::ReadStringFromFile(pInFile, m_ppstrSkinningBoneNames[i]);
					m_ppSkinningBoneFrameCaches[i] = NULL;
				}
			}
		}
		else if (!strcmp(pstrToken, "<BoneOffsets>:"))
		{
			m_nSkinningBones = ::ReadIntegerFromFile(pInFile);
			if (m_nSkinningBones > 0)
			{
				m_pxmf4x4BindPoseBoneOffsets = new XMFLOAT4X4[m_nSkinningBones];
				nReads = (UINT)::fread(m_pxmf4x4BindPoseBoneOffsets, sizeof(XMFLOAT4X4), m_nSkinningBones, pInFile);

				UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수
				m_pd3dcbBindPoseBoneOffsets = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
				m_pd3dcbBindPoseBoneOffsets->Map(0, NULL, (void **)&m_pcbxmf4x4MappedBindPoseBoneOffsets);

				for (int i = 0; i < m_nSkinningBones; i++)
				{
					XMStoreFloat4x4(&m_pcbxmf4x4MappedBindPoseBoneOffsets[i], XMMatrixTranspose(XMLoadFloat4x4(&m_pxmf4x4BindPoseBoneOffsets[i])));
				}
			}
		}
		else if (!strcmp(pstrToken, "<BoneIndices>:"))
		{
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			m_nVertices = ::ReadIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				m_pxmn4BoneIndices = new XMINT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmn4BoneIndices, sizeof(XMINT4), m_nVertices, pInFile);
				m_pd3dBoneIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmn4BoneIndices, sizeof(XMINT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneIndexUploadBuffer);

				m_d3dBoneIndexBufferView.BufferLocation = m_pd3dBoneIndexBuffer->GetGPUVirtualAddress();
				m_d3dBoneIndexBufferView.StrideInBytes = sizeof(XMINT4);
				m_d3dBoneIndexBufferView.SizeInBytes = sizeof(XMINT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<BoneWeights>:"))
		{
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			m_nVertices = ::ReadIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				m_pxmf4BoneWeights = new XMFLOAT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmf4BoneWeights, sizeof(XMFLOAT4), m_nVertices, pInFile);
				m_pd3dBoneWeightBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4BoneWeights, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneWeightUploadBuffer);

				m_d3dBoneWeightBufferView.BufferLocation = m_pd3dBoneWeightBuffer->GetGPUVirtualAddress();
				m_d3dBoneWeightBufferView.StrideInBytes = sizeof(XMFLOAT4);
				m_d3dBoneWeightBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "</SkinningInfo>"))
		{
			break;
		}
	}
}

void CSkinnedMesh::OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList, void *pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[7] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView, m_d3dNormalBufferView, m_d3dTangentBufferView, m_d3dBiTangentBufferView, m_d3dBoneIndexBufferView, m_d3dBoneWeightBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 7, pVertexBufferViews);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTexturedRectMesh::CTexturedRectMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth, float fxPosition, float fyPosition, float fzPosition) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 6;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];

	float fx = (fWidth * 0.5f) + fxPosition, fy = (fHeight * 0.5f) + fyPosition, fz = (fDepth * 0.5f) + fzPosition;

	if (fWidth == 0.0f)
	{
		if (fxPosition > 0.0f)
		{
			m_pxmf3Positions[0] = XMFLOAT3(fx, +fy, -fz); m_pxmf2TextureCoords0[0] = XMFLOAT2(1.0f, 0.0f);
			m_pxmf3Positions[1] = XMFLOAT3(fx, -fy, -fz); m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 1.0f);
			m_pxmf3Positions[2] = XMFLOAT3(fx, -fy, +fz); m_pxmf2TextureCoords0[2] = XMFLOAT2(0.0f, 1.0f);
			m_pxmf3Positions[3] = XMFLOAT3(fx, -fy, +fz); m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 1.0f);
			m_pxmf3Positions[4] = XMFLOAT3(fx, +fy, +fz); m_pxmf2TextureCoords0[4] = XMFLOAT2(0.0f, 0.0f);
			m_pxmf3Positions[5] = XMFLOAT3(fx, +fy, -fz); m_pxmf2TextureCoords0[5] = XMFLOAT2(1.0f, 0.0f);
		}
		else
		{
			m_pxmf3Positions[0] = XMFLOAT3(fx, +fy, +fz); m_pxmf2TextureCoords0[0] = XMFLOAT2(1.0f, 0.0f);
			m_pxmf3Positions[1] = XMFLOAT3(fx, -fy, +fz); m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 1.0f);
			m_pxmf3Positions[2] = XMFLOAT3(fx, -fy, -fz); m_pxmf2TextureCoords0[2] = XMFLOAT2(0.0f, 1.0f);
			m_pxmf3Positions[3] = XMFLOAT3(fx, -fy, -fz); m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 1.0f);
			m_pxmf3Positions[4] = XMFLOAT3(fx, +fy, -fz); m_pxmf2TextureCoords0[4] = XMFLOAT2(0.0f, 0.0f);
			m_pxmf3Positions[5] = XMFLOAT3(fx, +fy, +fz); m_pxmf2TextureCoords0[5] = XMFLOAT2(1.0f, 0.0f);
		}
	}
	else if (fHeight == 0.0f)
	{
		if (fyPosition > 0.0f)
		{
			m_pxmf3Positions[0] = XMFLOAT3(+fx, fy, -fz); m_pxmf2TextureCoords0[0] = XMFLOAT2(1.0f, 0.0f);
			m_pxmf3Positions[1] = XMFLOAT3(+fx, fy, +fz); m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 1.0f);
			m_pxmf3Positions[2] = XMFLOAT3(-fx, fy, +fz); m_pxmf2TextureCoords0[2] = XMFLOAT2(0.0f, 1.0f);
			m_pxmf3Positions[3] = XMFLOAT3(-fx, fy, +fz); m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 1.0f);
			m_pxmf3Positions[4] = XMFLOAT3(-fx, fy, -fz); m_pxmf2TextureCoords0[4] = XMFLOAT2(0.0f, 0.0f);
			m_pxmf3Positions[5] = XMFLOAT3(+fx, fy, -fz); m_pxmf2TextureCoords0[5] = XMFLOAT2(1.0f, 0.0f);
		}
		else
		{
			m_pxmf3Positions[0] = XMFLOAT3(+fx, fy, +fz); m_pxmf2TextureCoords0[0] = XMFLOAT2(1.0f, 0.0f);
			m_pxmf3Positions[1] = XMFLOAT3(+fx, fy, -fz); m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 1.0f);
			m_pxmf3Positions[2] = XMFLOAT3(-fx, fy, -fz); m_pxmf2TextureCoords0[2] = XMFLOAT2(0.0f, 1.0f);
			m_pxmf3Positions[3] = XMFLOAT3(-fx, fy, -fz); m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 1.0f);
			m_pxmf3Positions[4] = XMFLOAT3(-fx, fy, +fz); m_pxmf2TextureCoords0[4] = XMFLOAT2(0.0f, 0.0f);
			m_pxmf3Positions[5] = XMFLOAT3(+fx, fy, +fz); m_pxmf2TextureCoords0[5] = XMFLOAT2(1.0f, 0.0f);
		}
	}
	else if (fDepth == 0.0f)
	{
		if (fzPosition > 0.0f)
		{
			m_pxmf3Positions[0] = XMFLOAT3(+fx, +fy, fz); m_pxmf2TextureCoords0[0] = XMFLOAT2(1.0f, 0.0f);
			m_pxmf3Positions[1] = XMFLOAT3(+fx, -fy, fz); m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 1.0f);
			m_pxmf3Positions[2] = XMFLOAT3(-fx, -fy, fz); m_pxmf2TextureCoords0[2] = XMFLOAT2(0.0f, 1.0f);
			m_pxmf3Positions[3] = XMFLOAT3(-fx, -fy, fz); m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 1.0f);
			m_pxmf3Positions[4] = XMFLOAT3(-fx, +fy, fz); m_pxmf2TextureCoords0[4] = XMFLOAT2(0.0f, 0.0f);
			m_pxmf3Positions[5] = XMFLOAT3(+fx, +fy, fz); m_pxmf2TextureCoords0[5] = XMFLOAT2(1.0f, 0.0f);
		}
		else
		{
			m_pxmf3Positions[0] = XMFLOAT3(-fx, +fy, fz); m_pxmf2TextureCoords0[0] = XMFLOAT2(1.0f, 0.0f);
			m_pxmf3Positions[1] = XMFLOAT3(-fx, -fy, fz); m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 1.0f);
			m_pxmf3Positions[2] = XMFLOAT3(+fx, -fy, fz); m_pxmf2TextureCoords0[2] = XMFLOAT2(0.0f, 1.0f);
			m_pxmf3Positions[3] = XMFLOAT3(+fx, -fy, fz); m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 1.0f);
			m_pxmf3Positions[4] = XMFLOAT3(+fx, +fy, fz); m_pxmf2TextureCoords0[4] = XMFLOAT2(0.0f, 0.0f);
			m_pxmf3Positions[5] = XMFLOAT3(-fx, +fy, fz); m_pxmf2TextureCoords0[5] = XMFLOAT2(1.0f, 0.0f);
		}
	}

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
}

CTexturedRectMesh::~CTexturedRectMesh()
{
	if (m_pd3dTextureCoord0Buffer) m_pd3dTextureCoord0Buffer->Release();
	if (m_pxmf2TextureCoords0) delete[] m_pxmf2TextureCoords0;
}

void CTexturedRectMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
	m_pd3dTextureCoord0UploadBuffer = NULL;
}

void CTexturedRectMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);

	pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
}
//--------------------------------------------------------------------------------------------------

CRayLineMesh::CRayLineMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 Vector, float fWidth, float fHeight, float fDepth, float fxPosition, float fyPosition, float fzPosition) : CMesh(pd3dDevice, pd3dCommandList)
{
	int m_nNumber = 30;//사각형 각 변을 이만큼 쪼개겟다. 
	m_nVertices = 6 * m_nNumber * m_nNumber; //사각형에서 라인을 100개씩 그리려고한다. (10x10 인듯? 그럼) 
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//중심의 좌표가 float fxPosition, float fyPosition, float fzPosition
	//사각형의 너비가 float fWidth, float fHeight, float fDepth
	//float fDepth가 그려야할 길이라고 설정한다. 

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];
	//m_pxmf2Vector = new XMFLOAT3[m_nVertices];

	/*	XMVECTOR dirVector = XMLoadFloat3(&Vector);
	XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);  // 임의의 위쪽 벡터
	upVector = XMVector3Cross(dirVector, upVector);  // 방향 벡터와 위쪽 벡터의 외적 계산
	upVector = XMVector3Normalize(upVector);  // 벡터 정규화
	XMFLOAT3 perpendicular;
	XMStoreFloat3(&perpendicular, upVector);

	float fx = Vector3::ScalarProduct(perpendicular, -(fWidth * 0.5f)).x + fxPosition, fy = Vector3::ScalarProduct(perpendicular, -(fHeight * 0.5f)).y + fyPosition; //시작점
	float fz = Vector3::ScalarProduct(XMFLOAT3(fxPosition, fyPosition, fzPosition), fDepth).z; //끝점 //fzPosition +Vector3::ScalarProduct(Vector, fDepth).z;
	*/

	float fx = -(fWidth * 0.5f) + fxPosition, fz = -(fHeight * 0.5f) + fzPosition; //시작점
	float fy = fyPosition + Vector3::ScalarProduct(XMFLOAT3(fxPosition, fyPosition, fzPosition), fDepth).y; //끝점 //
	float dx = fWidth / m_nNumber; //윗변 크기
	float dy = fHeight / m_nNumber;//아랫변 크기 

	//시작점인듯 
	int num = 0;
	//for (int i = 0; i < m_nNumber; i++) {
	//	for (int z = 0; z < m_nNumber; z++) {
	//		m_pxmf3Positions[num] = XMFLOAT3(Vector3::ScalarProduct(Vector, fx + (dx * i)).x,  Vector3::ScalarProduct(Vector,fzPosition).y,Vector3::ScalarProduct(Vector, fz + (dy * z)).z); //시작선분점
	//							m_pxmf2TextureCoords0[num] = XMFLOAT2((1.0 / m_nNumber * i), (1.0 / m_nNumber * z)); num++; //m_pxmf2Vector[i] = XMFLOAT3();
	//		m_pxmf3Positions[num] = XMFLOAT3(Vector3::ScalarProduct(Vector, fx + (dx * i)).x, Vector3::ScalarProduct(Vector, fy).y, Vector3::ScalarProduct(Vector, fz + (dy * z)).z); //끝선분점 
	//							m_pxmf2TextureCoords0[num] = XMFLOAT2((1.0 / m_nNumber * i), (1.0 / m_nNumber * z)); num++; //m_pxmf2Vector[i] = XMFLOAT3(); ;
	//	}
	//}
	for (int i = 0; i < m_nNumber; i++) {
		for (int z = 0; z < m_nNumber; z++) {
			XMFLOAT3 A = XMFLOAT3(Vector3::ScalarProduct(Vector, fx + (dx * i)).x, Vector3::ScalarProduct(Vector, fzPosition).y, Vector3::ScalarProduct(Vector, fz + (dy * z)).z); //시작선분점
			XMFLOAT3 B = XMFLOAT3(Vector3::ScalarProduct(Vector, fx + (dx * i)).x, Vector3::ScalarProduct(Vector, fy).y, Vector3::ScalarProduct(Vector, fz + (dy * z)).z); //끝선분점 
			XMFLOAT2 ATexture = XMFLOAT2((1.0 / m_nNumber * i), (1.0 / m_nNumber * z)); 
			XMFLOAT2 BTexture = XMFLOAT2((1.0 / m_nNumber * i), (1.0 / m_nNumber * z)); 
			//LB 0, LT 1, RB 2, RT 3 
			m_pxmf3Positions[num] = XMFLOAT3(B.x-dy,B.y,B.z);
				m_pxmf2TextureCoords0[num] = BTexture; num++; //0

				m_pxmf3Positions[num] = XMFLOAT3(A.x - dx, A.y, A.z);
				m_pxmf2TextureCoords0[num] = ATexture; num++;//1

				m_pxmf3Positions[num] = XMFLOAT3(B.x + dy, B.y, B.z);
				m_pxmf2TextureCoords0[num] = BTexture; num++;//2

				m_pxmf3Positions[num] = XMFLOAT3(A.x - dx, A.y, A.z);
				m_pxmf2TextureCoords0[num] = ATexture; num++;//1

				m_pxmf3Positions[num] = XMFLOAT3(A.x + dx, A.y, A.z);
				m_pxmf2TextureCoords0[num] = ATexture; num++;//3

				m_pxmf3Positions[num] = XMFLOAT3(B.x + dy, B.y, B.z);
				m_pxmf2TextureCoords0[num] = BTexture; num++;//2
		}
	}


	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
}
CRayLineMesh::CRayLineMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 Vector, float fWidth, float fHeight, float fDepth, XMFLOAT3 Position) : CMesh(pd3dDevice, pd3dCommandList)
{
	int m_nNumber = 30;//사각형 각 변을 이만큼 쪼개겟다. 
	m_nVertices = 6 * m_nNumber * m_nNumber; //사각형에서 라인을 100개씩 그리려고한다. (10x10 인듯? 그럼) 
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//중심의 좌표가 float fxPosition, float fyPosition, float fzPosition
	//사각형의 너비가 float fWidth, float fHeight, float fDepth
	//float fDepth가 그려야할 길이라고 설정한다. 

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];
	m_pxmf2Vector = new XMFLOAT3[m_nVertices];


	float fx = -(fWidth * 0.5f) + Position.x, fz = -(fWidth * 0.5f) + Position.z; //시작점
	float fy = Position.y;
	float dx = fWidth / m_nNumber, dz = fWidth / m_nNumber;

	XMFLOAT3 nPosition =Vector3::Add(Position, Vector3::ScalarProduct(Vector, fDepth));
	float nfx = -(fHeight * 0.5) + nPosition.x, nfz = -(fHeight * 0.5) + nPosition.z;
	float nfy = nPosition.y;
	float ndx = fHeight / m_nNumber, ndz = fHeight / m_nNumber;

	////시작점인듯 
	int num = 0;
	//for (int i = 0; i < m_nNumber; i++) {
	//	for (int z = 0; z < m_nNumber; z++) {
	//		m_pxmf3Positions[num] = XMFLOAT3(fx + dx * i, fy, fz + dz * z);
	//		m_pxmf2TextureCoords0[num] = XMFLOAT2((1.0 / m_nNumber * i), (1.0 / m_nNumber * z)); num++; //m_pxmf2Vector[i] = XMFLOAT3();
	//		m_pxmf3Positions[num] = XMFLOAT3(nfx + ndx * i, nfy, nfz + ndz * z);
	//		m_pxmf2TextureCoords0[num] = XMFLOAT2((1.0 / m_nNumber * i), (1.0 / m_nNumber * z)); num++; //m_pxmf2Vector[i] = XMFLOAT3(); ;
	//	}
	//}

	for (int i = 0; i < m_nNumber; i++) {
		for (int z = 0; z < m_nNumber; z++) {
			XMFLOAT3 A = XMFLOAT3(fx + dx * i, fy, fz + dz * z); //시작선분점
			XMFLOAT3 B = XMFLOAT3(nfx + ndx * i, nfy, nfz + ndz * z); //끝선분점 
			XMFLOAT2 ATexture = XMFLOAT2((1.0 / m_nNumber * i), (1.0 / m_nNumber * z));
			XMFLOAT2 BTexture = XMFLOAT2((1.0 / m_nNumber * i), (1.0 / m_nNumber * z));
			//LB 0, LT 1, RB 2, RT 3 
			m_pxmf3Positions[num] = XMFLOAT3(B.x - ndz, B.y, B.z);
			m_pxmf2TextureCoords0[num] = BTexture; num++; //0

			m_pxmf3Positions[num] = XMFLOAT3(A.x - ndx, A.y, A.z);
			m_pxmf2TextureCoords0[num] = ATexture; num++;//1

			m_pxmf3Positions[num] = XMFLOAT3(B.x + ndz, B.y, B.z);
			m_pxmf2TextureCoords0[num] = BTexture; num++;//2

			m_pxmf3Positions[num] = XMFLOAT3(A.x - ndx, A.y, A.z);
			m_pxmf2TextureCoords0[num] = ATexture; num++;//1

			m_pxmf3Positions[num] = XMFLOAT3(A.x + ndx, A.y, A.z);
			m_pxmf2TextureCoords0[num] = ATexture; num++;//3

			m_pxmf3Positions[num] = XMFLOAT3(B.x + ndz, B.y, B.z);
			m_pxmf2TextureCoords0[num] = BTexture; num++;//2
		}
	}

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
}
CRayLineMesh::~CRayLineMesh()
{
	if (m_pd3dTextureCoord0Buffer) m_pd3dTextureCoord0Buffer->Release();
	if (m_pxmf2TextureCoords0) delete[] m_pxmf2TextureCoords0;
}

void CRayLineMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
	m_pd3dTextureCoord0UploadBuffer = NULL;
}

void CRayLineMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	 
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);

	pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
}





CParticleMesh::CParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	int index = 0;

	m_particleList = new ParticleType[1];
	m_currentParticleCount = 1;

	m_pxmf3Positions = new XMFLOAT3[6];
	color = new XMFLOAT4[6];
	texture = new XMFLOAT2[6];

	//SetParticle(m_currentParticleCount);

	//위치 이상해서 rect 가져다 써봤음 => 되네..?
	float fxPosition = 0, fyPosition = 0, fzPosition = 0;
	float fx = (fWidth * 0.5f) + fxPosition, fy = (fHeight * 0.5f) + fyPosition, fz = (fDepth * 0.5f) + fzPosition;

	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fy, fz);
	m_pxmf3Positions[1] = XMFLOAT3(-fx, -fy, fz);
	m_pxmf3Positions[2] = XMFLOAT3(+fx, -fy, fz);
	m_pxmf3Positions[3] = XMFLOAT3(+fx, -fy, fz);
	m_pxmf3Positions[4] = XMFLOAT3(+fx, +fy, fz);
	m_pxmf3Positions[5] = XMFLOAT3(-fx, +fy, fz);

	texture[0] = XMFLOAT2(1.0f, 0.0f);
	texture[1] = XMFLOAT2(1.0f, 1.0f);
	texture[2] = XMFLOAT2(0.0f, 1.0f);
	texture[3] = XMFLOAT2(0.0f, 1.0f);
	texture[4] = XMFLOAT2(0.0f, 0.0f);
	texture[5] = XMFLOAT2(1.0f, 0.0f);


	float red = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
	float green = 0.0f;
	float blue = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
	
	//float green = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
	//float blue = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;


	color[0] = XMFLOAT4(red, green, blue, 1.0f);
	color[1] = XMFLOAT4(red, green, blue, 1.0f);
	color[2] = XMFLOAT4(red, green, blue, 1.0f);
	color[3] = XMFLOAT4(red, green, blue, 1.0f);
	color[4] = XMFLOAT4(red, green, blue, 1.0f);
	color[5] = XMFLOAT4(red, green, blue, 1.0f);





	m_nVertices = 6;

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dTextureBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, texture, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureUploadBuffer);

	m_d3dTextureBufferView.BufferLocation = m_pd3dTextureBuffer->GetGPUVirtualAddress();
	m_d3dTextureBufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureBufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_pd3dColorBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, color, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dColoUploadBuffer);

	m_d3dColorBufferView.BufferLocation = m_pd3dColorBuffer->GetGPUVirtualAddress();
	m_d3dColorBufferView.StrideInBytes = sizeof(XMFLOAT4);
	m_d3dColorBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;

}

CParticleMesh::~CParticleMesh()
{
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
	if (m_pd3dTextureBuffer) m_pd3dTextureBuffer->Release();
}

void CParticleMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();
	if (m_pd3dTextureUploadBuffer) m_pd3dTextureUploadBuffer->Release();
	m_pd3dTextureUploadBuffer = NULL;
	if (m_pd3dColoUploadBuffer) m_pd3dColoUploadBuffer->Release();
	m_pd3dColoUploadBuffer = NULL;
}

void CParticleMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[3] = { m_d3dPositionBufferView, m_d3dTextureBufferView, m_d3dColorBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 3, pVertexBufferViews);
}

void CParticleMesh::SetParticle(int index)
{
	m_particleSize = 0.2;

	m_particleDeviationX = 0.5f;
	m_particleDeviationY = 0.1f;
	m_particleDeviationZ = 2.0f;

	for (int i = 0; i < index; ++i)
	{
		float positionX = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationX;
		float positionY = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationY;
		float positionZ = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationZ;
		float red = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
		float green = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
		float blue = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;

		//m_particleList[i].positionX = positionX;
		//m_particleList[i].positionY = positionY;
		//m_particleList[i].positionZ = positionZ;



		m_particleList[i].red = red;
		m_particleList[i].green = green;
		m_particleList[i].blue = blue;
	}

}

CFireMesh::CFireMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_noiseb = new NoiseBufferType;
	::ZeroMemory(m_noiseb, sizeof(NoiseBufferType));

	m_distortionb = new DistortionBufferType;
	::ZeroMemory(m_distortionb, sizeof(DistortionBufferType));

	m_noiseb->frameTime = 0;
	m_noiseb->scrollSpeeds = XMFLOAT3(1.3f, 2.1f, 2.3f);
	m_noiseb->scales = XMFLOAT3(1.0f, 2.0f, 3.0f);
	m_noiseb->padding = 0;

	m_distortionb->distortion1 = XMFLOAT2(0.1f, 0.2f);
	m_distortionb->distortion2 = XMFLOAT2(0.1f, 0.3f);
	m_distortionb->distortion3 = XMFLOAT2(0.1f, 0.1f);
	m_distortionb->distortionScale = 0.8f;
	m_distortionb->distortionBias = 0.5f;


	

	cout << "CFireMesh\n";
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pxmf3Positions = new XMFLOAT3[6];
	texture = new XMFLOAT2[6];

	float fxPosition = 0, fyPosition = 0, fzPosition = 0;
	float fx = (fWidth * 0.5f) + fxPosition, fy = (fHeight * 0.5f) + fyPosition, fz = (fDepth * 0.5f) + fzPosition;

	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fy, fz);
	m_pxmf3Positions[1] = XMFLOAT3(-fx, -fy, fz);
	m_pxmf3Positions[2] = XMFLOAT3(+fx, -fy, fz);
	m_pxmf3Positions[3] = XMFLOAT3(+fx, -fy, fz);
	m_pxmf3Positions[4] = XMFLOAT3(+fx, +fy, fz);
	m_pxmf3Positions[5] = XMFLOAT3(-fx, +fy, fz);

	texture[0] = XMFLOAT2(1.0f, 0.0f);
	texture[1] = XMFLOAT2(1.0f, 1.0f);
	texture[2] = XMFLOAT2(0.0f, 1.0f);
	texture[3] = XMFLOAT2(0.0f, 1.0f);
	texture[4] = XMFLOAT2(0.0f, 0.0f);
	texture[5] = XMFLOAT2(1.0f, 0.0f);


	m_nVertices = 6;

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dTextureBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, texture, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureUploadBuffer);

	m_d3dTextureBufferView.BufferLocation = m_pd3dTextureBuffer->GetGPUVirtualAddress();
	m_d3dTextureBufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureBufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

}

CFireMesh::~CFireMesh()
{
	if (m_pd3dTextureBuffer) m_pd3dTextureBuffer->Release();
}

void CFireMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();
	if (m_pd3dTextureUploadBuffer) m_pd3dTextureUploadBuffer->Release();
	m_pd3dTextureUploadBuffer = NULL;
}

void CFireMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dTextureBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);
}

void CFireMesh::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	/*cout << " CFireMesh CreateShaderVariables\n";
	UINT ncbElementBytes = ((sizeof(m_noiseb) + 255) & ~255); 
	m_pd3dcbNoise = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbNoise->Map(0, NULL, (void**)&m_noiseb);

	UINT ncbElementBytes2 = ((sizeof(m_distortionb) + 255) & ~255);
	m_pd3dcbDist = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes2, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbDist->Map(0, NULL, (void**)&m_distortionb);*/

}


void CFireMesh::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	/*D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneTransformsGpuVirtualAddress = m_pd3dcbNoise->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(22, d3dcbBoneTransformsGpuVirtualAddress);	
	D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneTransformsGpuVirtualAddress2 = m_pd3dcbDist->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(23, d3dcbBoneTransformsGpuVirtualAddress2);


	::memcpy(&m_noiseb->frameTime, &frameTime, sizeof(float));
	::memcpy(&m_noiseb->scrollSpeeds, &scrollSpeeds, sizeof(XMFLOAT3));
	::memcpy(&m_noiseb->scales, &scales, sizeof(XMFLOAT3));
	::memcpy(&m_noiseb->padding, &padding, sizeof(float));

	::memcpy(&m_noiseb->frameTime, &frameTime, sizeof(float));
	::memcpy(&m_noiseb->scrollSpeeds, &scrollSpeeds, sizeof(XMFLOAT3));
	::memcpy(&m_noiseb->scales, &scales, sizeof(XMFLOAT3));
	::memcpy(&m_noiseb->padding, &padding, sizeof(float));*/
}

ChealParticleMesh::ChealParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	int index = 0;

	m_particleList = new ParticleType[1];
	m_currentParticleCount = 1;

	m_pxmf3Positions = new XMFLOAT3[6];
	color = new XMFLOAT4[6];
	texture = new XMFLOAT2[6];


	float fxPosition = 0, fyPosition = 0, fzPosition = 0;
	float fx = (fWidth * 0.5f) + fxPosition, fy = (fHeight * 0.5f) + fyPosition, fz = (fDepth * 0.5f) + fzPosition;

	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fy, fz);
	m_pxmf3Positions[1] = XMFLOAT3(-fx, -fy, fz);
	m_pxmf3Positions[2] = XMFLOAT3(+fx, -fy, fz);
	m_pxmf3Positions[3] = XMFLOAT3(+fx, -fy, fz);
	m_pxmf3Positions[4] = XMFLOAT3(+fx, +fy, fz);
	m_pxmf3Positions[5] = XMFLOAT3(-fx, +fy, fz);

	texture[0] = XMFLOAT2(1.0f, 0.0f);
	texture[1] = XMFLOAT2(1.0f, 1.0f);
	texture[2] = XMFLOAT2(0.0f, 1.0f);
	texture[3] = XMFLOAT2(0.0f, 1.0f);
	texture[4] = XMFLOAT2(0.0f, 0.0f);
	texture[5] = XMFLOAT2(1.0f, 0.0f);


	float red = 0.0f;
	float green = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
	float blue = 0.0f;

	//float green = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
	//float blue = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;


	color[0] = XMFLOAT4(red, green, blue, 1.0f);
	color[1] = XMFLOAT4(red, green, blue, 1.0f);
	color[2] = XMFLOAT4(red, green, blue, 1.0f);
	color[3] = XMFLOAT4(red, green, blue, 1.0f);
	color[4] = XMFLOAT4(red, green, blue, 1.0f);
	color[5] = XMFLOAT4(red, green, blue, 1.0f);



	m_nVertices = 6;

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dTextureBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, texture, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureUploadBuffer);

	m_d3dTextureBufferView.BufferLocation = m_pd3dTextureBuffer->GetGPUVirtualAddress();
	m_d3dTextureBufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureBufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_pd3dColorBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, color, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dColoUploadBuffer);

	m_d3dColorBufferView.BufferLocation = m_pd3dColorBuffer->GetGPUVirtualAddress();
	m_d3dColorBufferView.StrideInBytes = sizeof(XMFLOAT4);
	m_d3dColorBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;
}

ChealParticleMesh::~ChealParticleMesh()
{
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
	if (m_pd3dTextureBuffer) m_pd3dTextureBuffer->Release();
}

void ChealParticleMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();
	if (m_pd3dTextureUploadBuffer) m_pd3dTextureUploadBuffer->Release();
	m_pd3dTextureUploadBuffer = NULL;
	if (m_pd3dColoUploadBuffer) m_pd3dColoUploadBuffer->Release();
	m_pd3dColoUploadBuffer = NULL;
}

void ChealParticleMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[3] = { m_d3dPositionBufferView, m_d3dTextureBufferView, m_d3dColorBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 3, pVertexBufferViews);
}


//===============================================


CMagicCircleMesh::CMagicCircleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth, float redc, float greenc, float bluec) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	int index = 0;

	m_particleList = new ParticleType[1];
	m_currentParticleCount = 1;

	m_pxmf3Positions = new XMFLOAT3[6];
	color = new XMFLOAT4[6];
	texture = new XMFLOAT2[6];


	float fxPosition = 0, fyPosition = 0, fzPosition = 0;
	float fx = (fWidth * 0.5f) + fxPosition, fy = (fHeight * 0.5f) + fyPosition, fz = (fDepth * 0.5f) + fzPosition;

	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fy, fz);
	m_pxmf3Positions[1] = XMFLOAT3(-fx, -fy, fz);
	m_pxmf3Positions[2] = XMFLOAT3(+fx, -fy, fz);
	m_pxmf3Positions[3] = XMFLOAT3(+fx, -fy, fz);
	m_pxmf3Positions[4] = XMFLOAT3(+fx, +fy, fz);
	m_pxmf3Positions[5] = XMFLOAT3(-fx, +fy, fz);

	texture[0] = XMFLOAT2(1.0f, 0.0f);
	texture[1] = XMFLOAT2(1.0f, 1.0f);
	texture[2] = XMFLOAT2(0.0f, 1.0f);
	texture[3] = XMFLOAT2(0.0f, 1.0f);
	texture[4] = XMFLOAT2(0.0f, 0.0f);
	texture[5] = XMFLOAT2(1.0f, 0.0f);


	float red = redc;
	float green = greenc;
	float blue = bluec;

	//float green = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
	//float blue = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;


	color[0] = XMFLOAT4(red, green, blue, 1.0f);
	color[1] = XMFLOAT4(red, green, blue, 1.0f);
	color[2] = XMFLOAT4(red, green, blue, 1.0f);
	color[3] = XMFLOAT4(red, green, blue, 1.0f);
	color[4] = XMFLOAT4(red, green, blue, 1.0f);
	color[5] = XMFLOAT4(red, green, blue, 1.0f);



	m_nVertices = 6;

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dTextureBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, texture, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureUploadBuffer);

	m_d3dTextureBufferView.BufferLocation = m_pd3dTextureBuffer->GetGPUVirtualAddress();
	m_d3dTextureBufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureBufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_pd3dColorBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, color, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dColoUploadBuffer);

	m_d3dColorBufferView.BufferLocation = m_pd3dColorBuffer->GetGPUVirtualAddress();
	m_d3dColorBufferView.StrideInBytes = sizeof(XMFLOAT4);
	m_d3dColorBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;
}

CMagicCircleMesh::~CMagicCircleMesh()
{
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
	if (m_pd3dTextureBuffer) m_pd3dTextureBuffer->Release();
}

void CMagicCircleMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();
	if (m_pd3dTextureUploadBuffer) m_pd3dTextureUploadBuffer->Release();
	m_pd3dTextureUploadBuffer = NULL;
	if (m_pd3dColoUploadBuffer) m_pd3dColoUploadBuffer->Release();
	m_pd3dColoUploadBuffer = NULL;
}

void CMagicCircleMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[3] = { m_d3dPositionBufferView, m_d3dTextureBufferView, m_d3dColorBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 3, pVertexBufferViews);
}
