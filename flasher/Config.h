//
#pragma once
#include "ComPort.h"
#include "LogFile.h"
#include "Hex2Bin.h"
#include "Resource.h"

#define DEF_CHILD_COUNT	4
#define DEF_PATH_STRING	256
#define DEF_BIN_LENGTH	0x1000000
struct SParam
{
	LONG		nproc;					//应用流程
	CHAR		suser[DEF_MAX_NAME];	//当前用户ID
	CHAR		sadmin[DEF_MAX_NAME];	//管理员ID
	CHAR		smodal[DEF_MAX_NAME];	//产品型号
	CHAR		sprog0[DEF_MAX_NAME];	//刷写前程序版本
	CHAR		sprog1[DEF_MAX_NAME];	//刷写后程序版本
	CHAR		sdata0[DEF_MAX_NAME];	//刷写前数据版本
	CHAR		sdata1[DEF_MAX_NAME];	//刷写后数据版本
	CHAR		star[DEF_PATH_STRING];	//刷写文件路径
	CHAR		slog[DEF_PATH_STRING];	//日志文件路径
	CHAR		slabel[DEF_PATH_STRING];//标签配置文件
	CLogFile	flog;					//日志文件操作
	DWORD		server;					//服务器IP地址
	//CHAR		serial[16];				//当前序列号计数
	CHAR		mailto[DEF_MAX_NAME];	//邮件发送地址
	LONG		ntester;				//功能台数量
	CHAR		stester[DEF_CHILD_COUNT][DEF_MAX_NAME];
										//功能台名称
	CComPort	tester[DEF_CHILD_COUNT];//功能台连接信息
	CComPort	flash[DEF_CHILD_COUNT];	//刷写器连接信息
	LONG		index[DEF_CHILD_COUNT];	//子窗口位置序号
	CHex2Bin	chipid;					//StartChipID文件
	CHex2Bin	monitor;				//Monitor004B文件
	CHex2Bin	target;					//ECU程序文件
	BYTE*		pchipid;				//StartChipID数据
	BYTE*		pmonitor;				//Monitor004B数据
	BYTE*		ptarget;				//ECU程序数据
	LONG		nstep;					//进度条总步数
};
//-----------------------------------------------
struct SLabelImage
{
	CHAR	path[DEF_PATH_STRING];
	LONG	x;
	LONG	y;
};
struct SLabelText
{
	CHAR	font[64];	//0~22: internal font, "Arial": customize font.
	CHAR	text[128];	//when type == 1: text, type == 2: null, type == 3: current integer.
	LONG	type;		//1:text, 2:date, 3:increasing integer number.
	LONG	x;
	LONG	y;
	LONG	ratio;		//internal font: 1~8, customize font: font height.
	LONG	bold;		//internal font: no use, customize font: 1 = bold.
};
struct SLabelCode128
{
	LONG	show;		//0: do not show text, 1: show text.
	CHAR	data[128];	//null: use serial no. to generate this bar code.
	LONG	x;
	LONG	y;
	LONG	width;		//1~14.
	LONG	height;		//0~999.
};
struct SLabelCodeQR
{
	CHAR	data[128];	//null: use serial no. to generate this bar code.
	LONG	x;
	LONG	y;
	LONG	width;		//1~10.
};
struct SLabel
{
	LONG			nwidth;	//the width of paper.
	LONG			nimage;
	SLabelImage*	pimage;
	LONG			ntext;
	SLabelText*		ptext;
	LONG			ncode128;
	SLabelCode128*	pcode128;
	LONG			ncodeqr;
	SLabelCodeQR*	pcodeqr;
};
//-----------------------------------------------

class CConfig : public CDialog
{
public:
	CConfig(CWnd* pParent = NULL);
	virtual ~CConfig();
public:
	static	SParam*	GetParam();
	static	void	LoadIni(CHAR* path);
	static	void	SaveIni(CHAR* path);
	static	LONG	OpenComPorts();
	static	void	CloseComPorts();
	static	void	ResetFuncIndex();

protected:
	static	void	InitPath();
	static	void	LoadBootFile();
	void			EnableApplyBtn();
	void			UpdateCtrl();
	BOOL			CheckContents();

protected:
	enum { IDD = IDD_CONFIG };
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

protected:
	HICON			m_hIcon;
	static	SParam	m_sParam;
	static	CHAR	m_szPath[_MAX_PATH];
	static	CHAR	m_szDevName[DEF_CHILD_COUNT][DEF_MAX_NAME];//配置文件中保存的各子窗口名称
	static	LONG	m_nIndex[DEF_CHILD_COUNT];//保存下拉列表所对应的功能台序号
	static	BOOL	m_bShutdown;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedLoad();
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedTar();
	afx_msg void OnBnClickedLog();
	afx_msg void OnBnClickedLabel();
	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedClose();
	afx_msg void OnEnChangeEditModal();
	afx_msg void OnEnChangeEditProg0();
	afx_msg void OnEnChangeEditProg1();
	afx_msg void OnEnChangeEditData0();
	afx_msg void OnEnChangeEditData1();
	afx_msg void OnEnChangeEditTar();
	afx_msg void OnEnChangeEditLog();
	afx_msg void OnEnChangeEditLabel();
	afx_msg void OnEnChangeEditMailto();
	afx_msg void OnCbnSelchangeComboProc();
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnCbnSelchangeCombo2();
	afx_msg void OnCbnSelchangeCombo3();
	afx_msg void OnCbnSelchangeCombo4();
	afx_msg void OnEnChangeIPAddress();
	afx_msg void OnIpnFieldchangedIpaddress1(NMHDR *pNMHDR, LRESULT *pResult);
};
