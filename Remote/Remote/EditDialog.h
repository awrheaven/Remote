#pragma once


// CEditDialog �Ի���

class CEditDialog : public CDialog
{
	DECLARE_DYNAMIC(CEditDialog)

public:
	CEditDialog(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CEditDialog();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_NEWFOLDER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnBnClickedOk();
	
	CString m_EditString;
};
