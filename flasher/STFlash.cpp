#include "stdafx.h"
#include "STFlash.h"
#include "ComPort.h"
#include "Config.h"
#include "LogFile.h"

const WORD _ENCRYPE_A[16] = { 19, 23, 29, 31, 37, 41, 43, 47, 59, 61, 67, 71, 73, 83, 89, 97};
const WORD _ENCRYPE_B[16] = {107,113,127,131,139,149,157,163,173,181,191,199,211,227,233,251};

#define DEF_MAX_TIMEOUT 10000 //because 0xFECA command in erase process needs 6187ms to receive.

struct SEcuMsg
{
	LONG	index;
	BYTE	value;
	LONG	len;
	LONG	receive;
	BYTE*	cmd;
};

CSTFlash::CSTFlash()
{
}
CSTFlash::~CSTFlash()
{
}
void CSTFlash::SetParam(CWnd* parent, CComPort* com, SParam* param)
{
	m_pParent = parent;
	m_pCom = com;
	m_pParam = param;
}
void CSTFlash::SaveError(CHAR* title, LONG index, BYTE* msg, LONG len)
{
	CLogFile log;
	CString strDate("");
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CHAR text[132];

	log.SaveLog(".\\error.log", title);//
	sprintf(text, ": %04d, %04d/%02d/%02d %02d:%02d:%02d, ", index, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	log.SaveLog(".\\error.log", text);//
	for (int i = 0; i < len && i < 64; i ++)
		sprintf(text, "%02X", msg[i]);
	log.SaveLog(".\\error.log", text);//
	sprintf(text, "%s, len = %d\n", len > 64 ? "..." : "", len);
	log.SaveLog(".\\error.log", text);//
}
LONG CSTFlash::SendECUCmd(BYTE* cmd, LONG len, LONG receive, LONG index, BYTE value)
{
	//DWORD ooo = GetTickCount();
	if (len > 0)//(receive once without sending message)
	{
		//TRACE("++%05d++ ", m_nStep);
		//for (int i = 0; i < len && i < 64; i ++)
		//	TRACE("%02X", cmd[i]);
		//TRACE("%s, len = %d\n", len > 64 ? "..." : "", len);
		if (m_pCom->SendHex(cmd, len) == FALSE)
		{
			SaveError("  send ", m_nStep, cmd, len);
			return -1;
		}
	}
	LONG count = m_pCom->Receive(receive, -1, DEF_MAX_TIMEOUT);
	//TRACE("--%05d-- ", m_nStep);
	//for (int i = 0; i < m_pCom->GetRLen() && i < 64; i ++)
	//	TRACE("%02X", m_pCom->GetRBuf(i));
	//TRACE("%s, --			len = %d, receive = %d\n", m_pCom->GetRLen() > 64 ? "..." : "", m_pCom->GetRLen(), GetTickCount() - ooo);
	if (index >= 0 && (m_pCom->GetRBuf(0) != cmd[0] || m_pCom->GetRBuf(index) != value))
	{
		TRACE("--%05d-- ====================================================================================================================\n", m_nStep);
		SaveError("receive", m_nStep, cmd, len);
		return -1;
	}
	//更新进度条。
	m_pParent->PostMessage(WM_PROGRESS_PLUS, m_nStep, 0);
	m_nStep ++;
	return count;
}
LONG CSTFlash::FlashECU()
{
	//读取ECU序列号。(测试前读取一次，这里读取一次，两个模块，不矛盾)
	//BYTE QUERY_SERIAL[] = "What's Your Serial Number?";
	//if (SendECUCmd(QUERY_SERIAL, sizeof(QUERY_SERIAL), 64, -1, 0) < 0)
	//	return 10000;
	//if (m_pCom->GetRLen() == 0)
	//	return 10001;
	//CHAR serial[DEF_MAX_NAME];
	//memcpy(serial, m_pCom->GetRBuf(), m_pCom->GetRLen());

	//设置波特率。
	m_pCom->SetBRate(9600);

	//开始刷写ECU程序。
	LONG ret = 0;
	m_nStep = 0;
	BYTE CMD00[] = {0x00};
	if (SendECUCmd(CMD00, sizeof(CMD00), 2, 1, 0xD5) < 0)
		return 20000;
	//
	ret = KFlasherChipID();
	if (ret != 0)
		return 30000 + ret;
	ret = KFlasherMonitor4B();
	if (ret != 0)
		return 40000 + ret;
	ret = KFlasherTestBaud();
	if (ret != 0)
		return 50000 + ret;
	ret = KFlasherErase();
	if (ret != 0)
		return 60000 + ret;
	ret = KFlasherCheckBank();
	if (ret != 0)
		return 70000 + ret;
	ret = KFlasherSetBaud();
	if (ret != 0)
		return 80000 + ret;
	ret = KFlasherTransBin();
	if (ret != 0)
		return 90000 + ret;

	//恢复ECU序列号。
	//if (SendECUCmd(serial, sizeof(serial), 64, -1, 0) < 0)
	//	return 10003;

	//刷写成功，返回刷写总步数。
	return 0 - m_nStep;
}
//-------------------------------------------------------
LONG CSTFlash::KFlasherChipID(void)
{
	BYTE FLASH_STAGE1[28] = {0xE6,0xF0,0x9A,0xF6,0xE6,0xF1,0x9A,0x00,0x9A,0xB7,0xFE,0x70,0xF3,0xF6,0xB2,0xFE,0xD7,0x00,0x00,0x00,0x89,0x60,0x7E,0xB7,0x28,0x11,0x3D,0xF6};
	BYTE CHIP_ID_ADDR[4] = {0xFA,0x00,0x00,0xF6};
	//发送STAGE1。
	if (SendECUCmd(FLASH_STAGE1, sizeof(FLASH_STAGE1), 28, -1, 0) <= 0)
		return 1000;
	//发送StartChipID的存储地址。
	if (SendECUCmd(CHIP_ID_ADDR, sizeof(CHIP_ID_ADDR), 4, -1, 0) <= 0)
		return 2000;

	//发送StartChipID。
	LONG len = m_pParam->chipid.GetRealSize();
	for (int i = 0; (i + 1) * 28 < len; i ++)
	{
		if (SendECUCmd(m_pParam->pchipid + 28 * i, 28, 28, -1, 0) <= 0)
			return 3000 + i;
	}
	if (SendECUCmd(m_pParam->pchipid + len / 28 * 28, len % 28, 17, -1, 0) <= 0)
		return 4000;
	return 0;
}
LONG CSTFlash::KFlasherMonitor4B(void)
{
	BYTE FLASH_STAGE3[28] = {0xE6,0xF0,0xA8,0xE7,0xE6,0xF1,0xA8,0x07,0x9A,0xB7,0xFE,0x70,0xF3,0xF6,0xB2,0xFE,0xD7,0x00,0x00,0x00,0x89,0x60,0x7E,0xB7,0x28,0x11,0x3D,0xF6};
	BYTE MONITOR_ADDR[4] = {0xFA,0x00,0x00,0xE0};
	//发送E000 ~ E7A8处的程序。
	if (SendECUCmd(FLASH_STAGE3, sizeof(FLASH_STAGE3), 28, -1, 0) <= 0)
		return 1000;
	//发送Monitor4b的存储地址。
	if (SendECUCmd(MONITOR_ADDR, sizeof(MONITOR_ADDR), 4, -1, 0) <= 0)
		return 2000;

	//发送Monitor4B。
	LONG len = m_pParam->monitor.GetRealSize();
	LONG i = 0;
	while ((i + 1) * 28 < len)
	{
		if (SendECUCmd(m_pParam->pmonitor + 28 * i, 28, 28, -1, 0) <= 0)
			return 3000 + i;
		i ++;
	}
	if (SendECUCmd(m_pParam->pmonitor + 28 * i, len - 28 * i, 37, -1, 0) <= 0)
		return 4000;
	return 0;
}
LONG CSTFlash::KFlasherTestBaud(void)
{
	BYTE CMDB7[]	 = {0xB7};
	BYTE CMD1000[]	 = {0x10, 0x00};
	BYTE CMD0100D0[] = {0x01, 0x00, 0xD0, 0xE3, 0x00, 0x00, 0x2A, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00};
	BYTE CMDFECA[]	 = {0xFE, 0xCA};
	BYTE CMD01000A[] = {0x01, 0x00, 0x0A, 0xFE, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00};
	BYTE CMD0200[]	 = {0x02, 0x00};
	BYTE CMD00[]	 = {0x00};
	BYTE CMD2A00[]	 = {0x2A, 0x00};
	BYTE CMD0600[]	 = {0x06, 0x00};
	BYTE CMD04004D[] = {0x04, 0x00, 0x4D, 0x00, 0x4D, 0x00};
	SEcuMsg msgs[]  = {
		//index	value	len	rec	cmd
		{1,		0x7B,	1,	2,	CMDB7},		//0
		{-1,	0,		2,	2,	CMD1000},	//1
		{16,	0xF6,	16,	18,	CMD0100D0},	//2
		{50,	0x5A,	2,	51,	CMDFECA},	//3
		{1,		0x7B,	1,	2,	CMDB7},		//4
		{-1,	0,		2,	2,	CMD1000},	//5
		{16,	0x2B,	16,	18,	CMD01000A},	//6
		{2,		0x40,	2,	13,	CMDFECA},	//7
		{1,		0x7B,	1,	2,	CMDB7},		//8
		{-1,	0,		2,	2,	CMD0200},	//9
		{2,		0x04,	2,	4,	CMD0200},	//10
		{2,		0xA5,	2,	3,	CMDFECA},	//11
		{-1,	0,		1,	3,	CMD00},		//12
		{2,		0x5A,	2,	3,	CMD2A00},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD0600},
		{6,		0xA4,	6,	8,	CMD04004D},
		{2,		0xA5,	2,	3,	CMDFECA},
		{-1,	0,		-1,	0,	NULL},
	};
	//发送预定义消息。
	int i = 0;
	while (msgs[i].len != -1)
	{
		if (SendECUCmd(msgs[i].cmd, msgs[i].len, msgs[i].receive, msgs[i].index, msgs[i].value) <= 0)
			return 1000 + i;
		if (i == 12 && m_pCom->GetRBuf(2) != 0x15 && m_pCom->GetRBuf(2) != 0x16)//CMD00
			return 1000 + i;
		i ++;
	}
	//发送0x00~FF消息。
	BYTE cmd = 0;
	while (cmd < 0xFF)
	{
		if (SendECUCmd(&cmd, 1, 2, 1, (0xFF - cmd)) <= 0)
			return 2000 + cmd;
		cmd ++;
	}
	if (SendECUCmd(&cmd, 1, 3, 1, (0xFF - cmd)) <= 0)
		return 3000;
	if (m_pCom->GetRBuf(2) != 0x5A)
		return 4000;
	return 0;
}
LONG CSTFlash::KFlasherErase(void)
{
	BYTE CMDB7[]	 = {0xB7};
	BYTE CMD0400[]	 = {0x04, 0x00};
	BYTE CMD0201FF[] = {0x02, 0x01, 0xFF, 0x0F};
	BYTE CMDFECA[]	 = {0xFE, 0xCA};
	BYTE CMD005A[]	 = {0x00, 0x5A};
	SEcuMsg msgs[]  = {
		//index	value	len	rec	cmd
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD0400},
		{4,		0x15,	4,	6,	CMD0201FF},
		{3,		0x5A,	2,	4,	CMDFECA},
		//{1,	0x5A,	0,	2,	NULL},//(最后一个FECA消息，需要接收两次，第一次之后大约5秒才会收到第二次回复。第二次接收到的消息内容为0x005A。)
		{-2,	0,		-1,	0,	NULL},
	};
	int i = 0;
	while (msgs[i].len != -1)
	{
		if (SendECUCmd(msgs[i].cmd, msgs[i].len, msgs[i].receive, msgs[i].index, msgs[i].value) <= 0)
			return 1000 + i;
		i ++;
	}
	return 0;
}
LONG CSTFlash::KFlasherCheckBank(void)
{
	BYTE CMDB7[]	 = {0xB7};
	BYTE CMD1000[]	 = {0x10, 0x00};
	BYTE CMD010000[] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x20, 0x00, 0xff, 0xff, 0x00, 0x00};
	BYTE CMDFECA[]	 = {0xFE, 0xCA};
	BYTE CMD012001[] = {0x01, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD014001[] = {0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD016001[] = {0x01, 0x00, 0x00, 0x60, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD018001[] = {0x01, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD00[]	 = {0x00};
	BYTE CMD010002[] = {0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x80, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD010003[] = {0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x80, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD010004[] = {0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x80, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD010005[] = {0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x80, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD010006[] = {0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x80, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD010007[] = {0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x80, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	BYTE CMD010008[] = {0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x80, 0x00, 0x00, 0x20, 0x00, 0xFF, 0xFF, 0x00, 0x00};
	SEcuMsg msgs[]  = {
		//index	value	len	rec	cmd
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0x40,	16,	18,	CMD010000},
		{8,		0x5A,	2,	9,	CMDFECA},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0x60,	16,	18,	CMD012001},
		{8,		0x5A,	2,	9,	CMDFECA},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0x80,	16,	18,	CMD014001},
		{8,		0x5A,	2,	9,	CMDFECA},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xA0,	16,	18,	CMD016001},
		{8,		0x5A,	2,	9,	CMDFECA},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xF0,	16,	18,	CMD018001},
		{8,		0x5A,	2,	9,	CMDFECA},
		//
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xB1,	16,	18,	CMD010002},
		{8,		0x5A,	2,	9,	CMDFECA},
		//{6,	0x5A,	0,	7,	NULL},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xB2,	16,	18,	CMD010003},
		{8,		0x5A,	2,	9,	CMDFECA},
		//{6,	0x5A,	0,	7,	NULL},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xB3,	16,	18,	CMD010004},
		{8,		0x5A,	2,	9,	CMDFECA},
		//{6,	0x5A,	0,	7,	NULL},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xB4,	16,	18,	CMD010005},
		{8,		0x5A,	2,	9,	CMDFECA},
		//{6,	0x5A,	0,	7,	NULL},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xB5,	16,	18,	CMD010006},
		{8,		0x5A,	2,	9,	CMDFECA},
		//{6,	0x5A,	0,	7,	NULL},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xB6,	16,	18,	CMD010007},
		{8,		0x5A,	2,	9,	CMDFECA},
		//{6,	0x5A,	0,	7,	NULL},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD1000},
		{16,	0xB7,	16,	18,	CMD010008},
		{8,		0x5A,	2,	9,	CMDFECA},
		//{6,	0x5A,	0,	7,	NULL},
		{-1,	0,		-1,	0,	NULL},
	};
	//发送预定义消息。
	int i = 0;
	while (msgs[i].len != -1)
	{
		if (SendECUCmd(msgs[i].cmd, msgs[i].len, msgs[i].receive, msgs[i].index, msgs[i].value) <= 0)
			return 1000 + i;
		i ++;
	}
	return 0;
}
LONG CSTFlash::KFlasherSetBaud(void)
{
	BYTE CMD00[]	 = {0x00};
	BYTE CMDB7[]	 = {0xB7};
	BYTE CMD0600[]	 = {0x06, 0x00};
	BYTE CMD04000C[] = {0x04, 0x00, 0x0C, 0x00, 0x4D, 0x00};
	BYTE CMDFECA[]	 = {0xFE, 0xCA};
	SEcuMsg msgs[]  = {
		//index	value	len	rec	cmd
		{-1,	0,		1,	2,	CMD00},
		{1,		0x7B,	1,	2,	CMDB7},
		{-1,	0,		2,	2,	CMD0600},
		{6,		0x63,	6,	8,	CMD04000C},
		{2,		0xA5,	2,	3,	CMDFECA},
		//{5,		0x00,	6,	6,	CMD04000C},
		//{1,		0x80,	2,	2,	CMDFECA},
		{-1,	0,		-1,	0,	NULL},
	};
	//发送预定义消息。
	int i = 0;
	while (msgs[i].len != -1)
	{
		if (SendECUCmd(msgs[i].cmd, msgs[i].len, msgs[i].receive, msgs[i].index, msgs[i].value) <= 0)
			return 1000 + i;
		i ++;
	}

	//设置高波特率。
	m_pCom->SetBRate(57600);
	//发送0x00~FF消息。
	BYTE cmd = 0;
	while (cmd < 0xFF)
	{
		if (SendECUCmd(&cmd, 1, 2, 1, (0xFF - cmd)) <= 0)
			return 2000 + cmd;
		cmd ++;
	}
	if (SendECUCmd(&cmd, 1, 3, 1, (0xFF - cmd)) <= 0)
		return 3000;
	if (m_pCom->GetRBuf(2) != 0x5A)
		return 4000;
	return 0;
}
LONG CSTFlash::KFlasherTransBin()
{
	for (int i = 0; i < m_pParam->target.GetIndexLen(); i ++)
	{
		DWORD maddr = m_pParam->target.GetIndexAddr(i);
		DWORD msize = m_pParam->target.GetIndexSize(i);
		if (maddr == 0 && msize == 0)
			continue;//return 1300 + i;
		//
		BYTE CMDB7[] = {0xB7};
		if (SendECUCmd(CMDB7, sizeof(CMDB7), 2, 1, 0x7B) <= 0)
			return 1100 + i;
		BYTE CMD0A00[] = {0x0A, 0x00};
		if (SendECUCmd(CMD0A00, sizeof(CMD0A00), 2, -1, 0) <= 0)
			return 1200 + i;
		BYTE cmd[256];
		cmd[0] = 0x06;
		cmd[1] = 0x01;
		cmd[2] = maddr % 256;
		cmd[3] = (maddr >> 8) % 256;
		cmd[4] = (maddr >> 16) % 256 == 0 ? 1 : (maddr >> 16) % 256;
		cmd[5] = 0x00;
		cmd[6] = msize % 256;
		cmd[7] = (msize >> 8) % 256;
		cmd[8] = (msize >> 16) % 256;
		cmd[9] = 0x00;
		//
		DWORD CRC = 0;
		for (int j = 0; j < 10; j ++)
			CRC += cmd[j];
		CRC += 0x0A;
		if (SendECUCmd(cmd, 10, 12, 10, CRC % 0x0100) <= 0)
			return 1400 + i;
		if (m_pCom->GetRBuf(11) != CRC / 0x0100)
			return 1500 + i;
		//
		BYTE CMDFECA[] = {0xFE, 0xCA};
		if (SendECUCmd(CMDFECA, sizeof(CMDFECA), 2, -1, 0) <= 0)
			return 1600 + i;
		//
		CRC = 0;
		LONG pos = 0;
		if (msize >= 96)
		{
			while (pos < msize - 96)
			{
				for (int j = 0; j < 96; j++)
				{
					cmd[j] = m_pParam->target.GetBinData(maddr + pos + j);
					CRC += cmd[j];
				}
				if (SendECUCmd(cmd, 96, 96, -1, 0) <= 0)
					return 2000 + 200 * i + pos / 96;
				pos += 96;
			}
		}
		if (pos < msize)
		{
			LONG len = 0;
			if ((msize - pos) % 4 == 0 )
				len = msize - pos;
			else
				len = ((msize - pos) / 4 + 1) * 4;
			for (int j = 0; j < len; j ++)
			{
				cmd[j] = m_pParam->target.GetBinData(maddr + pos + j);
				CRC += cmd[j];
			}
			if (SendECUCmd(cmd, len, len + 3, len + 2, 0x5A) <= 0)
				return 1700 + i;
			if (m_pCom->GetRBuf(len) != (CRC % 256) || m_pCom->GetRBuf(len + 1) != ((CRC >> 8) % 256))
				return 1800 + i;
		}
	}
	return 0;
}