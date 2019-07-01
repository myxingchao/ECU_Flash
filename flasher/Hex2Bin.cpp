#include "stdafx.h"
#include "Hex2Bin.h"

CHex2Bin::CHex2Bin()
{
	m_pBinBuf = NULL;
}
CHex2Bin::~CHex2Bin()
{
	if (m_pBinBuf != NULL)
		delete m_pBinBuf;
}
BYTE* CHex2Bin::HexFile2Bin(CHAR* path, LONG maxsize, BOOL reverse, BOOL jump)
{
	//��HEX�ļ�������ȫ�����ݡ�
	HANDLE fh = ::CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fh == INVALID_HANDLE_VALUE)
		return NULL;
	DWORD count = ::GetFileSize(fh, NULL);
	CHAR* hbuf = new CHAR[count + 1];
	::ReadFile(fh, hbuf, count, &count, NULL);
	hbuf[count] = 0;
	::CloseHandle(fh);

	//ת��ΪBIN��ʽ��
	HexData2Bin(hbuf, maxsize, reverse, jump);
	delete hbuf;
	return m_pBinBuf;
}
BYTE* CHex2Bin::HexData2Bin(CHAR* data, LONG maxsize, BOOL reverse, BOOL jump)
{
	//��������BIN�ռ䡣
	if (m_pBinBuf != NULL)
	{
		delete m_pBinBuf;
		m_pBinBuf = NULL;
	}
	m_pBinBuf = new BYTE[maxsize];
	memset(m_pBinBuf, 0xFF, maxsize);

	//��HEX����ת��ΪBIN��
	LONG hpos = 0;
	LONG base = 0;
	LONG bpos = 0;
	while (data[hpos] != 0)
	{
		//����CRC & \r\n��
		if (data[hpos++] != ':')
			continue;
		//�õ����еĳ��Ⱥ͵�ַ��
		LONG datalen = (Asc2Bin(data[hpos++]) << 4) + Asc2Bin(data[hpos++]);
		LONG address = GetRowAddr(data + hpos);
		hpos += 4;
		//�������ã�����ǰ�������ݵĲ��֡�
		if (bpos == 0 && jump == TRUE)
			base = 0 - address;

		//�õ����е����͡�
		hpos ++;//����"0"
		switch (Asc2Bin(data[hpos++]))
		{
			case 0://ת������
				address += base;
				bpos = address;
				for (int i = 0; i < datalen; i ++)
					m_pBinBuf[bpos++] = (BYTE)((Asc2Bin(data[hpos++]) << 4) + Asc2Bin(data[hpos++]));
				break;
			case 2://���Ļ�׼��ַ
				base = GetRowAddr(data + hpos) << 4;
				hpos += 4;
				break;
			case 4://����32λ��׼��ַ
				base = GetRowAddr(data + hpos) << 16;
				hpos += 4;
				break;
			case 1://�ļ�ĩβ��־
			default:
				datalen = 0;
				break;
		}
		if (datalen == 0)//(from switch case 1)
			break;
		//����ʵ�ʳ��ȡ�
		if (m_nBinSize < bpos)
			m_nBinSize = bpos;
	}

	//��BIN���ݷ�ת��
	if (reverse == TRUE)
	{
		BYTE* tmp = new BYTE[m_nBinSize];
		for (int i = 0; i < m_nBinSize; i ++)
			tmp[i] = m_pBinBuf[m_nBinSize - i - 1];
		memcpy(m_pBinBuf, tmp, m_nBinSize);
		delete tmp;
	}
	return m_pBinBuf;
}
BYTE CHex2Bin::GetBinData(LONG index)
{
	return m_pBinBuf[index];
}
LONG CHex2Bin::GetRealSize()
{
	return m_nBinSize;
}
LONG CHex2Bin::Asc2Bin(CHAR ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	else if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	else
		return 0;
}
LONG CHex2Bin::GetRowAddr(CHAR* txt)
{
	LONG address = 0;	
	for (int i = 0; i < 4; i ++)
	{
		address <<= 4;
		address += Asc2Bin(txt[i]);
	}
	return address;
}
LONG CHex2Bin::GetIndexAddr(LONG index)
{
	return m_nBinIndex[index][0];
}
LONG CHex2Bin::GetIndexSize(LONG index)
{
	return m_nBinIndex[index][1];
}
LONG CHex2Bin::GetIndexLen()
{
	return m_nIndexLen;
}
void CHex2Bin::CreateIndex()
{
	//��BIN�ļ����ҵ���ȫΪFF���У���¼�����������С�����[0]�������׵�ַ��[1]�б��������������ÿ���ֽ���������������
	memset(m_nBinIndex, 0, sizeof(m_nBinIndex));
	LONG index = 0;
	LONG pos = 0;
	while (pos < m_nBinSize)
	{
		//������ҷǿ����ݡ�
		while (pos < m_nBinSize && m_pBinBuf[pos] == 0xFF)
			pos ++;
		if (pos == m_nBinSize)
			break;
		//�ҵ��ǿ����ݣ��������׵�ַ����������
		pos -= pos % DEF_INDEX_UNIT;
		m_nBinIndex[index][0] = pos;///
		//����ȫ��Ϊ�յ������С�
		while (pos < m_nBinSize)
		{
			//(������һ������)
			pos += DEF_INDEX_UNIT;
			//(��������Ϊ�յ���)
			while (pos < m_nBinSize && m_pBinBuf[pos] != 0xFF)
				pos += DEF_INDEX_UNIT;
			if (pos == m_nBinSize)
				break;
			//(�������ڷǿյ�����)
			pos ++;
			while (pos < m_nBinSize && pos % DEF_INDEX_UNIT != 0 && m_pBinBuf[pos] == 0xFF)
				pos ++;
			//(���δ�������ݣ���ص���������ѭ��)
			if (pos == m_nBinSize)
			{
				pos -= pos % DEF_INDEX_UNIT;
				break;
			}
			else if (pos % DEF_INDEX_UNIT == 0)
			{
				pos -= DEF_INDEX_UNIT;
				break;
			}
			else//(������������ݣ���ص����׼���)
			{
				pos -= pos % DEF_INDEX_UNIT;
			}
		}
		//�ҵ�ȫ��Ϊ�յ������У�����ǰ�ķǿ����ݳ��ȼ���������
		m_nBinIndex[index][1] = pos - m_nBinIndex[index][0];///
		index ++;
	}
	m_nIndexLen = index;
}