// MainFrm.cpp : CMainFrame 클래스의 구현
//

#include "stdafx.h"
#include "CT.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CMainFrame 생성/소멸

CMainFrame::CMainFrame()
{
	// TODO: 여기에 멤버 초기화 코드를 추가합니다.
}

CMainFrame::~CMainFrame()
{
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	cs.style = 0
		;

	if( cs.hMenu )
	{		
		::DestroyMenu( cs.hMenu );
		cs.hMenu = NULL;
	}

	return TRUE;
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{ 
	BOOL bRet = CFrameWnd::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext);

	if(bRet)
	{
		ModifyStyle(WS_CAPTION, 0, SWP_FRAMECHANGED);
		ModifyStyle(WS_THICKFRAME, 0, SWP_FRAMECHANGED);
	}
	return bRet;
} 


// CMainFrame 진단

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame 메시지 처리기




int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.

	CRect rect;
	SystemParametersInfo( SPI_GETWORKAREA, 0, &rect, 0 ); 
	if( __argc == 1 ) ::SetWindowPos( m_hWnd, HWND_DESKTOP, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER );
	else ::SetWindowPos( m_hWnd, 0, rect.left, rect.top, 384, 240, 0 );

	return 0;
}
