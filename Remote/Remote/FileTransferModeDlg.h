#pragma once


// CFileTransferModeDlg �Ի���

class CFileTransferModeDlg : public CDialog
{
	DECLARE_DYNAMIC(CFileTransferModeDlg)

public:
	CFileTransferModeDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CFileTransferModeDlg();


	CString m_strFileName;
// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_TRANSMODE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg	void OnEndDialog(UINT id);
};
