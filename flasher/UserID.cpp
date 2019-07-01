//
#include "stdafx.h"
#include "UserID.h"

//-------------------------------------------------------------
CUserID::CUserID(CWnd* pParent)	: CDialog(CUserID::IDD, pParent)
{
	m_szUser[0] = 0;
}
CUserID::~CUserID()
{
}
void CUserID::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}
BEGIN_MESSAGE_MAP(CUserID, CDialog)
	ON_BN_CLICKED(IDC_OK, &CUserID::OnBnClickedOK)
END_MESSAGE_MAP()
//-------------------------------------------------------------
CHAR* CUserID::Get()
{
	return m_szUser;
}
void CUserID::Set(CHAR* user)
{
	strcpy(m_szUser, user);
}
BOOL CUserID::OnInitDialog()
{
	CDialog::OnInitDialog();
	//设置用户ID输入框。
	RECT rt;
	GetDlgItem(IDC_EDIT_USER)->GetWindowRect(&rt);
	ScreenToClient(&rt);
	GetDlgItem(IDC_EDIT_USER)->MoveWindow(rt.left, rt.top + 1, rt.right - rt.left - 1, rt.bottom - rt.top - 1 - 1);
	((CEdit*)GetDlgItem(IDC_EDIT_USER))->SetMargins(6, 6);
	GetDlgItem(IDC_EDIT_USER)->SetWindowText(m_szUser);
	((CEdit*)GetDlgItem(IDC_EDIT_USER))->SetSel(0, -1);
	return TRUE;
}
void CUserID::OnBnClickedOK()
{
	GetDlgItem(IDC_EDIT_USER)->GetWindowText(m_szUser, sizeof(m_szUser));
	if (strlen(m_szUser) > 0 || MessageBox("必须填写操作员编号，否则无法继续操作。", NULL, MB_OKCANCEL | MB_ICONINFORMATION) != IDOK)
		OnOK();
	else
		GetDlgItem(IDC_EDIT_USER)->SetFocus();
}