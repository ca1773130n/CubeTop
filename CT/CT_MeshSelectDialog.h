#pragma once

#include "stdafx.h"
#include "Resource.h"

// CCubeTopMeshSelectDialog 대화 상자입니다.
typedef class CCubeTopMeshSelectDialog CCubeTopMeshSelectDialog;

class CCubeTopMeshSelectDialog : public CDialog
{
	DECLARE_DYNAMIC(CCubeTopMeshSelectDialog)

public:

	WCHAR m_lpszHomeDirName[255];
	CTabCtrl *m_ctrlTab;
	CListBox *m_listBox;

	CCubeTopMeshSelectDialog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CCubeTopMeshSelectDialog();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_MESHSELECTDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	afx_msg void OnSelchangeTabSelect(NMHDR* pNMHDR, LRESULT* pResult);
	
	DECLARE_MESSAGE_MAP()
public:	
	afx_msg void OnSelchangeListBox();
	afx_msg void OnLbnDblclkListbox();
public:
	afx_msg void OnBnClickedButton1();
};
