#pragma once

#include "stdafx.h"
#include "Resource.h"

// CCubeTopMeshSelectDialog ��ȭ �����Դϴ�.
typedef class CCubeTopMeshSelectDialog CCubeTopMeshSelectDialog;

class CCubeTopMeshSelectDialog : public CDialog
{
	DECLARE_DYNAMIC(CCubeTopMeshSelectDialog)

public:

	WCHAR m_lpszHomeDirName[255];
	CTabCtrl *m_ctrlTab;
	CListBox *m_listBox;

	CCubeTopMeshSelectDialog(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CCubeTopMeshSelectDialog();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_MESHSELECTDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	afx_msg void OnSelchangeTabSelect(NMHDR* pNMHDR, LRESULT* pResult);
	
	DECLARE_MESSAGE_MAP()
public:	
	afx_msg void OnSelchangeListBox();
	afx_msg void OnLbnDblclkListbox();
public:
	afx_msg void OnBnClickedButton1();
};
