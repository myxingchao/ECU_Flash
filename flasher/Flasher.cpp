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
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	// 则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	// 标准初始化
	// 如果未使用这些功能并希望减小最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程更改用于存储设置的注册表项
	// 修改该字符串，例如修改为公司或组织名
	SetRegistryKey(_T("KS Flasher"));

	CMain dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();
	return FALSE;
}