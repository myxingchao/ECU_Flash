//
#include "stdafx.h"
//#include "Flasher.h"
#include "Config.h"
#include "EnumSerial.h"
#include "Bootloader.h"

SParam	CConfig::m_sParam;
CHAR	CConfig::m_szDevName[DEF_CHILD_COUNT][DEF_MAX_NAME];
BOOL	CConfig::m_bShutdown;
LONG	CConfig::m_nIndex[DEF_CHILD_COUNT];

static const CHAR*	_USB_COM_FT232L			= "USB Serial Port";
static const CHAR*	_USB_COM_PL2303			= "Prolific USB";
static const CHAR*	_COMBO_PROC[]			= {"  PCB Fabricating", "  Incoming Check", "  Predelivery Check", ""};

//-------------------------------------------------------------
static const CHAR*	_INI_SECTION_COMMON		= "COMMON";
static const CHAR*	_INI_KEY_PROC			= "PROC";
static const CHAR*	_INI_KEY_ADMIN			= "ADMIN";
static const CHAR*	_INI_KEY_MODAL			= "MODAL";
static const CHAR*	_INI_KEY_PROG0			= "PROG0";
static const CHAR*	_INI_KEY_PROG1			= "PROG1";
static const CHAR*	_INI_KEY_DATA0			= "DATA0";
static const CHAR*	_INI_KEY_DATA1			= "DATA1";
static const CHAR*	_INI_KEY_TAR			= "TAR";
static const CHAR*	_INI_KEY_LOG			= "LOG";
static const CHAR*	_INI_KEY_LABEL			= "LABEL";
static const CHAR*	_INI_KEY_SHUTDN			= "SHUTDN";
static const CHAR*	_INI_KEY_SERVER			= "SERVER";
static const CHAR*	_INI_KEY_PSTEP			= "PSTEP";
static const CHAR*	_INI_KEY_MAILTO			= "MAILTO";
static const CHAR*	_INI_SECTION_DEVICE		= "DEVICE";
static const CHAR*	_INI_KEY_DEV			= "DEVICE";
//-------------------------------------------------------------
CHAR CConfig::m_szPath[_MAX_PATH];
void CConfig::InitPath()
{
	char dir[_MAX_DIR];
	char drive[_MAX_DRIVE];
	char file[_MAX_FNAME];
	::GetModuleFileName(NULL, m_szPath, _MAX_PATH);
	_splitpath(m_szPath, drive, dir, file, NULL);
	sprintf(m_szPath, "%s%s%s.ini", drive, dir, file);
}
SParam* CConfig::GetParam()
{
	return &m_sParam;
}
void CConfig::LoadBootFile()
{
	char dir[_MAX_DIR];
	char drive[_MAX_DRIVE];
	char path[_MAX_FNAME];
	::GetModuleFileName(NULL, path, _MAX_PATH);
	_splitpath(path, drive, dir, NULL, NULL);
	sprintf(path, "%s%s%startchipid.hex", drive, dir);
	m_sParam.chipid.HexFile2Bin(path, 0xA0, TRUE, TRUE);
	sprintf(path, "%s%s%monitor004b.h86", drive, dir);
	m_sParam.monitor.HexFile2Bin(path, 0x7B0, TRUE, FALSE);
}
void CConfig::LoadIni(CHAR* path)
{
	if (strlen(m_szPath) == 0)//第一次使用
	{
		//初始化配置信息。
		memset(&m_sParam, 0, sizeof(m_sParam));
		for (int i = 0; i < DEF_CHILD_COUNT; i ++)
		{
			m_sParam.tester[i].Disconnect();
			m_sParam.flash[i].Disconnect();
		}
		//初始化配置文件路径。
		CConfig::InitPath();
	}

	//如果不指定路径，则从默认路径加载配置文件。
	if (path == NULL)
		path = m_szPath;

	//从文件读入配置信息。
	GetPrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_ADMIN,	"",			m_sParam.sadmin, sizeof(m_sParam.sadmin),	path);
	m_sParam.server = ::GetPrivateProfileInt(_INI_SECTION_COMMON, _INI_KEY_SERVER,	0, path);
	m_sParam.nproc	= ::GetPrivateProfileInt(_INI_SECTION_COMMON, _INI_KEY_PROC,	0, path);
	GetPrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_MODAL,	"",			m_sParam.smodal, sizeof(m_sParam.smodal),	path);
	GetPrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_PROG0,	"",			m_sParam.sprog0, sizeof(m_sParam.sprog0),	path);
	GetPrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_PROG1,	"",			m_sParam.sprog1, sizeof(m_sParam.sprog1),	path);
	GetPrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_DATA0,	"",			m_sParam.sdata0, sizeof(m_sParam.sdata0),	path);
	GetPrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_DATA1,	"",			m_sParam.sdata1, sizeof(m_sParam.sdata1),	path);
	GetPrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_TAR,		"",			m_sParam.star,	 sizeof(m_sParam.star),		path);
	GetPrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_LOG,		"C:\\",		m_sParam.slog,	 sizeof(m_sParam.slog),		path);
	GetPrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_LABEL,	"",			m_sParam.slabel, sizeof(m_sParam.slabel),	path);
	GetPrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_MAILTO,	"",			m_sParam.mailto, sizeof(m_sParam.mailto),	path);
	m_sParam.nstep	= ::GetPrivateProfileInt(_INI_SECTION_COMMON, _INI_KEY_PSTEP,  4000, path);
	CHAR key[16];
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
	{
		sprintf(key, "%s%d", _INI_KEY_DEV, i + 1);
		GetPrivateProfileString(_INI_SECTION_DEVICE, key, "", m_szDevName[i], sizeof(m_sParam.stester[i]), path);
	}
	m_bShutdown = ::GetPrivateProfileInt(_INI_SECTION_COMMON, _INI_KEY_SHUTDN, TRUE, path);

	//加载启动文件。
	m_sParam.pchipid = m_sParam.chipid.HexData2Bin(_START_CHIP_ID, 0xA0, TRUE, TRUE);
	m_sParam.pmonitor = m_sParam.monitor.HexData2Bin(_MONITOR_004_B, 0x7B0, TRUE, TRUE);
	//加载程序文件。
	m_sParam.ptarget = m_sParam.target.HexFile2Bin(m_sParam.star, DEF_BIN_LENGTH, FALSE, FALSE);
	m_sParam.target.CreateIndex();
}
void CConfig::SaveIni(CHAR* path)
{
	//如果不指定路径，则在默认路径保存配置文件。
	if (path == NULL)
		path = m_szPath;

	//向文件写入配置信息。
	char key[32], text[32];
	sprintf(text, "%d", m_sParam.server);
	WritePrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_SERVER,	text,			 path);
	sprintf(text, "%d", m_sParam.nproc);
	WritePrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_PROC,	text,			 path);
	WritePrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_MODAL,	m_sParam.smodal, path);
	WritePrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_PROG0,	m_sParam.sprog0, path);
	WritePrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_PROG1,	m_sParam.sprog1, path);
	WritePrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_DATA0,	m_sParam.sdata0, path);
	WritePrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_DATA1,	m_sParam.sdata1, path);
	WritePrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_TAR,	m_sParam.star,	 path);
	WritePrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_LOG,	m_sParam.slog,	 path);
	WritePrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_LABEL,	m_sParam.slabel, path);
	WritePrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_MAILTO,	m_sParam.mailto, path);
	sprintf(text, "%d", m_sParam.nstep);
	WritePrivateProfileString(_INI_SECTION_COMMON, _INI_KEY_PSTEP,	text,			 path);
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
	{
		sprintf(key, "%s%d", _INI_KEY_DEV, i + 1);
		WritePrivateProfileString(_INI_SECTION_DEVICE, key,	m_sParam.index[i] == -1 ? "" : m_sParam.stester[m_sParam.index[i]], path);
	}
}

//-------------------------------------------------------------
LONG CConfig::OpenComPorts()
{
	//重置已有连接。
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
	{
		m_sParam.tester[i].Disconnect();
		m_sParam.flash[i].Disconnect();
	}
	
	//连接所有USB转串口设备。
	CArray<SSerInfo,SSerInfo&> asi; 
	EnumSerialPorts(asi, TRUE);
	CHAR name[DEF_MAX_NAME];
	CComPort adp[asi.GetSize()];
	LONG m = 0;
	for (int i = 0; i < asi.GetSize(); i ++)
	{
		sprintf(name, "\\\\.\\%s", asi[i].strFriendlyName.GetBuffer() + asi[i].strFriendlyName.Find("COM"));
		name[strstr(name, ")") - name] = 0;
		//打开转换盒端口。
		if ((asi[i].strFriendlyName.Find(_USB_COM_PL2303) >= 0 || asi[i].strFriendlyName.Find(_USB_COM_FT232L) >= 0)
			&& m_sParam.adp[m].Connect(name, 9600) == TRUE)
			m ++;
	}
	//将功能台和刷写器配对。
	CComPort adpx;
	for (int i = 0; i < asi.GetSize(); i ++)
	{
		if (m_sParam.tester[i].IsActive() == FALSE)
			continue;
	}


	LONG m = 0, n = 0;
	for (int i = 0; i < asi.GetSize(); i ++)
	{
		sprintf(name, "\\\\.\\%s", asi[i].strFriendlyName.GetBuffer() + asi[i].strFriendlyName.Find("COM"));
		name[strstr(name, ")") - name] = 0;
		//打开功能台和刷写器的端口。
		if (asi[i].strFriendlyName.Find(_USB_COM_PL2303) >= 0 && m_sParam.tester[m].Connect(name, 9600) == TRUE)
			m ++;//(功能台)
		else if (asi[i].strFriendlyName.Find(_USB_COM_FT232L) >= 0 && m_sParam.flash[n].Connect(name, 9600) == TRUE)
			n ++;//(刷写器)
	}

	//将功能台和刷写器配对。
	CComPort ctmp;
	for (int i = 0; i < 2 * DEF_CHILD_COUNT; i ++)
	{
		if (m_sParam.tester[i].IsActive() == FALSE)
			continue;
		//向功能台发送验证指令。
		m_sParam.tester[i].SendCmd("0x038400");
		//逐个确认刷写器回复。
		LONG j = 0;
		while (j < n)
		{
			m_sParam.flash[j].Receive(3, 32, 96);
			if (m_sParam.flash[j].GetRLen() > 0 && m_sParam.flash[j].GetRBuf(0) == 0x03)
				break;
			j ++;
		}
		//没有收到消息。
		if (j == n)
		{
			strcpy(m_sParam.stester[i], "");
			//断开该功能台。
			m_sParam.tester[i].Disconnect();
			m --;
		}
		else//收到消息。
		{
			//保存返回的功能台名称。
			strcpy(m_sParam.stester[i], (CHAR*)(m_sParam.flash[j].GetRBuf() + 1));
			//if (i != j)按照功能台顺序，将刷写器排序。
			memcpy(&ctmp, &m_sParam.flash[j], sizeof(ctmp));
			memcpy(&m_sParam.flash[j], &m_sParam.flash[i], sizeof(ctmp));
			memcpy(&m_sParam.flash[i], &ctmp, sizeof(ctmp));
		}
	}
	m_sParam.ntester = m;
	//断开没有对应功能台的刷写器。
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
	{
		if (m_sParam.tester[i].IsActive() == FALSE && m_sParam.flash[i].IsActive() == TRUE)
			m_sParam.flash[i].Disconnect();
	}
	//根据配置文件内容生成索引。
	if (m_sParam.ntester > 0)
		CConfig::ResetFuncIndex();
	return m_sParam.ntester;
}
void CConfig::ResetFuncIndex()
{
	//重新整理可用的功能台序号。
	LONG m = 0;
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
	{
		if (m_sParam.tester[i].IsActive() == TRUE)
			m_sParam.index[m ++] = i;
	}
	for (int i = m_sParam.ntester; i < DEF_CHILD_COUNT; i ++)
		m_sParam.index[i] = -1;
	//按保存的功能台名称重新排序。
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
	{
		if (strlen(m_szDevName[i]) == 0)
			continue;
		m = 0;
		while (m < m_sParam.ntester && (strlen(m_sParam.stester[m]) == 0 || strstr(m_szDevName[i], m_sParam.stester[m]) == NULL))
			m ++;
		if (m == i || m == m_sParam.ntester)
			continue;
		LONG t = m_sParam.index[i];
		m_sParam.index[i] = m_sParam.index[m];
		m_sParam.index[m] = t;
	}
}
void CConfig::CloseComPorts()
{
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
	{
		m_sParam.tester[i].Disconnect();
		m_sParam.flash[i].Disconnect();
	}
}
//-------------------------------------------------------------
CConfig::CConfig(CWnd* pParent)	: CDialog(CConfig::IDD, pParent)
{
}
CConfig::~CConfig()
{
}
void CConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}
BEGIN_MESSAGE_MAP(CConfig, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_LOAD,			&CConfig::OnBnClickedLoad)
	ON_BN_CLICKED(IDC_SAVE,			&CConfig::OnBnClickedSave)
	ON_BN_CLICKED(IDC_BTN_TAR,		&CConfig::OnBnClickedTar)
	ON_BN_CLICKED(IDC_BTN_LOG,		&CConfig::OnBnClickedLog)
	ON_BN_CLICKED(IDC_BTN_LABEL,	&CConfig::OnBnClickedLabel)
	ON_BN_CLICKED(IDC_APPLY,		&CConfig::OnBnClickedApply)
	ON_BN_CLICKED(IDC_CLOSE,		&CConfig::OnBnClickedClose)
	ON_CBN_SELCHANGE(IDC_COMBO_PROC,&CConfig::OnCbnSelchangeComboProc)
	ON_EN_CHANGE(IDC_EDITB_MODAL,	&CConfig::OnEnChangeEditModal)
	ON_EN_CHANGE(IDC_EDITB_PROG0,	&CConfig::OnEnChangeEditProg0)
	ON_EN_CHANGE(IDC_EDITB_PROG1,	&CConfig::OnEnChangeEditProg1)
	ON_EN_CHANGE(IDC_EDITB_DATA0,	&CConfig::OnEnChangeEditData0)
	ON_EN_CHANGE(IDC_EDITB_DATA1,	&CConfig::OnEnChangeEditData1)
	ON_EN_CHANGE(IDC_EDITB_TAR,		&CConfig::OnEnChangeEditTar)
	ON_EN_CHANGE(IDC_EDITB_LOG,		&CConfig::OnEnChangeEditLog)
	ON_EN_CHANGE(IDC_EDITB_LABEL,	&CConfig::OnEnChangeEditLabel)
	ON_EN_CHANGE(IDC_EDITB_MAILTO,	&CConfig::OnEnChangeEditMailto)
	ON_CBN_SELCHANGE(IDC_COMBO1,	&CConfig::OnCbnSelchangeCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO2,	&CConfig::OnCbnSelchangeCombo2)
	ON_CBN_SELCHANGE(IDC_COMBO3,	&CConfig::OnCbnSelchangeCombo3)
	ON_CBN_SELCHANGE(IDC_COMBO4,	&CConfig::OnCbnSelchangeCombo4)
	ON_EN_CHANGE(IDC_SERVER_IP,		&CConfig::OnEnChangeIPAddress)
	ON_NOTIFY(IPN_FIELDCHANGED,		IDC_SERVER_IP, &CConfig::OnIpnFieldchangedIpaddress1)
END_MESSAGE_MAP()

BOOL CConfig::OnInitDialog()
{
	//调整控件位置。
	CWnd* pwnd;
	RECT rt;
	for (int i = 0; i < 10; i ++)
	{
		switch (i)
		{
		case 0:		pwnd = GetDlgItem(IDC_EDITB_MODAL);		break;
		case 1:		pwnd = GetDlgItem(IDC_EDITB_PROG0);		break;
		case 2:		pwnd = GetDlgItem(IDC_EDITB_PROG1);		break;
		case 3:		pwnd = GetDlgItem(IDC_EDITB_DATA0);		break;
		case 4:		pwnd = GetDlgItem(IDC_EDITB_DATA1);		break;
		case 5:		pwnd = GetDlgItem(IDC_EDITB_TAR);		break;
		case 6:		pwnd = GetDlgItem(IDC_EDITB_LOG);		break;
		case 7:		pwnd = GetDlgItem(IDC_EDITB_LABEL);		break;
		case 8:		pwnd = GetDlgItem(IDC_SERVER_IP);		break;
		case 9:		pwnd = GetDlgItem(IDC_EDITB_MAILTO);	break;
		}
		pwnd->GetWindowRect(&rt);
		ScreenToClient(&rt);
		pwnd->MoveWindow(rt.left, rt.top + 1, rt.right - rt.left - 1, rt.bottom - rt.top - 1 - 1);
		((CEdit*)pwnd)->SetMargins(6, 6);
	}
	GetDlgItem(IDC_COMBO_PROC)->GetWindowRect(&rt);
	ScreenToClient(&rt);
	GetDlgItem(IDC_COMBO_PROC)->MoveWindow(rt.left, rt.top - 1, rt.right - rt.left, rt.bottom - rt.top + 1);
	GetDlgItem(IDC_EDIT_ERROR)->GetWindowRect(&rt);
	ScreenToClient(&rt);
	GetDlgItem(IDC_EDIT_ERROR)->MoveWindow(rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top + 1);

	//初始化控件数据。
	UpdateCtrl();
	CDialog::OnInitDialog();
	//将焦点设置到第一个控件。
	return FALSE;
}
void CConfig::UpdateCtrl()
{
	//添加制程列表选项。
	((CComboBox*)GetDlgItem(IDC_COMBO_PROC))->ResetContent();
	int m = 0;
	while (strlen(_COMBO_PROC[m]) > 0)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_PROC))->InsertString(m, _COMBO_PROC[m]);
		m ++;
	}

	//设置编辑框的内容。
	((CIPAddressCtrl*)GetDlgItem(IDC_SERVER_IP))->SetAddress(m_sParam.server);
	((CComboBox*)GetDlgItem(IDC_COMBO_PROC))->SetCurSel(m_sParam.nproc);
	OnCbnSelchangeComboProc();
	GetDlgItem(IDC_EDITB_MODAL	)->SetWindowText(m_sParam.smodal);
	GetDlgItem(IDC_EDITB_PROG0	)->SetWindowText(m_sParam.sprog0);
	GetDlgItem(IDC_EDITB_PROG1	)->SetWindowText(m_sParam.sprog1);
	GetDlgItem(IDC_EDITB_DATA0	)->SetWindowText(m_sParam.sdata0);
	GetDlgItem(IDC_EDITB_DATA1	)->SetWindowText(m_sParam.sdata1);
	GetDlgItem(IDC_EDITB_TAR	)->SetWindowText(m_sParam.star);
	GetDlgItem(IDC_EDITB_LOG	)->SetWindowText(m_sParam.slog);
	GetDlgItem(IDC_EDITB_LABEL	)->SetWindowText(m_sParam.slabel);
	GetDlgItem(IDC_EDITB_MAILTO	)->SetWindowText(m_sParam.mailto);

	//设置下拉框的内容。
	((CComboBox*)GetDlgItem(IDC_COMBO1))->ResetContent();
	((CComboBox*)GetDlgItem(IDC_COMBO2))->ResetContent();
	((CComboBox*)GetDlgItem(IDC_COMBO3))->ResetContent();
	((CComboBox*)GetDlgItem(IDC_COMBO4))->ResetContent();
	CHAR text[DEF_MAX_NAME];
	m = 0;
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
	{
		if (m_sParam.index[i] < 0)
			continue;
		m_nIndex[m] = m_sParam.index[i];
		sprintf(text, "%s (%s)", m_sParam.stester[m_sParam.index[i]], m_sParam.tester[m_sParam.index[i]].GetCom());
		((CComboBox*)GetDlgItem(IDC_COMBO1))->InsertString(m, text);
		((CComboBox*)GetDlgItem(IDC_COMBO2))->InsertString(m, text);
		((CComboBox*)GetDlgItem(IDC_COMBO3))->InsertString(m, text);
		((CComboBox*)GetDlgItem(IDC_COMBO4))->InsertString(m, text);
		m ++;
	}
	while (m < DEF_CHILD_COUNT)
		m_nIndex[m ++] = -1;
	//添加None选项（一直都有，可以不使用某些功能台）
	sprintf(text, "(None)");
	((CComboBox*)GetDlgItem(IDC_COMBO1))->InsertString(m_sParam.ntester, text);
	((CComboBox*)GetDlgItem(IDC_COMBO2))->InsertString(m_sParam.ntester, text);
	((CComboBox*)GetDlgItem(IDC_COMBO3))->InsertString(m_sParam.ntester, text);
	((CComboBox*)GetDlgItem(IDC_COMBO4))->InsertString(m_sParam.ntester, text);
	//设置当前选中项目。
	memcpy(m_nIndex, m_sParam.index, sizeof(m_nIndex));
	m = 0;
	((CComboBox*)GetDlgItem(IDC_COMBO1))->SetCurSel(m_sParam.index[0] < 0 ? m_sParam.ntester : m ++);
	((CComboBox*)GetDlgItem(IDC_COMBO2))->SetCurSel(m_sParam.index[1] < 0 ? m_sParam.ntester : m ++);
	((CComboBox*)GetDlgItem(IDC_COMBO3))->SetCurSel(m_sParam.index[2] < 0 ? m_sParam.ntester : m ++);
	((CComboBox*)GetDlgItem(IDC_COMBO4))->SetCurSel(m_sParam.index[3] < 0 ? m_sParam.ntester : m ++);

	GetDlgItem(IDC_APPLY)->EnableWindow(FALSE);
	GetDlgItem(IDC_SAVE)->EnableWindow(TRUE);
}
BOOL CConfig::CheckContents()
{
	CHAR text[DEF_MAX_NAME];
	//（被测产品型号）
	GetDlgItem(IDC_EDITB_MODAL)->GetWindowText(text, sizeof(text));
	if (strlen(text) == 0 && ::MessageBox(m_hWnd, "You forget to input the PRODUCT MODAL.", "", MB_OK | MB_ICONINFORMATION) == IDOK)
		return FALSE;
	//（刷写前程序版本）
	GetDlgItem(IDC_EDITB_PROG0)->GetWindowText(text, sizeof(text));
	if (strlen(text) == 0 && ::MessageBox(m_hWnd, "You forget to input the PROGRAM VERSION 0.", "", MB_OK | MB_ICONINFORMATION) == IDOK)
		return FALSE;
	//（刷写后程序版本）
	GetDlgItem(IDC_EDITB_PROG1)->GetWindowText(text, sizeof(text));
	if (strlen(text) == 0 && ::MessageBox(m_hWnd, "You forget to input the PROGRAM VERSION 1", "", MB_OK | MB_ICONINFORMATION) == IDOK)
		return FALSE;
	//（刷写前数据版本）
	GetDlgItem(IDC_EDITB_DATA0)->GetWindowText(text, sizeof(text));
	if (strlen(text) == 0 && ::MessageBox(m_hWnd, "You forget to input the DATA VERSION 0", "", MB_OK | MB_ICONINFORMATION) == IDOK)
		return FALSE;
	//（刷写后数据版本）
	GetDlgItem(IDC_EDITB_DATA1)->GetWindowText(text, sizeof(text));
	if (strlen(text) == 0 && ::MessageBox(m_hWnd, "You forget to input the DATA VERSION 1", "", MB_OK | MB_ICONINFORMATION) == IDOK)
		return FALSE;
	//（待刷文件位置）
	GetDlgItem(IDC_EDITB_TAR)->GetWindowText(text, sizeof(text));
	if (strlen(text) == 0 && ::MessageBox(m_hWnd, "You forget to input the TARGET FILE LOCATION.", "", MB_OK | MB_ICONINFORMATION) == IDOK)
		return FALSE;
	//（记录文件位置）
	GetDlgItem(IDC_EDITB_LOG)->GetWindowText(text, sizeof(text));
	if (strlen(text) == 0 && ::MessageBox(m_hWnd, "You forget to input the LOG FILE LOCATION.", "", MB_OK | MB_ICONINFORMATION) == IDOK)
		return FALSE;
	//（标签配置文件位置）
	GetDlgItem(IDC_EDITB_LABEL)->GetWindowText(text, sizeof(text));
	if (strlen(text) == 0 && ::MessageBox(m_hWnd, "You forget to input the LABEL CONFIG FILE LOCATION.", "", MB_OK | MB_ICONINFORMATION) == IDOK)
		return FALSE;
	//（设备位置）
	if ((((CComboBox*)GetDlgItem(IDC_COMBO1))->GetCurSel() < m_sParam.ntester &&
		(((CComboBox*)GetDlgItem(IDC_COMBO1))->GetCurSel() == ((CComboBox*)GetDlgItem(IDC_COMBO2))->GetCurSel() ||
		 ((CComboBox*)GetDlgItem(IDC_COMBO1))->GetCurSel() == ((CComboBox*)GetDlgItem(IDC_COMBO3))->GetCurSel() ||
		 ((CComboBox*)GetDlgItem(IDC_COMBO1))->GetCurSel() == ((CComboBox*)GetDlgItem(IDC_COMBO4))->GetCurSel())
		 || ((CComboBox*)GetDlgItem(IDC_COMBO2))->GetCurSel() < m_sParam.ntester &&
		(((CComboBox*)GetDlgItem(IDC_COMBO2))->GetCurSel() == ((CComboBox*)GetDlgItem(IDC_COMBO3))->GetCurSel() ||
		 ((CComboBox*)GetDlgItem(IDC_COMBO2))->GetCurSel() == ((CComboBox*)GetDlgItem(IDC_COMBO4))->GetCurSel())
		 || ((CComboBox*)GetDlgItem(IDC_COMBO3))->GetCurSel() < m_sParam.ntester &&
		((CComboBox*)GetDlgItem(IDC_COMBO3))->GetCurSel() == ((CComboBox*)GetDlgItem(IDC_COMBO4))->GetCurSel())
		&& ::MessageBox(m_hWnd, "Some devices are duplicating!", "Caution", MB_OK | MB_ICONINFORMATION) == IDOK)
		return FALSE;
	return TRUE;
}
void CConfig::OnBnClickedApply()
{
	//检查输入合法性。
	if (CheckContents() == FALSE)
		return;

	//保存编辑框控件的设置内容。
	((CIPAddressCtrl*)GetDlgItem(IDC_SERVER_IP))->GetAddress(m_sParam.server);
	m_sParam.nproc = ((CComboBox*)GetDlgItem(IDC_COMBO_PROC))->GetCurSel();
	GetDlgItem(IDC_EDITB_MODAL	)->GetWindowText(m_sParam.smodal,	sizeof(m_sParam.smodal));
	GetDlgItem(IDC_EDITB_PROG0	)->GetWindowText(m_sParam.sprog0,	sizeof(m_sParam.sprog0));
	GetDlgItem(IDC_EDITB_PROG1	)->GetWindowText(m_sParam.sprog1,	sizeof(m_sParam.sprog1));
	GetDlgItem(IDC_EDITB_DATA0	)->GetWindowText(m_sParam.sdata0,	sizeof(m_sParam.sdata0));
	GetDlgItem(IDC_EDITB_DATA1	)->GetWindowText(m_sParam.sdata1,	sizeof(m_sParam.sdata1));
	GetDlgItem(IDC_EDITB_TAR	)->GetWindowText(m_sParam.star,		sizeof(m_sParam.star));
	GetDlgItem(IDC_EDITB_LOG	)->GetWindowText(m_sParam.slog,		sizeof(m_sParam.slog));
	GetDlgItem(IDC_EDITB_LABEL	)->GetWindowText(m_sParam.slabel,	sizeof(m_sParam.slabel));
	GetDlgItem(IDC_EDITB_MAILTO	)->GetWindowText(m_sParam.mailto,	sizeof(m_sParam.mailto));

	//根据设定的子窗口位置对tester和flash重新排序。
	/*CHAR text[DEF_MAX_NAME]; (改为使用List序号，不再查找文字内容）
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
	{
		//查找新的位置。
		switch (i)
		{
			case 0:	GetDlgItem(IDC_COMBO1)->GetWindowText(text, sizeof(text)); break;
			case 1:	GetDlgItem(IDC_COMBO2)->GetWindowText(text, sizeof(text)); break;
			case 2:	GetDlgItem(IDC_COMBO3)->GetWindowText(text, sizeof(text)); break;
			case 3:	GetDlgItem(IDC_COMBO4)->GetWindowText(text, sizeof(text)); break;
			default: break;
		}
		LONG x = 0;
		while (x < DEF_CHILD_COUNT && (strlen(m_sParam.tester[x].GetCom()) == 0 || strstr(text, m_sParam.tester[x].GetCom()) == NULL))
			x ++;
		//重置功能台序号映射。
		if (x < DEF_CHILD_COUNT)
			m_sParam.index[i] = x;
		else
			m_sParam.index[i] = -1;
	}*/
	m_sParam.index[0] = m_nIndex[((CComboBox*)GetDlgItem(IDC_COMBO1))->GetCurSel()];
	m_sParam.index[1] = m_nIndex[((CComboBox*)GetDlgItem(IDC_COMBO2))->GetCurSel()];
	m_sParam.index[2] = m_nIndex[((CComboBox*)GetDlgItem(IDC_COMBO3))->GetCurSel()];
	m_sParam.index[3] = m_nIndex[((CComboBox*)GetDlgItem(IDC_COMBO4))->GetCurSel()];
	//更新配置文件信息，以便重新连接后恢复排布顺序。
	for (int i = 0; i < DEF_CHILD_COUNT; i ++)
		strcpy(m_szDevName[i], m_sParam.index[i] == -1 ? "" : m_sParam.stester[m_sParam.index[i]]);

	//因为路径改变，重新加载程序文件。
	m_sParam.ptarget = m_sParam.target.HexFile2Bin(m_sParam.star, DEF_BIN_LENGTH, FALSE, FALSE);

	GetDlgItem(IDC_APPLY)->EnableWindow(FALSE);
	GetDlgItem(IDC_SAVE)->EnableWindow(TRUE);
}
void CConfig::OnBnClickedClose()
{
	//如果有内容变动，提示用户保存。
	if (GetDlgItem(IDC_APPLY)->IsWindowEnabled() == TRUE && ::MessageBox(m_hWnd, "Do you want to save current contents?", "Caution", MB_YESNO | MB_ICONINFORMATION) == IDYES)
		OnBnClickedApply();
	//关闭对话框。
	OnOK();
}
void CConfig::OnBnClickedLoad()
{
	CFileDialog dlg(TRUE, "ini", "", OFN_HIDEREADONLY, _T("Config Files (*.ini)|*.ini||"), this);
	dlg.DoModal();
	//加载配置文件。
	CConfig::LoadIni(dlg.GetPathName().GetBuffer());
	CConfig::ResetFuncIndex();
	//更新控件内容。
	UpdateCtrl();
}
void CConfig::OnBnClickedSave()
{
	CFileDialog dlg(FALSE, "ini", "", OFN_HIDEREADONLY, _T("Config Files (*.ini)|*.ini||"), this);
	dlg.DoModal();
	CConfig::SaveIni(dlg.GetPathName().GetBuffer());
}
void CConfig::OnBnClickedTar()
{
	CFileDialog dlg(TRUE, "h86", "", OFN_HIDEREADONLY, _T("H86 Files (*.h86)|*.h86||"), this);
	dlg.DoModal();
	GetDlgItem(IDC_EDITB_TAR)->SetWindowText(dlg.GetPathName().GetBuffer());
}
void CConfig::OnBnClickedLog()
{
	CFileDialog dlg(FALSE, "csv", "", OFN_HIDEREADONLY, _T("Log Files (*.csv)|*.csv;*.txt;*.xls||"), this);
	dlg.DoModal();
	GetDlgItem(IDC_EDITB_LOG)->SetWindowText(dlg.GetPathName().GetBuffer());
}
void CConfig::OnBnClickedLabel()
{
	CFileDialog dlg(FALSE, "ini", "", OFN_HIDEREADONLY, _T("Label Config Files (*.ini)|*.ini;*.txt||"), this);
	dlg.DoModal();
	GetDlgItem(IDC_EDITB_LABEL)->SetWindowText(dlg.GetPathName().GetBuffer());
}
//-------------------------------------------
void CConfig::EnableApplyBtn()
{
	GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
	GetDlgItem(IDC_SAVE)->EnableWindow(FALSE);
}
void CConfig::OnCbnSelchangeComboProc()
{
	if (GetDlgItem(IDC_COMBO_PROC)->IsWindowEnabled() == FALSE)
	{
		GetDlgItem(IDC_EDITB_PROG0	)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITB_DATA0	)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITB_PROG1	)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITB_DATA1	)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITB_TAR	)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITB_LABEL	)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_TAR		)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LABEL	)->EnableWindow(FALSE);
		return;
	}
	switch (((CComboBox*)GetDlgItem(IDC_COMBO_PROC))->GetCurSel())
	{
	case 0://"PCB Fabricating"
		GetDlgItem(IDC_EDITB_PROG0	)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITB_DATA0	)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITB_PROG1	)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDITB_DATA1	)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDITB_TAR	)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDITB_LABEL	)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_TAR		)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_LABEL	)->EnableWindow(TRUE);
		break;
	case 1://"Incoming Check"
		GetDlgItem(IDC_EDITB_PROG0	)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDITB_DATA0	)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDITB_PROG1	)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITB_DATA1	)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITB_TAR	)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITB_LABEL	)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_TAR		)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LABEL	)->EnableWindow(FALSE);
		break;
	case 2://"Predelivery Check"
		GetDlgItem(IDC_EDITB_PROG0	)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDITB_DATA0	)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDITB_PROG1	)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDITB_DATA1	)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDITB_TAR	)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDITB_LABEL	)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_TAR		)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_LABEL	)->EnableWindow(TRUE);
		break;
	}
	EnableApplyBtn();
}
void CConfig::OnEnChangeEditModal()
{
	EnableApplyBtn();
}
void CConfig::OnEnChangeEditProg0()
{
	EnableApplyBtn();
}
void CConfig::OnEnChangeEditProg1()
{
	EnableApplyBtn();
}
void CConfig::OnEnChangeEditData0()
{
	EnableApplyBtn();
}
void CConfig::OnEnChangeEditData1()
{
	EnableApplyBtn();
}
void CConfig::OnEnChangeEditTar()
{
	EnableApplyBtn();
}
void CConfig::OnEnChangeEditLog()
{
	EnableApplyBtn();
}
void CConfig::OnEnChangeEditLabel()
{
	EnableApplyBtn();
}
void CConfig::OnEnChangeEditMailto()
{
	EnableApplyBtn();
}
void CConfig::OnCbnSelchangeCombo1()
{
	EnableApplyBtn();
}
void CConfig::OnCbnSelchangeCombo2()
{
	EnableApplyBtn();
}
void CConfig::OnCbnSelchangeCombo3()
{
	EnableApplyBtn();
}
void CConfig::OnCbnSelchangeCombo4()
{
	EnableApplyBtn();
}
void CConfig::OnEnChangeIPAddress()
{
	CHAR text[16];
	GetDlgItem(IDC_SERVER_IP)->GetWindowText(text, sizeof(text));
	BOOL enable = strcmp(text, "0.0.0.0") == 0 ? TRUE : FALSE;
	GetDlgItem(IDC_COMBO_PROC	)->EnableWindow(enable);
	GetDlgItem(IDC_EDITB_MODAL	)->EnableWindow(enable);
	GetDlgItem(IDC_EDITB_LOG	)->EnableWindow(enable);
	GetDlgItem(IDC_EDITB_MAILTO	)->EnableWindow(enable);
	GetDlgItem(IDC_BTN_LOG		)->EnableWindow(enable);
	OnCbnSelchangeComboProc();
	EnableApplyBtn();
}
void CConfig::OnIpnFieldchangedIpaddress1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	if (pIPAddr->iValue > 255)
		pIPAddr->iValue = 255;
	if (pIPAddr->iValue < 0)
		pIPAddr->iValue = 0;
	*pResult = 0;
}
//-------------------------------------------