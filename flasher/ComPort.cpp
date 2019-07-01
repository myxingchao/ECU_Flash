#include "StdAfx.h"
#include "ComPort.h"

CComPort::CComPort(void)
{
	m_hCom = INVALID_HANDLE_VALUE;
	m_nRBuf = 0;
}
CComPort::~CComPort(void)
{
}
BOOL CComPort::Connect(CHAR* cname, LONG brate)
{
	//以同步方式打开串口。
	strcpy(m_sCom, strstr(cname, "COM"));
	m_hCom = ::CreateFile(cname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (m_hCom == INVALID_HANDLE_VALUE)
		return FALSE;

	// 设置输入输出缓冲区大小。
	::SetupComm(m_hCom, 300, 300);	

	// 设定超时（读一次输入缓冲区的内容后读操作就立即返回，而不管是否读入了要求的字符）。
	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = MAXDWORD;
	TimeOuts.ReadTotalTimeoutMultiplier = 0;
	TimeOuts.ReadTotalTimeoutConstant = 0;
	TimeOuts.WriteTotalTimeoutMultiplier = 100;
	TimeOuts.WriteTotalTimeoutConstant = 500;
	::SetCommTimeouts(m_hCom, &TimeOuts);

	//设置通信参数。
	DCB dcb;
	GetCommState(m_hCom, &dcb);
	dcb.BaudRate = brate;			//波特率
	dcb.ByteSize = 8;				//每个字节有8位
	dcb.Parity = NOPARITY;			//无奇偶校验位
	dcb.StopBits = ONESTOPBIT;		//1位停止位
	::SetCommState(m_hCom, &dcb);	//设置串口
	return TRUE;
}
void CComPort::SetBRate(LONG brate)
{
	//设置通信参数。
	DCB dcb;
	GetCommState(m_hCom, &dcb);
	dcb.BaudRate = brate;			//波特率
	dcb.ByteSize = 8;				//每个字节有8位
	dcb.Parity = NOPARITY;			//无奇偶校验位
	dcb.StopBits = ONESTOPBIT;		//1位停止位
	::SetCommState(m_hCom, &dcb);	//设置串口
}
void CComPort::Disconnect()
{
	if (m_hCom != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hCom);
	m_hCom = INVALID_HANDLE_VALUE;
}
CHAR* CComPort::GetCom()
{
	return m_sCom;
}
BOOL CComPort::IsActive()
{
	if (m_hCom != INVALID_HANDLE_VALUE)
		return TRUE;
	else
		return FALSE;
}
BYTE* CComPort::GetRBuf()
{
	return m_pRBuf;
}
LONG CComPort::GetRLen()
{
	return m_nRBuf;
}
BYTE CComPort::GetRBuf(LONG index)
{
	return m_pRBuf[index];
}
BOOL CComPort::SendAsc(CHAR* szmsg)
{
	//ooo
	//TRACE("\r\n ----- %5s, ----- %s", m_sCom, szmsg);

	DWORD rlen;
	if (::WriteFile(m_hCom, szmsg, strlen(szmsg), &rlen, NULL) == TRUE && rlen == strlen(szmsg))
		return TRUE;
	else
		return FALSE;
}
BOOL CComPort::SendHex(BYTE* pdata, LONG len)
{
	//ooo
	//TRACE("\r\n ----- %5s, ----- 0x", m_sCom);
	//for (int i = 0; i < len; i ++)
	//	TRACE("%02X", pdata[i]);

	DWORD rlen;
	if (::WriteFile(m_hCom, pdata, len, &rlen, NULL) == TRUE && rlen == len)
		return TRUE;
	else
		return FALSE;
}
BOOL CComPort::SendCmd(CHAR* szcmd)
{
	CHAR* p = szcmd;
	if (strstr(szcmd, "0x") != NULL)
		p += 2;
	DWORD len = strlen(p);

	//准备发送内容。
	CHAR data[3];
	data[2] = 0;
	BYTE* cmd = new BYTE[len / 2];
	//"0x03A5C3"->03A5C3。
	for (int i = 0; i < len / 2; i ++)
	{
		memcpy(data, p + 2 * i, 2);
		cmd[i] = ::strtol(data, NULL, 16);
	}
	//ooo
	//TRACE("\r\n ----- %5s, ----- 0x", m_sCom);
	//for (int i = 0; i < len / 2; i ++)
	//	TRACE("%02X", cmd[i]);

	BOOL ret = ::WriteFile(m_hCom, cmd, len / 2, &len, NULL) == TRUE && len == strlen(p) / 2 ? TRUE : FALSE;
	delete cmd;
	return ret;
}
LONG CComPort::Receive(LONG total, LONG sleep, LONG tmout)
{
	//清空接收缓冲。
	memset(m_pRBuf, 0, sizeof(m_pRBuf));
	m_nRBuf = 0;

	//循环直到收到数据，或超时。
	DWORD time = GetTickCount();
	DWORD len = 0;
	while (m_nRBuf < total && ::GetTickCount() - time < tmout)
	{
		//等待16ms，可将刷写时CPU使用率从100%降至4%以下，但是会延迟到7s左右。
		if (sleep > 0)
			::Sleep(sleep);
		else if (total - m_nRBuf >= 64)
			::Sleep(16);
		if (::ReadFile(m_hCom, m_pRBuf + m_nRBuf, DEF_COM_RLEN, &len, NULL) == FALSE)
			return -1;
		m_nRBuf += len;
	}

	/* ooo
	if (m_nRBuf > 0)
	{
		TRACE("\r\n  %5s, -----  0x", m_sCom);
		for (int i = 0; i < m_nRBuf; i ++)
			TRACE("%02X", m_pRBuf[i]);
	}*/
	//没有收到信息，则不清除缓冲。
	if (m_nRBuf > 0)
		::PurgeComm(m_hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	return m_nRBuf;
}