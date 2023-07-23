#pragma once

// (구)640 X 480 VGA


#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "Sound.h"


extern int g_myid;

enum SCENE_STATE {
	SCENE_LOBBY, SCENE_INGAME
};

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

    void BuildObjects();
	void BuildSounds();

    void ReleaseObjects();

    void ProcessInput();
    void AnimateObjects();
    void FrameAdvance();

	void UpdateUI();
	wstring ChangeMission(MissionType mT);
	wstring ChangeScripts(MissionType mT);
	wstring ChangeBossScripts(MissionType mT);



	chrono::steady_clock::time_point scriptsStartTime;
	bool scriptsOn = false;

	short matcnt = 0;
	float blackholetime = 0.0f;

	// 서버
	bool ConnectServer();
	void RecvServer();
	void ProcessPacket(char* p);

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	//
	void CheckSceneChange(bool State, int num);
	bool AroundSculpture();
	void CameraUpdateChange();


private:
	HINSTANCE					m_hInstance;
	HWND						m_hWnd; 

	int							m_nWndClientWidth;
	int							m_nWndClientHeight;
        
	IDXGIFactory4				*m_pdxgiFactory = NULL;
	IDXGISwapChain3				*m_pdxgiSwapChain = NULL;
	ID3D12Device				*m_pd3dDevice = NULL;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	static const UINT			m_nSwapChainBuffers = 2;
	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource				*m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12Resource				* m_pd3dGodRayShadowMap;				//그림자맵 
	ID3D12DescriptorHeap		*m_pd3dRtvDescriptorHeap = NULL;

	ID3D12Resource				*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap		*m_pd3dDsvDescriptorHeap = NULL;

	ID3D12CommandAllocator		*m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue			*m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList	*m_pd3dCommandList = NULL;

	ID3D12Fence					*m_pd3dFence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;

#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;

	CScene						*m_pScene = NULL;
	CAirplanePlayer						*m_pPlayer[3] = {};
	CCamera						*m_pCamera = NULL;
	bool iscut =false;
	bool isending = false;

	DWORD m_pBeforeCamera = NULL;


	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[70];

	UILayer* m_pUILayer = NULL;
	CUI* m_pUI = NULL;



	CSound* m_bgm;
	CSound* m_lobbybgm;

	CSound* m_effectSound[static_cast<int>(Sounds::COUNT)] = {};


	int a = 0;
	int b = 0;

	//
	wstring roomNum;
	short room_num = 0;
	short killCnt = 0; 
	short skipnum;
	
	float pDist = 0;
	float bDist = 0;

	int  bossdie = 0;

	short firstSc = -2;
	MissionType curMissionType = MissionType::TU_SIT;
	MissionType pastMissionType = MissionType::TU_END;

	XMFLOAT3 planetPos = {};

	// 내부 
	CScene* m_pInsideScene = NULL;
	CTerrainPlayer* m_pInsidePlayer[3] = {};
	CCamera* m_pInsideCamera = NULL;

	bool b_Inside;
	bool b_BeforeCheckInside = false;

	bool b_CameraScene = false;

	bool turnon = true;
	// 
	

	// 서버
	PlayerType player_type;

	// 보석 개수 > ItemType : 무슨 보석인지 / char : 해당 보석 개수
	std::unordered_map<ItemType, char> items;

	SOCKET sock;

	char buf[RECV_BUF_SIZE * 2];
	int buf_cur_size = 0;

	bool isHealing = false;
	bool isConnect = false;

	// 현재 회복량
	char healAmount = 10;

	SCENE_STATE _state;

};

