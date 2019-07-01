//-------------------------------------------------------------
#include "stdafx.h"
#include "Resource.h"

//-------------------------------------------------------------
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
	enum { IDD = IDD_ABOUT };
protected:
	DECLARE_MESSAGE_MAP()
};
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

//-------------------------------------------------------------
#include "Main.h"
#include "Wait.h"
#include "Child.h"
#include "Printer.h"
#include "Config.h"
#include "UserID.h"
#include "ServerThd.h"
#include "ClientThd.h"

#define DEF_COLOR_RED		RGB(150, 60, 60)
#define DEF_COLOR_BLUE		RGB(90, 90, 200)
#define DEF_COLOR_GREEN		RGB(30, 130, 30)
#define DEF_COLOR_YELLOW	RGB(170, 120, 0)
static const DWORD _CHILD_COLORS[4] = {DEF_COLOR_RED, DEF_COLOR_BLUE, DEF_COLOR_GREEN, DEF_COLOR_YELLOW};

#define DEF_TOPBTN	20
#define DEF_MARGIN	20

//-------------------------------------------------------------
CMain::CMain(CWnd* pParent)	: CDialog(CMain::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	memset(&m_pChild, NULL, sizeof(m_pChild));
	memset(&m_pAnchor, NULL, sizeof(m_pAnchor));
	m_nProcRun = 0;
	LoadPrintDll("BPLADLL3.2.dll");
}
CMain::~CMain()
{
	FreePrintDll();
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
		delete m_pChild[i];
}
BEGIN_MESSAGE_MAP(CMain, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ABOUT, &CMain::OnBnClickedAbout)
	ON_BN_CLICKED(IDC_CONFIG, &CMain::OnBnClickedConfig)
	ON_BN_CLICKED(IDC_INIT, &CMain::OnBnClickedInit)
	ON_BN_CLICKED(IDC_USERID, &CMain::OnBnClickedUserID)
	ON_BN_CLICKED(IDC_CLOSE, &CMain::OnBnClickedClose)
END_MESSAGE_MAP()

//-------------------------------------------------------------
UINT CMain::OpenComThd(LPVOID param)
{
	LONG count = CConfig::OpenComPorts();
	//�������֮����Ϣ�رյȴ����ڡ�
	((CWait*)param)->PostMessage(WM_CLOSE, NULL, NULL);
	return count;
}
BOOL CMain::OnInitDialog()
{
	CDialog::OnInitDialog();
	//��������...���˵�����ӵ�ϵͳ�˵��С�IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString content;
		content.LoadString(IDS_ABOUTBOX);
		if (!content.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, content);
		}
	}
	//���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�ִ�д˲�����
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	RECT rt;
	::GetClientRect(m_hWnd, &rt);
	m_szOld.cx = rt.right - rt.left;
	m_szOld.cy = rt.bottom - rt.top;
	m_szMin = m_szOld;
	m_szDelta.cx = 0;
	m_szDelta.cy = 0;

	//�����ĸ��Ӵ��ڡ�
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
	{
		m_pChild[i] = new CChild(this);
		m_pChild[i]->SetColor(_CHILD_COLORS[i]);
		m_pChild[i]->Create(IDD_CHILD, this);
		m_pChild[i]->ShowWindow(SW_SHOW);
	}
	//�ƶ��ĸ��Ӵ��ڵ�ָ��λ�á�
	LONG DEF_FRAME_W = rt.right / 2 - DEF_MARGIN / 2 * 3;
	LONG DEF_FRAME_H = (rt.bottom - DEF_TOPBTN) / 2 - DEF_MARGIN / 2 * 3;
	m_pChild[0]->MoveWindow(DEF_MARGIN,							 DEF_TOPBTN + DEF_MARGIN,				DEF_FRAME_W, DEF_FRAME_H);
	m_pChild[1]->MoveWindow(rt.right - DEF_MARGIN - DEF_FRAME_W, DEF_TOPBTN + DEF_MARGIN,				DEF_FRAME_W, DEF_FRAME_H);
	m_pChild[2]->MoveWindow(DEF_MARGIN,							 rt.bottom - DEF_MARGIN - DEF_FRAME_H,	DEF_FRAME_W, DEF_FRAME_H);
	m_pChild[3]->MoveWindow(rt.right - DEF_MARGIN - DEF_FRAME_W, rt.bottom - DEF_MARGIN - DEF_FRAME_H,	DEF_FRAME_W, DEF_FRAME_H);

	//���ø��ؼ��ƶ�������
	SetControlInfo(GetDlgItem(IDC_ABOUT),	ANCHOR_LEFT		| ANCHOR_TOP);
	SetControlInfo(GetDlgItem(IDC_CONFIG),	ANCHOR_LEFT		| ANCHOR_TOP);
	SetControlInfo(GetDlgItem(IDC_INIT),	ANCHOR_RIGHT	| ANCHOR_TOP);
	SetControlInfo(GetDlgItem(IDC_USERID),	ANCHOR_RIGHT	| ANCHOR_TOP);
	SetControlInfo(GetDlgItem(IDC_CLOSE),	ANCHOR_RIGHT	| ANCHOR_TOP);
	SetControlInfo(GetDlgItem(IDC_TMARK),	ANCHOR_LEFT2	| ANCHOR_TOP);
	SetControlInfo(m_pChild[0], ANCHOR_LEFT  | ANCHOR_TOP	 | RESIZE_HOR2 | RESIZE_VER2);
	SetControlInfo(m_pChild[1], ANCHOR_RIGHT | ANCHOR_TOP	 | RESIZE_HOR2 | RESIZE_VER2);
	SetControlInfo(m_pChild[2], ANCHOR_LEFT  | ANCHOR_BOTTOM | RESIZE_HOR2 | RESIZE_VER2);
	SetControlInfo(m_pChild[3], ANCHOR_RIGHT | ANCHOR_BOTTOM | RESIZE_HOR2 | RESIZE_VER2);
 
	//��������󻯡�
	::ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);
	//���������ļ���
	CConfig::LoadIni(NULL);
	//�������̨��ˢд�����ӡ�
	OnBnClickedInit();
	return TRUE;
}
void CMain::OnBnClickedInit()
{
	//ͣ�ø��Ӵ��ڡ�
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
		m_pChild[i]->SetReady(NULL, NULL, NULL, NULL);

	//�������̨��ˢд�����ӡ�
	CWait wait;
	wait.SetTargetProc(OpenComThd, this);
	while (wait.GetRetValue() == 0)
	{
		wait.DoModal();
		if (wait.GetRetValue() > 0 ||
			::MessageBox(m_hWnd, "Some tester can not be connected.\nDo you want to try again?", "Error", MB_YESNO | MB_ICONINFORMATION) == IDNO)
			break;
	}

	//������Ӵ��󣬽������ð�ť��
	if (wait.GetRetValue() == 0)
	{
		for (int i = 0; i < DEF_CHILD_COUNT; i ++)
		{
			m_pChild[i]->EnableWindow(FALSE);
			m_pChild[i]->Invalidate(FALSE);
		}
		GetDlgItem(IDC_CONFIG)->EnableWindow(FALSE);
	}
	else//����������󣬵������ô��ڣ�ȷ���û�����λ�á�
	{
		OnBnClickedUserID();
	}
}
void CMain::OnBnClickedUserID()
{
	CUserID dlg;
	dlg.DoModal();
	strcpy(CConfig::GetParam()->suser, dlg.Get());
	//������Ӵ��ڡ�
	if (strlen(dlg.Get()) == 0)//(����)
	{
		for (int i = 0; i < DEF_CHILD_COUNT; i ++)
			m_pChild[i]->EnableWindow(FALSE);
		Invalidate();
	}
	else//(����)
	{
		for (int i = 0; i < DEF_CHILD_COUNT; i ++)
		{
			if (CConfig::GetParam()->index[i] >= 0)
				m_pChild[i]->SetReady(CConfig::GetParam(),
									 &CConfig::GetParam()->tester[CConfig::GetParam()->index[i]],
									 &CConfig::GetParam()->flash[CConfig::GetParam()->index[i]],
									  CConfig::GetParam()->stester[CConfig::GetParam()->index[i]]);
			else//(CConfig::GetParam()->index[i] == -1)
				m_pChild[i]->SetReady(NULL, NULL, NULL, NULL);
		}
	}
	//����Ƿ��ǹ���Ա��
	if (strlen(CConfig::GetParam()->suser) > 0 && strlen(CConfig::GetParam()->sadmin) > 0
		&& strcmp(CConfig::GetParam()->sadmin, CConfig::GetParam()->suser) == 0)
		GetDlgItem(IDC_CONFIG)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_CONFIG)->EnableWindow(FALSE);
}
void CMain::OnBnClickedConfig()
{
	//ͣ�ø��Ӵ��ڡ�
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
		m_pChild[i]->SetReady(NULL, NULL, NULL, NULL);
	//��ʾ���ô��ڡ�
	CConfig dlg;
	dlg.DoModal();
	//������Ӵ��ڡ�
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
	{
		if (CConfig::GetParam()->index[i] >= 0)
			m_pChild[i]->SetReady(CConfig::GetParam(),
								 &CConfig::GetParam()->tester[CConfig::GetParam()->index[i]],
								 &CConfig::GetParam()->flash[CConfig::GetParam()->index[i]],
								  CConfig::GetParam()->stester[CConfig::GetParam()->index[i]]);
		else//CConfig::GetParam()->index[i] == -1
			m_pChild[i]->SetReady(NULL, NULL, NULL, NULL);
	}
}
void CMain::OnBnClickedClose()
{
	//���������ļ���
	CConfig::SaveIni(NULL);
	//��ʾ�Ƿ�ػ���
	//�رճ���
	GetDlgItem(IDC_CLOSE)->EnableWindow(FALSE);
	CConfig::CloseComPorts();
	OnOK();
}
void CMain::OnBnClickedAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}
LRESULT CMain::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_DISABLE_CONFIG)
	{
		//���Ժ�ˢд�����н������ð�ť��
		m_nProcRun += (wParam == TRUE ? 1 : -1);
		GetDlgItem(IDC_CONFIG)->EnableWindow(m_nProcRun == 0 ? TRUE : FALSE);
		GetDlgItem(IDC_INIT)->EnableWindow(m_nProcRun == 0 ? TRUE : FALSE);
	}
	return CDialog::WindowProc(message, wParam, lParam);
}

//-------------------------------------------------------------
void CMain::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}
void CMain::OnPaint()
{
	// �����Ի��������С����ť������Ҫ����Ĵ���
	// �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
	// �⽫�ɿ���Զ���ɡ�
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}
HCURSOR CMain::OnQueryDragIcon()
{
	//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
	return static_cast<HCURSOR>(m_hIcon);
}
void CMain::SetControlInfo(CWnd* pwnd, LONG anchor)
{
	LONG i = 0;
	while (m_pAnchor[i][0] != NULL)
		i ++;
	m_pAnchor[i][0] = (LONG)pwnd;
	m_pAnchor[i][1] = anchor;
	if (anchor & ANCHOR_LEFT2)
	{
		RECT rt;
		pwnd->GetWindowRect(&rt);
		ScreenToClient(&rt);
		m_pAnchor[i][2] = rt.right;
	}
	else if (anchor & RESIZE_HOR2)
	{
		RECT rt;
		pwnd->GetWindowRect(&rt);
		m_pAnchor[i][2] = rt.right - rt.left;
	}
	if (anchor & ANCHOR_TOP2)
	{
		RECT rt;
		pwnd->GetWindowRect(&rt);
		ScreenToClient(&rt);
		m_pAnchor[i][3] = rt.bottom;
	}
	else if (anchor & RESIZE_VER2)
	{
		RECT rt;
		pwnd->GetWindowRect(&rt);
		m_pAnchor[i][3] = rt.bottom - rt.top;
	}
}
void CMain::OnSize(UINT type, int cx, int cy) 
{
	if (type == SIZE_MINIMIZED)
		return;
	if (cx < m_szMin.cx)
		cx = m_szMin.cx;
	if (cy < m_szMin.cy)
		cy = m_szMin.cy;

	LONG dx = cx - m_szOld.cx;
	LONG dy = cy - m_szOld.cy;
	m_szDelta.cx += dx;
	m_szDelta.cy += dy;

	LONG i = 0;
	while (m_pAnchor[i][0] != NULL)
	{
		RECT rt;
		((CWnd*)m_pAnchor[i][0])->GetWindowRect(&rt);
		ScreenToClient(&rt);

		//horizontal
		LONG pos = rt.right;
		if (m_pAnchor[i][1] & RESIZE_HOR)
			rt.right += dx;
		else if (m_pAnchor[i][1] & RESIZE_HOR2)
			rt.right = rt.left + m_pAnchor[i][2] + m_szDelta.cx/2;
		if (m_pAnchor[i][1] & ANCHOR_RIGHT)//move left
		{
			rt.left += dx - (rt.right - pos);
			rt.right += dx - (rt.right - pos);
		}
		else if (m_pAnchor[i][1] & ANCHOR_LEFT2)//move to center
		{
			rt.right = m_pAnchor[i][2] + m_szDelta.cx/2;
			rt.left += rt.right - pos;
		}

		//vertical
		pos = rt.bottom;
		if (m_pAnchor[i][1] & RESIZE_VER)
			rt.bottom += dy;
		else if (m_pAnchor[i][1] & RESIZE_VER2)
			rt.bottom = rt.top + m_pAnchor[i][3] + m_szDelta.cy/2;
		if (m_pAnchor[i][1] & ANCHOR_BOTTOM)//move top
		{
			rt.top += dy - (rt.bottom - pos);
			rt.bottom += dy - (rt.bottom - pos);
		}
		else if (m_pAnchor[i][1] & ANCHOR_TOP2)//move to center
		{
			rt.bottom = rt.top + m_pAnchor[i][3] + m_szDelta.cy/2;
			rt.top = pos - rt.top + m_pAnchor[i][3] + m_szDelta.cy/2;
		}

		//reset position
		((CWnd*)m_pAnchor[i][0])->MoveWindow(&rt);
		i ++;
	}
	m_szOld.cx = cx;
	m_szOld.cy = cy;
	CDialog::OnSize(type, cx, cy);
}