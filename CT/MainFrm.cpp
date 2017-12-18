// MainFrm.cpp : CMainFrame Ŭ������ ����
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


// CMainFrame ����/�Ҹ�

CMainFrame::CMainFrame()
{
	// TODO: ���⿡ ��� �ʱ�ȭ �ڵ带 �߰��մϴ�.
}

CMainFrame::~CMainFrame()
{
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: CREATESTRUCT cs�� �����Ͽ� ���⿡��
	//  Window Ŭ���� �Ǵ� ��Ÿ���� �����մϴ�.

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


// CMainFrame ����

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


// CMainFrame �޽��� ó����




int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  ���⿡ Ư��ȭ�� �ۼ� �ڵ带 �߰��մϴ�.

	CRect rect;
	SystemParametersInfo( SPI_GETWORKAREA, 0, &rect, 0 ); 
	if( __argc == 1 ) ::SetWindowPos( m_hWnd, HWND_DESKTOP, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER );
	else ::SetWindowPos( m_hWnd, 0, rect.left, rect.top, 384, 240, 0 );

	return 0;
}
