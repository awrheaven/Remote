// SettingDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Remote.h"
#include "SettingDlg.h"
#include "afxdialogex.h"


// CSettingDlg �Ի���

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


// CSettingDlg ��Ϣ�������


BOOL CSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	INT iPort = ((CRemoteApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "ListenPort");
	INT iMaxConnection = ((CRemoteApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "MaxConnection");

	m_nListenPort = iPort;
	m_nMax_Connect = iMaxConnection;

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CSettingDlg::OnBnClickedButtonMsg()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	HWND FatherHandle = NULL;
	FatherHandle = ::FindWindow(NULL, "Remote");
	::SendMessage(FatherHandle, WM_CLOSE, NULL, NULL);
}


void CSettingDlg::OnBnClickedButtonSettingapply()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	((CRemoteApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "ListenPort", m_nListenPort);
	//��ini�ļ���д��ֵ
	((CRemoteApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "MaxConnection", m_nMax_Connect);

	//MessageBox(L"���óɹ����������������Ч��");
	m_ApplyButton.EnableWindow(FALSE);
	m_ApplyButton.ShowWindow(SW_HIDE);
}


void CSettingDlg::OnEnChangeEditPort()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������

	m_ApplyButton.ShowWindow(SW_NORMAL);
	m_ApplyButton.EnableWindow(TRUE);
}


void CSettingDlg::OnEnChangeEditMax()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	HWND hApplyButton = ::GetDlgItem(m_hWnd, IDC_BUTTON_SETTINGAPPLY);

	::ShowWindow(hApplyButton, SW_NORMAL);
	::EnableWindow(hApplyButton, TRUE);
}
