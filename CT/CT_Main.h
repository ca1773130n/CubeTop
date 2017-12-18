#pragma once

#include "stdafx.h"
#include "CT_Object.h"
#include "CT_Selection.h"
#include "CT_Morph.h"
#include "CT_PhysX.h"
#include "CT_CustomUI.h"
#include "CT_RenderManager.h"
#include "CT_Camera.h"
#include "CT_Settings.h"
#include "CT_MeshSelectDialog.h"
#include "Unicode.h"
#include <queue>
#include <stack>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 메인 상태 플래그 구조체

 메인 프로그램의 상태(현재 어떤 모드인지)를 나타내는 각종 플래그들.
 필요한 플래그가 있으면 추가한다.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct stStateFlags
{
	bool bIsIdleNow;				///< 아이들 상태인지 여부. TRUE일경우 렌더링을 중지한다. 
	bool bDragMode;					///< 드래그 모드.
	bool bTurnMode;					///< 뒤집기 모드. 
	bool bLiftMode;					///< 띄우기 모드.
	bool bLassoMode;				///< 올가미 선택 모드
	bool bLassoModeReady;			///< 올가미 선택 모드가 준비되었음.
	bool bLassoDirection;			///< 올가미 선택 방향 
	bool bChipRotateMode;			///< 회전이동 모드
	bool bChipRotateModeReady;		///< 회전이동 모드가 준비되었음.
	bool bPilingMode;				///< 파일링 모드 중.
	bool bPilingModeDone;			///< 파일링 모드 완료단계
	bool bPilingAddModeDone;		///< 파일링추가 모드 완료단계
	bool bBillboardBrowseMode;		///< 빌보드 브라우징모드.
	bool bCameraRotateMode;			///< 카메라 패닝 모드.			
	bool bMeshSelectMode;			///< 메시 선택 모드.
	bool m_bMorphingMode;			///< 메시 모핑 모드.
	bool bCubeChanging;				///< 큐브 전환중.
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 입력 상태 플래그 구조체

 키보드, 마우스 등의 입력장치 상태를 나타내는 각종 플래그들. 
 필요한 플래그가 있으면 추가한다.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct stInputState
{
	bool		bLbuttonDown;			///< 마우스 왼쪽버튼 눌림
	bool		bMbuttonDown;			///< 마우스 가운데버튼 눌림
	bool		bRbuttonDown;			///< 마우스 오른쪽버튼 눌림
	int			nWheelDelta;			///< 마우스 휠

	CPoint		ptMousePos;				///< 마우스 2D 포지션
	CPoint		ptOldMousePos;			///< 이전 마우스 2D 포지션
	D3DXVECTOR3 vStartDragPos;			///< 드래그모드 시작점 3D 벡터
	D3DXVECTOR3 vStartDragLocalPos;		///< 드래그모드 시작점의 오브젝트기준 로컬 3D 벡터
	D3DXVECTOR3 v3DMousePos;			///< 마우스 포지션 3D 벡터
	D3DXVECTOR3 vOld3DMousePos;			///< 이전 마우스 포지션 3D 벡터
	D3DXVECTOR3 vOldOld3DMousePos;		///< 이전 이전 마우스 포지션 3D 벡터

	bool		bCtrlDown;				///< 컨트롤키 눌림
	bool		bAltDown;				///< 알트키 눌림
	bool		bShiftDown;				///< 시프트키 눌림
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 프로그램 메인 클래스

 메시지 이벤트 처리 및 대부분의 인터랙션을 구현한다. 메인 클래스에서 프로그램 구동에 필요한 모든 데이터를 가지고 있으며
 구조가 큰 것들은 클래스 또는 구조체로 만들어 나간다.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtMain
{
public:	
	LPDIRECT3DDEVICE9			m_pd3dDevice;					///< D3D 장치

	LPDIRECT3DVERTEXBUFFER9		m_pMainVB;						///< 기본 버텍스 버퍼. 크게 잡아놓고 다용도로 사용.
	LPDIRECT3DINDEXBUFFER9		m_pMainIB;						///< 기본 인덱스 버퍼.

	WCHAR						m_lpszHomeDirectory[255];
	/************************************************************************/
	/* 기본 메시 객체들                                                     */
	/************************************************************************/
	CDXUTMesh					m_MeshChip;						///< 칩 기본 메시
	CDXUTMesh					m_MeshCube;						///< 큐브 기본 메시
	CDXUTMesh					m_MeshWorld;					///< 월드 기본 메시
	CDXUTMesh					m_MeshImage;					///< 이미지 객체 기본 메시

	/************************************************************************/
	/* 플래그 구조체들                                                      */
	/************************************************************************/
	stStateFlags				m_stStateFlags;					///< 메인 상태 플래그들
	stInputState				m_stInputState;					///< 입력장치 상태 플래그들

	/************************************************************************/
	/* 다이얼로그들                                                         */
	/************************************************************************/
	CCubeTopMeshSelectDialog*	m_pMeshSelectDialog;

	/************************************************************************/
	/* 각종 툴 객체들                                                       */
	/************************************************************************/
	CtObjectSelection			m_OSelection;					///< 메인 셀렉션 객체
	CtObjectSelection			m_OClipboard;					///< 클립보드
	CtToolSelectFan				m_ToolLasso;					///< 올가미 선택 툴 객체

	/************************************************************************/
	/* 칩, 큐브 직접 포인터들                                               */
	/************************************************************************/
	CtObjectCube*				m_pMainCube;					///< 메인큐브(바탕화면 디렉토리)
	CtObjectCube*				m_pCurrentCube;					///< 현재 큐브
	CtObjectCube*				m_pChangingCube;				///< 큐브 전환모드로 인해 바뀌어지는 큐브

	CtObjectBase*				m_pHoveredObj;					///< 롤오버된 객체 포인터
	CtObjectBase*				m_pSelectedObj;					///< 선택된 객체 포인터(선택된 객체가 1개인 경우)
	CtObjectBase*				m_pMorphingSrcObj;				///< 메시 모핑 진행중인 객체 포인터
	CtObjectBase*				m_pMorphingTgtObj;				///< 타겟 메시를 갖는 더미 객체 포인터. 모핑과정에 필요.

	/************************************************************************/
	/* 모듈들                                                               */
	/************************************************************************/
	CtPhysX						m_PhysX;						///< PhysX 객체	
	CtRenderManager				m_RenderManager;				///< 렌더링 매니져
	CtCustomUI					m_CustomUI;						///< CustomUI 객체
	ZMorpher					m_Morpher;						///< 3D 모델 모핑을 위한 모퍼 객체

	/************************************************************************/
	/* 카메라 및 조명                                                       */
	/************************************************************************/
	deque<CBaseCamera*>			m_Cameras;						///< 카메라 리스트
	CBaseCamera*				m_pCurrentCam;					///< 현재 카메라
	CtFirstPersonCamera			m_DefaultCamera;				///< 기본 카메라

	/************************************************************************/
	/* 기타                                                                 */
	/************************************************************************/	

public:

	/// 생성자
	CtMain( VOID );

	/// 소멸자
	~CtMain( VOID );

	/************************************************************************/
	/* 초기화 함수들                                                        */
	/************************************************************************/

	/// 직렬화 함수. 장면의 클래스 객체 정보를 파일에 저장 및 파일로부터 복구한다.
	VOID			Serialize( BOOL bStoring );
	
	/// 메인 프로그램 초기화 
	HRESULT Initialize( VOID );

	/// Direct3D 초기화 
	HRESULT			InitD3D( VOID );

	/// CustomUI 초기화
	VOID			InitCustomUI( VOID );

	/// 카메라 초기화
	VOID			InitCamera( VOID );

	/// 기본 버텍스 및 인덱스 버퍼를 초기화 하는 함수.
	HRESULT			InitBuffers( VOID );
	
	/// 메인 큐브 생성 함수
	VOID			MakeMainCube( VOID );

	/// 트레이 아이콘 추가
	VOID			RegistTrayIcon( VOID );

	/// 마우스 Picking 함수
	FLOAT			CheckHover( VOID );
	
	/************************************************************************/
	/* 객체 속성 변경 관련 함수들                                           */
	/************************************************************************/

	/// 오브젝트 크기 조정. 각 축에 대해 비율을 지정.
	VOID			ScaleObject( CtObjectBase* pObj, FLOAT xAmount, FLOAT yAmount, FLOAT zAmount );

	/************************************************************************/
	/* 폴더 및 파일 객체 로드 및 브라우징 관련 함수들                       */
	/************************************************************************/

	/// 큐브의 포함 객체들을 로드하는 함수. 쓰레드로 작동.
	VOID			LoadObjects( CtObjectCube* pCube );

	/// 현재 큐브의 객체 배열에 칩 객체를 추가
	CtObjectChip*	AddChip( CtObjectCube* pCube, LPCWSTR lpszFileName, SHFILEINFO *sfi, UINT index );

	/// 현재 큐브의 객체 배열에 큐브 객체를 추가
	CtObjectCube*	AddCube( CtObjectCube* pCube, LPCWSTR lpszFileName, SHFILEINFO *sfi, UINT index );

	/// 지정된 크기의 박스 형태 PhysX 액터 생성
	NxActor*		CreateBoxActor( CtObjectCube* pCube, CtObjectBase *pObj, const NxVec3& pos, float sizeX, float sizeY, float sizeZ );

	/// 큐브 내부로 이동
	VOID			JumpIntoCube( CtObjectCube* pCube );
	
	/// 상위 폴더 큐브로 이동(현재 큐브에서 벗어남)
	VOID			JumpOutFromCube( CtObjectCube *pCube );
	
	VOID			MoveObject( CtObjectBase* pObj, CtObjectCube* pSrc, CtObjectCube* pDst );

	/************************************************************************/
	/* 선택물 및 툴 관련 함수들                                             */
	/************************************************************************/

	/// 메인 셀렉션을 클리어
	VOID			ClearSelection( VOID );

	/// 주어진 파일명으로부터 객체를 선택
	VOID			SelectObjectByFilename( LPCWSTR filename );

	/// 드래그 모드 시작
	VOID			StartDragMode( VOID );

	/// 뒤집기 모드 시작
	VOID			StartTurnMode( VOID );

	/// 뒤집기 모드 마무리
	VOID			EndTurnMode( VOID );

	/// 오브젝트 빌보딩
	VOID			BillboardObject( VOID );

	/// 빌보드 브라우징 모드
	VOID			BillboardBrowse( VOID );

	/************************************************************************/
	/* 장면 드로잉 관련 함수들                                              */
	/************************************************************************/

	/// 메인 렌더링 함수
	VOID			Render( FLOAT fElapsedTime );

	/// 오브젝트 드로잉 함수. 
	VOID			DrawObject( CtObjectBase *pObj, bool bReflection = FALSE );

	/// 큐브가 포함하는 객체들의 그림자를 모두 드로잉하는 함수
	VOID			DrawObjectShadow( CtObjectCube* pCube );

	/// 객체들의 반사체를 그리는 함수
	VOID			DrawObjectReflection( VOID );

	/// 선택 또는 롤오버된 객체들의 이름을 그리는 함수
	VOID			RenderIconName( VOID );

	/// 주어진 영역, 색상, 텍스트포맷에 따라 문자열을 출력하는 함수.
	VOID RenderText( CDXUTTextHelper* pTxtHelper, int left, int top, int right, int bottom, D3DXCOLOR& color, DWORD dwFlags, BOOL bBeginEnd, const WCHAR* strMsg, ... );

	/************************************************************************/
	/* PhysX 및 업데이트 함수들(OnFrameMove에서 쓰이는것들)                 */
	/************************************************************************/

	/// 객체의 액터 포즈 등을 업데이트하는 함수
	VOID			UpdateActor( CtObjectBase* pObj );

	/************************************************************************/
	/* 카메라 및 조명 관련 함수들                                           */
	/************************************************************************/

	/// 셋팅된 값에 따라 world, view, projection 매트릭스를 설정하는 함수.
	VOID			CamView( VOID );

	/// 현재 카메라를 주어진 인자의 카메라로 설정하는 함수.
	VOID			CamSet( CBaseCamera* pCam );

	/// 카메라 뷰를 기본값으로 셋팅하는 함수
	VOID			CamReset( BOOL bInit = FALSE );

	/// 주어진 만큼 카메라를 줌인/줌아웃 하는 함수
	VOID			CamZoom( short amount );

	/// 오브젝트에 대해 자동 포커싱하는 함수.
	VOID			CamZoomToObject( CtObjectBase *ptr );

	/// XY평면상의 카메라 Panning을 하는 함수.
	VOID			CamPan( int iMoveX, int iMoveY );

	/// 메인 조명 초기화 함수.
	VOID			SetupLights( VOID );

	/************************************************************************/
	/* 매트릭스 변환 관련 함수들                                            */
	/************************************************************************/

	/// 매트릭스 스택에 현재 월드 매트릭스를 저장
	VOID			PushMatrix( VOID );

	/// 매트릭스 스택에 저장된 월드 매트릭스를 불러옴.
	VOID			PopMatrix( VOID );

	/// 월드 좌표계를 이동
	VOID			SetT( NxVec3 vTargetPos );

	/// 월드 좌표계를 주어진 객체의 좌표계로 변환(이동 및 회전, 스케일링)
	D3DXMATRIX		SetTRS( CtObjectBase *ptr, BOOL bUseOldWorldMatrix );

	/************************************************************************/
	/* 키보드, 마우스 이벤트 핸들러                                         */
	/************************************************************************/

	/// 마우스 이동 이벤트 핸들러
	BOOL			OnMouseMove( CPoint point );

	/// 마우스 왼쪽버튼 이벤트 핸들러
	VOID			OnLButtonDown( CPoint point );

	/// 마우스 가운데버튼 이벤트 핸들러
	VOID			OnMButtonDown( CPoint point );

	/// 마우스 오른쪽버튼 이벤트 핸들러
	VOID			OnRButtonDown( CPoint point );

	/// 마우스 왼쪽버튼 떼어짐 이벤트 핸들러
	VOID			OnLButtonUp( CPoint point );	

	/// 마우스 가운데버튼 떼어짐 이벤트 핸들러
	VOID			OnMButtonUp( CPoint point );

	/// 마우스 오른쪽버튼 떼어짐 이벤트 핸들러
	VOID			OnRButtonUp( CPoint point );

	/// 마우스 휠 이벤트 핸들러
	VOID			OnMouseWheel( short amount );
	
	/// 마우스 더블클릭 이벤트 핸들러
	VOID			OnLButtonDblClk( CPoint point );

	/// 키보드 눌림 핸들러
	BOOL			OnKeyDown( BYTE keycode );

	/// 키보드 떼어짐 핸들러
	BOOL			OnKeyUp( BYTE keycode );

	/// 컨텍스트 메뉴 핸들러
	VOID			OnContextMenu( CPoint point );

	/************************************************************************/
	/* 객체 메시 설정 관련 함수들                                           */
	/************************************************************************/

	/// 오브젝트 타입에 맞게 기본 메시 설정
	VOID			ChangeMeshToBasicModel( CtObjectBase *ptr );

	/// 오브젝트 메시를 클립아트에서 선택하는 모드
	VOID			ChangeMeshToClipartModel( CtObjectBase *ptr );

	/// 오브젝트 메시를 지정된 파일로 설정 
	VOID			ChangeMeshToCustomModel( CtObjectBase *ptr, LPCWSTR meshfilepath );

	/// 오브젝트 메시를 지정된 파일로 설정 
	VOID ChangeMeshToSelectedModel( CtObjectBase *ptr );

	/************************************************************************/
	/* 객체 3D 메시 모핑 관련                                               */
	/************************************************************************/

	/// 객체의 메시를 주어진 메시 파일로 모핑하는 함수.
	HRESULT			MorphMesh( CtObjectBase* pObj, LPCWSTR filepath );	

	/// 객체의 모핑용 버텍스 버퍼 생성 함수.
	HRESULT			CreateMeshVB( CtObjectBase* pObj, DWORD numVert );

	/// 객체의 모핑용 인덱스 버퍼 생성 함수.
	HRESULT			CreateMeshIB( CtObjectBase* pObj, DWORD numFace );

	/// 객체의 모핑용 버퍼 설정 함수.
	HRESULT			ResetBuffersFromMorphMesh( CtObjectBase* pObj );

	/************************************************************************/
	/* DXUT 콜백 핸들러 함수들                                              */
	/************************************************************************/

	/// 장치의 caps를 검사하여 설정한 환경대로 렌더링이 수행 가능한지 판단
	BOOL			IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
	
	/// 장치 셋팅이 변경될때의 핸들러
	BOOL			ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
	
	/// 장치 생성시의 핸들러
	HRESULT			OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
	
	/// 장치 리셋시의 핸들러
	HRESULT			OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
	
	/// 프레임 이동시 핸들러
	VOID			OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
	
	/// 프레임 렌더링시 핸들러
	VOID			OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
	
	/// 기본 메시지 핸들러
	LRESULT			MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
	
	/// 장치를 잃었을때의 핸들러
	VOID			OnLostDevice( void* pUserContext );
	
	/// 장치 파괴시의 핸들러
	VOID			OnDestroyDevice( void* pUserContext );
	
	/// CustomUI 이벤트 핸들러
	VOID			OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

	VOID OnMenu_ChangeBasicModel( VOID );

	VOID OnMenu_ChangeClipartModel( VOID );

	VOID OnMenu_ChangeCustomModel( VOID );

	VOID OnMenu_ChangeImageBasedModel( VOID );

	VOID OnMenu_SelectionPilingNormal( VOID );

	VOID OnMenu_SelectionPilingByType( VOID );

	VOID OnMenu_PilePopChip( VOID );

	VOID OnMenu_PileBreak( VOID );

	VOID OnMenu_MorphStart( VOID );

	VOID OnMenu_MorphSetBack( VOID );

	VOID OnMenu_FreezeObject( VOID );

	VOID OnMenu_UnFreezeObject( VOID );

	VOID OnMenu_BillboardObject( VOID );

	void OnMorphDivide();
	void OnMorphCut();
	void OnMorphEmbed();
};