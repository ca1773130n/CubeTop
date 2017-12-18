// CubeTopMeshSelectDialog.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Resource.h"
#include "CT_Main.h"
#include "CT_MeshSelectDialog.h"
#include "CT_Globals.h"
// CCubeTopMeshSelectDialog ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CCubeTopMeshSelectDialog, CDialog)

CCubeTopMeshSelectDialog::CCubeTopMeshSelectDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCubeTopMeshSelectDialog::IDD, pParent)
{	
}

CCubeTopMeshSelectDialog::~CCubeTopMeshSelectDialog()
{
}

void CCubeTopMeshSelectDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCubeTopMeshSelectDialog, CDialog)		
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, OnSelchangeTabSelect)
	ON_LBN_SELCHANGE(IDC_LISTBOX, &CCubeTopMeshSelectDialog::OnSelchangeListBox)
	ON_LBN_DBLCLK(IDC_LISTBOX, &CCubeTopMeshSelectDialog::OnLbnDblclkListbox)	
	ON_BN_CLICKED(IDC_BUTTON1, &CCubeTopMeshSelectDialog::OnBnClickedButton1)	
END_MESSAGE_MAP()


// CCubeTopMeshSelectDialog �޽��� ó�����Դϴ�.

void CCubeTopMeshSelectDialog::OnSelchangeTabSelect(NMHDR* pNMHDR, LRESULT* pResult) 
{	
	CListBox *pListbox = (CListBox*)GetDlgItem( IDC_LISTBOX );

	int count = pListbox->GetCount();
	
	pListbox->ResetContent();			
	
	WCHAR tab_text[40]; 

	TC_ITEM data; 
	data.mask = TCIF_TEXT|TCIF_IMAGE; 
	data.pszText = tab_text; 
	data.cchTextMax = 39; 

	int selected = m_ctrlTab->GetCurSel();
	TRACE(_T("selected tab = %d\r\n"), selected);
	
	m_ctrlTab->GetItem( selected, &data );
	
	int ndx = 0;
	CFileFind finder;	
	SHFILEINFO sfi; 
	bool bWorking;

	WCHAR dirname[255];
	wsprintf( dirname, L"%s\\data\\models\\%s\\*.*", m_lpszHomeDirName, data.pszText );
	bWorking = finder.FindFile( dirname );

	while (bWorking)
	{
		// ���� model ���丮�� ������丮��(�� ī�װ�)�� Ž��
		bWorking = finder.FindNextFile(); 

		if(finder.IsDots()) continue;		

		SHGetFileInfo( 
			finder.GetFilePath(), 
			0, 
			&sfi, 
			sizeof(SHFILEINFO), 
			SHGFI_DISPLAYNAME 
			); 

		if( finder.IsDirectory() ) 
		{
			TCITEM ti;
			ti.mask = TCIF_TEXT;
			WCHAR tabname[255];

			wsprintf( tabname, finder.GetFileName() );

			ti.pszText = tabname;
			
			pListbox->AddString( tabname );
		}
	}

	*pResult = 0;		
}

void CCubeTopMeshSelectDialog::OnSelchangeListBox() 
{
	CListBox *pListbox = (CListBox*)GetDlgItem( IDC_LISTBOX );
	CString str;
	int nIndex = pListbox->GetCurSel();
	if( nIndex < 0 ) return;

	pListbox->GetText(nIndex, str);

	int ndx = 0;
	CFileFind finder;	
	SHFILEINFO sfi; 
	bool bWorking;

	WCHAR dirname[255], dirname2[255], filename[255];
	CTabCtrl *pTab = (CTabCtrl*)GetDlgItem( IDC_TAB );
	
	TCITEM ti;
	ti.pszText = filename;
	ti.mask = TCIF_TEXT|TCIF_IMAGE;
	ti.cchTextMax = 39; 
	int tabIndex = pTab->GetCurSel();
	pTab->GetItem( tabIndex, &ti );

	wsprintf( dirname, L"%s\\data\\models\\%s\\%s", m_lpszHomeDirName, ti.pszText, str );
	wsprintf( dirname2, L"%s*.*", dirname );
	bWorking = finder.FindFile( dirname2 );

	SetCurrentDirectory( dirname );

	while (bWorking)
	{
		// ���� model ���丮�� ������丮��(�� ī�װ�)�� Ž��
		bWorking = finder.FindNextFile(); 

		if(finder.IsDots()) continue;		

		SHGetFileInfo( 
			finder.GetFilePath(), 
			0, 
			&sfi, 
			sizeof(SHFILEINFO), 
			SHGFI_DISPLAYNAME 
		);

		if( lstrcmp( finder.GetFileName(), L"model.X") )
		{
			WCHAR fullpath[255];
			wsprintf( fullpath, L"%s\\model.X", dirname );
			GetCTmain()->ChangeMeshToCustomModel( GetCTmain()->m_pSelectedObj, fullpath );
		}
	}
}

void CCubeTopMeshSelectDialog::OnLbnDblclkListbox()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	//CListBox *pListbox = (CListBox*)GetDlgItem( IDC_LISTBOX );
	//CString str;
	//pListbox->GetText(pListbox->GetCurSel(), str);
	GetCTmain()->m_stStateFlags.bMeshSelectMode = FALSE;
	DestroyWindow();

	//MessageBox(str,0,0);
}

void CCubeTopMeshSelectDialog::OnBnClickedButton1()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	GetCTmain()->m_stStateFlags.bMeshSelectMode = FALSE;
	DestroyWindow();
}
