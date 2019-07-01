//
#include "stdafx.h"
#include "Resource.h"
#include "Wait.h"

CWait::CWait(CWnd* pParent /*=NULL*/)
	: CDialog(CWait::IDD, pParent)
{
	m_dwRet = 0;
}
CWait::~CWait()
{
}
void CWait::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PIC, m_arPicture);
}
BEGIN_MESSAGE_MAP(CWait, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

void CWait::SetTargetProc(LPVOID process, LPVOID param)
{
	m_pProcess = (AFX_THREADPROC)process;
	m_pParam = param;
}
LPVOID CWait::GetParam()
{
	return m_pParam;
}
BOOL CWait::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_arPicture.Load(MAKEINTRESOURCE(IDR_WAITTING1), "IMAGE") != FALSE)
	{
		//reset dialog size to fit picture
		SIZE szPIC = m_arPicture.GetSize();
		m_arPicture.MoveWindow(1, 1, szPIC.cx, szPIC.cy, FALSE);
		RECT rect;
		GetWindowRect(&rect);
		MoveWindow(rect.left, rect.top, szPIC.cx + 2, szPIC.cy + 2, TRUE);

		//display start
		m_arPicture.Draw();
	}

	//start waitting thread(this thread will sand a close message to parent at last)
	m_pWorkThread = AfxBeginThread(m_pProcess, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, 0);
	m_pWorkThread->m_bAutoDelete = FALSE;
	m_pWorkThread->ResumeThread();

	//return TRUE unless you set the focus to a control
	return TRUE;
}
DWORD CWait::GetRetValue()
{
	return m_dwRet;
}
void CWait::OnClose()
{
	WaitForSingleObject(m_pWorkThread->m_hThread, 2000);
	GetExitCodeThread(m_pWorkThread->m_hThread, &m_dwRet);
	delete m_pWorkThread;
	CDialog::OnClose();
}
