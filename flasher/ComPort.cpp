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
	//��ͬ����ʽ�򿪴��ڡ�
	strcpy(m_sCom, strstr(cname, "COM"));
	m_hCom = ::CreateFile(cname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (m_hCom == INVALID_HANDLE_VALUE)
		return FALSE;

	// �������������������С��
	::SetupComm(m_hCom, 300, 300);	

	// �趨��ʱ����һ�����뻺���������ݺ���������������أ��������Ƿ������Ҫ����ַ�����
	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = MAXDWORD;
	TimeOuts.ReadTotalTimeoutMultiplier = 0;
	TimeOuts.ReadTotalTimeoutConstant = 0;
	TimeOuts.WriteTotalTimeoutMultiplier = 100;
	TimeOuts.WriteTotalTimeoutConstant = 500;
	::SetCommTimeouts(m_hCom, &TimeOuts);

	//����ͨ�Ų�����
	DCB dcb;
	GetCommState(m_hCom, &dcb);
	dcb.BaudRate = brate;			//������
	dcb.ByteSize = 8;				//ÿ���ֽ���8λ
	dcb.Parity = NOPARITY;			//����żУ��λ
	dcb.StopBits = ONESTOPBIT;		//1λֹͣλ
	::SetCommState(m_hCom, &dcb);	//���ô���
	return TRUE;
}
void CComPort::SetBRate(LONG brate)
{
	//����ͨ�Ų�����
	DCB dcb;
	GetCommState(m_hCom, &dcb);
	dcb.BaudRate = brate;			//������
	dcb.ByteSize = 8;				//ÿ���ֽ���8λ
	dcb.Parity = NOPARITY;			//����żУ��λ
	dcb.StopBits = ONESTOPBIT;		//1λֹͣλ
	::SetCommState(m_hCom, &dcb);	//���ô���
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

	//׼���������ݡ�
	CHAR data[3];
	data[2] = 0;
	BYTE* cmd = new BYTE[len / 2];
	//"0x03A5C3"->03A5C3��
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
	//��ս��ջ��塣
	memset(m_pRBuf, 0, sizeof(m_pRBuf));
	m_nRBuf = 0;

	//ѭ��ֱ���յ����ݣ���ʱ��
	DWORD time = GetTickCount();
	DWORD len = 0;
	while (m_nRBuf < total && ::GetTickCount() - time < tmout)
	{
		//�ȴ�16ms���ɽ�ˢдʱCPUʹ���ʴ�100%����4%���£����ǻ��ӳٵ�7s���ҡ�
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
	//û���յ���Ϣ����������塣
	if (m_nRBuf > 0)
		::PurgeComm(m_hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	return m_nRBuf;
}