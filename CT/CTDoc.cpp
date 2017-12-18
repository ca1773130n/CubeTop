// CTDoc.cpp : CCTDoc 클래스의 구현
//

#include "stdafx.h"
#include "CTDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCTDoc

IMPLEMENT_DYNCREATE(CCTDoc, CDocument)

BEGIN_MESSAGE_MAP(CCTDoc, CDocument)
END_MESSAGE_MAP()


// CCTDoc 생성/소멸

CCTDoc::CCTDoc()
{
	// TODO: 여기에 일회성 생성 코드를 추가합니다.

}

CCTDoc::~CCTDoc()
{
	
}

BOOL CCTDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 여기에 재초기화 코드를 추가합니다.
	// SDI 문서는 이 문서를 다시 사용합니다.

	return TRUE;
}




// CCTDoc serialization

void CCTDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 여기에 저장 코드를 추가합니다.
	}
	else
	{
		// TODO: 여기에 로딩 코드를 추가합니다.
	}
}


// CCTDoc 진단

#ifdef _DEBUG
void CCTDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCTDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CCTDoc 명령
