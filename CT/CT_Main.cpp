#include "stdafx.h"
#include "CT_Main.h"
#include "ShellContextMenu.h"
#include "CommonUtils.h"

#include <iostream>

// 메인 클래스 전역 변수
CtMain* g_pCTMain = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief DXUT 콜백 함수들. 
 
 DXUT 기본 콜백 함수들로 DXUT 초기화시에 등록하는 함수들이다.
 여기서는 단순히 메인 클래스의 같은 이름의 핸들러 함수로 연결시킨다. 이유는 메인클래스 멤버들을 편하게 사용하기 위함.
 메인 클래스 멤버 자체를 콜백함수로 사용하는것도 가능하나 이 방법이 더 낫다.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace CALLBACKS
{
	bool CALLBACK 
	IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
	{
		return g_pCTMain->IsDeviceAcceptable( pCaps, AdapterFormat, BackBufferFormat, bWindowed, pUserContext );
	}

	bool CALLBACK 
	ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
	{
		return g_pCTMain->ModifyDeviceSettings( pDeviceSettings, pUserContext );
	}

	HRESULT CALLBACK 
	OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
	{
		return g_pCTMain->OnCreateDevice( pd3dDevice, pBackBufferSurfaceDesc, pUserContext );
	}

	HRESULT CALLBACK 
	OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
	{
		return g_pCTMain->OnResetDevice( pd3dDevice, pBackBufferSurfaceDesc, pUserContext );
	}

	void CALLBACK 
	OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
	{
		g_pCTMain->OnFrameMove( pd3dDevice, fTime, fElapsedTime, pUserContext );
	}

	void CALLBACK 
	OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
	{	
		g_pCTMain->OnFrameRender( pd3dDevice, fTime, fElapsedTime, pUserContext );
	}

	LRESULT CALLBACK 
	MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
	{	
		return g_pCTMain->MsgProc( hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing, pUserContext );
	}

	void CALLBACK 
	OnLostDevice( void* pUserContext )
	{
		g_pCTMain->OnLostDevice( pUserContext );
	}

	void CALLBACK 
	OnDestroyDevice( void* pUserContext )
	{
		g_pCTMain->OnDestroyDevice( pUserContext );
	}

	/// CustomUI 이벤트 핸들러. 
	void CALLBACK 
	OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
	{
		g_pCTMain->OnGUIEvent( nEvent, nControlID, pControl, pUserContext );
	}		
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 큐브의 포함 객체들을 로드하는 Thread 함수

 큐브 로딩과 렌더링을 병행하기 위해 쓰레드를 만들어 수행한다. LoadObjects()를 쓰레드로 수행하고
 종료하기 전에 메시지를 보내 로드가 끝났음을 알린다.

 * \param pParam	로드할 큐브 객체 포인터
 * \return			에러 코드
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT
ThreadLoadCubeObjects( LPVOID pParam )
{	
	CtObjectCube* pCube = (CtObjectCube*)pParam;
	g_pCTMain->LoadObjects( pCube );			

	// 유저 메시지를 보내 로드가 끝났음을 알린다
	// CtMain::MsgProc 에서 메시지를 받아서 해당 큐브에 대한 Serialize()를 수행한다.
	SendMessage( AfxGetMainWnd()->m_hWnd, WM_USER, (WPARAM)pParam, 0 );
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 클래스 생성자

 메인 클래스는 CCTApp 클래스에 멤버변수로 선언되어 있기 때문에 생성자는 기본 생성자만 사용한다.
 생성자를 오버로딩할 필요가 있으면 메인클래스를 동적으로 생성하도록 수정해 사용한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CtMain::CtMain( VOID )
{
	ZeroMemory( &m_stInputState, sizeof(stInputState) );
	ZeroMemory( &m_stStateFlags, sizeof(stStateFlags) );
	
	m_pMainCube						= NULL;
	m_pCurrentCube					= NULL;
	m_pChangingCube					= NULL;

	m_pHoveredObj					= NULL;
	m_pSelectedObj					= NULL;
	m_pMorphingSrcObj				= NULL;
	m_pMorphingTgtObj				= NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 클래스 소멸자

 모든 제거 작업을 수행. 동적으로 생성한 모든 객체들을 파괴해야 한다. 
 모든 필요한 제거 함수들을 여기에 작성한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CtMain::~CtMain( VOID )
{	
	SAFE_RELEASE( m_pd3dDevice );
	delete m_pMainCube;	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 메인 클래스 직렬화 함수

 CtObjectBase 클래스의 Serialize 함수는 객체 자신에 대한 직렬화이고, 이 메인 클래스 직렬화 함수는
 프로그램 전체 범위에서의 직렬화 기능이다. 메인 클래스의 유지해야할 속성들을 이 함수를 통해 저장 및 복구한다.
 프로그램 환경설정 등의 셋팅값들은 INI 파일을 활용하고 여기에서는 INI 파일에 저장 및 복구하기 까다로운(예를 들어 행렬)
 속성들을 다룬다.

 * \param bStoring	TRUE인 경우 저장, FALSE인 경우 복구
 * \return			없음
 */
 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::Serialize( BOOL bStoring )
{
	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 메인 클래스 초기화 함수

 프로그램 구동 초기화를 모두 수행하는 함수이다. 

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT 
CtMain::Initialize( VOID )
{	
	HRESULT hr;

	// GDI+ 초기화
	ULONG_PTR gpToken;
	GdiplusStartupInput gpsi;
	if ( GdiplusStartup(&gpToken,&gpsi,NULL) != Ok ) {
		MessageBox(NULL, L"GDI+ 라이브러리를 초기화할 수 없습니다.",L"알림",MB_OK);
		exit(0);
	}

	// Direct3D 초기화
	V( InitD3D() );

	// 기본 메쉬 로드
	V( m_MeshChip.Create( m_pd3dDevice, DEFAULT_MESHFILE_CHIP ) );
	V( m_MeshCube.Create( m_pd3dDevice, DEFAULT_MESHFILE_CUBE ) );
	V( m_MeshImage.Create( m_pd3dDevice, DEFAULT_MESHFILE_IMAGE ) );
	V( m_MeshWorld.Create( m_pd3dDevice, DEFAULT_MESHFILE_WORLD ) );

	// 숏컷 화살표 모양을 잠시 제거
	//bool bShortcutEnabled = SetShortCutArrowDisable();

	// 레지스트리 복구
	//if( bShortcutEnabled ) SetShortCutArrowEnable();

	// 홈 디렉토리 경로 저장
	GetCurrentDirectory( 255, m_lpszHomeDirectory );

	// PhysX 초기화
	m_PhysX.Init();
	
	// 카메라 초기화
	InitCamera();

	// 렌더매니저 
	m_RenderManager.Init();	

	// 커스텀 UI 초기화
	InitCustomUI();

	// 메인 큐브 객체 로드	
	MakeMainCube();
	AfxBeginThread( ThreadLoadCubeObjects, m_pMainCube, THREAD_PRIORITY_LOWEST );
	
	// 기본 VB, IB 초기화
	InitBuffers();		

	// 고정 조명 : 차후 Render함수에 넣어야할것	
	SetupLights();

	// 툴 초기화
	m_ToolLasso.Init( DEFAULT_LASSOVERTEX_SIZE );	

	return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Direct3D 초기화 함수. 
 
 기본 DXUT 초기화 루틴을 수행하고 필요한 추가 작업을 작성한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT 
CtMain::InitD3D( VOID )
{	
	HRESULT hr;

	CRect rect;
	SystemParametersInfo( SPI_GETWORKAREA, 0, &rect, 0 ); 
	HWND hWnd = AfxGetMainWnd()->m_hWnd;	

	//////////////////////////////////////////////////////////////////////////
	// DXUT 콜백함수 등록 및 초기화	
	DXUTSetCallbackDeviceCreated( CALLBACKS::OnCreateDevice ); 
	DXUTSetCallbackDeviceReset( CALLBACKS::OnResetDevice ); 
	DXUTSetCallbackDeviceLost( CALLBACKS::OnLostDevice ); 
	DXUTSetCallbackDeviceDestroyed( CALLBACKS::OnDestroyDevice ); 
	DXUTSetCallbackMsgProc( CALLBACKS::MsgProc ); 	
	DXUTSetCallbackFrameRender( CALLBACKS::OnFrameRender ); 
	DXUTSetCallbackFrameMove( CALLBACKS::OnFrameMove ); 
	//////////////////////////////////////////////////////////////////////////
	
	V( DXUTInit(true, true) );	
	DXUTSetCursorSettings( true, true );	

	// MFC의 윈도우를 DXUT윈도우로 셋트한다. 이걸 해주지 않으면 아래 DXUTCreateDevice~함수에서 윈도우를 만들어 버림.
	V( DXUTSetWindow(hWnd, hWnd, hWnd) );

	//////////////////////////////////////////////////////////////////////////
	// 장치 생성. 
	DXUTDeviceSettings ds;
	V( DXUTFindValidDeviceSettings(&ds) );
	ds.pp.BackBufferWidth			= rect.Width();
	ds.pp.BackBufferHeight			= rect.Height();		
	ds.pp.AutoDepthStencilFormat	= D3DFMT_D24S8;
	ds.pp.PresentationInterval		= D3DPRESENT_INTERVAL_IMMEDIATE;
	
	// FSAA
	LPDIRECT3D9 pD3D = DXUTGetD3DObject();
	DWORD aalevel;
	if( SUCCEEDED(pD3D->CheckDeviceMultiSampleType( D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL , D3DFMT_D24S8, FALSE, 
		D3DMULTISAMPLE_8_SAMPLES, &aalevel ) ) )
	{
		ds.pp.MultiSampleQuality = aalevel-1;
		ds.pp.MultiSampleType = D3DMULTISAMPLE_8_SAMPLES;
	}
	else if( SUCCEEDED(pD3D->CheckDeviceMultiSampleType( D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL , D3DFMT_D24S8, FALSE, 
		D3DMULTISAMPLE_2_SAMPLES, &aalevel ) ) )
	{
		ds.pp.MultiSampleQuality = aalevel-1;
		ds.pp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
	}

	V( DXUTCreateDeviceFromSettings(&ds) );	
	//////////////////////////////////////////////////////////////////////////

	m_pd3dDevice = DXUTGetD3DDevice();

	return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 마우스 피킹 함수.

 마우스 포인터가 가리키는 지점의 3D 좌표를 계산하고, 롤오버에 따른 추가 작업도 수행한다.

 * \param VOID	없음
 * \return		Eye로부터 히트된 3D지점까지의 거리
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FLOAT
CtMain::CheckHover( void )
{
	D3DXVECTOR3		vDir, vOrig;
	CPoint			point;
	NxVec3			vMouse;
	NxRay			worldRay;
	NxRaycastHit	nHit;
	NxShape			*pHitShape = NULL;
	
	// 피킹 ray를 정의
	GetUnprojectedPoint( m_stInputState.ptMousePos, &vDir, &vOrig );
	worldRay.dir = NxVec3(vDir.x, vDir.y, vDir.z);
	worldRay.orig = NxVec3(vOrig.x, vOrig.y, vOrig.z); 

	// 초기화
	m_pHoveredObj = NULL;
	m_stStateFlags.bChipRotateModeReady = FALSE;

	// 현재 모드에 따라 피킹을 다르게 수행
	if( m_stStateFlags.bDragMode )	
	{
		if( m_stStateFlags.bLiftMode )	pHitShape = m_pCurrentCube->GetScene()->raycastClosestShape( worldRay, NX_ALL_SHAPES, nHit, ACTOR_GROUP_SELECTED, NX_MAX_F32, 0xffffffff, 0, 0 );					
		else				pHitShape = m_pCurrentCube->GetScene()->raycastClosestShape( worldRay, NX_ALL_SHAPES, nHit, ACTOR_GROUP_DEFAULT, NX_MAX_F32, 0xffffffff, 0, 0 );					
	}
	else 
	{
		pHitShape = m_pCurrentCube->GetScene()->raycastClosestShape( worldRay, NX_ALL_SHAPES, nHit, 0xffffffff, NX_MAX_F32, 0xffffffff, 0, 0 );			
	}

	// 마우스 3D 좌표 결정
	vMouse = worldRay.orig + worldRay.dir * nHit.distance;
	m_stInputState.v3DMousePos = NxVectorToD3DVector( vMouse );

	//////////////////////////////////////////////////////////////////////////
	// 롤오버된 객체가 있는 경우 추가 작업 수행
	if( pHitShape ) 
	{		
		m_pHoveredObj = (CtObjectBase*)pHitShape->getActor().userData;
		if( m_pHoveredObj )//&& !m_stStateFlags.bDragMode ) 
		{
			m_pHoveredObj->Hover();

			// 포인터가 가리키는 지점이 객체 메쉬의 모서리 부분인지 확인
			NxVec3 vOffset = vMouse - m_pHoveredObj->GetPose().t;
			NxVec3 vDir2 = (vMouse + vOffset * 0.2f) - worldRay.orig;
			vDir2.normalize();
			worldRay.dir = vDir2;
			if( !pHitShape->raycast( worldRay, m_pCurrentCam->GetFarClip(), NX_RAYCAST_SHAPE, nHit, TRUE ) )
			{
				if( m_pHoveredObj->IsPiled() ) 
				{
					CtObjectChip *pChip = (CtObjectChip*)m_pHoveredObj;
					if( pChip->IsBoundaryOfFile() ) m_stStateFlags.bChipRotateModeReady = TRUE;
				}
				else m_stStateFlags.bChipRotateModeReady = TRUE;
			}
		}
	}	
	//////////////////////////////////////////////////////////////////////////

	return nHit.distance;
}

/*************************************************************************************************************************************************************/
/* 객체 속성 변경 관련 함수들                                                                                                                                */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
* \brief 객체의 크기를 각 축에 대해 변화시키는 함수.

각 축의 크기는 상대값(offset)이며 벡터로 표현한다. 기본값은 (1,1,1)로 실제 메시가 갖는 크기이며, (2,1,1)로 변경한 경우 x축 방향으로 2배 커진다.

* \param pObj		객체 포인터
* \param xAmount	X축 크기
* \param yAmount	Y축 크기
* \param zAmount	Z축 크기
* \return			없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID		
CtMain::ScaleObject( CtObjectBase* pObj, FLOAT xAmount, FLOAT yAmount, FLOAT zAmount )
{
	pObj->Scale( xAmount, yAmount, zAmount );
	if( pObj->HasCustomMesh() ) m_PhysX.GenerateTriangleMesh( pObj, TRUE );
}

/*************************************************************************************************************************************************************/
/* 선택물 및 툴 관련 함수들																																	 */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 선택물 클리어 함수. 선택물 배열을 비우고 객체들을 선택 해제한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::ClearSelection( VOID )
{
	m_OSelection.Clear();
	m_CustomUI.OnObjectSelectionClear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
* \brief 파일명으로부터 객체를 선택하는 함수.

* \param filename	파일명
* \return			없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID		
CtMain::SelectObjectByFilename( LPCWSTR filename )
{
	CtObjectBase** pObjects = m_pCurrentCube->GetObjects();
	for( DWORD i=0; i < m_pCurrentCube->GetNumObjects(); i++ )
	{
		if( !lstrcmpW(pObjects[i]->GetFileInfo()->lpszFileName, filename) ) 
		{
			m_OSelection.Add( pObjects[i] );
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 마우스 드래그 모드 시작 함수.

 화면에 마우스를 드래그하는 경우는 객체를 이동하거나 선택 영역을 지정하는 경우 등이다. 이에 대한 준비 작업을 수행한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::StartDragMode( VOID )
{
	m_stStateFlags.bDragMode = TRUE;	
	m_stStateFlags.bLassoMode = FALSE;

	m_stInputState.vStartDragPos = m_OSelection.GetCenter();
	m_stInputState.vStartDragLocalPos = m_stInputState.vStartDragPos - m_OSelection.GetCenter();

	// 선택물이 한개인 경우에만 적용되는 회전이동모드 등의 처리를 수행
	if( m_OSelection.GetNumObjects() == 1 )
	{
		D3DXVECTOR3 vObject = NxVectorToD3DVector( m_pSelectedObj->GetPose().t );		

		if( m_stStateFlags.bChipRotateModeReady )
		{
			D3DXVECTOR3 vAnchor = m_stInputState.v3DMousePos - vObject;
			vAnchor.y = 0;
			m_stStateFlags.bChipRotateMode = TRUE;			
			m_pSelectedObj->SetAnchor( vAnchor );
		}
		else
		{
			m_stInputState.vStartDragLocalPos = m_stInputState.v3DMousePos - vObject;
		}
	}	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 뒤집기 모드 시작 함수.

 객체의 액터의 shape에 대해 physx 머티리얼을 미리 생성해 둔 뒤집기용 고마찰력 재질로 변화시킴.

 * \note 
 씬의 기본 재질(고마찰력, 보통 등)은 CtPhysX::CreateScene()에서 등록한다.

 * \see CtPhysX::CreateScene

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::StartTurnMode( VOID )
{
	m_stStateFlags.bTurnMode = TRUE;
	if( m_stStateFlags.bDragMode && m_OSelection.GetNumObjects() == 1 )
	{
		// 마찰력을 상승시켜 뒤집기 모드 수행			
		NxShape	 *const* shape = m_pSelectedObj->GetActor()->getShapes();
		for( UINT i=0; i < m_pSelectedObj->GetActor()->getNbShapes(); i++ )
			shape[i]->setMaterial( ACTOR_MATERIAL_INDEX_HIGHFRICTION );
	}		
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 뒤집기 모드 종료 함수.

 객체들의 마찰력을 원상태로 복구시킴.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::EndTurnMode( VOID )
{
	if( m_stStateFlags.bDragMode && m_stStateFlags.bTurnMode )
	{
		// 마찰력을 원래대로 회복시킴
		NxShape	 *const* shape = m_pSelectedObj->GetActor()->getShapes();
		for( UINT i=0; i < m_pSelectedObj->GetActor()->getNbShapes(); i++ )
			shape[i]->setMaterial( ACTOR_MATERIAL_INDEX_DEFAULT );
	}		
}

VOID
CtMain::BillboardObject( VOID )
{
	D3DXMATRIX matBill;
	D3DXMatrixInverse( &matBill, 0, &matBill );
	m_pCurrentCam->GetBillboardMatrix( &matBill );	
	m_OSelection.DoAtAllObject( SBM_BILLBOARD );
}

VOID
CtMain::BillboardBrowse( VOID )
{	
	DWORD numObj = numObj = m_OSelection.GetNumObjects();

	// 그리드 계산
	Rect r;
	DWORD numObjLine = (DWORD)(sqrt( (double)numObj ) + 1);
	SystemParametersInfo( SPI_GETWORKAREA, 0, &r, 0 ); 	
	FLOAT h = CHIP_WIDTH;//r.Height * m_pCurrentCam->GetFOV() / 50.f / (FLOAT)numObjLine;

	FLOAT fFOV = m_pCurrentCam->GetFOV();
	D3DXVECTOR3 vEye = *m_pCurrentCam->GetEyePt();
	D3DXVECTOR3 vLook = *m_pCurrentCam->GetLookAtPt();
	D3DXVECTOR3 vRay = vLook - vEye;
	D3DXVECTOR3 vStart;
	D3DXVECTOR3 vRight;
	D3DXVECTOR3 vTgtPos;

	D3DXPLANE plane( 0, 0, 1, 0 );
	D3DXVECTOR3 vLookAt;
	D3DXPlaneIntersectLine( &vLookAt, &plane, &vEye, &vLook );	

	D3DXMATRIX matBill;
	m_pCurrentCam->GetBillboardMatrix( &matBill );

	// 그리드에 배열
	CtObjectBase** pObjects = m_OSelection.GetObjects();
	CtObjectBase* pObj = 0;

	vStart = m_OSelection.GetCenter() + D3DXVECTOR3( (h * numObjLine) / 2.f, 0, (h * numObjLine) / 2.f );///D3DXVECTOR3( -h * numObjLine / 2.f, vEye.y, vEye.z );
	vRight = D3DXVECTOR3( h, 0, 0 );
	D3DXVec3TransformCoord( &vStart, &vStart, &matBill );
	D3DXVec3TransformCoord( &vRight, &vRight, &matBill );

	m_OClipboard.Clear();
	NxMat34 matPose;
	matPose.M = NxMat33(NX_IDENTITY_MATRIX);
	D3DXVECTOR3 vScale, vSize;
	float scaleFactor;
	for( DWORD i=0; i < numObjLine; i++ )
	{
		for( DWORD j=0; j < numObjLine; j++ )
		{
			/*
			pObj = pObjects[i * numObjLine + j];
			vScale = pObj->GetScale();
			vSize = pObj->GetActorSize();

			vTgtPos.x = vLookAt.x + vStart.x + j * vRight.x;
			vTgtPos.y = vLookAt.y + vStart.y + j * vRight.y;
			vTgtPos.z = vLookAt.z + (numObjLine - i) * h;

			pObj->BillboardPosScale( &matBill, NxVec3(vTgtPos.x, vTgtPos.z, vTgtPos.y), h / MYMAX(vSize.x * vScale.x, vSize.y * vScale.y) * 0.5f );			
			*/
			if( i * numObjLine + j > numObj - 1 ) break;

			pObj = pObjects[i * numObjLine + j];
			vScale = pObj->GetScale();
			vSize = pObj->GetActorSize();

			scaleFactor = (h / MYMAX(vSize.x * vScale.x, vSize.y * vScale.y));
			vTgtPos.x = vStart.x - j * h;
			vTgtPos.y = vStart.z - i * h;
			vTgtPos.z = (pObj->GetActorSize().y * pObj->GetScale().y * scaleFactor) / 2.f;

			matPose.t = NxVec3( vTgtPos.x, vTgtPos.z, vTgtPos.y );
			pObj->SetPoseTransition( matPose, pObj->GetScale() * scaleFactor, 0.3f, 1 );
			//pObj->BillboardPosScale( &matBill, NxVec3(vTgtPos.x, vTgtPos.z, vTgtPos.y),  );			
			//pObj->RaiseStateFlag( OBJECT_STATE_REWINDFOWARD );

			//pObj->SetCollisionGroup( ACTOR_GROUP_BREAKINGCHIP );
			m_OClipboard.Add( pObj );
		}
	}

	m_stStateFlags.bBillboardBrowseMode = TRUE;
}

/*************************************************************************************************************************************************************/
/* 장면 드로잉 관련 함수들                                                                                                                                   */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 메인 렌더링 함수.

 * \note 함수 호출 순서
 DXUTRender3DExvironment() -> CALLBACKS::OnFrameRender() -> CtMain::OnFrameRender() -> CtMain::Render()

 * \param fElapsedTime	DXUT에서 제공하는 경과 시간. 이전 렌더링이 수행되고 나서부터 현시점까지의 경과 시간.
 * \return				없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::Render( FLOAT fElapsedTime )
{
	m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_ARGB(255,255,255,255), 1.0f, 0 );	 

	// 카메라 속성값 대로 View, Projection 설정
	CamView();
		
	//////////////////////////////////////////////////////////////////////////
	// 큐브 전환시 현재 큐브 포인터를 렌더링 도중에 바꾸는 경우
	// 문제가 생기므로 포인터를 저장해 두었다가 새롭게 렌더링할때 바꾸어 줌
	//////////////////////////////////////////////////////////////////////////
	if( m_pChangingCube != NULL )
	{
		m_pCurrentCube = m_pChangingCube;
		m_pChangingCube = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	// 드로잉 시작
	//////////////////////////////////////////////////////////////////////////
	if( SUCCEEDED(m_pd3dDevice->BeginScene()) )
	{     
		CtObjectBase **pObjects = m_pCurrentCube->GetObjects();

		//////////////////////////////////////////////////////////////////////////
		// 오브젝트 그림자 및 반사체 드로잉
		//////////////////////////////////////////////////////////////////////////						
		DrawObjectShadow( m_pCurrentCube );
		//DrawObjectReflection();		
		
		//////////////////////////////////////////////////////////////////////////
		// 월드 드로잉
		//////////////////////////////////////////////////////////////////////////
		//m_pd3dDevice->SetTexture( 0, 0 );
		//m_MeshWorld.Render( m_pd3dDevice );

		//////////////////////////////////////////////////////////////////////////
		// 오브젝트 드로잉
		//////////////////////////////////////////////////////////////////////////		
		PushMatrix();
		for( DWORD i=0; i < m_pCurrentCube->GetNumObjects(); i++ )
		{	
			if( !pObjects[i]->IsHidden() )
			{				
				SetTRS( pObjects[i], FALSE );
				DrawObject( pObjects[i] );							
			}			
		}
		PopMatrix();	

		//////////////////////////////////////////////////////////////////////////
		// 오브젝트 이름 출력
		//////////////////////////////////////////////////////////////////////////
		RenderIconName();

		//////////////////////////////////////////////////////////////////////////
		// CustomUI 렌더링
		//////////////////////////////////////////////////////////////////////////
		m_CustomUI.OnRender( fElapsedTime );

		//////////////////////////////////////////////////////////////////////////
		// 툴 및 기타 렌더링
		//////////////////////////////////////////////////////////////////////////
		if( m_stStateFlags.bLassoMode ) m_ToolLasso.Draw();

		m_pd3dDevice->EndScene();

		//if( m_pMeshSelectDialog ) m_pMeshSelectDialog->ShowWindow( SW_SHOW );
	}	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 드로잉 함수.

 CtObjectBase 로부터 파생되는 객체를 그린다. 내부적으로 객체 타입을 검사하여 타입에 맞게 그린다.

 * \note 
 
 bReflection 값에 따라 반사체를 그릴때에는 구분하여 그린다. 예를 들어 반사체를 그릴때 동일하게 정밀하게 그릴 필요는 없으므로.

 * \param *pObj			객체 포인터
 * \param bReflection	TRUE면 반사체를 그리고 FASLE 이면 일반적으로 그림.
 * \return				없음 
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::DrawObject( CtObjectBase *pObj, bool bReflection )
{
	//////////////////////////////////////////////////////////////////////////
	// 큐브의 경우 속이 비치는 투명한 메쉬이거나 롤오버될 경우 안의 내용을
	// 보여주어야 하므로 큐브에 속해 있는 오브젝트들을 먼저 모두 그린 뒤에 
	// 큐브를 그린다. 큐브 내부의 오브젝트들은 1/32으로 축소된 크기로 그려진다.
	// (큐브 기본 사이즈 1, 확대된 큐브 사이즈 32 이므로)
	//////////////////////////////////////////////////////////////////////////	
	D3DXMATRIXA16 matWorld, matScale, matTrans;
	if( bReflection == FALSE && pObj->IsCube() && (pObj->HasCustomMesh() || pObj->IsHovered()) )
	{		
		/*
		CtObjectCube *pCube = (CtObjectCube*)pObj;

		PushMatrix();

		m_pd3dDevice->GetTransform( D3DTS_WORLD, &matWorld );

		D3DXMatrixScaling( &matScale, 1/32.f, 1/32.f, 1/32.f );		
		matScale *= matWorld;

		m_pd3dDevice->SetTransform( D3DTS_WORLD, &matScale );

		// 큐브 내부의 객체들을 먼저 그린다		
		CtObjectBase** pObj = pCube->GetObjects();
		for( int i=0; i < pCube->GetNumObjects(); i++ )
		{			
			PushMatrix();
			SetTRS( pObj[i], TRUE );			
			DrawObject( pObj[i] );			
			PopMatrix();
		}

		PopMatrix();
		*/
	}		

	D3DMATERIAL9 pMat;

	if( pObj->HasMorphMesh() )
	{	
		/*
		ZObject* pZObj = pObj->m_pMorphObj;
		ZMesh** pMeshes = 0;
		for( DWORD i=0; i < pZObj->m_dwNumSubset; i++ )
		{
			pMeshes = pZObj->m_pMeshes.GetData();
			for( DWORD j=0; j < pZObj->m_pMeshes.GetSize(); j++ )
			{
				m_pd3dDevice->SetStreamSource( 0, pObj->m_pMeshVB, 0, sizeof(MESHVERTEX) );
				m_pd3dDevice->SetFVF( D3DFVF_MESHVERTEX );
				m_pd3dDevice->SetIndices( pObj->m_pMeshIB );

				m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
				m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB((int)((1-m_Morpher.m_fAlpha) * 255),(int)((1-m_Morpher.m_fAlpha) * 255),(int)((1-m_Morpher.m_fAlpha) * 255),(int)((1-m_Morpher.m_fAlpha) * 255)) ); 

				m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
				m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
				m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
				m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
				m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
				m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

				m_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
				m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_LERP );
				m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG0, D3DTA_TFACTOR );
				m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
				m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
				m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
				m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

				m_pd3dDevice->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 0 );
				m_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_MODULATE );
				m_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
				m_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
				m_pd3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
				m_pd3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
				m_pd3dDevice->SetMaterial( &pMeshes[j]->m_pMaterials[0] );
				m_pd3dDevice->SetTexture( 0, m_pMorphingSrcObj->m_pDXUTMesh->m_pTextures[0] );
				m_pd3dDevice->SetTexture( 1, m_pMorphingTgtObj->m_pDXUTMesh->m_pTextures[0] );		
			
				m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, pMeshes[j]->m_dwVertStart, pMeshes[j]->m_dwNumFaces * 3, pMeshes[j]->m_dwFaceStart, pMeshes[j]->m_dwNumFaces );
				m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
				m_pd3dDevice->SetTexture( 0, NULL );		
				m_pd3dDevice->SetTexture( 1, NULL );		
				m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DISABLE );
				m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
				m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
				m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
				m_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
				m_pd3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
			}
			
		}	
		*/	
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// 머티리얼의 갯수는 메쉬의 sub-object 갯수와 같다. 
		// 여러 sub-object를 갖는 메쉬는 그 갯수만큼 DrawObject 함수를 호출해야 메쉬가 
		// 완전히 그려진다. 따라서 머티리얼 수 만큼 for문 내에서 드로잉 한다.
		//////////////////////////////////////////////////////////////////////////
		LPDIRECT3DBASETEXTURE9* pTextures = (LPDIRECT3DBASETEXTURE9*)pObj->GetTextures();
		CDXUTMesh* pDXUTMesh	= pObj->GetDXUTMesh();
		FLOAT fTransparency		= pObj->GetTransparency(); 
		BOOL bHasCustomMesh		= pObj->HasCustomMesh();
		CtObjectType type		= pObj->GetType();

		m_RenderManager.GetStateBlock()->Capture();

		for( UINT i=0; i < pDXUTMesh->m_dwNumMaterials; i++ )
		{	
			if( pObj->IsCube() && !bHasCustomMesh ) 
				pMat = ((CtObjectCube*)pObj)->GetMaterial();
			else 
				pMat = pDXUTMesh->m_pMaterials[i];

			//////////////////////////////////////////////////////////////////////////
			// 객체 롤오버시 행동. 일반 오브젝트는 밝게, 큐브는 투명하게 처리
			//////////////////////////////////////////////////////////////////////////
			if( pObj == m_pHoveredObj || pObj->IsSelected() )		
			{				
				pMat.Ambient.r += 0.05f;
				pMat.Ambient.g += 0.05f;
				pMat.Ambient.b += 0.05f;
			}
			else pObj->UnHover();

			// 객체의 투명도 속성에 따라 머티리얼 알파값 조절
			if( fTransparency > 0 ) pMat.Diffuse.a = 1 - fTransparency;

			//////////////////////////////////////////////////////////////////////////
			// 기본 메쉬를 갖는 칩에 대한 텍스쳐 설정 루틴.
			// 하나의 메쉬에 부위별로 여러 동적 텍스쳐를 입히기 위해 기본 칩 메쉬는 
			// 5개의 sub object로 이루어져 있다.
			//////////////////////////////////////////////////////////////////////////
			if( bHasCustomMesh )
			{
				m_pd3dDevice->SetTexture( 0, pDXUTMesh->m_pTextures[i] );						
				m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );                								
				m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );		
				m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
				m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );                			
				m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
				m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );	

				m_pd3dDevice->SetMaterial( &pMat );
				pDXUTMesh->m_pMesh->DrawSubset( i );		
			}
			else
			{
				switch( type )
				{
				case OBJECT_TYPE_CHIP:
					{
						switch( i )
						{
						case 0:	// 몸체 box
							m_pd3dDevice->SetTexture( 0, pDXUTMesh->m_pTextures[i] );						
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );                								
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );		
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );                			
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );				
							break;
						case 1:	// 윗면 아이콘 표시를 위한 사각형
						case 2:	// 아랫면 아이콘 표시를 위한 사각형
							m_pd3dDevice->SetTexture( 0, pTextures[0] );	
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );                							
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,	 D3DTOP_MODULATE );				
							pMat.Ambient.r = pMat.Ambient.g = pMat.Ambient.b = 0.5f;						
							pMat.Diffuse.r = pMat.Diffuse.g = pMat.Diffuse.b = 1.f;						
							break;
						case 3:	// 옆면 아이콘 표시를 위한 사각형											
							m_pd3dDevice->SetTexture( 0, pTextures[0] );									
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );                							
							break;
						case 4:	// 옆면 이름 표시를 위한 사각형				
							m_pd3dDevice->SetTexture( 0, pTextures[1] );									
							break;
						}

						m_pd3dDevice->SetMaterial( &pMat );
						pDXUTMesh->m_pMesh->DrawSubset( i );		

						break;
					}
				case OBJECT_TYPE_CUBE:
					{
						if( i < 6 )
						{
							m_pd3dDevice->SetTexture( 0, pTextures[1] );
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );		
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );		
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SUBTRACT );                			
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );		
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );		
							m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );    
						}
						else m_pd3dDevice->SetTexture( 0, 0 );	

						m_pd3dDevice->SetMaterial( &pMat );
						pDXUTMesh->m_pMesh->DrawSubset( i );		

						break;
					}
				case OBJECT_TYPE_IMAGE:
					{
						m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
						m_pd3dDevice->SetTexture( 0, pTextures[0] );						
						m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );                								
						m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
						m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );                			
						m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

						m_RenderManager.ToggleSpecular();
						pDXUTMesh->m_pMesh->DrawSubset( i );
						m_RenderManager.ToggleSpecular();

						/*
						pMat.Diffuse.r = pMat.Diffuse.g = pMat.Diffuse.b = 0.8f;
						m_pd3dDevice->SetMaterial( &pMat );
						m_pd3dDevice->SetTexture( 0, 0 );	

						D3DXVECTOR3 vSize, vScale;
						vScale = pObj->GetScale();
						vSize = pObj->GetActorSize();
						D3DXMatrixScaling( &matScale, 1 + (0.01f / (vSize.x * vScale.x)), 1 + (0.01f / (vSize.y * vScale.y)), 0.95 );

						PushMatrix();
						m_pd3dDevice->GetTransform( D3DTS_WORLD, &matWorld );
						matScale *= matWorld;
						m_pd3dDevice->SetTransform( D3DTS_WORLD, &matScale );

						pDXUTMesh->m_pMesh->DrawSubset( i );
						PopMatrix();
						*/

						m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
						break;
					}
				}		
			}						
		}

		m_RenderManager.GetStateBlock()->Apply();
	}	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체들의 그림자를 그리는 함수.

 모든 객체들의 기본 ground plane 에 대한 그림자를 한꺼번에 그린다. 
 객체가 많은 경우 그림자를 그리는 것이 퍼포먼스에 영향을 주므로 최대한 단순하게 그린다. 
 여기서는 ShadowMap을 사용하지 않고 스텐실을 사용해 그림자를 표현한다.

 * \param pCube	그림자를 그릴 객체들을 가진 부모 큐브 객체 포인터
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::DrawObjectShadow( CtObjectCube* pCube )
{	
	D3DXPLANE		plane( 0, 1, 0, 0.f );			// 기본 ground 평면
	D3DXMATRIX		matShadow, matScale, matWorld;	// 그림자, 스케일, 월드 행렬
	D3DXMATRIXA16	glMat;							// 피직스 행렬을 얻어서 담을 4X4 행렬
	D3DXVECTOR3		vPosition, vLdir, vScale;		// 객체 포지션 벡터, 조명 방향 벡터, 객체 스케일 벡터
	CtObjectBase**	pObjects;	
	CtObjectBase*	pObj;
	CDXUTMesh*		pDXUTMesh;
	DWORD			dwNumObj;
		
	m_RenderManager.GetStateBlock()->Capture();

	m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE,		FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,		TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,		D3DCMP_EQUAL );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILREF,			0x0 );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILMASK,		0xffffffff );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILWRITEMASK,	0xffffffff );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL,		D3DSTENCILOP_KEEP );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,		D3DSTENCILOP_KEEP );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,		D3DSTENCILOP_INCR );
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,			FALSE );

	m_pd3dDevice->GetTransform( D3DTS_WORLD, &matWorld );

	PushMatrix();
	
	// 기본 그림자 머티리얼(검은색)을 지정
	m_pd3dDevice->SetMaterial( &m_RenderManager.GetShadowMaterial() );

	// 큐브 내의 모든 객체의 그림자를 그린다
	pObjects = pCube->GetObjects();
	dwNumObj = pCube->GetNumObjects();

	for( DWORD i=0; i < dwNumObj; i++ )
	{
		pObj = pObjects[i];

		if( pObj->IsHidden() ) continue;		

		pObj->GetPose().getColumnMajor44(glMat);
		vPosition = D3DXVECTOR3( glMat._41, glMat._42, glMat._43 );
		vScale = pObj->GetScale();
		vLdir = m_RenderManager.GetLightPosition( 0 ) - vPosition;	 	
		D3DXVec3Normalize( &vLdir, &vLdir );

		D3DXMatrixShadow( &matShadow, &D3DXVECTOR4( vLdir.x, vLdir.y, vLdir.z, 0 ), &plane );
		D3DXMatrixScaling( &matScale, vScale.x, vScale.y, vScale.z );
		
		matScale *= glMat * matShadow;

		m_pd3dDevice->SetTransform( D3DTS_WORLD, &matScale );
		
		m_pd3dDevice->SetTexture( 0, 0 );

		pDXUTMesh = pObj->GetDXUTMesh();
		for( DWORD i=0; i < pDXUTMesh->m_dwNumMaterials; i++ )
			pDXUTMesh->m_pMesh->DrawSubset( i );
	}

	PopMatrix();	

	m_RenderManager.GetStateBlock()->Apply();	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체들의 반사체를 모두 그려주는 함수

 * \note 
 여기서는 객체들의 반사 행렬을 구해 행렬변환 후 그리는 방법을 사용한다. 트릭을 써서 기본 월드 배경 메시(알파가 들어간 평면 메시)를 통해 흐릿한 반사체를
 그리고 있으며 객체들을 그대로 그리므로 성능이 좋지 못하다. 셰이더를 사용한 반사체 드로잉 방법이 좋을 것 같음

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::DrawObjectReflection( VOID )
{
	// 월드 행렬
	D3DXMATRIX W, R;

	// 기본 반사 행렬을 얻는다
	R = m_RenderManager.GetReflectMatrix();

	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

	CtObjectBase** pObjects = m_pCurrentCube->GetObjects();
	DWORD dwNumObj = m_pCurrentCube->GetNumObjects();

	PushMatrix();
	for( int i=0; i < dwNumObj; i++ )
	{	
		if( pObjects[i]->IsHidden() || pObjects[i]->HasMorphMesh() ) continue;
		
		W = SetTRS( pObjects[i], FALSE );
		W *= R;
		m_pd3dDevice->SetTransform( D3DTS_WORLD, &W );

		DrawObject( pObjects[i], TRUE );		
	}
	PopMatrix();

	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체의 Display name을 렌더링하는 함수

 객체의 Display name은 파일명과는 다를 수 있다. 윈도우 바탕화면에 표시되는 아이콘 이름임.
 CustomUI에 미리 생성한 CDXUTTextHelper 클래스를 사용한다. 객체의 포지션을 프로젝션한 스크린 좌표 살짝 아래에 이름을 출력함. 

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::RenderIconName( VOID )
{	
	NxVec3				vPos;								// 객체 포지션
	D3DXVECTOR3			vProjected;							// vPos를 프로젝션한 벡터
	UINT				screenX, screenY;					// 최종 스크린 좌표

	CDXUTTextHelper*	pTextHelper = m_CustomUI.GetTextHelper();

	//////////////////////////////////////////////////////////////////////////
	// Begin(), End() 사이에서 행렬을 얻으면 안되므로 미리 구해놓는다.
	D3DXVECTOR3 vScreen;
	D3DVIEWPORT9 vp;	
	D3DXMATRIX matProjection, matView, matWorld;

	LPDIRECT3DDEVICE9 pDev = DXUTGetD3DDevice();
	pDev->GetViewport( &vp );
	pDev->GetTransform( D3DTS_PROJECTION, &matProjection );
	pDev->GetTransform( D3DTS_VIEW, &matView );
	pDev->GetTransform( D3DTS_WORLD, &matWorld );
	//////////////////////////////////////////////////////////////////////////

	pTextHelper->Begin();
	
	// 선택된 모든 객체의 이름을 출력
	CtObjectBase** pObjects = m_OSelection.GetObjects();
	for( DWORD i=0; i < m_OSelection.GetNumObjects(); i++ )
	{
		vPos = pObjects[i]->GetPose().t;
		D3DXVec3Project( &vProjected, &D3DXVECTOR3(vPos.x, vPos.y, vPos.z), &vp, &matProjection, &matView, &matWorld );
		screenX = (int)vProjected.x;
		screenY = (int)vProjected.y;

		RenderText( pTextHelper, screenX-49, screenY+17, screenX+51, screenY+101, D3DXCOLOR(0,0,0,0.5f), DT_NOCLIP|DT_CENTER|DT_WORDBREAK, FALSE, pObjects[i]->GetFileInfo()->lpszDisplayName );
		RenderText( pTextHelper, screenX-50, screenY+16, screenX+50, screenY+100, D3DXCOLOR(1,1,1,1), DT_NOCLIP|DT_CENTER|DT_WORDBREAK, FALSE, pObjects[i]->GetFileInfo()->lpszDisplayName );
	}

	// 롤오버된 객체의 이름을 출력
	if( m_pHoveredObj )
	{
		vPos = m_pHoveredObj->GetPose().t;
		D3DXVec3Project( &vProjected, &D3DXVECTOR3(vPos.x, vPos.y, vPos.z), &vp, &matProjection, &matView, &matWorld );
		screenX = (int)vProjected.x;
		screenY = (int)vProjected.y;

		RenderText( pTextHelper, screenX-49, screenY+17, screenX+51, screenY+101, D3DXCOLOR(0,0,0,0.5f), DT_NOCLIP|DT_CENTER|DT_WORDBREAK, FALSE, m_pHoveredObj->GetFileInfo()->lpszDisplayName );
		RenderText( pTextHelper, screenX-50, screenY+16, screenX+50, screenY+100, D3DXCOLOR(1,1,1,1), DT_NOCLIP|DT_CENTER|DT_WORDBREAK, FALSE, m_pHoveredObj->GetFileInfo()->lpszDisplayName );
	}       

	pTextHelper->End();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 텍스트 렌더링 함수

 지정한 포맷에 맞게 텍스트를 출력하는 함수이다. 

 * \warning 

 bBeginEnd가 FALSE인 경우 이 함수를 호출하기 전 반드시 pTxtHelper의 Begin() 이 호출되어 있어야 함.
 이런 구조의 이유는 Begin, End 호출을 줄이기 위함.

 * \param pTxtHelper	DXUT 텍스트 헬퍼 포인터
 * \param left			출력할 위치
 * \param top			출력할 위치
 * \param right			출력할 위치
 * \param bottom		출력할 위치
 * \param color			텍스트 색상
 * \param dwFlags		출력 flag. DrawFormattedTextLine 의 msdn 참조
 * \param bBeginEnd		Begin(), End()를 호출할지 여부
 * \param strMsg		출력할 문자열
 * \param ...			출력할 포맷
 * \return				없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID 
CtMain::RenderText( CDXUTTextHelper* pTxtHelper, int left, int top, int right, int bottom, D3DXCOLOR& color, DWORD dwFlags, BOOL bBeginEnd, const WCHAR* strMsg, ... )
{
	va_list args;
	va_start( args, strMsg );

	pTxtHelper->SetInsertionPos( 5, 5 );	
	pTxtHelper->SetForegroundColor( color );

	RECT rc;		
	SetRect( &rc, left, top, right, bottom );	

	if ( strMsg ) {
		if (bBeginEnd) pTxtHelper->Begin();
		pTxtHelper->DrawFormattedTextLine(rc, dwFlags, strMsg, args);
		if (bBeginEnd) pTxtHelper->End();
	}

	va_end( args );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체의 액터를 업데이트하는 함수

 이 함수에서 다음 프레임에서 그려질 객체의 포즈를 결정한다. 
 키프레임 애니메이션이 셋트되어 있는 경우 보간된 포즈 값을 얻고, 그렇지 않으면 시뮬레이션된 결과를 얻는다.

 이 함수에서는 특히 키프레임 애니메이션이 종료되었을 때의 후처를 수행한다. 애니메이션이 끝난 객첻들은 대부분 무언가 추가로 
 작업해 줄 것들이 있는데 이곳에서 처리해 준다.

 * \note
 객체가 갖는 액터의 실제 포즈값은 언제나 getGlobalPose()로 얻어야 한다. 그러나 키프레임 애니메이션도 필요하므로 
 객체를 실제 그릴때 참조하는 포즈는 CtObjectBase::m_matPose 변수이다. 이 변수에 시뮬레이션 된 값 또는 키프레임 보간값을
 넣어서 이 변수 값으로 행렬변환하여 그리게 된다. 

 또 키프레임 보간을 해 나갈때 일일이 setGlobalPose()를 수행하여 피직스 엔진과 애니메이션되고 있는 객체의 포즈를 일치시킬수도 있으나
 성능상 그렇게 하지 않고 애니메이션이 종료되는 마지막에만 한번 수행해 준다.
 
 * \see 
 SetTRS() 참조.

 * \param pObj	객체 포인터
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::UpdateActor( CtObjectBase* pObj )
{
	NxActor* pActor = pObj->GetActor();

	/************************************************************************/
	/* 키프레임 애니메이션 관련                                             */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	// 키프레임이 있을 경우 애니메이션 데이터로부터 포즈값을 얻음
	// 키프레임이 없으면 PhysX 시뮬레이터로부터 포즈를 얻음
	//////////////////////////////////////////////////////////////////////////
	CtAnimInfo<CtObjectKeyData>* pAnimInfo = pObj->GetAnimInfo();
	if( pAnimInfo->IsEnabled() )
	{
		m_stStateFlags.bIsIdleNow = FALSE;
		
		// FrameMove 함수는 애니메이션 프레임이 끝에 다다르면 TRUE를 반환한다
		if( pAnimInfo->FrameMove() == TRUE )
		{			
			if( pObj->GetPoseTemp().isIdentity() ) pObj->SetPoseTemp( pObj->GetPose() );
			pActor->setGlobalPose( pObj->GetPose() );
			pActor->putToSleep();

			//////////////////////////////////////////////////////////////////////////
			// 큐브 전환 애니메이션 후처리
			//////////////////////////////////////////////////////////////////////////
			if( m_stStateFlags.bCubeChanging )
			{				
				CtObjectCube *pCube = (CtObjectCube*)pObj;
				if( pCube->IsMaximized() )
				{					
					// 최소화 된 후
					pCube->SetMaximized( FALSE );					
					pCube->SetTransparency( 0 );
				}
				else
				{
					// 최대화 된 후
					pCube->SetMaximized( TRUE );
					m_pChangingCube = pCube;					
				}				
				m_stStateFlags.bCubeChanging = FALSE;
			}
			//////////////////////////////////////////////////////////////////////////
			// 빌보딩 후처리
			//////////////////////////////////////////////////////////////////////////
			if( m_stStateFlags.bBillboardBrowseMode )
			{
				pObj->Scale();								
				pObj->Freeze();
			}
			//////////////////////////////////////////////////////////////////////////
			// 파일링 후처리
			//////////////////////////////////////////////////////////////////////////
			if( m_stStateFlags.bPilingModeDone )
			{
				CtObjectChip* pChip = (CtObjectChip*)pObj;
				CtObjectChip* pPrev = pChip->GetPrev();
	
				NxActor *pActor1, *pActor2;
				NxJoint *pJoint;

				UINT iPilingNow = m_OSelection.GetPilingNow();

				pChip->SetState( OBJECT_STATE_PILED );
				pChip->Freeze();

				if( pPrev )
				{		
					pActor1 = pPrev->GetActor();
					pActor2 = pChip->GetActor();

					// 앞위 액터 사이에 고정 조인트를 생성하여 연결한다
					pJoint = m_PhysX.CreateFixedJoint( m_pCurrentCube->GetScene(), pActor1, pActor2 );
					pChip->SetPileJointPrev( pJoint );
					pPrev->SetPileJointNext( pJoint );					
				}
				if( !pChip->GetNext() ) 
				{					
					while( pChip )
					{
						pChip->UnFreeze();						
						pChip = pChip->GetPrev();
					}
					m_OSelection.SetPilingNow( --iPilingNow );
				}				
				
				if( iPilingNow == 0 ) 
					m_stStateFlags.bPilingModeDone = FALSE;
			}
			//////////////////////////////////////////////////////////////////////////
			// 파일링 추가 후처리
			//////////////////////////////////////////////////////////////////////////
			if( m_stStateFlags.bPilingAddModeDone )
			{				
				m_stStateFlags.bPilingAddModeDone = FALSE;
		
				CtObjectChip* pChip = (CtObjectChip*)pObj;
				CtObjectChip* pPrev = pChip->GetPrev();
				NxActor* pActor1 = pPrev->GetActor();
				NxActor* pActor2 = pChip->GetActor();

				pActor2->setMass( 1.f / (pChip->GetPileOrder()) );
				pPrev->Freeze();

				NxMat34 matPose = pActor1->getGlobalPose();
				NxVec3 vUp = matPose.M.getColumn( 1 );
				vUp.normalize();
				D3DXVECTOR3 vScale1 = pPrev->GetScale();
				D3DXVECTOR3 vScale2 = pChip->GetScale();

				vUp *= ( (CHIP_THICKNESS * vScale1.z) / 2.f ) + ( (CHIP_THICKNESS * vScale2.z) / 2.f);
				matPose.t += vUp;

				pActor2->setLinearVelocity( NxVec3(0,0,0) );
				pActor2->putToSleep();
				pActor2->setGlobalPose( matPose );
				pChip->Freeze();

				NxJoint* pJoint = m_PhysX.CreateFixedJoint( m_pCurrentCube->GetScene(), pActor1, pActor2 );
				pPrev->SetPileJointNext( pJoint );
				pChip->SetPileJointPrev( pJoint );

				pActor1->putToSleep();				
				pActor2->putToSleep();								
				pChip->UnFreeze();
				pPrev->UnFreeze();

				pActor2->clearActorFlag( NX_AF_DISABLE_COLLISION );
				pChip->SetCollisionGroup( ACTOR_GROUP_DEFAULT );				

				pActor2->setContactReportFlags( NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD | NX_NOTIFY_FORCES );
				pActor2->setContactReportThreshold( CONTACT_FORCE_THRESHOLD_ADDCHIP_TO_PILE );			
			}			

			pActor->clearActorFlag( NX_AF_DISABLE_COLLISION );			
		}		
		// 키프레임 애니메이션 중인 경우 보간값을 얻어 객체의 변수에 넣는다.
		else
		{
			CtObjectKeyData keyData = pAnimInfo->GetData();		
			pObj->SetPose( keyData.matPose );
			pObj->SetScale( keyData.vScale );
		}
	}
	else
	{		
		// 키프레임이 없는 경우 시뮬레이션된 포즈를 얻음
		pObj->SetPose( pActor->getGlobalPose() );
	}

	// 객체가 sleep 중이지 않으면 idle 변수를 false로 지정한다.
	// 모든 객체가 sleep 중이면 idle 값이 true가 되어 렌더링하지 않도록 하게 된다.
	if( !pActor->isSleeping() )	m_stStateFlags.bIsIdleNow = FALSE;

	/************************************************************************/
	/* 객체 포즈와 관련된 드래그 등 툴에 대한 처리                          */
	/************************************************************************/

	///////////////////////////////////////////////////////////////////////////
	// 드래그 모드인 경우 마우스 위치를 향하도록 객체의 속도를 설정
	// 마우스 오른쪽 버튼을 눌러 리프트 모드 추가시 객체의 z포지션값을 증가시킴
	// 이때 마우스가 가리키는 3D좌표에 대해 z값이 + 5된 위치에 떠있게 함
	///////////////////////////////////////////////////////////////////////////
	if( m_stStateFlags.bDragMode && !m_stInputState.bShiftDown && pObj->IsSelected() ) 
	{		
		pObj->RaiseStateFlag( OBJECT_STATE_FORCED );

		D3DXVECTOR3 vPosition = NxVectorToD3DVector( pObj->GetPose().t );
		D3DXVECTOR3 vMouse = m_stInputState.v3DMousePos;
		D3DXVECTOR3 vStart;

		//////////////////////////////////////////////////////////////////////////
		// 드래그 중간에 오브젝트가 회전되었을 수 있으므로
		// 앵커 벡터의 방향을 마우스가 집고 있는 방향으로 일치시킨다.
		//////////////////////////////////////////////////////////////////////////
		//pObj->m_vAnchor.sameDirection( m_stInputState.vOld3DMousePos - vPosition );

		if( m_stStateFlags.bChipRotateMode ) 
			vStart = pObj->GetAnchor();
		else 
			vStart = m_stInputState.vStartDragLocalPos;

		//////////////////////////////////////////////////////////////////////////
		// 리프트 모드시 마우스 3D좌표(마우스가 가리키는 물체 표면의 3D좌표)의 Z값을
		// 일정 값만큼 높게 설정하여 마우스가 가리키는 위치보다 위에 오브젝트가 
		// 떠있게끔 한다.
		//////////////////////////////////////////////////////////////////////////
		if( pObj->IsLifted() )
		{
			vMouse.y = m_stInputState.v3DMousePos.y + 5;									
			vStart = D3DXVECTOR3(0,0,0);
		}		
		else
		{
			//////////////////////////////////////////////////////////////////////////
			// 리프트 모드가 아닌 일반 드래그 모드에서는 기본적으로 XY방향만을 이동시키며 
			// Z방향 이동은 Lift로 처리한다. 따라서 마우스의 3D좌표의 Z값을 오브젝트 포지션의
			// Z값으로 지정한다.
			// (CheckHover 함수에서 마우스의 3D좌표를 결정할 때 오브젝트 표면과 충돌처리한다.
			// 현재 오브젝트의 3D 포지션에서 마우스의 3D좌표로 향하도록 하는데 있어서
			// 그 실제 3D좌표로 오브젝트를 향하게 하면 Z방향의 이동이 일어나므로 이것을
			// 방지하기 위함)
			//////////////////////////////////////////////////////////////////////////
			vMouse.y = vPosition.y;			
		}	

		D3DXVECTOR3 vVel, vAnchor;		
		if( m_stStateFlags.bChipRotateMode )
		{	
			vVel = vMouse - vPosition;
			vAnchor = pObj->GetAnchor();
			if( D3DXVec3Length(&vAnchor) < D3DXVec3Length(&vVel) )
			{		
				vVel -= vAnchor;
				vVel.y = 0;				
			}						
		}
		else
		{
			// 결정된 마우스 좌표로 향하도록 오브젝트의 속도를 지정
			vVel = vMouse - vPosition - vStart;			
			vVel *= 4.0f;
		}

		pObj->GetActor()->setLinearVelocity( D3DXVec3ToNxVec3(vVel) );		
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 프로그램 기본 버텍스, 인덱스 버퍼를 생성

 기본 VB 및 IB는 다용도로 사용한다. 설정된 크기 만큼 생성하여 수동으로 렌더링할 primitive가 있을 경우 사용한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT
CtMain::InitBuffers( VOID )
{	
	HRESULT hr;

	V_RETURN( m_pd3dDevice->CreateVertexBuffer( sizeof(VERTEX) * MAIN_VERTEXBUFFER_SIZE,
		D3DUSAGE_WRITEONLY, 
		D3DFVF_VERTEX,
		D3DPOOL_MANAGED, 
		&m_pMainVB, 
		NULL ) );

	V_RETURN( m_pd3dDevice->CreateIndexBuffer( sizeof(WORD) * MAIN_INDEXBUFFER_SIZE, 
		D3DUSAGE_WRITEONLY, 
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&m_pMainIB,
		NULL ) );
	
	return ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 프로그램 초기 디렉토리에 대한 큐브(메인 큐브)를 생성

 * \note 기본 디렉토리 경로는 차후 INI 파일 셋팅값에서 불러오는것으로 대체

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID 
CtMain::MakeMainCube( VOID )
{
	HRESULT hr;

	LPITEMIDLIST	DesktopPidl; 
	TCHAR			DesktopDir[MAX_PATH], strDrv[4], strDir[MAX_PATH], strFName[MAX_PATH], strExt[MAX_PATH];
	CString			strFullPath, strPath;

	LPCTSTR pCmd = GetCommandLine();
	if( __argc > 1 ) 
	{
		// 프로그램 실행시 커맨드 라인에 경로명을 넣어주면 그 디렉토리를 기본 디렉토리로 한다.
		strFullPath = pCmd;		
	}
	else
	{
		// 인자가 없으면 바탕화면이 기본 디렉토리임.
		V( SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &DesktopPidl) );
		if( !SHGetPathFromIDList( DesktopPidl, DesktopDir ) ) 
		{ 
			MessageBox( NULL, L"SHGetPathFromIDList() 실패", 0, 0 );     
			exit(0);
		} 
		strFullPath = DesktopDir;		
	}

	// 메인 큐브 생성
	_wsplitpath( strFullPath, strDrv, strDir, strFName, strExt );
	strPath = CString( strDrv ) + CString( strDir );
	m_pCurrentCube = new CtObjectCube( strFName, strPath, strFName, L"파일 폴더" );
	m_pMainCube = m_pCurrentCube;		
}

/*************************************************************************************************************************************************************/
/* 카메라 관련 함수들																																		 */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 카메라 행렬 변환 함수

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::CamView( VOID )
{	
	// 모델뷰 카메라만 월드변환 수행
	//if( m_pCurrentCam->GetType() == DXUT_CAM_TYPE_MV )
		//m_pd3dDevice->SetTransform( D3DTS_WORLD, m_pCurrentCam->GetWorldMatrix() );	

	m_pd3dDevice->SetTransform( D3DTS_VIEW, m_pCurrentCam->GetViewMatrix() );
	m_pd3dDevice->SetTransform( D3DTS_PROJECTION, m_pCurrentCam->GetProjMatrix() );	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 현재 카메라를 지정하는 함수

 * \param pCam	카메라 객체 포인터
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::CamSet( CBaseCamera* pCam )
{
	m_pCurrentCam = pCam;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 카메라 초기화 함수

 * \param bInit	초기화 수행 여부. TRUE인 경우 월드 0,0,0 좌표를 바라보도록 한다.
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID 
CtMain::CamReset( BOOL bInit /* = FALSE */ )
{
	CRect rect;
	GetClientRect( AfxGetMainWnd()->m_hWnd, &rect );
	
	//////////////////////////////////////////////////////////////////////////
	// 일반 및 와이드 모니터 등 종횡비가 다른 해상도를 모두 지원하기 위해
	// FOV 값을 적절히 계산하는 공식
	//////////////////////////////////////////////////////////////////////////
	FLOAT fAspectRatio	= (FLOAT)rect.right / (FLOAT)rect.bottom;
	FLOAT fFOV			= DEFAULT_FOV - (fAspectRatio - 1.333f) * 3 / 10.f; 

	D3DXVECTOR3 vEye, vLookAt;
	if( bInit )
	{
		// 카메라 시점을 기본값으로 초기화
		vEye = D3DXVECTOR3( 0, 30, 25 );
		vLookAt = D3DXVECTOR3( 0, 0, 0 );
		m_pCurrentCam->SetViewParams( &vEye, &vLookAt );
		m_pCurrentCam->SetProjParams( fFOV, fAspectRatio, m_pCurrentCam->GetNearClip(), m_pCurrentCam->GetFarClip() );
	}
	else
	{
		// FOV 값을 되돌리는 애니메이션
		if( typeid(*m_pCurrentCam) == typeid(CtFirstPersonCamera) )
			((CtFirstPersonCamera*)m_pCurrentCam)->SetPoseTransition( 0.4f, 0.3f, CTKA_CAM_EYE | CTKA_CAM_LOOKAT | CTKA_CAM_FOV, CTKA_CAM_EYE, m_pCurrentCam->GetDefaultEye(), CTKA_CAM_LOOKAT, m_pCurrentCam->GetDefaultLookAt(), CTKA_CAM_FOV, fFOV );
	}	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 카메라 줌인/아웃 함수.

 마우스 휠을 사용하여 카메라 줌 기능을 수행한다.

 * \param amount	윈도우 메시지로부터 넘어온 휠 스크롤 값. 보통 +- 120을 가짐.
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID 
CtMain::CamZoom( short amount )
{
	FLOAT fFOV = m_pCurrentCam->GetFOV();
	FLOAT slice = ( (FLOAT)amount / 2400.f / 5 * fFOV );

	if( m_stInputState.bAltDown ) slice /= 10.f;

	// 수동으로 스무스하게 줌인/아웃 한다. 키프레임 애니메이션까지 쓸필요를 못느낌..
	for( int i=0; i < 5; i++ )
	{
		fFOV += slice * i;
		if( fFOV < MAX_FOV ) fFOV = MAX_FOV;
		if( fFOV > PI ) fFOV = PI;

		m_pCurrentCam->SetProjParams( fFOV, m_pCurrentCam->GetAspect(), m_pCurrentCam->GetNearClip(), m_pCurrentCam->GetFarClip() );
		DXUTRender3DEnvironment();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 자동 포커싱 함수.
 
 객체 크기에 맞게 카메라 시점 및 줌을 설정한다. 애니메이션 사용.

 * \param *ptr	객체 포인터
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::CamZoomToObject( CtObjectBase *ptr )
{
	D3DXVECTOR3 vSize, vScale;
	vSize = ptr->GetActorSize();
	vScale = ptr->GetScale();

	// 객체 액터의 실제 사이즈를 통해 최대값 계산. 이것을 토대로 FOV값 결정
	FLOAT fmax = MYMAX( vSize.x * vScale.x, MYMAX(vSize.y * vScale.y, vSize.z * vScale.z) );

	NxVec3 vPosition = ptr->GetPose().t;

	D3DXVECTOR3 vEye, vLookat;
	vEye = *m_pCurrentCam->GetEyePt();
	vLookat = D3DXVECTOR3( vPosition.x, vPosition.y, vPosition.z );

	D3DXVECTOR3 vtemp = vEye - vLookat;
	FLOAT fFOV = fmax / ( D3DXVec3Length(&vtemp) / 3 );
	if( fFOV < MAX_FOV ) fFOV = MAX_FOV;
	if( fFOV > DEFAULT_FOV ) fFOV = DEFAULT_FOV;

	if( m_pCurrentCam->GetType() == DXUT_CAM_TYPE_FP )
		((CtFirstPersonCamera*)m_pCurrentCam)->SetPoseTransition( 0.4f, 0.8f, CTKA_CAM_LOOKAT | CTKA_CAM_FOV, CTKA_CAM_LOOKAT, vLookat, CTKA_CAM_FOV, fFOV );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 카메라 패닝 함수.

 기본적으로 프로그램 공간이 Y축이 up vector이다. XZ방향으로 panning을 수행하는 함수.

 * \param iMoveX	마우스 X좌표 이동량
 * \param iMoveY	마우스 Y좌표 이동량
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID	
CtMain::CamPan( int iMoveX, int iMoveY )
{
	D3DXVECTOR3 vEye, vLookat;
	vEye = *m_pCurrentCam->GetEyePt();
	vLookat = *m_pCurrentCam->GetLookAtPt();

	vLookat.x += (float)iMoveX/20.f * m_pCurrentCam->GetFOV();
	vLookat.z += (float)iMoveY/20.f * m_pCurrentCam->GetFOV();

	// 해상도 및 현재 FOV 값에 따라 이동량을 조절한다. 줌이 많이 된 상태에서 조금만 마우스를 움직여도 많이 이동하면 안좋으므로..
	if( vLookat.x < -DESK_WIDTH/2 ) vLookat.x = -DESK_WIDTH/2;
	if( vLookat.x >  DESK_WIDTH/2 ) vLookat.x =  DESK_WIDTH/2;
	if( vLookat.z < -DESK_HEIGHT/2 ) vLookat.z = -DESK_HEIGHT/2;
	if( vLookat.z >  DESK_HEIGHT/2 ) vLookat.z =  DESK_HEIGHT/2;

	vEye.x = vLookat.x;
	vEye.z = vLookat.z - 18.75f * m_pCurrentCam->GetAspect();

	m_pCurrentCam->SetViewParams( &vEye, &vLookat );	
	return;
}

/*************************************************************************************************************************************************************/
/* 라이팅 관련 함수들																																		 */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 기본 조명 셋팅 함수

 현재 조명들은 고정되어 있으며 밝은 느낌을 위해 3개를 사용한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::SetupLights( VOID )
{
	D3DLIGHT9 light;
	ZeroMemory( &light, sizeof(D3DLIGHT9) );
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Position.x = 15;
	light.Position.y = 32;
	light.Position.z = -15;

	// 그림자를 생성할 조명의 방향 벡터
	light.Direction = D3DXVECTOR3( -0.707f, -0.707f, 0.707f );
	
	light.Falloff = 1;
	light.Diffuse.r = 0.73f;
	light.Diffuse.g = 0.73f;
	light.Diffuse.b = 0.73f;
	light.Diffuse.a = 1;
	light.Specular.r = 0.8f;
	light.Specular.g = 0.8f;
	light.Specular.b = 0.8f;	
	light.Specular.a = 1;	
	light.Ambient.r = 0.2f;
	light.Ambient.g = 0.2f;
	light.Ambient.b = 0.2f;	
	light.Ambient.a = 1;	
	light.Range = 1024.0f;
	light.Attenuation0 = 0.4f;

	D3DLIGHT9 light2;
	ZeroMemory( &light2, sizeof(D3DLIGHT9) );
	light2.Type = D3DLIGHT_POINT;
	light2.Position.x = 0;
	light2.Position.y = 32;
	light2.Position.z = 0;
	light2.Diffuse.r = 0.12f;
	light2.Diffuse.g = 0.12f;
	light2.Diffuse.b = 0.12f;	
	light2.Diffuse.a = 1;	
	light2.Specular.r = 0.2f;
	light2.Specular.g = 0.2f;
	light2.Specular.b = 0.2f;	
	light2.Specular.a = 1;	
	light2.Ambient.r = 0.2f;
	light2.Ambient.g = 0.2f;
	light2.Ambient.b = 0.2f;	
	light2.Ambient.a = 1;	
	light2.Range = 64.0f;
	light2.Attenuation0 = 0.4f;

	D3DLIGHT9 light3;
	ZeroMemory( &light3, sizeof(D3DLIGHT9) );
	light3.Type = D3DLIGHT_POINT;
	light3.Position.x = -15;
	light3.Position.y = 15;
	light3.Position.z = -5;
	light3.Diffuse.r = 0.35f;
	light3.Diffuse.g = 0.35f;
	light3.Diffuse.b = 0.35f;	
	light3.Diffuse.a = 1;	
	light3.Specular.r = 0.2f;
	light3.Specular.g = 0.2f;
	light3.Specular.b = 0.2f;	
	light3.Specular.a = 1;	
	light3.Ambient.r = 0.2f;
	light3.Ambient.g = 0.2f;
	light3.Ambient.b = 0.2f;	
	light3.Ambient.a = 1;	
	light3.Range = 64.0f;
	light3.Attenuation0 = 0.6f;

	m_pd3dDevice->SetLight( 0, &light );
	m_pd3dDevice->LightEnable( 0, TRUE );	
	m_pd3dDevice->SetLight( 1, &light2 );
	m_pd3dDevice->LightEnable( 1, TRUE );	
	m_pd3dDevice->SetLight( 2, &light3 );
	m_pd3dDevice->LightEnable( 2, TRUE );	
}

/*************************************************************************************************************************************************************/
/* 트랜스폼 관련 함수들																																		 */
/*************************************************************************************************************************************************************/


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 현재 행렬 저장 함수
 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::PushMatrix( VOID )
{
	m_RenderManager.PushMatrix();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
* \brief 행렬 복구 함수
* \param VOID	없음
* \return		없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::PopMatrix( VOID )
{	
	m_RenderManager.PopMatrix();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 월드변환 함수(이동만)

 회전은 하지 않고 이동만 수행하는 함수이다.

 * \param vTargetPos	이동할 위치
 * \return				없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::SetT( NxVec3 vTargetPos )
{
	D3DXMATRIX matTrans, matWorld;
	m_pd3dDevice->GetTransform( D3DTS_WORLD, &matWorld );
	D3DXMatrixTranslation( &matTrans, vTargetPos.x, vTargetPos.y, vTargetPos.z );
	matTrans *= matWorld;
	m_pd3dDevice->SetTransform( D3DTS_WORLD, &matTrans );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 월드 변환 함수(이동, 회전, 스케일링)

 객체의 포즈값을 읽어서 객체의 로컬 좌표계로 월드변환을 수행한다.

 * \param *ptr					객체 포인터
 * \param bUseOldWorldMatrix	TRUE인 경우 기존 월드행렬에서 추가로 변환, FALSE인 경우 객체 포즈 행렬만 가지고 변환
 * \return						변환한 월드 행렬
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3DXMATRIX 
CtMain::SetTRS( CtObjectBase *ptr, BOOL bUseOldWorldMatrix )
{
	D3DXMATRIX matOld;
	D3DXMATRIXA16 glMat;
	ptr->GetPose().getColumnMajor44(glMat);

	NxVec3 vScale = ptr->GetScale();
	D3DXMATRIXA16 matScale;	
	D3DXMatrixScaling( &matScale, vScale.x, vScale.y, vScale.z );

	if( bUseOldWorldMatrix )
	{
		m_pd3dDevice->GetTransform( D3DTS_WORLD, &matOld );
		glMat *= matOld;
	}	
	matScale *= glMat;

	m_pd3dDevice->SetTransform( D3DTS_WORLD, &matScale );

	return matScale;
}

/*************************************************************************************************************************************************************/
/* 브라우징 관련 함수들																																		 */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 큐브 내부로 진입하는 함수

 지정한 큐브의 내부로 진입하는 애니메이션을 수행하면서, 해당 큐브의 객체 로드 쓰레드 함수를 호출한다.

 * \param *pCube	큐브 객체 포인터
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::JumpIntoCube( CtObjectCube *pCube )
{
	// 큐브의 기존 포즈를 저장
	pCube->SavePoseScale();

	pCube->SetTransparency( 0.5f );

	// 오브젝트 로드. 로딩속도가 꽤 걸리므로 쓰레드를 만들어 수행
	if( pCube->GetNumObjects() == 0 ) AfxBeginThread( ThreadLoadCubeObjects, (LPVOID)pCube, THREAD_PRIORITY_NORMAL );	
	
	// 큐브의 충돌처리를 off
	//pCube->GetActor()->raiseActorFlag( NX_AF_DISABLE_COLLISION );		

	// 큐브를 중앙으로 이동시키면서 회전행렬을 단위행렬화 하는 애니메이션 
	NxMat34 matID;
	matID.id();
	pCube->SetPoseTransition( matID, pCube->GetScale(), 0.5f, 0.2f );

	// 큐브를 최대화시키는 애니메이션 
	pCube->SetPoseTransition( matID, D3DXVECTOR3(32, 32, 32), 0.6f, 0.5f );

	m_stStateFlags.bCubeChanging = TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 큐브에서 벗어나 부모 큐브로 돌아가는 함수

 지정한 큐브에서 벗어나 해당 큐브의 부모 큐브로 장면을 이동한다.

 * \param *pCube	벗어날 큐브 객체 포인터
 * \return			없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::JumpOutFromCube( CtObjectCube *pCube )
{	
	if( pCube == m_pMainCube ) return;

	// 큐브를 원래 포즈로 복귀시키는 애니메이션 
	pCube->SetPoseTransition( pCube->GetPoseTemp(), pCube->GetScaleTemp(), 0.6f, 0.5f );
	
	m_OSelection.Add( (CtObjectBase*)pCube );

	m_pCurrentCube = pCube->GetParentCube();

	m_stStateFlags.bCubeChanging = TRUE;
	pCube->Select();
}

VOID
CtMain::MoveObject( CtObjectBase* pObj, CtObjectCube* pSrc, CtObjectCube* pDst )
{	
	pSrc->RemoveObject( pObj );
	pDst->AddObject( pObj );
	NxScene* pScene1 = pSrc->GetScene();
	NxScene* pScene2 = pDst->GetScene();

	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	
	NxActor* pActor1 = pObj->GetActor();
	pActor1->saveToDesc( actorDesc );
	pActor1->saveBodyToDesc( bodyDesc );
	NxShape	 *const* pShapes = pActor1->getShapes();
	
	if( pObj->HasCustomMesh() )
	{

	}
	else
	{
		D3DXVECTOR3 vActorSize = pObj->GetActorSize();
		D3DXVECTOR3 vSize = pObj->GetScale();
		
		NxActor* pActor;
		pActor = CreateBoxActor( pDst, pObj, NxVec3(0,0,10), vActorSize.x * vSize.x, vActorSize.y * vSize.y, vActorSize.z * vSize.z );

		pObj->ResetMass( 1 );
		pObj->SetActorSize( D3DXVECTOR3(CUBE_WIDTH, CUBE_WIDTH, CUBE_WIDTH) );
		pObj->SetActor( pActor );
	}	
	pScene1->releaseActor( *pActor1 );	
}


/*************************************************************************************************************************************************************/
/* 객체 메시 설정 관련 함수들																																 */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체의 타입에 따라 기본 메시를 갖도록 (리)셋하는 함수

 객체가 custom 메시를 가졌다가 다시 기본 메시를 갖도록 해야 하는 경우가 있을 수 있으며, LoadObjects에서 객체의 기본 메시를 지정하기 위해
 이 함수를 호출함.

 * \param *ptr	객체 포인터
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::ChangeMeshToBasicModel( CtObjectBase *ptr )
{
	switch( ptr->GetType() )
	{
	case OBJECT_TYPE_CUBE:
		{
			ptr->SetDXUTMesh( &m_MeshCube );
			ptr->SetScale( D3DXVECTOR3(1,1,1) );
			break;
		}
	case OBJECT_TYPE_IMAGE:
		{
			ptr->SetDXUTMesh( &m_MeshImage );
			break;
		}
	case OBJECT_TYPE_CHIP:
		{
			ptr->SetDXUTMesh( &m_MeshChip );
			ptr->SetScale( D3DXVECTOR3(1,1,1) );
			break;
		}
	}
	CheckHover();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체의 메시를 클립아트에서 선택하는 모드
 
 객체가 가질 메시들의 데이터베이스를 제공하여 사용자가 객체를 원하는 메시로 지정할 수 있도록 한다.
 
 * \param *ptr	객체 포인터
 * \return		없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::ChangeMeshToClipartModel( CtObjectBase *ptr )
{	
	// 클립아트 선택 다이얼로그가 없으면 생성
	if( m_pMeshSelectDialog == NULL )
	{
		m_pMeshSelectDialog = new CCubeTopMeshSelectDialog( AfxGetMainWnd() );
		m_pMeshSelectDialog->Create( IDD_MESHSELECTDIALOG, AfxGetMainWnd() );
	}
	CTabCtrl *pTabCtrl = (CTabCtrl*)m_pMeshSelectDialog->GetDlgItem( IDC_TAB );
	m_pMeshSelectDialog->m_ctrlTab = pTabCtrl;
	CListBox *pListBox = (CListBox*)m_pMeshSelectDialog->GetDlgItem( IDC_LISTBOX );
	m_pMeshSelectDialog->m_listBox = pListBox;

	wsprintf( m_pMeshSelectDialog->m_lpszHomeDirName, L"%s", m_lpszHomeDirectory );				

	m_stStateFlags.bMeshSelectMode = TRUE;

	// 메쉬 데이터 폴더들을 검색
	int ndx = 0;
	CFileFind finder;	
	SHFILEINFO sfi; 
	bool bWorking;
	
	WCHAR dirname[255];
	wsprintf( dirname, L"%s\\data\\models\\*.*", m_lpszHomeDirectory );
	bWorking = finder.FindFile( dirname );

	while (bWorking)
	{
		// 먼저 model 디렉토리의 서브디렉토리들(모델 카테고리)을 탐색
		bWorking = finder.FindNextFile(); 

		if(finder.IsDots()) continue;		

		SHGetFileInfo( 
			finder.GetFilePath(), 
			0, 
			&sfi, 
			sizeof(SHFILEINFO), 
			SHGFI_DISPLAYNAME 
			); 

		// 모델명 리스트를 작성
		if( finder.IsDirectory() ) 
		{			
			TCITEM ti;
			ti.mask = TCIF_TEXT;
			WCHAR tabname[255];

			wsprintf( tabname, finder.GetFileName() );

			ti.pszText = tabname;
			m_pMeshSelectDialog->m_ctrlTab->InsertItem( 0, &ti );

			ndx++;
		}
	}

	pTabCtrl->SetCurSel(0);	
	m_pMeshSelectDialog->ShowWindow( SW_SHOW );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체의 메시를 사용자가 지정한 메시 파일로 변경하는 함수
 
 사용자 자신이 직접 지정하는 메시 파일로 객체의 메시를 변경한다.
 
 * \param *ptr	객체 포인터
 * \return		없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::ChangeMeshToSelectedModel( CtObjectBase *ptr )
{		
	HRESULT hr;

	// 다이얼로그에서 파일 지정
	CFileDialog dlg( TRUE, 0, 0, 0, NULL, AfxGetMainWnd() ); 
	dlg.DoModal();	
	CString path = dlg.GetPathName();

	ChangeMeshToCustomModel( ptr, path );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체의 메시를 주어진 경로명의 파일로 변경하는 함수
 
 변경할 메시의 경로명을 주어주면 해당 파일로 객체의 메시를 변경한다.
 
 * \param *ptr	객체 포인터
 * \return		없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::ChangeMeshToCustomModel( CtObjectBase *ptr, LPCWSTR meshfilepath )
{	
	HRESULT hr;

	// 메쉬파일정보 입력
	//stFileInfo stFI = ptr->m_stFileInfo;	
	//wsprintf( stFI.lpszMeshFilePath, meshfilepath );
	ptr->GetFileInfo()->lpszMeshFilePath = meshfilepath;

	// 기존 정보 삭제(블렌더 G에서 삭제할것)

	// 트랜스폼 정보 생성
	V( ptr->ImportMesh(meshfilepath) );

	// 메쉬 액터 생성
	m_PhysX.GenerateTriangleMesh( ptr, TRUE );

	// 커스텀 메쉬 플래그 on
	ptr->SetHasCustomMesh( TRUE );	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief CustomUI 초기화 함수.

 CtCustomUI 객체를 초기화하고 메인 클래스 단에서 추가로 작업할 것이 있으면 작성한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::InitCustomUI( VOID )
{
	m_CustomUI.Initialize( m_pd3dDevice, m_RenderManager.GetStateBlock(), m_Cameras );

	m_CustomUI.GetHUD()->SetCallback( CALLBACKS::OnGUIEvent );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 카메라 초기화 함수

 메인 초기화 함수에서 호출한다. 기본 카메라 속성을 설정한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::InitCamera( VOID )
{
	// 기본 카메라
	m_DefaultCamera.SetName( L"Default Cam" );
	m_DefaultCamera.SetEnablePositionMovement( TRUE );
	m_DefaultCamera.SetRotateButtons( FALSE, TRUE, FALSE );
	m_DefaultCamera.SetNumberOfFramesToSmoothMouseData( 6 );
		
	CamSet( &m_DefaultCamera );
	CamReset( TRUE );

	FLOAT fAspectRatio		= m_pCurrentCam->GetAspect();
	D3DXVECTOR3 vEyePt		= D3DXVECTOR3( 0, 40.f / fAspectRatio, -18.75f * fAspectRatio );
	D3DXVECTOR3 vLookatPt	= D3DXVECTOR3( 0, 0, 0 );
	m_pCurrentCam->SetViewParams( &vEyePt, &vLookatPt );
	
	m_Cameras.push_back( &m_DefaultCamera );
}

/*************************************************************************************************************************************************************/
/* 키보드, 마우스 이벤트 핸들러들																															 */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 마우스 이동 이벤트 핸들러

 마우스 이동시에 CheckHover() 함수를 사용하여 picking을 수행하여 마우스의 3D 좌표를 구한다.
 또 각각의 현재 상태(모드)에 따라 카메라를 회전하거나, 선택 툴을 사용하거나 하는 등 다르게 처리한다.

 * \param point		마우스 좌표
 * \return			카메라에 메시지를 보낼지 여부를 반환. TRUE면 메시지를 카메라에 보내고 FALSE면 메시지 전달을 멈춘다
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL 
CtMain::OnMouseMove( CPoint point )
{		
	m_stInputState.ptMousePos = point;	
	int moveX = m_stInputState.ptMousePos.x - m_stInputState.ptOldMousePos.x;
	int moveY = m_stInputState.ptMousePos.y - m_stInputState.ptOldMousePos.y;
	
	//////////////////////////////////////////////
	// 마우스 가운데 버튼 눌린 경우 카메라 회전
	//////////////////////////////////////////////
	if( m_stInputState.bMbuttonDown )
	{
		m_stStateFlags.bCameraRotateMode = TRUE;
		return TRUE;
	}

	//////////////////////////////////////////////////////////////////////////
	// 오브젝트들에 대한 롤오버 검사 및 마우스 3D 좌표 계산.
	// 비용이 많이 들기 때문에 롤오버 체크 관계없는 루틴은 윗부분으로 보낼 것
	//////////////////////////////////////////////////////////////////////////
	CheckHover();
	NxVec3 vMove3D = m_stInputState.v3DMousePos - m_stInputState.vOld3DMousePos;	

	//////////////////////////////////////////////
	// 자유회전이동 모드	
	//////////////////////////////////////////////
	if( m_stStateFlags.bChipRotateMode )
	{	
		// 내적으로 old와 new 3d 마우스 좌표의 사이각 구함
		NxVec3 vOldMouse, vMouse;
		NxVec3 nPos = m_pSelectedObj->GetPose().t;		

		vOldMouse = D3DXVec3ToNxVec3( m_stInputState.vOld3DMousePos ) - nPos;
		vMouse = D3DXVec3ToNxVec3( m_stInputState.v3DMousePos ) - nPos;

		vOldMouse.y = vMouse.y = 0;

		FLOAT cosAng = vOldMouse.dot( vMouse );
		cosAng /= ( (FLOAT)vOldMouse.magnitude() * (FLOAT)vMouse.magnitude() );
		FLOAT angle = (FLOAT)acos( cosAng );

		// 범위 제한(버그 방지)
		if( cosAng > 1 || cosAng < -1 ) return FALSE;
		if( angle > 0.5f || angle < -0.5f ) return FALSE;

		// 외적으로 방향 구함
		NxVec3 vCross;		
		vCross = vOldMouse.cross( vMouse );		

		// 사이각만큼 회전
		if( vCross.y < 0 ) angle *= -1;

		m_pSelectedObj->RotateGlobalY( angle );
	}	
	//////////////////////////////////////////////
	// Lasso 툴 모드	
	//////////////////////////////////////////////
	if( m_stStateFlags.bLassoModeReady ) 
	{
		int numV = m_ToolLasso.GetNumVertex();
		
		if( numV == 1 )
		{
			// Lasso 모드 처음 시작
			m_ToolLasso.Begin( m_stInputState.ptMousePos );
		}
		else
		{
			m_stStateFlags.bLassoMode = TRUE;

			if( !m_stStateFlags.bPilingMode ) 
			{
				FLOAT t, u, v;
				D3DXVECTOR3 v0, v1, v2, vChip, vDir, vEye;	

				v0 = m_ToolLasso.GetStartVertex();
				v1 = m_stInputState.vOld3DMousePos;
				v2 = m_stInputState.v3DMousePos;

				m_ToolLasso.Process( v1, v2, m_stInputState.ptMousePos );

				vEye = *m_pCurrentCam->GetEyePt();

				CtObjectBase **pObj = m_pCurrentCube->GetObjects();
				for( int i=0; i < m_pCurrentCube->GetNumObjects(); i++ )
				{	
					if( pObj[i]->IsSelected() == FALSE ) 
					{
						vChip = NxVectorToD3DVector( pObj[i]->GetPose().t );
						vDir = vChip - vEye;
						if( D3DXIntersectTri( &v0, &v1, &v2, &vEye, &vDir, &u, &v, &t ) )
						{	
							//////////////////////////////////////////////////////////////////////////
							// 셀렉션 추가
							//////////////////////////////////////////////////////////////////////////
							m_OSelection.Add( pObj[i] );
							m_CustomUI.OnObjectSelection( pObj[i] );
						}

						/*
						vDir = D3DXVECTOR3(m_vStartLassoPos.x, m_vStartLassoPos.y, m_vStartLassoPos.z) - (*m_pCurrentCam->GetEyePt());

						if( D3DXIntersectTri( &D3DXVECTOR3(m_vNextLassoPos.x, m_vNextLassoPos.y, m_vNextLassoPos.z), &v1, &v2, m_pCurrentCam->GetEyePt(), &vDir, &u, &v, &t ) )
						{
							// 새로운 중점							
							//ShiftLassoVertex();							
						}
						*/
					}				
				}		

			}
		}
	}
	m_stInputState.ptOldMousePos = m_stInputState.ptMousePos;
	m_stInputState.vOldOld3DMousePos = m_stInputState.vOld3DMousePos;
	m_stInputState.vOld3DMousePos = m_stInputState.v3DMousePos;

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 마우스 왼쪽 버튼 누름 이벤트 핸들러

 마우스 L버튼 클릭은 기본적으로 객체를 드래그하거나, 선택 영역 툴을 사용해 영역을 그리는데 사용된다.

 * \param point		마우스 좌표
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID 
CtMain::OnLButtonDown( CPoint point )
{
	m_stInputState.bLbuttonDown = TRUE;	

	//ZeroMemory( &m_stStateFlags, sizeof(stStateFlags) );
	
	//////////////////////////////////////////////////////////////////////////
	// 오브젝트 선택 및 드래그 모드
	//////////////////////////////////////////////////////////////////////////
	m_ToolLasso.End();
	
	if( m_pHoveredObj == NULL ) 
	{	
		m_CustomUI.HideContextMenu( OBJECT_TYPE_SELECTION );
		m_CustomUI.HideContextMenu( OBJECT_TYPE_CHIP );
		m_CustomUI.HideContextMenu( OBJECT_TYPE_CUBE );
		m_CustomUI.HideContextMenu( OBJECT_TYPE_PILE );

		if( !m_stInputState.bShiftDown ) ClearSelection();
		m_stStateFlags.bDragMode = FALSE;

		m_stStateFlags.bLassoModeReady = TRUE;		
		m_ToolLasso.Ready( m_stInputState.v3DMousePos, m_stInputState.ptMousePos );
	}
	else
	{		
		// 롤오버 오브젝트 클릭시 셀렉션에 추가
		if( !m_OSelection.IsInSelection( m_pHoveredObj ) )
		{
			// Shift 키를 누르지 않은 경우 기존 셀렉션을 비우고 추가한다.
			// Shift 키를 누른 경우는 기존 셀렉션에 추가
			if( !m_stInputState.bShiftDown ) ClearSelection();

			m_pSelectedObj = m_pHoveredObj;		
			m_OSelection.Add( m_pSelectedObj );

			if( m_stStateFlags.bBillboardBrowseMode )
				m_OClipboard.Remove( m_pSelectedObj );
		}				

		if( !m_stStateFlags.bBillboardBrowseMode ) StartDragMode();					
	}	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 마우스 왼쪽 버튼 떼어짐 이벤트 핸들러

 마우스 왼쪽 버튼이 떼어지는 경우 드래그 모드 등 관련된 모드를 종료하고 선택 툴 모드였을 경우 추가적인 처리를 해 준다.
 객체(들)을 선택한 경우 CustomUI 컨텍스트 메뉴를 보여준다.

 * \param point		마우스 좌표
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::OnLButtonUp( CPoint point )
{	
	m_stInputState.bLbuttonDown = FALSE;

	if( m_stInputState.vOld3DMousePos == m_stInputState.v3DMousePos )
		m_stStateFlags.bDragMode = FALSE;
	
	//////////////////////////////////////////////////////////////////////////
	// Lasso 모드 종료
	//////////////////////////////////////////////////////////////////////////
	if( m_stStateFlags.bLassoMode )
	{
		if( m_pHoveredObj != NULL && !m_stStateFlags.bLassoMode && !m_stStateFlags.bPilingMode )
		{			
			m_pHoveredObj->Select();
		}
		/*
		else 
		{			
			if( m_stStateFlags.bPilingMode && !m_stStateFlags.bPilingModeDone )
			{
				m_OSelection.CancelPilingMode();
			}	
		}
		*/			
		if( m_OSelection.GetNumObjects() > 0 && !m_stInputState.bShiftDown && !m_stStateFlags.bCubeChanging ) 
		{
			if( m_OSelection.GetNumObjects() == 1 )
			{
				m_pSelectedObj = m_OSelection.GetObjects()[0];
				if( m_pSelectedObj->IsChip() ) m_CustomUI.ShowContextMenu( OBJECT_TYPE_CHIP, m_stInputState.ptMousePos );
				if( m_pSelectedObj->IsCube() ) m_CustomUI.ShowContextMenu( OBJECT_TYPE_CUBE, m_stInputState.ptMousePos );
			}
			else if( m_stStateFlags.bLassoMode ) m_CustomUI.ShowContextMenu( OBJECT_TYPE_SELECTION, m_stInputState.ptMousePos );			
		}
	}	

	if( m_stStateFlags.bBillboardBrowseMode )
	{
		// 원상복귀
		if( m_OSelection.GetNumObjects() == 0 )
		{
			m_OClipboard.DoAtAllObject( SBM_SETPOSEBACK );
			m_OClipboard.DoAtAllObject( SBM_UNFREEZE );
			m_OClipboard.Clear();
			m_stStateFlags.bBillboardBrowseMode = FALSE;
		}				
	}

	m_stStateFlags.bDragMode = m_stStateFlags.bChipRotateMode = m_stStateFlags.bPilingMode = m_stStateFlags.bChipRotateModeReady = m_stStateFlags.bLassoModeReady = FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 마우스 왼쪽 버튼 더블클릭 이벤트 핸들러

 일반적인 클릭과 구분되는 더블클릭은 UI측면에서 마우스 클릭을 구분할 수 있는 몇 안되는 요소(마우스 입력 조합이 얼마 안되며, 터치를 염두에 두면 더더욱 적다) 중 하나이므로
 중요하다. 기본적으로 객체를 클릭하면 객체를 실행(열기)하게 된다. 특수 객체들에 대한 더블클릭시 특수한 기능으로 활용할 수 있다.(ex: 부분공간 더블클릭시 내부 객체 컨트롤 등..)

 * \param point		마우스 좌표
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::OnLButtonDblClk( CPoint point )
{
	// 배경에 더블클릭시 상위폴더로 이동
	if( m_pHoveredObj == NULL )
	{
		JumpOutFromCube( m_pCurrentCube );
		return;
	}

	// 오브젝트에 더블클릭시
	switch( m_pSelectedObj->GetType() )
	{
	case OBJECT_TYPE_CHIP:
		{
			// 칩 오브젝트는 실행한다
			CtFileInfo* pFI = m_pSelectedObj->GetFileInfo();
			ShellExecute( NULL, L"open", pFI->lpszFileName, NULL, pFI->lpszFilePath, SW_SHOW );				
			break;
		}
	case OBJECT_TYPE_CUBE:
		{
			// 큐브 오브젝트는 열어서 큐브 내부로 들어간다
			JumpIntoCube( (CtObjectCube*)m_pSelectedObj );			
		}
	}	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 마우스 가운데 버튼 누름 이벤트 핸들러

 현재 사용하지 않고 있다. 가운데 버튼을 통해 드래그를 수행하여 어떤 기능을 구현하고자 한다면 이곳에서 가운데버튼 드래그 모드를 on 시켜주어야 할것이다.

 * \param point		마우스 좌표
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::OnMButtonDown( CPoint point )
{	
	m_stInputState.bMbuttonDown = TRUE;
	//ZeroMemory( &m_stStateFlags, sizeof(stStateFlags) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 마우스 가운데 버튼 떼어짐 이벤트 핸들러

 현재는 마우스 가운데 버튼은 카메라의 오토 포커싱 용도로 활용되고 있다. 객체 위에서 M버튼 클릭시 객체에 자동포커싱을, 바탕에 클릭시 원래대로 복귀한다

 * \param point		마우스 좌표
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::OnMButtonUp( CPoint point )
{	
	m_stInputState.bMbuttonDown = FALSE;

	if( !m_stStateFlags.bCameraRotateMode )
	{
		if( m_pHoveredObj == NULL ) 
			CamReset();
		else 
			CamZoomToObject( m_pHoveredObj );	
	}	

	m_stStateFlags.bCameraRotateMode = FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 마우스 오른쪽 버튼 누름 이벤트 핸들러

 현재 마우스 우클릭시 객체의 리프트 모드를 on 시켜 객체를 띄우게 된다.

 * \param point		마우스 좌표
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::OnRButtonDown( CPoint point )
{
	m_stInputState.bRbuttonDown = TRUE;

	// 드래그모드에서 우클릭시 객체를 띄우는 리프트 모드 추가	
	if( m_stStateFlags.bDragMode || m_stStateFlags.bChipRotateMode ) 
	{
		m_stStateFlags.bLiftMode = TRUE;
		m_OSelection.DoAtAllObject( SBM_LIFT );
	}	

	if( m_pHoveredObj ) 
	{
		if( !m_OSelection.IsInSelection(m_pHoveredObj) )
		{
			ClearSelection();
			m_pSelectedObj = m_pHoveredObj;		
			m_OSelection.Add( m_pHoveredObj );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 마우스 오른쪽 버튼 떼어짐 이벤트 핸들러

 우클릭 해제시 현재는 리프트 모드 해제 및 윈도우 컨텍스트 메뉴 기능을 수행한다.

 * \param point		마우스 좌표
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::OnRButtonUp( CPoint point )
{	
	m_stInputState.bRbuttonDown = FALSE;

	if( m_stStateFlags.bLiftMode )
	{
		m_stStateFlags.bLiftMode = FALSE;	
		m_OSelection.DoAtAllObject( SBM_UNLIFT );
	}

	if( m_OSelection.GetNumObjects() > 0 && !m_stStateFlags.bDragMode )
	{
		OnContextMenu( point );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 마우스 휠 이벤트 핸들러

 마우스 휠은 현재 카메라의 줌인/아웃 기능과 함께 Ctrl 키와 조합하여 객체들의 사이즈 조정 기능을 수행한다.
 휠의 특성을 잘 활용하여 UI의 기능에 활용하면 좋을 것이다.
	
 * \param amount	휠 드래그 정도. 보통 +- 120의 값을 가진다. WM_WHEEL 참조. 
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID 
CtMain::OnMouseWheel( short amount )
{
	if( m_stInputState.bCtrlDown ) 
	{
		if( amount > 0 ) 
			m_OSelection.DoAtAllObject( SBM_SCALEOFFSET, (FLOAT)1.1f, (FLOAT)1.1f, (FLOAT)1.1f );
		else
			m_OSelection.DoAtAllObject( SBM_SCALEOFFSET, (FLOAT)0.9f, (FLOAT)0.9f, (FLOAT)0.9f );
	}
	else
		CamZoom( amount );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 키보드 누름 이벤트 핸들러

 되도록이면 마우스만을 사용하여 UI를 수행하는것이 좋으나, 기능이 복잡하게 되면 마우스만으로는 힘들다.
 그렇다고 table top 인터페이스처럼 모든 것을 마임이나 컨텍스트 메뉴를 사용하게 되면 신속한 조작이 힘들고 활용성이 떨어질 수가 있다.
 현재는 데스크탑 인터페이스를 구현하는 프로그램이므로 키보드를 활용하고 터치 기반 인터페이스로 전환하려면 마임이나 컨텍스트 메뉴, 아이콘등을 사용할 수밖에 없다.

 * \note
 ALT, ESC등 특수 키 들은 메시지가 translate되기 전에 처리되어야 한다. CTAPP::Run() 함수의 메인 메시지 루프를 보면 translate 전에 
 CtMain::MsgProc()을 수행하는 것을 볼 수 있다. 이는 PreTranslateMessage() 와 동일하게 메시지를 CtMain 클래스에서 먼저 처리하도록 되어 있는 것이다.
 만약 메시지 처리 순서나 구조를 바꾸려 한다면 위의 사항을 고려해야 한다.(특수키)

 * \param keycode	가상 키 코드
 * \return			카메라에 메시지를 전달할지 여부. TRUE면 메시지를 카메라에 전달하고, FALSE면 메시지 전달을 여기서 끝낸다.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
CtMain::OnKeyDown( BYTE keycode )
{	
	switch( keycode )
	{		
	case VK_F8:
		{
			m_RenderManager.ToggleWireframe();
			break;
		}
	case VK_F9:
		{
			m_RenderManager.ToggleSmooth();
			break;
		}
	case VK_F11:
		{			
			break;
		}
	case VK_CONTROL:
		{
			m_stInputState.bCtrlDown = TRUE;
			StartTurnMode();
			break;
		}	
	case VK_SHIFT:
		{
			m_stInputState.bShiftDown = TRUE;
			break;
		}
	case VK_SPACE:
		{			
			m_stStateFlags.bCameraRotateMode = TRUE;			
			break;
		}
	case VK_HOME:
		{
			break;
		}
	case VK_END:
		{
			break;
		}
	case VK_INSERT:
		{
			break;
		}
	case VK_DELETE:
		{
			break;
		}
	case VK_PRIOR:
		{
			break;
		}
	case VK_NEXT:
		{
			break;
		}
	case VK_ADD:
		{		
			break;
		}
	case VK_MULTIPLY:
		{		
			break;
		}
	case VK_DIVIDE:
		{		
			break;
		}
	default:
		{
			return TRUE;
		}
	}	

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 키보드 떼어짐 이벤트 핸들러

 * \param keycode	가상 키 코드
 * \return			카메라에 메시지를 전달할지 여부. TRUE면 메시지를 카메라에 전달하고, FALSE면 메시지 전달을 여기서 끝낸다.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
CtMain::OnKeyUp( BYTE keycode )
{
	switch( keycode )
	{
	case VK_CONTROL:
		{
			m_stInputState.bCtrlDown = FALSE;
			if( m_stStateFlags.bTurnMode ) EndTurnMode();			
			break;
		}
	case VK_SHIFT:
		{			
			//if( m_OSelection.GetNumObjects() > 0 )
				//m_CustomUI.ShowContextMenu( OBJECT_TYPE_SELECTION, m_stInputState.ptMousePos );
			m_stInputState.bShiftDown = FALSE;
			break;
		}
	case VK_SPACE:
		{
			m_stStateFlags.bCameraRotateMode = FALSE;
			break;
		}
	default:
		{
			return TRUE;
		}
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 컨텍스트 메뉴 핸들러 함수

 OnRButtonUp 함수에서 컨텍스트 메뉴 호출 상황이라 판단하면 이 핸들러를 호출한다. 
 바탕화면이나 탐색기에서처럼 해당 객체(들)에 대한 윈도우 셸 컨텍스트 메뉴를 띄운다.

 * \param point		마우스 포지션
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID	
CtMain::OnContextMenu( CPoint point )
{
	CStringArray arrayFiles;
	CWnd* pWnd = AfxGetMainWnd();

	// string array에 선택된 객체들의 경로를 넣는다
	/*
	DWORD nItem = m_OSelection.GetNumObjects();
	CtObjectBase** pObjects = m_OSelection.GetObjects();
	for( DWORD i=0; i < nItem; i++ )
		arrayFiles.Add( pObjects[i]->GetFilePath() );

	CShellContextMenu scm;
	scm.SetObjects( arrayFiles );

	// 컨텍스트 메뉴 보여줌
	UINT idCommand = scm.ShowContextMenu( pWnd, point );

	// 선택된 메뉴에 따라
	switch( idCommand )
	{
	case 19:
		{
			// 이름 바꾸기는 따로 처리해야 한다. 
			break;
		}
	case 0:
		{
			// 커맨드가 없으면 아무것도 안함
			break;
		}
	default:
		{
			// 나머지는 모두 해당 메뉴를 실행
			pWnd->SendMessage( WM_COMMAND, idCommand, 0 );
			
		}
	}
	*/
	// 컨텍스트 메뉴
	/*
	int numsel = m_OSelection.GetNumObjects();
	if( numsel == 1 )
	{			
		CMenu popup;
		CMenu *MyMenu = 0;
		CPoint pt;
		GetCursorPos(&pt);

		switch( m_pSelectedObj->GetType() )
		{
		case OBJECT_TYPE_CHIP:
			{
				if( m_pSelectedObj->IsPiled() ) popup.LoadMenu(IDR_MENU_PILE);
				else popup.LoadMenu(IDR_MENU_BASIC);
				break;
			}				
		case OBJECT_TYPE_CUBE:
			popup.LoadMenu(IDR_MENU_BASIC);				
			break;				
		case OBJECT_TYPE_SELECTION:
			popup.LoadMenu(IDR_MENU_SELECTION);
			break;
		}			
		MyMenu = popup.GetSubMenu(0);				
		if( MyMenu ) MyMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, AfxGetMainWnd());							
		return;
	}		
	if( numsel > 1 )
	{
		CMenu popup;
		CMenu *MyMenu = 0;
		CPoint pt;
		GetCursorPos(&pt);

		bool all_cip = TRUE;
		CtObjectBase** pObj = m_OSelection.GetObjects();
		for( int i=0; i < numsel; i++ )
		{
			if( pObj[i]->IsPiled() == FALSE ) 
			{
				all_cip = FALSE;
				break;
			}
		}

		if( all_cip == TRUE )
		{
			popup.LoadMenu(IDR_MENU_PILE);
		}
		else
		{
			popup.LoadMenu(IDR_MENU_SELECTION);				
		}			

		MyMenu = popup.GetSubMenu(0);				

		if( MyMenu ) MyMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, AfxGetMainWnd());							
		return;
	}
	*/
	if( m_OSelection.GetNumObjects() > 0 ) 
	{
		if( m_OSelection.GetNumObjects() == 1 )
		{
			if( m_pSelectedObj->IsChip() ) 
			{
				if( m_pSelectedObj->IsPiled() )
					m_CustomUI.ShowContextMenu( OBJECT_TYPE_PILE, m_stInputState.ptMousePos );
				else
					m_CustomUI.ShowContextMenu( OBJECT_TYPE_CHIP, m_stInputState.ptMousePos );
			}
			if( m_pSelectedObj->IsCube() ) m_CustomUI.ShowContextMenu( OBJECT_TYPE_CUBE, m_stInputState.ptMousePos );
		}
		else 
		{
			bool bAllPiledChip = TRUE;
			CtObjectBase** pObj = m_OSelection.GetObjects();
			for( int i=0; i < m_OSelection.GetNumObjects(); i++ )
			{
				if( !pObj[i]->IsPiled() ) bAllPiledChip = FALSE;
			}

			if( bAllPiledChip )
				m_CustomUI.ShowContextMenu( OBJECT_TYPE_PILE, m_stInputState.ptMousePos );			
			else
				m_CustomUI.ShowContextMenu( OBJECT_TYPE_SELECTION, m_stInputState.ptMousePos );			
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 주어진 객체의 메시를 지정한 파일 경로의 메시로 모핑하여 바꾸는 함수

 이 함수에서는 ZMorpher 모듈을 사용하여 객체의 DXUT 메시를 지정한 파일의 메시로 모핑을 수행한다.

 더미 CtObject 객체를 만들어 ImportMesh를 통해 DXUT 메시를 갖도록 하고, 소스 객체의 DXUT메시와 함께 ZMorpher에 넘겨 모핑을 위한 계산을 하고
 계산이 끝나면 소스 메시 형태로 돌아가게 한 다음 프레임이 이동할 때마다 보간값 alpha를 늘려가면서 렌더링하면 자연스럽게 타겟 메시로 모핑된다.
 보간 렌더링 과정이 완전히 끝나면 타겟 DXUT메시를 객체의 메시로 지정하고 PhysX TriMesh 액터를 쿠킹한다..

 * \param pObj		객체 포인터
 * \param filepath	메시 파일 경로
 * \return			에러 코드
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT 
CtMain::MorphMesh( CtObjectBase* pObj, LPCWSTR filepath )
{
	/*
	HRESULT hr;

	pObj->UnHover();
	ClearSelection();

	// 더미 CT오브젝트 생성
	CtObjectBase* pDummyObj = new CtObjectBase();
	pDummyObj->m_AnimInfo.Disable();
	pDummyObj->UnHover();
	pDummyObj->SetParentCube( m_pCurrentCube );

	m_pCurrentCube->m_ObjectArray.Add( pDummyObj );

	m_pMorphingSrcObj = pObj;
	m_pMorphingTgtObj = pDummyObj;

	// 타겟 모델 로드
	NxActor* pActor = pDummyObj->GetActor();
	pDummyObj->ImportMesh( filepath );	

	m_PhysX.GenerateTriangleMesh( pDummyObj, TRUE );
	NxMat34 vPose = pObj->GetActor()->getGlobalPose();
	pActor->setGlobalPose( vPose );
	pActor->setGlobalPosition( vPose.t + NxVec3(4,0,0) );


	// 디버깅 
	pObj->m_bHasMorphMesh = TRUE;
	pDummyObj->m_bHasMorphMesh = TRUE;

	pObj->m_pMorphObj = m_Morpher.m_SrcObj;
	pDummyObj->m_pMorphObj = m_Morpher.m_TgtObj;

	// 모핑용 메쉬 생성
	m_Morpher.Initialize();
	m_Morpher.m_SrcObj->m_pUserData = pObj;
	m_Morpher.m_TgtObj->m_pUserData = pDummyObj;

	m_Morpher.PrepareForMorph( pObj->m_pDXUTMesh, pDummyObj->m_pDXUTMesh );
	
	

	pObj->m_pMorphObj->CalcObjectInfo();
	V( CreateMeshVB(pObj, pObj->m_pMorphObj->m_dwNumFaces) );
	V( CreateMeshIB(pObj, pObj->m_pMorphObj->m_dwNumFaces) );

	pDummyObj->m_pMorphObj->CalcObjectInfo();
	V( CreateMeshVB(pDummyObj, pDummyObj->m_pMorphObj->m_dwNumFaces) );
	V( CreateMeshIB(pDummyObj, pDummyObj->m_pMorphObj->m_dwNumFaces) );

	pObj->m_bHasCustomMesh = TRUE;

	//m_pCurrentCube->m_ObjectArray.Remove( m_pCurrentCube->GetNumObjects() - 1 );

	
	m_Morpher.MergeEmbeddings();

	m_Morpher.TransformBackToSrcMesh();
	ResetBuffersFromMorphMesh( pObj );
	/*
	for( float i=0; i <= 1; i += 0.01f )
	{
		m_Morpher.SetInterpolatedMesh( i );
		ResetBuffersFromMorphMesh( m_pMorphingSrcObj );
		Render();
	}

	pObj->m_pDXUTMesh = pDummyObj->m_pDXUTMesh;
	pObj->m_bHasMorphMesh = FALSE;
	pObj->m_bHasCustomMesh = TRUE;
	pObj->GenerateTriangleMesh( m_PhysX, TRUE );
	*/

	ResetBuffersFromMorphMesh( m_pMorphingSrcObj );
	ResetBuffersFromMorphMesh( m_pMorphingTgtObj );
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 메시 버텍스 버퍼 생성 함수

 이 함수는 모핑용 메시 버텍스 버퍼를 생성하는 함수이다. 
 ZObject를 렌더링하기 위해서는 그것의 vertex, index들을 VB, IB로 만들어 DrawIndexedPrimitive를 호출하여야 하기 때문

 * \param pObj		객체 포인터
 * \param numVert	생성할 버텍스 버퍼 크기
 * \return			에러 코드
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT
CtMain::CreateMeshVB( CtObjectBase* pObj, DWORD numVert )
{
	HRESULT hr;

	LPDIRECT3DVERTEXBUFFER9 pVB = pObj->GetVB();
	SAFE_RELEASE( pVB );
	
	V_RETURN( m_pd3dDevice->CreateVertexBuffer(sizeof(MESHVERTEX) * numVert, 0, D3DFVF_MESHVERTEX, D3DPOOL_DEFAULT, &pVB, NULL) );
	
	pObj->SetVB( pVB );

	pObj->GetMorphObject()->m_dwVBSize = numVert;

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 메시 인덱스 버퍼 생성 함수

 이 함수는 모핑용 메시 인덱스 버퍼를 생성하는 함수이다. 
 ZObject를 렌더링하기 위해서는 그것의 vertex, index들을 VB, IB로 만들어 DrawIndexedPrimitive를 호출하여야 하기 때문

 * \param pObj		객체 포인터
 * \param numFace	인덱스 버퍼 크기(실제 크기는 tri당 인덱스 3개이므로 곱하기 3)
 * \return			에러 코드
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT
CtMain::CreateMeshIB( CtObjectBase* pObj, DWORD numFace )
{
	HRESULT hr;

	LPDIRECT3DINDEXBUFFER9 pIB = pObj->GetIB();
	SAFE_RELEASE( pIB );

	V_RETURN( m_pd3dDevice->CreateIndexBuffer(sizeof(WORD) * numFace * 3, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pIB,	NULL) );
	
	pObj->SetIB( pIB );
	pObj->GetMorphObject()->m_dwIBSize = numFace;

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief ZObject로부터 VB, IB를 채우는 함수

 ZObject가 표현하는 메시 구조로부터, 미리 생성해 둔 버텍스 및 인덱스 버퍼를 채워 렌더링 가능하게 한다.
 만약 객체가 새로운 ZObject를 갖게 되면 VB, IB를 새로 생성해야 하는데, 기존의 버퍼 크기보다 큰 공간을 필요로 할때 VB, IB를 제거하고 다시 생성한다.

 * \param pObj	객체 포인터
 * \return		에러 코드
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT 
CtMain::ResetBuffersFromMorphMesh( CtObjectBase* pObj )
{
	HRESULT hr;

	ZObject* zObj = pObj->GetMorphObject();
	zObj->CalcObjectInfo();

	DWORD totverts = zObj->m_dwNumFaces * 3;
	DWORD totfaces = zObj->m_dwNumFaces;

	// ZObject가 기존 VB, IB보다 큰 버퍼를 요구하면 다시 생성한다
	if( zObj->m_dwVBSize < totverts ) V_RETURN( CreateMeshVB(pObj, totverts) );
	if( zObj->m_dwIBSize < totfaces ) V_RETURN( CreateMeshIB(pObj, totfaces) );

	MESHVERTEX* pVertices = 0;
	WORD *pIndices = 0;
	pObj->GetVB()->Lock( 0, 0, (VOID**)&pVertices, D3DLOCK_DISCARD );
	pObj->GetIB()->Lock( 0, 0, (VOID**)&pIndices, D3DLOCK_DISCARD );

	DWORD vIndex = 0, fIndex = 0, sIndex = 0;
	ZVert** pV;
	ZFace** pF;
	ZEdge* pEdge;

	// ZObject의 모든 ZMesh의 정보를 VB, IB에 채운다
	ZMesh** pMeshes = zObj->m_pMeshes.GetData();		
	for( int i=0; i < zObj->m_dwNumSubset; i++ )
	{
		for( DWORD j=0; j < zObj->m_pMeshes.GetSize(); j++ )
		{
			sIndex = pMeshes[j]->GetIndexOfSubsetID( i );
			pF = pMeshes[j]->m_pFaces[sIndex].GetData();

			for( DWORD k=0; k < pMeshes[j]->m_pFaces[sIndex].GetSize(); k++ )
			{
				// VB, IB를 채우기 위해서는 엣지만 보면 된다. 노멀 및 텍스쳐 좌표 뿐 아니라
				// 버텍스 역시 엣지가 포인터로 가지고 있기 때문
				pEdge = pF[k]->edge;
				for( int m=0; m < 3; m++, pEdge = pEdge->next, fIndex++ )
				{
					pVertices[fIndex].pos = pEdge->v1->pos;
					pVertices[fIndex].normal = pEdge->normal;
					pVertices[fIndex].texcoord = pEdge->texcoord;					
					pIndices[fIndex] = fIndex;
				}
			}
		}
	}

	pObj->GetVB()->Unlock();
	pObj->GetIB()->Unlock();

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 로드 함수

 큐브가 가리키는 디렉토리 내의 모든 파일 및 디렉토리를 로드하여 객체로 생성하고, 큐브의 객체 배열에 넣는다.
 * \param pCube	객체들을 로드할 타겟 큐브 포인터
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID 
CtMain::LoadObjects( CtObjectCube* pCube )
{
	CFileFind		finder;	
	SHFILEINFO		sfi;				// 셸 파일 정보 클래스
	UINT			ndx = 0;			// 객체 인덱스 번호
	bool			bWorking;			// TRUE인 동안 계속 찾음
	CtObjectBase*	pNewObj;			// 새롭게 생성될 객체 포인터
	WCHAR			pathstr[255];		

	// 아이콘 크기를 잠시 크게 만듬
	CoInitialize(NULL);
	//DWORD old = SetShellLargeIconSize(96);

	wsprintf( pathstr, L"%s\\%s\\*.*", pCube->GetFileInfo()->lpszFilePath, pCube->GetFileInfo()->lpszFileName );
	bWorking = finder.FindFile( pathstr );

	// CustomUI 브라우저에 파일 목록을 표시하기 위해 포인터를 얻어둠
	CDXUTListBox* pListBox = m_CustomUI.GetFL()->GetListBox( IDC_HUD_LISTBOX_FILELIST );
	pListBox->RemoveAllItems();

	srand( time(NULL) );
	CRITICAL_SECTION* pCS = DXUTGetCriticalSection();

	// 디렉토리 내의 모든 파일 및 폴더 검색	
	memset(&sfi, 0, sizeof(sfi));

	while (bWorking)
	{
		bWorking = finder.FindNextFile(); 

		if(finder.IsDots()) continue;	
		ndx++;

		// 파일 정보 및 아이콘을 얻음
		CString filePath = finder.GetFilePath();
		::SHGetFileInfo( 
			filePath, 
			0, 
			&sfi, 
			sizeof(SHFILEINFO), 
			SHGFI_DISPLAYNAME | SHGFI_SELECTED | SHGFI_TYPENAME | SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_SHELLICONSIZE | SHGFI_LARGEICON
			); 

		//************************************************************************
		EnterCriticalSection( pCS );		
		//************************************************************************

		// 디렉토리인 경우
		if( finder.IsDirectory() )
		{		
			CtObjectCube* pNewCube = AddCube( pCube, finder.GetFileName(), &sfi, ndx );			

			
			// compartment를 사용할 경우 씬이 많아지면 오류가 발생하는 관계로 사용하지 않음

			if( GetPhysX()->m_bHasHW ) 
			{
				NxCompartmentDesc cdesc;
				cdesc.type = NX_SCT_RIGIDBODY;
				cdesc.deviceCode = NX_DC_PPU_AUTO_ASSIGN;
				cdesc.flags &= ~NX_CF_INHERIT_SETTINGS;
				pNewCube->SetPhysXCompartment( pNewCube->GetScene()->createCompartment( cdesc ) );
			}
			
			pNewObj = (CtObjectBase*)pNewCube;
		}
		// 파일인 경우
		else
		{
			pNewObj = (CtObjectBase*)AddChip( pCube, finder.GetFileName(), &sfi, ndx );			
		}		

		// 객체의 타입에 따라 기본 메시를 갖도록 함
		ChangeMeshToBasicModel( pNewObj );

		// CustomUI 브라우저에 파일 리스트에 추가		
		pListBox->AddItem( pNewObj->GetFileInfo()->lpszFileName, 0 );

		//************************************************************************
		LeaveCriticalSection( pCS );		
		//************************************************************************

		DestroyIcon( sfi.hIcon );
	}

	finder.Close();

	// 아이콘 크기를 잠시 크게 만듬
	//SetShellLargeIconSize( old );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 칩 객체 추가 함수

 파일 정보로부터 칩 객체를 생성하고 전달받은 큐브 객체의 객체 배열에 넣는다.

 * \param pCube			부모 큐브 객체 포인터
 * \param lpszFileName	파일명
 * \param *sfi			파일 정보 객체 포인터
 * \param index			객체 인덱스 번호. 부모 디렉토리 내에서 파일의 순서
 * \return				생성된 칩 객체의 포인터
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CtObjectChip* 
CtMain::AddChip( CtObjectCube* pCube, LPCWSTR lpszFileName, SHFILEINFO *sfi, UINT index )
{
	// 칩 객체 생성
	WCHAR filepath[255];
	wsprintf( filepath, L"%s%s\\", pCube->GetFileInfo()->lpszFilePath, pCube->GetFileInfo()->lpszFileName );
	CtObjectChip *newChip = new CtObjectChip( lpszFileName, filepath, sfi->szDisplayName, sfi->szTypeName );			
	newChip->SetParentCube( pCube );	

	// 아이콘 추출
	newChip->LoadIcon( sfi->hIcon );

	// 초기 위치를 정한다
	D3DXVECTOR3 vLook = *m_pCurrentCam->GetLookAtPt();
	D3DXVECTOR3 vEye = *m_pCurrentCam->GetEyePt();
	D3DXVECTOR3 vPos, vScale, vSize;
	D3DXPLANE plane( 1, 1, 1, 0 );
	D3DXPlaneIntersectLine( &vPos, &plane, &vLook, &vEye );
	vPos.x += (rand() % 2 == 1) ? (rand() % 8) : -(rand() % 8);
	vPos.z += (rand() % 2 == 1) ? (rand() % 8) : -(rand() % 8);
	vPos.y = CHIP_THICKNESS;

	// 파일 타입(문서, 미디어)에 따라 객체를 다르게 생성할 필요가 있을경우
	switch( newChip->GetType() )
	{
	case OBJECT_TYPE_IMAGE:
		{
			D3DSURFACE_DESC surfDesc;
			LPDIRECT3DTEXTURE9* pTexture = newChip->GetTextures();
			pTexture[0]->GetLevelDesc( 0, &surfDesc );

			vScale = D3DXVECTOR3( surfDesc.Width / 400.f, IMAGE_THICKNESS * surfDesc.Width / 2000.f, surfDesc.Height / 400.f );
			vSize = D3DXVECTOR3( 1, 1, 1 );
			break;
		}
	case OBJECT_TYPE_CHIP:
		{
			// 칩의 경우 옆면 텍스쳐 생성
			newChip->GenerateNameTexture();
		}
	default:
		{
			vSize = D3DXVECTOR3( CHIP_WIDTH, CHIP_THICKNESS, CHIP_HEIGHT );
			vScale = D3DXVECTOR3( 1, 1, 1 );
		}
	}
	newChip->SetActorSize( vSize );
	newChip->SetScale( vScale );

	// 기본 박스 모양 액터 생성
	NxActor* pNewActor = CreateBoxActor( pCube, newChip, D3DXVec3ToNxVec3(vPos), vSize.x * vScale.x, vSize.y * vScale.y, vSize.z * vScale.z );
	newChip->ResetMass( 1 );

	// 리스트에 삽입
	pCube->AddObject( (CtObjectBase*)newChip );		

	return newChip;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 큐브 객체 추가 함수.

 파일 정보(디렉토리인 경우)로부터 큐브 객체를 생성하고 부모 큐브 객체 배열에 삽입한다.

 * \param pCube				부모 큐브 객체 포인터
 * \param lpszFileName		디렉토리 이름
 * \param *sfi				파일 정보 객체
 * \param index				객체 인덱스 번호. 부모 디렉토리에서 디렉토리가 위치한 순서
 * \return					생성된 큐브 객체 포인터
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CtObjectCube* 
CtMain::AddCube( CtObjectCube* pCube, LPCWSTR lpszFileName, SHFILEINFO *sfi, UINT index )
{
	// 큐브 객체 생성
	WCHAR filepath[255];
	wsprintf( filepath, L"%s%s\\", pCube->GetFileInfo()->lpszFilePath, pCube->GetFileInfo()->lpszFileName );
	CtObjectCube *newCube = new CtObjectCube( lpszFileName, filepath, sfi->szDisplayName, sfi->szTypeName );			
	newCube->SetParentCube( pCube );	

	D3DXVECTOR3	vLook = *m_pCurrentCam->GetLookAtPt();
	D3DXVECTOR3	vEye = *m_pCurrentCam->GetEyePt();
	D3DXVECTOR3	vPos;
	D3DXPLANE	plane( 1, 1, 1, 0 );
	D3DXPlaneIntersectLine( &vPos, &plane, &vLook, &vEye );
	vPos.x += (rand() % 2 == 1) ? (rand() % 8) : -(rand() % 8);
	vPos.z += (rand() % 2 == 1) ? (rand() % 8) : -(rand() % 8);
	vPos.y = CUBE_WIDTH;

	// 기본 정육면체 박스 액터 생성
	CreateBoxActor( pCube, newCube, D3DXVec3ToNxVec3(vPos), CUBE_WIDTH, CUBE_WIDTH, CUBE_WIDTH );
	newCube->SetActorSize( D3DXVECTOR3(CUBE_WIDTH, CUBE_WIDTH, CUBE_WIDTH) );
	newCube->ResetMass( 1 );	

	// 랜덤 머티리얼 지정
	D3DMATERIAL9 mat;
	ZeroMemory( &mat, sizeof(D3DMATERIAL9) );
	mat.Diffuse.r = 0.9;//(rand() % 256) / 256.f;
	mat.Diffuse.g = 0.9;//(rand() % 256) / 256.f;
	mat.Diffuse.b = 0.9;//(rand() % 256) / 256.f;
	mat.Diffuse.a = 1;
	mat.Specular.r = mat.Specular.g = mat.Specular.b = 0.0;
	mat.Specular.a = 1;
	mat.Power = 100;
	mat.Ambient.r = mat.Diffuse.r / 2.f;
	mat.Ambient.g = mat.Diffuse.g / 2.f;
	mat.Ambient.b = mat.Diffuse.b / 2.f;
	mat.Ambient.a = 1;
	newCube->SetMaterial( mat );

	// 옆면 텍스쳐 생성
	newCube->GenerateNameTexture();

	// 리스트에 삽입
	pCube->AddObject( (CtObjectBase*)newCube );		

	newCube->GetActor()->setContactReportFlags( NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD | NX_NOTIFY_FORCES );
	newCube->GetActor()->setContactReportThreshold( CONTACT_FORCE_THRESHOLD_ADDCHIP_TO_PILE );

	return newCube;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 기본 박스 형태 PhysX 액터 생성

 지정한 부모 큐브의 씬의 지정한 위치에 크기의 박스 액터를 생성한다.

 * \param pCube		부모 큐브 객체 포인터
 * \param *pObj		새로 생성한 객체 포인터
 * \param pos		액터의 위치 벡터 초기값
 * \param sizeX		X축 사이즈 : offset이 아닌 실제 크기
 * \param sizeY		Y축 사이즈
 * \param sizeZ		Z축 사이즈
 * \return			생성한 액터 포인터
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NxActor* 
CtMain::CreateBoxActor( CtObjectCube* pCube, CtObjectBase *pObj, const NxVec3& pos, float sizeX, float sizeY, float sizeZ )
{
	if( !pCube->GetScene() ) return NULL;	

	// body를 만들어야 다이나믹 액터가 됨
	NxBodyDesc bodyDesc;
	bodyDesc.angularDamping	= 0.05f;			

	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions = NxVec3(sizeX / 2.f, sizeY / 2.f, sizeZ / 2.f);
	boxDesc.group = ACTOR_GROUP_DEFAULT;

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&boxDesc);
	actorDesc.body			= &bodyDesc;
	actorDesc.density		= 1;					
	actorDesc.globalPose.t  = pos;

	NxActor* pActor = NULL;
	pObj->SetActor( pActor = pCube->GetScene()->createActor(actorDesc) );	

	pActor->userData = (void*)pObj;

	return pActor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 장치 적합 여부 판단 DXUT 콜백 함수 핸들러
 * \param pCaps 
 * \param AdapterFormat 
 * \param BackBufferFormat 
 * \param bWindowed 
 * \param pUserContext 
 * \return 
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL	
CtMain::IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 장치 변경 DXUT 콜백 함수 핸들러
 * \param pDeviceSettings 
 * \param pUserContext 
 * \return 
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL	
CtMain::ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 장치 생성 DXUT 콜백 함수 핸들러
 * \param pd3dDevice 
 * \param pBackBufferSurfaceDesc 
 * \param pUserContext 
 * \return 
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT
CtMain::OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
* \brief 장치 리셋 DXUT 콜백 함수 핸들러
* \param pd3dDevice 
* \param pBackBufferSurfaceDesc 
* \param pUserContext 
* \return 
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT
CtMain::OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	m_pd3dDevice = pd3dDevice;
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 프레임 이동 DXUT 콜백 함수 핸들러

 프레임이 렌더링되고 다음 프레임을 렌더링하는 사이에 처리할 것들을 작성.

 * \param pd3dDevice	D3D 장치
 * \param fTime			누적 시간
 * \param fElapsedTime	경과 시간
 * \param pUserContext	사용자 데이터
 * \return				없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
	/************************************************************************/
	/* PhysX 시뮬레이션                                                     */
	/************************************************************************/
	// 현재는 단순하게 하고 있으나 timestep을 ElapsedTime등을 가지고 정교하게 계산하여 적용하면 
	// 시뮬레이션 렉을 줄여 좀더 부드럽게 만들수 있음.
	if( m_PhysX.OnSimulation() == TRUE ) 
	{
		m_pCurrentCube->GetScene()->simulate( fElapsedTime );

		m_pCurrentCube->GetScene()->flushStream();
		while( !m_pCurrentCube->GetScene()->fetchResults(NX_RIGID_BODY_FINISHED, TRUE) );									
	}

	/************************************************************************/
	/* 객체들의 포즈를 업데이트                                             */
	/************************************************************************/
	// 시뮬레이션 값 또는 키프레임으로부터 얻은 값을 포즈값으로 지정.
	CtObjectBase** pObjects = m_pCurrentCube->GetObjects();
	for( DWORD i=0; i < m_pCurrentCube->GetNumObjects(); i++ )
	{
		if( pObjects[i]->IsDeleted() )
		{
			m_pCurrentCube->RemoveObject( pObjects[i], TRUE );
			continue;
		}
		UpdateActor( pObjects[i] );
	}

	// 현재 카메라에 프레임 이동 함수 적용
	if( typeid(*m_pCurrentCam) == typeid(CtFirstPersonCamera) )
		((CtFirstPersonCamera*)m_pCurrentCam)->FrameMove( fElapsedTime );		
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 메인 클래스 렌더링 핸들러 함수. 메인 Render() 함수 호출 및 기타 필요한 부분 작성.

 * \param pd3dDevice	D3D 장치
 * \param fTime			누적 시간
 * \param fElapsedTime	경과 시간
 * \param pUserContext	사용자 데이터
 * \return				없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
	Render( fElapsedTime );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 메인 메시지 핸들러 함수.

 * \param hWnd					윈도우 핸들
 * \param uMsg					메시지
 * \param wParam				WPARAM
 * \param lParam				LPARAM
 * \param pbNoFurtherProcessing	TRUE면 메시지 처리 완료. 메시지 전달을 멈춤. FALSE면 계속 전달
 * \param pUserContext			사용자 데이터
 * \return						없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
CtMain::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
	// 다이얼로그에 우선권을 줌
	LRESULT res;

	if( m_stStateFlags.bMeshSelectMode )
	{
		*pbNoFurtherProcessing = TRUE;
		return TRUE;
	}

	//((CWnd*)m_pMeshSelectDialog)->OnWndMsg( uMsg, wParam, lParam, &res );
	// Custom UI
	//if( !!m_stStateFlags.bMeshSelectMode )
		if( m_CustomUI.MsgProc( uMsg, wParam, lParam) ) return TRUE;
		
	// 메인 메시지 핸들링을 여기에서
	CPoint pt;
	GetCursorPos( &pt );	

	switch( uMsg )
	{
	case WM_MOUSEMOVE:
		{			
			if( OnMouseMove(pt) ) break;
			return TRUE;
		}
	case WM_LBUTTONDOWN:
		{
			OnLButtonDown( pt );
			return TRUE;
		}
	case WM_LBUTTONUP:
		{
			OnLButtonUp( pt );
			return TRUE;
		}
	case WM_LBUTTONDBLCLK:
		{
			OnLButtonDblClk( pt );
			return TRUE;
		}
	case WM_MBUTTONDOWN:
		{
			OnMButtonDown( pt );
			break;
		}
	case WM_MBUTTONUP:
		{
			OnMButtonUp( pt );
			break;
		}
	case WM_RBUTTONDOWN:
		{
			OnRButtonDown( pt );
			break;
		}
	case WM_RBUTTONUP:
		{
			OnRButtonUp( pt );
			break;
		}
	case WM_MOUSEWHEEL:
		{
			OnMouseWheel( (short)HIWORD(wParam) );
			return TRUE;
		}
	case WM_KEYDOWN:
		{
			if( OnKeyDown(wParam) ) break;
			return TRUE;
		}
	case WM_KEYUP:
		{
			if( OnKeyUp(wParam) ) break;
			return TRUE;
		}
	case WM_USER:
		{
			((CtObjectCube*)wParam)->Serialize( FALSE );
			break;
		}
	case WM_COMMAND:
		{
			switch( wParam )
			{
			case IDC_CHANGE_BASICMODEL:
				break;
			case IDC_CHANGE_CLIPARTMODEL:
				ChangeMeshToClipartModel(m_pSelectedObj);
				break;
			case IDC_CHANGE_CUSTOMMODEL:
				break;
			case IDC_TRANSFORM_QUICK_TEST:
				break;
			case ID_TRANSFORM_SETBACK:
				break;
			case ID_MENU_BASIC_BILLBOARD:
				BillboardBrowse();
				break;
			case IDC_PILE_POPCHIP:
				m_OSelection.PopChipFromPile( m_pCurrentCam->GetEyePt() );
				break;
			case IDC_PILE_BREAKPILE:
				m_OSelection.BreakPiles();
				break;
			case IDC_MORPH_DIVIDE:
				break;
			case IDC_MORPH_CUT:
				break;
			case IDC_MORPH_EMBED:
				break;
			case IDC_CM_MESH:
				ChangeMeshToClipartModel( m_pSelectedObj );				
				break;
			case IDC_VIEW_BILLBOARD:
				BillboardBrowse();
				break;
			}
			return TRUE;
		}
	default:
		{			
			return FALSE;			
		}
	}

	// 마지막으로 카메라 조작에 메시지를 넘김
	if( m_pCurrentCam->HandleMessages( hWnd, uMsg, wParam, lParam ) ) return TRUE;


	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 장치를 잃었을 때의 DXUT 콜백 함수 핸들러
 * \param pUserContext 
 * \return 없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::OnLostDevice( void* pUserContext )
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
* \brief 장치를 파괴할 때의 DXUT 콜백 함수 핸들러
* \param pUserContext 
* \return 없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID	
CtMain::OnDestroyDevice( void* pUserContext )
{
	m_CustomUI.OnDestroyDevice();
	m_RenderManager.OnDestroyDevice();

	m_MeshChip.Destroy();
	m_MeshCube.Destroy();
	m_MeshWorld.Destroy();
	m_MeshImage.Destroy();

	SAFE_RELEASE( m_pMainVB );		
	SAFE_RELEASE( m_pMainIB );	

	SAFE_RELEASE( m_pd3dDevice );
	m_pd3dDevice = 0;	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief CustomUI 이벤트 핸들러

 CustomUI에서의 메시지 핸들링 함수이다. 2D로 표현되는 CustomUI 컨트롤들에 대한 조작을 여기서 처리한다.

 * \param nEvent		이벤트 ID
 * \param nControlID	이벤트 발생한 컨트롤 ID
 * \param pControl		이벤트 발생한 컨트롤 포인터
 * \param pUserContext	유저 데이터
 * \return				없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtMain::OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	CDXUTDialog* pHUD = m_CustomUI.GetHUD();
	CDXUTDialog* pFI = m_CustomUI.GetFI();
	CDXUTDialog* pOI = m_CustomUI.GetOI();

	CDXUTListBox* pListBox;

	switch( nControlID )
	{
	/************************************************************************/
	/* 메인 HUD                                                             */
	/************************************************************************/
	case IDC_HUD_BUTTON_TOGGLE:
		{
			m_CustomUI.ToggleBrowser();
			break;
		}		
	case IDC_HUD_BUTTON_REWIND:					
		{ 
			if( m_OSelection.GetNumObjects() > 0 ) m_OSelection.DoAtAllObject( SBM_REWIND );
			break;
		}	
	case IDC_HUD_BUTTON_FOWARD:					
		{ 
			if( m_OSelection.GetNumObjects() > 0 ) m_OSelection.DoAtAllObject( SBM_FOWARD );
			break; 
		}
	case IDC_HUD_BUTTON_PLAYPS:					
		{ 
			m_PhysX.ToggleSimulation();
			break; 
		}
	case IDC_HUD_BUTTON_UPDIRE:					
		{ 
			JumpOutFromCube( m_pCurrentCube );
			break; 
		}
	case IDC_HUD_BUTTON_HMDIRE:					
		{
			if( m_pCurrentCube != m_pMainCube ) JumpIntoCube( m_pMainCube );
			break; 
		}
	case IDC_HUD_COMBOBOX_CAMERALIST:				
		{
			/*
			pListBox = ((CDXUTListBox*)pControl);
			switch( nEvent )
			{
			case EVENT_LISTBOX_ITEM_DBLCLK:
			{
			break;
			}
			case EVENT_LISTBOX_SELECTION:
			{
			break;
			}
			case EVENT_LISTBOX_SELECTION_END:
			{				
			pCTmain->m_OSelection.Clear();
			DXUTListBoxItem** pItems = pListBox->GetItemArray()->GetData();
			for( DWORD i=0; i < pListBox->GetItemArray()->GetSize(); i++ )
			if( pItems[i]->bSelected ) pCTmain->SelectObjectByFilename( pItems[i]->strText );
			}
			}
			*/

			break; 
		}
	case IDC_HUD_TEXT_PATHNAME:					
		{ break; }
	case IDC_HUDARCBALL:							
		{ break; }
	case IDC_HUD_ARCBALL:
		{ break; }
	case IDC_HUD_LISTBOX_FILELIST:						
		{ 
			pListBox = ((CDXUTListBox*)pControl);
			switch( nEvent )
			{
			case EVENT_LISTBOX_ITEM_DBLCLK:
				{
					break;
				}
			case EVENT_LISTBOX_SELECTION:
				{
					break;
				}
			case EVENT_LISTBOX_SELECTION_END:
				{				
					m_OSelection.Clear();
					DXUTListBoxItem** pItems = pListBox->GetItemArray()->GetData();
					for( DWORD i=0; i < pListBox->GetItemArray()->GetSize(); i++ )
						if( pItems[i]->bSelected ) SelectObjectByFilename( pItems[i]->strText );
				}
			}	
			break; 
		}
	case IDC_HUD_TEXTBOX_FILEINFO:				
		{ break; }
	case IDC_HUD_BUTTON_CAMERA:					
		{ break; }
	case IDC_HUD_BUTTON_SEARCH:					
		{ break; }
	case IDC_HUD_BUTTON_CLIPBD:					
		{ break; }
	case IDC_HUD_BUTTON_SUBRGN:					
		{ break; }
	case IDC_HUD_BUTTON_MTVIEW:					
		{ break; }
	case IDC_HUD_BUTTON_ARRANG:					
		{ break; }
	case IDC_HUD_BUTTON_PRIMIT:					
		{ break; }
	case IDC_HUD_BUTTON_CONFIG:					
		{ break; }

	/************************************************************************/
	/* 추가 브라우저 - 파일정보                                             */
	/************************************************************************/

	/************************************************************************/
	/* 추가 브라우저 - 객체정보                                             */
	/************************************************************************/
	case IDC_OI_STATIC_MATERIALINFO:				
		{ break; }
	case IDC_OI_COMBOBOX_MATERIALLIST:			
		{ break; }
	case IDC_OI_COLORBOX_AMBIENT:					
		{ break; }
	case IDC_OI_STATIC_COLORBOX_AMBIENT_RGB	:	
		{ break; }
	case IDC_OI_COLORBOX_DIFFUSE:					
		{ break; }
	case IDC_OI_STATIC_COLORBOX_DIFFUSE_RGB:		
		{ break; }
	case IDC_OI_COLORBOX_SPECULAR:				
		{ break; }
	case IDC_OI_STATIC_COLORBOX_SPECULAR_RGB:		
		{ break; }
	case IDC_OI_STATIC_COLORBOX_AMBIENT:			
		{ break; }
	case IDC_OI_STATIC_COLORBOX_DIFFUSE:			
		{ break; }
	case IDC_OI_STATIC_COLORBOX_SPECULAR:			
		{ break; }
	case IDC_OI_SLIDER_AMBIENT_R:					
		{ break; }
	case IDC_OI_SLIDER_AMBIENT_G:					
		{ break; }
	case IDC_OI_SLIDER_AMBIENT_B:					
		{ break; }
	case IDC_OI_SLIDER_AMBIENT_A:					
		{ break; }
	case IDC_OI_SLIDER_DIFFUSE_R:					
		{ break; }
	case IDC_OI_SLIDER_DIFFUSE_G:					
		{ break; }
	case IDC_OI_SLIDER_DIFFUSE_B:					
		{ break; }
	case IDC_OI_SLIDER_DIFFUSE_A:					
		{ break; }
	case IDC_OI_SLIDER_SPECULAR_R:				
		{ break; }
	case IDC_OI_SLIDER_SPECULAR_G:				
		{ break; }
	case IDC_OI_SLIDER_SPECULAR_B:				
		{ break; }
	case IDC_OI_SLIDER_SPECULAR_A:				
		{ break; }
	case IDC_OI_STATIC_SIZE:						
		{ break; }
	case IDC_OI_STATIC_SIZE_XYZ:					
		{ break; }
	case IDC_OI_SLIDER_SIZE_X:					
		{ break; }
	case IDC_OI_SLIDER_SIZE_Y:					
		{ break; }
	case IDC_OI_SLIDER_SIZE_Z:					
		{ break; }
	case IDC_OI_STATIC_PHYSICS:					
		{ break; }
	case IDC_OI_STATIC_PHYSICS_PROPERTIES:		
		{ break; }
	case IDC_OI_SLIDER_PHYSICS_1:					
		{ break; }
	case IDC_OI_SLIDER_PHYSICS_2:					
		{ break; }
	case IDC_OI_SLIDER_PHYSICS_3:					
		{ break; }

	/************************************************************************/
	/* 셀렉션 컨텍스트 메뉴		                                            */
	/************************************************************************/
	case IDC_CM_SELECTION_PILING:
		{
			m_OSelection.StartPilingMode();
			m_OSelection.LinkByFixedJoint( TRUE );
			m_stStateFlags.bPilingModeDone = TRUE;
			break;
		}
	case IDC_CM_SELECTION_PILINGBYTYPE:
		{
			m_OSelection.PileByFileTypes();
			m_stStateFlags.bPilingModeDone = TRUE;
			break;
		}
	case IDC_CM_LOCK:
		{
			m_OSelection.DoAtAllObject( SBM_FREEZE );			
			break;
		}
	case IDC_CM_UNLOCK:
		{
			m_OSelection.DoAtAllObject( SBM_UNFREEZE );			
			break;
		}
	case IDC_CM_SELECTION_LINK:
		{
			m_OSelection.StartPilingMode();
			m_OSelection.LinkByFixedJoint( FALSE );
			break;
		}
	case IDC_CM_SELECTION_UNLINK:
		{
			m_OSelection.BreakPiles();			
			break;
		}
	case IDC_CM_DELETE:
		{			
			m_OSelection.DoAtAllObject( SBM_DELETE );						
			break;
		}
	case IDC_CM_BROWSE:
		{
			JumpIntoCube( (CtObjectCube*)m_pSelectedObj );
			break;
		}
	case IDC_CM_GRID:
		{
			m_stStateFlags.bLassoMode = FALSE;
			if( m_OSelection.GetNumObjects() == 1 && m_pSelectedObj->IsPiled() )
			{
				m_OSelection.Clear();
				CtObjectChip* pChip = (CtObjectChip*)m_pSelectedObj;
				while( pChip->GetPrev() ) pChip = pChip->GetPrev();
				while( pChip->GetNext() )
				{
					m_OSelection.Add( pChip );
					pChip = pChip->GetNext();
				}
				m_OSelection.Add( pChip );
				BillboardBrowse();
			}
			else
				BillboardBrowse();

			m_OSelection.Clear();
			break;
		}
	case IDC_CM_POPCHIP:
		{
			if( m_stStateFlags.bBillboardBrowseMode )
			{
				m_OClipboard.DoAtAllObject( SBM_SETPOSEBACK );
				m_OClipboard.DoAtAllObject( SBM_UNFREEZE );
				m_OClipboard.Clear();
				m_stStateFlags.bBillboardBrowseMode = FALSE;
			}
			m_OSelection.PopChipFromPile( m_pCurrentCam->GetEyePt() );
			break;
		}
	case IDC_CM_BREAKPILE:
		{
			m_OSelection.BreakPiles();
			break;
		}
	case IDC_CM_MESH:
		{
			ChangeMeshToSelectedModel( m_pSelectedObj );
			break;
		}
	}
	m_CustomUI.HideContextMenu( OBJECT_TYPE_SELECTION );
	m_CustomUI.HideContextMenu( OBJECT_TYPE_CHIP );
	m_CustomUI.HideContextMenu( OBJECT_TYPE_CUBE );
	m_CustomUI.HideContextMenu( OBJECT_TYPE_PILE );
}











//////////////////////////////////////////////////////////////////////////
//
// 큐브, 칩 컨텍스트 메뉴 핸들러
//
//////////////////////////////////////////////////////////////////////////

VOID 
CtMain::OnMenu_ChangeBasicModel( VOID )
{
	ChangeMeshToBasicModel( m_pSelectedObj );	
}

VOID CtMain::OnMenu_ChangeClipartModel( VOID )
{
	ChangeMeshToClipartModel( m_pSelectedObj );
}

VOID CtMain::OnMenu_ChangeCustomModel( VOID )
{
	CFileDialog dlg(TRUE, 0, 0, 0, NULL, AfxGetMainWnd()); 
	dlg.DoModal();	
	CString filename = dlg.GetPathName();

	if( lstrlen( filename) > 0 ) ChangeMeshToCustomModel( m_pSelectedObj, filename );
}

VOID CtMain::OnMenu_ChangeImageBasedModel( VOID )
{	
}

VOID CtMain::OnMenu_MorphStart( VOID )
{
	CFileDialog dlg(TRUE, 0, 0, 0, NULL, AfxGetMainWnd()); 
	dlg.DoModal();	
	CString filename = dlg.GetPathName();

	if( lstrlen( filename) > 0 )
	{
		HRESULT hr;
		if( FAILED( hr = MorphMesh( m_pSelectedObj, filename ) ) )
		{
			//MessageBox( DXGetErrorString9(hr), 0, 0 );
			exit( 0 );
		}
	}	
}

VOID CtMain::OnMenu_MorphSetBack( VOID )
{
	//Morph_SetBack();
}

VOID CtMain::OnMenu_FreezeObject( VOID )
{
	m_OSelection.DoAtAllObject( SBM_FREEZE );
}

VOID CtMain::OnMenu_UnFreezeObject( VOID )
{
	m_OSelection.DoAtAllObject( SBM_UNFREEZE );
}

VOID CtMain::OnMenu_BillboardObject( VOID )
{
	BillboardBrowse();
}

//////////////////////////////////////////////////////////////////////////
//
// 셀렉션 컨텍스트 메뉴 핸들러
//
//////////////////////////////////////////////////////////////////////////

VOID CtMain::OnMenu_SelectionPilingNormal( VOID )
{
	m_OSelection.StartPilingMode();
	m_OSelection.LinkByFixedJoint( TRUE );
	m_stStateFlags.bPilingModeDone = TRUE;
}

VOID CtMain::OnMenu_SelectionPilingByType( VOID )
{
	m_OSelection.PileByFileTypes();
}

//////////////////////////////////////////////////////////////////////////
//
// 파일 컨텍스트 메뉴 핸들러
//
//////////////////////////////////////////////////////////////////////////

VOID CtMain::OnMenu_PilePopChip( VOID )
{
}

VOID CtMain::OnMenu_PileBreak( VOID )
{
	m_OSelection.BreakPiles();
}

void CtMain::OnMorphDivide()
{
	m_pMorphingTgtObj->GetActor()->setGlobalPose( m_pMorphingTgtObj->GetPose() );
	m_Morpher.DivideSrcMeshesByTgtMeshes();
	ResetBuffersFromMorphMesh( m_pMorphingSrcObj );

}

void CtMain::OnMorphCut()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	m_Morpher.MergeEmbeddings();
}

void CtMain::OnMorphEmbed()
{
	m_Morpher.m_SrcObj->EmbedToSphere();
	m_Morpher.m_TgtObj->EmbedToSphere();
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}
