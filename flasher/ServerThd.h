#pragma once
#include <winsock2.h>

#define DEFAULT_PORT	"27015"
#define DEFAULT_BLEN	512

class CServerThd
{
protected:
	CServerThd();
	virtual	~CServerThd();
public:
	CHAR*		GetHostIP();

protected:
	static UINT ServerListen(LPVOID param);
	void		ServerListen(CHAR* log);
	static UINT ServerReceive(LPVOID param);
	void		ServerReceive(CHAR* log);
protected:
	CWinThread*	m_pListenThd;
	CWinThread*	m_pServerThd[256];
	HANDLE		m_hExit;
	SOCKET		m_pCurSocket;
	SOCKET		m_pServerSocket[256];
	LONG		m_nThd;
};


/*
#define		ADDINFSIZE			4
#define		SOCKETBUFFSIZE		(32*1024)
#define		ETHMAXTEMPBUFFER	(256*1024)

struct	SEthTransTableSend
{
	char	buffer[ETHMAXTEMPBUFFER + ADDINFSIZE];
	int		now;
	int		length;
	int		remain;
	int		status;
};
struct	SEthTransTableRecv
{
	char*	buffP;
	char*	buffOrgP;				// Delete用保存ポインタ
	int		now;
	int		length;
	int		remain;
	int		status;				// CheckEndに使用。クエリの最終ブロックの場合、０。
};
struct	SNFPowerSettingTable
{
	int		model;
	int		wiring;
	int		rin;
	float	voltage;
	int		range;
	float	frequency;
	int		output;
	float	limit;
	int		header;
	int		rinctrl;
	int		impedance;
	int		sts;
	int		srq;
	int		ers;
};

class CServerThd
{
protected:
	CServerThd();
	virtual	~CServerThd();

public:
	static	UINT ServerThd(LPVOID param);
	CString GetSentLog();
	CString GetRecvLog();

private:
	CWinThread*				m_pServerThd;
	SEthTransTableRecv		m_sRecvTbl;
	SEthTransTableSend		m_sSendTbl;
	int						m_nEtherErrorNo;
	CString					m_strSentLog;
	CString					m_strRecvLog;
	static int				m_nThdNum;

private:
	int		ReadSocket(SOCKET socket, char* buf, int len);
	int		WriteSocket(SOCKET socket, char* buf, int len);
	int		WriteSocketAdd(SOCKET socket, char* buf, int len, int end);
	int		CheckEnd(void);
	int		FlashRoutine(SOCKET socket);
	int		ReceiveSetup(SOCKET socket);
	int		ReceiveOnly(SOCKET socket, char* buf, int blen, int* rlen);
	int		ReceiveBlock(SOCKET socket);

	int		Receive(SOCKET socket, char* buf, int blen, int* rlen);
	int		Send(SOCKET socket, char* msg, int len);

	CString CutLeft(char symbol, CString* inData, bool cutValue);
	void	AddSentLog(char* text);
	void	AddRecvLog(char* text);
	void	ClearLog();
	void	SyncLog();
	void	DisableLog();
}*/