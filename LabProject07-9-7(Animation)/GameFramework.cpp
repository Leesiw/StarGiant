//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"

CGameFramework::CGameFramework()
{
	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;

	for (int i = 0; i < m_nSwapChainBuffers; i++) m_ppd3dSwapChainBackBuffers[i] = NULL;
	m_nSwapChainBufferIndex = 0;

	m_pd3dCommandAllocator = NULL;
	m_pd3dCommandQueue = NULL;
	m_pd3dCommandList = NULL;

	m_pd3dRtvDescriptorHeap = NULL;
	m_pd3dDsvDescriptorHeap = NULL;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pScene = NULL;
	for (int i = 0; i < 3; ++i)
		m_pPlayer[i] = NULL;

	m_pInsideScene = NULL;
	for(int i=0; i<3; ++i)
		m_pInsidePlayer[i] = NULL;

	items[ItemType::JEWEL_ATT] = 0;
	items[ItemType::JEWEL_DEF] = 0;
	items[ItemType::JEWEL_HEAL] = 0;
	items[ItemType::JEWEL_HP] = 0;
	
	cDirection = 0;

	planetPos = { 10000.0f,10000.0f,10000.0f };

	_state = SCENE_LOBBY;
	//_state = SCENE_INGAME;

	a = 0;
	b = 0;
	
	scriptsStartTime = steady_clock::now();
	_tcscpy_s(m_pszFrameRate, _T("StarGiant ("));
}

CGameFramework::~CGameFramework()
{
	if (isConnect)
	{
		/*
		CS_LOGOUT_PACKET packet{};
		packet.size = sizeof(packet);
		packet.type = CS_PACKET_LOGOUT;
		send(g_socket, reinterpret_cast<char*>(&packet), sizeof(packet), NULL);*/

		closesocket(sock);
		WSACleanup();
	}
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

	CoInitialize(NULL);

	CSound::Init();

	BuildObjects();
	isConnect = ConnectServer();

	BuildSounds();
	m_lobbybgm->play();

	return(true);
}

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1 **)&m_pdxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain **)&m_pdxgiSwapChain);
#endif
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug *pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void **)&m_pdxgiFactory);

	IDXGIAdapter1 *pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice))) break;
	}

	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void **)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice);
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **)&m_pd3dFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	HRESULT hResult;

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void **)&m_pd3dCommandQueue);

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&m_pd3dCommandAllocator);

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&m_pd3dCommandList);
	hResult = m_pd3dCommandList->Close();
}
void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
	// 그림자 맵을 위한 랜더 타겟 뷰 생성
	//m_pd3dDevice->CreateRenderTargetView(m_pd3dGodRayShadowMap, nullptr, d3dRtvCPUDescriptorHandle); // 추가된 부분
}


void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);

	//// 그림자 맵을 위한 렌더 타겟 텍스처 생성
	//D3D12_RESOURCE_DESC d3dShadowMapDesc;
	//ZeroMemory(&d3dShadowMapDesc, sizeof(D3D12_RESOURCE_DESC));
	//d3dShadowMapDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	//d3dShadowMapDesc.Alignment = 0;
	//d3dShadowMapDesc.Width = m_nWndClientWidth;
	//d3dShadowMapDesc.Height = m_nWndClientHeight;
	//d3dShadowMapDesc.DepthOrArraySize = 1;
	//d3dShadowMapDesc.MipLevels = 1;
	//d3dShadowMapDesc.Format = DXGI_FORMAT_R32_TYPELESS; // 그림자 맵의 포맷 설정
	//d3dShadowMapDesc.SampleDesc.Count = 1;
	//d3dShadowMapDesc.SampleDesc.Quality = 0;
	//d3dShadowMapDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	//d3dShadowMapDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//d3dClearValue.Format = DXGI_FORMAT_D32_FLOAT;


	//HRESULT hr = m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dShadowMapDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dGodRayShadowMap);
	//if (FAILED(hr))
	//{
	//	// 그림자 맵 생성에 실패한 경우 처리할 내용 나중에 쓰기 
	//}
}


void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);


	switch (nMessageID)
	{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			::SetCapture(hWnd);

			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			::ReleaseCapture();
			break;
		case WM_MOUSEMOVE:
			break;
		default:
			break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	//


	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;

		case '0':
		case VK_NUMPAD0:
			cout << "0";
			roomNum += L"0";
			break;

		case '1':
		case VK_NUMPAD1:
			cout << "1";
			roomNum += L"1";

			break;

		case '2':
		case VK_NUMPAD2:
			cout << "2";
			roomNum += L"2";
			break;

		case '3':
		case VK_NUMPAD3:
			cout << "3";
			roomNum += L"3";
			break;

		case '4':
		case VK_NUMPAD4:
			cout << "4";
			roomNum += L"4";
			break;

		case '5':
		case VK_NUMPAD5:
			cout << "5";
			roomNum += L"5";
			break;

		case '6':
		case VK_NUMPAD6:
			cout << "6";
			roomNum += L"6";
			break;
		case '7':
		case VK_NUMPAD7:
			cout << "7";
			roomNum += L"7";
			break;
		case '8':
		case VK_NUMPAD8:
			cout << "8";
			roomNum += L"8";
			break;
		case '9':
		case VK_NUMPAD9:
			cout << "9";
			roomNum += L"9";
			break;

		case VK_BACK:
		{
			if (!roomNum.empty())
			{
				roomNum.pop_back(); 
				cout << "백키\n"; 
			}
			break;
		}

		case VK_F1:
		case VK_F2:
		case VK_F3:
			m_pCamera = m_pPlayer[0]->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
			break;
		case VK_F4:
			m_pCamera = m_pPlayer[0]->ChangeCamera(DRIVE_CAMERA, m_GameTimer.GetTimeElapsed());
			break;
		case VK_F5:
			m_pCamera = m_pPlayer[0]->ChangeCamera(ATTACT_CAMERA_L, m_GameTimer.GetTimeElapsed());
			break;
		case VK_F6:
			m_pCamera = m_pPlayer[0]->ChangeCamera(ATTACT_CAMERA_C, m_GameTimer.GetTimeElapsed());
			break;
		case VK_F7:
			m_pCamera = m_pPlayer[0]->ChangeCamera(ATTACT_CAMERA_R, m_GameTimer.GetTimeElapsed());
			break;
		case VK_F9:
			ChangeSwapChainState();
			break;
		case VK_CONTROL:
			((CAirplanePlayer*)m_pPlayer[0])->FireBullet(NULL);
			std::cout << "총알";
			break;
		case VK_SPACE: {
			// 이후 컷씬 중일때만 보낼 수 있게 변경
			CS_CUTSCENE_END_PACKET my_packet;
			my_packet.size = sizeof(CS_CUTSCENE_END_PACKET);
			my_packet.type = CS_CUTSCENE_END;
			send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
			break;
			/*
			if (((CTerrainPlayer*)m_pInsidePlayer[g_myid])->motion != AnimationState::SIT) {
				((CTerrainPlayer*)m_pInsidePlayer[g_myid])->motion = AnimationState::SIT;
				std::cout << "앉기";
			}
			else
			{
				std::cout << "서기";
				((CTerrainPlayer*)m_pInsidePlayer[g_myid])->motion = AnimationState::IDLE;
			}*/
		}
		case VK_TAB: {
			//b_Inside = !b_Inside;
			m_pInsideScene->m_pShadowMapToViewport->b_RTV1 = !m_pInsideScene->m_pShadowMapToViewport->b_RTV1;
			std::cout << "";
			break;
		}
		case 'I':
		{
			CS_NEXT_MISSION_PACKET my_packet;
			my_packet.size = sizeof(CS_NEXT_MISSION_PACKET);
			my_packet.type = CS_INVINCIBLE_MODE;

			send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
			break;
		}
		case 'R': //컷씬 테스트
		{

		if (b_Inside&& m_pInsidePlayer[g_myid]->GetCamera()->GetMode() != CUT_SCENE_CAMERA) { //내부일때
			m_pBeforeCamera = m_pInsidePlayer[g_myid]->GetCamera()->GetMode(); // 저장하고

			cout << "Inside m_pCamera->GetMode() - " << m_pInsideCamera->GetMode() << endl;
			b_BeforeCheckInside = true;
			b_Inside = false; // 외부로 이동시키고 끝나면 다시 내부로 이동시켜야됨
			m_pCamera->SetTarget(planetPos);
			m_pCamera->SetDist(1000.0f);
			cout << "m_pCamera->GetMode() - " << m_pCamera->GetMode() << endl;
			m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());

			/*m_pInsideCamera->SetTarget({ 414.456f,224.f,676.309f });
			m_pInsideCamera = m_pInsidePlayer[g_myid]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());*/
		}

		else if(!b_Inside && m_pPlayer[0]->GetCamera()->GetMode() != CUT_SCENE_CAMERA) { //외부일 때
			m_pBeforeCamera = m_pPlayer[0]->GetCamera()->GetMode();
			m_pCamera->SetTarget(m_pScene->m_ppGod->GetPosition());
			m_pCamera->SetDist(3000.0f);
			m_pCamera->canDolly = true; //줌
			m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
			cout << "m_pCamera->GetMode() - " << m_pInsideCamera->GetMode() << endl;

		}
		cout << "m_pBeforeCamera->GetMode() - " << m_pBeforeCamera << endl;
		cout << "r";

		break;
		}

		case 'E': //컷씬 테스트 돌아오기
		{
			if (m_pBeforeCamera != NULL) {
				if (b_Inside) {
					m_pInsideCamera = m_pInsidePlayer[g_myid]->ChangeCamera(m_pBeforeCamera, m_GameTimer.GetTimeElapsed());
					cout << "Inside m_pBeforeCamera->GetMode() - " << m_pBeforeCamera << endl;
				}
				else {
					if (b_BeforeCheckInside)
					{
						b_Inside = true;
						m_pInsideCamera = m_pInsidePlayer[g_myid]->ChangeCamera(m_pBeforeCamera, m_GameTimer.GetTimeElapsed());
						cout << "Inside m_pBeforeCamera->GetMode() - " << m_pBeforeCamera << endl;
					}
					else {
						m_pCamera = m_pPlayer[g_myid]->ChangeCamera(m_pBeforeCamera, m_GameTimer.GetTimeElapsed());
						cout << "m_pBeforeCamera->GetMode() - " << m_pBeforeCamera << endl;
					}

				}
			}
			cout << "e";

			break;
		}

		case 'M':
		{
			room_num = static_cast<short>(std::stoi(roomNum));
			CS_LOGIN_PACKET packet;
			packet.size = sizeof(packet);
			packet.type = CS_LOGIN;
			packet.room_id = room_num;
			if (_state == SCENE_LOBBY) {
				send(sock, reinterpret_cast<char*>(&packet), sizeof(packet), NULL);
			}
			break;
		}
		case 'P':
		{
			CS_NEXT_MISSION_PACKET packet;
			packet.size = sizeof(packet);
			packet.type = CS_START;
			if (_state == SCENE_LOBBY) {
				send(sock, reinterpret_cast<char*>(&packet), sizeof(packet), NULL);
			}

			break;
		}
		case 'N':
			if (_state == SCENE_INGAME) {
				CS_NEXT_MISSION_PACKET my_packet;
				my_packet.size = sizeof(CS_NEXT_MISSION_PACKET);
				my_packet.type = CS_NEXT_MISSION;

				send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
			}
			break;
		case 'F': //F키 상호작용 앉기
			if (b_Inside || m_pInsidePlayer[g_myid]->GetSitState())
			{
				int State = m_pInsideScene->CheckSitCollisions();
				if (!isConnect) {
					CheckSceneChange(m_pInsidePlayer[g_myid]->GetSitState(), State);
				}
				if (State == 3) {
					if (_state == SCENE_INGAME) {
						CS_CHANGE_PACKET my_packet;
						my_packet.size = sizeof(CS_CHANGE_PACKET);
						my_packet.type = CS_CHANGE;
						my_packet.player_type = PlayerType::MOVE;
						send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
					}
				}
				else if (State >= 0) {
					if (_state == SCENE_INGAME) {
						CS_CHANGE_PACKET my_packet;
						my_packet.size = sizeof(CS_CHANGE_PACKET);
						my_packet.type = CS_CHANGE;
						my_packet.player_type = (PlayerType)(State + 2);
						send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
					}
				}
			}
			//m_pScene->m_ppEnemies[0]->DieSprite(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature());
			//m_pScene->m_ppSprite[0]->CountDiedTime(3.0f);

			if (b_Inside) {
				scriptsOn = true;
			}


			if (player_type != PlayerType::INSIDE) {
				if (_state == SCENE_INGAME) {
					CS_CHANGE_PACKET my_packet;
					my_packet.size = sizeof(CS_CHANGE_PACKET);
					my_packet.type = CS_CHANGE;
					my_packet.player_type = PlayerType::INSIDE;
					send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
				}
			}
			break;

		}
		break;
	default:
		break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	
	switch (nMessageID)
	{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
				m_GameTimer.Stop();
			else
				m_GameTimer.Start();
			break;
		}
		case WM_SIZE:
			break;
		case WM_LBUTTONDOWN:
		{
			int mouseX = LOWORD(lParam);
			int mouseY = HIWORD(lParam);

			// 화면의 일정 부분을 나타내는 영역을 정의합니다.
			int screenAreaLeft = 500;
			int screenAreaTop = 650;
			int screenAreaRight = 1100;
			int screenAreaBottom = 800;


			int screenAreaLeft1 = 1100;
			int screenAreaTop1 = 550;
			int screenAreaRight1 = 1450;
			int screenAreaBottom1 = 650;

			int screenAreaLeft2 = 150;
			int screenAreaTop2 = 550;
			int screenAreaRight2 = 450;
			int screenAreaBottom2 = 650;

			if (_state == SCENE_LOBBY) {
				if (mouseX >= screenAreaLeft1 && mouseX <= screenAreaRight1 && mouseY >= screenAreaTop1 && mouseY <= screenAreaBottom1 && !roomNum.empty()) {
					room_num = static_cast<short>(std::stoi(roomNum));
					CS_LOGIN_PACKET packet;
					packet.size = sizeof(packet);
					packet.type = CS_LOGIN;
					packet.room_id = room_num;
					if (_state == SCENE_LOBBY) {
						send(sock, reinterpret_cast<char*>(&packet), sizeof(packet), NULL);
					}
					m_effectSound[static_cast<int>(Sounds::CLICK)]->play();
					cout << "매칭";
				}

				if (mouseX >= screenAreaLeft2 && mouseX <= screenAreaRight2 && mouseY >= screenAreaTop2 && mouseY <= screenAreaBottom2) {
					cout << "오토매칭";
					CS_LOGIN_PACKET packet;
					packet.size = sizeof(packet);
					packet.type = CS_LOGIN;
					packet.room_id = -1;
					if (_state == SCENE_LOBBY) {
						send(sock, reinterpret_cast<char*>(&packet), sizeof(packet), NULL);
					}
					m_effectSound[static_cast<int>(Sounds::CLICK)]->play();


				}

				// 나중에 수정해야됨
				if (mouseX >= screenAreaLeft && mouseX <= screenAreaRight && mouseY >= screenAreaTop && mouseY <= screenAreaBottom && matcnt > 0) {
					CS_NEXT_MISSION_PACKET packet;
					packet.size = sizeof(packet);
					packet.type = CS_START;
					if (_state == SCENE_LOBBY) {
						send(sock, reinterpret_cast<char*>(&packet), sizeof(packet), NULL);
					}
					m_effectSound[static_cast<int>(Sounds::CLICK)]->play();
					cout << "플레이";
				}
			}


			if (player_type >= PlayerType::ATTACK1 && player_type <= PlayerType::ATTACK3) {
				CS_ATTACK_PACKET packet;
				packet.size = sizeof(packet);
				packet.type = CS_ATTACK;

				packet.data.pos = m_pCamera->GetPosition();
				packet.data.direction = m_pCamera->GetLookVector();

				if (std::isnan(m_pCamera->GetPosition().x))cout << "server x nan!!\n";
				if (_state == SCENE_INGAME) {
					send(sock, reinterpret_cast<char*>(&packet), sizeof(packet), NULL);
				}
				m_effectSound[static_cast<int>(Sounds::GUN)]->play();
			}
			else if (player_type == PlayerType::INSIDE && AroundSculpture()) {
				CS_HEAL_PACKET my_packet;
				my_packet.size = sizeof(CS_HEAL_PACKET);
				my_packet.type = CS_HEAL;
				my_packet.start = true;
				if (_state == SCENE_INGAME) {
					send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
				}
			}
			break;
		}
        case WM_RBUTTONDOWN:
			_state = SCENE_INGAME;
			break;
        case WM_LBUTTONUP:
			if (isHealing && player_type == PlayerType::INSIDE && AroundSculpture()) {
				CS_HEAL_PACKET my_packet;
				my_packet.size = sizeof(CS_HEAL_PACKET);
				my_packet.type = CS_HEAL;
				my_packet.start = false;
				if (_state == SCENE_INGAME) {
					send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
				}
				isHealing = false;
			}
			break;
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:
			OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
            break;
        case WM_KEYDOWN:
			switch (wParam)
			{
				break;
			}
        case WM_KEYUP:
			OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
			break;
	}
	return(0);
}

void CGameFramework::CheckSceneChange(bool State, int num)
{
	if (State) 
	{
		b_CameraScene = true;
	}
	else
	{
		b_Inside = true;
		m_pInsideCamera->SceneTimer = 0;
	}




	if (num == 0)m_pCamera = m_pPlayer[g_myid]->ChangeCamera(ATTACT_CAMERA_L, m_GameTimer.GetTimeElapsed());
	if (num == 1)m_pCamera = m_pPlayer[g_myid]->ChangeCamera(ATTACT_CAMERA_C, m_GameTimer.GetTimeElapsed());
	if (num == 2)m_pCamera = m_pPlayer[g_myid]->ChangeCamera(ATTACT_CAMERA_R, m_GameTimer.GetTimeElapsed());
	if (num == 3)m_pCamera = m_pPlayer[g_myid]->ChangeCamera(DRIVE_CAMERA, m_GameTimer.GetTimeElapsed());

	if (num == 4)m_pCamera = m_pPlayer[g_myid]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());


//추가할 사항 없으면 F에 몰아넣기로 수정
}

bool CGameFramework::AroundSculpture()
{
	XMFLOAT3 xmf3Position = m_pInsidePlayer[g_myid]->GetPosition();
	if (323.f < xmf3Position.x && xmf3Position.x < 361.f && 643.f < xmf3Position.z && xmf3Position.z < 691.75f) {
		return true;
	}
	return false;
}

void CGameFramework::CameraUpdateChange()
{

	if (a==0 &&m_pPlayer[0]->getHp() < 90 && m_pCamera->m_bCameraShaking ==false) { 
		m_pCamera->maxShakingTime = 1.f;
		m_pCamera->m_bCameraShaking = true;
		a = 1;
	}
	if (a == 1 && m_pPlayer[0]->getHp() < 80 && m_pCamera->m_bCameraShaking == false) {
		m_pCamera->maxShakingTime = 1.f;
		m_pCamera->m_bCameraShaking = true;
		a = 2;
	}
	if (a == 2 && m_pPlayer[0]->getHp() < 50 && m_pCamera->m_bCameraShaking == false) {
		m_pCamera->maxShakingTime = 1.5f;
		m_pCamera->m_bCameraShaking = true;
		a = 3;
	}
	if (a == 3 && m_pPlayer[0]->getHp() < 20 && m_pCamera->m_bCameraShaking == false) {
		m_pCamera->maxShakingTime = 1.5f;
		m_pCamera->m_bCameraShaking = true;
		a = 4;
	}

	if (b==0 && curMissionType == MissionType::ESCAPE_BLACK_HOLE) {
		m_pCamera->maxShakingTime = 1.0f;
		m_pCamera->m_bCameraShaking = true;
		b = 1;
	}
	if (b == 1 && curMissionType == MissionType::ESCAPE_BLACK_HOLE) {
		XMVECTOR v1 = XMLoadFloat3(&m_pPlayer[0]->GetPosition());
		XMVECTOR v2 = XMLoadFloat3(&m_pScene->m_ppBlackhole->GetPosition());
		XMVECTOR dist = XMVector3Length(XMVectorSubtract(v2, v1));
		float distance;
		XMStoreFloat(&distance, dist);

		if (distance <= 300.0f) {
			m_pCamera->maxShakingTime = 0.5f;
			m_pCamera->m_bCameraShaking = true;
		}

	}

	//CS_TURN은 어차피 플레이어 전부 내부에서 시작함
	if (curMissionType == MissionType::CS_TURN && !iscut) {
		cout << "Inside m_pCamera->GetMode() - " << m_pInsideCamera->GetMode() << endl;
		cout << "CS_TURN\n";
		if (!isending) m_pBeforeCamera = m_pInsidePlayer[g_myid]->GetCamera()->GetMode();
		m_pInsideCamera->SetTarget({ 530.219f, 230.f, 593.263f });
		m_pInsideCamera->SetDist(-25.0f);
		m_pInsideCamera->canTurn = true;
		m_pInsideCamera = m_pInsidePlayer[g_myid]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		m_pInsideCamera->endc = false;

		iscut = true;
		isending = false;

		cout << "Inside m_pCamera->GetMode() - " << m_pInsideCamera->GetMode() << endl;
	}

	//CS_SHOW_PLANET 내부일때,
	if (curMissionType == MissionType::CS_SHOW_PLANET && b_Inside && m_pInsidePlayer[g_myid] && !iscut) {
		if (!isending) m_pBeforeCamera = m_pInsidePlayer[g_myid]->GetCamera()->GetMode(); // 저장하고
		b_BeforeCheckInside = true;
		b_Inside = false; // 외부로 이동시키고 끝나면 다시 내부로 이동시켜야됨
		m_pCamera->SetTarget(planetPos);
		m_pCamera->SetDist(1000.0f);
		m_pCamera->canDolly = true; //줌
		cout << "Inside m_pCamera->GetMode() - " << m_pCamera->GetMode() << endl;
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());

		iscut = true;
		isending = false;


	}	

	//CS_SHOW_PLANET 외부일때,
	else if (curMissionType == MissionType::CS_SHOW_PLANET && !b_Inside && !iscut) {
		if (!isending) m_pBeforeCamera = m_pPlayer[0]->GetCamera()->GetMode();
		m_pCamera->SetTarget(planetPos);
		m_pCamera->SetDist(1000.0f);
		m_pCamera->canDolly = true; //줌
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		cout << "m_pCamera->GetMode() - " << m_pInsideCamera->GetMode() << endl;
		iscut = true;
		isending = false;


	}


	//CS_BOSS_SCREAM 내부일때,
	if (curMissionType == MissionType::CS_BOSS_SCREAM && b_Inside && m_pInsidePlayer[g_myid] && !iscut) {
		if (!isending) m_pBeforeCamera = m_pInsidePlayer[g_myid]->GetCamera()->GetMode(); // 저장하고
		b_BeforeCheckInside = true;
		b_Inside = false; // 외부로 이동시키고 끝나면 다시 내부로 이동시켜야됨
		m_pCamera->canDolly = true; //줌
		m_pCamera->SetTarget(m_pScene->m_ppBoss->GetPosition());
		m_pCamera->SetDist(1500.0f);
		cout << "Inside m_pCamera->GetMode() - " << m_pCamera->GetMode() << endl;
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		iscut = true;
		isending = false;


	}

	//CS_BOSS_SCREAM 외부일때,
	else if (curMissionType == MissionType::CS_BOSS_SCREAM && !b_Inside && !iscut) {
		if (!isending) m_pBeforeCamera = m_pPlayer[0]->GetCamera()->GetMode();
		m_pCamera->SetTarget(m_pScene->m_ppBoss->GetPosition());
		m_pCamera->SetDist(1500.0f);
		m_pCamera->canDolly = true; //줌
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		cout << "m_pCamera->GetMode() - " << m_pInsideCamera->GetMode() << endl;
		iscut = true;
		isending = false;


	}
	//CS_ANGRY_BOSS 내부일때,
	if (curMissionType == MissionType::CS_ANGRY_BOSS && b_Inside && m_pInsidePlayer[g_myid] && !iscut) {
		if (!isending) m_pBeforeCamera = m_pInsidePlayer[g_myid]->GetCamera()->GetMode(); // 저장하고
		b_BeforeCheckInside = true;
		b_Inside = false; // 외부로 이동시키고 끝나면 다시 내부로 이동시켜야됨
		m_pCamera->canDolly = true; //줌
		m_pCamera->SetTarget(m_pScene->m_ppBoss->GetPosition());
		m_pCamera->SetDist(1500.0f);
		cout << "Inside m_pCamera->GetMode() - " << m_pCamera->GetMode() << endl;
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		iscut = true;
		isending = false;


	}

	//CS_ANGRY_BOSS 외부일때,
	else if (curMissionType == MissionType::CS_ANGRY_BOSS && !b_Inside && !iscut) {
		if (!isending) m_pBeforeCamera = m_pPlayer[0]->GetCamera()->GetMode();
		m_pCamera->SetTarget(m_pScene->m_ppBoss->GetPosition());
		m_pCamera->SetDist(1500.0f);
		m_pCamera->canDolly = true; //줌
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		cout << "m_pCamera->GetMode() - " << m_pInsideCamera->GetMode() << endl;
		iscut = true;
		isending = false;


	}

	//CS_SHOW_STARGIANT 내부일때,
	if (curMissionType == MissionType::CS_SHOW_STARGIANT && b_Inside && m_pInsidePlayer[g_myid] && !iscut) {
		if (!isending) m_pBeforeCamera = m_pInsidePlayer[g_myid]->GetCamera()->GetMode(); // 저장하고
		b_BeforeCheckInside = true;
		b_Inside = false; // 외부로 이동시키고 끝나면 다시 내부로 이동시켜야됨
		m_pCamera->canDolly = true; //줌
		m_pCamera->SetTarget(m_pScene->m_ppBoss->GetPosition());
		m_pCamera->SetDist(1300.0f);
		cout << "Inside m_pCamera->GetMode() - " << m_pCamera->GetMode() << endl;
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		iscut = true;
		isending = false;


	}

	//CS_SHOW_STARGIANT 외부일때,
	else if (curMissionType == MissionType::CS_SHOW_STARGIANT && !b_Inside && !iscut) {
		if (!isending) m_pBeforeCamera = m_pPlayer[0]->GetCamera()->GetMode();
		m_pCamera->SetTarget(m_pScene->m_ppBoss->GetPosition());
		m_pCamera->SetDist(1300.0f);
		m_pCamera->canDolly = true; //줌
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		cout << "m_pCamera->GetMode() - " << m_pInsideCamera->GetMode() << endl;
		iscut = true;
		isending = false;


	}

	//CS_SHOW_BLACK_HOLE 내부일때,
	if (curMissionType == MissionType::CS_SHOW_BLACK_HOLE && b_Inside && m_pInsidePlayer[g_myid] && !iscut) {
		if (!isending) m_pBeforeCamera = m_pInsidePlayer[g_myid]->GetCamera()->GetMode(); // 저장하고
		b_BeforeCheckInside = true;
		b_Inside = false; // 외부로 이동시키고 끝나면 다시 내부로 이동시켜야됨
		m_pCamera->canDolly = true; //줌
		m_pCamera->SetTarget(m_pScene->m_ppBlackhole->GetPosition());
		m_pCamera->SetDist(100.0f);
		cout << "Inside m_pCamera->GetMode() - " << m_pCamera->GetMode() << endl;
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		iscut = true;
		isending = false;

		m_effectSound[static_cast<int>(Sounds::DARK)]->play();
	}

	//CS_SHOW_BLACK_HOLE 외부일때,
	else if (curMissionType == MissionType::CS_SHOW_BLACK_HOLE && !iscut) {
		if (!isending) m_pBeforeCamera = m_pPlayer[0]->GetCamera()->GetMode();
		m_pCamera->SetTarget(m_pScene->m_ppBlackhole->GetPosition());
		m_pCamera->SetDist(100.0f);
		m_pCamera->canDolly = true; //줌
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		cout << "m_pCamera->GetMode() - " << m_pInsideCamera->GetMode() << endl;
		iscut = true;
		isending = false;

		m_effectSound[static_cast<int>(Sounds::DARK)]->play();

	}


	//CS_SHOW_GOD 내부일때,
	if (curMissionType == MissionType::CS_SHOW_GOD && b_Inside && m_pInsidePlayer[g_myid] && !iscut) {
		if (!isending) m_pBeforeCamera = m_pInsidePlayer[g_myid]->GetCamera()->GetMode(); // 저장하고
		b_BeforeCheckInside = true;
		b_Inside = false; // 외부로 이동시키고 끝나면 다시 내부로 이동시켜야됨
		m_pCamera->canDolly = true; //줌
		m_pCamera->SetTarget(m_pScene->m_ppGod->GetPosition());
		m_pCamera->SetDist(500.0f);
		cout << "Inside m_pCamera->GetMode() - " << m_pCamera->GetMode() << endl;
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		iscut = true;
		isending = false;


	}

	//CS_SHOW_GOD 외부일때,
	else if (curMissionType == MissionType::CS_SHOW_GOD && !b_Inside && !iscut) {
		if (!isending) m_pBeforeCamera = m_pPlayer[0]->GetCamera()->GetMode();
		m_pCamera->SetTarget(m_pScene->m_ppGod->GetPosition());
		m_pCamera->SetDist(500.0f);
		m_pCamera->canDolly = true; //줌
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		cout << "m_pCamera->GetMode() - " << m_pInsideCamera->GetMode() << endl;
		iscut = true;
		isending = false;


	}

	//CS_ANGRY_GOD 내부일때,
	if (curMissionType == MissionType::CS_ANGRY_GOD && b_Inside && m_pInsidePlayer[g_myid] && !iscut) {
		if (!isending) m_pBeforeCamera = m_pInsidePlayer[g_myid]->GetCamera()->GetMode(); // 저장하고
		b_BeforeCheckInside = true;
		b_Inside = false; // 외부로 이동시키고 끝나면 다시 내부로 이동시켜야됨
		m_pCamera->canDolly = true; //줌
		m_pCamera->SetTarget(m_pScene->m_ppGod->GetPosition());
		m_pCamera->SetDist(2500.0f);
		cout << "Inside m_pCamera->GetMode() - " << m_pCamera->GetMode() << endl;
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		iscut = true;
		isending = false;


	}

	//CS_ANGRY_GOD 외부일때,
	else if (curMissionType == MissionType::CS_ANGRY_GOD && !b_Inside && !iscut) {
		if (!isending) m_pBeforeCamera = m_pPlayer[0]->GetCamera()->GetMode();
		m_pCamera->SetTarget(m_pScene->m_ppGod->GetPosition());
		m_pCamera->SetDist(2500.0f);
		m_pCamera->canDolly = true; //줌
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		cout << "m_pCamera->GetMode() - " << m_pInsideCamera->GetMode() << endl;
		iscut = true;
		isending = false;


	}
	

	//CS_BAD_ENDING 내부일때,
	if (curMissionType == MissionType::CS_BAD_ENDING && b_Inside && m_pInsidePlayer[g_myid] && !iscut) {
		if (!isending) m_pBeforeCamera = m_pInsidePlayer[g_myid]->GetCamera()->GetMode(); // 저장하고
		b_BeforeCheckInside = true;
		b_Inside = false; // 외부로 이동시키고 끝나면 다시 내부로 이동시켜야됨
		m_pCamera->SetTarget(m_pPlayer[0]->GetPosition());
		m_pCamera->SetDist(500.0f);

		cout << "Inside m_pCamera->GetMode() - " << m_pCamera->GetMode() << endl;
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		iscut = true;
		isending = false;

		m_effectSound[static_cast<int>(Sounds::BAD)]->play();
	}

	//CS_BAD_ENDING 외부일때,
	else if (curMissionType == MissionType::CS_BAD_ENDING && !b_Inside && !iscut) {
		if (!isending) m_pBeforeCamera = m_pPlayer[0]->GetCamera()->GetMode();
		m_pCamera->SetTarget(m_pPlayer[0]->GetPosition());
		m_pCamera->SetDist(500.0f);

		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		cout << "m_pCamera->GetMode() - " << m_pInsideCamera->GetMode() << endl;
		iscut = true;
		isending = false;

		m_effectSound[static_cast<int>(Sounds::BAD)]->play();

	}

	//CS_ENDING 내부일때,
	if (curMissionType == MissionType::CS_ENDING && b_Inside && m_pInsidePlayer[g_myid] && !iscut) {
		if (!isending) m_pBeforeCamera = m_pInsidePlayer[g_myid]->GetCamera()->GetMode(); // 저장하고
		b_BeforeCheckInside = true;
		b_Inside = false; // 외부로 이동시키고 끝나면 다시 내부로 이동시켜야됨
		m_pCamera->endc = true;
		m_pCamera->SetTarget({ m_pScene->m_ppGod->GetPosition().x, m_pScene->m_ppGod->GetPosition().y, m_pScene->m_ppGod->GetPosition().z });
		m_pCamera->SetDist(1500.f); 
		cout << "Inside m_pCamera->GetMode() - " << m_pCamera->GetMode() << endl;
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		iscut = true;
		isending = false;


	}

	//CS_ENDING 외부일때,
	else if (curMissionType == MissionType::CS_ENDING && !b_Inside && !iscut) {
		if (!isending) m_pBeforeCamera = m_pPlayer[0]->GetCamera()->GetMode();
		m_pCamera->SetTarget({ m_pScene->m_ppGod->GetPosition().x, m_pScene->m_ppGod->GetPosition().y, m_pScene->m_ppGod->GetPosition().z });
		m_pCamera->SetDist(1500.f);
		m_pCamera->endc = true;
		m_pCamera = m_pPlayer[0]->ChangeToCutSceneCamera(CUT_SCENE_CAMERA, m_GameTimer.GetTimeElapsed());
		cout << "m_pCamera->GetMode() - " << m_pInsideCamera->GetMode() << endl;
		iscut = true;
		isending = false;
	}
	


	//컷씬 끝나면 서버로 보내기
	if (m_pInsideCamera->getcanchange() == false || m_pCamera->getcanchange() == false) {
		//cout << "보내기";
		CS_CUTSCENE_END_PACKET my_packet;
		my_packet.size = sizeof(CS_CUTSCENE_END_PACKET);
		my_packet.type = CS_CUTSCENE_END;
		send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);

		if (curMissionType == MissionType::CS_ENDING) {
			b_Inside = true;
			_state = SCENE_LOBBY;

			m_pInsideCamera = m_pInsidePlayer[g_myid]->ChangeCamera(THIRD_PERSON_CAMERA, m_GameTimer.GetTimeElapsed());
			m_pCamera = m_pPlayer[0]->ChangeCamera(DRIVE_CAMERA, m_GameTimer.GetTimeElapsed());
			m_pCamera->fAnglenu = 0;
			m_pCamera->canchange = true;
			m_pInsideCamera->canchange = true;
			m_pCamera->canTurn = false;
			m_pCamera->canDolly = false;
			iscut = false;


			//m_pInsideCamera = m_pInsidePlayer[g_myid]->ChangeCamera(THIRD_PERSON_CAMERA, m_GameTimer.GetTimeElapsed());
		}
	}

	//미션 넘어가면 카메라 변경해주기
	if (curMissionType == MissionType::TU_SIT  && m_pInsidePlayer[g_myid]->GetCamera()->GetMode() == CUT_SCENE_CAMERA)
	{
		cout<<"change sit\n";
		m_pInsideCamera = m_pInsidePlayer[g_myid]->ChangeCamera(THIRD_PERSON_CAMERA, m_GameTimer.GetTimeElapsed());
		m_pCamera->fAnglenu = 0;
		m_pInsideCamera->canchange = true;
		m_pCamera->canchange = true;
		m_pCamera->canTurn = false;
		m_pCamera->canDolly = false;
		iscut = false;

	}

	if ((curMissionType == MissionType::DEFEAT_BOSS || curMissionType == MissionType::DEFEAT_BOSS2 || curMissionType == MissionType::GO_PLANET
		|| curMissionType == MissionType::GO_CENTER || curMissionType == MissionType::ESCAPE_BLACK_HOLE || curMissionType == MissionType::KILL_GOD || curMissionType == MissionType::KILL_GOD2)
		&& (m_pInsidePlayer[g_myid]->GetCamera()->GetMode() == CUT_SCENE_CAMERA || m_pPlayer[0]->GetCamera()->GetMode() == CUT_SCENE_CAMERA))
	{
		if (b_BeforeCheckInside) {
			b_Inside = true;
			m_pInsideCamera = m_pInsidePlayer[g_myid]->ChangeCamera(m_pBeforeCamera, m_GameTimer.GetTimeElapsed());
			m_pCamera->fAnglenu = 0;
			m_pInsideCamera->canchange = true;
			m_pCamera->canchange = true;
			m_pCamera->canTurn = false;
			m_pCamera->canDolly = false;

			b_BeforeCheckInside = false;
			iscut = false;

		}
		else {
			m_pCamera = m_pPlayer[0]->ChangeCamera(m_pBeforeCamera, m_GameTimer.GetTimeElapsed());	
			m_pCamera->fAnglenu = 0;
			m_pCamera->canchange = true;
			m_pInsideCamera->canchange = true;
			m_pCamera->canTurn = false;
			m_pCamera->canDolly = false;
			iscut = false;

		}
		m_effectSound[static_cast<int>(Sounds::DARK)]->stop();

	}

	if (curMissionType != MissionType::CS_BAD_ENDING && pastMissionType== MissionType::CS_BAD_ENDING)
	{
		pastMissionType = curMissionType;
		isending = true;
		if (b_BeforeCheckInside) {
			b_Inside = true;
			m_pInsideCamera = m_pInsidePlayer[g_myid]->ChangeCamera(m_pBeforeCamera, m_GameTimer.GetTimeElapsed());
			//m_pInsideCamera->canTurn = true;
			//m_pCamera->canTurn = true;
			m_pCamera->fAnglenu = 0;
			m_pInsideCamera->canchange = true;
			m_pCamera->canchange = true;
			m_pCamera->canTurn = false;
			m_pCamera->canDolly = false;
			b_BeforeCheckInside = false;
			iscut = false;

		}
		else {
			m_pCamera = m_pPlayer[0]->ChangeCamera(m_pBeforeCamera, m_GameTimer.GetTimeElapsed());
			//m_pCamera->canTurn = true;
			//m_pInsideCamera->canTurn = true;		
			m_pCamera->fAnglenu = 0;
			m_pCamera->canchange = true;
			m_pInsideCamera->canchange = true;
			m_pCamera->canTurn = false;
			m_pCamera->canDolly = false;
			iscut = false;

		}
	}


}

void error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L"에러" << lpMsgBuf << std::endl;

//	MessageBox(hWnd, lpMsgBuf, L"ERROR", 0);
	LocalFree(lpMsgBuf);
	// while (true);
}

void CGameFramework::OnDestroy()
{
    ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();
	if (m_pd3dGodRayShadowMap) m_pd3dGodRayShadowMap->Release();

	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();

	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
    if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();

	CSound::Release();
#if defined(_DEBUG)
	IDXGIDebug1	*pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void **)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

#define _WITH_TERRAIN_PLAYER

void CGameFramework::BuildObjects()
{
	if (!m_pUILayer)
	{
		m_pUILayer = new UILayer(m_nSwapChainBuffers, m_pd3dDevice, m_pd3dCommandQueue);
	}
	m_pUILayer->Resize(m_ppd3dSwapChainBackBuffers, m_nWndClientWidth, m_nWndClientHeight);

	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	m_pScene = new CScene();
	if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList);
	m_pInsideScene = new CScene();
	if (m_pInsideScene) m_pInsideScene->BuildInsideObjects(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetDescriptor());


#ifdef _WITH_TERRAIN_PLAYER
	CTerrainPlayer* pPlayer[3];
	for (int i = 0; i < 3; ++i) {
		pPlayer[i] = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pScene->m_pTerrain, i);
		pPlayer[i]->SetPosition(XMFLOAT3(425.0f + 10.0f * i, 250.0f, 640.0f));
		pPlayer[i]->SetScale(XMFLOAT3(15.0f, 15.0f, 15.0f));
	}

	CAirplanePlayer* pAirPlayer[1];
	pAirPlayer[0] = new CAirplanePlayer(m_pd3dDevice, m_pd3dCommandList, m_pInsideScene->GetGraphicsRootSignature(), m_pInsideScene->m_pTerrain);
	pAirPlayer[0]->SetPosition(XMFLOAT3(425.0f, 250.0f, 640.0f));
	pAirPlayer[0]->SetScale(XMFLOAT3(15.0f, 15.0f, 15.0f));


#else
	CAirplanePlayer* pPlayer = new CAirplanePlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), NULL);
	pPlayer->SetPosition(XMFLOAT3(425.0f, 240.0f, 640.0f));
#endif

	for (int i = 0; i < m_pScene->m_nScenePlayer; ++i) {
		m_pScene->m_pPlayer[i] = m_pPlayer[i] = pAirPlayer[i];
	}
	m_pCamera = m_pPlayer[0]->GetCamera();
	m_pCamera = m_pPlayer[0]->ChangeCamera((DWORD)(4), m_GameTimer.GetTimeElapsed());
	for (int i = 0; i < m_pInsideScene->m_nScenePlayer; ++i) {
		m_pInsideScene->m_pPlayer[i] = m_pInsidePlayer[i] = pPlayer[i];
	}

	m_pInsideCamera = m_pInsidePlayer[g_myid]->GetCamera();


	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();
	if (m_pScene) m_pScene->ReleaseUploadBuffers();

	if (m_pPlayer)
	{
		m_pScene->m_pPlayer[0]->ReleaseUploadBuffers();
		cout << "m_pPlayer" << endl;

	}
	if (m_pInsidePlayer)
		for (int i = 0; i < 3; ++i) {
			m_pInsideScene->m_pPlayer[i]->ReleaseUploadBuffers();
			cout << "i : " << i << endl;
		}



//	if (m_pInsideScene) m_pInsideScene->ReleaseUploadBuffers();
	/*if (m_pPlayer) for(int i=0; i< 1; ++i)if(m_pScene->m_pPlayer[i])m_pScene->m_pPlayer[i]->ReleaseUploadBuffers();
	if (m_pInsidePlayer)for (int i = 0; i < m_pInsideScene->m_nScenePlayer; ++i) if(m_pInsideScene->m_pPlayer[i])m_pInsideScene->m_pPlayer[i]->ReleaseUploadBuffers();*/

	m_GameTimer.Reset();
}

void CGameFramework::BuildSounds()
{
	m_bgm[static_cast<int>(BGMSounds::MAIN)] = new CSound("Sound/background.wav", true, 0.3f);
	m_bgm[static_cast<int>(BGMSounds::BOSS)] = new CSound("Sound/boss_stage1.mp3", true, 0.3f);
	m_bgm[static_cast<int>(BGMSounds::GOD)] = new CSound("Sound/boss_stage2.mp3", true, 0.3f);

	m_lobbybgm = new CSound("Sound/lobby.mp3", true, 0.3f);

	m_effectSound[static_cast<int>(Sounds::GUN)] = new CSound("Sound/gun.mp3", false, 1.0f);
	m_effectSound[static_cast<int>(Sounds::EXP)] = new CSound("Sound/explosion.mp3", false, 0.5f);
	m_effectSound[static_cast<int>(Sounds::WALK)] = new CSound("Sound/walk.wav", false, 1.0f);
	m_effectSound[static_cast<int>(Sounds::BAD)] = new CSound("Sound/bad.mp3", false, 1.0f);
	m_effectSound[static_cast<int>(Sounds::CLEAR)] = new CSound("Sound/clear.mp3", false, 0.2f);
	m_effectSound[static_cast<int>(Sounds::CLICK)] = new CSound("Sound/click.mp3", false, 0.2f);
	m_effectSound[static_cast<int>(Sounds::DARK)] = new CSound("Sound/dark.mp3", false, 0.2f);
	m_effectSound[static_cast<int>(Sounds::ROAR)] = new CSound("Sound/roaring.mp3", false, 1.0f);
	m_effectSound[static_cast<int>(Sounds::FIRE)] = new CSound("Sound/roaring.mp3", false, 1.0f);
	m_effectSound[static_cast<int>(Sounds::GROWL)] = new CSound("Sound/roaring.mp3", false, 1.0f);
	m_effectSound[static_cast<int>(Sounds::CLAW)] = new CSound("Sound/claw.mp3", false, 1.0f);
	m_effectSound[static_cast<int>(Sounds::BASIC_ATTACT)] = new CSound("Sound/claw.mp3", false, 1.0f);
	m_effectSound[static_cast<int>(Sounds::GOD1)] = new CSound("Sound/god1.mp3", false, 1.0f);
	m_effectSound[static_cast<int>(Sounds::GOD2)] = new CSound("Sound/god2.mp3", false, 1.0f);

}

void CGameFramework::UpdateSounds()
{
	if (_state == SCENE_LOBBY) {
		m_lobbybgm->Update(); 
		m_bgm[2]->stop(); 
	}
	else {
		m_lobbybgm->stop();

		if (curMissionType < MissionType::CS_BOSS_SCREAM) {
			m_bgm[0]->Update();
		}
		else if (curMissionType >= MissionType::CS_BOSS_SCREAM) {
			m_bgm[1]->Update();
		}
		else if (curMissionType >= MissionType::CS_SHOW_GOD) {
			m_bgm[2]->Update();
		}
	}



	if (m_pScene->m_ppBoss->soundon != -1)
	{
		if (m_pScene->m_ppBoss->soundo0nPAST != m_pScene->m_ppBoss->soundon)
			m_effectSound[m_pScene->m_ppBoss->soundon]->play();
		m_effectSound[m_pScene->m_ppBoss->soundon]->Update();
		m_pScene->m_ppBoss->soundo0nPAST = m_pScene->m_ppBoss->soundon;
	}

	m_pScene->m_ppBoss->soundo0nPAST = m_pScene->m_ppBoss->soundon;

	if (m_pScene->m_ppGod->soundon != -1)
	{
		if (m_pScene->m_ppGod->soundo0nPAST != m_pScene->m_ppGod->soundon)
			m_effectSound[m_pScene->m_ppGod->soundon]->play();
		m_effectSound[m_pScene->m_ppGod->soundon]->Update();
		m_pScene->m_ppGod->soundo0nPAST = m_pScene->m_ppGod->soundon;
	}

	m_pScene->m_ppGod->soundo0nPAST = m_pScene->m_ppGod->soundon;


	for (int i = 0; i < static_cast<int>(Sounds::COUNT); ++i)
		if (m_effectSound[i] || i != static_cast<int>(Sounds::WALK) || (i >= static_cast<int>(Sounds::ROAR) && i <= static_cast<int>(Sounds::GOD2)))
			m_effectSound[i]->Update();


}

void CGameFramework::ReleaseObjects()
{
	if (m_pUILayer) m_pUILayer->ReleaseResources();
	if (m_pUILayer) delete m_pUILayer;

	if (m_pPlayer) m_pPlayer[0]->Release();
	if (m_pInsidePlayer) for (int i = 0; i < 3; ++i)m_pInsidePlayer[i]->Release();

	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
	if (m_pInsideScene) m_pInsideScene->ReleaseObjects();
	if (m_pInsideScene) delete m_pInsideScene;
	if (m_bgm)for (int i = 0; i < static_cast<int>(BGMSounds::COUNT); ++i) delete m_bgm;
	if (m_bgm)for (int i = 0; i < static_cast<int>(BGMSounds::COUNT); ++i) m_bgm[i]->Release();

	if (m_lobbybgm) delete m_lobbybgm;
	if (m_lobbybgm) m_lobbybgm->Release();


	if (m_effectSound)for (int i = 0; i < static_cast<int>(Sounds::COUNT); ++i) delete m_effectSound;
	if (m_effectSound)for (int i = 0; i < static_cast<int>(Sounds::COUNT); ++i) m_effectSound[i]->Release();

}

void CGameFramework::ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = false;
	if (GetKeyboardState(pKeysBuffer) && m_pScene) bProcessedByScene = m_pScene->ProcessInput(pKeysBuffer);
	if (!bProcessedByScene)
	{
		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;
		GetCursorPos(&ptCursorPos);
		ScreenToClient(m_hWnd, &ptCursorPos);
		RECT rect;
		GetClientRect(m_hWnd, &rect);


		XMFLOAT3 a = m_pInsidePlayer[0]->GetPosition();
		a.y = 10.f;
		m_pInsidePlayer[0]->SetPosition( a);
		// if문 > 마우스 커서가 게임 화면 안에 있을 때
		//if (ptCursorPos.x >= 0 && ptCursorPos.x < rect.right 
		//	&& ptCursorPos.y >= 0 && ptCursorPos.y < rect.bottom)
		{
			if (!b_Inside&&!isConnect || player_type != PlayerType::INSIDE && isConnect) { // 외부 우주선. 클라 화면 중앙과 커서의 위치 차이에 따라 회전
				//ShowCursor(true); //커서 보이게 > 조종법을 이 방식으로 한다면 화면 전환시마다 이걸해줘야 함
				float x = (float)(rect.right - rect.left) / 2;
				float y = (float)(rect.bottom - rect.top) / 2;
				cxDelta = (float)(ptCursorPos.x - x) / 200.0f;
				cyDelta = (float)(ptCursorPos.y - y) / 200.0f;
			}
			else if(b_Inside && !isConnect || player_type == PlayerType::INSIDE && isConnect) { // 내부 플레이어. 이전 마우스 커서와 현재 마우스 커서 위치 차이에 따라 회전.
				//ShowCursor(false); // 커서 안보이게 > 조종법을 이 방식으로 한다면 화면 전환시마다 이걸해줘야 함
				ScreenToClient(m_hWnd, &m_ptOldCursorPos);
				cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 2.0f; // 감도 조절 필요
				ptCursorPos.x = rect.right / 2; ptCursorPos.y = rect.bottom / 2;
				ClientToScreen(m_hWnd, &ptCursorPos);
				//SetCursorPos(ptCursorPos.x, ptCursorPos.y);	// 화면 중앙에 커서 고정
			}
		}
		GetCursorPos(&m_ptOldCursorPos);
		//SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		//SetCursor(NULL);

		DWORD dwDirection = 0;
		if (pKeysBuffer['W'] & 0xF0) dwDirection |= DIR_FORWARD;
		if (pKeysBuffer['S'] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (pKeysBuffer['A'] & 0xF0) dwDirection |= DIR_LEFT;
		if (pKeysBuffer['D'] & 0xF0) dwDirection |= DIR_RIGHT;
		if (pKeysBuffer[VK_PRIOR] & 0xF0) dwDirection |= DIR_UP;
		if (pKeysBuffer[VK_NEXT] & 0xF0) dwDirection |= DIR_DOWN;

		if (isConnect) {
			if (player_type == PlayerType::INSIDE) {
				if (m_pInsidePlayer[g_myid]->motion == AnimationState::IDLE) {
					if (dwDirection != 0) {
						CS_ANIMATION_CHANGE_PACKET my_packet;
						my_packet.size = sizeof(CS_ANIMATION_CHANGE_PACKET);
						my_packet.type = CS_ANIMATION_CHANGE;
						my_packet.state = static_cast<char>(AnimationState::WALK);
						send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
						m_pInsidePlayer[g_myid]->motion = AnimationState::WALK;
					}
				}
				else if (m_pInsidePlayer[g_myid]->motion == AnimationState::WALK) {
					if (dwDirection == 0) {
						CS_ANIMATION_CHANGE_PACKET my_packet;
						my_packet.size = sizeof(CS_ANIMATION_CHANGE_PACKET);
						my_packet.type = CS_ANIMATION_CHANGE;
						my_packet.state = static_cast<char>(AnimationState::IDLE);
						send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
						m_pInsidePlayer[g_myid]->motion = AnimationState::IDLE;
					}
				}
			}
		}

		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				if (b_Inside && !isConnect || player_type == PlayerType::INSIDE && isConnect) {
					m_pInsidePlayer[g_myid]->Rotate(0.0f, cxDelta, 0.0f);
				}
				else {
					if (m_pPlayer[0]->GetCamera()->GetMode() == ATTACT_CAMERA_C || m_pPlayer[0]->GetCamera()->GetMode() == ATTACT_CAMERA_L || m_pPlayer[0]->GetCamera()->GetMode() == ATTACT_CAMERA_R) {
						m_pPlayer[0]->Rotate2(cxDelta, cyDelta, 0.0f);
						/*cout << "cyDelta - " << cyDelta << endl;
						cout << "cxDelta - " << cxDelta << endl;*/

					}
					else
						m_pPlayer[0]->Rotate(cyDelta, cxDelta, 0.0f);
				}
			}

			if (isConnect) {
				if(player_type == PlayerType::INSIDE){
					CS_INSIDE_PACKET my_packet;
					my_packet.size = sizeof(CS_INSIDE_PACKET);
					my_packet.type = CS_INSIDE_MOVE;
					my_packet.data.dwDirection = dwDirection;
					my_packet.data.yaw = m_pInsidePlayer[g_myid]->GetYaw();
					if (_state == SCENE_INGAME && (dwDirection != 0 || cxDelta != 0.0f)) {
						send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
					}

				}
				else if(player_type == PlayerType::MOVE) {
					char cDrt = 0;
					if (pKeysBuffer['W'] & 0xF0) cDrt |= option0;
					if (pKeysBuffer['S'] & 0xF0) cDrt |= option1;
					if (pKeysBuffer['A'] & 0xF0) cDrt |= option2;
					if (pKeysBuffer['D'] & 0xF0) cDrt |= option3;

					if (cDrt != cDirection) {
						cDirection = cDrt;
						CS_KEY_INPUT_PACKET my_packet;
						my_packet.size = sizeof(CS_KEY_INPUT_PACKET);
						my_packet.type = CS_KEY_INPUT;
						my_packet.key = cDrt;
						send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
					}

					CS_SPACESHIP_QUATERNION_PACKET my_packet;
					my_packet.size = sizeof(CS_SPACESHIP_QUATERNION_PACKET);
					my_packet.type = CS_SPACESHIP_QUATERNION;
					XMFLOAT4 a;
					XMStoreFloat4(&a, m_pPlayer[0]->GetQuaternion());
					my_packet.Quaternion = a;

					if (_state == SCENE_INGAME) {
						send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
					}
				}
			}
			else {
				//이동 부분 분할할지 모르겠어서 우선 여따 같이넣어둠 
				if (dwDirection&&!b_Inside) m_pPlayer[0]->Move(dwDirection, 20.25f, true);
				if (dwDirection && b_Inside) {
					m_pInsidePlayer[g_myid]->Move(dwDirection, 10.25f, true);
				}
			}

			//if (m_pInsidePlayer[g_myid]->motion == AnimationState::WALK) {
			//	cout << "walk";
			//	m_effectSound[static_cast<int>(Sounds::WALK)]->play();
			//}
			//if (m_pInsidePlayer[g_myid]->motion != AnimationState::WALK) {
			//	m_effectSound[static_cast<int>(Sounds::WALK)]->stop();
			//}
		}

	}

	if (isConnect) {
		m_pPlayer[0]->UpdateOnServer(player_type == PlayerType::INSIDE);
		for (int i = 0; i < 3; ++i)m_pInsidePlayer[i]->UpdateOnServer(i != g_myid);
	}
	if(!b_Inside) for (int i = 0; i < 1; ++i)m_pPlayer[i]->Update(m_GameTimer.GetTimeElapsed());
	else for (int i = 0; i < 3; ++i)m_pInsidePlayer[i]->Update(m_GameTimer.GetTimeElapsed());

	if (player_type == PlayerType::INSIDE && isHealing && !AroundSculpture()) {
		CS_HEAL_PACKET my_packet;
		my_packet.size = sizeof(CS_HEAL_PACKET);
		my_packet.type = CS_HEAL;
		my_packet.start = false;
		if (_state == SCENE_INGAME) {
			send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
		}
		isHealing = false;
	}
}

void CGameFramework::AnimateObjects()
{
	RecvServer();

	float fTimeElapsed = m_GameTimer.GetTimeElapsed();

	for (int i = 0; i < ENEMIES; i++)
	{
		if (m_pScene->m_ppEnemies[i] && m_pScene->m_ppEnemies[i]->isAlive)
		{
			m_pScene->m_ppEnemies[i]->AI(fTimeElapsed, m_pPlayer[0]->GetPosition());
		}
	}

	if (m_pScene) m_pScene->AnimateObjects(fTimeElapsed);
	if (m_pInsideScene) m_pInsideScene->AnimateObjects(fTimeElapsed);

	for (int i = 0; i < 1; ++i)m_pPlayer[i]->Animate(fTimeElapsed);
	for (int i = 0; i < 3; ++i)m_pInsidePlayer[i]->Animate(fTimeElapsed);


}

void CGameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

//#define _WITH_PLAYER_TOP

void CGameFramework::FrameAdvance()
{
	m_GameTimer.Tick(30.0f);
	ProcessInput();
	CameraUpdateChange();

	


	AnimateObjects();

	UpdateUI();

	UpdateSounds();


	//if (std::isnan(m_pCamera->GetPosition().x))cout << "x nan!!\n";
	//if (std::isnan(m_pCamera->GetPosition().y))cout << "y nan!!\n";
	//if (std::isnan(m_pCamera->GetPosition().z))cout << "z nan!!\n";

	//if (std::isnan(m_pInsideCamera->GetPosition().x))cout << "m_pInsideCamera x nan!!\n";
	//if (std::isnan(m_pInsideCamera->GetPosition().y))cout << "m_pInsideCamera y nan!!\n";
	//if (std::isnan(m_pInsideCamera->GetPosition().z))cout << "m_pInsideCamera z nan!!\n";
	//if (std::isnan(m_pCamera->GetLookVector().x))cout << "GetLookVector x nan!!\n";
	//if (std::isnan(m_pCamera->GetLookVector().y))cout << "GetLookVector y nan!!\n";
	//if (std::isnan(m_pCamera->GetLookVector().z))cout << "GetLookVector z nan!!\n";
	//if (std::isnan(m_pCamera->GetUpVector().x))cout << "GetUpVector x nan!!\n";
	//if (std::isnan(m_pCamera->GetUpVector().y))cout << "GetUpVector y nan!!\n";
	//if (std::isnan(m_pCamera->GetUpVector().z))cout << "GetUpVector z nan!!\n";
	//if (std::isnan(m_pCamera->GetRightVector().z))cout << "GetRightVector x nan!!\n";
	//if (std::isnan(m_pCamera->GetRightVector().z))cout << "GetRightVector y nan!!\n";
	//if (std::isnan(m_pCamera->GetRightVector().z))cout << "GetRightVector z nan!!\n";



	//static int b = 0;


	//XMFLOAT3 a;

	//
	//	a = m_pCamera->GetPosition();
	//	a.x = a.x / 0;

	//	if (std::isnan(a.x)) {
	//		cout << "a가 난이 됐음s\n";
	//	}

	//	m_pCamera->SetPosition(a);
	//	b = 1;
	//	cout << "s난으로 한번감\n";


	if (std::isnan(m_pCamera->GetPosition().x)) {
		m_pCamera->SetPosition(m_pPlayer[0]->GetPosition());
		cout << "m_pCamera GetPosition 돌아와\n";
	}
	if (std::isnan(m_pCamera->GetUpVector().x)) {
		m_pCamera->SetUpVector(m_pPlayer[0]->GetUpVector());
		cout << "m_pCamera GetUpVector 돌아와\n";
	}

	if (std::isnan(m_pCamera->GetRightVector().x)) {
		m_pCamera->SetRightVector(m_pPlayer[0]->GetRightVector());
		cout << "m_pCamera GetRightVector 돌아와\n";
	}
	if (std::isnan(m_pCamera->GetLookVector().x)) {
		m_pCamera->SetLookVector(m_pPlayer[0]->GetLookVector());
		cout << "m_pCamera SetLookVector 돌아와\n";
	}

	if (std::isnan(m_pInsideCamera->GetPosition().x)) {
		m_pInsideCamera->SetPosition(m_pPlayer[0]->GetPosition());
		cout << "m_pInsideCamera 돌아와\n";
	}




	m_pPlayer[0]->curMissionType = curMissionType;




	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	if (b_Inside) {
		m_pInsideScene->OnPrepareRender(m_pd3dCommandList);
		m_pInsideScene->OnPreRender(m_pd3dCommandList);
	}

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor/*Colors::Azure*/, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	/*float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], pfClearColor/*Colors::Azure, 0, NULL);

	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dCommandList->OMSetRenderTargets(1, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], TRUE, &d3dDsvCPUDescriptorHandle);

	m_pScene->Render(m_pd3dCommandList, m_pCamera);
	*/

	if (b_CameraScene)b_Inside = b_CameraScene = m_pInsideCamera->CameraSence1(b_CameraScene);
	if (m_pScene && !b_Inside) m_pScene->Render(m_pd3dCommandList, m_pCamera);
	if (m_pInsideScene && b_Inside) m_pInsideScene->Render(m_pd3dCommandList, m_pInsideCamera);

	//for (int i = 0; i < static_cast<int>(DragonSounds::COUNT); ++i)if (m_pScene->m_ppBoss->b_effectSound[i]) {
	//	m_pScene->m_ppBoss->b_effectSound[i]->Update();
	//}


#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif


	if ((m_pPlayer && !b_Inside)&& player_type ==PlayerType::MOVE) if(m_pPlayer[0])m_pPlayer[0]->Render(m_pd3dCommandList, m_pCamera);
	if (m_pInsidePlayer && b_Inside)for (int i = 0; i < 3; ++i) {
		if (m_pInsidePlayer[i]->isAlive) {
			m_pInsidePlayer[i]->Render(m_pd3dCommandList, m_pInsideCamera);
		}
	}
	if (m_pScene && !b_Inside) m_pScene->RenderUI(m_pd3dCommandList, m_pCamera);
	if (m_pInsideScene && b_Inside)m_pInsideScene->RenderUIInside(m_pd3dCommandList, m_pInsideCamera);


	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	hResult = m_pd3dCommandList->Close();
	
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();
	m_pUILayer->Render(m_nSwapChainBufferIndex, curMissionType, m_pScene->m_ppBoss->CurState, _state, m_GameTimer.GetTimeElapsed());

#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	m_pdxgiSwapChain->Present(0, 0);
#endif
#endif

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	size_t nLength = _tcslen(m_pszFrameRate);
	XMFLOAT3 xmf3Position;
	if(b_Inside) xmf3Position = m_pInsidePlayer[g_myid]->GetLookVector();
	else xmf3Position = m_pPlayer[0]->GetPosition();
	_stprintf_s(m_pszFrameRate + nLength, 70 - nLength, _T("(%.2f, %.2f, %.2f) HP : %d"), xmf3Position.x, xmf3Position.y, xmf3Position.z, m_pPlayer[0]->hp);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}

void CGameFramework::UpdateUI()
{

	vector<wstring> labels;
	wchar_t position_ui[50];
	_stprintf_s(position_ui, _countof(position_ui), _T("UI 테스트\n"));

	wstring uiText = L"UI 테스트";
	wstring uiScripts = L"UI 테스트";
	wstring uiBossScripts = L"boss 대사";

	wstring uiJew = L"";


	/*for (auto s : labels)
	{
		uiText += s;
	}*/

	uiText = ChangeMission(curMissionType);


	if (scriptsOn) {
		scriptsStartTime = steady_clock::now();
		cout << "대사 시작\n";
		scriptsOn = false;
	}




	uiScripts = ChangeScripts(curMissionType);
	if(curMissionType<MissionType::CS_SHOW_GOD)
	uiBossScripts = ChangeBossScripts(curMissionType, m_pScene->m_ppBoss->GetcurHp());
	else{ uiBossScripts = ChangeBossScripts(curMissionType, m_pScene->m_ppGod->GetcurHp()); }


	if (duration_cast<seconds>(steady_clock::now() - scriptsStartTime).count() >= 5)
	{
		uiScripts = L" ";
	}

	if (bossScriptsOn) {
		bossScriptsTime += m_GameTimer.GetTimeElapsed();

	}
	if (bossScriptsTime>=3) {
		uiBossScripts = L" ";
		bossScriptsOn = false;
	}

	wstring JEWEL_ATT;
	JEWEL_ATT = to_wstring(items[ItemType::JEWEL_ATT]);
	wstring JEWEL_DEF;
	JEWEL_DEF = to_wstring(items[ItemType::JEWEL_DEF]);
	wstring JEWEL_HEAL;
	JEWEL_HEAL = to_wstring(items[ItemType::JEWEL_HEAL]);
	wstring JEWEL_HP;
	JEWEL_HP = to_wstring(items[ItemType::JEWEL_HP]);

	uiJew += JEWEL_ATT;
	uiJew += L"\n\n";
	uiJew += JEWEL_DEF;
	uiJew += L"\n\n";
	uiJew += JEWEL_HEAL;
	uiJew += L"\n\n";
	uiJew += JEWEL_HP;

	std::wstring matwst = std::to_wstring(matcnt);

	std::wstring skipwst = std::to_wstring(skipnum);



	m_pUILayer->UpdateLabels(uiText);
	m_pUILayer->UpdateLabels_Scripts(uiScripts);
	m_pUILayer->UpdateLabels_BossScripts(uiBossScripts);

	

	m_pUILayer->UpdateLabels_Jew(uiJew);

	if (matcnt <= 0) {
		m_pUILayer->UpdateLabels_Lobby(L"room number");
		m_pUILayer->UpdateLabels_LobbyMatching(L"룸 넘버를 입력하세요");
		m_pUILayer->UpdateLabels_Lobby(roomNum);

	}
	else {
		m_pUILayer->UpdateLabels_LobbyMatching(matwst + L" / 3");
		m_pUILayer->UpdateLabels_Lobby(roomNum);
		m_pUILayer->noData = false;
	}
	m_pUILayer->UpdateLabels_Skip(L"스킵은 spacebar - " + skipwst + L" / 3");
	

	m_pUILayer->UpdateHp(m_pPlayer[0]->hp, m_pPlayer[0]->max_hp);
	/*if(m_pScene->m_ppBoss->GetState()!= BossState::SLEEP)*/
	m_pUILayer->UpdateBossHp(m_pScene->m_ppBoss->BossHP, m_pScene->m_ppBoss->MAXBossHP);
	m_pUILayer->UpdateGodBossHp(m_pScene->m_ppGod->GodHP, m_pScene->m_ppGod->MAXGodHP);

	//for (int i = 0; i < ENEMIES; ++i)
	//{
	//	m_pUILayer->UpdateDots(i, { 0,0 });
	//}


	for (int i = 0; i < ENEMIES; ++i) {
		m_pUILayer->UpdateDots(i, m_pPlayer[0], m_pScene->m_ppEnemies[i]->GetPosition(), m_pScene->m_ppEnemies[i]->isAlive);
	}
	
	if(curMissionType ==MissionType::FIND_BOSS)
		m_pUILayer->UpdateBossNevi(BOSS_ID, m_pPlayer[0], m_pScene->m_ppBoss->GetPosition());

	if (curMissionType == MissionType::GO_PLANET) {
		m_pUILayer->UpdatePlanetNevi(m_pPlayer[0], planetPos);
		pDist = m_pUILayer->UpdatePlanetDist(m_pPlayer[0], planetPos);
	}

	if (m_pScene->m_ppBoss->BossHP < 0)
		bossdie = 1;

}

wstring CGameFramework::ChangeMission(MissionType mType)
{
	wstring uiText = L"미션 - ";
	wstring enemyCountStr;
	wstring jewelCntStr;
	wstring planetDist;
	wstring bossDist;
	wstring bossDie;
	wstring centerDist;
	wstring blackholetimewst;





	short jewelCnt = 0;

	jewelCnt += items[ItemType::JEWEL_ATT];
	jewelCnt += items[ItemType::JEWEL_DEF];
	jewelCnt += items[ItemType::JEWEL_HEAL];
	jewelCnt += items[ItemType::JEWEL_HP];



	XMVECTOR v1 = XMLoadFloat3(&m_pPlayer[0]->GetPosition());
	XMVECTOR v2 = XMLoadFloat3(&m_pScene->m_ppBoss->GetPosition());
	XMVECTOR dist = XMVector3Length(XMVectorSubtract(v2, v1));
	XMVECTOR center = { 0,0,0 };
	XMVECTOR distcenter = XMVector3Length(XMVectorSubtract(center, v1));

	float distance;
	XMStoreFloat(&distance, dist);
	float distanceCenter;
	XMStoreFloat(&distanceCenter, distcenter);

	distance = distance - 1500.0f;

	distanceCenter = distanceCenter;

	enemyCountStr = to_wstring(killCnt);
	blackholetimewst = to_wstring(blackholetime);

	jewelCntStr = to_wstring(jewelCnt);
	planetDist = to_wstring(pDist);
	bossDist = to_wstring(distance);

	bossDie = to_wstring(bossdie);

	centerDist = to_wstring(distanceCenter);

	wstring uiTextSpace = L" ";
	wstring uiTextCnt = L" / 20";

	switch (mType) {
	case MissionType::TU_SIT:
	{
		uiText = L"튜토리얼 - 조종석에서 상호작용으로 우주선 조종하기";
		break;
	}
	case MissionType::TU_KILL:
	{
		uiText = L"튜토리얼 - 공격석에서 몬스터 처치하기";
		enemyCountStr = to_wstring(killCnt);
		break;
	}
	case MissionType::TU_HILL:
	{
		uiText = L"튜토리얼 - 조각상에서 좌클릭으로 체력 회복하기";
		break;
	}
	case MissionType::GET_JEWELS:
	{
		uiText = L"미션 - 보석을 1개 얻어라";
	//	uiText += jewelCntStr;
	//	uiText += L" / 4 )";

		break;
	}
	case MissionType::Kill_MONSTER:
	{
		uiText = L"미션 - 몬스터를 처치하라 (";
		uiText += enemyCountStr;
		uiText += L" / 15 )";
		break;
	}
	case MissionType::GO_PLANET:
	{
		uiText = L"미션 - 목표 지점으로 도달하라 ( ";
		uiText += planetDist;
		uiText += L"m 남음 )";

		break;
	}
	case MissionType::KILL_MONSTER_ONE_MORE_TIME:
	{
		uiText = L"미션 - 몬스터를 처치하라 (";
		uiText += enemyCountStr;
		uiText += L" / 20 )";
		break;
	}
	case MissionType::FIND_BOSS:
	{
		uiText = L"미션 - 보스를 추적하라 ( ";
		uiText += bossDist;
		uiText += L"m 남음 )";
		break;
	}
	case MissionType::DEFEAT_BOSS2:
	case MissionType::DEFEAT_BOSS:
	{
		uiText = L"미션 - 보스를 처치하라 ( ";
		uiText += bossDie;
		uiText += L" / 1 )";

		break;
	}
	//2R
	case MissionType::GO_CENTER:
	{
		uiText = L"미션 - 우주의 중심으로 가라 ( ";
		uiText += centerDist;
		uiText += L"m 남음 )";
		break;
	}

	case MissionType::KILL_MONSTER3:
	{
		uiText = L"미션 - 몬스터를 처치하라 ( ";
		uiText += enemyCountStr;
		uiText += L" / 20 )"; //
		break;
	}
	case MissionType::KILL_METEOR:
	{
		uiText = L"미션 - 운석을 처치하라 ( ";
		uiText += enemyCountStr;
		uiText += L" / 5 )"; //
		break;
	}
	case MissionType::ESCAPE_BLACK_HOLE:
	{
		uiText = L"미션 - 블랙홀에서 벗어나라! ( ";
		uiText += blackholetimewst;
		uiText += L"s )"; //
		break;
	}
	case MissionType::GO_CENTER_REAL:
	{
		uiText = L"미션 - 진짜 우주의 중심으로 가라! ( ";
		uiText += centerDist;
		uiText += L"m 남음 )";
		break;
	}
	case MissionType::KILL_GOD2:
	case MissionType::KILL_GOD:
	{
		uiText = L"미션 - 신을 처치하라 ( ";
		uiText += bossDie;
		uiText += L" / 1 )";
		break;
	}
	
	default:
		uiText = L" ";
		break;
	};

	return uiText;

}

wstring CGameFramework::ChangeScripts(MissionType mType)
{
	wstring uiScripts = L" ";

	/*if (pastMissionType == curMissionType)
	{
		uiScripts = L" ";
		return uiScripts;
	}*/

	switch (mType) {
	case MissionType::CS_TURN:
	{
		uiScripts = L"어서와!";
		if (firstSc == -2) {
			firstSc = -1;
			scriptsOn = true;
		}

		break;
	}
	case MissionType::TU_SIT:
	{
		uiScripts = L"우선 조종석에 앉아서 우주선을 조종해 봐!";
		if (firstSc == -1) {
			firstSc = 0;
			scriptsOn = true;
		}
	
		break;
	}
	case MissionType::TU_KILL:
	{
		uiScripts = L"잘했어! 다음은 공격석에 앉아서 적을 처치해 봐!";
		if (firstSc == 0) {
			firstSc = 1;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::TU_HILL:
	{
		uiScripts = L"이런! 우주선의 체력이 감소했어!\n조각상에서 좌클릭을 해서 체력을 회복시켜 봐!";
		if (firstSc == 1) {
			firstSc = 2;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::GET_JEWELS:
	{
		//\n드래곤을 물리치기 위해선우리 우주선을 강화해야 해
		uiScripts = L"기본 조작은 설명해 줬으니 이제 본격적으로 들어가야겠지?\n특별한 에너지를 가진 보석을 모아서 능력치를 올려보자!";
		if (firstSc == 2) {
			firstSc = 3;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::Kill_MONSTER:
	{
		uiScripts = L"이런... 우주선을 방해하는 놈들이 있어,\n저놈들을 해치우고 앞으로 나아가자";
		if (firstSc == 3) {
			firstSc = 4;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::CS_SHOW_PLANET:
	{
		uiScripts = L"좋았어! 00행성에 보스가 있다는 정보를 입수했어!";
		if (firstSc == 4) {
			firstSc = 5;
			scriptsOn = true;
		}

		break;
	}
	case MissionType::GO_PLANET:
	{
		uiScripts = L"00행성까지 가자!";
		if (firstSc == 5) {
			firstSc = 6;
			scriptsOn = true;
		}

		break;
	}
	case MissionType::KILL_MONSTER_ONE_MORE_TIME:
	{
		uiScripts = L"발 빠른 드래곤 같으니, 벌써 도망쳤나봐.\n새로운 위치 갱신을 위해 잠시만 기다려줄래?";
		if (firstSc == 6) {
			firstSc = 7;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::FIND_BOSS:
	{
		uiScripts = L"계산 완료! 이번엔 진짜야. 내비게이션 정보를 업데이트해줄게,\n우주의 평화를 위해서 부탁할게";
		if (firstSc == 7) {
			firstSc = 8;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::DEFEAT_BOSS:
	{
		uiScripts = L"우주의 평화를 위해서 드래곤을 처치해 줘!!!";
		if (firstSc == 8) {
			firstSc = 9;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::CS_SHOW_STARGIANT:
	{
		uiScripts = L"스타 자이언트야...!";
		if (firstSc == 9) {
			firstSc = 10;
			scriptsOn = true;
		}
		break;
	}

	case MissionType::GO_CENTER:
	{
		uiScripts = L"우주의 평화를 위해서 드래곤을 처치해 줘!!!";
		if (firstSc == 10) {
			firstSc = 11;
			scriptsOn = true;
		}
		break;
	}

	case MissionType::KILL_MONSTER3:
	{
		uiScripts = L"다시 한번 몬스터를 처치해줘";
		if (firstSc == 11) {
			firstSc = 12;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::KILL_METEOR:
	{
		uiScripts = L"운석을 맞춰서 파괴해줘!";
		if (firstSc == 12) {
			firstSc = 13;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::CS_SHOW_BLACK_HOLE:
	{
		uiScripts = L"이런... 블랙홀이잖아?";
		if (firstSc == 13) {
			firstSc = 14;
			scriptsOn = true;
		}
		break;
	}	
	case MissionType::ESCAPE_BLACK_HOLE:
	{
		uiScripts = L"빨리 블랙홀에서 벗어나야 돼!";
		if (firstSc == 14) {
			firstSc = 15;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::GO_CENTER_REAL:
	{
		uiScripts = L"휴... 이제 진짜 우주의 중심으로 가야 돼";
		if (firstSc == 15) {
			firstSc = 16;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::CS_SHOW_GOD:
	{
		uiScripts = L"이런...";
		if (firstSc == 16) {
			firstSc = 17;
			scriptsOn = true;
		}
		break;
	}	
	case MissionType::KILL_GOD:
	{
		uiScripts = L"신을 처치해야 돼";
		if (firstSc == 17) {
			firstSc = 18;
			scriptsOn = true;
		}
		break;
	}

	case MissionType::CS_ENDING:
	{
		uiScripts = L"지금까지 정말 고생많았어!";
		if (firstSc == 18) {
			firstSc = 19;
			scriptsOn = true;
		}
		break;
	}

	default:
		uiScripts = L" ";
		break;
	};


	return uiScripts;
}

wstring CGameFramework::ChangeBossScripts(MissionType mType, short hp)
{
	wstring uiScripts = L" ";

	switch (mType) {
	case MissionType::CS_BOSS_SCREAM:
	{	
		uiScripts = L"스타 자이언트는 내 것이다...";
		if (firstbSc == 0) {
			firstbSc = 1;
			bossScriptsOn = true;
			bossScriptsTime = 0;

			m_bgm[1]->play();
			m_bgm[0]->stop();
		}
		break;
	}
	case MissionType::DEFEAT_BOSS:
	{
		uiScripts = L"너가 이길 수 있을까?";
		if (firstbSc == 1) {
			firstbSc = 2;
			bossScriptsOn = true;
			bossScriptsTime = 0;

		}
		if (hp < 50) {
			uiScripts = L"봐주지 않겠다...";
			if (firstbSc == 2) {
				firstbSc = 3;
				bossScriptsOn = true;
				bossScriptsTime = 0;

			}
		}

		if (hp < 20) {
			uiScripts = L"안 돼...";
			if (firstbSc == 3) {
				firstbSc = 4;
				bossScriptsOn = true;
				bossScriptsTime = 0;
			}
		}

		break;
	}
	case MissionType::CS_SHOW_STARGIANT:
	{
		uiScripts = L"내 보물......";
		if (firstbSc == 4 || firstbSc == 2 || firstbSc == 3) {
			firstbSc = 5;
			bossScriptsOn = true;
			bossScriptsTime = 0;
		}
		break;
	}
	case MissionType::CS_SHOW_GOD:
	{
		uiScripts = L"스타 자이언트를 내놓아라...";
		if (firstbSc == 5) {
			firstbSc = 6;
			bossScriptsOn = true;
			bossScriptsTime = 0;
			m_bgm[2]->play();
			m_bgm[1]->stop();

		}
		break;
	}
	case MissionType::KILL_GOD:
	{
		uiScripts = L"감히...";
		if (firstbSc ==6) {
			firstbSc = 7;
			bossScriptsOn = true;
			bossScriptsTime = 0;

		}

		if (hp < 50) {
			uiScripts = L"봐주지 않겠다...";
			if (firstbSc == 7) {
				firstbSc = 8;
				bossScriptsOn = true;
				bossScriptsTime = 0;

			}
		}

		if (hp < 20) {
			uiScripts = L"안 돼...";
			if (firstbSc == 8) {
				firstbSc = 9;
				bossScriptsOn = true;
				bossScriptsTime = 0;
			}
		}

		break;
	}
	case MissionType::CS_ENDING:
	{
		uiScripts = L"......";
		if (firstbSc == 9 || firstbSc == 8 || firstbSc == 7) {
			firstbSc = 0;
			bossScriptsOn = true;
			bossScriptsTime = 0;

			m_bgm[0]->play();
			m_bgm[1]->stop();
		}
		break;
	}
	default:
		uiScripts = L" ";
		break;
	};

	return uiScripts;
}


// 서버
bool CGameFramework::ConnectServer()
{
	if (!isConnect) {

		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { return false; }

		// socket 생성
		sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
		if (sock == INVALID_SOCKET) { return false; }

		// connect
		SOCKADDR_IN server_address{};
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(PORT_NUM);
		inet_pton(AF_INET, "127.0.0.1", &(server_address.sin_addr.s_addr));

		if(connect(sock, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address)) == SOCKET_ERROR)
		{
			return false;
		}

		// Non-blocking
		unsigned long noblock = 1;
		int nRet = ioctlsocket(sock, FIONBIO, &noblock);
		return true;
	}
	return true;
}

void CGameFramework::RecvServer()
{
	if (isConnect) {
		DWORD recv_byte = 0;
		char recv_buf[RECV_BUF_SIZE];
		WSABUF mybuf_r;
		mybuf_r.buf = recv_buf;//buf + buf_cur_size; 
		mybuf_r.len = RECV_BUF_SIZE;

		DWORD recv_flag = 0;
		int	RByte = WSARecv(sock, &mybuf_r, 1, &recv_byte, &recv_flag, 0, 0);
		if (RByte == SOCKET_ERROR || recv_byte == 0) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
			}
		}
		else {
			memcpy(buf + buf_cur_size, recv_buf, recv_byte);
			buf_cur_size += recv_byte;
			int packet_size;

			while (buf_cur_size > 0) {
				packet_size = (unsigned char)buf[0];
				if (buf_cur_size >= packet_size && packet_size != 0) {
					ProcessPacket(buf);
					buf_cur_size -= packet_size;
					memcpy(buf, &buf[packet_size], buf_cur_size);
				}
				else {
					break;
				}
			}
		}

	}
}

void CGameFramework::ProcessPacket(char* p)
{
	switch (p[1])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(p);
		g_myid = packet->data.id;

		m_pInsideCamera = m_pInsidePlayer[g_myid]->GetCamera();
		player_type = packet->data.player_type;

		if (player_type == PlayerType::INSIDE) {
			b_Inside = true;
		}
		else {
			b_Inside = false;
		}
		m_pInsidePlayer[g_myid]->type = player_type;
		m_pInsidePlayer[g_myid]->isAlive = true;
		INSIDE_PLAYER_INFO info{};
		info.pos.x = packet->data.x;
		info.pos.z = packet->data.z;
		info.m_fYaw = packet->data.yaw;

		m_pInsidePlayer[g_myid]->SetPlayerInfo(info);

		int player_cnt = 0;
		for (int i = 0; i < 3; ++i) {
			if (m_pInsidePlayer[i]->isAlive) {
				++player_cnt;
			}
		}
		matcnt = player_cnt;

	//	float y = m_pInsidePlayer[g_myid]->GetPosition().y;
	//	m_pInsidePlayer[g_myid]->SetPosition({ packet->data.x, y, packet->data.z });
		break;
	}
	case SC_CHANGE:
	{
		SC_CHANGE_PACKET* packet = reinterpret_cast<SC_CHANGE_PACKET*>(p);

		if (m_pInsidePlayer[packet->data.id]->type == packet->data.player_type) { break; }
		m_pInsidePlayer[packet->data.id]->type = packet->data.player_type;

		// 여기서 온 정보에 따라 해당 캐릭터가 특정 자리에 앉게 하거나 일어나게 한다
		if (packet->data.player_type == PlayerType::INSIDE) {
			((CTerrainPlayer*)m_pInsidePlayer[packet->data.id])->motion = AnimationState::IDLE;
			m_pInsidePlayer[packet->data.id]->SetSitState(false);
		}
		else if (packet->data.player_type == PlayerType::MOVE) {
			((CTerrainPlayer*)m_pInsidePlayer[packet->data.id])->motion = AnimationState::SIT;
			m_pInsidePlayer[packet->data.id]->SetPosition(m_pInsideScene->m_SitPos[3]);
			m_pInsidePlayer[packet->data.id]->is_update = true;
			m_pInsidePlayer[packet->data.id]->Rotate(0.f, 90.f - m_pInsidePlayer[packet->data.id]->GetYaw(), 0.f);
			m_pInsidePlayer[packet->data.id]->SetSitState(true);

		}
		else {	// Attack
			int i = (int)packet->data.player_type - 2;
			((CTerrainPlayer*)m_pInsidePlayer[packet->data.id])->motion = AnimationState::SIT;
			m_pInsidePlayer[packet->data.id]->SetPosition(m_pInsideScene->m_SitPos[i]);
			m_pInsidePlayer[packet->data.id]->is_update = true;
			m_pInsidePlayer[packet->data.id]->Rotate(0.f, 90.f * (float)i - m_pInsidePlayer[packet->data.id]->GetYaw(), 0.f);
			m_pInsidePlayer[packet->data.id]->SetSitState(true);
		}

		if (packet->data.id == g_myid) {
			// 정보에 따라 카메라/씬 전환 (MOVE : 3인칭 우주선 외부, ATTACK1/2/3 : 1인칭 공격 모드, INSIDE : 우주선 내부 3인칭)
			player_type = packet->data.player_type;
			if (player_type == PlayerType::INSIDE) {
				b_Inside = true;
			}
			else {
				b_Inside = false;
				if (player_type == PlayerType::MOVE) {
					m_pCamera = m_pPlayer[0]->ChangeCamera(DRIVE_CAMERA, m_GameTimer.GetTimeElapsed());
				}
				else if (player_type == PlayerType::ATTACK1) {
					m_pCamera = m_pPlayer[0]->ChangeCamera(ATTACT_CAMERA_L, m_GameTimer.GetTimeElapsed());
				}
				else if (player_type == PlayerType::ATTACK2) {
					m_pCamera = m_pPlayer[0]->ChangeCamera(ATTACT_CAMERA_C, m_GameTimer.GetTimeElapsed());
				}
				else if (player_type == PlayerType::ATTACK3) {
					m_pCamera = m_pPlayer[0]->ChangeCamera(ATTACT_CAMERA_R, m_GameTimer.GetTimeElapsed());
				}
			}
		}
		break;
	}
	case SC_ADD_PLAYER:
	{
		SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(p);
		m_pInsidePlayer[packet->data.id]->isAlive = true;
		m_pInsidePlayer[packet->data.id]->type = player_type;
		float y = m_pInsidePlayer[packet->data.id]->GetPosition().y;
		m_pInsidePlayer[packet->data.id]->SetPosition({ packet->data.x, y, packet->data.z });
		m_pInsidePlayer[packet->data.id]->Rotate(0.f, packet->data.yaw - m_pInsidePlayer[packet->data.id]->GetYaw(), 0.f);

		INSIDE_PLAYER_INFO info{};
		info.pos.x = packet->data.x;
		info.pos.z = packet->data.z;
		info.m_fYaw = packet->data.yaw;
		m_pInsidePlayer[packet->data.id]->SetPlayerInfo(info);
		int player_cnt = 0;
		for (int i = 0; i < 3; ++i) {
			if (m_pInsidePlayer[i]->isAlive) {
				++player_cnt;
			}
		}
		matcnt = player_cnt;
		break;
	}
	case SC_REMOVE_PLAYER:
	{
		SC_REMOVE_PLAYER_PACKET* packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(p);
		m_pInsidePlayer[packet->id]->isAlive = false;
		int player_cnt = 0;
		for (int i = 0; i < 3; ++i) {
			if (m_pInsidePlayer[i]->isAlive) {
				++player_cnt;
			}
		}
		matcnt = player_cnt;
		break;
	}
	case SC_METEO:
	{
		SC_METEO_PACKET* packet = reinterpret_cast<SC_METEO_PACKET*>(p);
		//m_pScene->TransformMeteor(packet->data);
		break;
	}
	case SC_ALL_METEOR:
	{
		SC_ALL_METEOR_PACKET* packet = reinterpret_cast<SC_ALL_METEOR_PACKET*>(p);
		m_pScene->TransformMeteor(packet->pos);
		break;
	}
	case SC_MOVE_SPACESHIP:
	{
		SC_MOVE_SPACESHIP_PACKET* packet = reinterpret_cast<SC_MOVE_SPACESHIP_PACKET*>(p);
		m_pPlayer[0]->SetPosition(packet->pos);
		m_pCamera->Update(packet->pos, m_GameTimer.GetTimeElapsed());
		m_pPlayer[0]->Update(m_GameTimer.GetTimeElapsed());
		break;
	}
	case SC_SPACESHIP_QUATERNION:
	{
		CS_SPACESHIP_QUATERNION_PACKET* packet = reinterpret_cast<CS_SPACESHIP_QUATERNION_PACKET*>(p);
		if (player_type == PlayerType::MOVE) { break; }
		m_pPlayer[0]->SetQuaternion(packet->Quaternion);
		m_pCamera->Update(m_pPlayer[0]->GetPosition(), m_GameTimer.GetTimeElapsed());
		m_pPlayer[0]->Update(m_GameTimer.GetTimeElapsed());
		break;
	}
	case SC_MOVE_INSIDEPLAYER:
	{
		SC_MOVE_INSIDE_PACKET* packet = reinterpret_cast<SC_MOVE_INSIDE_PACKET*>(p);
		m_pInsidePlayer[packet->data.id]->SetPlayerInfo(packet->data);
		break;
	}
	case SC_SPAWN_ENEMY:
	{
		SC_SPAWN_ENEMY_PACKET* packet = reinterpret_cast<SC_SPAWN_ENEMY_PACKET*>(p);

		if (!m_pScene->m_ppEnemies[packet->data.id]->isAlive) {
			m_pScene->m_ppEnemies[packet->data.id]->isAlive = true;
		}
		m_pScene->m_ppEnemies[packet->data.id]->SetPosition(packet->data.pos);

		//m_pScene->m_ppEnemies[packet->data.id]->Rotate(&packet->data.Quaternion);
		m_pScene->m_ppEnemies[packet->data.id]->m_xmf3Destination = packet->data.destination;

		m_pScene->m_ppEnemies[packet->data.id]->Maxhp = packet->data.max_hp;
		m_pScene->m_ppEnemies[packet->data.id]->hp = packet->data.max_hp;
		m_pScene->m_ppEnemies[packet->data.id]->state = packet->data.state;
		break;
	}
	case SC_MOVE_ENEMY:
	{
		SC_MOVE_ENEMY_PACKET* packet = reinterpret_cast<SC_MOVE_ENEMY_PACKET*>(p);

		//if (!m_pScene->m_ppEnemies[packet->data.id]->isAlive) {
			//m_pScene->m_ppEnemies[packet->data.id]->isAlive = true;
		//}
		if (m_pScene->m_ppEnemies[packet->data.id]->state == EnemyState::MOVE) {
			m_pScene->m_ppEnemies[packet->data.id]->LookAtPosition(0.05f, packet->data.pos);
		}

		m_pScene->m_ppEnemies[packet->data.id]->SetPosition(packet->data.pos);

		//m_pScene->m_ppEnemies[packet->data.id]->ResetRotate();
		// m_pScene->m_ppEnemies[packet->data.id]->Rotate(&packet->data.Quaternion);

		m_pScene->m_ppEnemies[packet->data.id]->isUpdate = true;
		break;
	}
	case SC_ENEMY_STATE:
	{
		SC_ENEMY_STATE_PACKET* packet = reinterpret_cast<SC_ENEMY_STATE_PACKET*>(p);

		m_pScene->m_ppEnemies[packet->data.id]->state = packet->data.state;
		break;
	}
	case SC_MOVE_BOSS:
	{
		SC_MOVE_BOSS_PACKET* packet = reinterpret_cast<SC_MOVE_BOSS_PACKET*>(p);

		m_pScene->m_ppBoss->SetQuaternion(packet->data.Quaternion);
		m_pScene->m_ppBoss->SetPosition(packet->data.pos);
		break;
	}
	case SC_MOVE_GOD:
	{
		SC_MOVE_BOSS_PACKET* packet = reinterpret_cast<SC_MOVE_BOSS_PACKET*>(p);

		m_pScene->m_ppGod->SetQuaternion(packet->data.Quaternion);
		m_pScene->m_ppGod->SetPosition(packet->data.pos);
		break;
	}
	case SC_BULLET:
	{
		SC_BULLET_PACKET* packet = reinterpret_cast<SC_BULLET_PACKET*>(p);

		for (int i = 0; i < ENEMY_BULLETS; ++i) {
			if (!m_pScene->m_ppEnemyBullets[i]->m_bActive) {
				m_pScene->m_ppEnemyBullets[i]->SetFirePosition(packet->data.pos);
				m_pScene->m_ppEnemyBullets[i]->SetMovingDirection(packet->data.direction);
				m_pScene->m_ppEnemyBullets[i]->SetEnemyFire4x4(m_pPlayer[0]->GetPosition());
				m_pScene->m_ppEnemyBullets[i]->SetActive(true);
				break;
			}
		}
		break;
	}
	case SC_BULLET_HIT:
	{
		SC_BULLET_HIT_PACKET* packet = reinterpret_cast<SC_BULLET_HIT_PACKET*>(p);

		// 적/플레이어 hp 감소. 폭발 애니메이션/소리/죽음 등
		if (packet->data.id >= 0) {	// 적 + 보스
			if (packet->data.id == BOSS_ID)	// 보스
			{
				if (packet->data.hp <= 0) {
					// 보스 죽음 (게임 클리어?)
				}
				m_pScene->m_ppBoss->BossHP = packet->data.hp;
				

				break;
			}

			if (packet->data.id == GOD_ID)	// 보스
			{
				if (packet->data.hp <= 0) {
					// 보스 죽음 (게임 클리어?)
				}
				m_pScene->m_ppGod->GodHP = packet->data.hp;

				break;
			}

			if (packet->data.hp <= 0) { // 죽음
				m_pScene->m_ppEnemies[packet->data.id]->isAlive = false;
				//m_pScene->AddDieSprite(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pScene->m_ppEnemies[packet->data.id]->GetPosition());
				m_pScene->AddDieSprite(m_pScene->m_ppEnemies[packet->data.id]->GetPosition(), packet->data.id);
				m_pScene->setParticleStart(20, m_pScene->m_ppEnemies[packet->data.id]->GetPosition());
				m_effectSound[static_cast<int>(Sounds::EXP)]->play();

			}
			else {
				m_pScene->m_ppEnemies[packet->data.id]->hp = packet->data.hp;
				m_pScene->setParticleStart(20, m_pScene->m_ppEnemies[packet->data.id]->GetPosition());
				

				cout << "att\n";
			}
		}
		else { // 플레이어 타격
			m_pPlayer[0]->hp = packet->data.hp;
			if (packet->data.hp <= 0)
			{
				pastMissionType = curMissionType;
				// 게임 오버 
			}
		}
		break;
	}
	case SC_SPAWN_MISSILE:
	{
		SC_SPAWN_MISSILE_PACKET* packet = reinterpret_cast<SC_SPAWN_MISSILE_PACKET*>(p);

		m_pScene->m_ppEnemyMissiles[packet->data.id]->SetPosition(packet->data.pos);

		if (!m_pScene->m_ppEnemyMissiles[packet->data.id]->m_bActive) {
			m_pScene->m_ppEnemyMissiles[packet->data.id]->m_bActive = true;
		}
		m_pScene->m_ppEnemyMissiles[packet->data.id]->SetPosition(packet->data.pos);
		m_pScene->m_ppEnemyMissiles[packet->data.id]->ResetRotate();
		m_pScene->m_ppEnemyMissiles[packet->data.id]->Rotate(&packet->data.Quaternion);
		break;
	}
	case SC_MISSILE:
	{
		SC_MISSILE_PACKET* packet = reinterpret_cast<SC_MISSILE_PACKET*>(p);
		/*
		if (!m_pScene->m_ppEnemyMissiles[packet->data.id]->m_bActive) {
			m_pScene->m_ppEnemyMissiles[packet->data.id]->m_bActive = true;
		}*/
		m_pScene->m_ppEnemyMissiles[packet->data.id]->LookAtPosition(0.1f, packet->data.pos);
		m_pScene->m_ppEnemyMissiles[packet->data.id]->SetPosition(packet->data.pos);
		
		//m_pScene->m_ppEnemyMissiles[packet->data.id]->ResetRotate();
		//m_pScene->m_ppEnemyMissiles[packet->data.id]->Rotate(&packet->data.Quaternion);
		break;
	}
	case SC_REMOVE_MISSILE:
	{
		SC_REMOVE_MISSILE_PACKET* packet = reinterpret_cast<SC_REMOVE_MISSILE_PACKET*>(p);
		m_pScene->m_ppEnemyMissiles[packet->id]->m_bActive = false;
		break;
	}
	case SC_ANIMATION_CHANGE:
	{
		SC_ANIMATION_CHANGE_PACKET* packet = reinterpret_cast<SC_ANIMATION_CHANGE_PACKET*>(p);
		if (packet->data.id == g_myid) { break; }
		if (packet->data.id < 3) {	// 내부 플레이어
			if (int(m_pInsidePlayer[packet->data.id]->motion) != packet->data.animation) {
				m_pInsidePlayer[packet->data.id]->motion = (AnimationState)packet->data.animation;
			}
			break;
		}
		if (packet->data.id == BOSS_ID) {
			if (curMissionType == MissionType::DEFEAT_BOSS || curMissionType == MissionType::DEFEAT_BOSS2)
				m_pScene->m_ppBoss->CurMotion = (BossAnimation)packet->data.animation;
			// 보스 State 변경??
			break;
		}
		if (packet->data.id == GOD_ID) {
			if(curMissionType == MissionType::KILL_GOD || curMissionType == MissionType::KILL_GOD2)
				m_pScene->m_ppGod->CurMotion = (GodAnimation)packet->data.animation;
			// 보스 State 변경??
			break;
		}
		break;
	}
	case SC_ITEM:
	{
		SC_ITEM_PACKET* packet = reinterpret_cast<SC_ITEM_PACKET*>(p);
		items[packet->data.type] = packet->data.num;

		if (packet->data.type == ItemType::JEWEL_HP) {
			m_pPlayer[0]->max_hp = 100 + 10 * packet->data.num;
		}

		if (packet->data.type == ItemType::JEWEL_HEAL) {
			healAmount = 10 + packet->data.num;
		}


		break;
	}
	case SC_HEAL:
	{
		isHealing = true;
		break;
	}
	case SC_MISSION_START:
	{
		SC_MISSION_START_PACKET* packet = reinterpret_cast<SC_MISSION_START_PACKET*>(p);
		curMissionType = packet->next_mission;

		if (curMissionType == MissionType::KILL_MONSTER_ONE_MORE_TIME) {
			killCnt = 0;
		}
		if (curMissionType == MissionType::CS_BAD_ENDING) {
			for (int i = 0; i < MISSILES; ++i) {
				m_pScene->m_ppEnemyMissiles[i]->m_bActive = false;
			}
		}

		if (curMissionType == MissionType::CS_SHOW_GOD) {
			m_pPlayer[0]->SetPosition(XMFLOAT3(1300.f, 0.f, -700.f));
		}
		else if (curMissionType == MissionType::CS_BOSS_SCREAM) {
			m_pPlayer[0]->SetPosition(XMFLOAT3(2300.f, 0.f, -1300.f));
		}
		else if (curMissionType != MissionType::CS_BAD_ENDING) {
			m_effectSound[static_cast<int>(Sounds::CLEAR)]->play();
		}
		break;
	}
	case SC_KILL_NUM:
	{
		SC_KILL_NUM_PACKET* packet = reinterpret_cast<SC_KILL_NUM_PACKET*>(p);
		killCnt = packet->num;
		break;
	}
	case SC_START:
	{
		_state = SCENE_INGAME;
		m_bgm[0]->play();
		break;
	}
	case SC_BLACK_HOLE:
	{
		SC_BLACK_HOLE_PACKET* packet = reinterpret_cast<SC_BLACK_HOLE_PACKET*>(p);
		m_pScene->setBlackholePos(packet->pos);
		// packet->pos; 블랙홀 위치
	}
	case SC_CUTSCENE_END_NUM:
	{
		SC_CUTSCENE_END_NUM_PACKET* packet = reinterpret_cast<SC_CUTSCENE_END_NUM_PACKET*>(p);
		skipnum = packet->num;	// 스킵한 클라 수
		break;
	}
	case SC_BLACK_HOLE_TIME:
	{
		SC_BLACK_HOLE_TIME_PACKET* packet = reinterpret_cast<SC_BLACK_HOLE_TIME_PACKET*>(p);
		blackholetime = packet->time;	// 블랙홀 시간 float 타입 30.0f -> 0.0f
		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", p[1]);
		break;
	}
}
