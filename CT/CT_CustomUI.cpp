#include "stdafx.h"
#include "CT_CustomUI.h"

/*************************************************************************************************************************************************************/
/* CustomUI 메인 클래스																																		 */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 생성자

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CtCustomUI::CtCustomUI( VOID )
{
	m_bBrowserOn = FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 소멸자

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CtCustomUI::~CtCustomUI( VOID )
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 초기화 함수

 메인 HUD 및 브라우저의 다이얼로그를 구성한다. 컨트롤들을 생성하여 배치시키고 초기화시킨다.
  
 * \param pDev			장치 포인터
 * \param pStateBlock	D3D State Block. RenderManager와 공유한다.
 * \param cameras		카메라 객체 리스트
 * \return				없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtCustomUI::Initialize( LPDIRECT3DDEVICE9 pDev, LPDIRECT3DSTATEBLOCK9 pStateBlock, deque<CBaseCamera*>& cameras )
{
	HRESULT hr;

	m_pDev = pDev;
	V( m_DRM.OnCreateDevice(m_pDev) );
	m_DRM.m_pStateBlock = pStateBlock;

	m_dlgHUD.Init( &m_DRM, TRUE, DEFAULT_CONTROLTEXTURE_FILENAME );
	m_dlgHUD.SetFont( 0, L"Tahoma", 14, FW_BOLD );
	m_dlgHUD.SetFont( 1, DEFAULT_ICONNAME_FONT, 12, FW_NORMAL );
	m_dlgFileList.Init( &m_DRM, TRUE, DEFAULT_CONTROLTEXTURE_FILENAME );
	m_dlgHUD.SetFont( 1, DEFAULT_ICONNAME_FONT, 12, FW_NORMAL );
	m_dlgFileInfo.Init( &m_DRM, TRUE, DEFAULT_CONTROLTEXTURE_FILENAME );
	m_dlgFileInfo.SetFont( 1, DEFAULT_ICONNAME_FONT, 12, FW_NORMAL );
	m_dlgObjectInfo.Init( &m_DRM, TRUE, DEFAULT_CONTROLTEXTURE_FILENAME );
	m_dlgObjectInfo.SetFont( 1, DEFAULT_ICONNAME_FONT, 12, FW_NORMAL );
	
	ID3DXFont* pFont;
	ID3DXSprite* pSprite;
	D3DXCreateSprite( m_pDev, &pSprite );	
	D3DXCreateFont( m_pDev, 15, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, DEFAULT_ICONNAME_FONT, &pFont );	

	m_pTextHelper = new CDXUTTextHelper( pFont, pSprite, 15 );	

	/************************************************************************/
	/* HUD 설정                                                             */
	/************************************************************************/
	CRect rect;
	SystemParametersInfo( SPI_GETWORKAREA, 0, &rect, 0 ); 
	m_iScreenWidth = rect.Width();
	m_iScreenHeight = rect.Height();	
	m_iControlFrameBorderSize = m_iScreenWidth * 0.0109375f; 	

	UINT sizeScreenMargin = 10;
	UINT sizeMainButtons = m_iScreenWidth * 0.0417f;
	UINT iSpacing = sizeMainButtons / 10.f;
	UINT sizeMainButtonSpacing = m_iControlFrameBorderSize * 0.8f;
		
	//////////////////////////////////////////////////////////////////////////
	// 브라우저 토글 
	//////////////////////////////////////////////////////////////////////////
	m_dlgHUD.AddImageButton( IDC_HUD_BUTTON_TOGGLE, sizeScreenMargin, sizeScreenMargin, sizeMainButtons, sizeMainButtons, iSpacing, m_iControlFrameBorderSize, DXUT_IBT_TOGGLE, 0, TRUE, TRUE, 0 );

	//////////////////////////////////////////////////////////////////////////
	// 시뮬레이션 컨트롤
	//////////////////////////////////////////////////////////////////////////
	UINT sizeSCButton = sizeMainButtons * 0.8f;
	UINT posSCButtonX = sizeScreenMargin + sizeMainButtons + sizeMainButtonSpacing;
	m_dlgHUD.AddImageButton( IDC_HUD_BUTTON_REWIND, posSCButtonX, sizeScreenMargin, sizeSCButton, sizeSCButton, iSpacing, m_iControlFrameBorderSize, DXUT_IBT_REWIND, 0, TRUE, FALSE, 0 );
	m_dlgHUD.AddImageButton( IDC_HUD_BUTTON_PLAYPS, posSCButtonX + 1 * (sizeSCButton + sizeMainButtonSpacing), sizeScreenMargin, sizeSCButton, sizeSCButton, iSpacing, m_iControlFrameBorderSize, DXUT_IBT_PLAYPS, 0, TRUE, TRUE, 0 );
	m_dlgHUD.AddImageButton( IDC_HUD_BUTTON_FOWARD, posSCButtonX + 2 * (sizeSCButton + sizeMainButtonSpacing), sizeScreenMargin, sizeSCButton, sizeSCButton, iSpacing, m_iControlFrameBorderSize, DXUT_IBT_FOWARD, 0, TRUE, FALSE, 0 );

	//////////////////////////////////////////////////////////////////////////
	// 디렉토리 변경 버튼(상위, 홈 디렉토리로 이동)	
	//////////////////////////////////////////////////////////////////////////
	m_dlgHUD.AddImageButton( IDC_HUD_BUTTON_UPDIRE, posSCButtonX + 3 * (sizeSCButton + sizeMainButtonSpacing), sizeScreenMargin, sizeSCButton, sizeSCButton, iSpacing, m_iControlFrameBorderSize, DXUT_IBT_UPDIRE, 0, TRUE, FALSE, 0 );
	m_dlgHUD.AddImageButton( IDC_HUD_BUTTON_HMDIRE, posSCButtonX + 4 * (sizeSCButton + sizeMainButtonSpacing), sizeScreenMargin, sizeSCButton, sizeSCButton, iSpacing, m_iControlFrameBorderSize, DXUT_IBT_HMDIRE, 0, TRUE, FALSE, 0 );

	//////////////////////////////////////////////////////////////////////////
	// 카메라 목록
	//////////////////////////////////////////////////////////////////////////
	UINT sizeComboBoxX = 160;
	UINT posComboBoxX = posSCButtonX + 5 * (sizeSCButton + sizeMainButtonSpacing);
	CDXUTComboBox* pComboBox;
	m_dlgHUD.AddComboBox( IDC_HUD_COMBOBOX_CAMERALIST, posComboBoxX, sizeScreenMargin, sizeComboBoxX, 26, m_iControlFrameBorderSize, 0, TRUE, &pComboBox );
	for( int i=0; i < cameras.size(); i++ ) 
		pComboBox->AddItem( cameras[i]->GetName(), 0 );

	//////////////////////////////////////////////////////////////////////////
	// 현재 디렉토리명 및 파일명
	//////////////////////////////////////////////////////////////////////////
	UINT sizePathTextbar = 26;
	UINT sizePathX = m_iScreenWidth - posComboBoxX - sizeComboBoxX - sizeMainButtonSpacing - sizeScreenMargin;
	m_dlgHUD.AddTextBox( IDC_HUD_TEXT_PATHNAME, posComboBoxX + sizeComboBoxX + sizeMainButtonSpacing, sizeScreenMargin, sizePathX, sizePathTextbar, 6, m_iControlFrameBorderSize, L"", D3DCOLOR_ARGB(200,0,0,0), DT_LEFT, 0 );

	//////////////////////////////////////////////////////////////////////////
	// 카메라 조절용 ArcBall
	//////////////////////////////////////////////////////////////////////////
	UINT sizeArcBall = m_iScreenWidth * 0.0625f;	
	UINT posArcBallStartX = m_iScreenWidth / 2.f - sizeArcBall / 2.f;
	UINT posArcBallStartY = m_iScreenHeight - sizeArcBall - sizeScreenMargin;
	//m_dlgHUD.AddArcBall( IDC_HUD_ARCBALL, posArcBallStartX, posArcBallStartY, sizeArcBall, sizeArcBall, 0 );	

	//////////////////////////////////////////////////////////////////////////
	// 메인 버튼들
	//////////////////////////////////////////////////////////////////////////
	UINT posMainButtonsY = posArcBallStartY + (sizeArcBall - sizeMainButtons) / 2.f;
	/*
	m_dlgHUD.AddImageButton( IDC_HUD_BUTTON_CAMERA, posArcBallStartX - 4 * (sizeMainButtons + sizeMainButtonSpacing), posMainButtonsY, sizeMainButtons, sizeMainButtons, iSpacing, m_iControlFrameBorderSize, DXUT_IBT_CAMERA, 0, TRUE, FALSE, 0 );
	m_dlgHUD.AddImageButton( IDC_HUD_BUTTON_SEARCH, posArcBallStartX - 3 * (sizeMainButtons + sizeMainButtonSpacing), posMainButtonsY, sizeMainButtons, sizeMainButtons, iSpacing, m_iControlFrameBorderSize, DXUT_IBT_SEARCH, 0, TRUE, FALSE, 0 );
	m_dlgHUD.AddImageButton( IDC_HUD_BUTTON_CLIPBD, posArcBallStartX - 2 * (sizeMainButtons + sizeMainButtonSpacing), posMainButtonsY, sizeMainButtons, sizeMainButtons, iSpacing, m_iControlFrameBorderSize, DXUT_IBT_CLIPBD, 0, TRUE, FALSE, 0 );
	m_dlgHUD.AddImageButton( IDC_HUD_BUTTON_MTVIEW, posArcBallStartX - (sizeMainButtons + sizeMainButtonSpacing), posMainButtonsY, sizeMainButtons, sizeMainButtons, iSpacing, m_iControlFrameBorderSize, DXUT_IBT_MTVIEW, 0, TRUE, FALSE, 0 );
	m_dlgHUD.AddImageButton( IDC_HUD_BUTTON_SUBRGN, posArcBallStartX + sizeArcBall + sizeMainButtonSpacing, posMainButtonsY, sizeMainButtons, sizeMainButtons, iSpacing, m_iControlFrameBorderSize, DXUT_IBT_SUBRGN, 0, TRUE, FALSE, 0 );
	m_dlgHUD.AddImageButton( IDCcc_HUD_BUTTON_ARRANG, posArcBallStartX + sizeArcBall + sizeMainButtonSpacing + (sizeMainButtons + sizeMainButtonSpacing), posMainButtonsY, sizeMainButtons, sizeMainButtons, iSpacing, m_iControlFrameBorderSize, DXUT_IBT_ARRANG, 0, TRUE, FALSE, 0 );
	m_dlgHUD.AddImageButton( IDC_HUD_BUTTON_PRIMIT, posArcBallStartX + sizeArcBall + sizeMainButtonSpacing + 2 * (sizeMainButtons + sizeMainButtonSpacing), posMainButtonsY, sizeMainButtons, sizeMainButtons, iSpacing, m_iControlFrameBorderSize, DXUT_IBT_PRIMIT, 0, TRUE, FALSE, 0 );
	m_dlgHUD.AddImageButton( IDC_HUD_BUTTON_CONFIG, posArcBallStartX + sizeArcBall + sizeMainButtonSpacing + 3 * (sizeMainButtons + sizeMainButtonSpacing), posMainButtonsY, sizeMainButtons, sizeMainButtons, iSpacing, m_iControlFrameBorderSize, DXUT_IBT_CONFIG, 0, TRUE, FALSE, 0 );
	*/

	/************************************************************************/
	/* 객체 정보                                                            */
	/************************************************************************/
	UINT sizeSCButtonSpacing = m_iScreenWidth * 0.002f + 5;
	UINT posURButtonX = m_iScreenWidth - sizeScreenMargin - sizeSCButton * 2 - sizeMainButtonSpacing;

	UINT sizeObjectInfoX = m_iScreenWidth * 0.15f;
	UINT sizeObjectInfoY = 326;
	UINT posObjectInfoX = m_iScreenWidth - sizeScreenMargin - sizeObjectInfoX;
	UINT posObjectInfoY = m_iScreenHeight - sizeScreenMargin - sizeObjectInfoY - (m_iScreenHeight - posMainButtonsY - sizeMainButtons);

	m_dlgObjectInfo.SetSize( sizeObjectInfoX, sizeObjectInfoY );
	m_dlgObjectInfo.SetLocation( posObjectInfoX, posObjectInfoY );

	//////////////////////////////////////////////////////////////////////////
	// 색상 정보 텍스트
	//////////////////////////////////////////////////////////////////////////
	//m_dlgObjectInfo.AddStatic( IDC_OI_STATIC_MATERIALINFO, L"색상 정보", iSpacing, iSpacing, 100, 20, TRUE, 0 );
	
	//////////////////////////////////////////////////////////////////////////
	// 색상 리스트 콤보박스
	//////////////////////////////////////////////////////////////////////////
	m_dlgObjectInfo.AddComboBox( IDC_OI_COMBOBOX_MATERIALLIST, iSpacing, iSpacing + 20, sizeObjectInfoX - 2 * iSpacing, 20, m_iControlFrameBorderSize, 0, TRUE, 0 );
	m_dlgObjectInfo.SetCaptionText( L"Object Attributes" );
	m_dlgObjectInfo.EnableCaption( TRUE );

	//////////////////////////////////////////////////////////////////////////
	// ambient, diffuse, specular
	//////////////////////////////////////////////////////////////////////////
	UINT sizeSlider = sizeObjectInfoX - 71;
	UINT sizeStatic = sizeObjectInfoX - 2 * iSpacing;
	UINT posColorSetAmbient = iSpacing + 48;	
	m_dlgObjectInfo.AddStatic( IDC_OI_STATIC_COLORBOX_AMBIENT, L"Ambient", iSpacing, posColorSetAmbient, sizeStatic, 12, 1, D3DCOLOR_ARGB(255, 50, 50, 50), 0, 0, FALSE, 0 );	
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_AMBIENT_R, iSpacing, posColorSetAmbient + 12, sizeSlider, 9, 0, 100, 100, FALSE, 0 );
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_AMBIENT_G, iSpacing, posColorSetAmbient + 24, sizeSlider, 9, 0, 100, 100, FALSE, 0 );
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_AMBIENT_B, iSpacing, posColorSetAmbient + 36, sizeSlider, 9, 0, 100, 100, FALSE, 0 );
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_AMBIENT_A, iSpacing, posColorSetAmbient + 48, sizeSlider, 9, 0, 100, 100, FALSE, 0 );
	m_dlgObjectInfo.AddStatic( IDC_OI_STATIC_COLORBOX_AMBIENT_RGB, L"R\nG\nB\nA", iSpacing + sizeSlider + 6, posColorSetAmbient + 11, 10, 48, 1, D3DCOLOR_ARGB(255, 80, 80, 80), 0, 0, FALSE, 0 );
	m_dlgObjectInfo.AddColorBox( IDC_OI_COLORBOX_AMBIENT, iSpacing + sizeObjectInfoX - 55, posColorSetAmbient + 12, 45, 45, D3DCOLOR_ARGB(255,255,255,255), 0 );
	UINT posColorSetDiffuse = iSpacing + 106 ;
	m_dlgObjectInfo.AddStatic( IDC_OI_STATIC_COLORBOX_DIFFUSE, L"Diffuse", iSpacing, posColorSetDiffuse, sizeStatic, 12, 1, D3DCOLOR_ARGB(255, 50, 50, 50), 0, 0, FALSE, 0 );	
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_DIFFUSE_R, iSpacing, posColorSetDiffuse + 12, sizeSlider, 9, 0, 100, 100, FALSE, 0 );
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_DIFFUSE_G, iSpacing, posColorSetDiffuse + 24, sizeSlider, 9, 0, 100, 100, FALSE, 0 );
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_DIFFUSE_B, iSpacing, posColorSetDiffuse + 36, sizeSlider, 9, 0, 100, 100, FALSE, 0 );
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_DIFFUSE_A, iSpacing, posColorSetDiffuse + 48, sizeSlider, 9, 0, 100, 100, FALSE, 0 );
	m_dlgObjectInfo.AddStatic( IDC_OI_STATIC_COLORBOX_DIFFUSE_RGB, L"R\nG\nB\nA", iSpacing + sizeSlider + 6, posColorSetDiffuse + 11, 10, 48, 1, D3DCOLOR_ARGB(255, 80, 80, 80), 0, 0, FALSE, 0 );
	m_dlgObjectInfo.AddColorBox( IDC_OI_COLORBOX_DIFFUSE, iSpacing + sizeObjectInfoX - 55, posColorSetDiffuse + 12, 45, 45, D3DCOLOR_ARGB(255,255,255,255), 0 );
	UINT posColorSetSpecular = iSpacing + 164;
	m_dlgObjectInfo.AddStatic( IDC_OI_STATIC_COLORBOX_SPECULAR, L"Specular", iSpacing, posColorSetSpecular, sizeStatic, 12, 1, D3DCOLOR_ARGB(255, 50, 50, 50), 0, 0, FALSE, 0 );	
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_SPECULAR_R, iSpacing, posColorSetSpecular + 12, sizeSlider, 9, 0, 100, 100, FALSE, 0 );
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_SPECULAR_G, iSpacing, posColorSetSpecular + 24, sizeSlider, 9, 0, 100, 100, FALSE, 0 );
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_SPECULAR_B, iSpacing, posColorSetSpecular + 36, sizeSlider, 9, 0, 100, 100, FALSE, 0 );
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_SPECULAR_A, iSpacing, posColorSetSpecular + 48, sizeSlider, 9, 0, 100, 100, FALSE, 0 );
	m_dlgObjectInfo.AddStatic( IDC_OI_STATIC_COLORBOX_SPECULAR_RGB, L"R\nG\nB\nA", iSpacing + sizeSlider + 6, posColorSetSpecular + 11, 10, 48, 1, D3DCOLOR_ARGB(255, 80, 80, 80), 0, 0, FALSE, 0 );
	m_dlgObjectInfo.AddColorBox( IDC_OI_COLORBOX_SPECULAR, iSpacing + sizeObjectInfoX - 55, posColorSetSpecular + 12, 45, 45, D3DCOLOR_ARGB(255,255,255,255), 0 );

	//////////////////////////////////////////////////////////////////////////
	// 사이즈 
	//////////////////////////////////////////////////////////////////////////
	UINT posSize = posColorSetSpecular + 58;
	UINT sizeSizeSlider = sizeObjectInfoX - 21;
	m_dlgObjectInfo.AddStatic( IDC_OI_STATIC_SIZE, L"Object Size", iSpacing, posSize, sizeStatic, 12, 1, D3DCOLOR_ARGB(255, 50, 50, 50), 0, 0, FALSE, 0 );	
	m_dlgObjectInfo.AddStatic( IDC_OI_STATIC_SIZE_XYZ, L"X\nY\nZ", iSpacing + sizeSizeSlider + 4, posSize + 11, 7, 36, 1, D3DCOLOR_ARGB(255, 80, 80, 80), 0, 0, FALSE, 0 );
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_SIZE_X, iSpacing, posSize + 12, sizeSizeSlider, 9, 1, 100, 50, FALSE, 0 );
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_SIZE_Y, iSpacing, posSize + 24, sizeSizeSlider, 9, 1, 100, 50, FALSE, 0 );
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_SIZE_Z, iSpacing, posSize + 36, sizeSizeSlider, 9, 1, 100, 50, FALSE, 0 );

	//////////////////////////////////////////////////////////////////////////
	// 질량, 마찰력, 반발력
	//////////////////////////////////////////////////////////////////////////
	UINT posPhysics = posSize + 48;
	UINT sizePhysicsSlider = sizeObjectInfoX - iSpacing * 2 - 51;
	m_dlgObjectInfo.AddStatic( IDC_OI_STATIC_PHYSICS, L"Physics Properties", iSpacing, posPhysics, sizeStatic, 12, 1, D3DCOLOR_ARGB(255, 50, 50, 50), 0, 0, FALSE, 0 );	
	m_dlgObjectInfo.AddStatic( IDC_OI_STATIC_PHYSICS_PROPERTIES, L"Mass\nFriction\nRestitution", iSpacing + sizePhysicsSlider + 4, posPhysics + 11, 46, 34, 1, D3DCOLOR_ARGB(255, 50, 50, 50), DT_RIGHT, 0, FALSE, 0 );	
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_PHYSICS_1, iSpacing, posPhysics + 12, sizePhysicsSlider, 9, 1, 100, 50, FALSE, 0 );
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_PHYSICS_2, iSpacing, posPhysics + 24, sizePhysicsSlider, 9, 1, 100, 50, FALSE, 0 );
	m_dlgObjectInfo.AddSlider( IDC_OI_SLIDER_PHYSICS_3, iSpacing, posPhysics + 36, sizePhysicsSlider, 9, 1, 100, 50, FALSE, 0 );
	
	/************************************************************************/
	/* 파일 정보                                                            */
	/************************************************************************/
	UINT sizeFileInfoX = sizeObjectInfoX;
	UINT sizeFileInfoY = 260;
	UINT posFileInfoX = posObjectInfoX;
	UINT posFileInfoY = posObjectInfoY - sizeFileInfoY;

	m_dlgFileInfo.SetSize( sizeFileInfoX, sizeFileInfoY );
	m_dlgFileInfo.SetLocation( posFileInfoX, posFileInfoY );
	
	/************************************************************************/
	/* 파일리스트                                                           */
	/************************************************************************/
	UINT sizeListBoxX = sizeObjectInfoX;	
	UINT sizeListBoxY = posFileInfoY - sizeScreenMargin - sizePathTextbar - 2 * sizeMainButtonSpacing;
	UINT posListBoxX = m_iScreenWidth - sizeScreenMargin - sizeListBoxX;
	UINT posListBoxY = posFileInfoY - sizeListBoxY - sizeMainButtonSpacing;	
	
	CDXUTListBox* pListBox;
	m_dlgFileList.AddListBox( IDC_HUD_LISTBOX_FILELIST, posListBoxX, posListBoxY, sizeListBoxX, sizeListBoxY, m_iControlFrameBorderSize, CDXUTListBox::MULTISELECTION, &pListBox );	
	pListBox->SetVisible( FALSE );

	/************************************************************************/
	/* 컨텍스트 메뉴 관련                                                   */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	// 셀렉션 컨텍스트 메뉴 관련
	//////////////////////////////////////////////////////////////////////////	
	m_dlgHUD.AddContextMenu( IDC_CM_SELECTION, L"Selection", 0, TRUE, &m_pContextMenuSelection );
	m_dlgHUD.AddContextMenu( IDC_CM_CHIP, L"Chip", 0, TRUE, &m_pContextMenuChip );
	m_dlgHUD.AddContextMenu( IDC_CM_CUBE, L"PiledChip", 0, TRUE, &m_pContextMenuPiledChip );
	m_dlgHUD.AddContextMenu( IDC_CM_CUBE, L"Cube", 0, TRUE, &m_pContextMenuCube );

	// 배경 원
	CDXUTCircleImageButton *pCIButton, *pCIButtonBG;
	//m_dlgHUD.AddCircleImageButton( IDC_CM_BACKGROUND, 0, 0, 0, 0, m_iControlFrameBorderSize, DXUT_CIBT_CM_BACKGROUND, 0, FALSE, TRUE, &pCIButtonBG );
	//m_ContextMenuSelection.AddControl( pCIButtonBG );
	//pCIButtonBG->SetVisible( FALSE );

	int circleButtonSize = sizeMainButtons * 0.3f;
	float fRadius = sizeMainButtons * 0.3f;
	int iCircleSpacing = 1;
	// 메뉴 버튼들	
	m_dlgHUD.AddCircleImageButton( IDC_CM_SELECTION_PILING, circleButtonSize, circleButtonSize, fRadius, iCircleSpacing, m_iControlFrameBorderSize, DXUT_CIBT_PILING, 0, TRUE, FALSE, &pCIButton );
	m_pContextMenuSelection->AddControl( pCIButton );
	pCIButton->SetVisible( FALSE );	
	m_dlgHUD.AddCircleImageButton( IDC_CM_SELECTION_PILINGBYTYPE, circleButtonSize, circleButtonSize, fRadius, iCircleSpacing, m_iControlFrameBorderSize, DXUT_CIBT_PILINGBYTYPES, 0, TRUE, FALSE, &pCIButton );
	m_pContextMenuSelection->AddControl( pCIButton );	
	pCIButton->SetVisible( FALSE );
	m_dlgHUD.AddCircleImageButton( IDC_CM_LOCK, circleButtonSize, circleButtonSize, fRadius, iCircleSpacing, m_iControlFrameBorderSize, DXUT_CIBT_LOCK, 0, TRUE, FALSE, &pCIButton );
	m_pContextMenuSelection->AddControl( pCIButton );	
	m_pContextMenuChip->AddControl( pCIButton );
	m_pContextMenuCube->AddControl( pCIButton );
	pCIButton->SetVisible( FALSE );
	m_dlgHUD.AddCircleImageButton( IDC_CM_UNLOCK, circleButtonSize, circleButtonSize, fRadius, iCircleSpacing, m_iControlFrameBorderSize, DXUT_CIBT_UNLOCK, 0, TRUE, FALSE, &pCIButton );
	m_pContextMenuSelection->AddControl( pCIButton );	
	m_pContextMenuChip->AddControl( pCIButton );
	m_pContextMenuCube->AddControl( pCIButton );
	pCIButton->SetVisible( FALSE );
	m_dlgHUD.AddCircleImageButton( IDC_CM_SELECTION_LINK, circleButtonSize, circleButtonSize, fRadius, iCircleSpacing, m_iControlFrameBorderSize, DXUT_CIBT_LINK, 0, TRUE, FALSE, &pCIButton );
	m_pContextMenuSelection->AddControl( pCIButton );	
	pCIButton->SetVisible( FALSE );
	m_dlgHUD.AddCircleImageButton( IDC_CM_SELECTION_UNLINK, circleButtonSize, circleButtonSize, fRadius, iCircleSpacing, m_iControlFrameBorderSize, DXUT_CIBT_UNLINK, 0, TRUE, FALSE, &pCIButton );
	m_pContextMenuSelection->AddControl( pCIButton );	
	pCIButton->SetVisible( FALSE );
	m_dlgHUD.AddCircleImageButton( IDC_CM_COPY, circleButtonSize, circleButtonSize, fRadius, iCircleSpacing, m_iControlFrameBorderSize, DXUT_CIBT_COPY, 0, TRUE, FALSE, &pCIButton );
	m_pContextMenuSelection->AddControl( pCIButton );	
	m_pContextMenuChip->AddControl( pCIButton );
	m_pContextMenuCube->AddControl( pCIButton );
	pCIButton->SetVisible( FALSE );
	m_dlgHUD.AddCircleImageButton( IDC_CM_CUT, circleButtonSize, circleButtonSize, fRadius, iCircleSpacing, m_iControlFrameBorderSize, DXUT_CIBT_CUT, 0, TRUE, FALSE, &pCIButton );
	m_pContextMenuSelection->AddControl( pCIButton );	
	m_pContextMenuChip->AddControl( pCIButton );
	m_pContextMenuCube->AddControl( pCIButton );
	pCIButton->SetVisible( FALSE );
	m_dlgHUD.AddCircleImageButton( IDC_CM_DELETE, circleButtonSize, circleButtonSize, fRadius, iCircleSpacing, m_iControlFrameBorderSize, DXUT_CIBT_DELETE, 0, TRUE, FALSE, &pCIButton );
	m_pContextMenuSelection->AddControl( pCIButton );	
	m_pContextMenuChip->AddControl( pCIButton );
	m_pContextMenuCube->AddControl( pCIButton );
	pCIButton->SetVisible( FALSE );

	m_dlgHUD.AddCircleImageButton( IDC_CM_BROWSE, circleButtonSize, circleButtonSize, fRadius, iCircleSpacing, m_iControlFrameBorderSize, DXUT_IBT_SEARCH, 0, TRUE, FALSE, &pCIButton );
	m_pContextMenuCube->AddControl( pCIButton );
	pCIButton->SetVisible( FALSE );
	m_dlgHUD.AddCircleImageButton( IDC_CM_GRID, circleButtonSize, circleButtonSize, fRadius, iCircleSpacing, m_iControlFrameBorderSize, DXUT_IBT_SUBRGN, 0, TRUE, FALSE, &pCIButton );
	m_pContextMenuSelection->AddControl( pCIButton );
	m_pContextMenuPiledChip->AddControl( pCIButton );
	pCIButton->SetVisible( FALSE );

	m_dlgHUD.AddCircleImageButton( IDC_CM_POPCHIP, circleButtonSize, circleButtonSize, fRadius, iCircleSpacing, m_iControlFrameBorderSize, DXUT_CIBT_UNLINK, 0, TRUE, FALSE, &pCIButton );
	m_pContextMenuPiledChip->AddControl( pCIButton );
	pCIButton->SetVisible( FALSE );
	m_dlgHUD.AddCircleImageButton( IDC_CM_BREAKPILE, circleButtonSize, circleButtonSize, fRadius, iCircleSpacing, m_iControlFrameBorderSize, DXUT_CIBT_BREAKPILE, 0, TRUE, FALSE, &pCIButton );
	m_pContextMenuPiledChip->AddControl( pCIButton );
	pCIButton->SetVisible( FALSE );

	m_dlgHUD.AddCircleImageButton( IDC_CM_MESH, circleButtonSize, circleButtonSize, fRadius, iCircleSpacing, m_iControlFrameBorderSize, DXUT_CIBT_MESH, 0, TRUE, FALSE, &pCIButton );
	m_pContextMenuChip->AddControl( pCIButton );
	pCIButton->SetVisible( FALSE );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 브라우저 토글 함수

 추가 브라우저 다이얼로그를 켜고 끈다

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID 
CtCustomUI::ToggleBrowser( VOID )
{
	m_bBrowserOn = !m_bBrowserOn; 
	m_dlgFileList.GetListBox( IDC_HUD_LISTBOX_FILELIST )->SetVisible( m_bBrowserOn );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 선택 핸들러 함수

 객체를 선택할 때 선택된 객체들을 추가 브라우저의 파일 리스트 박스에 표시한다.

 * \param pObj	객체 포인터
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtCustomUI::OnObjectSelection( CtObjectBase* pObj )
{
	CDXUTListBox* pListBox = m_dlgFileList.GetListBox( IDC_HUD_LISTBOX_FILELIST );	

	CString strFileName = pObj->GetFileInfo()->lpszFileName;

	DXUTListBoxItem** pItems = pListBox->GetItemArray()->GetData();	
	for( DWORD i=0; i < pListBox->GetItemArray()->GetSize(); i++ )
	{
		if( lstrcmpW(pItems[i]->strText, strFileName) == 0 ) 
		{
			pListBox->SelectItem( i );
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 선택을 해제할때의 핸들러 함수

 객체들을 선택 해제할 때 추가 브라우저의 파일 리스트 박스에서 표시를 해제함

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID			
CtCustomUI::OnObjectSelectionClear( VOID )
{
	CDXUTListBox* pListBox = m_dlgFileList.GetListBox( IDC_HUD_LISTBOX_FILELIST );	
	pListBox->UnSelectAll();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 메인 렌더링 함수

 CustomUI를 구성하는 모든 다이얼로그를 렌더링한다.

 * \param fElapsedTime	경과 시간
 * \return				없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID						
CtCustomUI::OnRender( FLOAT fElapsedTime )
{
	//////////////////////////////////////////////////////////////////////////
	// 동적 HUD 설정
	//////////////////////////////////////////////////////////////////////////
	m_dlgHUD.GetTextBox( IDC_HUD_TEXT_PATHNAME )->SetText( GetCTmain()->m_pCurrentCube->GetFilePath() );

	

	// 선택물이 있는 경우
	/*
	if( GetCTmain()->m_pHoveredObj )
	{
		// 파일 정보 박스
		TCHAR fulltext[1024];
		CDXUTTextBox* pTextBox = m_dlgHUD.GetTextBox( IDC_HUD_TEXTBOX_FILEINFO );
		stFileInfo* pFileInfo = &pCTmain->m_pHoveredObj->m_stFileInfo;
		
		CFile file( CString(pFileInfo->lpszFilePath) + CString(pFileInfo->lpszFileName), CFile::modeRead );
		wsprintfW( fulltext, L"파일명\n%s\n\n파일 종류\n%s\n\n파일 크기\n%d 바이트\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\n", pFileInfo->lpszFileName, pFileInfo->lpszFileType, file.GetLength() );
		
		pTextBox->SetText( fulltext );
	}
	*/
	
	
	//////////////////////////////////////////////////////////////////////////
	// 렌더링
	//////////////////////////////////////////////////////////////////////////
	m_dlgHUD.OnRender( fElapsedTime );
	
	if( m_bBrowserOn )
	{
		m_dlgObjectInfo.OnRender( fElapsedTime, TRUE, m_iControlFrameBorderSize );
		m_dlgFileInfo.OnRender( fElapsedTime, TRUE, m_iControlFrameBorderSize );		
		m_dlgFileList.OnRender( fElapsedTime, TRUE, m_iControlFrameBorderSize );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 컨텍스트 메뉴 보이기 함수

 지정한 타입의 컨텍스트 메뉴를 보이도록 설정한다.

 * \param type	컨텍스트 메뉴 종류
 * \param pt	표시할 2D 좌표
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtCustomUI::ShowContextMenu( CtObjectType type, POINT pt )
{
	CDXUTContextMenu* pContextMenu = 0;

	switch( type )
	{
	case OBJECT_TYPE_CHIP:
		{
			pContextMenu = m_pContextMenuChip;
			break;
		}
	case OBJECT_TYPE_CUBE:
		{
			pContextMenu = m_pContextMenuCube;
			break;
		}
	case OBJECT_TYPE_SELECTION:
		{
			pContextMenu = m_pContextMenuSelection;
			break;
		}
	case OBJECT_TYPE_PILE:
		{
			pContextMenu = m_pContextMenuPiledChip;
			break;
		}
	}		

	pContextMenu->Show( pt );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 컨텍스트 메뉴 감추기 함수

 컨텍스트 메뉴를 숨긴다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID 
CtCustomUI::HideContextMenu( CtObjectType type )
{
	CDXUTContextMenu* pContextMenu = 0;

	switch( type )
	{
	case OBJECT_TYPE_CHIP:
		{
			pContextMenu = m_pContextMenuChip;
			break;
		}
	case OBJECT_TYPE_CUBE:
		{
			pContextMenu = m_pContextMenuCube;
			break;
		}
	case OBJECT_TYPE_SELECTION:
		{
			pContextMenu = m_pContextMenuSelection;
			break;
		}
	case OBJECT_TYPE_PILE:
		{
			pContextMenu = m_pContextMenuPiledChip;
			break;
		}
	}		

	pContextMenu->SetVisible( FALSE );
	pContextMenu->Hide();	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 컨텍스트 메뉴 렌더링 함수

 보이도록 설정된 컨텍스트 메뉴를 렌더링한다.

 * \param fElapsedTime	경과 시간
 * \return				없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID						
CtCustomUI::RenderContextMenu( FLOAT fElapsedTime )
{
	//if( m_pContextMenuSelection->IsVisible() )	m_pContextMenuSelection->Render( fElapsedTime);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 메인 메시지 핸들링 함수

 전달받은 메시지를 우선순위에 따라 다이얼로그에 전달한다.

 * \param uMsg		메시지
 * \param wParam	WPARAM
 * \param lParam	LPARAM
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
CtCustomUI::MsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{	
	bool bNoFurtherProcess;
	HWND hWnd = AfxGetMainWnd()->m_hWnd;

	if( (bNoFurtherProcess = m_DRM.MsgProc(hWnd, uMsg, wParam, lParam)) == TRUE ) return 1;
	if( m_bBrowserOn )
	{
		if( (bNoFurtherProcess = m_dlgFileList.MsgProc(hWnd, uMsg, wParam, lParam)) == TRUE ) return 1;
		if( (bNoFurtherProcess = m_dlgFileInfo.MsgProc(hWnd, uMsg, wParam, lParam)) == TRUE ) return 1;
		if( (bNoFurtherProcess = m_dlgObjectInfo.MsgProc(hWnd, uMsg, wParam, lParam)) == TRUE ) return 1;
	}
	if( (bNoFurtherProcess = m_dlgHUD.MsgProc(hWnd, uMsg, wParam, lParam)) == TRUE ) return 1;
	
	if( m_pContextMenuSelection && m_pContextMenuSelection->m_bVisible ) 
		m_pContextMenuSelection->MsgProc( uMsg, wParam, lParam );

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 장치가 파괴될 때의 핸들링 함수

 동적 다이얼로그 및 컨트롤을 모두 해제하고 필요한 추가 소멸 작업을 수행한다.
 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtCustomUI::OnDestroyDevice( VOID )
{
	m_DRM.OnDestroyDevice();
	m_dlgHUD.RemoveAllControls();
	m_dlgFileInfo.RemoveAllControls();
	m_dlgObjectInfo.RemoveAllControls();
}

