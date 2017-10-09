#pragma once
#include "Remote.h"
#include "afxwin.h"

// CSettingDlg �Ի���

class CSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CSettingDlg)

public:
	CSettingDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSettingDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SET };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	UINT m_nListenPort;
	UINT m_nMax_Connect;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonMsg();
	afx_msg void OnBnClickedButtonSettingapply();
	afx_msg void OnEnChangeEditPort();
	afx_msg void OnEnChangeEditMax();
	CButton m_ApplyButton;
};
