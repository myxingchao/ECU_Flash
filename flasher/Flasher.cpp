// Flasher.cpp
#include "stdafx.h"
#include "Flasher.h"
#include "Main.h"
#include "Child.h"

BEGIN_MESSAGE_MAP(CFlasherApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

//application instance.
CFlasherApp theApp;

CFlasherApp::CFlasherApp()
{
}
BOOL CFlasherApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	// ����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õĹ����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ�����̸������ڴ洢���õ�ע�����
	// �޸ĸ��ַ����������޸�Ϊ��˾����֯��
	SetRegistryKey(_T("KS Flasher"));

	CMain dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();
	return FALSE;
}