#pragma once


// CBuildDlg �Ի���

class CBuildDlg : public CDialog
{
	DECLARE_DYNAMIC(CBuildDlg)

public:
	CBuildDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CBuildDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_BUILD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
	

	afx_msg void OnBnClickedOk();
	CString m_strIP;
	CString m_strPort;
};
