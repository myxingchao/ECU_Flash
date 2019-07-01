// FlasherDlg.cpp
#include "stdafx.h"
#include "Resource.h"
#include "Child.h"
#include "ComPort.h"
#include "Printer.h"

#define DEF_MAX_TEST_TIME	90000	//(测试超时时间，两次测试进度更新的最大间隔)
#define DEF_MAX_EDIT_LEN	5 * 256 //display error code with the format like: 189, 140, ...
#define DEF_FLASH_TIMEOUT	10000	//because 0xFECA command in erase process needs 6187ms to receive.
#define DEF_CHECK_INTERVAL	1000	//(连接确认周期)
#define WM_PROGRESS_PLUS	WM_USER + 1001

HANDLE CChild::m_hPrint = INVALID_HANDLE_VALUE;
CChild::CChild(CWnd* pParent /*=NULL*/)
	: CDialog(CChild::IDD, pParent)
{
	memset(&m_sLabel, NULL, sizeof(m_sLabel));
	memset(m_pAnchor, NULL, sizeof(m_pAnchor));
	//手动重置事件在Wait之后状态不变，可以再次通过Wait，除非手动调用Reset；
	//自动重置事件在Wait之后自动重置，下次调用Set之前，不能再次通过Wait。
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
	//强制结束刷写线程。
	if (m_pProcThd != NULL)
		::TerminateThread(m_pProcThd->m_hThread, 0);
	//等待连接线程结束。
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
	if (param != NULL)//(启用窗口)
	{
		EnableWindow(TRUE);
		GetDlgItem(IDC_SERIAL)->SetWindowText("");
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(0);
		m_szOutput[0] = NULL;
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		GetDlgItem(IDC_START)->EnableWindow(FALSE);
		GetDlgItem(IDC_START)->SetWindowText("等待连接");
		//设置设备名称。
		CHAR text[64];
		sprintf(text, "         设备名称：%s (%s)", name, tcom->GetCom());
		GetDlgItem(IDC_TITLE2)->SetWindowText(text);
		//设置通信参数。
		m_pParam = param;
		m_pTCom = tcom;
		m_pFCom = fcom;
		//开启线程等待ECU连接。
		//(虽然exit是自动事件，只要使用一次就会自动无效，但是显示Config窗口之前，设置SetReady为FALSE时，会设置一次exit，而此时线程并未运行，
		// 直到关闭Config窗口想要开启wait线程时，exit标志仍然有效，所以必须在启动wait线程之前手动reset，否则会立即退出。)
		::ResetEvent(m_hExit);
		::SetEvent(m_hStop);//start
		m_pWaitThd = AfxBeginThread(WaitThd, (LPVOID)this);
	}
	else if (m_pParam != NULL)//(禁用窗口)
	{
		EnableWindow(FALSE);
		GetDlgItem(IDC_SERIAL)->SetWindowText("");
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(0);
		m_szOutput[0] = NULL;
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		Invalidate();//(the display will be delayed without this)
		UpdateWindow();
		GetDlgItem(IDC_START)->SetWindowText("未使用");
		//设置设备名称。
		CHAR text[64];
		sprintf(text, "         设备名称：", name, tcom->GetCom());
		GetDlgItem(IDC_TITLE2)->SetWindowText(text);
		//让等待线程继续运行，直到退出。
		if (m_pParam != NULL)
		{
			::SetEvent(m_hExit);
			::SetEvent(m_hStop);//start
			if (m_pWaitThd != NULL)
				::WaitForSingleObject(m_pWaitThd->m_hThread, DEF_CHECK_INTERVAL + 32);
			m_pWaitThd = NULL;
			//设置通信参数。
			m_pParam = param;
			m_pTCom = tcom;
			m_pFCom = fcom;
		}
	}
}
BOOL CChild::OnInitDialog()
{
	CDialog::OnInitDialog();

	//设置信息栏字体。
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
	//子类化开始按钮和边框。(边框无法自绘？)
	m_btnStart.SubclassDlgItem(IDC_START, this);
	m_btnStart.ModifyStyle(0, BS_OWNERDRAW, 0);
	m_btnStart.SetColor(m_dwColor);
	//m_frmChild.SubclassDlgItem(IDC_FRAME, this);
	//m_frmChild.ModifyStyle(0, BS_OWNERDRAW, 0);
	//m_frmChild.SetColor(m_dwColor);

	//调整控件位置。
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

	//设置控件自动排布。
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
	GetDlgItem(IDC_START)->SetWindowText("开始");
	return TRUE;
}
UINT CChild::WaitThd(LPVOID param)
{
	((CChild*)param)->WaitThd();
	return 0;
}
void CChild::WaitThd()
{
	//功能台刚启动时可能会因为ECU连接，主动发送0x038D00，需要清理。
	m_pTCom->Receive(256, 0, 256);
	m_pTCom->SendCmd("0x038D00");
	while (true)
	{
		//等待ECU连接。
		while (true)
		{
			::WaitForSingleObject(m_hStop, INFINITE);
			//等待之前先检查hExit，如果被设置，则退出线程。
			if (::WaitForSingleObject(m_hExit, 0) != WAIT_TIMEOUT)//set
				return;
			//等待接收断开/连接消息。
			if (m_pTCom->Receive(3, 128, DEF_CHECK_INTERVAL) >= 3 && m_pTCom->GetRBuf(0) == 0x03 && m_pTCom->GetRBuf(1) == 0x8D)
				break;
		}

		//更新控件状态。
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(0);
		GetDlgItem(IDC_SERIAL)->SetWindowText("");
		m_szOutput[0] = NULL;
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		GetDlgItem(IDC_START)->EnableWindow(FALSE);
		if (m_pTCom->GetRBuf(2) == 0x01)//卸下ECU
		{
			GetDlgItem(IDC_START)->SetWindowText("等待连接");
		}
		else//(m_pTCom->GetRBuf(2) == 0x00)//连接ECU
		{
			GetDlgItem(IDC_START)->SetWindowText("等待输入");
			//读取并显示序列号。
			ReadSerialNo();
		}

		//开启其他ECU等待线程。
		if (m_bNewECU == FALSE)
		{
			m_bNewECU = TRUE;
			Sleep(1000);//(用来撕标签)
			::SetEvent(CChild::m_hPrint);
		}
	}
}
BOOL CChild::ReadSerialNo()
{
	//读取ECU序列号。
	ASSERT(m_pFCom->Receive(256, 0, 128) == 0);//(clear receive buffer)
	m_pFCom->SendAsc("SERIAL NO.");
	m_pFCom->Receive(128, 16, 1000);
	//sprintf((CHAR*)m_pFCom->GetRBuf(), "2361201001-20090235-M6006-KHH43M111-X2GIT013-00000000-131228-14010180065");//ooo
	if (strcmp((CHAR*)m_pFCom->GetRBuf(), "SERIAL NO.") == 0)
		m_pFCom->GetRBuf()[0] = NULL;
	if (m_pFCom->GetRBuf(0) != NULL)
	{
		sprintf(m_szOutput, "序列号已读取，请开始测试。");
		//将序列号分行显示。
		GetDlgItem(IDC_SERIAL)->SetWindowText((CHAR*)m_pFCom->GetRBuf());
		FormatSerial();
		return TRUE;
	}
	else//(显示错误信息)
	{
		sprintf(m_szOutput, "无法读取序列号，请扫描二维码输入。");
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		return FALSE;
	}
}
void CChild::FormatSerial()
{
	//确认是否已经格式化。
	CHAR text[DEF_MAX_EDIT_LEN];
	GetDlgItem(IDC_SERIAL)->GetWindowText(text, sizeof(text));
	if (strstr(text, "\r\n") == text + strlen(text) - 2)
		text[strlen(text) - 2] = 0;
	if (strlen(text) == 0 || strstr(text, "\r\n") != NULL)
		return;
	//保存序列号，用于打印标签。
	strcpy(m_szSerial, text);
	//将序列号分行显示在编辑框中。
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
	//将开始按钮设为有效。
	GetDlgItem(IDC_START)->GetWindowText(text, sizeof(text));
	if (strcmp(text, "等待输入") == 0)
	{
		GetDlgItem(IDC_START)->SetWindowText("开始");
		GetDlgItem(IDC_START)->EnableWindow(TRUE);
	}
}
void CChild::OnEnSetfocusSerial()
{
	//点击序列号后，将已输入的序列号全选中，方便重新输入。
	((CEdit*)GetDlgItem(IDC_SERIAL))->SetSel(0, -1, TRUE);
}
//-------------------------------------------------------------
void CChild::OnBnClickedStart()
{
	CHAR text[16];
	GetDlgItem(IDC_START)->GetWindowText(text, sizeof(text));
	//测试尚未开始，开始测试&刷写。
	if (strcmp(text, "开始") == 0)
		m_pProcThd = AfxBeginThread(ProcThd, (LPVOID)this);
	else//测试已经结束，重新打印标签。
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
	//读取标签配置信息。
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
			//修改将文字信息为当天日期。
			SYSTEMTIME st;
			::GetLocalTime(&st);
			sprintf(label->ptext[i].text, "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
		}
		else if (label->ptext[i].type == 3)//number
		{
			//更新配置文件中的数字编号。
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
	case 0:	sprintf(m_szOutput + strlen(m_szOutput), "\r\n正常打印完毕。");					break;
	case 11:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：枚举错误。");			break;
	case 12:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：没有发现打印机。");	break;
	case 13:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：无法连接。");			break;
	case 14:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：试试重启打印机。");	break;
	case 15:
			sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：");
			char buf[3];
			int len;
			if (Get_StatusData(0, buf, sizeof(buf), &len, 3000) != SDK_SUCCESS);
			{
				sprintf(m_szOutput + strlen(m_szOutput), "试试重启打印机。");	
				break;
			}
			for (int i = 0; i < 8; i ++)
			{
				if (buf[0] & (1 << i) != 0)
				{
					switch (i)
					{
					case 0: sprintf(m_szOutput + strlen(m_szOutput), "打印，");				break;
					case 1: sprintf(m_szOutput + strlen(m_szOutput), "无标签，");			break;
					case 2: sprintf(m_szOutput + strlen(m_szOutput), "打印机暂停，");		break;
					case 3: sprintf(m_szOutput + strlen(m_szOutput), "打印机忙，");			break;
					case 4: sprintf(m_szOutput + strlen(m_szOutput), "未知错误，");			break;
					case 5: sprintf(m_szOutput + strlen(m_szOutput), "缺色带，");			break;
					case 6: sprintf(m_szOutput + strlen(m_szOutput), "缺纸，");				break;
					case 7: sprintf(m_szOutput + strlen(m_szOutput), "解释器忙，");			break;
					}
				}
				if (buf[1] & (1 << i) != 0)
				{
					switch (i)
					{
					case 0: sprintf(m_szOutput + strlen(m_szOutput), "打印头损坏，");		break;
					case 1: sprintf(m_szOutput + strlen(m_szOutput), "标签纸丢失标志，");	break;
					case 2: sprintf(m_szOutput + strlen(m_szOutput), "切刀超时，");			break;
					case 3: sprintf(m_szOutput + strlen(m_szOutput), "打印头抬起，");		break;
					case 4: sprintf(m_szOutput + strlen(m_szOutput), "未知错误，");			break;
					case 5: sprintf(m_szOutput + strlen(m_szOutput), "打印头过热，");		break;
					case 6: sprintf(m_szOutput + strlen(m_szOutput), "初始化标志位1，");	break;
					case 7: sprintf(m_szOutput + strlen(m_szOutput), "通讯错误，");			break;
					}
				}
			}
			m_szOutput[strlen(m_szOutput) - 1] = 0;//(delete the last ",")
			break;
	case 16:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：无法设置参数。");		break;
	case 17:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：出纸位置错误。");		break;
	case 19:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：无法进入打印状态。");	break;
	case 21:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：无法输出位图。");		break;
	case 22:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：无法输出点阵文字。");	break;
	case 23:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：无法输出矢量文字。");	break;
	case 24:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：无法输出128条码。");	break;
	case 25:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：无法输出QR条码。");	break;
	case 41:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：无法打印。");			break;
	case 42:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：无法断开连接。");		break;
	default:sprintf(m_szOutput + strlen(m_szOutput), "\r\n打印机异常：详情不明。");			break;
	}
	GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
}
LONG CChild::PrintLabel(SLabel* label)
{
	//连接打印机。
	int len;
	CHAR buf[256];
	/*/不需要驱动名也能连接，枚举驱动名貌似没用？
	if (Enum_DeviceNameList(1, buf, sizeof(buf), &len, "SBTP") != SDK_SUCCESS)
		return 11;
	if (strlen(buf) == 0)
		return 12;*/
	USBPara pusb;
	//strcpy(pusb.cDevName, buf);	//"BTP-2300E3(U) 1"
	pusb.cDevName[0] = NULL;
	pusb.iUSBMode = 0;			//0:API模式，1:类模式。
	pusb.iDevID = -1;			//-1:直接打开，0:通过内部ID打开。
	int hUSB = Comm_OpenPort(PORT_USBDEVICE, &pusb, sizeof(pusb), 0, NULL);
	if (hUSB < 0)
		return 13;
	if (Get_StatusData(hUSB, buf, sizeof(buf), &len, 3000) != SDK_SUCCESS)
		return 14;
	if (buf[0] != 0 || buf[1] != 0)
		return 15;

	//设置打印机。
	BasePara pbase;
	pbase.iDPI = 200;			//200DPI
	pbase.iUnit = 1;			//毫米/10
	pbase.iOutMode = 3;			//撕离
	pbase.iPaperType = 3;		//标签纸
	pbase.iPrintMode = 2;		//热转印
	pbase.iAllRotateMode = 0;	//不旋转
	pbase.iAllMirror = 0;		//不镜像
	if (Set_BasePara(hUSB, &pbase, sizeof(pbase)) != SDK_SUCCESS)
		return 16;
	if (Set_OutPosition(hUSB, 300) != SDK_SUCCESS)//出纸偏移距离
		return 17;

	//打印错误信息。
	if (label->nwidth == 0)//(no label info)
	{
		//设置标签模式。(如果将宽度设为750，则每行只能输出7个错误码，无法设置更多)
		if (Prt_EnterLabelMode(hUSB, 0, 1500, 0, 0, 2, 12) != SDK_SUCCESS)
			return 19;

		CHAR row[256];
		//设置打印序列号二维码。
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
		//设置打印文字位置。
		TruetypePrintPara txt;
		txt.iStartX = 150 + 70;//(不知道为什么是150，按理说应该是(1500 - 750) / 2 = 375)
		txt.iFontHeight = 30;
		txt.iFontWidth = 0;
		txt.iBold = 0;
		txt.iItalic = 0;
		txt.iUnderline = 0;
		txt.iRotate = 1;
		strcpy(txt.cFontName, "SimSun");
		//设置打印序列号文字。
		txt.cText = m_szSerial;
		txt.iStartY = 400;
		if (Prt_LabelPrintSetting(hUSB, PRINT_TREUTYPE, &txt, sizeof(txt), 0) != SDK_SUCCESS)
			return 23;
		//设置打印错误内容。
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
		row[49] = 0;//(每行最多输出49个字符，确保没有溢出)
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

		//开始打印。
		if (Prt_EndLabelAndPrint(hUSB, 1, 1, 1) != SDK_SUCCESS)
			return 41;
		//断开打印机。
		if (Comm_ClosePort(hUSB) != SDK_SUCCESS)
			return 42;
		return 0;
	}

	//设置标签模式。
	if (Prt_EnterLabelMode(hUSB, 0, label->nwidth, 0, 0, 2, 12) != SDK_SUCCESS)
		return 19;

	//取得程序路径。
	CHAR dir[_MAX_DIR];
	CHAR drive[_MAX_DRIVE];
	CHAR file[_MAX_FNAME];
	::GetModuleFileName(NULL, file, _MAX_PATH);
	_splitpath(file, drive, dir, file, NULL);
	//准备打印图形。
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

	//准备打印文字。
	for (int i = 0; i < label->ntext; i ++)
	{
		//使用内部点阵字体。
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
		else//使用系统矢量字体。
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

	//准备打印序列号。
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

	//准备打印二维码。
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

	//开始打印。
	if (Prt_EndLabelAndPrint(hUSB, 1, 1, 1) != SDK_SUCCESS)
		return 41;
	//断开打印机。
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
	GetDlgItem(IDC_START)->SetWindowText("正在测试");

	//断开ECU连接继电器。
	m_pTCom->SendCmd("0x038600");
	//LED常亮提示。
	m_pTCom->SendCmd("0x038C00");

	//执行功能测试。
	CHAR cmd[16];
	sprintf(cmd, "0x%02X87%s", 2 + strlen(m_pParam->smodal) / 2, m_pParam->smodal);
	m_pTCom->Receive(256, 0, 128);//(clear receive buffer)
	m_pTCom->SendCmd(cmd);

	//初始化进度条。
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetBkColor(CLR_DEFAULT);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetBarColor(CLR_DEFAULT);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetRange(0, 100);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(0);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetStep(1);

	//测试过程中更新进度条。
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
		//显示错误信息。
		sprintf(m_szOutput + strlen(m_szOutput), "\r\n测试超时。");
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		//保存故障信息。
		m_pParam->flog.SaveLog(m_pParam->slog, ",Testing Error: Timeout");
		return FALSE;
	}

	//查询测试结果。
	GetDlgItem(IDC_START)->SetWindowText("读取结果");
	m_pTCom->Receive(256, 0, 128);//(clear receive buffer)
	m_pTCom->SendCmd("0x038900");
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetRange(0, 100);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(0);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetStep(100 * 500 / 10000);
	SetTimer(1, 500, NULL);
	m_pTCom->Receive(4096, 64, 10000);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(100);
	KillTimer(1);
	
	if (m_pTCom->GetRBuf(0) == 0x02 && m_pTCom->GetRBuf(1) == 0x89)//测试成功
	{
		//保存成功信息到文件。
		sprintf(m_szOutput + strlen(m_szOutput), "\r\n测试成功。");
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		m_pParam->flog.SaveLog(m_pParam->slog, ",Test Succeeded");
		return TRUE;
	}
	else//测试失败，显示故障码并返回。
	{
		//显示故障码。
		sprintf(m_szOutput + strlen(m_szOutput), "\r\n测试失败，错误码: \r\n");
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
		//保存故障码到文件。
		m_pParam->flog.SaveLog(m_pParam->slog, ",");
		m_pParam->flog.SaveLog(m_pParam->slog, m_szOutput + pos);
		//return FALSE;
		return TRUE;//ooo
	}
}
BOOL CChild::ProcFlash()
{
	GetDlgItem(IDC_START)->SetWindowText("准备刷写");

	//设置进度条和提示信息。
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetBkColor(RGB(170, 190, 170));
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetBarColor(RGB(250, 190, 250));
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetRange(0, m_pParam->nstep);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(0);
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetStep(1);
	GetDlgItem(IDC_START)->SetWindowText("正在刷写");

	//出错时，最多重新刷写两次。
	LONG error;
	LONG limit = 3;
	while (limit > 0)
	{
		limit --;
		//闭合ECU连接继电器。
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
		else//继电器切换成功，开始刷写。
		{
			error = FlashECU();
		}
		//设置进度条满格。（可能在最后一个WM_PROGRESS_PLUS消息处理之前）
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(m_pParam->nstep);
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->Invalidate(FALSE);

		//断开ECU连接继电器。
		::Sleep(500);
		m_pTCom->SendCmd("0x038600");

		if (error <= 0)
		{
			//刷写成功，更新刷写步数。
			m_pParam->nstep = 0 - error;
			limit = -1;
		}
		else//(error > 0)
		{
			//显示故障信息。
			sprintf(m_szOutput + strlen(m_szOutput), "\r\n刷写失败，错误码: %05d。", error);
			GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
			//保存故障信息。
			m_pParam->flog.SaveLog(m_pParam->slog, ",Flashing Error: ");
			m_pParam->flog.SaveLog(m_pParam->slog, m_szOutput + strlen(m_szOutput) - 5);
			//提示重新刷写。
			if (limit > 0)
			{
				sprintf(m_szOutput + strlen(m_szOutput), "重新刷写...");
				GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
				m_pParam->flog.SaveLog(m_pParam->slog, ",Flash Again.");
				::Sleep(1000);
				((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(0);
			}
		}
	}
	if (limit == 0)
	{
		sprintf(m_szOutput + strlen(m_szOutput), "\r\n刷写次数超过限制，中止尝试。");
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		//保存成功信息到文件。
		m_pParam->flog.SaveLog(m_pParam->slog, ",Flashing Faild.");
		return FALSE;
	}
	else//(limit != 0)
	{
		sprintf(m_szOutput + strlen(m_szOutput), "\r\n刷写成功。");
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		//保存成功信息到文件。
		m_pParam->flog.SaveLog(m_pParam->slog, ",Flashing Succeeded.");
		return TRUE;
	}
}
void CChild::Process()
{
	//禁用主窗口设置按钮。
	GetParent()->PostMessage(WM_DISABLE_CONFIG, TRUE, 0);
	//开始检测，按钮不可用。
	GetDlgItem(IDC_START)->EnableWindow(FALSE);
	//暂停等待线程直到完成新的刷写工序。
	::ResetEvent(m_hStop);//stop
	//make sure the wait thread has been suspended.
	::Sleep(DEF_CHECK_INTERVAL + 32);

	//保存操作时间、操作者、序列号。
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CHAR text[DEF_MAX_EDIT_LEN];
	sprintf(text, "\r\n%04d/%02d/%02d %02d:%02d:%02d,%s,%s,", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, m_pParam->suser, m_szSerial);
	m_pParam->flog.SaveLog(m_pParam->slog, text);

	//检查版本、测试、刷写。
	BOOL ret;
	switch (m_pParam->nproc)
	{
	case 0://PCBA
		ret = ProcFlash()											== TRUE &&
			  ProcTest()											== TRUE &&
			  CheckVersion("1", m_pParam->sprog1, m_pParam->sdata1) == TRUE
			? TRUE : FALSE;
		break;
	case 1://初检
		ret = CheckVersion("0", m_pParam->sprog0, m_pParam->sdata0) == TRUE &&
			  ProcTest()											== TRUE
			? TRUE : FALSE;
		break;
	case 2://终检
		ret = CheckVersion("0", m_pParam->sprog0, m_pParam->sdata0) == TRUE &&
			  ProcTest()											== TRUE &&
			  ProcFlash()											== TRUE &&
			  CheckVersion("1", m_pParam->sprog1, m_pParam->sdata1) == TRUE
			? TRUE : FALSE;
		break;
	}

	//打印标签。
	if (m_pParam->nproc != 1)//(PCBA或终检)
	{
		GetDlgItem(IDC_START)->SetWindowText("排队...");
		::WaitForSingleObject(CChild::m_hPrint, INFINITE);
		m_sLabel.nwidth = 0;
		if (ret == TRUE)//(加载正常标签信息)
			LoadLabel(&m_sLabel);
		ShowPrintInfo(PrintLabel(&m_sLabel));
		GetDlgItem(IDC_START)->SetWindowText("重新打印");
		GetDlgItem(IDC_START)->EnableWindow(TRUE);
	}

	//使ECU端LED闪烁&声音提示。
	m_pTCom->SendCmd("0x038C02");
	::MessageBeep(MB_ICONQUESTION);

	//设置工序完成标志。
	m_bNewECU = FALSE;
	//开启等待线程监控卸下ECU。
	sprintf(m_szOutput + strlen(m_szOutput), "\r\n\r\n---请卸下ECU---");
	GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
	::SetEvent(m_hStop);//start
	//启用主窗口设置按钮。
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
	//更新进度条。
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
	//调用ComposeCmd有两种方式：
	//如果是固定长度，可以在此函数下定义长度，调用时只用在缓冲区中设置内部参数，函数调用的参数有2个，第一个是命令，第二个是缓冲区指针。
	//如果是可变长度，命令字和参数都要写入缓冲区，函数调用时第一个参数是缓冲区指针，第二个参数是命令长度。
	//命令长度始终是真正的命令长度，命令字和参数的个数，不算命令头和校验码。
	//一些固定长度的命令，此函数自动加入长度信息。
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
	//此函数没有设定命令字，命令字是cmd指向的第一个字节。
	DWORD ret = ComposeCmd(cmd[0], cmd + 1, len);
	memmove(cmd, cmd + 1, ret - 1);
	return ret;
}
DWORD CChild::ComposeCmd(BYTE id, BYTE* cmd, UINT len)
{
	//id 是K线的命令字。
	//cmd 指向一个字符串，调用此函数前装载有命令字后的数据，函数调用后指向整个要发送的命令。
	//len 是此命令的长度，不包括头文件和校验码。
	//返回值是整个命令的长度，包括头文件和校验码。
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
	if (len < 64)//如果数据长度可以使用短格式
	{
		cmd[0] += len;
		ret = len + 4;
		n = 3;
	}
	else//需要有一个数据长度字节
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

	//返回整理之后的命令总长度。
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
	//闭合ECU连接继电器。
	m_pTCom->Receive(256, 0, 128);//(clear receive buffer)
	m_pTCom->SendCmd("0x038500");
	::Sleep(2000);
	m_pFCom->Receive(256, 0, 256);//(ignore relay noises)
	m_pTCom->Receive(3, 0, 10000);

	//设置初始波特率。
	m_pFCom->SetBRate(240);
	BYTE cmd[16];
	cmd[0] = 0xE0;
	if (m_pFCom->SendHex(cmd, 1) == FALSE)
		return FALSE;
	::Sleep(64);

	//设置K线波特率。
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

	//读取数据版本。
	if (ReadAddress(0x92000, 8, len) == FALSE)
		return FALSE;
	memcpy(m_szDataVer, m_pFCom->GetRBuf() + 4 + len, m_pFCom->GetRLen() - 4 - len);
	m_szDataVer[m_pFCom->GetRLen() - 4 - len] = 0;
	//读取程序版本。
	if (ReadAddress(0xA005C, 12, len) == FALSE)
		return FALSE;
	memcpy(m_szProgVer, m_pFCom->GetRBuf() + 4 + len, m_pFCom->GetRLen() - 4 - len);
	m_szProgVer[m_pFCom->GetRLen() - 4 - len] = 0;

	//保存版本信息。
	m_pParam->flog.SaveLog(m_pParam->slog, ",Program");
	m_pParam->flog.SaveLog(m_pParam->slog, info);
	m_pParam->flog.SaveLog(m_pParam->slog, ": ");
	m_pParam->flog.SaveLog(m_pParam->slog, m_szProgVer);
	m_pParam->flog.SaveLog(m_pParam->slog, ",Data");
	m_pParam->flog.SaveLog(m_pParam->slog, info);
	m_pParam->flog.SaveLog(m_pParam->slog, ": ");
	m_pParam->flog.SaveLog(m_pParam->slog, m_szDataVer);

	//断开ECU连接继电器。
	::Sleep(1000);
	m_pTCom->SendCmd("0x038600");
	::Sleep(2000);
	return TRUE;
}
BOOL CChild::ReadVersionI(CHAR* info)
{
	//读取版本信息。
	m_pTCom->Receive(256, 0, 256);
	m_pTCom->SendCmd("0x038A00");
	if (m_pTCom->Receive(3, 0, 5000) != 3 || m_pTCom->GetRBuf(0) != 0x03 || m_pTCom->GetRBuf(1) != 0x8A || m_pTCom->GetRBuf(2) != 0x0F)
		return FALSE;

	//接收版本信息。
	m_pTCom->SendCmd("0x038A01");
	m_pTCom->Receive(32, 16, 512);
	strcpy(m_szProgVer, (CHAR*)m_pTCom->GetRBuf() + 3);
	m_pTCom->SendCmd("0x038A02");
	m_pTCom->Receive(32, 16, 512);
	strcpy(m_szDataVer, (CHAR*)m_pTCom->GetRBuf() + 3);

	//保存版本信息。
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
	//读取程序版本和数据版本。
	if (ReadVersionI(ver) == FALSE)
	{
		//显示故障信息。
		sprintf(m_szOutput + strlen(m_szOutput), "\r\n读取版本%s失败。", ver);
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		//保存故障信息。
		m_pParam->flog.SaveLog(m_pParam->slog, ",Faild when reading version");
		m_pParam->flog.SaveLog(m_pParam->slog, ver);
		m_pParam->flog.SaveLog(m_pParam->slog, ". ");
		return FALSE;
	}
	//比较初始程序版本、数据版本。
	else if (strcmp(prog, m_szProgVer) != 0 || strcmp(data, m_szDataVer) != 0)
	{
		//显示故障信息。
		if (strcmp(prog, m_szProgVer) != 0)
			sprintf(m_szOutput + strlen(m_szOutput), "\r\n程序版本%s为：%s，不符合要求。", ver, m_szProgVer);
		if (strcmp(data, m_szDataVer) != 0)
			sprintf(m_szOutput + strlen(m_szOutput), "\r\n数据版本%s为：%s，不符合要求。", ver, m_szDataVer);
		GetDlgItem(IDC_OUTPUT)->SetWindowText(m_szOutput);
		//保存故障信息。
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
	//读取ECU序列号。(测试前读取一次，这里读取一次，两个模块，不矛盾)
	//BYTE QUERY_SERIAL[] = "What's Your Serial Number?";
	//if (SendEcuMsg(QUERY_SERIAL, sizeof(QUERY_SERIAL), 64, -1, 0) < 0)
	//	return 10000;
	//if (m_pFCom->GetRLen() == 0)
	//	return 10001;
	//CHAR serial[DEF_MAX_NAME];
	//memcpy(serial, m_pFCom->GetRBuf(), m_pFCom->GetRLen());

	//设置波特率。
	m_pFCom->SetBRate(9600);

	//开始刷写ECU程序。
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

	//恢复ECU序列号。
	//if (SendEcuMsg(serial, sizeof(serial), 64, -1, 0) < 0)
	//	return 10003;

	//刷写成功，返回刷写总步数。
	return 0 - m_nStep;
}
LONG CChild::KFlasherChipID(void)
{
	BYTE FLASH_STAGE1[28] = {0xE6,0xF0,0x9A,0xF6,0xE6,0xF1,0x9A,0x00,0x9A,0xB7,0xFE,0x70,0xF3,0xF6,0xB2,0xFE,0xD7,0x00,0x00,0x00,0x89,0x60,0x7E,0xB7,0x28,0x11,0x3D,0xF6};
	BYTE CHIP_ID_ADDR[4] = {0xFA,0x00,0x00,0xF6};
	//发送STAGE1。
	if (SendEcuMsg(FLASH_STAGE1, sizeof(FLASH_STAGE1), 28, -1, 0) <= 0)
		return 1000;
	//发送StartChipID的存储地址。
	if (SendEcuMsg(CHIP_ID_ADDR, sizeof(CHIP_ID_ADDR), 4, -1, 0) <= 0)
		return 2000;

	//发送StartChipID。
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
	//发送E000 ~ E7A8处的程序。
	if (SendEcuMsg(FLASH_STAGE3, sizeof(FLASH_STAGE3), 28, -1, 0) <= 0)
		return 1000;
	//发送Monitor4b的存储地址。
	if (SendEcuMsg(MONITOR_ADDR, sizeof(MONITOR_ADDR), 4, -1, 0) <= 0)
		return 2000;

	//发送Monitor4B。
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
	//发送预定义消息。
	int i = 0;
	while (msgs[i].len != -1)
	{
		if (SendEcuMsg(msgs[i].cmd, msgs[i].len, msgs[i].receive, msgs[i].index, msgs[i].value) <= 0)
			return 1000 + i;
		if (i == 12 && m_pFCom->GetRBuf(2) != 0x15 && m_pFCom->GetRBuf(2) != 0x16)//CMD00
			return 1000 + i;
		i ++;
	}
	//发送0x00~FF消息。
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
		//{1,	0x5A,	0,	2,	NULL},//(最后一个FECA消息，需要接收两次，第一次之后大约5秒才会收到第二次回复。第二次接收到的消息内容为0x005A。)
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
	//发送预定义消息。
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
	//发送预定义消息。
	int i = 0;
	while (msgs[i].len != -1)
	{
		if (SendEcuMsg(msgs[i].cmd, msgs[i].len, msgs[i].receive, msgs[i].index, msgs[i].value) <= 0)
			return 1000 + i;
		i ++;
	}

	//设置高波特率。
	m_pFCom->SetBRate(57600);
	//发送0x00~FF消息。
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
	//如果间隔不足64ms，则timer起始时间会被重置；如果间隔超过64ms，则timer将会执行，并对已有序列号进行格式化。
	//使用扫描条形码的方式，间隔不会超过16ms，超过64ms则认为扫描已结束。
	SetTimer(2, 64, NULL);
}
void CChild::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)//自动增加进度条。
	{
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->StepIt();
	}
	else if (nIDEvent == 2)//自动格式化序列号。
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