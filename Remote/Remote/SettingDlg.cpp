// SettingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Remote.h"
#include "SettingDlg.h"
#include "afxdialogex.h"


// CSettingDlg 对话框

IMPLEMENT_DYNAMIC(CSettingDlg, CDialog)

CSettingDlg::CSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_SET, pParent)
	, m_nListenPort(0)
	, m_nMax_Connect(0)
{

}

CSettingDlg::~CSettingDlg()
{
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_SETTINGAPPLY, m_ApplyButton);
}


BEGIN_MESSAGE_MAP(CSettingDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_MSG, &CSettingDlg::OnBnClickedButtonMsg)
	ON_BN_CLICKED(IDC_BUTTON_SETTINGAPPLY, &CSettingDlg::OnBnClickedButtonSettingapply)
	ON_EN_CHANGE(IDC_EDIT_PORT, &CSettingDlg::OnEnChangeEditPort)
	ON_EN_CHANGE(IDC_EDIT_MAX, &CSettingDlg::OnEnChangeEditMax)
END_MESSAGE_MAP()


// CSettingDlg 消息处理程序


BOOL CSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	INT iPort = ((CRemoteApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "ListenPort");
	INT iMaxConnection = ((CRemoteApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "MaxConnection");

	m_nListenPort = iPort;
	m_nMax_Connect = iMaxConnection;

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CSettingDlg::OnBnClickedButtonMsg()
{
	// TODO: 在此添加控件通知处理程序代码
	HWND FatherHandle = NULL;
	FatherHandle = ::FindWindow(NULL, "Remote");
	::SendMessage(FatherHandle, WM_CLOSE, NULL, NULL);
}


void CSettingDlg::OnBnClickedButtonSettingapply()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	((CRemoteApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "ListenPort", m_nListenPort);
	//向ini文件中写入值
	((CRemoteApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "MaxConnection", m_nMax_Connect);

	//MessageBox(L"设置成功，重启本程序后生效！");
	m_ApplyButton.EnableWindow(FALSE);
	m_ApplyButton.ShowWindow(SW_HIDE);
}


void CSettingDlg::OnEnChangeEditPort()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	m_ApplyButton.ShowWindow(SW_NORMAL);
	m_ApplyButton.EnableWindow(TRUE);
}


void CSettingDlg::OnEnChangeEditMax()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	HWND hApplyButton = ::GetDlgItem(m_hWnd, IDC_BUTTON_SETTINGAPPLY);

	::ShowWindow(hApplyButton, SW_NORMAL);
	::EnableWindow(hApplyButton, TRUE);
}
