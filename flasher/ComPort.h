#pragma once

#define DEF_MAX_NAME		64
#define DEF_COM_RLEN		4096

class CComPort
{
public:
	CComPort();
	~CComPort();

public:
	BOOL	Connect(CHAR* cname, LONG brate);
	void	Disconnect();
	void	SetBRate(LONG brate);
	BOOL	IsActive();
	CHAR*	GetCom();
	BOOL	SendHex(BYTE* pdata, LONG len);
	BOOL	SendCmd(CHAR* szcmd);
	BOOL	SendAsc(CHAR* szmsg);
	LONG	Receive(LONG total, LONG sleep, LONG tmout);
	BYTE*	GetRBuf();
	BYTE	GetRBuf(LONG index);
	LONG	GetRLen();

public://ooo
	HANDLE	m_hCom;
	LONG	m_nInterval;
	LONG	m_nTimeout;
	LONG	m_nRBuf;
	CHAR	m_sCom[DEF_MAX_NAME];
	BYTE	m_pTBuf[DEF_COM_RLEN];
	BYTE	m_pRBuf[DEF_COM_RLEN];
};