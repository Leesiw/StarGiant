//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "Camera.h"
 #include "Timer.h"


#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

class CShader;
class CStandardShader;
class CGameTimer;

enum class AnimationState
{
	IDLE, WALK, SIT
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

struct SRVROOTARGUMENTINFO
{
	int								m_nRootParameterIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGpuDescriptorHandle;
};

struct CB_PLUS_INFO
{
	XMFLOAT4X4 gmtxTexture;
	float		gfCurrentTime;
};

class CTexture
{
public:
	CTexture(int nTextureResources = 1, UINT nResourceType = RESOURCE_TEXTURE2D, int nSamplers = 0, int nRootParameters = 0);
	virtual ~CTexture();

private:
	int								m_nReferences = 0;

	UINT							m_nTextureType = RESOURCE_TEXTURE2D;
	UINT* m_pnResourceTypes = NULL;

	int								m_nTextures = 0;
	ID3D12Resource** m_ppd3dTextures = NULL;
	ID3D12Resource** m_ppd3dTextureUploadBuffers;

	int								m_nRootParameters = 0;
	int* m_pnRootParameterIndices = NULL;

	int								m_nSamplers = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSamplerGpuDescriptorHandles = NULL;

	D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSrvGpuDescriptorHandles = NULL;

	DXGI_FORMAT* m_pdxgiBufferFormats = NULL;
	int* m_pnBufferElements = NULL;

public:
	SRVROOTARGUMENTINFO* m_pRootArgumentInfos = NULL;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dsrvGpuDescriptorHandle);
	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nIndex);
	void ReleaseShaderVariables();

	void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nIndex, bool bIsDDSFile = true);
	ID3D12Resource* CreateTexture(ID3D12Device* pd3dDevice, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue, UINT nResourceType, UINT nIndex);

	void SetSrvGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle);
	void SetRootParameterIndex(int nIndex, UINT nRootParameterIndex)
	{
		m_pnRootParameterIndices[nIndex] = nRootParameterIndex;
	}

	int GetTextures() { return(m_nTextures); }
	ID3D12Resource* GetTexture(int nIndex) { return(m_ppd3dTextures[nIndex]); }
	UINT GetTextureType() { return(m_nTextureType); }
	UINT GetTextureType(int nIndex) { return(m_pnResourceTypes[nIndex]); }
	int GetRootParameters() { return(m_nRootParameters); }

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int nIndex);

	void ReleaseUploadBuffers();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MATERIAL_ALBEDO_MAP				0x01
#define MATERIAL_SPECULAR_MAP			0x02
#define MATERIAL_NORMAL_MAP				0x04
#define MATERIAL_METALLIC_MAP			0x08
#define MATERIAL_EMISSION_MAP			0x10
#define MATERIAL_DETAIL_ALBEDO_MAP		0x20
#define MATERIAL_DETAIL_NORMAL_MAP		0x40

class CGameObject;
class CSpriteObject;

class CMaterial
{
public:
	CMaterial(int nTextures);
	virtual ~CMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	CShader							*m_pShader = NULL;

	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	void SetShader(CShader *pShader);
	void SetMaterialType(UINT nType) { m_nType |= nType; }
	void SetTexture(CTexture *pTexture, UINT nTexture = 0);

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void ReleaseUploadBuffers();

public:
	UINT							m_nType = 0x00;

	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

public:
	int 							m_nTextures = 0;
	_TCHAR							(*m_ppstrTextureNames)[64] = NULL;
	CTexture						**m_ppTextures = NULL; //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

	void LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR *pwstrTextureName, CTexture **ppTexture, CGameObject *pParent, FILE *pInFile, CShader *pShader);

public:
	static CShader					*m_pStandardShader;
	static CShader					*m_pSkinnedAnimationShader;

	static void CMaterial::PrepareShaders(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);

	void SetStandardShader() { CMaterial::SetShader(m_pStandardShader); }
	void SetSkinnedAnimationShader() { CMaterial::SetShader(m_pSkinnedAnimationShader); }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct CALLBACKKEY
{
   float  							m_fTime = 0.0f;
   void  							*m_pCallbackData = NULL;
};

#define _WITH_ANIMATION_INTERPOLATION

class CAnimationCallbackHandler
{
public:
	CAnimationCallbackHandler() { }
	~CAnimationCallbackHandler() { }

public:
   virtual void HandleCallback(void *pCallbackData, float fTrackPosition) { }
};

class CAnimationSet
{
public:
	CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrameTransforms, int nSkinningBones, char *pstrName);
	~CAnimationSet();

public:
	char							m_pstrAnimationSetName[64];

	float							m_fLength = 0.0f;
	int								m_nFramesPerSecond = 0; //m_fTicksPerSecond

	int								m_nKeyFrames = 0;
	float							*m_pfKeyFrameTimes = NULL;
	XMFLOAT4X4						**m_ppxmf4x4KeyFrameTransforms = NULL;

#ifdef _WITH_ANIMATION_SRT
	int								m_nKeyFrameScales = 0;
	float							*m_pfKeyFrameScaleTimes = NULL;
	XMFLOAT3						**m_ppxmf3KeyFrameScales = NULL;
	int								m_nKeyFrameRotations = 0;
	float							*m_pfKeyFrameRotationTimes = NULL;
	XMFLOAT4						**m_ppxmf4KeyFrameRotations = NULL;
	int								m_nKeyFrameTranslations = 0;
	float							*m_pfKeyFrameTranslationTimes = NULL;
	XMFLOAT3						**m_ppxmf3KeyFrameTranslations = NULL;
#endif

	float 							m_fPosition = 0.0f;
    int 							m_nType = ANIMATION_TYPE_LOOP; //Once, Loop, PingPong

	int 							m_nCallbackKeys = 0;
	CALLBACKKEY 					*m_pCallbackKeys = NULL;

	CAnimationCallbackHandler 		*m_pAnimationCallbackHandler = NULL;

public:
	void SetPosition(float fTrackPosition);

	XMFLOAT4X4 GetSRT(int nBone);

	void SetCallbackKeys(int nCallbackKeys);
	void SetCallbackKey(int nKeyIndex, float fTime, void *pData);
	void SetAnimationCallbackHandler(CAnimationCallbackHandler *pCallbackHandler);

	void HandleCallback();
};

class CAnimationSets
{
private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	CAnimationSets(int nAnimationSets);
	~CAnimationSets();

public:
	int								m_nAnimationSets = 0;
	CAnimationSet					**m_pAnimationSets = NULL;

	int								m_nAnimatedBoneFrames = 0; 
	CGameObject						**m_ppAnimatedBoneFrameCaches = NULL; //[m_nAnimatedBoneFrames]

public:
	void SetCallbackKeys(int nAnimationSet, int nCallbackKeys);
	void SetCallbackKey(int nAnimationSet, int nKeyIndex, float fTime, void *pData);
	void SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler *pCallbackHandler);
};

class CAnimationTrack
{
public:
	CAnimationTrack() { }
	~CAnimationTrack() { }

public:
    BOOL 							m_bEnable = true;
    float 							m_fSpeed = 1.0f;
    float 							m_fPosition = 0.0f;
    float 							m_fWeight = 1.0f;

	int 							m_nAnimationSet = 0;

public:
	void SetAnimationSet(int nAnimationSet) { m_nAnimationSet = nAnimationSet; }

	void SetEnable(bool bEnable) { m_bEnable = bEnable; }
	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	void SetWeight(float fWeight) { m_fWeight = fWeight; }
	void SetPosition(float fPosition) { m_fPosition = fPosition; }
};

class CLoadedModelInfo
{
public:
	CLoadedModelInfo() { }
	~CLoadedModelInfo();

    CGameObject						*m_pModelRootObject = NULL;

	int 							m_nSkinnedMeshes = 0;
	CSkinnedMesh					**m_ppSkinnedMeshes = NULL; //[SkinnedMeshes], Skinned Mesh Cache

	CAnimationSets					*m_pAnimationSets = NULL;


public:
	void PrepareSkinning();
};

class CAnimationController 
{
public:
	CAnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nAnimationTracks, CLoadedModelInfo *pModel);
	~CAnimationController();

public:
    float 							m_fTime = 0.0f;

    int 							m_nAnimationTracks = 0;
    CAnimationTrack 				*m_pAnimationTracks = NULL;

	CAnimationSets					*m_pAnimationSets = NULL;

	int 							m_nSkinnedMeshes = 0;
	CSkinnedMesh					**m_ppSkinnedMeshes = NULL; //[SkinnedMeshes], Skinned Mesh Cache

	ID3D12Resource					**m_ppd3dcbSkinningBoneTransforms = NULL; //[SkinnedMeshes]
	XMFLOAT4X4						**m_ppcbxmf4x4MappedSkinningBoneTransforms = NULL; //[SkinnedMeshes]

public:
	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);

	void SetTrackEnable(int nAnimationTrack, bool bEnable);
	void SetTrackPosition(int nAnimationTrack, float fPosition);
	void SetTrackSpeed(int nAnimationTrack, float fSpeed);
	void SetTrackWeight(int nAnimationTrack, float fWeight);
	void SetTrackAllUnable(int Cnt);

	void SetCallbackKeys(int nAnimationSet, int nCallbackKeys);
	void SetCallbackKey(int nAnimationSet, int nKeyIndex, float fTime, void *pData);
	void SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler *pCallbackHandler);

	void AdvanceTime(float fElapsedTime, CGameObject *pRootGameObject);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CGameObject
{
private:
	int								m_nReferences = 0;

public:
	void AddRef();
	void Release();

public:
	CGameObject();
	CGameObject(int nMaterials);
    virtual ~CGameObject();

public:
	char							m_pstrFrameName[64];

	CMesh							*m_pMesh = NULL;

	int								m_nMaterials = 0;
	CMaterial						**m_ppMaterials = NULL;

	XMFLOAT4X4						m_xmf4x4ToParent;
	XMFLOAT4X4						m_xmf4x4World;
	XMFLOAT4X4						m_xmf4x4Texture;

	CGameObject 					*m_pParent = NULL;
	CGameObject 					*m_pChild = NULL;
	CGameObject 					*m_pSibling = NULL;

	XMFLOAT3						m_xmf3Scale = XMFLOAT3(1.f, 1.f, 1.f);


	BoundingBox			aabb;
	bool						m_bActive = false;

	XMFLOAT3					m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3MovingDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	float						m_fMovingSpeed = 0.0f;
	float						m_fMovingRange = 0.0f;


	void SetMesh(CMesh *pMesh);
	void SetShader(CShader *pShader);
	void SetShader(int nMaterial, CShader *pShader);
	void SetMaterial(int nMaterial, CMaterial *pMaterial);
	void SetTexture(CTexture* texture);

	void SetChild(CGameObject *pChild, bool bReferenceUpdate=false);

	virtual void BuildMaterials(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }

	virtual void OnPrepareAnimate() { }
	void Animate(float fTimeElapsed);

	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, CMaterial *pMaterial);

	void ReleaseUploadBuffers();
	virtual void ReleaseUploadBuffers2();


	XMFLOAT3 GetPosition();
	XMFLOAT2 GetPositionXY() { return(XMFLOAT2(m_xmf4x4World._41, m_xmf4x4World._42)); };
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float x, float y, float z);
	void SetLookAt(XMFLOAT3 xmf3Target, XMFLOAT3 xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f));
	void SetQuaternion(XMFLOAT4 Quaternion);


	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4 *pxmf4Quaternion);

	CGameObject *GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent=NULL);

	CGameObject *FindFrame(char *pstrFrameName);

	CTexture *FindReplicatedTexture(_TCHAR *pstrTextureName);

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); }

	void ResetScale();

public:
	CAnimationController 			*m_pSkinnedAnimationController = NULL;

	CSkinnedMesh *FindSkinnedMesh(char *pstrSkinnedMeshName);
	void FindAndSetSkinnedMesh(CSkinnedMesh **ppSkinnedMeshes, int *pnSkinnedMesh);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
	void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);
	float GetTrackAnimationPosition(int nAnimationTrack);


	void LoadMaterialsFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject *pParent, FILE *pInFile, CShader *pShader);



	static void LoadAnimationFromFile(FILE *pInFile, CLoadedModelInfo *pLoadedModel);
	static CGameObject *LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CGameObject *pParent, FILE *pInFile, CShader *pShader, int *pnSkinnedMeshes);

	static CLoadedModelInfo *LoadGeometryAndAnimationFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, char *pstrFileName, CShader *pShader);

	static void PrintFrameInfo(CGameObject *pGameObject, CGameObject *pParent);


	void SetActive(bool bActive) { m_bActive = bActive; }



};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CHeightMapTerrain : public CGameObject
{
public:
	CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color);
	virtual ~CHeightMapTerrain();

private:
	CHeightMapImage				*m_pHeightMapImage;

	int							m_nWidth;
	int							m_nLength;

	XMFLOAT3					m_xmf3Scale;

public:
	float GetHeight(float x, float z, bool bReverseQuad = false) { return(m_pHeightMapImage->GetHeight(x, z, bReverseQuad) * m_xmf3Scale.y); } //World
	XMFLOAT3 GetNormal(float x, float z) { return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z))); }

	int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
	float GetLength() { return(m_nLength * m_xmf3Scale.z); }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBox : public CGameObject
{
public:
	CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CSkyBox();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSuperCobraObject : public CGameObject
{
public:
	CSuperCobraObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CSuperCobraObject();

private:
	CGameObject					*m_pMainRotorFrame = NULL;
	CGameObject					*m_pTailRotorFrame = NULL;

public:
	virtual void OnPrepareAnimate();
	virtual void Animate(float fTimeElapsed);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CGunshipObject : public CGameObject
{
public:
	CGunshipObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CGunshipObject();

private:
	CGameObject					*m_pMainRotorFrame = NULL;
	CGameObject					*m_pTailRotorFrame = NULL;

public:
	virtual void OnPrepareAnimate();
	virtual void Animate(float fTimeElapsed);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMi24Object : public CGameObject
{
public:
	CMi24Object(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CMi24Object();

private:
	CGameObject					*m_pMainRotorFrame = NULL;
	CGameObject					*m_pTailRotorFrame = NULL;

public:
	virtual void OnPrepareAnimate();
	virtual void Animate(float fTimeElapsed);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CAngrybotObject : public CGameObject
{
public:
	CAngrybotObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks);
	virtual ~CAngrybotObject();
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMeteorObject : public CGameObject
{
public:
	CMeteorObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CMeteorObject();

	virtual void Animate(float fTimeElapsed);
private:
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMonsterObject : public CGameObject
{
public:
	CMonsterObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks);
	virtual ~CMonsterObject();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CHumanoidObject : public CGameObject
{
public:
	CHumanoidObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks);
	virtual ~CHumanoidObject();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CEthanObject : public CGameObject
{
public:
	CEthanObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks);
	virtual ~CEthanObject();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CLionObject : public CGameObject
{
public:
	CLionObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks);
	virtual ~CLionObject();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CZebraObject : public CGameObject
{
public:
	CZebraObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks);
	virtual ~CZebraObject();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CEagleObject : public CGameObject
{
public:
	CEagleObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks);
	virtual ~CEagleObject();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CEnemyObject : public CGameObject
{
public:
	bool isAlive;
	short hp;
	short Maxhp = hp;

	bool isUpdate = false;

	float						m_fAttackRange = 300.0f;

	XMFLOAT3					m_xmf3Velocity;
	XMFLOAT3					m_xmf3Destination;

	EnemyState state;

	CEnemyObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CEnemyObject();

	virtual void AI(float fTimeElapsed, XMFLOAT3& pl_pos);
	virtual void MoveAI(float fTimeElapsed, XMFLOAT3& pl_pos);
	virtual void AimingAI(float fTimeElapsed, XMFLOAT3& pl_pos);
	void LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos);
	virtual XMFLOAT3 GetEnemyPosition() { return XMFLOAT3(m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43); }
	
	void VelocityUpdate(float fTimeElapsed, XMFLOAT3& pos);

	void Reset();
	short GetcurHp() { return hp; }
	short GetMaxHp() { return Maxhp; }

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CInsideShipObject : public CGameObject
{
public:
	CInsideShipObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CInsideShipObject();
};
//===================================


class CBulletObject : public CGameObject
{
public:
	CBulletObject(float fEffectiveRange);
	CBulletObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);

	virtual ~CBulletObject() {};
public:
	virtual void Animate(float fElapsedTime);

	float						m_fBulletEffectiveRange = 150.0f;
	float						m_fMovingDistance = 0.0f;
	float						m_fRotationAngle = 0.0f;
	XMFLOAT3					m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float						m_fElapsedTimeAfterFire = 0.0f;
	float						m_fLockingDelayTime = 0.3f;
	float						m_fLockingTime = 4.0f;
	CGameObject* m_pLockedObject = NULL;

	XMFLOAT4X4						m_xmf4x4Transform;


	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	bool						is_fire = false;
	bool						is_enemy_fire = false;
	float						m_fRotationSpeed = 0.0f;

	float m_fPitch, m_fYaw, m_fRoll;

	void SetRotationAxis(XMFLOAT3& xmf3RotationAxis) { m_xmf3RotationAxis = Vector3::Normalize(xmf3RotationAxis); }
	void SetRotationSpeed(float fSpeed) { m_fRotationSpeed = fSpeed; }
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
	void SetMovingDirection(XMFLOAT3& xmf3MovingDirection) { m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection); }
	XMFLOAT3 GetDirection() { return m_xmf3MovingDirection; }

	void SetFirePosition(XMFLOAT3 xmf3FirePosition){
		m_xmf3FirePosition = xmf3FirePosition; SetPosition(xmf3FirePosition);
		 m_xmf4x4ToParent._41 = m_xmf3FirePosition.x;
		 m_xmf4x4ToParent._42 = m_xmf3FirePosition.y;
		 m_xmf4x4ToParent._43 = m_xmf3FirePosition.z;
	};
	void SetEnemyFire4x4(XMFLOAT3 Player_Position);
	void Move(DWORD dwDirection, float fDistance);
	void Move(const XMFLOAT3& xmf3Shift);

	void Reset();


};

class CMissileObject : public CGameObject
{
public:
	CMissileObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CMissileObject() {};
public:
	//virtual void Animate(float fElapsedTime);

	void LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos);

	void ResetRotate();
};

class CUIObject : public CGameObject
{
public:
	CUIObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CUIObject() {};

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};

class CSpriteObject : public CGameObject
{
public:
	CSpriteObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, XMFLOAT3 Center,XMFLOAT3 Size,int type =0);
	virtual ~CSpriteObject() {};

	float m_fSpeed = 0.1f;
	float m_fTime = 0.0f;
	float m_fCntTime = 0.0f;
	int m_nRow = 0;
	int m_nCol = 0;
	int m_nRows = 1;
	int m_nCols = 1;
	int SpriteMode = 0;
	bool is_Alive = false;
	int TargetNum = -1;

	CB_PLUS_INFO* m_pcbPlusInfo = NULL;
	ID3D12Resource* m_pcbplusShaderVariable = NULL;

	void Animate(float fElapsedTime);
	void SetRowColumn(int nRow, int nCol) { m_nRows = nRow; m_nCols = nCol; }
	void SetCntTime(float Time) { m_fCntTime = Time; }
	void SetSpeed(float Speed) { m_fSpeed = Speed; }
	void SetfollowPosition(XMFLOAT3 Target, XMFLOAT3 Distance, XMFLOAT3 LookAt);
	void SetNewTexture(ID3D12Device* pd3dDevice,CTexture* pSpriteTexture);

	float GetCntTime() {return m_fCntTime;};
	virtual ID3D12Resource* GetShaderVariables() { return m_pcbplusShaderVariable; }


	virtual void CreateShaderVariable(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource* m_pd3dcbPlusInfo);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	void CountDiedTime(float dieTime);
};
class CSprite2Object : public CSpriteObject
{
	CSprite2Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, XMFLOAT3 Center, XMFLOAT3 Size, int type = 0);
	virtual ~CSprite2Object() {};
};

//====================================================================================================
class CMascotObject : public CGameObject
{
public:
	CMascotObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CMascotObject();
};

//====================================================================================================
class CBlackHole : public CGameObject
{
public:
	CBlackHole() {};
	virtual ~CBlackHole() {};
	CBlackHole(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nWidth = 20, UINT nHeight = 20, UINT nDepth = 0);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
public:
	CTexture* m_blackholeTexture;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CBlackHoleMeteorObject : public CGameObject
{
public:
	CBlackHoleMeteorObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CBlackHoleMeteorObject() {};

	void Animate(float fTimeElapsed, XMFLOAT3 cPos);
private:
	float m_fRotationSpeed = 1.0f;  // 공전 속도
	float m_fDistanceFromCenter;  // 공전 중심으로부터의 거리
	XMFLOAT3 m_xmf3MovingDirection;  // 공전 방향


	float ffffTimeElapsed = 0.0f;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CJewelObject : public CGameObject
{
public:
	CJewelObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CJewelObject() {};
	virtual void endingMove(float fElapsedTime, XMFLOAT3 tarpos);
	virtual void Animate(float fElapsedTime){ CGameObject::Animate(fElapsedTime); };


	float anitime =0;

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CParticleObject : public CGameObject
{
public:
	CParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CParticleObject() {};

	virtual void Animate(float fElapsedTime);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);


	void setPos(XMFLOAT3 pos);
	int getMaxParticle() { return m_maxParticles; };
	bool isLive = false;
private:
	float velocity = 0;
	float m_particleVelocity = 0;
	int m_maxParticles = 0;

	XMFLOAT3A position;

	float angleX = 0;
	float angleY = 0;
	float angleZ = 0;

	float lifeTime = 1.5f;
	float ffTimeElapsed = 0.0f;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CFireObject : public CGameObject
{
public:
	CFireObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CFireObject() {};

	float time;
	CB_PLUS_INFO* m_pcbPlusInfo = NULL;
	ID3D12Resource* m_pcbplusShaderVariable = NULL;
	CGameTimer					m_GameTimer;
	
	virtual void Animate(float fElapsedTime);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource* m_pd3dcbPlusInfo);
	virtual ID3D12Resource* GetShaderVariables() { return m_pcbplusShaderVariable; }

	void SetLookAt(XMFLOAT3& xmf3TargetPosition, XMFLOAT3& xmf3Up);

	void Rotate180Degrees();

private:
	float ffTimeElapsed = 0.0f;

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CFlameParticleObject : public CGameObject
{
public:
	CFlameParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CFlameParticleObject() {};

	virtual void Animate(float fElapsedTime);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	int getMaxParticle() { return m_maxParticles; };
	void setTarpos(XMFLOAT3 tarpos) { TargetPos = tarpos; }
	void setPos(XMFLOAT3 pos);

	bool isLive = false;


	XMVECTOR direction;
	XMVECTOR targetDirection = { 0,0,0,0 };
	int count = 0;

private:
	float velocity = 1;
	float m_particleVelocity = 0;
	int m_maxParticles = 0;

	XMFLOAT3 position;

	XMFLOAT3 TargetPos;


	float angleX = 0;
	float angleY = 0;
	float angleZ = 0;

	float intervalX = 0;
	float intervalY = 0;
	float intervalZ = 0;

	float lifeTime = 4.0f;
	float ffTimeElapsed = 0.0f;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ChealParticleObject : public CGameObject
{
public:
	ChealParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~ChealParticleObject() {};

	virtual void Animate(float fElapsedTime);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	int getMaxParticle() { return m_maxParticles; };


	void setPos(XMFLOAT3 pos);
	bool isLive = false;
private:
	float velocity = 0;
	float m_particleVelocity = 0;
	int m_maxParticles = 0;

	XMFLOAT3A position;

	float angleX = 0;
	float angleY = 0;
	float angleZ = 0;

	float intervalX = 0;
	float intervalY = 0;
	float intervalZ = 0;

	float lifeTime = 2.0f;
	float ffTimeElapsed = 0.0f;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSkullObject : public CGameObject
{
public:
	CSkullObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CSkullObject() {};

	virtual void Animate(float fElapsedTime);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	int getMaxParticle() { return m_maxParticles; };
	void setTarpos(XMFLOAT3 tarpos) { TargetPos = tarpos; }
	void setPos(XMFLOAT3 pos);

	bool isLive = false;


	XMVECTOR direction;
	XMVECTOR targetDirection = { 0,0,0,0 };
	int count = 0;

private:
	float velocity = 1;
	float m_particleVelocity = 0;
	int m_maxParticles = 0;

	XMFLOAT3 position;

	XMFLOAT3 TargetPos;


	float angleX = 0;
	float angleY = 0;
	float angleZ = 0;

	float intervalX = 0;
	float intervalY = 0;
	float intervalZ = 0;

	float lifeTime = 4.0f;
	float ffTimeElapsed = 0.0f;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CLineObject : public CGameObject
{
public:
	CLineObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CLineObject() {};

	virtual void Animate(float fElapsedTime);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	int getMaxParticle() { return m_maxParticles; };
	void setTarpos(XMFLOAT3 tarpos) { TargetPos = tarpos; }
	void setPos(XMFLOAT3 pos);

	bool isLive = false;


	XMVECTOR direction;
	XMVECTOR targetDirection = { 0,0,0,0 };
	int count = 0;

private:
	float velocity = 1;
	float m_particleVelocity = 0;
	int m_maxParticles = 0;

	XMFLOAT3 position;

	XMFLOAT3 TargetPos;


	float angleX = 0;
	float angleY = 0;
	float angleZ = 0;

	float intervalX = 0;
	float intervalY = 0;
	float intervalZ = 0;

	float lifeTime = 1.5f;
	float ffTimeElapsed = 0.0f;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CMagicCircleObject : public CGameObject
{
public:
	CMagicCircleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int cnt, int num);
	virtual ~CMagicCircleObject() {};

	virtual void Animate(float fElapsedTime);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	int getMaxParticle() { return m_maxParticles; };
	void setTarpos(XMFLOAT3 tarpos) { TargetPos = tarpos; }
	void setPos(XMFLOAT3 pos);

	bool isLive = false;


	XMVECTOR direction;
	XMVECTOR targetDirection = { 0,0,0,0 };
	int count = 0;

private:
	float velocity = 1;
	float m_particleVelocity = 0;
	int m_maxParticles = 0;

	XMFLOAT3 position;

	XMFLOAT3 TargetPos;


	float angleX = 0;
	float angleY = 0;
	float angleZ = 0;

	float intervalX = 0;
	float intervalY = 0;
	float intervalZ = 0;

	float lifeTime = 1.5f;
	float ffTimeElapsed = 0.0f;
};


