#pragma once
#include "Player.h"

struct TextBlock
{
    std::wstring        strText;
    D2D1_RECT_F         d2dLayoutRect;
    IDWriteTextFormat* pdwFormat;
};

class UILayer
{
public:
    UILayer(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, int m_nWndClientWidth, int m_nWndClientHeight);

    void UpdateLabels(const std::wstring& strUIText);
    void UpdateLabels_Scripts(const std::wstring& strUIText);
    void UpdateLabels_BossScripts(const std::wstring& strUIText);

    void UpdateLabels_Jew(const std::wstring& strUIText);


    void UpdateLabels_Lobby(const std::wstring& strUIText);
    void UpdateLabels_LobbyMatching(const std::wstring& strUIText);
    void UpdateLabels_Skip(const std::wstring& strUIText);





    void UpdateDots(int id, CAirplanePlayer* player, XMFLOAT3& epos, bool live = true);
    void UpdateBossNevi(int id, CAirplanePlayer* player, XMFLOAT3& bpos);
    void UpdatePlanetNevi(CAirplanePlayer* player, XMFLOAT3& lpos);

    float UpdatePlanetDist(CAirplanePlayer* player, XMFLOAT3& lpos);

    D2D1_POINT_2F a = {};
    float angle = 0;
    float angle2 = 0;

    bool noData = true;


    void UpdateHp(short curhp, short maxHp = 100);
    void UpdateBossHp(short curhp, short maxHp = 100);
    void UpdateGodBossHp(short curhp, short maxHp = 100);



    XMFLOAT4X4 UpdateMat(const XMFLOAT3& ppos);

    void Render(UINT nFrame, MissionType mty = MissionType::TU_SIT, BossState bst = BossState::SLEEP, int sst = 0, float etime = 0);
    void ReleaseResources();
    void Resize(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);


    void SetScreenSize(int width, int height) {
        m_nScreenWidth = width;
        m_nScreenHeight = height;
    };
private:
    UINT GetRenderTargetsCount() { return static_cast<UINT>(m_vWrappedRenderTargets.size()); }
    void Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);
    void InitializeImage(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);
    void DrawDot(int dotCnt = NULL, XMFLOAT3[] = NULL);

    int m_nScreenWidth;
    int m_nScreenHeight;

    UINT originalWidth, originalHeight;
    float scaleX ;
    float scaleY;






    //float bossHpbarLeft = m_nScreenWidth / 10;
    //float bossHpbarRight = m_nScreenWidth / 10 * 9;
    //float bossHpbarTop = m_nScreenHeight / 10 * 1;
    //float bossHpbarBottom = m_nScreenHeight / 10 * 1.5;


    //float GodbossHpbarLeft = m_nScreenWidth / 10;
    //float GodbossHpbarRight = m_nScreenWidth / 10 * 9;
    //float GodbossHpbarTop = m_nScreenHeight / 10 * 1;
    //float GodbossHpbarBottom = m_nScreenHeight / 10 * 1.5;

    float m_fWidth;
    float m_fHeight;

    float hpBar = 0;
    float BosshpBar = 0;

    float GodBosshpBar = 0;

    float actime = 0;
    float actime2 = 0;



    ID3D11DeviceContext* m_pd3d11DeviceContext = NULL;
    ID3D11On12Device* m_pd3d11On12Device = NULL;
    IDWriteFactory* m_pd2dWriteFactory = NULL;
    ID2D1Factory3* m_pd2dFactory = NULL;
    ID2D1Device2* m_pd2dDevice = NULL;
    ID2D1DeviceContext2* m_pd2dDeviceContext = NULL;
    ID2D1SolidColorBrush* m_pd2dTextBrush = NULL;
    ID2D1SolidColorBrush* m_pd2dTextBlackBrush = NULL;
    ID2D1SolidColorBrush* m_pd2dTextGrayBrush = NULL;



    ID2D1SolidColorBrush* Redbrush = NULL;
    ID2D1SolidColorBrush* Whitebrush = NULL;
    ID2D1SolidColorBrush* DotBossbrush = NULL;


    IDWriteTextFormat* m_pdwTextFormat = NULL;
    IDWriteTextFormat* m_pdwScriptsFormat = NULL;
    IDWriteTextFormat* m_pdwBossScriptsFormat = NULL;

    IDWriteTextFormat* m_pdwJewFormat = NULL;

    IDWriteTextFormat* m_pdwLobbyFormat = NULL;
    IDWriteTextFormat* m_pdwLobbyMatchingFormat = NULL;
    IDWriteTextFormat* m_pdwSkipFormat = NULL;








    IWICImagingFactory* m_pwicImagingFactory = NULL;
    ID2D1Effect* m_pd2dfxBitmapSource = NULL;
    ID2D1Effect* m_pd2dfxBitmapSource_jew = NULL;
    ID2D1Effect* m_pd2dfxBitmapSource_logo = NULL;
    ID2D1Effect* m_pd2dfxBitmapSource_clear_logo = NULL;
    ID2D1Effect* m_pd2dfxBitmapSource_died_logo = NULL;



    ID2D1Effect* m_pd2dfxBitmapSource_nevi = NULL;
    ID2D1Effect* m_pd2dfxBitmapSource_nevi2 = NULL;

    ID2D1Effect* m_pd2dfxBitmapSource_Lobby = NULL;






    ID2D1Effect* m_pd2dfxGaussianBlur = NULL;
    ID2D1Effect* m_pd2dfxGaussianBlur_jew = NULL;
    ID2D1Effect* m_pd2dfxGaussianBlur_logo = NULL;
    ID2D1Effect* m_pd2dfxGaussianBlur_clear_logo = NULL;
    ID2D1Effect* m_pd2dfxGaussianBlur_died_logo = NULL;



    ID2D1Effect* m_pd2dfxGaussianBlur_nevi = NULL;
    ID2D1Effect* m_pd2dfxGaussianBlur_nevi2 = NULL;

    ID2D1Effect* m_pd2dfxGaussianBlur_Lobby = NULL;
    ID2D1Effect* m_pd2dfxGaussianBlur_MatchingLobby = NULL;






    ID2D1Effect* m_pd2dfxSize = NULL;
    ID2D1Effect* m_pd2dfxSize_jew = NULL;
    ID2D1Effect* m_pd2dfxSize_logo = NULL;
    ID2D1Effect* m_pd2dfxSize_clear_logo = NULL;
    ID2D1Effect* m_pd2dfxSize_died_logo = NULL;




    ID2D1Effect* m_pd2dfxSize_nevi = NULL;
    ID2D1Effect* m_pd2dfxSize_nevi2 = NULL;


    ID2D1Effect* m_pd2dfxSize_Lobby = NULL;
    ID2D1Effect* m_pd2dfxSize_MatchingLobby = NULL;







    ID2D1DrawingStateBlock1* m_pd2dsbDrawingState = NULL;
    IWICFormatConverter* m_pwicFormatConverter[5] = {};

    int							m_nDrawEffectImage = 0;


    std::vector<ID3D11Resource*>    m_vWrappedRenderTargets;
    std::vector<ID2D1Bitmap1*>      m_vd2dRenderTargets;
    std::vector<TextBlock>          m_vTextBlocks;
    std::vector<TextBlock>          m_vScriptsBlocks;
    std::vector<TextBlock>          m_vJewBlocks;
    std::vector<TextBlock>          m_vLobbyBlocks;
    std::vector<TextBlock>          m_vLobbyMatchingBlocks;
    std::vector<TextBlock>          m_vSkipBlocks;

    std::vector<TextBlock>          m_vBossScriptsBlocks;







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