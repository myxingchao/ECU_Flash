// FlasherDlg.cpp
#include "stdafx.h"
#include "Resource.h"
#include "Child.h"
#include "ComPort.h"
#include "Printer.h"

#define DEF_MAX_TEST_TIME	90000	//(���Գ�ʱʱ�䣬���β��Խ��ȸ��µ������)
#define DEF_MAX_EDIT_LEN	5 * 256 //display error code with the format like: 189, 140, ...
#define DEF_FLASH_TIMEOUT	10000	//because 0xFECA command in erase process needs 6187ms to receive.
#define DEF_CHECK_INTERVAL	1000	//(����ȷ������)
#define WM_PROGRESS_PLUS	WM_USER + 1001

HANDLE CChild::m_hPrint = INVALID_HANDLE_VALUE;
CChild::CChild(CWnd* pParent /*=NULL*/)
	: CDialog(CChild::IDD, pParent)
{
	memset(&m_sLabel, NULL, sizeof(m_sLabel));
	memset(m_pAnchor, NULL, sizeof(m_pAnchor));
	//�ֶ������¼���Wait֮��״̬���䣬�����ٴ�ͨ��Wait�������ֶ�����Reset��
	//�Զ������¼���Wait֮���Զ����ã��´ε���Set֮ǰ�������ٴ�ͨ��Wait��
	m_hStop = ::CreateEvent(NULL, TRUE, FALSE, NULL);//stop
	m_hExit = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_pWaitThd = NULL;
	m_pProcThd = NULL;
	m_bNewECU = FALSE;
	if (CChild::m_hPrint == INVALID_HANDLE_VALUE)
		CChild::m_hPrint = ::CreateEvent(NULL, FALSE, TRUE, NULL);
}
CChild::~CChild()
{
	//ǿ�ƽ���ˢд�̡߳�
	if (m_pProcThd != NULL)
		::TerminateThread(m_pProcThd->m_hThread, 0);
	//�ȴ������߳̽�����
	::SetEvent(m_hExit);
	::SetEvent(m_hStop);//start
	if (m_pWaitThd != NULL)
		::WaitForSingleObject(m_pWaitThd->m_hThread, DEF_CHECK_INTERVAL + 32);
	::CloseHandle(m_hExit);
	::CloseHandle(m_hStop);
	
	if (CChild::m_hPrint != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(CChild::m_hPrint);
		CChild::m_hPrint = INVALID_HANDLE_VALUE;
	}
}
BEGIN_MESSAGE_MAP(CChild, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, &CChild::OnBnClickedStart)
	ON_WM_ENABLE()
	ON_EN_SETFOCUS(IDC_SERIAL, &CChild::OnEnSetfocusSerial)
	ON_EN_CHANGE(IDC_SERIAL, &CChild::OnEnChangeSerial)
END_MESSAGE_MAP()
//
void CChild::SetColor(DWORD color)
{
	m_dwColor = color;
}
void CChild::OnEnable(BOOL bEnable)
{
	GetDlgItem(IDC_TITLE)->EnableWindow(bEnable);
	GetDlgItem(IDC_TITLE2)->EnableWindow(bEnable);
	GetDlgItem(IDC_SERIAL)->EnableWindow(bEnable);
	GetDlgItem(IDC_OUTPUT)->EnableWindow(bEnable);
	GetDlgItem(IDC_PROGRESS)->ShowWindow(bEnable == FALSE ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_PROGHIDE)->ShowWindow(bEnable == FALSE ? SW_SHOW : SW_HIDE);
	CDialog::OnEnable(bEnable);
}
void CChild::SetReady(SParam* param, CComPort* tcom, CComPort* fcom, CHAR* name)
{
	if (param != NULL)//(���ô���)
	{
		EnableWindow(TRUE);
		GetDlgItem(IDC_SERIAL)->SetWindowText("");
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(0);
		m_szOutput[0] = NULL;
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		GetDlgItem(IDC_START)->EnableWindow(FALSE);
		GetDlgItem(IDC_START)->SetWindowText("�ȴ�����");
		//�����豸���ơ�
		CHAR text[64];
		sprintf(text, "         �豸���ƣ�%s (%s)", name, tcom->GetCom());
		GetDlgItem(IDC_TITLE2)->SetWindowText(text);
		//����ͨ�Ų�����
		m_pParam = param;
		m_pTCom = tcom;
		m_pFCom = fcom;
		//�����̵߳ȴ�ECU���ӡ�
		//(��Ȼexit���Զ��¼���ֻҪʹ��һ�ξͻ��Զ���Ч��������ʾConfig����֮ǰ������SetReadyΪFALSEʱ��������һ��exit������ʱ�̲߳�δ���У�
		// ֱ���ر�Config������Ҫ����wait�߳�ʱ��exit��־��Ȼ��Ч�����Ա���������wait�߳�֮ǰ�ֶ�reset������������˳���)
		::ResetEvent(m_hExit);
		::SetEvent(m_hStop);//start
		m_pWaitThd = AfxBeginThread(WaitThd, (LPVOID)this);
	}
	else if (m_pParam != NULL)//(���ô���)
	{
		EnableWindow(FALSE);
		GetDlgItem(IDC_SERIAL)->SetWindowText("");
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(0);
		m_szOutput[0] = NULL;
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		Invalidate();//(the display will be delayed without this)
		UpdateWindow();
		GetDlgItem(IDC_START)->SetWindowText("δʹ��");
		//�����豸���ơ�
		CHAR text[64];
		sprintf(text, "         �豸���ƣ�", name, tcom->GetCom());
		GetDlgItem(IDC_TITLE2)->SetWindowText(text);
		//�õȴ��̼߳������У�ֱ���˳���
		if (m_pParam != NULL)
		{
			::SetEvent(m_hExit);
			::SetEvent(m_hStop);//start
			if (m_pWaitThd != NULL)
				::WaitForSingleObject(m_pWaitThd->m_hThread, DEF_CHECK_INTERVAL + 32);
			m_pWaitThd = NULL;
			//����ͨ�Ų�����
			m_pParam = param;
			m_pTCom = tcom;
			m_pFCom = fcom;
		}
	}
}
BOOL CChild::OnInitDialog()
{
	CDialog::OnInitDialog();

	//������Ϣ�����塣
	HFONT hf = ::CreateFont(
		21,							// nHeight
		0,							// nWidth
		0,							// nEscapement
		0,							// nOrientation
		FW_MEDIUM,					// nWeight
		FALSE,						// bItalic
		FALSE,						// bUnderline
		0,							// cStrikeOut
		DEFAULT_CHARSET,			// nCharSet
		OUT_DEFAULT_PRECIS,			// nOutPrecision
		CLIP_DEFAULT_PRECIS,		// nClipPrecision
		ANTIALIASED_QUALITY,		// nQuality
		DEFAULT_PITCH | FF_SWISS,	// nPitchAndFamily
		"Arial");					// lpszFacename
	GetDlgItem(IDC_TITLE)->SendMessage((UINT)WM_SETFONT, (WPARAM)hf, FALSE);
	GetDlgItem(IDC_TITLE2)->SendMessage((UINT)WM_SETFONT, (WPARAM)hf, FALSE);
	GetDlgItem(IDC_SERIAL)->SendMessage((UINT)WM_SETFONT, (WPARAM)hf, FALSE);
	GetDlgItem(IDC_OUTPUT)->SendMessage((UINT)WM_SETFONT, (WPARAM)hf, FALSE);
	((CEdit*)GetDlgItem(IDC_SERIAL))->SetMargins(5, 5);
	((CEdit*)GetDlgItem(IDC_OUTPUT))->SetMargins(5, 5);
	//RECT rt; (can not work?)
	//((CEdit*)GetDlgItem(IDC_SERIAL))->GetRect(&rt);
	//rt.top += 5;
	//((CEdit*)GetDlgItem(IDC_SERIAL))->SetRect(&rt);
	//((CEdit*)GetDlgItem(IDC_OUTPUT))->GetRect(&rt);
	//rt.top += 5;
	//((CEdit*)GetDlgItem(IDC_OUTPUT))->SetRect(&rt);
	//GetDlgItem(IDC_SERIAL)->SendMessage(EM_SETRECT, 1, (LPARAM)&rt);
	//���໯��ʼ��ť�ͱ߿�(�߿��޷��Ի棿)
	m_btnStart.SubclassDlgItem(IDC_START, this);
	m_btnStart.ModifyStyle(0, BS_OWNERDRAW, 0);
	m_btnStart.SetColor(m_dwColor);
	//m_frmChild.SubclassDlgItem(IDC_FRAME, this);
	//m_frmChild.ModifyStyle(0, BS_OWNERDRAW, 0);
	//m_frmChild.SetColor(m_dwColor);

	//�����ؼ�λ�á�
	RECT rt;
	GetDlgItem(IDC_PROGRESS)->GetWindowRect(&rt);
	ScreenToClient(&rt);
	GetDlgItem(IDC_PROGHIDE)->MoveWindow(&rt, TRUE);
	GetDlgItem(IDC_PROGHIDE)->EnableWindow(FALSE);
	GetDlgItem(IDC_OUTPUT)->GetWindowRect(&rt);
	ScreenToClient(&rt);
	rt.left -= 1;
	GetDlgItem(IDC_OUTPUT)->MoveWindow(&rt, FALSE);
	GetDlgItem(IDC_START)->GetWindowRect(&rt);
	ScreenToClient(&rt);
	rt.left -= 1;
	rt.right -= 1;
	GetDlgItem(IDC_START)->MoveWindow(&rt, FALSE);

	//���ÿؼ��Զ��Ų���
	::GetClientRect(m_hWnd, &rt);
	m_szOld.cx = rt.right - rt.left;
	m_szOld.cy = rt.bottom - rt.top;
	m_szDelta.cx = 0;
	m_szDelta.cy = 0;
	SetControlInfo(GetDlgItem(IDC_SERIAL),	 ANCHOR_LEFT  | ANCHOR_TOP	  | RESIZE_VER);
	SetControlInfo(GetDlgItem(IDC_TITLE2),	 ANCHOR_RIGHT | ANCHOR_TOP	  | RESIZE_HOR);
	SetControlInfo(GetDlgItem(IDC_OUTPUT),	 ANCHOR_RIGHT | ANCHOR_TOP	  | RESIZE_BOTH);
	SetControlInfo(GetDlgItem(IDC_PROGRESS), ANCHOR_LEFT  | ANCHOR_BOTTOM | RESIZE_HOR);
	SetControlInfo(GetDlgItem(IDC_PROGHIDE), ANCHOR_LEFT  | ANCHOR_BOTTOM | RESIZE_HOR);
	SetControlInfo(GetDlgItem(IDC_START),	 ANCHOR_RIGHT | ANCHOR_BOTTOM);
	SetControlInfo(GetDlgItem(IDC_FRAME),	 ANCHOR_LEFT  | ANCHOR_TOP	  | RESIZE_BOTH);
	GetDlgItem(IDC_START)->SetWindowText("��ʼ");
	return TRUE;
}
UINT CChild::WaitThd(LPVOID param)
{
	((CChild*)param)->WaitThd();
	return 0;
}
void CChild::WaitThd()
{
	//����̨������ʱ���ܻ���ΪECU���ӣ���������0x038D00����Ҫ����
	m_pTCom->Receive(256, 0, 256);
	m_pTCom->SendCmd("0x038D00");
	while (true)
	{
		//�ȴ�ECU���ӡ�
		while (true)
		{
			::WaitForSingleObject(m_hStop, INFINITE);
			//�ȴ�֮ǰ�ȼ��hExit����������ã����˳��̡߳�
			if (::WaitForSingleObject(m_hExit, 0) != WAIT_TIMEOUT)//set
				return;
			//�ȴ����նϿ�/������Ϣ��
			if (m_pTCom->Receive(3, 128, DEF_CHECK_INTERVAL) >= 3 && m_pTCom->GetRBuf(0) == 0x03 && m_pTCom->GetRBuf(1) == 0x8D)
				break;
		}

		//���¿ؼ�״̬��
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(0);
		GetDlgItem(IDC_SERIAL)->SetWindowText("");
		m_szOutput[0] = NULL;
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		GetDlgItem(IDC_START)->EnableWindow(FALSE);
		if (m_pTCom->GetRBuf(2) == 0x01)//ж��ECU
		{
			GetDlgItem(IDC_START)->SetWindowText("�ȴ�����");
		}
		else//(m_pTCom->GetRBuf(2) == 0x00)//����ECU
		{
			GetDlgItem(IDC_START)->SetWindowText("�ȴ�����");
			//��ȡ����ʾ���кš�
			ReadSerialNo();
		}

		//��������ECU�ȴ��̡߳�
		if (m_bNewECU == FALSE)
		{
			m_bNewECU = TRUE;
			Sleep(1000);//(����˺��ǩ)
			::SetEvent(CChild::m_hPrint);
		}
	}
}
BOOL CChild::ReadSerialNo()
{
	//��ȡECU���кš�
	ASSERT(m_pFCom->Receive(256, 0, 128) == 0);//(clear receive buffer)
	m_pFCom->SendAsc("SERIAL NO.");
	m_pFCom->Receive(128, 16, 1000);
	//sprintf((CHAR*)m_pFCom->GetRBuf(), "2361201001-20090235-M6006-KHH43M111-X2GIT013-00000000-131228-14010180065");//ooo
	if (strcmp((CHAR*)m_pFCom->GetRBuf(), "SERIAL NO.") == 0)
		m_pFCom->GetRBuf()[0] = NULL;
	if (m_pFCom->GetRBuf(0) != NULL)
	{
		sprintf(m_szOutput, "���к��Ѷ�ȡ���뿪ʼ���ԡ�");
		//�����кŷ�����ʾ��
		GetDlgItem(IDC_SERIAL)->SetWindowText((CHAR*)m_pFCom->GetRBuf());
		FormatSerial();
		return TRUE;
	}
	else//(��ʾ������Ϣ)
	{
		sprintf(m_szOutput, "�޷���ȡ���кţ���ɨ���ά�����롣");
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		return FALSE;
	}
}
void CChild::FormatSerial()
{
	//ȷ���Ƿ��Ѿ���ʽ����
	CHAR text[DEF_MAX_EDIT_LEN];
	GetDlgItem(IDC_SERIAL)->GetWindowText(text, sizeof(text));
	if (strstr(text, "\r\n") == text + strlen(text) - 2)
		text[strlen(text) - 2] = 0;
	if (strlen(text) == 0 || strstr(text, "\r\n") != NULL)
		return;
	//�������кţ����ڴ�ӡ��ǩ��
	strcpy(m_szSerial, text);
	//�����кŷ�����ʾ�ڱ༭���С�
	CHAR* p = text;
	LONG len;
	LONG pos = 0;
	while (strlen(p) > 0)
	{
		len = strstr(p, "-") == NULL ? strlen(p) : strstr(p, "-") - p + 1;
		memcpy(text + pos, p, len);
		pos += len;
		p += len;
		text[pos++] = '\r';
		text[pos++] = '\n';
	}
	text[pos] = 0;
	GetDlgItem(IDC_SERIAL)->SetWindowText(text);
	//����ʼ��ť��Ϊ��Ч��
	GetDlgItem(IDC_START)->GetWindowText(text, sizeof(text));
	if (strcmp(text, "�ȴ�����") == 0)
	{
		GetDlgItem(IDC_START)->SetWindowText("��ʼ");
		GetDlgItem(IDC_START)->EnableWindow(TRUE);
	}
}
void CChild::OnEnSetfocusSerial()
{
	//������кź󣬽�����������к�ȫѡ�У������������롣
	((CEdit*)GetDlgItem(IDC_SERIAL))->SetSel(0, -1, TRUE);
}
//-------------------------------------------------------------
void CChild::OnBnClickedStart()
{
	CHAR text[16];
	GetDlgItem(IDC_START)->GetWindowText(text, sizeof(text));
	//������δ��ʼ����ʼ����&ˢд��
	if (strcmp(text, "��ʼ") == 0)
		m_pProcThd = AfxBeginThread(ProcThd, (LPVOID)this);
	else//�����Ѿ����������´�ӡ��ǩ��
		ShowPrintInfo(PrintLabel(&m_sLabel));
}
//-------------------------------------------------------------
static const CHAR*	_INI_SECTION_LABEL		= "LABEL";
static const CHAR*	_INI_KEY_LABEL_WIDTH	= "WIDTH";
static const CHAR*	_INI_SECTION_COUNT		= "COUNT";
static const CHAR*	_INI_SECTION_IMAGE		= "IMAGE";
static const CHAR*	_INI_SECTION_TEXT		= "TEXT";
static const CHAR*	_INI_SECTION_CODE128	= "CODE128";
static const CHAR*	_INI_SECTION_CODEQR		= "CODEQR";
static const CHAR*	_INI_KEY_PATH			= "PATH";
static const CHAR*	_INI_KEY_TEXT			= "TEXT";
static const CHAR*	_INI_KEY_DATA			= "DATA";
static const CHAR*	_INI_KEY_X				= "X";
static const CHAR*	_INI_KEY_Y				= "Y";
static const CHAR*	_INI_KEY_FONT			= "FONT";
static const CHAR*	_INI_KEY_TYPE			= "TYPE";
static const CHAR*	_INI_KEY_WIDTH			= "WIDTH";
static const CHAR*	_INI_KEY_HEIGHT			= "HEIGHT";
static const CHAR*	_INI_KEY_BOLD			= "BOLD";
static const CHAR*	_INI_KEY_SHOW			= "SHOW";
static const CHAR*	_INI_KEY_RATIO			= "RATIO";
//-------------------------------------------------------------
void CChild::LoadLabel(SLabel* label)
{
	CHAR* path = m_pParam->slabel;
	//��ȡ��ǩ������Ϣ��
	label->nwidth		= ::GetPrivateProfileInt(_INI_SECTION_LABEL, _INI_KEY_LABEL_WIDTH,	750,	path);
	label->nimage		= ::GetPrivateProfileInt(_INI_SECTION_COUNT, _INI_SECTION_IMAGE,	0,		path);
	label->ntext		= ::GetPrivateProfileInt(_INI_SECTION_COUNT, _INI_SECTION_TEXT,		0,		path);
	label->ncode128		= ::GetPrivateProfileInt(_INI_SECTION_COUNT, _INI_SECTION_CODE128,	0,		path);
	label->ncodeqr		= ::GetPrivateProfileInt(_INI_SECTION_COUNT, _INI_SECTION_CODEQR,	0,		path);
	label->pimage		= new SLabelImage[label->nimage];
	label->ptext		= new SLabelText[label->ntext];
	label->pcode128		= new SLabelCode128[label->ncode128];
	label->pcodeqr		= new SLabelCodeQR[label->ncodeqr];
	CHAR sec[32], buf[32];
	for (int i = 0; i < label->nimage; i ++)
	{
		sprintf(sec, "%s-%d", _INI_SECTION_IMAGE, i + 1);
		GetPrivateProfileString(sec, _INI_KEY_PATH, "", label->pimage[i].path, sizeof(label->pimage[i].path), path);
		label->pimage[i].x		= ::GetPrivateProfileInt(sec, _INI_KEY_X,		0, path);
		label->pimage[i].y		= ::GetPrivateProfileInt(sec, _INI_KEY_Y,		0, path);
	}
	for (int i = 0; i < label->ntext; i ++)
	{
		sprintf(sec, "%s-%d", _INI_SECTION_TEXT, i + 1);
		GetPrivateProfileString(sec, _INI_KEY_FONT, "", label->ptext[i].font, sizeof(label->ptext[i].font), path);
		label->ptext[i].type	= ::GetPrivateProfileInt(sec, _INI_KEY_TYPE,	0, path);
		label->ptext[i].x		= ::GetPrivateProfileInt(sec, _INI_KEY_X,		0, path);
		label->ptext[i].y		= ::GetPrivateProfileInt(sec, _INI_KEY_Y,		0, path);
		label->ptext[i].ratio	= ::GetPrivateProfileInt(sec, _INI_KEY_RATIO,	0, path);
		label->ptext[i].bold	= ::GetPrivateProfileInt(sec, _INI_KEY_BOLD,	0, path);
		GetPrivateProfileString(sec, _INI_KEY_TEXT, "", label->ptext[i].text, sizeof(label->ptext[i].text), path);
		if (label->ptext[i].type == 2)//date
		{
			//�޸Ľ�������ϢΪ�������ڡ�
			SYSTEMTIME st;
			::GetLocalTime(&st);
			sprintf(label->ptext[i].text, "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
		}
		else if (label->ptext[i].type == 3)//number
		{
			//���������ļ��е����ֱ�š�
			sprintf(buf, "%0*d", strlen(label->ptext[i].text), strtol(label->ptext[i].text, NULL, 10) + 1);
			WritePrivateProfileString(sec, _INI_KEY_TEXT, buf, path);
		}
	}
	for (int i = 0; i < label->ncode128; i ++)
	{
		sprintf(sec, "%s-%d", _INI_SECTION_CODE128, i + 1);
		GetPrivateProfileString(sec, _INI_KEY_DATA, "", label->pcode128[i].data, sizeof(label->pcode128[i].data), path);
		label->pcode128[i].show		= ::GetPrivateProfileInt(sec, _INI_KEY_SHOW,	0, path);
		label->pcode128[i].x		= ::GetPrivateProfileInt(sec, _INI_KEY_X,		0, path);
		label->pcode128[i].y		= ::GetPrivateProfileInt(sec, _INI_KEY_Y,		0, path);
		label->pcode128[i].width	= ::GetPrivateProfileInt(sec, _INI_KEY_WIDTH,	0, path);
		label->pcode128[i].height	= ::GetPrivateProfileInt(sec, _INI_KEY_HEIGHT,	0, path);
	}
	for (int i = 0; i < label->ncodeqr; i ++)
	{
		sprintf(sec, "%s-%d", _INI_SECTION_CODEQR, i + 1);
		GetPrivateProfileString(sec, _INI_KEY_DATA, "", label->pcodeqr[i].data, sizeof(label->pcodeqr[i].data), path);
		label->pcodeqr[i].x			= ::GetPrivateProfileInt(sec, _INI_KEY_X,		0, path);
		label->pcodeqr[i].y			= ::GetPrivateProfileInt(sec, _INI_KEY_Y,		0, path);
		label->pcodeqr[i].width		= ::GetPrivateProfileInt(sec, _INI_KEY_WIDTH,	0, path);
	}
}
void CChild::ShowPrintInfo(LONG error)
{
	switch (error)
	{
	case 0:	sprintf(m_szOutput + strlen(m_szOutput), "\r\n������ӡ��ϡ�");					break;
	case 11:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣��ö�ٴ���");			break;
	case 12:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣��û�з��ִ�ӡ����");	break;
	case 13:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣���޷����ӡ�");			break;
	case 14:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣������������ӡ����");	break;
	case 15:
			sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣��");
			char buf[3];
			int len;
			if (Get_StatusData(0, buf, sizeof(buf), &len, 3000) != SDK_SUCCESS);
			{
				sprintf(m_szOutput + strlen(m_szOutput), "����������ӡ����");	
				break;
			}
			for (int i = 0; i < 8; i ++)
			{
				if (buf[0] & (1 << i) != 0)
				{
					switch (i)
					{
					case 0: sprintf(m_szOutput + strlen(m_szOutput), "��ӡ��");				break;
					case 1: sprintf(m_szOutput + strlen(m_szOutput), "�ޱ�ǩ��");			break;
					case 2: sprintf(m_szOutput + strlen(m_szOutput), "��ӡ����ͣ��");		break;
					case 3: sprintf(m_szOutput + strlen(m_szOutput), "��ӡ��æ��");			break;
					case 4: sprintf(m_szOutput + strlen(m_szOutput), "δ֪����");			break;
					case 5: sprintf(m_szOutput + strlen(m_szOutput), "ȱɫ����");			break;
					case 6: sprintf(m_szOutput + strlen(m_szOutput), "ȱֽ��");				break;
					case 7: sprintf(m_szOutput + strlen(m_szOutput), "������æ��");			break;
					}
				}
				if (buf[1] & (1 << i) != 0)
				{
					switch (i)
					{
					case 0: sprintf(m_szOutput + strlen(m_szOutput), "��ӡͷ�𻵣�");		break;
					case 1: sprintf(m_szOutput + strlen(m_szOutput), "��ǩֽ��ʧ��־��");	break;
					case 2: sprintf(m_szOutput + strlen(m_szOutput), "�е���ʱ��");			break;
					case 3: sprintf(m_szOutput + strlen(m_szOutput), "��ӡͷ̧��");		break;
					case 4: sprintf(m_szOutput + strlen(m_szOutput), "δ֪����");			break;
					case 5: sprintf(m_szOutput + strlen(m_szOutput), "��ӡͷ���ȣ�");		break;
					case 6: sprintf(m_szOutput + strlen(m_szOutput), "��ʼ����־λ1��");	break;
					case 7: sprintf(m_szOutput + strlen(m_szOutput), "ͨѶ����");			break;
					}
				}
			}
			m_szOutput[strlen(m_szOutput) - 1] = 0;//(delete the last ",")
			break;
	case 16:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣���޷����ò�����");		break;
	case 17:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣����ֽλ�ô���");		break;
	case 19:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣���޷������ӡ״̬��");	break;
	case 21:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣���޷����λͼ��");		break;
	case 22:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣���޷�����������֡�");	break;
	case 23:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣���޷����ʸ�����֡�");	break;
	case 24:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣���޷����128���롣");	break;
	case 25:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣���޷����QR���롣");	break;
	case 41:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣���޷���ӡ��");			break;
	case 42:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣���޷��Ͽ����ӡ�");		break;
	default:sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ӡ���쳣�����鲻����");			break;
	}
	GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
}
LONG CChild::PrintLabel(SLabel* label)
{
	//���Ӵ�ӡ����
	int len;
	CHAR buf[256];
	/*/����Ҫ������Ҳ�����ӣ�ö��������ò��û�ã�
	if (Enum_DeviceNameList(1, buf, sizeof(buf), &len, "SBTP") != SDK_SUCCESS)
		return 11;
	if (strlen(buf) == 0)
		return 12;*/
	USBPara pusb;
	//strcpy(pusb.cDevName, buf);	//"BTP-2300E3(U) 1"
	pusb.cDevName[0] = NULL;
	pusb.iUSBMode = 0;			//0:APIģʽ��1:��ģʽ��
	pusb.iDevID = -1;			//-1:ֱ�Ӵ򿪣�0:ͨ���ڲ�ID�򿪡�
	int hUSB = Comm_OpenPort(PORT_USBDEVICE, &pusb, sizeof(pusb), 0, NULL);
	if (hUSB < 0)
		return 13;
	if (Get_StatusData(hUSB, buf, sizeof(buf), &len, 3000) != SDK_SUCCESS)
		return 14;
	if (buf[0] != 0 || buf[1] != 0)
		return 15;

	//���ô�ӡ����
	BasePara pbase;
	pbase.iDPI = 200;			//200DPI
	pbase.iUnit = 1;			//����/10
	pbase.iOutMode = 3;			//˺��
	pbase.iPaperType = 3;		//��ǩֽ
	pbase.iPrintMode = 2;		//��תӡ
	pbase.iAllRotateMode = 0;	//����ת
	pbase.iAllMirror = 0;		//������
	if (Set_BasePara(hUSB, &pbase, sizeof(pbase)) != SDK_SUCCESS)
		return 16;
	if (Set_OutPosition(hUSB, 300) != SDK_SUCCESS)//��ֽƫ�ƾ���
		return 17;

	//��ӡ������Ϣ��
	if (label->nwidth == 0)//(no label info)
	{
		//���ñ�ǩģʽ��(����������Ϊ750����ÿ��ֻ�����7�������룬�޷����ø���)
		if (Prt_EnterLabelMode(hUSB, 0, 1500, 0, 0, 2, 12) != SDK_SUCCESS)
			return 19;

		CHAR row[256];
		//���ô�ӡ���кŶ�ά�롣
		sprintf(row, "QA,%s", m_szSerial);
		QRPrintPara qr;
		qr.cCodeData = row;
		qr.iDataLen = strlen(qr.cCodeData);
		qr.iStartX = 150 + 600;
		qr.iStartY = 335;
		qr.iWeigth = 3;
		qr.iSymbolType = 1;
		qr.iLanguageMode = 0;
		if (Prt_LabelPrintSetting(hUSB, PRINT_QR, &qr, sizeof(qr), 0) != SDK_SUCCESS)
			return 25;
		//���ô�ӡ����λ�á�
		TruetypePrintPara txt;
		txt.iStartX = 150 + 70;//(��֪��Ϊʲô��150������˵Ӧ����(1500 - 750) / 2 = 375)
		txt.iFontHeight = 30;
		txt.iFontWidth = 0;
		txt.iBold = 0;
		txt.iItalic = 0;
		txt.iUnderline = 0;
		txt.iRotate = 1;
		strcpy(txt.cFontName, "SimSun");
		//���ô�ӡ���к����֡�
		txt.cText = m_szSerial;
		txt.iStartY = 400;
		if (Prt_LabelPrintSetting(hUSB, PRINT_TREUTYPE, &txt, sizeof(txt), 0) != SDK_SUCCESS)
			return 23;
		//���ô�ӡ�������ݡ�
		CHAR* s = m_szOutput;
		CHAR* p = strstr(s, "\r\n");
		txt.cText = row;
		txt.iStartY = 350;
		while(txt.iStartY > 50 && p != NULL)
		{
			memcpy(row, s, p - s);
			row[p - s] = 0;
			s = p + 2;
			if (strlen(row) == 0)
				continue;
			if (Prt_LabelPrintSetting(hUSB, PRINT_TREUTYPE, &txt, sizeof(txt), 0) != SDK_SUCCESS)
				return 23;
			txt.iStartY -= 50;
		}
		strcpy(row, s);
		row[49] = 0;//(ÿ��������49���ַ���ȷ��û�����)
		if (Prt_LabelPrintSetting(hUSB, PRINT_TREUTYPE, &txt, sizeof(txt), 0) != SDK_SUCCESS)
			return 23;

		//InnerFontSetting txt;
		//txt.cTextData = text;
		//txt.iFontType = 13;
		//txt.iWidthMultiple = 1;
		//txt.iHeightMultiple = 1;
		//txt.iMirrorEnable = 0;
		//txt.iSpace = 0;
		//txt.iRotate = 1;
		//txt.iStartX = 50;
		//txt.iStartY = 450;
		//if (Prt_LabelPrintSetting(hUSB, PRINT_INNERFONT, &txt, sizeof(txt), 0) != SDK_SUCCESS)
		//	return 22;

		//��ʼ��ӡ��
		if (Prt_EndLabelAndPrint(hUSB, 1, 1, 1) != SDK_SUCCESS)
			return 41;
		//�Ͽ���ӡ����
		if (Comm_ClosePort(hUSB) != SDK_SUCCESS)
			return 42;
		return 0;
	}

	//���ñ�ǩģʽ��
	if (Prt_EnterLabelMode(hUSB, 0, label->nwidth, 0, 0, 2, 12) != SDK_SUCCESS)
		return 19;

	//ȡ�ó���·����
	CHAR dir[_MAX_DIR];
	CHAR drive[_MAX_DRIVE];
	CHAR file[_MAX_FNAME];
	::GetModuleFileName(NULL, file, _MAX_PATH);
	_splitpath(file, drive, dir, file, NULL);
	//׼����ӡͼ�Ρ�
	ImageSetting img;
	for (int i = 0; i < label->nimage; i ++)
	{
		if (strstr(label->pimage[i].path, ":") == 0)
			sprintf(img.cImageName, "%s%s%s", drive, dir, label->pimage[i].path);
		else
			sprintf(img.cImageName, label->pimage[i].path);
		img.iStartX = label->pimage[i].x;
		img.iStartY = label->pimage[i].y;
		if (Prt_LabelPrintSetting(hUSB, PRINT_IMAGE, &img, sizeof(img), 0) != SDK_SUCCESS)
			return 21;
	}

	//׼����ӡ���֡�
	for (int i = 0; i < label->ntext; i ++)
	{
		//ʹ���ڲ��������塣
		if (label->ptext[i].font[0] == 0 || strtol(label->ptext[i].font, NULL, 10) != 0)
		{
			InnerFontSetting txt;
			txt.cTextData = label->ptext[i].text;
			txt.iFontType = strtol(label->ptext[i].font, NULL, 10);
			txt.iStartX = label->ptext[i].x;
			txt.iStartY = label->ptext[i].y;
			txt.iWidthMultiple = label->ptext[i].ratio;
			txt.iHeightMultiple = label->ptext[i].ratio;
			txt.iRotate = 1;
			txt.iMirrorEnable = 0;
			txt.iSpace = 0;
			if (Prt_LabelPrintSetting(hUSB, PRINT_INNERFONT, &txt, sizeof(txt), 0) != SDK_SUCCESS)
				return 22;
		}
		else//ʹ��ϵͳʸ�����塣
		{
			TruetypePrintPara txt;
			txt.cText = label->ptext[i].text;
			strcpy(txt.cFontName, label->ptext[i].font);
			txt.iStartX = label->ptext[i].x;
			txt.iStartY = label->ptext[i].y;
			txt.iFontHeight = label->ptext[i].ratio;
			txt.iFontWidth = 0;
			txt.iBold = label->ptext[i].bold;
			txt.iItalic = 0;
			txt.iUnderline = 0;
			txt.iRotate = 1;
			if (Prt_LabelPrintSetting(hUSB, PRINT_TREUTYPE, &txt, sizeof(txt), 0) != SDK_SUCCESS)
				return 23;
		}
	}

	//׼����ӡ���кš�
	Code128PrintPara bar;
	for (int i = 0; i < label->ncode128; i ++)
	{
		if (strlen(label->pcode128[i].data) == 0)
			bar.cCodeData = m_szSerial;
		else//direct output
			bar.cCodeData = label->pcode128[i].data;
		bar.iHeight = label->pcode128[i].height;
		bar.iNumberBase = label->pcode128[i].width;
		bar.iStartX = label->pcode128[i].x;
		bar.iStartY = label->pcode128[i].y;
		bar.iType = label->pcode128[i].show;
		bar.iRotate = 1;
		if (Prt_LabelPrintSetting(hUSB, PRINT_CODE_128, &bar, sizeof(bar), 0) != SDK_SUCCESS)
			return 24;
	}

	//׼����ӡ��ά�롣
	QRPrintPara qr;
	qr.cCodeData = buf;
	for (int i = 0; i < label->ncodeqr; i ++)
	{
		if (strlen(label->pcodeqr[i].data) == 0)
			sprintf(buf, "QA,%s", m_szSerial);
		else//direct output
			sprintf(buf, "QA,%s", label->pcodeqr[i].data);
		qr.iDataLen = strlen(qr.cCodeData);
		qr.iStartX = label->pcodeqr[i].x;
		qr.iStartY = label->pcodeqr[i].y;
		qr.iWeigth = label->pcodeqr[i].width;
		qr.iSymbolType = 1;
		qr.iLanguageMode = 0;
		if (Prt_LabelPrintSetting(hUSB, PRINT_QR, &qr, sizeof(qr), 0) != SDK_SUCCESS)
			return 25;
	}

	//��ʼ��ӡ��
	if (Prt_EndLabelAndPrint(hUSB, 1, 1, 1) != SDK_SUCCESS)
		return 41;
	//�Ͽ���ӡ����
	if (Comm_ClosePort(hUSB) != SDK_SUCCESS)
		return 42;
	return 0;
}
//-------------------------------------------------------------
UINT CChild::ProcThd(LPVOID param)
{
	((CChild*)param)->Process();
	return 0;
}
BOOL CChild::ProcTest()
{
	GetDlgItem(IDC_START)->SetWindowText("���ڲ���");

	//�Ͽ�ECU���Ӽ̵�����
	m_pTCom->SendCmd("0x038600");
	//LED������ʾ��
	m_pTCom->SendCmd("0x038C00");

	//ִ�й��ܲ��ԡ�
	CHAR cmd[16];
	sprintf(cmd, "0x%02X87%s", 2 + strlen(m_pParam->smodal) / 2, m_pParam->smodal);
	m_pTCom->Receive(256, 0, 128);//(clear receive buffer)
	m_pTCom->SendCmd(cmd);

	//��ʼ����������
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetBkColor(CLR_DEFAULT);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetBarColor(CLR_DEFAULT);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetRange(0, 100);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(0);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetStep(1);

	//���Թ����и��½�������
	m_pTCom->Receive(3, 32, DEF_MAX_TEST_TIME);
	while (m_pTCom->GetRBuf(0) != 0x03 || m_pTCom->GetRBuf(1) != 0x88 || m_pTCom->GetRBuf(2) != 100)
	{
		if (m_pTCom->GetRBuf(0) == 0x03 && m_pTCom->GetRBuf(1) == 0x88)
			((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(m_pTCom->GetRBuf(2));
		if (m_pTCom->Receive(3, 32, DEF_MAX_TEST_TIME) < 3)
			break;
	}
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(100);
	if (m_pTCom->GetRBuf(0) != 0x03 || m_pTCom->GetRBuf(1) != 0x88 || m_pTCom->GetRBuf(2) != 100)
	{
		//��ʾ������Ϣ��
		sprintf(m_szOutput + strlen(m_szOutput), "\r\n���Գ�ʱ��");
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		//���������Ϣ��
		m_pParam->flog.SaveLog(m_pParam->slog, ",Testing Error: Timeout");
		return FALSE;
	}

	//��ѯ���Խ����
	GetDlgItem(IDC_START)->SetWindowText("��ȡ���");
	m_pTCom->Receive(256, 0, 128);//(clear receive buffer)
	m_pTCom->SendCmd("0x038900");
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetRange(0, 100);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(0);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetStep(100 * 500 / 10000);
	SetTimer(1, 500, NULL);
	m_pTCom->Receive(4096, 64, 10000);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(100);
	KillTimer(1);
	
	if (m_pTCom->GetRBuf(0) == 0x02 && m_pTCom->GetRBuf(1) == 0x89)//���Գɹ�
	{
		//����ɹ���Ϣ���ļ���
		sprintf(m_szOutput + strlen(m_szOutput), "\r\n���Գɹ���");
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		m_pParam->flog.SaveLog(m_pParam->slog, ",Test Succeeded");
		return TRUE;
	}
	else//����ʧ�ܣ���ʾ�����벢���ء�
	{
		//��ʾ�����롣
		sprintf(m_szOutput + strlen(m_szOutput), "\r\n����ʧ�ܣ�������: \r\n");
		LONG pos = strlen(m_szOutput);
		for (int i = 2; i < m_pTCom->GetRBuf(0); i ++)
		{
			if ((i - 2) % 10 == 9)
			{
				sprintf(m_szOutput + pos + 5 * (i - 2), "\r\n", m_pTCom->GetRBuf(i));
				pos += 2;
			}
			sprintf(m_szOutput + pos + 5 * (i - 2), "%03d, ", m_pTCom->GetRBuf(i));
		}
		m_szOutput[pos + 5 * (m_pTCom->GetRBuf(0) - 2) - 2] = 0;
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		//��������뵽�ļ���
		m_pParam->flog.SaveLog(m_pParam->slog, ",");
		m_pParam->flog.SaveLog(m_pParam->slog, m_szOutput + pos);
		//return FALSE;
		return TRUE;//ooo
	}
}
BOOL CChild::ProcFlash()
{
	GetDlgItem(IDC_START)->SetWindowText("׼��ˢд");

	//���ý���������ʾ��Ϣ��
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetBkColor(RGB(170, 190, 170));
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetBarColor(RGB(250, 190, 250));
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetRange(0, m_pParam->nstep);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(0);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetStep(1);
	GetDlgItem(IDC_START)->SetWindowText("����ˢд");

	//����ʱ���������ˢд���Ρ�
	LONG error;
	LONG limit = 3;
	while (limit > 0)
	{
		limit --;
		//�պ�ECU���Ӽ̵�����
		m_pTCom->SendCmd("0x038600");
		::Sleep(1000);
		m_pTCom->Receive(3, 0, 5000);
		if (m_pTCom->Receive(256, 0, 256) > 0)//(clear receive buffer)
			m_pTCom = m_pTCom;//ooo
		m_pTCom->SendCmd("0x038500");
		::Sleep(1000);
		m_pTCom->Receive(3, 0, 5000);
		if (m_pFCom->Receive(256, 0, 256) > 0)//(ignore relay noises)
			m_pFCom = m_pFCom;//ooo

		if (m_pTCom->GetRBuf(0) != 0x03 || m_pTCom->GetRBuf(1) != 0x85 || m_pTCom->GetRBuf(2) != 0x00)
		{
			error = 10000;
		}
		else//�̵����л��ɹ�����ʼˢд��
		{
			error = FlashECU();
		}
		//���ý��������񡣣����������һ��WM_PROGRESS_PLUS��Ϣ����֮ǰ��
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(m_pParam->nstep);
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->Invalidate(FALSE);

		//�Ͽ�ECU���Ӽ̵�����
		::Sleep(500);
		m_pTCom->SendCmd("0x038600");

		if (error <= 0)
		{
			//ˢд�ɹ�������ˢд������
			m_pParam->nstep = 0 - error;
			limit = -1;
		}
		else//(error > 0)
		{
			//��ʾ������Ϣ��
			sprintf(m_szOutput + strlen(m_szOutput), "\r\nˢдʧ�ܣ�������: %05d��", error);
			GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
			//���������Ϣ��
			m_pParam->flog.SaveLog(m_pParam->slog, ",Flashing Error: ");
			m_pParam->flog.SaveLog(m_pParam->slog, m_szOutput + strlen(m_szOutput) - 5);
			//��ʾ����ˢд��
			if (limit > 0)
			{
				sprintf(m_szOutput + strlen(m_szOutput), "����ˢд...");
				GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
				m_pParam->flog.SaveLog(m_pParam->slog, ",Flash Again.");
				::Sleep(1000);
				((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(0);
			}
		}
	}
	if (limit == 0)
	{
		sprintf(m_szOutput + strlen(m_szOutput), "\r\nˢд�����������ƣ���ֹ���ԡ�");
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		//����ɹ���Ϣ���ļ���
		m_pParam->flog.SaveLog(m_pParam->slog, ",Flashing Faild.");
		return FALSE;
	}
	else//(limit != 0)
	{
		sprintf(m_szOutput + strlen(m_szOutput), "\r\nˢд�ɹ���");
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		//����ɹ���Ϣ���ļ���
		m_pParam->flog.SaveLog(m_pParam->slog, ",Flashing Succeeded.");
		return TRUE;
	}
}
void CChild::Process()
{
	//�������������ð�ť��
	GetParent()->PostMessage(WM_DISABLE_CONFIG, TRUE, 0);
	//��ʼ��⣬��ť�����á�
	GetDlgItem(IDC_START)->EnableWindow(FALSE);
	//��ͣ�ȴ��߳�ֱ������µ�ˢд����
	::ResetEvent(m_hStop);//stop
	//make sure the wait thread has been suspended.
	::Sleep(DEF_CHECK_INTERVAL + 32);

	//�������ʱ�䡢�����ߡ����кš�
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CHAR text[DEF_MAX_EDIT_LEN];
	sprintf(text, "\r\n%04d/%02d/%02d %02d:%02d:%02d,%s,%s,", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, m_pParam->suser, m_szSerial);
	m_pParam->flog.SaveLog(m_pParam->slog, text);

	//���汾�����ԡ�ˢд��
	BOOL ret;
	switch (m_pParam->nproc)
	{
	case 0://PCBA
		ret = ProcFlash()											== TRUE &&
			  ProcTest()											== TRUE &&
			  CheckVersion("1", m_pParam->sprog1, m_pParam->sdata1) == TRUE
			? TRUE : FALSE;
		break;
	case 1://����
		ret = CheckVersion("0", m_pParam->sprog0, m_pParam->sdata0) == TRUE &&
			  ProcTest()											== TRUE
			? TRUE : FALSE;
		break;
	case 2://�ռ�
		ret = CheckVersion("0", m_pParam->sprog0, m_pParam->sdata0) == TRUE &&
			  ProcTest()											== TRUE &&
			  ProcFlash()											== TRUE &&
			  CheckVersion("1", m_pParam->sprog1, m_pParam->sdata1) == TRUE
			? TRUE : FALSE;
		break;
	}

	//��ӡ��ǩ��
	if (m_pParam->nproc != 1)//(PCBA���ռ�)
	{
		GetDlgItem(IDC_START)->SetWindowText("�Ŷ�...");
		::WaitForSingleObject(CChild::m_hPrint, INFINITE);
		m_sLabel.nwidth = 0;
		if (ret == TRUE)//(����������ǩ��Ϣ)
			LoadLabel(&m_sLabel);
		ShowPrintInfo(PrintLabel(&m_sLabel));
		GetDlgItem(IDC_START)->SetWindowText("���´�ӡ");
		GetDlgItem(IDC_START)->EnableWindow(TRUE);
	}

	//ʹECU��LED��˸&������ʾ��
	m_pTCom->SendCmd("0x038C02");
	::MessageBeep(MB_ICONQUESTION);

	//���ù�����ɱ�־��
	m_bNewECU = FALSE;
	//�����ȴ��̼߳��ж��ECU��
	sprintf(m_szOutput + strlen(m_szOutput), "\r\n\r\n---��ж��ECU---");
	GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
	::SetEvent(m_hStop);//start
	//�������������ð�ť��
	GetParent()->PostMessage(WM_DISABLE_CONFIG, FALSE, 0);
}
LRESULT CChild::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_PROGRESS_PLUS)
	{
		int low, high;
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->GetRange(low, high);
		if (((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->GetPos() < high)
			((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->StepIt();
	}
	return CDialog::WindowProc(message, wParam, lParam);
}
//-------------------------------------------------------------
struct SEcuMsg
{
	LONG	index;
	BYTE	value;
	LONG	len;
	LONG	receive;
	BYTE*	cmd;
};
void CChild::SaveError(CHAR* title, LONG index, BYTE* msg, LONG len)
{
	CLogFile log;
	log.SaveLog(".\\error.log", title);//
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CHAR text[132];
	sprintf(text, ": index=%04d, %02d-%02d-%02d, %02d:%02d:%02d, ", index, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	log.SaveLog(".\\error.log", text);//
	for (int i = 0; i < len && i < 64; i ++)
		sprintf(text, "%02X", msg[i]);
	log.SaveLog(".\\error.log", text);//
	sprintf(text, "%s, len = %d\n", len > 64 ? "..." : "", len);
	log.SaveLog(".\\error.log", text);//
}
LONG CChild::SendEcuMsg(BYTE* cmd, LONG len, LONG receive, LONG index, BYTE value)
{
	//DWORD ooo = GetTickCount();
	if (len > 0)//(receive once without sending message)
	{
		//TRACE("++%05d++ ", m_nStep);
		//for (int i = 0; i < len && i < 64; i ++)
		//	TRACE("%02X", cmd[i]);
		//TRACE("%s, len = %d\n", len > 64 ? "..." : "", len);
		if (m_pFCom->SendHex(cmd, len) == FALSE)
		{
			SaveError("sending error", m_nStep, cmd, len);
			return -1;
		}
	}
	LONG count = m_pFCom->Receive(receive, -1, DEF_FLASH_TIMEOUT);
	//TRACE("--%05d-- ", m_nStep);
	//for (int i = 0; i < m_pFCom->GetRLen() && i < 64; i ++)
	//	TRACE("%02X", m_pFCom->GetRBuf(i));
	//TRACE("%s, --			len = %d, receive = %d\n", m_pFCom->GetRLen() > 64 ? "..." : "", m_pFCom->GetRLen(), GetTickCount() - ooo);
	if (index >= 0 && (m_pFCom->GetRBuf(0) != cmd[0] || m_pFCom->GetRBuf(index) != value))
	{
		TRACE("\n--XXXX--%05d---- ", m_nStep);
		for (int i = 0; i < m_pFCom->GetRLen(); i ++)
			TRACE("%02X", m_pFCom->GetRBuf(i));
		SaveError("receiving error", m_nStep, cmd, len);
		SaveError("receive detail", m_nStep, m_pFCom->GetRBuf(), m_pFCom->GetRLen());
		return -1;
	}
	//���½�������
	PostMessage(WM_PROGRESS_PLUS, m_nStep, 0);
	m_nStep ++;
	return count;
}
//-------------------------------------------------------------
enum KSID
{
	StartCommunication				= 0x81,
	StartDiagnosticSession			= 0x10,
	StartRoutineByLocalIdentifier	= 0x31,
	RequestDownload					= 0x34,
	RequestTransferExit				= 0x37,
	EcuReset						= 0x11,
	TesterPresent					= 0x3E,
	ReadMemoryByAddress				= 0x23,
	WriteMemoryByAddress			= 0x3D,
	TransferData					= 0x36,
	AccessTimingParameter			= 0x83
};
DWORD CChild::ComposeCmd(BYTE id, BYTE *cmd)
{
	//����ComposeCmd�����ַ�ʽ��
	//����ǹ̶����ȣ������ڴ˺����¶��峤�ȣ�����ʱֻ���ڻ������������ڲ��������������õĲ�����2������һ��������ڶ����ǻ�����ָ�롣
	//����ǿɱ䳤�ȣ������ֺͲ�����Ҫд�뻺��������������ʱ��һ�������ǻ�����ָ�룬�ڶ�������������ȡ�
	//�����ʼ��������������ȣ������ֺͲ����ĸ�������������ͷ��У���롣
	//һЩ�̶����ȵ�����˺����Զ����볤����Ϣ��
	int len;
	switch(id)
	{
		case StartCommunication:			len = 1;	break;
		case StartRoutineByLocalIdentifier:	len = 2;	break;
		case RequestDownload:				len = 8;	break;
		case RequestTransferExit:			len = 1;	break;
		case EcuReset:						len = 2;	break;
		case TesterPresent:					len = 1;	break;
		case ReadMemoryByAddress:			len = 5;	break;
		default:							len = -1;	break;
	}
	return len > 0 ? ComposeCmd(id, cmd, len) : 0;
}
DWORD CChild::ComposeCmd(BYTE *cmd, UINT len)
{
	//�˺���û���趨�����֣���������cmdָ��ĵ�һ���ֽڡ�
	DWORD ret = ComposeCmd(cmd[0], cmd + 1, len);
	memmove(cmd, cmd + 1, ret - 1);
	return ret;
}
DWORD CChild::ComposeCmd(BYTE id, BYTE* cmd, UINT len)
{
	//id ��K�ߵ������֡�
	//cmd ָ��һ���ַ��������ô˺���ǰװ���������ֺ�����ݣ��������ú�ָ������Ҫ���͵����
	//len �Ǵ�����ĳ��ȣ�������ͷ�ļ���У���롣
	//����ֵ����������ĳ��ȣ�����ͷ�ļ���У���롣
	BYTE* tmp = new BYTE[len];
	tmp[0] = id;
	for (int i = 1; i < len; i ++)
		tmp[i] = cmd[i - 1];
	cmd[0] = 0x80;
	cmd[1] = 0x11;
	cmd[2] = 0xF1;
	BYTE sum = cmd[0] + cmd[1] + cmd[2] + len;

	DWORD ret;
	LONG n;
	if (len < 64)//������ݳ��ȿ���ʹ�ö̸�ʽ
	{
		cmd[0] += len;
		ret = len + 4;
		n = 3;
	}
	else//��Ҫ��һ�����ݳ����ֽ�
	{
		cmd[3] = len;
		ret = len + 5;
		n = 4;
	}
	for (int i = 0; i < len; i ++)
	{
		cmd[n ++] = tmp[i];
		sum += tmp[i];
	}
	cmd[n] = sum;
	delete tmp;

	//��������֮��������ܳ��ȡ�
	return ret;	
}

BOOL CChild::ReadAddress(DWORD address, BYTE size, BOOL offset)
{
	BYTE cmd[16];
	cmd[0] = (BYTE)(address >> 16);
	cmd[1] = (BYTE)((address >> 8) & 0xFF);
	cmd[2] = (BYTE)(address & 0xFF);
	cmd[3] = size;
	LONG len = ComposeCmd(ReadMemoryByAddress, cmd);
	if (m_pFCom->SendHex(cmd, len) == FALSE)
		return FALSE;
	LONG count = m_pFCom->Receive(32, -1, 256);
	if (m_pFCom->GetRBuf(offset + 3) != ReadMemoryByAddress + 0x40 && m_pFCom->GetRBuf(offset + 5) != 1)
		return FALSE;
	return TRUE;
}
BOOL CChild::ReadVersionD(CHAR* info)
{
	//�պ�ECU���Ӽ̵�����
	m_pTCom->Receive(256, 0, 128);//(clear receive buffer)
	m_pTCom->SendCmd("0x038500");
	::Sleep(2000);
	m_pFCom->Receive(256, 0, 256);//(ignore relay noises)
	m_pTCom->Receive(3, 0, 10000);

	//���ó�ʼ�����ʡ�
	m_pFCom->SetBRate(240);
	BYTE cmd[16];
	cmd[0] = 0xE0;
	if (m_pFCom->SendHex(cmd, 1) == FALSE)
		return FALSE;
	::Sleep(64);

	//����K�߲����ʡ�
	m_pFCom->SetBRate(10400);
	LONG len = ComposeCmd(StartCommunication, cmd);
	if (m_pFCom->SendHex(cmd, len) == FALSE)
		return FALSE;
	LONG count = m_pFCom->Receive(32, -1, 256);
	if (m_pFCom->GetRBuf(9) == 0xEF && m_pFCom->GetRBuf(10) == 0x8F)
		len = 9;
	else if (m_pFCom->GetRBuf(4) == 0xEF && m_pFCom->GetRBuf(5) == 0x8F)
		len = 0;
	else
		return FALSE;

	//��ȡ���ݰ汾��
	if (ReadAddress(0x92000, 8, len) == FALSE)
		return FALSE;
	memcpy(m_szDataVer, m_pFCom->GetRBuf() + 4 + len, m_pFCom->GetRLen() - 4 - len);
	m_szDataVer[m_pFCom->GetRLen() - 4 - len] = 0;
	//��ȡ����汾��
	if (ReadAddress(0xA005C, 12, len) == FALSE)
		return FALSE;
	memcpy(m_szProgVer, m_pFCom->GetRBuf() + 4 + len, m_pFCom->GetRLen() - 4 - len);
	m_szProgVer[m_pFCom->GetRLen() - 4 - len] = 0;

	//����汾��Ϣ��
	m_pParam->flog.SaveLog(m_pParam->slog, ",Program");
	m_pParam->flog.SaveLog(m_pParam->slog, info);
	m_pParam->flog.SaveLog(m_pParam->slog, ": ");
	m_pParam->flog.SaveLog(m_pParam->slog, m_szProgVer);
	m_pParam->flog.SaveLog(m_pParam->slog, ",Data");
	m_pParam->flog.SaveLog(m_pParam->slog, info);
	m_pParam->flog.SaveLog(m_pParam->slog, ": ");
	m_pParam->flog.SaveLog(m_pParam->slog, m_szDataVer);

	//�Ͽ�ECU���Ӽ̵�����
	::Sleep(1000);
	m_pTCom->SendCmd("0x038600");
	::Sleep(2000);
	return TRUE;
}
BOOL CChild::ReadVersionI(CHAR* info)
{
	//��ȡ�汾��Ϣ��
	m_pTCom->Receive(256, 0, 256);
	m_pTCom->SendCmd("0x038A00");
	if (m_pTCom->Receive(3, 0, 5000) != 3 || m_pTCom->GetRBuf(0) != 0x03 || m_pTCom->GetRBuf(1) != 0x8A || m_pTCom->GetRBuf(2) != 0x0F)
		return FALSE;

	//���հ汾��Ϣ��
	m_pTCom->SendCmd("0x038A01");
	m_pTCom->Receive(32, 16, 512);
	strcpy(m_szProgVer, (CHAR*)m_pTCom->GetRBuf() + 3);
	m_pTCom->SendCmd("0x038A02");
	m_pTCom->Receive(32, 16, 512);
	strcpy(m_szDataVer, (CHAR*)m_pTCom->GetRBuf() + 3);

	//����汾��Ϣ��
	m_pParam->flog.SaveLog(m_pParam->slog, ",Program");
	m_pParam->flog.SaveLog(m_pParam->slog, info);
	m_pParam->flog.SaveLog(m_pParam->slog, ": ");
	m_pParam->flog.SaveLog(m_pParam->slog, m_szProgVer);
	m_pParam->flog.SaveLog(m_pParam->slog, ",Data");
	m_pParam->flog.SaveLog(m_pParam->slog, info);
	m_pParam->flog.SaveLog(m_pParam->slog, ": ");
	m_pParam->flog.SaveLog(m_pParam->slog, m_szDataVer);
	return TRUE;
}
BOOL CChild::CheckVersion(CHAR* ver, CHAR* prog, CHAR* data)
{
	//��ȡ����汾�����ݰ汾��
	if (ReadVersionI(ver) == FALSE)
	{
		//��ʾ������Ϣ��
		sprintf(m_szOutput + strlen(m_szOutput), "\r\n��ȡ�汾%sʧ�ܡ�", ver);
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		//���������Ϣ��
		m_pParam->flog.SaveLog(m_pParam->slog, ",Faild when reading version");
		m_pParam->flog.SaveLog(m_pParam->slog, ver);
		m_pParam->flog.SaveLog(m_pParam->slog, ". ");
		return FALSE;
	}
	//�Ƚϳ�ʼ����汾�����ݰ汾��
	else if (strcmp(prog, m_szProgVer) != 0 || strcmp(data, m_szDataVer) != 0)
	{
		//��ʾ������Ϣ��
		if (strcmp(prog, m_szProgVer) != 0)
			sprintf(m_szOutput + strlen(m_szOutput), "\r\n����汾%sΪ��%s��������Ҫ��", ver, m_szProgVer);
		if (strcmp(data, m_szDataVer) != 0)
			sprintf(m_szOutput + strlen(m_szOutput), "\r\n���ݰ汾%sΪ��%s��������Ҫ��", ver, m_szDataVer);
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		//���������Ϣ��
		m_pParam->flog.SaveLog(m_pParam->slog, ",Version");
		m_pParam->flog.SaveLog(m_pParam->slog, ver);
		m_pParam->flog.SaveLog(m_pParam->slog, " is not right. ");
		return FALSE;
	}
	return TRUE;
}
//-------------------------------------------------------------
LONG CChild::FlashECU()
{
	//��ȡECU���кš�(����ǰ��ȡһ�Σ������ȡһ�Σ�����ģ�飬��ì��)
	//BYTE QUERY_SERIAL[] = "What's Your Serial Number?";
	//if (SendEcuMsg(QUERY_SERIAL, sizeof(QUERY_SERIAL), 64, -1, 0) < 0)
	//	return 10000;
	//if (m_pFCom->GetRLen() == 0)
	//	return 10001;
	//CHAR serial[DEF_MAX_NAME];
	//memcpy(serial, m_pFCom->GetRBuf(), m_pFCom->GetRLen());

	//���ò����ʡ�
	m_pFCom->SetBRate(9600);

	//��ʼˢдECU����
	LONG ret = 0;
	m_nStep = 0;
	BYTE CMD00[] = {0x00};
	if (SendEcuMsg(CMD00, sizeof(CMD00), 2, 1, 0xD5) < 0)
		return 20000;
	//
	ret = KFlasherChipID();
	if (ret != 0)
		return 30000 + ret;
	ret = KFlasherMonitor4B();
	if (ret != 0)
		return 40000 + ret;
	ret = KFlasherTestBaud();
	if (ret != 0)
		return 50000 + ret;
	ret = KFlasherErase();
	if (ret != 0)
		return 60000 + ret;
	ret = KFlasherCheckBank();
	if (ret != 0)
		return 70000 + ret;
	ret = KFlasherSetBaud();
	if (ret != 0)
		return 80000 + ret;
	ret = KFlasherTransBin();
	if (ret != 0)
		return 90000 + ret;

	//�ָ�ECU���кš�
	//if (SendEcuMsg(serial, sizeof(serial), 64, -1, 0) < 0)
	//	return 10003;

	//ˢд�ɹ�������ˢд�ܲ�����
	return 0 - m_nStep;
}
LONG CChild::KFlasherChipID(void)
{
	BYTE FLASH_STAGE1[28] = {0xE6,0xF0,0x9A,0xF6,0xE6,0xF1,0x9A,0x00,0x9A,0xB7,0xFE,0x70,0xF3,0xF6,0xB2,0xFE,0xD7,0x00,0x00,0x00,0x89,0x60,0x7E,0xB7,0x28,0x11,0x3D,0xF6};
	BYTE CHIP_ID_ADDR[4] = {0xFA,0x00,0x00,0xF6};
	//����STAGE1��
	if (SendEcuMsg(FLASH_STAGE1, sizeof(FLASH_STAGE1), 28, -1, 0) <= 0)
		return 1000;
	//����StartChipID�Ĵ洢��ַ��
	if (SendEcuMsg(CHIP_ID_ADDR, sizeof(CHIP_ID_ADDR), 4, -1, 0) <= 0)
		return 2000;

	//����StartChipID��
	LONG len = m_pParam->chipid.GetRealSize();
	for (int i = 0; (i + 1) * 28 < len; i ++)
	{
		if (SendEcuMsg(m_pParam->pchipid + 28 * i, 28, 28, -1, 0) <= 0)
			return 3000 + i;
	}
	if (SendEcuMsg(m_pParam->pchipid + len / 28 * 28, len % 28, 17, -1, 0) <= 0)
		return 4000;
	return 0;
}
LONG CChild::KFlasherMonitor4B(void)
{
	BYTE FLASH_STAGE3[28] = {0xE6,0xF0,0xA8,0xE7,0xE6,0xF1,0xA8,0x07,0x9A,0xB7,0xFE,0x70,0xF3,0xF6,0xB2,0xFE,0xD7,0x00,0x00,0x00,0x89,0x60,0x7E,0xB7,0x28,0x11,0x3D,0xF6};
	BYTE MONITOR_ADDR[4] = {0xFA,0x00,0x00,0xE0};
	//����E000 ~ E7A8���ĳ���
	if (SendEcuMsg(FLASH_STAGE3, sizeof(FLASH_STAGE3), 28, -1, 0) <= 0)
		return 1000;
	//����Monitor4b�Ĵ洢��ַ��
	if (SendEcuMsg(MONITOR_ADDR, sizeof(MONITOR_ADDR), 4, -1, 0) <= 0)
		return 2000;

	//����Monitor4B��
	LONG len = m_pParam->monitor.GetRealSize();
	LONG i = 0;
	while ((i + 1) * 28 < len)
	{
		if (SendEcuMsg(m_pParam->pmonitor + 28 * i, 28, 28, -1, 0) <= 0)
			return 3000 + i;
		i ++;
	}
	if (SendEcuMsg(m_pParam->pmonitor + 28 * i, len - 28 * i, 37, -1, 0) <= 0)
		return 4000;
	return 0;
}
LONG CChild::KFlasherTestBaud(void)
{
	BYTE CMDB7[]	 = {0xB7};
	BYTE CMD1000[]	 = {0x10, 0x00};
	BYTE CMD0100D0[] = {0x01, 0x00, 0xD0, 0xE3, 0x00, 0x00, 0x2A, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00};
	BYTE CMDFECA[]	 = {0xFE, 0xCA};
	BYTE CMD01000A[] = {0x01, 0x00, 0x0A, 0xFE, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00};
	BYTE CMD0200[]	 = {0x02, 0x00};
	BYTE CMD00[]	 = {0x00};
	BYTE CMD2A00[]	 = {0x2A, 0x00};
	BYTE CMD0600[]	 = {0x06, 0x00};
	BYTE CMD04004D[] = {0x04, 0x00, 0x4D, 0x00, 0x4D, 0x00};
	SEcuMsg msgs[]  = {
		//index	value	len	rec	cmd
		{1,		0x7B,	1,	2,	CMDB7},		//0
		{-1,	0,		2,	2,	CMD1000},	//1
		{16,	0xF6,	16,	18,	CMD0100D0},	//2
		{50,	0x5A,	2,	51,	CMDFECA},	//3
		{1,		0x7B,	1,	2,	CMDB7},		//4
		{-1,	0,		2,	2,	CMD1000},	//5
		{16,	0x2B,	16,	18,	CMD01000A},	//6
		{2,		0x40,	2,	13,	CMDFECA},	//7
		{1,		0x7B,	1,	2,	CMDB7},		//8
		{-1,	0,		2,	2,	CMD0200},	//9
		{2,		0x04,	2,	4,	CMD0200},	//10
		{2,		0xA5,	2,	3,	CMDFECA},	//11
		{-1,	0,		1,	3,	CMD00},		//12
		{2,		0x5A,	2,	3,	CMD2A00},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD0600},
		{6,		0xA4,	6,	8,	CMD04004D},
		{2,		0xA5,	2,	3,	CMDFECA},
		{-1,	0,		-1,	0,	NULL},
	};
	//����Ԥ������Ϣ��
	int i = 0;
	while (msgs[i].len != -1)
	{
		if (SendEcuMsg(msgs[i].cmd, msgs[i].len, msgs[i].receive, msgs[i].index, msgs[i].value) <= 0)
			return 1000 + i;
		if (i == 12 && m_pFCom->GetRBuf(2) != 0x15 && m_pFCom->GetRBuf(2) != 0x16)//CMD00
			return 1000 + i;
		i ++;
	}
	//����0x00~FF��Ϣ��
	BYTE cmd = 0;
	while (cmd < 0xFF)
	{
		if (SendEcuMsg(&cmd, 1, 2, 1, (0xFF - cmd)) <= 0)
			return 2000 + cmd;
		cmd ++;
	}
	if (SendEcuMsg(&cmd, 1, 3, 1, (0xFF - cmd)) <= 0)
		return 3000;
	if (m_pFCom->GetRBuf(2) != 0x5A)
		return 4000;
	return 0;
}
LONG CChild::KFlasherErase(void)
{
	BYTE CMDB7[]	 = {0xB7};
	BYTE CMD0400[]	 = {0x04, 0x00};
	BYTE CMD0201FF[] = {0x02, 0x01, 0xFF, 0x0F};
	BYTE CMDFECA[]	 = {0xFE, 0xCA};
	BYTE CMD005A[]	 = {0x00, 0x5A};
	SEcuMsg msgs[]  = {
		//index	value	len	rec	cmd
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD0400},
		{4,		0x15,	4,	6,	CMD0201FF},
		{3,		0x5A,	2,	4,	CMDFECA},
		//{1,	0x5A,	0,	2,	NULL},//(���һ��FECA��Ϣ����Ҫ�������Σ���һ��֮���Լ5��Ż��յ��ڶ��λظ����ڶ��ν��յ�����Ϣ����Ϊ0x005A��)
		{-2,	0,		-1,	0,	NULL},
	};
	int i = 0;
	while (msgs[i].len != -1)
	{
		if (SendEcuMsg(msgs[i].cmd, msgs[i].len, msgs[i].receive, msgs[i].index, msgs[i].value) <= 0)
			return 1000 + i;
		i ++;
	}
	return 0;
}
LONG CChild::KFlasherCheckBank(void)
{
	BYTE CMDB7[]	 = {0xB7};
	BYTE CMD1000[]	 = {0x10, 0x00};
	BYTE CMD010000[] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x20, 0x00, 0xff, 0xff, 0x00, 0x00};
	BYTE CMDFECA[]	 = {0xFE, 0xCA};
	BYTE CMD012001[] = {0x01, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD014001[] = {0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD016001[] = {0x01, 0x00, 0x00, 0x60, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD018001[] = {0x01, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD00[]	 = {0x00};
	BYTE CMD010002[] = {0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x80, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD010003[] = {0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x80, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD010004[] = {0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x80, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD010005[] = {0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x80, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD010006[] = {0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x80, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD010007[] = {0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x80, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD010008[] = {0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x80, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	SEcuMsg msgs[]  = {
		//index	value	len	rec	cmd
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0x40,	16,	18,	CMD010000},
		{8,		0x5A,	2,	9,	CMDFECA},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0x60,	16,	18,	CMD012001},
		{8,		0x5A,	2,	9,	CMDFECA},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0x80,	16,	18,	CMD014001},
		{8,		0x5A,	2,	9,	CMDFECA},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xA0,	16,	18,	CMD016001},
		{8,		0x5A,	2,	9,	CMDFECA},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xF0,	16,	18,	CMD018001},
		{8,		0x5A,	2,	9,	CMDFECA},
		//
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xB1,	16,	18,	CMD010002},
		{8,		0x5A,	2,	9,	CMDFECA},
		//{6,	0x5A,	0,	7,	NULL},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xB2,	16,	18,	CMD010003},
		{8,		0x5A,	2,	9,	CMDFECA},
		//{6,	0x5A,	0,	7,	NULL},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xB3,	16,	18,	CMD010004},
		{8,		0x5A,	2,	9,	CMDFECA},
		//{6,	0x5A,	0,	7,	NULL},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xB4,	16,	18,	CMD010005},
		{8,		0x5A,	2,	9,	CMDFECA},
		//{6,	0x5A,	0,	7,	NULL},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xB5,	16,	18,	CMD010006},
		{8,		0x5A,	2,	9,	CMDFECA},
		//{6,	0x5A,	0,	7,	NULL},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xB6,	16,	18,	CMD010007},
		{8,		0x5A,	2,	9,	CMDFECA},
		//{6,	0x5A,	0,	7,	NULL},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xB7,	16,	18,	CMD010008},
		{8,		0x5A,	2,	9,	CMDFECA},
		//{6,	0x5A,	0,	7,	NULL},
		{-1,	0,		-1,	0,	NULL},
	};
	//����Ԥ������Ϣ��
	int i = 0;
	while (msgs[i].len != -1)
	{
		if (SendEcuMsg(msgs[i].cmd, msgs[i].len, msgs[i].receive, msgs[i].index, msgs[i].value) <= 0)
			return 1000 + i;
		i ++;
	}
	return 0;
}
LONG CChild::KFlasherSetBaud(void)
{
	BYTE CMD00[]	 = {0x00};
	BYTE CMDB7[]	 = {0xB7};
	BYTE CMD0600[]	 = {0x06, 0x00};
	BYTE CMD04000C[] = {0x04, 0x00, 0x0C, 0x00, 0x4D, 0x00};
	BYTE CMDFECA[]	 = {0xFE, 0xCA};
	SEcuMsg msgs[]  = {
		//index	value	len	rec	cmd
		{-1,	0,		1,	2,	CMD00},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD0600},
		{6,		0x63,	6,	8,	CMD04000C},
		{2,		0xA5,	2,	3,	CMDFECA},
		//{5,		0x00,	6,	6,	CMD04000C},
		//{1,		0x80,	2,	2,	CMDFECA},
		{-1,	0,		-1,	0,	NULL},
	};
	//����Ԥ������Ϣ��
	int i = 0;
	while (msgs[i].len != -1)
	{
		if (SendEcuMsg(msgs[i].cmd, msgs[i].len, msgs[i].receive, msgs[i].index, msgs[i].value) <= 0)
			return 1000 + i;
		i ++;
	}

	//���ø߲����ʡ�
	m_pFCom->SetBRate(57600);
	//����0x00~FF��Ϣ��
	BYTE cmd = 0;
	while (cmd < 0xFF)
	{
		if (SendEcuMsg(&cmd, 1, 2, 1, (0xFF - cmd)) <= 0)
			return 2000 + cmd;
		cmd ++;
	}
	if (SendEcuMsg(&cmd, 1, 3, 1, (0xFF - cmd)) <= 0)
		return 3000;
	if (m_pFCom->GetRBuf(2) != 0x5A)
		return 4000;
	return 0;
}
LONG CChild::KFlasherTransBin()
{
	for (int i = 0; i < m_pParam->target.GetIndexLen(); i ++)
	{
		DWORD maddr = m_pParam->target.GetIndexAddr(i);
		DWORD msize = m_pParam->target.GetIndexSize(i);
		if (maddr == 0 && msize == 0)
			continue;//(ignore blank index)
		//
		BYTE CMDB7[] = {0xB7};
		if (SendEcuMsg(CMDB7, sizeof(CMDB7), 2, 1, 0x7B) <= 0)
			return 1100 + i;
		BYTE CMD0A00[] = {0x0A, 0x00};
		if (SendEcuMsg(CMD0A00, sizeof(CMD0A00), 2, -1, 0) <= 0)
			return 1200 + i;
		BYTE cmd[0x0100];
		cmd[0] = 0x06;
		cmd[1] = 0x01;
		cmd[2] = maddr % 0x0100;
		cmd[3] = (maddr >>  8) % 0x0100;
		cmd[4] = (maddr >> 16) % 0x0100 == 0 ? 1 : (maddr >> 16) % 0x0100;
		cmd[5] = 0x00;
		cmd[6] = msize % 0x0100;
		cmd[7] = (msize >>  8) % 0x0100;
		cmd[8] = (msize >> 16) % 0x0100;
		cmd[9] = 0x00;
		//
		DWORD CRC = 0;
		for (int j = 0; j < 10; j ++)
			CRC += cmd[j];
		CRC += 0x0A;
		if (SendEcuMsg(cmd, 10, 12, 10, CRC % 0x0100) <= 0)
			return 1400 + i;
		if (m_pFCom->GetRBuf(11) != CRC / 0x0100)
			return 1500 + i;
		//
		BYTE CMDFECA[] = {0xFE, 0xCA};
		if (SendEcuMsg(CMDFECA, sizeof(CMDFECA), 2, -1, 0) <= 0)
			return 1600 + i;
		//
		CRC = 0;
		LONG pos = 0;
		if (msize >= 96)
		{
			while (pos < msize - 96)
			{
				for (int j = 0; j < 96; j++)
				{
					cmd[j] = m_pParam->target.GetBinData(maddr + pos + j);
					CRC += cmd[j];
				}
				if (SendEcuMsg(cmd, 96, 96, -1, 0) <= 0)
					return 2000 + 200 * i + pos / 96;
				pos += 96;
			}
		}
		if (pos < msize)
		{
			LONG len = 0;
			if ((msize - pos) % 4 == 0 )
				len = msize - pos;
			else
				len = ((msize - pos) / 4 + 1) * 4;
			for (int j = 0; j < len; j ++)
			{
				cmd[j] = m_pParam->target.GetBinData(maddr + pos + j);
				CRC += cmd[j];
			}
			if (SendEcuMsg(cmd, len, len + 3, len + 2, 0x5A) <= 0)
				return 1700 + i;
			if (m_pFCom->GetRBuf(len) != (CRC % 0x0100) || m_pFCom->GetRBuf(len + 1) != ((CRC >> 8) % 0x0100))
				return 1800 + i;
		}
	}
	return 0;
}
//-------------------------------------------------------------
void CChild::OnEnChangeSerial()
{
	//����������64ms����timer��ʼʱ��ᱻ���ã�����������64ms����timer����ִ�У������������кŽ��и�ʽ����
	//ʹ��ɨ��������ķ�ʽ��������ᳬ��16ms������64ms����Ϊɨ���ѽ�����
	SetTimer(2, 64, NULL);
}
void CChild::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)//�Զ����ӽ�������
	{
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->StepIt();
	}
	else if (nIDEvent == 2)//�Զ���ʽ�����кš�
	{
		KillTimer(2);
		FormatSerial();
	}
}
void CChild::SetControlInfo(CWnd* pwnd, LONG anchor)
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
		m_pAnchor[i][3] = rt.bottom;
	}
	else if (anchor & RESIZE_VER2)
	{
		RECT rt;
		pwnd->GetWindowRect(&rt);
		m_pAnchor[i][3] = rt.bottom - rt.top;
	}
}
void CChild::OnSize(UINT type, int cx, int cy) 
{
	if (type == SIZE_MINIMIZED)
		return;
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
		if (m_pAnchor[i][1] & ANCHOR_RIGHT)//move to right
		{
			rt.left += dx - (rt.right - pos);
			rt.right += dx - (rt.right - pos);
		}
		else if (m_pAnchor[i][1] & ANCHOR_LEFT2)//move to center
		{
			rt.right = rt.left + m_pAnchor[i][2] + m_szDelta.cx/2;
			rt.left = pos - rt.left + m_pAnchor[i][2] + m_szDelta.cx/2;
		}

		//vertical
		pos = rt.bottom;
		if (m_pAnchor[i][1] & RESIZE_VER)
			rt.bottom += dy;
		else if (m_pAnchor[i][1] & RESIZE_VER2)
			rt.bottom = rt.top + m_pAnchor[i][3] + m_szDelta.cy/2;
		if (m_pAnchor[i][1] & ANCHOR_BOTTOM)//move to bottom
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