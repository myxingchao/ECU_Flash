#pragma once

#define DEF_INDEX_SIZE	200
#define DEF_INDEX_UNIT	16
class CHex2Bin
{
public:
	CHex2Bin();
	virtual ~CHex2Bin();

public:
	BYTE*	HexData2Bin(CHAR* data, LONG size, BOOL reverse, BOOL jump);
	BYTE*	HexFile2Bin(CHAR* path, LONG size, BOOL reverse, BOOL jump);
	BYTE	GetBinData(LONG index);
	LONG	GetRealSize();
	void	CreateIndex();
	LONG	GetIndexAddr(LONG index);
	LONG	GetIndexSize(LONG index);
	LONG	GetIndexLen();
private:
	LONG	m_nBinPos;
	BYTE*	m_pBinBuf;
	DWORD	m_nBinIndex[DEF_INDEX_SIZE][2];
	LONG	m_nIndexLen;
	LONG	m_nBinSize;

	LONG	GetRowAddr(CHAR* txt);
	LONG	Asc2Bin(CHAR ch);
};