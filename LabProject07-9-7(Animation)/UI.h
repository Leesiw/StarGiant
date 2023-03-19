#pragma once

struct TextBlock
{
    std::wstring        strText;
    D2D1_RECT_F         d2dLayoutRect;
    IDWriteTextFormat* pdwFormat;
};

class UILayer
{
public:
    UILayer(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);

    void UpdateLabels(const std::wstring& strUIText);
    void UpdateDots(int id, XMFLOAT3& ppos, XMFLOAT3& epos);
    XMFLOAT4X4 UpdateMat(const XMFLOAT3& ppos);

    void Render(UINT nFrame, int dotCnt = NULL, XMFLOAT3 [] = NULL);
    void ReleaseResources();
    void Resize(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);

private:
    UINT GetRenderTargetsCount() { return static_cast<UINT>(m_vWrappedRenderTargets.size()); }
    void Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);
    void InitializeImage(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);
    void DrawDot(int dotCnt = NULL, XMFLOAT3[] = NULL);




    float m_fWidth;
    float m_fHeight;

    ID3D11DeviceContext* m_pd3d11DeviceContext = NULL;
    ID3D11On12Device* m_pd3d11On12Device = NULL;
    IDWriteFactory* m_pd2dWriteFactory = NULL;
    ID2D1Factory3* m_pd2dFactory = NULL;
    ID2D1Device2* m_pd2dDevice = NULL;
    ID2D1DeviceContext2* m_pd2dDeviceContext = NULL;
    ID2D1SolidColorBrush* m_pd2dTextBrush = NULL;

    ID2D1SolidColorBrush* Dotbrush = NULL;
    ID2D1SolidColorBrush* DotBossbrush = NULL;


    IDWriteTextFormat* m_pdwTextFormat = NULL;


    IWICImagingFactory* m_pwicImagingFactory = NULL;
    ID2D1Effect* m_pd2dfxBitmapSource = NULL;
    ID2D1Effect* m_pd2dfxGaussianBlur = NULL;

    ID2D1Effect* m_pd2dfxSize = NULL;


    ID2D1DrawingStateBlock1* m_pd2dsbDrawingState = NULL;
    IWICFormatConverter* m_pwicFormatConverter = NULL;
    int							m_nDrawEffectImage = 0;


    std::vector<ID3D11Resource*>    m_vWrappedRenderTargets;
    std::vector<ID2D1Bitmap1*>      m_vd2dRenderTargets;
    std::vector<TextBlock>          m_vTextBlocks;

    XMFLOAT3 m_enemyDot[ENEMIES];
    XMFLOAT3 m_bossDot;

};

class CUI : public CGameObject
{
public: 
    CUI();
    virtual ~CUI();
    CUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int num = 0, UINT nWidth = 20, UINT nHeight = 20, UINT nDepth = 0);


    void MinmapUpdate();
    void HpbarUpdate(XMFLOAT3 pos, short MaxHp, short curHp);
    
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

public:
    CTexture* m_ppUITexture;
};