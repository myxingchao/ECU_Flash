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
	LONG		nproc;					//Ӧ������
	CHAR		suser[DEF_MAX_NAME];	//��ǰ�û�ID
	CHAR		sadmin[DEF_MAX_NAME];	//����ԱID
	CHAR		smodal[DEF_MAX_NAME];	//��Ʒ�ͺ�
	CHAR		sprog0[DEF_MAX_NAME];	//ˢдǰ����汾
	CHAR		sprog1[DEF_MAX_NAME];	//ˢд�����汾
	CHAR		sdata0[DEF_MAX_NAME];	//ˢдǰ���ݰ汾
	CHAR		sdata1[DEF_MAX_NAME];	//ˢд�����ݰ汾
	CHAR		star[DEF_PATH_STRING];	//ˢд�ļ�·��
	CHAR		slog[DEF_PATH_STRING];	//��־�ļ�·��
	CHAR		slabel[DEF_PATH_STRING];//��ǩ�����ļ�
	CLogFile	flog;					//��־�ļ�����
	DWORD		server;					//������IP��ַ
	//CHAR		serial[16];				//��ǰ���кż���
	CHAR		mailto[DEF_MAX_NAME];	//�ʼ����͵�ַ
	LONG		ntester;				//����̨����
	CHAR		stester[DEF_CHILD_COUNT][DEF_MAX_NAME];
										//����̨����
	CComPort	tester[DEF_CHILD_COUNT];//����̨������Ϣ
	CComPort	flash[DEF_CHILD_COUNT];	//ˢд��������Ϣ
	LONG		index[DEF_CHILD_COUNT];	//�Ӵ���λ�����
	CHex2Bin	chipid;					//StartChipID�ļ�
	CHex2Bin	monitor;				//Monitor004B�ļ�
	CHex2Bin	target;					//ECU�����ļ�
	BYTE*		pchipid;				//StartChipID����
	BYTE*		pmonitor;				//Monitor004B����
	BYTE*		ptarget;				//ECU��������
	LONG		nstep;					//�������ܲ���
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
	static	CHAR	m_szDevName[DEF_CHILD_COUNT][DEF_MAX_NAME];//�����ļ��б���ĸ��Ӵ�������
	static	LONG	m_nIndex[DEF_CHILD_COUNT];//���������б�����Ӧ�Ĺ���̨���
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
