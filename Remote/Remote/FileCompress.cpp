// FileCompress.cpp : 实现文件
//

#include "stdafx.h"
#include "Remote.h"
#include "FileCompress.h"
#include "afxdialogex.h"


// CFileCompress 对话框

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


// CFileCompress 消息处理程序


BOOL CFileCompress::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CString strTips;
	switch (m_ulType)
	{
		case 1:
		{
			strTips = "亲输入压缩文件名";
			SetDlgItemText(IDC_STATIC, strTips);
			break;
		}

	}
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
