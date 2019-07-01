#pragma once
#include "ColorBtn.h"
#include "ColorFrm.h"
#include "ColorStc.h"
#include "Config.h"
#include "Resource.h"

const	int	ANCHOR_LEFT			= 0x0000;	// Fix Left
const	int	ANCHOR_TOP			= 0x0000;	// Fix Top
const	int	ANCHOR_RIGHT		= 0x0001;	// Fix Right
const	int	ANCHOR_BOTTOM		= 0x0002;	// Fix Buttom
const	int	ANCHOR_LEFT2		= 0x0004;	// Hor Center
const	int	ANCHOR_TOP2			= 0x0008;	// Ver Center
const	int	RESIZE_HOR			= 0x0100;	// Resizable Horizontally
const	int	RESIZE_VER			= 0x0200;	// Resizable Vertically
const	int	RESIZE_BOTH			= (RESIZE_HOR | RESIZE_VER);
const	int	RESIZE_HOR2			= 0x0400;	// Resizable Horizontally(1/2 delta)
const	int	RESIZE_VER2			= 0x0800;	// Resizable Vertically(1/2 delta)

#define WM_DISABLE_CONFIG	WM_USER + 1001

class CComPort;
class CChild : public CDialog
{
public:
	CChild(CWnd* pParent = NULL);
	virtual ~CChild();
public:
	void	SetColor(DWORD color);		//设置开始按钮颜色
	void	SetReady(SParam* config, CComPort* tcom, CComPort* fcom, CHAR* name);//初始化窗口

protected:
	enum { IDD = IDD_CHILD };
	virtual BOOL OnInitDialog();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

protected:
	BOOL	ReadSerialNo();				//读取ECU序列号
	void	FormatSerial();				//将序列号分行显示
	void	LoadLabel(SLabel* info);	//加载标签配置信息
	LONG	PrintLabel(SLabel* label);	//打印标签
	void	ShowPrintInfo(LONG error);	//显示打印错误信息
	BOOL	ProcTest();					//实施测试
	BOOL	ProcFlash();				//实施刷写
	void	Process();					//测试、刷写线程
	static	UINT ProcThd(LPVOID param);	//测试、刷写线程
	void	WaitThd();					//ECU插拔检测线程
	static	UINT WaitThd(LPVOID param);	//ECU插拔检测线程
	void	SetControlInfo(CWnd* pwnd, LONG anchor);
protected:
	void	SaveError(CHAR* title, LONG index, BYTE* msg, LONG len);
	LONG	SendEcuMsg(BYTE* cmd, LONG len, LONG receive, LONG index, BYTE value);
	LONG	KFlasherChipID();
	LONG	KFlasherMonitor4B();
	LONG	KFlasherTestBaud();
	LONG	KFlasherErase();
	LONG	KFlasherCheckBank();
	LONG	KFlasherSetBaud();
	LONG	KFlasherTransBin();
	LONG	FlashECU();				//通过ECU Bootloader刷写程序
protected:
	DWORD	ComposeCmd(BYTE* cmd, UINT len);
	DWORD	ComposeCmd(BYTE id, BYTE* cmd);
	DWORD	ComposeCmd(BYTE id, BYTE* cmd, UINT len);
	BOOL	ReadAddress(DWORD address, BYTE size, BOOL offset);
	//LONG	SendKLineMsg();			//进行K线通信
	BOOL	ReadVersionD(CHAR* info);//直接读取ECU版本
	BOOL	ReadVersionI(CHAR* info);//通过功能台，读取ECU版本
	BOOL	CheckVersion(CHAR* ver, CHAR* prog, CHAR* data);//比较ECU版本

protected:
	SParam*			m_pParam;		//配置信息
	SLabel			m_sLabel;		//标签信息
	CHAR			m_szSerial[256];//产品序列号
	CHAR			m_szDataVer[16];//当前数据版本
	CHAR			m_szProgVer[16];//当前程序版本
	HANDLE			m_hStop;		//用于在测试刷写过程中，暂停接收ECU插拔消息
	HANDLE			m_hExit;		//退出ECU插拔检测线程的标志位
	BOOL			m_bNewECU;		//用于识别开始按钮的功能，可以用其他方法代替
	static	HANDLE	m_hPrint;		//打印队列标志
	CWinThread*		m_pProcThd;		//测试刷写线程
	CWinThread*		m_pWaitThd;		//ECU插拔检测线程
	CComPort*		m_pTCom;		//功能台线路
	CComPort*		m_pFCom;		//刷写线路
	LONG			m_nStep;		//刷写步数
	DWORD			m_nInputTm;		//序列号输入计时

	HICON			m_hIcon;		//无用
	CColorFrm		m_frmChild;		//控件框
	CColorBtn		m_btnStart;		//开始按钮
	DWORD			m_dwColor;		//按钮颜色
	CColorStc		m_stcInfo;		//无用
	CHAR			m_szOutput[5 * 256];//提示信息
	LONG			m_pAnchor[16][4];	//control handle, anchor type, original width, original height.
	SIZE			m_szOld;
	SIZE			m_szDelta;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedStart();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnEnSetfocusSerial();
	afx_msg void OnEnChangeSerial();
};
