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
	void	SetColor(DWORD color);		//���ÿ�ʼ��ť��ɫ
	void	SetReady(SParam* config, CComPort* tcom, CComPort* fcom, CHAR* name);//��ʼ������

protected:
	enum { IDD = IDD_CHILD };
	virtual BOOL OnInitDialog();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

protected:
	BOOL	ReadSerialNo();				//��ȡECU���к�
	void	FormatSerial();				//�����кŷ�����ʾ
	void	LoadLabel(SLabel* info);	//���ر�ǩ������Ϣ
	LONG	PrintLabel(SLabel* label);	//��ӡ��ǩ
	void	ShowPrintInfo(LONG error);	//��ʾ��ӡ������Ϣ
	BOOL	ProcTest();					//ʵʩ����
	BOOL	ProcFlash();				//ʵʩˢд
	void	Process();					//���ԡ�ˢд�߳�
	static	UINT ProcThd(LPVOID param);	//���ԡ�ˢд�߳�
	void	WaitThd();					//ECU��μ���߳�
	static	UINT WaitThd(LPVOID param);	//ECU��μ���߳�
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
	LONG	FlashECU();				//ͨ��ECU Bootloaderˢд����
protected:
	DWORD	ComposeCmd(BYTE* cmd, UINT len);
	DWORD	ComposeCmd(BYTE id, BYTE* cmd);
	DWORD	ComposeCmd(BYTE id, BYTE* cmd, UINT len);
	BOOL	ReadAddress(DWORD address, BYTE size, BOOL offset);
	//LONG	SendKLineMsg();			//����K��ͨ��
	BOOL	ReadVersionD(CHAR* info);//ֱ�Ӷ�ȡECU�汾
	BOOL	ReadVersionI(CHAR* info);//ͨ������̨����ȡECU�汾
	BOOL	CheckVersion(CHAR* ver, CHAR* prog, CHAR* data);//�Ƚ�ECU�汾

protected:
	SParam*			m_pParam;		//������Ϣ
	SLabel			m_sLabel;		//��ǩ��Ϣ
	CHAR			m_szSerial[256];//��Ʒ���к�
	CHAR			m_szDataVer[16];//��ǰ���ݰ汾
	CHAR			m_szProgVer[16];//��ǰ����汾
	HANDLE			m_hStop;		//�����ڲ���ˢд�����У���ͣ����ECU�����Ϣ
	HANDLE			m_hExit;		//�˳�ECU��μ���̵߳ı�־λ
	BOOL			m_bNewECU;		//����ʶ��ʼ��ť�Ĺ��ܣ�������������������
	static	HANDLE	m_hPrint;		//��ӡ���б�־
	CWinThread*		m_pProcThd;		//����ˢд�߳�
	CWinThread*		m_pWaitThd;		//ECU��μ���߳�
	CComPort*		m_pTCom;		//����̨��·
	CComPort*		m_pFCom;		//ˢд��·
	LONG			m_nStep;		//ˢд����
	DWORD			m_nInputTm;		//���к������ʱ

	HICON			m_hIcon;		//����
	CColorFrm		m_frmChild;		//�ؼ���
	CColorBtn		m_btnStart;		//��ʼ��ť
	DWORD			m_dwColor;		//��ť��ɫ
	CColorStc		m_stcInfo;		//����
	CHAR			m_szOutput[5 * 256];//��ʾ��Ϣ
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
