// CTView.cpp : CCTView 클래스의 구현
//

#include "stdafx.h"
#include "CTDoc.h"
#include "CTView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CCTView

IMPLEMENT_DYNCREATE(CCTView, CView)

//BEGIN_MESSAGE_MAP(CCTView, CView)
//END_MESSAGE_MAP()
	


// CCTView 생성/소멸

CCTView::CCTView()
{
	// TODO: 여기에 생성 코드니다.

}

CCTView::~CCTView()
{
	//delete m_pCTmain;
}

BOOL CCTView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.
	
	return CView::PreCreateWindow(cs);
}

// CCTView 그리기

void CCTView::OnDraw(CDC* /*pDC*/)
{
	CCTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
}


// CCTView 진단

#ifdef _DEBUG
void CCTView::AssertValid() const
{
	CView::AssertValid();
}

void CCTView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCTDoc* CCTView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCTDoc)));
	return (CCTDoc*)m_pDocument;
}
#endif //_DEBUG

