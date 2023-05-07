#include "stdafx.h"
#include "Shader.h"
#include "UI.h"
#include "Scene.h"



UILayer::UILayer(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue)
{
    m_fWidth = 0.0f;
    m_fHeight = 0.0f;
    m_vWrappedRenderTargets.resize(nFrame);
    m_vd2dRenderTargets.resize(nFrame);
    m_vTextBlocks.resize(1);
    m_vScriptsBlocks.resize(1);
    m_vJewBlocks.resize(1);

    Initialize(pd3dDevice, pd3dCommandQueue);
    InitializeImage(pd3dDevice, pd3dCommandQueue);
}

void UILayer::Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue)
{
    UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D2D1_FACTORY_OPTIONS d2dFactoryOptions = { };

#if defined(_DEBUG) || defined(DBG)
    d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ID3D11Device* pd3d11Device = NULL;
    ID3D12CommandQueue* ppd3dCommandQueues[] = { pd3dCommandQueue };
    ::D3D11On12CreateDevice(pd3dDevice, d3d11DeviceFlags, nullptr, 0, reinterpret_cast<IUnknown**>(ppd3dCommandQueues), _countof(ppd3dCommandQueues), 0, (ID3D11Device**)&pd3d11Device, (ID3D11DeviceContext**)&m_pd3d11DeviceContext, nullptr);

    pd3d11Device->QueryInterface(__uuidof(ID3D11On12Device), (void**)&m_pd3d11On12Device);
    pd3d11Device->Release();

#if defined(_DEBUG) || defined(DBG)
    ID3D12InfoQueue* pd3dInfoQueue;
    if (SUCCEEDED(pd3dDevice->QueryInterface(IID_PPV_ARGS(&pd3dInfoQueue))))
    {
        D3D12_MESSAGE_SEVERITY pd3dSeverities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO,
        };

        D3D12_MESSAGE_ID pd3dDenyIds[] =
        {
            D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
        };

        D3D12_INFO_QUEUE_FILTER d3dInforQueueFilter = { };
        d3dInforQueueFilter.DenyList.NumSeverities = _countof(pd3dSeverities);
        d3dInforQueueFilter.DenyList.pSeverityList = pd3dSeverities;
        d3dInforQueueFilter.DenyList.NumIDs = _countof(pd3dDenyIds);
        d3dInforQueueFilter.DenyList.pIDList = pd3dDenyIds;

        pd3dInfoQueue->PushStorageFilter(&d3dInforQueueFilter);
    }
    pd3dInfoQueue->Release();
#endif

    IDXGIDevice* pdxgiDevice = NULL;
    m_pd3d11On12Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pdxgiDevice);

    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, (void**)&m_pd2dFactory);
    HRESULT hResult = m_pd2dFactory->CreateDevice(pdxgiDevice, (ID2D1Device2**)&m_pd2dDevice);
    m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, (ID2D1DeviceContext2**)&m_pd2dDeviceContext);

    m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), (ID2D1SolidColorBrush**)&m_pd2dTextBrush);
    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &Redbrush);
    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &DotBossbrush);
    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &Whitebrush);




    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&m_pd2dWriteFactory);
    pdxgiDevice->Release();
}

void UILayer::InitializeImage(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue)
{

    ::CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)&m_pwicImagingFactory);

    m_pd2dFactory->CreateDrawingStateBlock(&m_pd2dsbDrawingState);
    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1BitmapSource, &m_pd2dfxBitmapSource);
    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1BitmapSource, &m_pd2dfxBitmapSource_jew);
    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1BitmapSource, &m_pd2dfxBitmapSource_logo);
    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1BitmapSource, &m_pd2dfxBitmapSource_nevi);
    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1BitmapSource, &m_pd2dfxBitmapSource_nevi2);



    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_pd2dfxGaussianBlur);
    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_pd2dfxGaussianBlur_jew);
    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_pd2dfxGaussianBlur_logo);

    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_pd2dfxGaussianBlur_nevi);
    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_pd2dfxGaussianBlur_nevi2);



    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1EdgeDetection, &m_pd2dfxSize);
    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1EdgeDetection, &m_pd2dfxSize_jew);
    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1EdgeDetection, &m_pd2dfxSize_logo);

    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1EdgeDetection, &m_pd2dfxSize_nevi);
    m_pd2dDeviceContext->CreateEffect(CLSID_D2D1EdgeDetection, &m_pd2dfxSize_nevi2);



    


    IWICBitmapDecoder* pwicBitmapDecoder;
    m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/mMap.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

    IWICBitmapFrameDecode* pwicFrameDecode;
    pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);

    m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter[0]);
    m_pwicFormatConverter[0]->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);

    m_pd2dfxBitmapSource->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter[0]);

    m_pd2dfxGaussianBlur->SetInputEffect(0, m_pd2dfxBitmapSource);
    m_pd2dfxGaussianBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

    m_pd2dfxSize->SetInputEffect(0, m_pd2dfxBitmapSource);
    m_pd2dfxSize->SetValue(D2D1_BITMAPSOURCE_PROP_SCALE, D2D1::Vector2F(0.1f, 0.1f));

    if (pwicBitmapDecoder) pwicBitmapDecoder->Release();
    if (pwicFrameDecode) pwicFrameDecode->Release();


    IWICBitmapDecoder* pwicBitmapDecoder_jewel;
    m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/jew1.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder_jewel);

    IWICBitmapFrameDecode* pwicFrameDecode_j;
    pwicBitmapDecoder_jewel->GetFrame(0, &pwicFrameDecode_j);

    m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter[1]);
    m_pwicFormatConverter[1]->Initialize(pwicFrameDecode_j, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);

    m_pd2dfxBitmapSource_jew->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter[1]);

    m_pd2dfxGaussianBlur_jew->SetInputEffect(0, m_pd2dfxBitmapSource_jew);
    m_pd2dfxGaussianBlur_jew->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);


    m_pd2dfxSize_jew->SetInputEffect(0, m_pd2dfxBitmapSource_jew);
    m_pd2dfxSize_jew->SetValue(D2D1_BITMAPSOURCE_PROP_SCALE, D2D1::Vector2F(0.1f, 0.1f));

   
    if (pwicBitmapDecoder_jewel) pwicBitmapDecoder_jewel->Release();
    if (pwicFrameDecode_j) pwicFrameDecode_j->Release();

    //==

    IWICBitmapDecoder* pwicBitmapDecoder_logo;
    m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/clear.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder_logo);

    IWICBitmapFrameDecode* pwicFrameDecode_lo;
    pwicBitmapDecoder_logo->GetFrame(0, &pwicFrameDecode_lo);

    m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter[1]);
    m_pwicFormatConverter[1]->Initialize(pwicFrameDecode_lo, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);

    m_pd2dfxBitmapSource_logo->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter[1]);

    m_pd2dfxGaussianBlur_logo->SetInputEffect(0, m_pd2dfxBitmapSource_logo);
    m_pd2dfxGaussianBlur_logo->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);


    m_pd2dfxSize_logo->SetInputEffect(0, m_pd2dfxBitmapSource_logo);
    m_pd2dfxSize_logo->SetValue(D2D1_BITMAPSOURCE_PROP_SCALE, D2D1::Vector2F(0.1f, 0.1f));


    if (pwicBitmapDecoder_logo) pwicBitmapDecoder_logo->Release();
    if (pwicFrameDecode_lo) pwicFrameDecode_lo->Release();

    //==

    IWICBitmapDecoder* pwicBitmapDecoder_nevi;
    m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/nevi.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder_nevi);

    IWICBitmapFrameDecode* pwicFrameDecode_n;
    pwicBitmapDecoder_nevi->GetFrame(0, &pwicFrameDecode_n);

    m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter[2]);
    m_pwicFormatConverter[2]->Initialize(pwicFrameDecode_n, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);

    m_pd2dfxBitmapSource_nevi->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter[2]);

    m_pd2dfxGaussianBlur_nevi->SetInputEffect(0, m_pd2dfxBitmapSource_nevi);
    m_pd2dfxGaussianBlur_nevi->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);


    m_pd2dfxSize_nevi->SetInputEffect(0, m_pd2dfxBitmapSource_nevi);
    m_pd2dfxSize_nevi->SetValue(D2D1_BITMAPSOURCE_PROP_SCALE, D2D1::Vector2F(0.1f, 0.1f));



    if (pwicBitmapDecoder_nevi) pwicBitmapDecoder_nevi->Release();
    if (pwicFrameDecode_n) pwicFrameDecode_n->Release();

    //==
    IWICBitmapDecoder* pwicBitmapDecoder_nevi2;
    m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/nevi2.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder_nevi2);

    IWICBitmapFrameDecode* pwicFrameDecode_n2;
    pwicBitmapDecoder_nevi2->GetFrame(0, &pwicFrameDecode_n2);

    m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter[3]);
    m_pwicFormatConverter[3]->Initialize(pwicFrameDecode_n2, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);

    m_pd2dfxBitmapSource_nevi2->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter[3]);

    m_pd2dfxGaussianBlur_nevi2->SetInputEffect(0, m_pd2dfxBitmapSource_nevi2);
    m_pd2dfxGaussianBlur_nevi2->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);


    m_pd2dfxSize_nevi2->SetInputEffect(0, m_pd2dfxBitmapSource_nevi2);
    m_pd2dfxSize_nevi2->SetValue(D2D1_BITMAPSOURCE_PROP_SCALE, D2D1::Vector2F(0.1f, 0.1f));



    if (pwicBitmapDecoder_nevi2) pwicBitmapDecoder_nevi2->Release();
    if (pwicFrameDecode_n2) pwicFrameDecode_n2->Release();


}

void UILayer::DrawDot(int dotCnt, XMFLOAT3[])
{
    for (int i= 0; i < dotCnt; ++i) {
        m_pd2dDeviceContext->FillEllipse(D2D1::Ellipse(D2D1::Point2F(100.0f, FRAME_BUFFER_HEIGHT / 2.0f + 100.0f), 5.0f, 5.0f), Redbrush); // 이게 중심
    }
}

void UILayer::UpdateLabels(const wstring& strUIText)
{
    m_vTextBlocks[0] = { strUIText, D2D1::RectF(0.0f, 25.0f, m_fWidth, m_fHeight), m_pdwTextFormat }; 
}

void UILayer::UpdateLabels_Scripts(const std::wstring& strUIText)
{
    m_vScriptsBlocks[0] = { strUIText, D2D1::RectF(0.0f, FRAME_BUFFER_HEIGHT - 100, m_fWidth, FRAME_BUFFER_HEIGHT - 20), m_pdwScriptsFormat };
}

void UILayer::UpdateLabels_Jew(const std::wstring& strUIText)
{
    m_vJewBlocks[0] = { strUIText, D2D1::RectF(15.0f, FRAME_BUFFER_HEIGHT / 9, FRAME_BUFFER_WIDTH / 16, FRAME_BUFFER_HEIGHT), m_pdwJewFormat };
}

void UILayer::UpdateDots(int id, CAirplanePlayer* player, XMFLOAT3& epos, bool live)
{
    XMFLOAT3 ppos = player->GetPosition();

    XMFLOAT3 playerf = player->GetLookVector();
    XMVECTOR playerPos = XMLoadFloat3(&ppos);
    XMVECTOR enemyPos = XMLoadFloat3(&epos);


    XMMATRIX playerTransform = XMMatrixTranslation(ppos.x, ppos.y, ppos.z);
    XMMATRIX playerInverse = XMMatrixInverse(nullptr, playerTransform);

    XMVECTOR enemyPosVector = XMLoadFloat3(&epos);
    XMVECTOR relativePosVector = XMVector3TransformCoord(enemyPosVector, playerInverse);
    XMFLOAT3 relativePos;
    XMStoreFloat3(&relativePos, relativePosVector);


    //플레이어의 방향 벡터를 이용하여 상대적인 위치 벡터를 계산
    XMVECTOR playerForward = XMLoadFloat3(&playerf);  // 플레이어의 방향 벡터
    XMVECTOR enemyRelativePos/* = enemyPos - playerPos*/;  // 상대적인 위치 벡터
    enemyRelativePos = XMVector3InverseRotate(relativePosVector, playerForward); //적의 위치 벡터를 플레이어의 방향 벡터의 역방향으로 회전시켜 상대적인 위치 벡터를 계산

    // 미니맵 중심점을 원점으로 하는 극좌표계로 변환
    float enemyDistance = XMVectorGetX(XMVector3Length(enemyRelativePos));  // 적과의 거리
    float enemyAngle = atan2f(XMVectorGetZ(enemyRelativePos), XMVectorGetX(enemyRelativePos));  // 적과의 각도
    XMFLOAT2 enemyMapPos = XMFLOAT2(enemyDistance * cosf(enemyAngle), enemyDistance * sinf(enemyAngle));  // 극좌표를 직교좌표로 변환


    XMFLOAT3 enemyMap3Pos(enemyMapPos.x, 0.0f, enemyMapPos.y);


    XMFLOAT3 cpos;
    cpos = enemyMap3Pos;

    float mapScale = 0.3f;

    cpos.x = cpos.x * mapScale;
    cpos.z = cpos.z * mapScale;

    cpos.x = cpos.x + 100.0f;
    cpos.z = cpos.z + FRAME_BUFFER_HEIGHT / 2.0f * 1.75;

    if (id == BOSS_ID)
    {
        m_bossDot.x = cpos.x;
        m_bossDot.z = cpos.z;
    }


    if (live == false)
    {
        m_enemyDot[id].x = 100.0f;
        m_enemyDot[id].z = FRAME_BUFFER_HEIGHT / 2.0f + 100.0f;
    }

    else if (!(cpos.x > 200.0f || cpos.x <-200.0f || cpos.z > FRAME_BUFFER_HEIGHT / 2.0f * 3.0 || cpos.z < -(FRAME_BUFFER_HEIGHT / 2.0f * 3.0))&& id!=BOSS_ID)
    {
        m_enemyDot[id].x = cpos.x;
        m_enemyDot[id].z = cpos.z;
    }
    else if (id != BOSS_ID)
    {
        m_enemyDot[id].x = 100.0f;
        m_enemyDot[id].z = FRAME_BUFFER_HEIGHT / 2.0f  + 100.0f;
    }

    else {
        m_enemyDot[id].x = 0;
        m_enemyDot[id].z = 0;
    }



   // m_enemyDot[0] = { 100.0f, 0.0f,  FRAME_BUFFER_HEIGHT / 2.0f + 100.0f }; //중점
   //cout << id << "- id : " << m_enemyDot[id].x <<endl;
   //cout << id << "- id : " << m_enemyDot[id].z << endl;
}

void UILayer::UpdateBossNevi(int id, CAirplanePlayer* player, XMFLOAT3& bossPos)
{

    XMFLOAT3 ppos = player->GetPosition();
    XMFLOAT3 direction = player->GetLookVector();

    XMFLOAT3 playerToBoss = XMFLOAT3(bossPos.x - ppos.x, bossPos.y - ppos.y, bossPos.z - ppos.z);

    XMVECTOR playerToBossVec = XMLoadFloat3(&playerToBoss);
    XMVECTOR playerDirectionVec = XMLoadFloat3(&direction);
    playerToBossVec = XMVector3Normalize(playerToBossVec);
    playerDirectionVec = XMVector3Normalize(playerDirectionVec);

    angle = XMVectorGetX(XMVector3AngleBetweenNormals(playerToBossVec, playerDirectionVec));

    angle = XMConvertToDegrees(angle);

}

void UILayer::UpdatePlanetNevi(CAirplanePlayer* player, XMFLOAT3& lpos)
{
    XMFLOAT3 ppos = player->GetPosition();
    XMFLOAT3 direction = player->GetLookVector();

    XMFLOAT3 playerToPlanet = XMFLOAT3(lpos.x - ppos.x, lpos.y - ppos.y, lpos.z - ppos.z);

    XMVECTOR playerToBossVec = XMLoadFloat3(&playerToPlanet);
    XMVECTOR playerDirectionVec = XMLoadFloat3(&direction);
    playerToBossVec = XMVector3Normalize(playerToBossVec);
    playerDirectionVec = XMVector3Normalize(playerDirectionVec);

    angle2 = XMVectorGetX(XMVector3AngleBetweenNormals(playerToBossVec, playerDirectionVec));

    angle2 = XMConvertToDegrees(angle2);
}

float UILayer::UpdatePlanetDist(CAirplanePlayer* player, XMFLOAT3& lpos)
{
    XMFLOAT3 ppos = player->GetPosition();

    XMVECTOR v1 = XMLoadFloat3(&ppos);
    XMVECTOR v2 = XMLoadFloat3(&lpos);
    XMVECTOR dist = XMVector3Length(XMVectorSubtract(v2, v1));

    float distance;
    XMStoreFloat(&distance, dist);

    return distance;
}



void UILayer::UpdateHp(short curhp, short maxHp)
{
    hpBar =(hpbarRight - hpbarLeft) - (float(curhp) / float(maxHp)) * (hpbarRight - hpbarLeft);
}

void UILayer::UpdateBossHp(short curhp, short maxHp)
{

   // BosshpBar = float(maxHp - curhp) * 1.5;

    BosshpBar = (bossHpbarRight - bossHpbarLeft) - (float(curhp) / float(maxHp)) * (bossHpbarRight - bossHpbarLeft);


}

XMFLOAT4X4 UILayer::UpdateMat(const XMFLOAT3& pos)
{
    // XMFLOAT3을 XMFLOAT4로 변환
    XMFLOAT4 position4(pos.x, pos.y, pos.z, 1.0f);

    // XMFLOAT4X4를 초기화
    XMFLOAT4X4 matrix;
    memset(&matrix, 0, sizeof(matrix));


    // 변환 행렬의 각 열에 XMFLOAT4를 대입
    matrix._11 = 1.0f; matrix._12 = 0.0f; matrix._13 = 0.0f; matrix._14 = 0.0f;
    matrix._21 = 0.0f; matrix._22 = 1.0f; matrix._23 = 0.0f; matrix._24 = 0.0f;
    matrix._31 = 0.0f; matrix._32 = 0.0f; matrix._33 = 1.0f; matrix._34 = 0.0f;
    matrix._41 = 0.0f; matrix._42 = 0.0f; matrix._43 = 0.0f; matrix._44 = 1.0f;

    // 변환 행렬의 마지막 열에 XMFLOAT4를 대입
    matrix._14 = position4.x;
    matrix._24 = position4.y;
    matrix._34 = position4.z;
    matrix._44 = position4.w;

    return matrix;
}

void UILayer::Render(UINT nFrame, MissionType mty, BossState bst)
{
    ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_vd2dRenderTargets[nFrame]);

    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));

    m_pd2dDeviceContext->BeginDraw();
    for (auto textBlock : m_vTextBlocks)
    { 
        m_pd2dDeviceContext->DrawText(textBlock.strText.c_str(), static_cast<UINT>(textBlock.strText.length()), textBlock.pdwFormat, textBlock.d2dLayoutRect, m_pd2dTextBrush);
    }

    for (auto textBlock : m_vScriptsBlocks)
    {
        m_pd2dDeviceContext->DrawText(textBlock.strText.c_str(), static_cast<UINT>(textBlock.strText.length()), textBlock.pdwFormat, textBlock.d2dLayoutRect, m_pd2dTextBrush);
    }

    for (auto textBlock : m_vJewBlocks)
    {
        m_pd2dDeviceContext->DrawText(textBlock.strText.c_str(), static_cast<UINT>(textBlock.strText.length()), textBlock.pdwFormat, textBlock.d2dLayoutRect, m_pd2dTextBrush);
    }

    D2D_POINT_2F d2dPoint = { 0.0f, FRAME_BUFFER_HEIGHT/2 * 1.5f };

    D2D_POINT_2F d2dPoint_jew = { 0.0f, 55.0f };

    D2D_POINT_2F d2dPoint_nevi = { -25, -75 };

    D2D_POINT_2F d2dPoint_logo = { FRAME_BUFFER_WIDTH/2 - 512, FRAME_BUFFER_HEIGHT /2 - 128};




    D2D_RECT_F d2dRect = { 0.0f, 0.0f, 100.0f, 200.0f };

    D2D1::Matrix3x2F matScale, matTranslation, matRot, matRot2, matTM, matTM2;
    matScale = D2D1::Matrix3x2F::Scale(0.05f, 0.05f);
    matTranslation = D2D1::Matrix3x2F::Translation(FRAME_BUFFER_WIDTH / 2.0f, FRAME_BUFFER_HEIGHT / 2.0f);
    matRot = D2D1::Matrix3x2F::Rotation(angle);
    matTM = matRot * matTranslation ;

    matRot2 = D2D1::Matrix3x2F::Rotation(angle2);
    matTM2 = matRot2 * matTranslation;


    //int a = m_pd2dDeviceContext->GetSize().height;
    //int b = m_pd2dDeviceContext->GetSize().width;
    //cout << "height : " << a << endl;
    //cout << "width : " << b << endl;
    m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
    m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur, &d2dPoint);
    m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur_jew, &d2dPoint_jew);

    if(bst ==BossState::DIE)
        m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur_logo, &d2dPoint_logo);

    m_pd2dDeviceContext->SetTransform(matTM);

   // m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Rotation(angle, a));

   /* if(mty==MissionType::FIND_BOSS)
     m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur_nevi, &d2dPoint_nevi);
    m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
    m_pd2dDeviceContext->SetTransform(matTM2);
    if (mty == MissionType::GO_PLANET) {
        m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur_nevi2, &d2dPoint_nevi);
    }*/

    m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());





    for (auto& a : m_enemyDot)
    {
        if(!(a.x == 100.0f && a.z == FRAME_BUFFER_HEIGHT / 2.0f + 100.0f))
            m_pd2dDeviceContext->FillEllipse(D2D1::Ellipse(D2D1::Point2F(a.x, a.z), 5.0f, 5.0f), Redbrush);
    }


  //  if (!(m_bossDot.x == 100.0f && m_bossDot.z == FRAME_BUFFER_HEIGHT / 2.0f + 100.0f))
    m_pd2dDeviceContext->FillEllipse(D2D1::Ellipse(D2D1::Point2F(m_bossDot.x, m_bossDot.z), 5.0f, 5.0f), DotBossbrush);



    m_pd2dDeviceContext->FillRectangle(D2D1::RectF(hpbarLeft, FRAME_BUFFER_HEIGHT - 60, hpbarRight, FRAME_BUFFER_HEIGHT - 50), Whitebrush); //배경
    if(hpbarRight - hpBar> hpbarLeft)
        m_pd2dDeviceContext->FillRectangle(D2D1::RectF(hpbarLeft, FRAME_BUFFER_HEIGHT - 60, hpbarRight - hpBar, FRAME_BUFFER_HEIGHT - 50), Redbrush); // hp


    if (mty == MissionType::DEFEAT_BOSS) {
        m_pd2dDeviceContext->FillRectangle(D2D1::RectF(bossHpbarLeft, bossHpbarTop, bossHpbarRight, bossHpbarBottom), Whitebrush); //배경
        if (bossHpbarRight - BosshpBar > bossHpbarLeft)
            m_pd2dDeviceContext->FillRectangle(D2D1::RectF(bossHpbarLeft, bossHpbarTop, bossHpbarRight - BosshpBar, bossHpbarBottom), Redbrush); // hp
    }



    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();


}

void UILayer::ReleaseResources()
{
    for (UINT i = 0; i < GetRenderTargetsCount(); i++)
    {
        ID3D11Resource* ppResources[] = { m_vWrappedRenderTargets[i] };
        m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    }
    m_pd2dDeviceContext->SetTarget(nullptr);
    m_pd3d11DeviceContext->Flush();
    for (UINT i = 0; i < GetRenderTargetsCount(); i++)
    {
        m_vd2dRenderTargets[i]->Release();
        m_vWrappedRenderTargets[i]->Release();
    }
    m_pd2dTextBrush->Release();
    Redbrush->Release();
    Whitebrush->Release();

    DotBossbrush->Release();

    m_pd2dDeviceContext->Release();
    m_pdwTextFormat->Release();
    m_pdwScriptsFormat->Release();
    m_pdwJewFormat->Release();


    m_pd2dWriteFactory->Release();
    m_pd2dDevice->Release();
    m_pd2dFactory->Release();
    m_pd3d11DeviceContext->Release();
    m_pd3d11On12Device->Release();
}

void UILayer::Resize(ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    m_fWidth = static_cast<float>(nWidth);
    m_fHeight = static_cast<float>(nHeight);

    D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

    for (UINT i = 0; i < GetRenderTargetsCount(); i++)
    {
        D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
        m_pd3d11On12Device->CreateWrappedResource(ppd3dRenderTargets[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&m_vWrappedRenderTargets[i]));
        IDXGISurface* pdxgiSurface = NULL;
        m_vWrappedRenderTargets[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);
        m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface, &d2dBitmapProperties, &m_vd2dRenderTargets[i]);
        pdxgiSurface->Release();
    }

    if (m_pd2dDeviceContext) m_pd2dDeviceContext->Release();
    m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pd2dDeviceContext);
    m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    if (m_pd2dTextBrush) m_pd2dTextBrush->Release();
    m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pd2dTextBrush);

    const float fFontSize = m_fHeight / 25.0f;
    const float fFontSize_Scripts = m_fHeight / 25.0f;

    const float fSmallFontSize = m_fHeight / 40.0f;

    m_pd2dWriteFactory->CreateTextFormat(L"굴림체", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", &m_pdwTextFormat);

    m_pdwTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_pdwTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    m_pd2dWriteFactory->CreateTextFormat(L"굴림체", nullptr, DWRITE_FONT_WEIGHT_MEDIUM, DWRITE_FONT_STYLE_ITALIC, DWRITE_FONT_STRETCH_NORMAL, fFontSize_Scripts, L"en-us", &m_pdwScriptsFormat);

    m_pdwScriptsFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_pdwScriptsFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);


    m_pd2dWriteFactory->CreateTextFormat(L"굴림체", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", &m_pdwJewFormat);

    m_pdwJewFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_pdwJewFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);


    //    m_pd2dWriteFactory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fSmallFontSize, L"en-us", &m_pdwTextFormat);
}

//=============================================

CUI::CUI()
{
}
CUI::~CUI()
{

}
CUI::CUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int num, UINT nWidth, UINT nHeight, UINT nDepth) : CGameObject(1)
{
    CTexturedRectMesh* pUIMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, nWidth, nHeight, nDepth);
    SetMesh(pUIMesh);

    CreateShaderVariables(pd3dDevice, pd3dCommandList);

    m_ppUITexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
    m_ppUITexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/crosshair.dds", 0);
    
    CUIShader* m_pUIShader;
    CMaterial* m_pUIMaterial;

    if (num == static_cast<int>(UIType::CROSSHAIR)) {
        m_ppUITexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
        m_ppUITexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/crosshair.dds", 0);
    }
    
    if (num == static_cast<int>(UIType::SCRATCH)) {
        m_ppUITexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
        m_ppUITexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/scratch.dds", 0);
    }

    if (num == static_cast<int>(UIType::HP)) {
        m_ppUITexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
        m_ppUITexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/hp.dds", 0);
    }
    
    if (num == static_cast<int>(UIInsideType::FSIT))
    {
        m_ppUITexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
        m_ppUITexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/f.dds", 0);
    }

    if (num == static_cast<int>(UIInsideType::NAME_1))
    {
        m_ppUITexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
        m_ppUITexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/1p.dds", 0);
    }
    if (num == static_cast<int>(UIInsideType::NAME_2))
    {
        m_ppUITexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
        m_ppUITexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/2p.dds", 0);
    }   
    if (num == static_cast<int>(UIInsideType::NAME_3))
    {
        m_ppUITexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
        m_ppUITexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/3p.dds", 0);
    }

    /*m_ppUITexture[static_cast<int>(UIType::CROSSHAIR)] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
    m_ppUITexture[static_cast<int>(UIType::CROSSHAIR)]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/crosshair.dds", 0);

    m_ppUITexture[static_cast<int>(UIType::MINIMAP)] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
    m_ppUITexture[static_cast<int>(UIType::MINIMAP)]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/minimap.dds", 0);

    m_ppUITexture[static_cast<int>(UIType::HP)] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
    m_ppUITexture[static_cast<int>(UIType::HP)]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"UI/hp.dds", 0);*/


    m_pUIShader = new CUIShader();

    m_pUIShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
    m_pUIShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

  
     CScene::CreateShaderResourceViews(pd3dDevice, m_ppUITexture, 15, false);

    m_pUIMaterial = new CMaterial(1);
    m_pUIMaterial->SetTexture(m_ppUITexture, 0);
    m_pUIMaterial->SetShader(m_pUIShader);
    SetMaterial(0, m_pUIMaterial);

}

void CUI::MinmapUpdate()
{

}

void CUI::HpbarUpdate(XMFLOAT3 pos, short MaxHp, short curHp) //일단 냅다 가운데임
{
    float ratioHp = float(curHp) / float(MaxHp);
    if (MaxHp == curHp)
        this->SetScale(1.0f, 1.0f, 1.0f);

    else if (curHp <= 0)
        this->SetScale(0.0f, 0.0f, 0.0f);

    else
    {
        this->SetScale(ratioHp, 1.0f, 1.0f);
        //this->SetPosition(pos.x + (ratioHp * 20), pos.y, pos.z);
    }
    
}


void CUI::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    //XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
    //SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f)); //빌보드



        //XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
        //CPlayer* pPlayer = pCamera->GetPlayer();
        //XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
        //XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
        //XMFLOAT3 xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 40.0f, false));
        //xmf3Position.y += 10.0f;
        //SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
        //SetPosition(xmf3Position);
        //

    //XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
    //XMFLOAT3 xmf3CameraLook = pCamera->GetLookVector();
    //CPlayer* pPlayer = pCamera->GetPlayer();
    //XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
    //XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
    //XMFLOAT3 xmf3Position = Vector3::Add(xmf3CameraPosition, Vector3::ScalarProduct(xmf3CameraLook, 50.0f, false));
    //SetPosition(xmf3Position);
    //SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));




    //cout << "player.x" << xmf3PlayerPosition.x<< endl;
    //cout << "player.y" << xmf3PlayerPosition.y << endl;
    //cout << "player.z" << xmf3PlayerPosition.z << endl;




    CGameObject::Render(pd3dCommandList, pCamera);


}



