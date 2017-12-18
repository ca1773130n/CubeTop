#pragma once

#include "stdafx.h"
#include "CT_Settings.h"
#include "CT_Object.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// UI 컨트롤 IDs
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define IDC_HUD_BUTTON_TOGGLE					0
#define IDC_HUDARCBALL							1
#define IDC_HUD_ARCBALL							2
#define IDC_HUD_LISTBOX_FILELIST				3
#define IDC_HUD_TEXTBOX_FILEINFO				4
#define IDC_HUD_BUTTON_CAMERA					5
#define IDC_HUD_BUTTON_SEARCH					6
#define IDC_HUD_BUTTON_CLIPBD					7
#define IDC_HUD_BUTTON_SUBRGN					8
#define IDC_HUD_BUTTON_MTVIEW					9
#define IDC_HUD_BUTTON_ARRANG					10
#define IDC_HUD_BUTTON_PRIMIT					11
#define IDC_HUD_BUTTON_CONFIG					12
#define IDC_HUD_BUTTON_REWIND					13
#define IDC_HUD_BUTTON_FOWARD					14
#define IDC_HUD_BUTTON_PLAYPS					15
#define IDC_HUD_BUTTON_UPDIRE					16
#define IDC_HUD_BUTTON_HMDIRE					17
#define IDC_HUD_TEXT_PATHNAME					18
#define IDC_HUD_COMBOBOX_CAMERALIST				19

#define IDC_OI_STATIC_MATERIALINFO				20
#define IDC_OI_COMBOBOX_MATERIALLIST			21
#define IDC_OI_COLORBOX_AMBIENT					22
#define IDC_OI_STATIC_COLORBOX_AMBIENT_RGB		23
#define IDC_OI_COLORBOX_DIFFUSE					24
#define IDC_OI_STATIC_COLORBOX_DIFFUSE_RGB		25
#define IDC_OI_COLORBOX_SPECULAR				26
#define IDC_OI_STATIC_COLORBOX_SPECULAR_RGB		27
#define IDC_OI_STATIC_COLORBOX_AMBIENT			28
#define IDC_OI_STATIC_COLORBOX_DIFFUSE			29
#define IDC_OI_STATIC_COLORBOX_SPECULAR			30
#define IDC_OI_SLIDER_AMBIENT_R					31
#define IDC_OI_SLIDER_AMBIENT_G					32
#define IDC_OI_SLIDER_AMBIENT_B					33
#define IDC_OI_SLIDER_AMBIENT_A					34
#define IDC_OI_SLIDER_DIFFUSE_R					35
#define IDC_OI_SLIDER_DIFFUSE_G					36
#define IDC_OI_SLIDER_DIFFUSE_B					37
#define IDC_OI_SLIDER_DIFFUSE_A					38
#define IDC_OI_SLIDER_SPECULAR_R				39
#define IDC_OI_SLIDER_SPECULAR_G				40
#define IDC_OI_SLIDER_SPECULAR_B				41
#define IDC_OI_SLIDER_SPECULAR_A				42
#define IDC_OI_STATIC_SIZE						43
#define IDC_OI_STATIC_SIZE_XYZ					44
#define IDC_OI_SLIDER_SIZE_X					45
#define IDC_OI_SLIDER_SIZE_Y					46
#define IDC_OI_SLIDER_SIZE_Z					47
#define IDC_OI_STATIC_PHYSICS					48
#define IDC_OI_STATIC_PHYSICS_PROPERTIES		49
#define IDC_OI_SLIDER_PHYSICS_1					50
#define IDC_OI_SLIDER_PHYSICS_2					51
#define IDC_OI_SLIDER_PHYSICS_3					52

#define	IDC_CM_SELECTION						100
#define IDC_CM_SELECTION_PILING					101
#define IDC_CM_SELECTION_PILINGBYTYPE			102
#define IDC_CM_SELECTION_LINK					105
#define IDC_CM_SELECTION_UNLINK					106

#define IDC_CM_LOCK								103
#define IDC_CM_UNLOCK							104
#define IDC_CM_COPY								107
#define IDC_CM_CUT								108
#define IDC_CM_DELETE							109
#define IDC_CM_GRID								110

#define	IDC_CM_CHIP								200
#define	IDC_CM_POPCHIP							201
#define	IDC_CM_BREAKPILE						202
#define	IDC_CM_CUBE								300
#define IDC_CM_BROWSE							301
#define IDC_CM_MESH								302

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief CustomUI 클래스

 Direct3D 상에서 윈도우 컨트롤을 구현하기 위한 CustomUI를 관리하는 클래스이다.
 CustomUI와 관련된 모든 수행을 이 클래스에서 수행한다.
 컨트롤들의 텍스쳐는 기본 텍스쳐가 아닌 수정된 텍스쳐(dxutcontrols.dds)를 사용하며, DXUTgui.cpp 및 .h 파일도 튜닝되어 있다.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtCustomUI 
{
private:
	LPDIRECT3DDEVICE9			m_pDev;								///< D3D 장치 포인터
	UINT						m_iScreenWidth, m_iScreenHeight;	///< 화면 작업 영역의 가로 세로 크기
	UINT						m_iControlFrameBorderSize;			///< 컨트롤 프레임의 두께

	CDXUTDialog					m_dlgHUD;							///< 기본 HUD 다이얼로그
	CDXUTDialog					m_dlgFileList;			
	CDXUTDialog					m_dlgFileInfo;						///< 추가 브라우징 다이얼로그 - 파일 정보
	CDXUTDialog					m_dlgObjectInfo;					///< 추가 브라우징 다이얼로그 - 객체 정보	
	CDXUTDialogResourceManager	m_DRM;								///< 다이얼로그 리소스 매니져

	CDXUTContextMenu*				m_pContextMenuChip;					///< 칩 컨텍스트 메뉴 객체
	CDXUTContextMenu*				m_pContextMenuPiledChip;			///< 칩 컨텍스트 메뉴 객체
	CDXUTContextMenu*				m_pContextMenuCube;					///< 큐브 컨텍스트 메뉴 객체
	CDXUTContextMenu*				m_pContextMenuSelection;			///< 셀렉션 컨텍스트 메뉴 객체

	CDXUTTextHelper*			m_pTextHelper;						///< 텍스트 헬퍼
	CDXUTTextHelper*			m_pTextHelperBold;					///< 굵은 글씨용 

	BOOL						m_bBrowserOn;						///< 추가 브라우저 창 보이기 및 감추기 여부

public:
	CtCustomUI( VOID );
	~CtCustomUI( VOID );

	/// 초기화 함수
	VOID Initialize( LPDIRECT3DDEVICE9 pDev, LPDIRECT3DSTATEBLOCK9 pStateBlock, deque<CBaseCamera*>& cameras );

	/// 기본 HUD 다이얼로그 포인터를 반환
	CDXUTDialog*				GetHUD( VOID ) { return &m_dlgHUD; }

	/// 추가 브라우저 다이얼로그 포인터를 반환
	CDXUTDialog*				GetFL( VOID ) { return &m_dlgFileList; }

	/// 추가 브라우저 다이얼로그 - 파일정보 포인터를 반환
	CDXUTDialog*				GetFI( VOID ) { return &m_dlgFileInfo; }

	/// 추가 브라우저 다이얼로그 - 객체정보 포인터를 반환
	CDXUTDialog*				GetOI( VOID ) { return &m_dlgObjectInfo; }

	/// 텍스트 헬퍼 포인터를 반환
	CDXUTTextHelper*			GetTextHelper( VOID ) { return m_pTextHelper; }

	/// 추가 브라우저 토글
	VOID						ToggleBrowser( VOID );

	/// 오브젝트 선택 핸들러
	VOID						OnObjectSelection( CtObjectBase* pObj );
	
	/// 오브젝트 선택 해제 핸들러
	VOID						OnObjectSelectionClear( VOID );

	/// 렌더링 메인 함수
	VOID						OnRender( FLOAT fElapsedTime );

	/************************************************************************/
	/* 컨텍스트 메뉴 함수들                                                 */
	/************************************************************************/

	/// 컨텍스트 메뉴 보이기
	VOID						ShowContextMenu( CtObjectType type, POINT pt );

	/// 컨텍스트 메뉴 감추기
	VOID						HideContextMenu( CtObjectType type );

	/// 컨텍스트 메뉴 렌더링
	VOID						RenderContextMenu( FLOAT fElapsedTime );

	/// 메시지 핸들러
	LRESULT						MsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

	/// 제거 함수
	VOID						OnDestroyDevice( VOID );

};
