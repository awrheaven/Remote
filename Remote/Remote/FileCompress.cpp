// FileCompress.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Remote.h"
#include "FileCompress.h"
#include "afxdialogex.h"


// CFileCompress �Ի���

IMPLEMENT_DYNAMIC(CFileCompress, CDialog)

CFileCompress::CFileCompress(CWnd* pParent /*=NULL*/,ULONG n)
	: CDialog(IDD_FILE_COMPRESS, pParent)
	, m_EditRarName(_T(""))
{
	m_ulType = n;
}

CFileCompress::~CFileCompress()
{
}

void CFileCompress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_RARNAME, m_EditRarName);
}


BEGIN_MESSAGE_MAP(CFileCompress, CDialog)
END_MESSAGE_MAP()


// CFileCompress ��Ϣ�������


BOOL CFileCompress::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	CString strTips;
	switch (m_ulType)
	{
		case 1:
		{
			strTips = "������ѹ���ļ���";
			SetDlgItemText(IDC_STATIC, strTips);
			break;
		}

	}
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}
