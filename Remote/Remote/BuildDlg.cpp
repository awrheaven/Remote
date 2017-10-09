// BuildDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Remote.h"
#include "BuildDlg.h"
#include "afxdialogex.h"
#include <Windows.h>

// CBuildDlg �Ի���

IMPLEMENT_DYNAMIC(CBuildDlg, CDialog)
/**/
struct CONNECT_ADDRESS
{
	DWORD dwFlag;
	char  szServerIP[MAX_PATH];
	int   iPort;
}g_ConnectAddress = { 0x1234567,"",0 };

INT MemoryFind(const char* szBuffer, const char* Key, INT iBufferSize, INT iKeySize);
CBuildDlg::CBuildDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_BUILD, pParent)
	, m_strIP(_T(""))
	, m_strPort(_T(""))
{

}

CBuildDlg::~CBuildDlg()
{
}

void CBuildDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_IP, m_strIP);
	DDX_Text(pDX, IDC_EDIT_PORT, m_strPort);
}


BEGIN_MESSAGE_MAP(CBuildDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CBuildDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CBuildDlg ��Ϣ�������

/************************************************************************/
/*   ������void CBuildDlg::OnBnClickedOk()                                 */
/************************************************************************/
void CBuildDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	

	CFile File;
	char szTemp[MAX_PATH] = { 0 };
	memset(szTemp, 0, MAX_PATH);

	CString strCurrentPath;
	CString strFile;
	CString strServerFile;
	DWORD dwFileSize = 0;
	BYTE* szBuffer = NULL;
	UpdateData(TRUE);
	strcpy(g_ConnectAddress.szServerIP, m_strIP);
	try
	{
		GetModuleFileName(NULL, szTemp, MAX_PATH);     //�õ��ļ���  
		strCurrentPath = szTemp;
		int iPos = strCurrentPath.ReverseFind('\\');
		strCurrentPath = strCurrentPath.Left(iPos);
		iPos = strCurrentPath.ReverseFind('\\');
		strCurrentPath = strCurrentPath.Left(iPos);
		iPos = strCurrentPath.ReverseFind('\\');
		strCurrentPath = strCurrentPath.Left(iPos);
		strFile = strCurrentPath + "\\RemoteClient\\Debug\\RemoteClient.exe";   //�õ���ǰδ�����ļ���


		//���ļ�
		File.Open(strFile, CFile::modeRead | CFile::typeBinary);
		dwFileSize = File.GetLength();
		szBuffer = new BYTE[dwFileSize];
		ZeroMemory(szBuffer, dwFileSize);
		//��ȡ�ļ�����
		File.Read(szBuffer, dwFileSize);
		File.Close();

		INT iOffset = MemoryFind((char*)szBuffer, 
			(char*)&g_ConnectAddress.dwFlag, dwFileSize, sizeof(DWORD));
		memcpy(szBuffer + iOffset, &g_ConnectAddress, sizeof(g_ConnectAddress));
		strServerFile = strCurrentPath + "\\RemoteClient.exe";
		File.Open(strServerFile, CFile::typeBinary | CFile::modeCreate | CFile::modeWrite);
		File.Write(szBuffer, dwFileSize);
		File.Close();
		int a = GetLastError();
		//delete[] szBuffer;
		MessageBox("���ɳɹ�");

	}
	catch (CMemoryException* e)
	{
		MessageBox("�ڴ治��");
	}
	catch (CFileException* e)
	{
		MessageBox("�ļ���������");
	}
	catch (CException* e)
	{
		MessageBox("δ֪����");
	}
	CDialog::OnOK();


}


INT MemoryFind(const char* szBuffer, const char* Key, INT iBufferSize, INT iKeySize)
{

	INT i, j;
	if (iKeySize == 0 || iBufferSize == 0)
	{
		return -1;
	}
	for (i = 0; i < iBufferSize;i++)
	{
		for (j = 0; j < iKeySize; j++)
		{
			if (szBuffer[i+j] != Key[j])
			{
				break;
			}
		}
		if (j == iKeySize)
		{
			return i;
		}

	}
	return -1;
}