// FileTransferModeDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Remote.h"
#include "FileTransferModeDlg.h"
#include "afxdialogex.h"


// CFileTransferModeDlg �Ի���

IMPLEMENT_DYNAMIC(CFileTransferModeDlg, CDialog)

CFileTransferModeDlg::CFileTransferModeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_TRANSMODE, pParent)
{

}

CFileTransferModeDlg::~CFileTransferModeDlg()
{
}

void CFileTransferModeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFileTransferModeDlg, CDialog)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_OVERWRITE, IDC_JUMP_ALL, OnEndDialog)
END_MESSAGE_MAP()


// CFileTransferModeDlg ��Ϣ�������


BOOL CFileTransferModeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	CString strTips;
	strTips.Format("˥�� զ��.... \" %s \" ", m_strFileName);


	for (int i = 0; i < strTips.GetLength(); i += 120)
	{
		strTips.Insert(i, "\n");
		i += 1;
	}
	SetDlgItemText(IDC_TIP, strTips);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}
VOID CFileTransferModeDlg::OnEndDialog(UINT id)
{
	// TODO: Add your control notification handler code here
	EndDialog(id);
}