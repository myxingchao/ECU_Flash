#pragma once

#define WM_PROGRESS_PLUS WM_USER + 1001

struct SParam;
class CComPort;
class CSTFlash
{
public:
	CSTFlash();
	virtual ~CSTFlash();

public:
	void	SetParam(CWnd* parent, CComPort* com, SParam* param);
	LONG	FlashECU();

protected:
	LONG	SendECUCmd(BYTE* cmd, LONG len, LONG receive, LONG index, BYTE value);
	void	SaveError(CHAR* title, LONG index, BYTE* msg, LONG len);
	LONG	KFlasherChipID();
	LONG	KFlasherMonitor4B();
	LONG	KFlasherTestBaud();
	LONG	KFlasherErase();
	LONG	KFlasherCheckBank();
	LONG	KFlasherSetBaud();
	LONG	KFlasherTransBin();

protected:
	CWnd*		m_pParent;
	CComPort*	m_pCom;
	SParam*		m_pParam;
	LONG		m_nStep;
};