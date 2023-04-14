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
	

	scriptsStartTime = steady_clock::now();
	_tcscpy_s(m_pszFrameRate, _T("LabProject ("));
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

	BuildObjects();
	isConnect = ConnectServer();

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
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void **)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
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

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
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
			case VK_SPACE:
				if (((CTerrainPlayer*)m_pInsidePlayer[g_myid])->motion != AnimationState::SIT) {
					((CTerrainPlayer*)m_pInsidePlayer[g_myid])->motion = AnimationState::SIT;
					std::cout << "앉기";
				}
				else
				{
					std::cout << "서기";
					((CTerrainPlayer*)m_pInsidePlayer[g_myid])->motion = AnimationState::IDLE;
				}

				break;
			case VK_TAB:
				b_Inside = !b_Inside;
				std::cout << "씬 전환";
				break;

			case 'F': //F키 상호작용 앉기
				if (b_Inside || m_pInsidePlayer[g_myid]->GetSitState())
				{
					int State = m_pInsideScene->CheckSitCollisions();
					if (!isConnect) {
						CheckSceneChange(m_pInsidePlayer[g_myid]->GetSitState(), State);
					}
					if (State == 3) {
						CS_CHANGE_PACKET my_packet;
						my_packet.size = sizeof(CS_CHANGE_PACKET);
						my_packet.type = CS_CHANGE;
						my_packet.player_type = PlayerType::MOVE;
						send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
					}
					else if(State >= 0) {
						CS_CHANGE_PACKET my_packet;
						my_packet.size = sizeof(CS_CHANGE_PACKET);
						my_packet.type = CS_CHANGE;
						my_packet.player_type = (PlayerType)(State + 2);
						send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
					}
				}

				if (b_Inside) {
					scriptsOn = true;
				}


				if (player_type != PlayerType::INSIDE) {
					CS_CHANGE_PACKET my_packet;
					my_packet.size = sizeof(CS_CHANGE_PACKET);
					my_packet.type = CS_CHANGE;
					my_packet.player_type = PlayerType::INSIDE;
					send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
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
			if (player_type >= PlayerType::ATTACK1 && player_type <= PlayerType::ATTACK3) {
				CS_ATTACK_PACKET packet;
				packet.size = sizeof(packet);
				packet.type = CS_ATTACK;

				packet.data.pos = m_pCamera->GetPosition();
				packet.data.direction = m_pCamera->GetLookVector();
				send(sock, reinterpret_cast<char*>(&packet), sizeof(packet), NULL);
			}
			else if (player_type == PlayerType::INSIDE && AroundSculpture()) {
				CS_HEAL_PACKET my_packet;
				my_packet.size = sizeof(CS_HEAL_PACKET);
				my_packet.type = CS_HEAL;
				my_packet.start = true;
				send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
			}
			break;
        case WM_RBUTTONDOWN:
			break;
        case WM_LBUTTONUP:
			if (isHealing && player_type == PlayerType::INSIDE && AroundSculpture()) {
				CS_HEAL_PACKET my_packet;
				my_packet.size = sizeof(CS_HEAL_PACKET);
				my_packet.type = CS_HEAL;
				my_packet.start = false;
				send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
				isHealing = false;
			}
			break;
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:
			OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
            break;
        case WM_KEYDOWN:
			switch (nMessageID)
			{
			case 'F':
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

void CGameFramework::OnDestroy()
{
    ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();

	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
    if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();

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
		pPlayer[i] = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pScene->m_pTerrain,i);
		pPlayer[i]->SetPosition(XMFLOAT3(425.0f + 10.0f * i, 250.0f, 640.0f));
		pPlayer[i]->SetScale(XMFLOAT3(15.0f, 15.0f, 15.0f));
	}

	CAirplanePlayer* pAirPlayer[3];
		pAirPlayer[0]= new CAirplanePlayer(m_pd3dDevice, m_pd3dCommandList, m_pInsideScene->GetGraphicsRootSignature(), m_pInsideScene->m_pTerrain);
		pAirPlayer[0]->SetPosition(XMFLOAT3(425.0f, 250.0f, 640.0f));
		pAirPlayer[0]->SetScale(XMFLOAT3(15.0f, 15.0f, 15.0f));


#else
	CAirplanePlayer *pPlayer = new CAirplanePlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), NULL);
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
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	if (m_pInsideScene) m_pInsideScene->ReleaseUploadBuffers();
	if (m_pPlayer) for(int i=0; i< m_pScene->m_nScenePlayer; ++i)m_pPlayer[i]->ReleaseUploadBuffers();
	if (m_pInsidePlayer)for (int i = 0; i < m_pInsideScene->m_nScenePlayer; ++i) m_pInsidePlayer[i]->ReleaseUploadBuffers();

	m_GameTimer.Reset();
}

void CGameFramework::ReleaseObjects()
{
	if (m_pUILayer) m_pUILayer->ReleaseResources();
	if (m_pUILayer) delete m_pUILayer;

	if (m_pPlayer) for (int i = 0; i < 3; ++i)m_pPlayer[i]->Release();
	if (m_pInsidePlayer) for (int i = 0; i < 3; ++i)m_pInsidePlayer[i]->Release();

	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
	if (m_pInsideScene) m_pInsideScene->ReleaseObjects();
	if (m_pInsideScene) delete m_pInsideScene;
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

		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				if (b_Inside && !isConnect || player_type == PlayerType::INSIDE && isConnect) {
					m_pInsidePlayer[g_myid]->Rotate(0.0f, cxDelta, 0.0f);
				}
				else {
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
					send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
				}
				else if(player_type == PlayerType::MOVE) {
					CS_SPACESHIP_PACKET my_packet;
					my_packet.size = sizeof(CS_SPACESHIP_PACKET);
					my_packet.type = CS_SPACESHIP_MOVE;
					my_packet.data.dwDirection = dwDirection;
					XMFLOAT4 a;
					XMStoreFloat4(&a, m_pPlayer[0]->GetQuaternion());
					my_packet.data.Quaternion = a;
						
					send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
				}
			}
			else {
				//이동 부분 분할할지 모르겠어서 우선 여따 같이넣어둠 
				if (dwDirection&&!b_Inside) m_pPlayer[0]->Move(dwDirection, 20.25f, true);
				if (dwDirection&& b_Inside) m_pInsidePlayer[g_myid]->Move(dwDirection, 10.25f, true);
			}
		}

	}

	if (isConnect) {
		m_pPlayer[0]->UpdateOnServer(player_type != PlayerType::MOVE);
		for (int i = 0; i < 3; ++i)m_pInsidePlayer[i]->UpdateOnServer(i != g_myid);
	}
	if(!b_Inside) for (int i = 0; i < 1; ++i)m_pPlayer[i]->Update(m_GameTimer.GetTimeElapsed());
	else for (int i = 0; i < 3; ++i)m_pInsidePlayer[i]->Update(m_GameTimer.GetTimeElapsed());

	if (player_type == PlayerType::INSIDE && isHealing && !AroundSculpture()) {
		CS_HEAL_PACKET my_packet;
		my_packet.size = sizeof(CS_HEAL_PACKET);
		my_packet.type = CS_HEAL;
		my_packet.start = false;
		send(sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
		isHealing = false;
	}
}

void CGameFramework::AnimateObjects()
{
	RecvServer();

	float fTimeElapsed = m_GameTimer.GetTimeElapsed();

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

    AnimateObjects();

	UpdateUI();


	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

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

	if(b_CameraScene)b_Inside = b_CameraScene = m_pInsideCamera->CameraSence1(b_CameraScene);
	if (m_pScene&&!b_Inside) m_pScene->Render(m_pd3dCommandList, m_pCamera);
	if (m_pInsideScene&&b_Inside) m_pInsideScene->Render(m_pd3dCommandList, m_pInsideCamera);

#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif
	if (m_pPlayer && !b_Inside) for (int i = 0; i < 1; ++i)m_pPlayer[i]->Render(m_pd3dCommandList, m_pCamera);
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
	m_pUILayer->Render(m_nSwapChainBufferIndex);

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


	if (duration_cast<seconds>(steady_clock::now() - scriptsStartTime).count() >= 5)
	{
		uiScripts = L"...";
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
	uiJew += L"     ";
	uiJew += JEWEL_DEF;
	uiJew += L"\n";
	uiJew += JEWEL_HEAL;
	uiJew += L"     ";
	uiJew += JEWEL_HP;

	m_pUILayer->UpdateLabels(uiText);
	m_pUILayer->UpdateLabels_Scripts(uiScripts);

	m_pUILayer->UpdateLabels_Jew(uiJew);


	m_pUILayer->UpdateHp(m_pPlayer[0]->getHp());
	//for (int i = 0; i < ENEMIES; ++i)
	//{
	//	m_pUILayer->UpdateDots(i, { 0,0 });
	//}
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();

	for (int i = 0; i < ENEMIES; ++i) {
		m_pUILayer->UpdateDots(fTimeElapsed, i, m_pPlayer[0], m_pScene->m_ppEnemies[i]->GetPosition());
	}

}

wstring CGameFramework::ChangeMission(MissionType mType)
{
	wstring uiText = L"미션 - ";
	wstring enemyCountStr;
	wstring jewelCntStr;

	enemyCountStr = to_wstring(killCnt);
	jewelCntStr = to_wstring(jewelCnt);

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
		uiText = L"미션 - 보석 4종류 1개 이상씩 얻어라 ( ";
		uiText += jewelCntStr;
		uiText += L" / 4 )";

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
		
		uiText += L"m 남음";

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

		break;
	}
	case MissionType::DEFEAT_BOSS:
	{
		uiText = L"미션 - 보스를 처치하라 ( ";

		uiText += L"0 / 1 )";

		break;
	}

	default:
		uiText = L"미션 검색중";
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
	case MissionType::TU_SIT:
	{
		uiScripts = L"우선 조종석에 앉아서 우주선을 조종해 봐!";
		if (firstSc == 0) {
			cout << "0";
			firstSc = 1;
			scriptsOn = true;
		}
	
		break;
	}
	case MissionType::TU_KILL:
	{
		uiScripts = L"잘했어! 다음은 공격석에 앉아서 적을 처치해 봐!";
		if (firstSc == 1) {
			cout << "1";
			firstSc = 2;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::TU_HILL:
	{
		uiScripts = L"이런! 우주선의 체력이 감소했어!\n조각상에서 좌클릭을 해서 체력을 회복시켜 봐!";
		if (firstSc == 2) {
			firstSc = 3;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::GET_JEWELS:
	{
		uiScripts = L"기본 조작은 설명해 줬으니 이제 본격적으로 들어가야겠지?\n드래곤을 물리치기 위해선우리 우주선을 강화해야 해.\n특별한 에너지를 가진 보석을 모아서 능력치를 올려보자!";
		if (firstSc == 3) {
			firstSc = 4;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::Kill_MONSTER:
	{
		uiScripts = L"이런... 우주선을 방해하는 놈들이 있어,\n저놈들을 해치우고 앞으로 나아가자";
		if (firstSc == 4) {
			firstSc = 5;
			scriptsOn = true;
		}
		break;
	}
	case MissionType::GO_PLANET:
	{
		uiScripts = L"좋았어! 00행성에 보스가 있다는 정보를 입수했어! \n00행성까지 가자!";
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

	default:
		uiScripts = L"파이팅! 파이팅!";
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

			while (buf_cur_size) {
				packet_size = buf[0];
				if (buf_cur_size >= packet_size && buf[0] != 0) {
					ProcessPacket(buf);
					buf_cur_size -= buf[0];
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
		break;
	}
	case SC_REMOVE_PLAYER:
	{
		SC_REMOVE_PLAYER_PACKET* packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(p);
		m_pInsidePlayer[packet->id]->isAlive = false;
		break;
	}
	case SC_SPAWN_METEO:
	{
		SC_SPAWN_METEO_PACKET* packet = reinterpret_cast<SC_SPAWN_METEO_PACKET*>(p);
		m_pScene->RespawnMeteor(m_pd3dDevice, m_pd3dCommandList, packet->data);
		break;
	}
	case SC_METEO:
	{
		SC_METEO_PACKET* packet = reinterpret_cast<SC_METEO_PACKET*>(p);
		m_pScene->TransformMeteor(packet->data);
		break;
	}
	case SC_METEO_DIRECTION:
	{
		SC_METEO_DIRECTION_PACKET* packet = reinterpret_cast<SC_METEO_DIRECTION_PACKET*>(p);
		m_pScene->m_ppMeteorObjects[packet->data.id]->m_xmf3MovingDirection = packet->data.dir;
		break;
	}
	case SC_MOVE_SPACESHIP:
	{
		SC_MOVE_SPACESHIP_PACKET* packet = reinterpret_cast<SC_MOVE_SPACESHIP_PACKET*>(p);
		m_pPlayer[0]->SetPlayerInfo(packet->data);
		//m_pPlayer[0]->SetPosition(playerInfo.pos);
		//m_pPlayer[0]->Rotate(0.0f, playerInfo.m_fYaw - m_pPlayer[0]->GetYaw(), 0.0f);
		//m_pCamera->Update(playerInfo.pos, m_GameTimer.GetTimeElapsed());
		//m_pPlayer->SetVelocity(playerInfo[3].velocity);
		//m_pPlayer->SetShift(playerInfo.shift);
		//m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
		//m_pPlayer[g_myid]->SetPlayerInfo(playerInfo[3]);
		break;
	}
	case SC_MOVE_INSIDEPLAYER:
	{
		SC_MOVE_INSIDE_PACKET* packet = reinterpret_cast<SC_MOVE_INSIDE_PACKET*>(p);
		m_pInsidePlayer[packet->data.id]->SetPlayerInfo(packet->data);
		if (int(m_pInsidePlayer[packet->data.id]->motion) != (int)packet->data.animation) {
			((CTerrainPlayer*)m_pInsidePlayer[packet->data.id])->motion = (AnimationState)packet->data.animation;
		}
		//m_pInsidePlayer[playerInfo.id]->SetPosition(playerInfo.pos);
		//m_pInsidePlayer[playerInfo.id]->Rotate(0.0f, playerInfo.m_fYaw - m_pPlayer[playerInfo.id]->GetYaw(), 0.0f);
		break;
	}
	case SC_SPAWN_ENEMY:
	{
		SC_SPAWN_ENEMY_PACKET* packet = reinterpret_cast<SC_SPAWN_ENEMY_PACKET*>(p);

		if (!m_pScene->m_ppEnemies[packet->data.id]->isAlive) {
			m_pScene->m_ppEnemies[packet->data.id]->isAlive = true;
		}
		m_pScene->m_ppEnemies[packet->data.id]->SetPosition(packet->data.pos);

		m_pScene->m_ppEnemies[packet->data.id]->ResetRotate();
		m_pScene->m_ppEnemies[packet->data.id]->Rotate(&packet->data.Quaternion);
		m_pScene->m_ppEnemies[packet->data.id]->destination = packet->data.destination;

		m_pScene->m_ppEnemies[packet->data.id]->Maxhp = packet->data.max_hp;
		m_pScene->m_ppEnemies[packet->data.id]->hp = packet->data.max_hp;
		break;
	}
	case SC_MOVE_ENEMY:
	{
		SC_MOVE_ENEMY_PACKET* packet = reinterpret_cast<SC_MOVE_ENEMY_PACKET*>(p);

		if (packet->data.id == BOSS_ID) {
			m_pScene->m_ppBoss->SetQuaternion(packet->data.Quaternion);
			m_pScene->m_ppBoss->SetPosition(packet->data.pos);
			break;
		}

		//if (!m_pScene->m_ppEnemies[packet->data.id]->isAlive) {
			//m_pScene->m_ppEnemies[packet->data.id]->isAlive = true;
		//}
		m_pScene->m_ppEnemies[packet->data.id]->SetPosition(packet->data.pos);

		m_pScene->m_ppEnemies[packet->data.id]->ResetRotate();
		m_pScene->m_ppEnemies[packet->data.id]->Rotate(&packet->data.Quaternion);
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
		if (packet->data.id >= 0) {
			if (packet->data.hp <= 0) { // 죽음
				m_pScene->m_ppEnemies[packet->data.id]->isAlive = false;

			}
			else {
				m_pScene->m_ppEnemies[packet->data.id]->hp = packet->data.hp;
			}
		}
		else { // 플레이어 타격
			m_pPlayer[0]->hp = packet->data.hp;
			if (packet->data.hp <= 0)
			{
				// 게임 오버 
			}
		}
		break;
	}
	case SC_MISSILE:
	{
		SC_MISSILE_PACKET* packet = reinterpret_cast<SC_MISSILE_PACKET*>(p);

		m_pScene->m_ppEnemyMissiles[packet->data.id]->SetPosition(packet->data.pos);

		if (!m_pScene->m_ppEnemyMissiles[packet->data.id]->m_bActive) {
			m_pScene->m_ppEnemyMissiles[packet->data.id]->m_bActive = true;
		}
		m_pScene->m_ppEnemyMissiles[packet->data.id]->SetPosition(packet->data.pos);
		m_pScene->m_ppEnemyMissiles[packet->data.id]->ResetRotate();
		m_pScene->m_ppEnemyMissiles[packet->data.id]->Rotate(&packet->data.Quaternion);
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

		if (packet->data.id < 3) {	// 내부 플레이어
			if (int(m_pInsidePlayer[packet->data.id]->motion) != packet->data.animation) {
				m_pInsidePlayer[packet->data.id]->motion = (AnimationState)packet->data.animation;
			}
			break;
		}
		if (packet->data.id == BOSS_ID) {
			m_pScene->m_ppBoss->CurMotion = (BossAnimation)packet->data.animation;
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

		jewelCnt += items[packet->data.type];

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
		break;
	}
	case SC_KILL_NUM:
	{
		SC_KILL_NUM_PACKET* packet = reinterpret_cast<SC_KILL_NUM_PACKET*>(p);
		killCnt = packet->num;
		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", p[1]);
		break;
	}
}
