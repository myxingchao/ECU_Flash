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
	//打开HEX文件，读出全部内容。
	HANDLE fh = ::CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fh == INVALID_HANDLE_VALUE)
		return NULL;
	DWORD count = ::GetFileSize(fh, NULL);
	CHAR* hbuf = new CHAR[count + 1];
	::ReadFile(fh, hbuf, count, &count, NULL);
	hbuf[count] = 0;
	::CloseHandle(fh);

	//转换为BIN格式。
	HexData2Bin(hbuf, maxsize, reverse, jump);
	delete hbuf;
	return m_pBinBuf;
}
BYTE* CHex2Bin::HexData2Bin(CHAR* data, LONG maxsize, BOOL reverse, BOOL jump)
{
	//重新生成BIN空间。
	if (m_pBinBuf != NULL)
	{
		delete m_pBinBuf;
		m_pBinBuf = NULL;
	}
	m_pBinBuf = new BYTE[maxsize];
	memset(m_pBinBuf, 0xFF, maxsize);

	//将HEX数据转换为BIN。
	LONG hpos = 0;
	LONG base = 0;
	LONG bpos = 0;
	while (data[hpos] != 0)
	{
		//跳过CRC & \r\n。
		if (data[hpos++] != ':')
			continue;
		//得到本行的长度和地址。
		LONG datalen = (Asc2Bin(data[hpos++]) << 4) + Asc2Bin(data[hpos++]);
		LONG address = GetRowAddr(data + hpos);
		hpos += 4;
		//根据设置，忽略前面无数据的部分。
		if (bpos == 0 && jump == TRUE)
			base = 0 - address;

		//得到本行的类型。
		hpos ++;//跳过"0"
		switch (Asc2Bin(data[hpos++]))
		{
			case 0://转换数据
				address += base;
				bpos = address;
				for (int i = 0; i < datalen; i ++)
					m_pBinBuf[bpos++] = (BYTE)((Asc2Bin(data[hpos++]) << 4) + Asc2Bin(data[hpos++]));
				break;
			case 2://更改基准地址
				base = GetRowAddr(data + hpos) << 4;
				hpos += 4;
				break;
			case 4://更改32位基准地址
				base = GetRowAddr(data + hpos) << 16;
				hpos += 4;
				break;
			case 1://文件末尾标志
			default:
				datalen = 0;
				break;
		}
		if (datalen == 0)//(from switch case 1)
			break;
		//保存实际长度。
		if (m_nBinSize < bpos)
			m_nBinSize = bpos;
	}

	//将BIN数据反转。
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
	//从BIN文件中找到不全为FF的行，记录在索引数组中。其中[0]保存行首地址，[1]中保存个数（并且是每行字节数的整倍数）。
	memset(m_nBinIndex, 0, sizeof(m_nBinIndex));
	LONG index = 0;
	LONG pos = 0;
	while (pos < m_nBinSize)
	{
		//逐个查找非空数据。
		while (pos < m_nBinSize && m_pBinBuf[pos] == 0xFF)
			pos ++;
		if (pos == m_nBinSize)
			break;
		//找到非空数据，将其行首地址记入索引。
		pos -= pos % DEF_INDEX_UNIT;
		m_nBinIndex[index][0] = pos;///
		//查找全行为空的数据行。
		while (pos < m_nBinSize)
		{
			//(跳到下一行行首)
			pos += DEF_INDEX_UNIT;
			//(查找行首为空的行)
			while (pos < m_nBinSize && m_pBinBuf[pos] != 0xFF)
				pos += DEF_INDEX_UNIT;
			if (pos == m_nBinSize)
				break;
			//(查找行内非空的数据)
			pos ++;
			while (pos < m_nBinSize && pos % DEF_INDEX_UNIT != 0 && m_pBinBuf[pos] == 0xFF)
				pos ++;
			//(如果未发现数据，则回到行首跳出循环)
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
			else//(如果发现有数据，则回到行首继续)
			{
				pos -= pos % DEF_INDEX_UNIT;
			}
		}
		//找到全行为空的数据行，将此前的非空数据长度记入索引。
		m_nBinIndex[index][1] = pos - m_nBinIndex[index][0];///
		index ++;
	}
	m_nIndexLen = index;
}