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
    void Render(UINT nFrame);
    void ReleaseResources();
    void Resize(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);

private:
    UINT GetRenderTargetsCount() { return static_cast<UINT>(m_vWrappedRenderTargets.size()); }
    void Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);

    float m_fWidth;
    float m_fHeight;

    ID3D11DeviceContext* m_pd3d11DeviceContext = NULL;
    ID3D11On12Device* m_pd3d11On12Device = NULL;
    IDWriteFactory* m_pd2dWriteFactory = NULL;
    ID2D1Factory3* m_pd2dFactory = NULL;
    ID2D1Device2* m_pd2dDevice = NULL;
    ID2D1DeviceContext2* m_pd2dDeviceContext = NULL;
    ID2D1SolidColorBrush* m_pd2dTextBrush = NULL;
    IDWriteTextFormat* m_pdwTextFormat = NULL;

    std::vector<ID3D11Resource*>    m_vWrappedRenderTargets;
    std::vector<ID2D1Bitmap1*>      m_vd2dRenderTargets;
    std::vector<TextBlock>          m_vTextBlocks;
};

class CUI : public CGameObject
{
public: 
    CUI();
    virtual ~CUI();
    CUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int num = 0, UINT nWidth = 20, UINT nHeight = 20, UINT nDepth = 0,
        XMFLOAT2 xmf2LeftTop = XMFLOAT2(0.f, 0.f), XMFLOAT2 xmf2LeftBot = XMFLOAT2(0.f, 0.f), XMFLOAT2 xmf2RightBot = XMFLOAT2(0.f, 0.f), XMFLOAT2 xmf2RightTop = XMFLOAT2(0.f, 0.f));


    void MinmapUpdate();
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

public:
  
};