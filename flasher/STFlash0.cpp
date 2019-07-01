#include "stdafx.h"
#include "STFlash.h"
#include "KProtocol.h"

const WORD _ENCRYPE_A[16] = { 19, 23, 29, 31, 37, 41, 43, 47, 59, 61, 67, 71, 73, 83, 89, 97};
const WORD _ENCRYPE_B[16] = {107,113,127,131,139,149,157,163,173,181,191,199,211,227,233,251};

DWORD DataDistribution[200][2];

CSTFlash::CSTFlash()
{
	m_pStartChip = new BYTE[1000];
	m_nDrStart = 0x72000;  //默认状态为去FLASH版ECU
}
CSTFlash::~CSTFlash()
{
}
BOOL CSTFlash::ReadHexFile(CHAR* path)
{
	return TRUE;
}
BYTE* CSTFlash::GetBin()
{
	return NULL;
}
BOOL CSTFlash::SendStartFile()
{
	return TRUE;
}
BOOL CSTFlash::LoadStartFile()
{
	return TRUE;
}

//=====================================================================================//

void CSTFlash::LoadH86File()
{
	CFile fp_new;
	CFileException e_dr;
	CString err_info;
	CFile fp_hex;
	DWORD length;
	BYTE *buf_hex;
	ST_Bin_Lenth = 0;
	fp_h86 = (char *)malloc(0x100000);
	//打开h86文件
	if(fp_hex.Open(_T("./test_00_xu7yib41.h86"), CFile::modeRead, &e_dr))
	{
		length=fp_hex.GetLength();
		buf_hex=(BYTE *)malloc(length+1);
		fp_hex.Read(buf_hex,length);
		buf_hex[length]=0;
		newhex=buf_hex;//将h86文件存放到一个CString里
		delete buf_hex;
		fp_hex.Close();
	}
	else{
		MessageBox(_T("Can't open test_00_xu7yib41.h86"));
		return;
	}
	hex.LoadHex(newhex, 1);
	p_bin_hex=hex.GetBin();
	return ;
}
bool CK_FlasherForCalDlg::fun_ST_KFlasher_startchipid_File_Load(void)
{
	DWORD len;
	CFileException Flash_fileException;	
	CFileException Flash_e_file;
	CFile Flash_startchipid;
	CString startchipid_path;
	char *K_Flasher_startchipid_temp = (char *)malloc(sizeof(BYTE) * 1000);
	char *K_Flasher_startchipid_temp_1 = (char *)malloc(sizeof(BYTE) * 1000);

	GetModuleFileName(NULL,startchipid_path.GetBuffer(MAX_PATH),MAX_PATH);	
	startchipid_path.ReleaseBuffer();
	CString mypath_startchipid = startchipid_path.Left(startchipid_path.ReverseFind('\\') + 1);
	mypath_startchipid = mypath_startchipid.Left(mypath_startchipid.ReverseFind('\\') + 1);
	mypath_startchipid = mypath_startchipid + _T("startchipid.hex");		

	if(Flash_startchipid.Open(mypath_startchipid, CFile::modeRead, &Flash_fileException))
	{
		int file_len = Flash_startchipid.GetLength();

		if(file_len > 0)
		{
			LPTSTR q = (LPTSTR)K_Flasher_startchipid_temp;
			int n = 0;
			int i = 0;
			int j = 0;
			Flash_startchipid.Read(q, file_len -17);
			K_Flasher_startchipid_temp[file_len - 13 - 4] = '\0';
			for(i = 0; i < file_len - 13; i++)
			{
				if(K_Flasher_startchipid_temp[i] == ':'){
					i = i+8;
				}
				else if(K_Flasher_startchipid_temp[i] == '\r'){
					i += 1;
					j = j-2;
				}
				else{
					K_Flasher_startchipid_temp_1[j] = K_Flasher_startchipid_temp[i];
					j++;
				}
			}
			K_Flasher_startchipid_temp_1[j] = '\0';
			j = strlen((char *)K_Flasher_startchipid_temp_1);
			len = strlen((char *)K_Flasher_startchipid_temp);
			memset(K_Flasher_startchipid_temp,NULL,1000);

			for(i = 0; i < j -1; i=i+2)
			{
				if(K_Flasher_startchipid_temp_1[j-i-1] >= '0' && K_Flasher_startchipid_temp_1[j-i-1] <= '9'){
					K_Flasher_startchipid_temp_1[j-i-1] = K_Flasher_startchipid_temp_1[j-i-1] - '0';
				}
				else{
					K_Flasher_startchipid_temp_1[j-i-1] = K_Flasher_startchipid_temp_1[j-i-1] -'A'+10;
				}
				if(K_Flasher_startchipid_temp_1[j-i-2] >= '0' && K_Flasher_startchipid_temp_1[j-i-2] <= '9'){
					K_Flasher_startchipid_temp_1[j-i-2] = K_Flasher_startchipid_temp_1[j-i-2] - '0';
				}
				else{
					K_Flasher_startchipid_temp_1[j-i-2] = K_Flasher_startchipid_temp_1[j-i-2] -'A'+10;
				}
				K_Flasher_startchipid[i/2] = (K_Flasher_startchipid_temp_1[j-i-2]<<4);
				K_Flasher_startchipid[i/2] += K_Flasher_startchipid_temp_1[j-i-1];
			}
			startchipid_len = i/2;
		}
		else
		{
			CString ifo_Error = _T("can not open ") + mypath_startchipid;
			MessageBox(ifo_Error);
			return 0;
		}
	}
	else{
		MessageBox(_T("NULL!"));
		return 0;
	}
	Flash_startchipid.Close();
	free(K_Flasher_startchipid_temp);
	free(K_Flasher_startchipid_temp_1);
	Sleep(200);
	return 1;
}
bool CK_FlasherForCalDlg::fun_ST_KFlasher_Monitor4b_File_Load(void)
{
	DWORD len;
	CFileException Flash_fileException;	
	CFileException Flash_e_file;
	CFile Flash_Monitor4b;
	CString Monitor4b_path;
	char *K_Flasher_Monitor4b_temp = (char *)malloc(sizeof(BYTE) * 20000);
	char *K_Flasher_Monitor4b_temp_1 = (char *)malloc(sizeof(BYTE) * 20000);

	GetModuleFileName(NULL,Monitor4b_path.GetBuffer(MAX_PATH),MAX_PATH);	
	Monitor4b_path.ReleaseBuffer();
	CString mypath_Monitor4b = Monitor4b_path.Left(Monitor4b_path.ReverseFind('\\') + 1);
	mypath_Monitor4b = mypath_Monitor4b.Left(mypath_Monitor4b.ReverseFind('\\') + 1);
	mypath_Monitor4b = mypath_Monitor4b + _T("Monitor004b.bin");		

	if(Flash_Monitor4b.Open(mypath_Monitor4b, CFile::modeRead, &Flash_fileException))
	{
		int Monitor4b_file_len = Flash_Monitor4b.GetLength();

		if(Monitor4b_file_len > 0)
		{
			//*
			LPTSTR Monitor4b_q = (LPTSTR)K_Flasher_Monitor4b_temp;
			int n = 0;
			int i = 0;
			int j = 0;
			LONG lActual = Flash_Monitor4b.Seek(0xe000, CFile::begin);
			Flash_Monitor4b.Read(Monitor4b_q, 0x7a8);//0xe7a9
			K_Flasher_Monitor4b_temp[0x7a8] = '\0';

			len = strlen((char *)K_Flasher_Monitor4b_temp);
			j = 0x7a8;
			for(i = 0; i < j; i++)
			{
				K_Flasher_Monitor4b[i] = (K_Flasher_Monitor4b_temp[j-i-1]);
			}
			K_Flasher_Monitor4b[0x7a8] = '\0';
			Monitor4b_len = 0x7a8;
			//*/
			/*
			int n = 0;
			int i = 0;
			int j = 0;
			LPTSTR Monitor4b_q = (LPTSTR)K_Flasher_Monitor4b_temp;
			Flash_Monitor4b.Read(Monitor4b_q, Monitor4b_file_len -19);
			K_Flasher_Monitor4b_temp[Monitor4b_file_len - 19] = '\0';
			for(i = 0; i < Monitor4b_file_len - 13; i++)
			{
				if(K_Flasher_Monitor4b_temp[i] == ':'){
					i = i+8;
				}
				else if(K_Flasher_Monitor4b_temp[i] == '\r'){
					i += 2;
					j = j-2;
				}
				else{
					K_Flasher_Monitor4b_temp_1[j] = K_Flasher_Monitor4b_temp[i];
					j++;
				}
			}
			K_Flasher_Monitor4b_temp_1[j] = '\0';
			j = strlen((char *)K_Flasher_Monitor4b_temp_1);
			len = strlen((char *)K_Flasher_Monitor4b_temp);
			memset(K_Flasher_Monitor4b_temp,NULL,20000);

			for(i = 0; i < j -1; i=i+2)
			{
				if(K_Flasher_Monitor4b_temp_1[j-i-1] >= '0' && K_Flasher_Monitor4b_temp_1[j-i-1] <= '9'){
					K_Flasher_Monitor4b_temp_1[j-i-1] = K_Flasher_Monitor4b_temp_1[j-i-1] - '0';
				}
				else{
					K_Flasher_Monitor4b_temp_1[j-i-1] = K_Flasher_Monitor4b_temp_1[j-i-1] -'A'+10;
				}
				if(K_Flasher_Monitor4b_temp_1[j-i-2] >= '0' && K_Flasher_Monitor4b_temp_1[j-i-2] <= '9'){
					K_Flasher_Monitor4b_temp_1[j-i-2] = K_Flasher_Monitor4b_temp_1[j-i-2] - '0';
				}
				else{
					K_Flasher_Monitor4b_temp_1[j-i-2] = K_Flasher_Monitor4b_temp_1[j-i-2] -'A'+10;
				}
				K_Flasher_Monitor4b[i/2] = (K_Flasher_Monitor4b_temp_1[j-i-2]<<4);
				K_Flasher_Monitor4b[i/2] += K_Flasher_Monitor4b_temp_1[j-i-1];
			}
			Monitor4b_len = i/2;
			*/
		}
		else
		{
			CString ifo_Error = _T("can not open ") + mypath_Monitor4b;
			MessageBox(ifo_Error);
			return 0;
		}
	}
	else{
		MessageBox(_T("NULL!"));
		return 0;
	}
	Flash_Monitor4b.Close();
	free(K_Flasher_Monitor4b_temp);
	free(K_Flasher_Monitor4b_temp_1);
	Sleep(200);
	return 1;
}
void CSTFlash::Flash_H86(void)
{
	BOOL bWriteStat;
	BYTE* PK_rdata_buf;
	DWORD len;
	WORD result=1;
	BOOL Pro_State = 0;

	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	Recive_Time = 20;
	K_tdata_buf[0] = 0x00;
	len = 1;
	bWriteStat = WriteFile(hCom, K_rdata_buf, len, &len, NULL);
	m_NeedTesterPresent = 0;

	if(bWriteStat)
	{
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);
	}
	if(K_rdata_buf[0] == 0x00 && K_rdata_buf[1] == 0xd5)
	{
		fun_resetDataDistribution();
		fun_formatData_bin();
		if (fun_ST_KFlasher_stage1() == FALSE)
			return FALSE;
		if (fun_ST_KFlasher_startchipid() == FALSE)
			return FALSE;
		if (fun_ST_KFlasher_monitor4b() == FALSE)
			return FALSE;
		if (fun_ST_KFlasher_baundTest() == FALSE)
			return FALSE;
		if (fun_ST_KFlasher_Erase() == FALSE)
			return FALSE;
		if (fun_ST_KFlasher_BankCheck() == FALSE)
			return FALSE;
		if (fun_ST_KFlasher_baundSet() == FALSE)
			return FALSE;
		//Recive_Time = 50;
		//PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
		fun_ST_KFlasher_TransBin();

		if(Pro_State == 1)
			SetTimer(1, 2000, NULL);
		else
			MessageBox(_T("Connect Error"));
	}
	else
	{
		CString ifo_Error = _T("");
		ifo_Error.Format(_T("%x%x"),K_rdata_buf[0],K_rdata_buf[1]);
		MessageBox(ifo_Error);
		return;
	}
}
void CSTFlash::fun_resetDataDistribution(void)
{
	for(int clear_i = 0; clear_i < 200; clear_i++){
		DataDistribution[clear_i][0] = 0;
		DataDistribution[clear_i][1] = 0;
	}
}
WORD CSTFlash::fun_formatData_bin(void)
{
	DWORD i=0;
	int j=0;
	int k=0;

	do{
		// 如果一直是非空，就一直继续找
		if((i%16) == 0 && p_bin_hex[i] != 0xFF)
		{
			i = i + 16;
		}
		else// 找到了一个非空的字节
		{
			// 从此非空的数据向后数，一直找256个字节
			for(j=0; j<16; j++)
			{
				if(p_bin_hex[i + j] != 0xFF)
					break;
			}

			if(j < 16)// 如果还没到256个字节就又有数据，则继续找
			{
				i += 16;
			}
			else// 如果找到了一个256个字节都是空的段
			{
				DataDistribution[k][1] = i - DataDistribution[k][0];// 将此之前的地址记录下来
				i += j;
				do{  // 从此地址继续找，直到找到一个非空的数据，将其地址记录下来
					if(p_bin_hex[i] != 0xFF)
					{
						i = i - (i%16);
						DataDistribution[k+1][0] = i;
						k++;
						break;
					}
					i++;
				} while(i < BIN_LENGTH);
			}
			i = i - (i%16);
		}
	} while(i < BIN_LENGTH);

	DataDistributionIndex = k;
	
	if(k > 5)
		return 0;
	else
		return 1;
}

bool CSTFlash::fun_ST_KFlasher_stage1(void)
{
	BOOL bWriteStat;
	BYTE* PK_rdata_buf;
	DWORD len;
	WORD result=1;
	BYTE K_Flasher_Stage1[28] = {0xE6,0xF0,0x9A,0xF6,0xE6,0xF1,0x9A,0x00,0x9A,0xB7,0xFE,0x70,0xF3,0xF6,0xB2,0xFE,0xD7,0x00,0x00,0x00,0x89,0x60,0x7E,0xB7,0x28,0x11,0x3D,0xF6};
	BYTE K_Flasher_startchipid_Address[4] = {0xfa,0x00,0x00,0xf6};
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	Recive_Time = 10;
	len = 28;
	bWriteStat = WriteFile(hCom, K_Flasher_Stage1, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	len = 4;
	bWriteStat = WriteFile(hCom, K_Flasher_startchipid_Address, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	Sleep(5);
	return 1;
}
bool CSTFlash::fun_ST_KFlasher_startchipid(void)
{
	BOOL bWriteStat;
	BYTE* PK_rdata_buf;
	DWORD len = 0;
	WORD result = 1;
	int i = 0;

	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	Recive_Time = 5;

	for(i = 0; (i + 1)*28 < startchipid_len; i++)
	{
		len = 28;
		bWriteStat = WriteFile(hCom, &K_Flasher_startchipid[i*28], len, &len, NULL);
		Sleep(1);
		OnReceive((LPVOID)this);
		Sleep(1);
	}
	len = startchipid_len - 28 * i;
	bWriteStat = WriteFile(hCom, &K_Flasher_startchipid[28 * (i)], len, &len, NULL);
	Recive_Time = 20;			
	OnReceive((LPVOID)this);
	//			


	len = strlen((char *)K_Flasher_startchipid);
	bWriteStat = WriteFile(hCom, K_Flasher_startchipid, len, &len, NULL);
	m_NeedTesterPresent = 0;

	if(bWriteStat)
		m_CommandNow = EcuReset;
	return 1;
}
bool CSTFlash::fun_ST_KFlasher_monitor4b(void)
{
	BOOL bWriteStat;
	BYTE* PK_rdata_buf;
	DWORD len;
	WORD result=1;
	int i = 0;
	BYTE K_Flasher_Stage3[28] = {0xe6,0xf0,0xa8,0xe7,0xe6,0xf1,0xa8,0x07,0x9A,0xB7,0xFE,0x70,0xF3,0xF6,0xB2,0xFE,0xD7,0x00,0x00,0x00,0x89,0x60,0x7E,0xB7,0x28,0x11,0x3D,0xF6};
	BYTE K_Flasher_monitor4b_Address[4] = {0xfa,0x00,0x00,0xe0};
	Recive_Time  = 5;
//--------------------------------------发送E000 ~ E7A8处的程序--------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 28;
	bWriteStat = WriteFile(hCom, K_Flasher_Stage3, len, &len, NULL);
	m_NeedTesterPresent = 0;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);

	}
//--------------------------------------发送Monitor4b的存储地址--------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 4;
	bWriteStat = WriteFile(hCom, K_Flasher_monitor4b_Address, len, &len, NULL);
	m_NeedTesterPresent = 0;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);

	}
//--------------------------------------发送Monitor4b----------------------------------//
	//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
//		
	for(i = 0; (i + 1)*28 < Monitor4b_len;i++)
	{
		len = 28;
		bWriteStat = WriteFile(hCom, &K_Flasher_Monitor4b[i*28], len, &len, NULL);
		Sleep(1);
		OnReceive((LPVOID)this);
		Sleep(1);
	}
	len = Monitor4b_len - 28 * i;
	bWriteStat = WriteFile(hCom, &K_Flasher_Monitor4b[28 * i], len, &len, NULL);
	Sleep(100);
	Recive_Time = 20;
	OnReceive((LPVOID)this);

	return 1;
}
bool CSTFlash::fun_ST_KFlasher_baundTest(void)
{
	int i = 0;
	BOOL bWriteStat;
	BYTE* PK_rdata_buf;
	DWORD len;
	WORD result=1;
	BYTE K_Flasher_baundTest_B7 = 0xb7;
	BYTE K_Flasher_baundTest_Data = 0x00;
	CString err_ifo;
	CString err_ifo_temp;

/*	BYTE K_Flasher_baundTest[28] = {\
		0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,\
		0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,\
		0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,\
		0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,\
		0x30,0x31,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,\
		0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,\
		0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,\
		0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,\
		0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,\
		0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,\
		0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,\
		0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,\
		\};*/
	BYTE K_Flasher_startchipid_Address[4] = {0xfa,0x00,0x00,0xf6};
	Recive_Time  = 50;
//----------------------------------------b7-----------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	bWriteStat = WriteFile(hCom, &K_Flasher_baundTest_B7, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_Flasher_baundTest_B7 && K_rdata_buf[1] == 0x7b){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------10 00---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x10;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	Sleep(5);

//------------01 00 d0 e3  00 00 2a 00 00 00 08 00  00 00 00 00----------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x01;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0xd0;
	K_tdata_buf[3] = 0xe3;
	K_tdata_buf[4] = 0x00;
	K_tdata_buf[5] = 0x00;
	K_tdata_buf[6] = 0x2a;
	K_tdata_buf[7] = 0x00;
	K_tdata_buf[8] = 0x00;
	K_tdata_buf[9] = 0x00;
	K_tdata_buf[10] = 0x08;
	K_tdata_buf[11] = 0x00;
	K_tdata_buf[12] = 0x00;
	K_tdata_buf[13] = 0x00;
	K_tdata_buf[14] = 0x00;
	K_tdata_buf[15] = 0x00;
	len = 16;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[16] == 0xf6){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);
//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[2] == 0x01){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);
//----------------------------------------b7-----------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	bWriteStat = WriteFile(hCom, &K_Flasher_baundTest_B7, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_Flasher_baundTest_B7 && K_rdata_buf[1] == 0x7b){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------10 00---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x10;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	Sleep(5);

//------------01 00 0a fe  00 00 02 00 00 00 10 00  00 00 00 00----------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x01;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x0a;
	K_tdata_buf[3] = 0xfe;
	K_tdata_buf[4] = 0x00;
	K_tdata_buf[5] = 0x00;
	K_tdata_buf[6] = 0x02;
	K_tdata_buf[7] = 0x00;
	K_tdata_buf[8] = 0x00;
	K_tdata_buf[9] = 0x00;
	K_tdata_buf[10] = 0x10;
	K_tdata_buf[11] = 0x00;
	K_tdata_buf[12] = 0x00;
	K_tdata_buf[13] = 0x00;
	K_tdata_buf[14] = 0x00;
	K_tdata_buf[15] = 0x00;
	len = 16;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[16] == 0x2b){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[2] == 0x40){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------b7-----------------------------------//
	len = 1;
	bWriteStat = WriteFile(hCom, &K_Flasher_baundTest_B7, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_Flasher_baundTest_B7 && K_rdata_buf[1] == 0x7b){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------02 00---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x02;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	/*
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[2] == 0x04){
	}
	else{
	//	OnReceive((LPVOID)this);
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}*/
	Sleep(5);

//----------------------------------------02 00---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x02;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[2] == 0x04){
	}
	else{
	//	OnReceive((LPVOID)this);
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[2] == 0xa5){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------00 -----------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x00;
	len = 1;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[2] == 0x15){
	}
	else{
		/*
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
		*/
	}
	Sleep(5);

//----------------------------------------2a 00---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x2a;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[2] == 0x5a){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------b7-----------------------------------//
	len = 1;
	bWriteStat = WriteFile(hCom, &K_Flasher_baundTest_B7, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_Flasher_baundTest_B7 && K_rdata_buf[1] == 0x7b){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------06 00---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x06;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	Sleep(5);
 
//--------------------------------04 00 4d 00  4d 00 --------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x04;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x4d;
	K_tdata_buf[3] = 0x00;
	K_tdata_buf[4] = 0x4d;
	K_tdata_buf[5] = 0x00;
	len = 6;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[6] == 0xa4){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[2] == 0xa5){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------data 0~0xff---------------------------------//
	Recive_Time  = 20;
	for(K_Flasher_baundTest_Data = 0x00; K_Flasher_baundTest_Data <= 0xFF; K_Flasher_baundTest_Data++){
		memset(K_tdata_buf, '\0', K_MAX_FRAME);
		memset(K_rdata_buf, '\0', K_MAX_FRAME);
		len = 1;
		bWriteStat = WriteFile(hCom, &K_Flasher_baundTest_Data, len, &len, NULL);
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == K_Flasher_baundTest_Data && K_rdata_buf[1] == (0xff - K_Flasher_baundTest_Data)){
			if(K_Flasher_baundTest_Data == 0xFF && K_rdata_buf[2] != 0x5a){	
				err_ifo = "Error Infomation ";
				for(i=0; i<K_rdata_len; i++){
					err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
					err_ifo += err_ifo_temp;
				}
				MessageBox(err_ifo);
				Recive_Time  = 50;
				return 0;
			}
			else if(K_Flasher_baundTest_Data == 0xFF && K_rdata_buf[2] == 0x5a){
				Recive_Time  = 50;
				return 1;
			}
		}
		else{
			err_ifo = "Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			Recive_Time  = 50;
			return 0;
		}		
	}
	Recive_Time  = 50;
	return 1;
}

bool CSTFlash::fun_ST_KFlasher_Erase(void)
{

	BOOL bWriteStat;
	BYTE* PK_rdata_buf;
	DWORD len;
	WORD result=1;
	int i = 0;
	CString err_ifo;
	CString err_ifo_temp;
//-------------------------------------- b7 ------------------------------------------//
	m_NeedTesterPresent = 0;
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0xb7;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	m_NeedTesterPresent = 0;
	Recive_Time  = 50;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
		}
		else{
			err_ifo = "Erase Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			return 0;
		}
		Sleep(5);
	}

//--------------------------------------0x40----------------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x04;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);

//--------------------------------------02 01 ff 0f ----------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x02;
	K_tdata_buf[1] = 0x01;
	K_tdata_buf[2] = 0xff;
	K_tdata_buf[3] = 0x0f;
	len = 4;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[4] == 0x15){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	Sleep(2000);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == 0x00 && K_rdata_buf[1] == 0x5a){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

	return 1;
}
bool CSTFlash::fun_ST_KFlasher_BankCheck(void)
{
	BOOL bWriteStat;
	BYTE* PK_rdata_buf;
	DWORD len;
	WORD result=1;
	int i = 0;
	CString err_ifo;
	CString err_ifo_temp;
//-----------------------------------------------------------------------------------//
//-------------------------检查10000~12000是否为空-----------------------------------//
//-----------------------------------------------------------------------------------//
//-------------------------------------- b7 ------------------------------------------//
	m_NeedTesterPresent = 0;
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0xb7;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	m_NeedTesterPresent = 0;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		Sleep(5);
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
		}
		else{
			err_ifo = "Erase Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			return 0;
		}
		Sleep(5);
	}

//--------------------------------------10 00----------------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x10;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);

//-------------------01 00 00 00 01 00 00 10 00 00 20 00 FF FF 00 00-----------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	BYTE buf[16] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, x020, 0x00, 0xff, 0xff, 0x00, 0x00};
	K_tdata_buf[0] = 0x01;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x00;
	K_tdata_buf[3] = 0x00;
	K_tdata_buf[4] = 0x01;
	K_tdata_buf[5] = 0x00;
	K_tdata_buf[6] = 0x00;
	K_tdata_buf[7] = 0x10;
	K_tdata_buf[8] = 0x00;
	K_tdata_buf[9] = 0x00;
	K_tdata_buf[10] = 0x20;
	K_tdata_buf[11] = 0x00;
	K_tdata_buf[12] = 0xff;
	K_tdata_buf[13] = 0xff;
	K_tdata_buf[14] = 0x00;
	K_tdata_buf[15] = 0x00;
	len = 16;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[16] == 0x40){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00){
	}
	else{
		err_ifo = "NOT BANK Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);
//-----------------------------------------------------------------------------------//
//-------------------------检查12000~14000是否为空-----------------------------------//
//-----------------------------------------------------------------------------------//
	m_NeedTesterPresent = 0;
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0xb7;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	m_NeedTesterPresent = 0;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
		}
		else{
			err_ifo = "Erase Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			return 0;
		}
		Sleep(5);
	}

//--------------------------------------10 00----------------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x10;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);

//-------------------01 00 00 20 01 00 00 10 00 00 20 00 FF FF 00 00-----------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x01;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x00;
	K_tdata_buf[3] = 0x20;
	K_tdata_buf[4] = 0x01;
	K_tdata_buf[5] = 0x00;
	K_tdata_buf[6] = 0x00;
	K_tdata_buf[7] = 0x10;
	K_tdata_buf[8] = 0x00;
	K_tdata_buf[9] = 0x00;
	K_tdata_buf[10] = 0x20;
	K_tdata_buf[11] = 0x00;
	K_tdata_buf[12] = 0xff;
	K_tdata_buf[13] = 0xff;
	K_tdata_buf[14] = 0x00;
	K_tdata_buf[15] = 0x00;
	len = 16;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[16] == 0x60){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00){
	}
	else{
		err_ifo = "NOT BANK Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);
//-----------------------------------------------------------------------------------//
//-------------------------检查14000~16000是否为空-----------------------------------//
//-----------------------------------------------------------------------------------//
	m_NeedTesterPresent = 0;
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0xb7;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	m_NeedTesterPresent = 0;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
		}
		else{
			err_ifo = "Erase Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			return 0;
		}
		Sleep(5);
	}

//--------------------------------------10 00----------------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x10;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);

//-------------------01 00 00 40 01 00 00 10 00 00 20 00 FF FF 00 00-----------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x01;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x00;
	K_tdata_buf[3] = 0x40;
	K_tdata_buf[4] = 0x01;
	K_tdata_buf[5] = 0x00;
	K_tdata_buf[6] = 0x00;
	K_tdata_buf[7] = 0x10;
	K_tdata_buf[8] = 0x00;
	K_tdata_buf[9] = 0x00;
	K_tdata_buf[10] = 0x20;
	K_tdata_buf[11] = 0x00;
	K_tdata_buf[12] = 0xff;
	K_tdata_buf[13] = 0xff;
	K_tdata_buf[14] = 0x00;
	K_tdata_buf[15] = 0x00;
	len = 16;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[16] == 0x80){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00){
	}
	else{
		err_ifo = "NOT BANK Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);
//-----------------------------------------------------------------------------------//
//-------------------------检查16000~18000是否为空-----------------------------------//
//-----------------------------------------------------------------------------------//
	m_NeedTesterPresent = 0;
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0xb7;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	m_NeedTesterPresent = 0;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
		}
		else{
			err_ifo = "Erase Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			return 0;
		}
		Sleep(5);
	}

//--------------------------------------10 00----------------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x10;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);

//-------------------01 00 00 60 01 00 00 10 00 00 20 00 FF FF 00 00-----------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x01;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x00;
	K_tdata_buf[3] = 0x60;
	K_tdata_buf[4] = 0x01;
	K_tdata_buf[5] = 0x00;
	K_tdata_buf[6] = 0x00;
	K_tdata_buf[7] = 0x10;
	K_tdata_buf[8] = 0x00;
	K_tdata_buf[9] = 0x00;
	K_tdata_buf[10] = 0x20;
	K_tdata_buf[11] = 0x00;
	K_tdata_buf[12] = 0xff;
	K_tdata_buf[13] = 0xff;
	K_tdata_buf[14] = 0x00;
	K_tdata_buf[15] = 0x00;
	len = 16;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[16] == 0xa0){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00){
	}
	else{
		err_ifo = "NOT BANK Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);
//-----------------------------------------------------------------------------------//
//-------------------------检查18000~20000是否为空-----------------------------------//
//-----------------------------------------------------------------------------------//
	m_NeedTesterPresent = 0;
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0xb7;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	m_NeedTesterPresent = 0;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
		}
		else{
			err_ifo = "Erase Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			return 0;
		}
		Sleep(5);
	}

//--------------------------------------10 00----------------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x10;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);

//-------------------01 00 00 80 01 00 00 40 00 00 20 00 FF FF 00 00-----------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x01;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x00;
	K_tdata_buf[3] = 0x80;
	K_tdata_buf[4] = 0x01;
	K_tdata_buf[5] = 0x00;
	K_tdata_buf[6] = 0x00;
	K_tdata_buf[7] = 0x40;
	K_tdata_buf[8] = 0x00;
	K_tdata_buf[9] = 0x00;
	K_tdata_buf[10] = 0x20;
	K_tdata_buf[11] = 0x00;
	K_tdata_buf[12] = 0xff;
	K_tdata_buf[13] = 0xff;
	K_tdata_buf[14] = 0x00;
	K_tdata_buf[15] = 0x00;
	len = 16;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[16] == 0xf0){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00){
	}
	else{
		err_ifo = "NOT BANK Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);
//-----------------------------------------------------------------------------------//
//-------------------------检查20000~30000是否为空-----------------------------------//
//-----------------------------------------------------------------------------------//
	m_NeedTesterPresent = 0;
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0xb7;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	m_NeedTesterPresent = 0;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
		}
		else{
			err_ifo = "Erase Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			return 0;
		}
		Sleep(5);
	}

//--------------------------------------10 00----------------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x10;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);

//-------------------01 00 00 00 02 00 00 80 00 00 20 00 FF FF 00 00-----------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x01;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x00;
	K_tdata_buf[3] = 0x00;
	K_tdata_buf[4] = 0x02;
	K_tdata_buf[5] = 0x00;
	K_tdata_buf[6] = 0x00;
	K_tdata_buf[7] = 0x80;
	K_tdata_buf[8] = 0x00;
	K_tdata_buf[9] = 0x00;
	K_tdata_buf[10] = 0x20;
	K_tdata_buf[11] = 0x00;
	K_tdata_buf[12] = 0xff;
	K_tdata_buf[13] = 0xff;
	K_tdata_buf[14] = 0x00;
	K_tdata_buf[15] = 0x00;
	len = 16;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(20);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[16] == 0xb1){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(50);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(20);
	OnReceive((LPVOID)this);
	Sleep(20);
	OnReceive((LPVOID)this);
	//此处接收2次，可能会有问题
	//*
	if((K_rdata_buf[0] == 0x00 && K_rdata_buf[1] == 0x00 && K_rdata_buf[3] == 0x00) || (K_rdata_buf[0] == 0xfe && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00 && K_rdata_buf[4] == 0x00)){
	}
	else{
		err_ifo = "NOT BANK Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	//*/
	Sleep(50);
//-----------------------------------------------------------------------------------//
//-------------------------检查30000~40000是否为空-----------------------------------//
//-----------------------------------------------------------------------------------//
	m_NeedTesterPresent = 0;
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0xb7;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	m_NeedTesterPresent = 0;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
		}
		else{
			err_ifo = "Erase Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			return 0;
		}
		Sleep(5);
	}

//--------------------------------------10 00----------------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x10;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);

//-------------------01 00 00 00 03 00 00 80 00 00 20 00 FF FF 00 00-----------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x01;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x00;
	K_tdata_buf[3] = 0x00;
	K_tdata_buf[4] = 0x03;
	K_tdata_buf[5] = 0x00;
	K_tdata_buf[6] = 0x00;
	K_tdata_buf[7] = 0x80;
	K_tdata_buf[8] = 0x00;
	K_tdata_buf[9] = 0x00;
	K_tdata_buf[10] = 0x20;
	K_tdata_buf[11] = 0x00;
	K_tdata_buf[12] = 0xff;
	K_tdata_buf[13] = 0xff;
	K_tdata_buf[14] = 0x00;
	K_tdata_buf[15] = 0x00;
	len = 16;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[16] == 0xb2){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(20);
	OnReceive((LPVOID)this);
	Sleep(20);
	OnReceive((LPVOID)this);
	if((K_rdata_buf[0] == 0x00 && K_rdata_buf[1] == 0x00 && K_rdata_buf[3] == 0x00) || (K_rdata_buf[0] == 0xfe && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00 && K_rdata_buf[4] == 0x00)){

//	if(K_rdata_buf[0] == 0x00 && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00){
	}
	else{
		err_ifo = "NOT BANK Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);
//-----------------------------------------------------------------------------------//
//-------------------------检查40000~50000是否为空-----------------------------------//
//-----------------------------------------------------------------------------------//
	m_NeedTesterPresent = 0;
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0xb7;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	m_NeedTesterPresent = 0;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
		}
		else{
			err_ifo = "Erase Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			return 0;
		}
		Sleep(5);
	}

//--------------------------------------10 00----------------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x10;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);

//-------------------01 00 00 00 04 00 00 80 00 00 20 00 FF FF 00 00-----------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x01;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x00;
	K_tdata_buf[3] = 0x00;
	K_tdata_buf[4] = 0x04;
	K_tdata_buf[5] = 0x00;
	K_tdata_buf[6] = 0x00;
	K_tdata_buf[7] = 0x80;
	K_tdata_buf[8] = 0x00;
	K_tdata_buf[9] = 0x00;
	K_tdata_buf[10] = 0x20;
	K_tdata_buf[11] = 0x00;
	K_tdata_buf[12] = 0xff;
	K_tdata_buf[13] = 0xff;
	K_tdata_buf[14] = 0x00;
	K_tdata_buf[15] = 0x00;
	len = 16;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[16] == 0xb3){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(20);
	OnReceive((LPVOID)this);
	Sleep(20);
	OnReceive((LPVOID)this);
	if((K_rdata_buf[0] == 0x00 && K_rdata_buf[1] == 0x00 && K_rdata_buf[3] == 0x00) || (K_rdata_buf[0] == 0xfe && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00 && K_rdata_buf[4] == 0x00)){

//	if(K_rdata_buf[0] == 0x00 && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00){
	}
	else{
		err_ifo = "NOT BANK Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);
//-----------------------------------------------------------------------------------//
//-------------------------检查50000~60000是否为空-----------------------------------//
//-----------------------------------------------------------------------------------//
	m_NeedTesterPresent = 0;
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0xb7;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	m_NeedTesterPresent = 0;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
		}
		else{
			err_ifo = "Erase Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			return 0;
		}
		Sleep(5);
	}

//--------------------------------------10 00----------------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x10;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);

//-------------------01 00 00 00 05 00 00 80 00 00 20 00 FF FF 00 00-----------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x01;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x00;
	K_tdata_buf[3] = 0x00;
	K_tdata_buf[4] = 0x05;
	K_tdata_buf[5] = 0x00;
	K_tdata_buf[6] = 0x00;
	K_tdata_buf[7] = 0x80;
	K_tdata_buf[8] = 0x00;
	K_tdata_buf[9] = 0x00;
	K_tdata_buf[10] = 0x20;
	K_tdata_buf[11] = 0x00;
	K_tdata_buf[12] = 0xff;
	K_tdata_buf[13] = 0xff;
	K_tdata_buf[14] = 0x00;
	K_tdata_buf[15] = 0x00;
	len = 16;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[16] == 0xb4){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(20);
	OnReceive((LPVOID)this);
	Sleep(20);
	OnReceive((LPVOID)this);
	if((K_rdata_buf[0] == 0x00 && K_rdata_buf[1] == 0x00 && K_rdata_buf[3] == 0x00) || (K_rdata_buf[0] == 0xfe && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00 && K_rdata_buf[4] == 0x00)){

//	if(K_rdata_buf[0] == 0x00 && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00){
	}
	else{
		err_ifo = "NOT BANK Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);
//-----------------------------------------------------------------------------------//
//-------------------------检查60000~70000是否为空-----------------------------------//
//-----------------------------------------------------------------------------------//
	m_NeedTesterPresent = 0;
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0xb7;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	m_NeedTesterPresent = 0;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
		}
		else{
			err_ifo = "Erase Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			return 0;
		}
		Sleep(5);
	}

//--------------------------------------10 00----------------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x10;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);

//-------------------01 00 00 00 06 00 00 80 00 00 20 00 FF FF 00 00-----------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x01;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x00;
	K_tdata_buf[3] = 0x00;
	K_tdata_buf[4] = 0x06;
	K_tdata_buf[5] = 0x00;
	K_tdata_buf[6] = 0x00;
	K_tdata_buf[7] = 0x80;
	K_tdata_buf[8] = 0x00;
	K_tdata_buf[9] = 0x00;
	K_tdata_buf[10] = 0x20;
	K_tdata_buf[11] = 0x00;
	K_tdata_buf[12] = 0xff;
	K_tdata_buf[13] = 0xff;
	K_tdata_buf[14] = 0x00;
	K_tdata_buf[15] = 0x00;
	len = 16;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[16] == 0xb5){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(20);
	OnReceive((LPVOID)this);
	Sleep(20);
	OnReceive((LPVOID)this);
	if((K_rdata_buf[0] == 0x00 && K_rdata_buf[1] == 0x00 && K_rdata_buf[3] == 0x00) || (K_rdata_buf[0] == 0xfe && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00 && K_rdata_buf[4] == 0x00)){

//	if(K_rdata_buf[0] == 0x00 && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00){
	}
	else{
		err_ifo = "NOT BANK Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);
//-----------------------------------------------------------------------------------//
//-------------------------检查70000~80000是否为空-----------------------------------//
//-----------------------------------------------------------------------------------//
	m_NeedTesterPresent = 0;
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0xb7;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	m_NeedTesterPresent = 0;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
		}
		else{
			err_ifo = "Erase Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			return 0;
		}
		Sleep(5);
	}

//--------------------------------------10 00----------------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x10;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);

//-------------------01 00 00 00 07 00 00 80 00 00 20 00 FF FF 00 00-----------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x01;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x00;
	K_tdata_buf[3] = 0x00;
	K_tdata_buf[4] = 0x07;
	K_tdata_buf[5] = 0x00;
	K_tdata_buf[6] = 0x00;
	K_tdata_buf[7] = 0x80;
	K_tdata_buf[8] = 0x00;
	K_tdata_buf[9] = 0x00;
	K_tdata_buf[10] = 0x20;
	K_tdata_buf[11] = 0x00;
	K_tdata_buf[12] = 0xff;
	K_tdata_buf[13] = 0xff;
	K_tdata_buf[14] = 0x00;
	K_tdata_buf[15] = 0x00;
	len = 16;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[16] == 0xb6){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(20);
	OnReceive((LPVOID)this);
	Sleep(20);
	OnReceive((LPVOID)this);
	if((K_rdata_buf[0] == 0x00 && K_rdata_buf[1] == 0x00 && K_rdata_buf[3] == 0x00) || (K_rdata_buf[0] == 0xfe && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00 && K_rdata_buf[4] == 0x00)){

//	if(K_rdata_buf[0] == 0x00 && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00){
	}
	else{
		err_ifo = "NOT BANK Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//-----------------------------------------------------------------------------------//
//-------------------------检查80000~90000是否为空-----------------------------------//
//-----------------------------------------------------------------------------------//
	m_NeedTesterPresent = 0;
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0xb7;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	m_NeedTesterPresent = 0;
	if(bWriteStat){
		m_CommandNow = ecuReset;
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
		}
		else{
			err_ifo = "Erase Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			return 0;
		}
		Sleep(5);
	}

//--------------------------------------10 00----------------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x10;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);

//-------------------01 00 00 00 08 00 00 80 00 00 20 00 FF FF 00 00-----------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x01;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x00;
	K_tdata_buf[3] = 0x00;
	K_tdata_buf[4] = 0x08;
	K_tdata_buf[5] = 0x00;
	K_tdata_buf[6] = 0x00;
	K_tdata_buf[7] = 0x80;
	K_tdata_buf[8] = 0x00;
	K_tdata_buf[9] = 0x00;
	K_tdata_buf[10] = 0x20;
	K_tdata_buf[11] = 0x00;
	K_tdata_buf[12] = 0xff;
	K_tdata_buf[13] = 0xff;
	K_tdata_buf[14] = 0x00;
	K_tdata_buf[15] = 0x00;
	len = 16;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(5);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[16] == 0xb7){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	Sleep(20);
	OnReceive((LPVOID)this);
	Sleep(20);
	OnReceive((LPVOID)this);
	if((K_rdata_buf[0] == 0x00 && K_rdata_buf[1] == 0x00 && K_rdata_buf[3] == 0x00) || (K_rdata_buf[0] == 0xfe && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00 && K_rdata_buf[4] == 0x00)){

//	if(K_rdata_buf[0] == 0x00 && K_rdata_buf[2] == 0x00 && K_rdata_buf[3] == 0x00){
	}
	else{
		err_ifo = "NOT BANK Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);
	return 1;
}
bool CSTFlash::fun_ST_KFlasher_baundSet(void)
{
	int i = 0;
	BOOL bWriteStat;
	BYTE* PK_rdata_buf;
	DWORD len;
	WORD result=1;
	CString err_ifo;
	CString err_ifo_temp;
	BYTE K_Flasher_baundTest_Data = 0;

//---------------------------------------00-----------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0x00;
	bWriteStat = WriteFile(hCom, &K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	Sleep(5);

//----------------------------------------b7-----------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	len = 1;
	K_tdata_buf[0] = 0xb7;
	bWriteStat = WriteFile(hCom, &K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------06 00---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x06;
	K_tdata_buf[1] = 0x00;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	Sleep(5);

//------------------------------04 00 0c 00 4d 00------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0x04;
	K_tdata_buf[1] = 0x00;
	K_tdata_buf[2] = 0x0c;
	K_tdata_buf[3] = 0x00;
	K_tdata_buf[4] = 0x4d;
	K_tdata_buf[5] = 0x00;
	len = 6;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[6] == 0x63){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}
	Sleep(5);

//----------------------------------------fe ca---------------------------------//
	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	K_tdata_buf[0] = 0xfe;
	K_tdata_buf[1] = 0xca;
	len = 2;
	bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
	OnReceive((LPVOID)this);
	if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[2] == 0xa5){
	}
	else{
		err_ifo = "Error Infomation ";
		for(i=0; i<K_rdata_len; i++){
			err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
			err_ifo += err_ifo_temp;
		}
		MessageBox(err_ifo);
		return 0;
	}

//------------------------------重新设置波特率-----------------------------------------//
	dcb.BaudRate = 57600;							// 波特率
	dcb.ByteSize=8;									// 每个字节有8位
	dcb.Parity=NOPARITY;							// 无奇偶校验位
	dcb.StopBits=ONESTOPBIT;						// 1位停止位
	SetCommState(hCom, &dcb);						// 设置串口

//----------------------------------------data 0~0xff---------------------------------//
	for(K_Flasher_baundTest_Data = 0x00; K_Flasher_baundTest_Data <= 0xFF; K_Flasher_baundTest_Data++){
		memset(K_tdata_buf, '\0', K_MAX_FRAME);
		memset(K_rdata_buf, '\0', K_MAX_FRAME);
		len = 1;
		bWriteStat = WriteFile(hCom, &K_Flasher_baundTest_Data, len, &len, NULL);
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == K_Flasher_baundTest_Data && K_rdata_buf[1] == (0xff - K_Flasher_baundTest_Data)){
			if(K_Flasher_baundTest_Data == 0xFF && K_rdata_buf[2] != 0x5a){	
				err_ifo = "Error Infomation ";
				for(i=0; i<K_rdata_len; i++){
					err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
					err_ifo += err_ifo_temp;
				}
				MessageBox(err_ifo);
				return 0;
			}
			else if(K_Flasher_baundTest_Data == 0xFF && K_rdata_buf[2] == 0x5a){
				return 1;
			}
		}
		else{
			err_ifo = "Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			MessageBox(err_ifo);
			return 0;
		}		
	}
}
bool CSTFlash::fun_ST_KFlasher_TransBin(void)
{
	int k=0;
	BYTE MNROBL;
	DWORD i, j;
	CString str;
	int T_end;
	DWORD memoryAddress, memorySize;
	BOOL bWriteStat;
	BYTE* PK_rdata_buf;
	DWORD len = 0;
	WORD result = 1;	
	CString err_ifo;
	CString err_ifo_temp;
	DWORD R_Data_CRC = 0;
	//	MessageBox(_T("Trans BIN"));
	for(k = 0; k < DataDistributionIndex + 1; k++)
	{
	//5A B7 7B 0A 00 06 01 00 00 01 00 40 02 00 00 54 00 FE CA 
	//-----------------------------------------------------------------------------------//
	//-------------------------刷写指定地址范围------------------------------------------//
	//-----------------------------------------------------------------------------------//
	//------------------------------------- b7 ------------------------------------------//
		Recive_Time = 50;
		m_NeedTesterPresent = 0;
		memset(K_tdata_buf, '\0', K_MAX_FRAME);
		memset(K_rdata_buf, '\0', K_MAX_FRAME);
		len = 1;
		K_tdata_buf[0] = 0xb7;
		bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
		m_NeedTesterPresent = 0;
		if(bWriteStat){
			m_CommandNow = ecuReset;
			Sleep(5);
			OnReceive((LPVOID)this);
			if(K_rdata_buf[0] == 0xb7 && K_rdata_buf[1] == 0x7b){
			}
			else{
				err_ifo = "Trans Data Error Infomation ";
				for(i=0; i<K_rdata_len; i++){
					err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
					err_ifo += err_ifo_temp;
				}
				MessageBox(err_ifo);
				return 0;
			}
			Sleep(5);
		}

	//--------------------------------------0a 00----------------------------------------//
		memset(K_tdata_buf, '\0', K_MAX_FRAME);
		memset(K_rdata_buf, '\0', K_MAX_FRAME);
		K_tdata_buf[0] = 0x0a;
		K_tdata_buf[1] = 0x00;
		len = 2;
		bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
		OnReceive((LPVOID)this);

	//-------------------【06 01】 00 00 01 00 【40 02 00 00】 54 00-----------------//
		BYTE memoryAddress_L, memoryAddress_M, memoryAddress_H;
		BYTE memorySize_L, memorySize_M, memorySize_H;
		memset(K_tdata_buf, '\0', K_MAX_FRAME);
		memset(K_rdata_buf, '\0', K_MAX_FRAME);
		memoryAddress = DataDistribution[k][0];
		memorySize = DataDistribution[k][1];
		if(memoryAddress == 0 && memorySize == 0)
		{
			return 0;
		}
		memoryAddress_L = memoryAddress % 256;
		memoryAddress_M = (memoryAddress >> 8) % 256;
		memoryAddress_H = (memoryAddress >> 16) % 256;
		if(memoryAddress_H == 0){
			memoryAddress_H = 1;
		}
		memorySize_L = memorySize % 256;
		memorySize_M = (memorySize >> 8) % 256;
		memorySize_H = (memorySize >> 16) % 256;
		
		R_Data_CRC = 0;
		K_tdata_buf[0] = 0x06;
		K_tdata_buf[1] = 0x01;
		K_tdata_buf[2] = memoryAddress_L;
		K_tdata_buf[3] = memoryAddress_M;
		K_tdata_buf[4] = memoryAddress_H;
		K_tdata_buf[5] = 0x00;
		K_tdata_buf[6] = memorySize_L;
		K_tdata_buf[7] = memorySize_M;
		K_tdata_buf[8] = memorySize_H;
		K_tdata_buf[9] = 0x00;
		len = 10;
		int crc_I = 0;
		for(crc_I = 0; crc_I < 10; crc_I++)
		{
			R_Data_CRC += K_tdata_buf[crc_I];
		}
		R_Data_CRC += 0x0a;
		bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
		Sleep(5);
		OnReceive((LPVOID)this);
		if(K_rdata_buf[0] == K_tdata_buf[0] && K_rdata_buf[10] == (R_Data_CRC % 0x0100) && K_rdata_buf[11] == (R_Data_CRC / 0x0100)){
		}
		else{
			err_ifo = "Error Infomation ";
			for(i=0; i<K_rdata_len; i++){
				err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
				err_ifo += err_ifo_temp;
			}
			err_ifo_temp = " CRC ";
			err_ifo += err_ifo_temp;
			err_ifo_temp.Format(_T("%04X "),R_Data_CRC);
			err_ifo += err_ifo_temp;

			MessageBox(err_ifo);
			return 0;
		}
		Sleep(5);

	//----------------------------------------fe ca---------------------------------//
		memset(K_tdata_buf, '\0', K_MAX_FRAME);
		memset(K_rdata_buf, '\0', K_MAX_FRAME);
		K_tdata_buf[0] = 0xfe;
		K_tdata_buf[1] = 0xca;
		len = 2;
		bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
		Sleep(5);
		OnReceive((LPVOID)this);
	//-------------------------------------tuans bin---------------------------------//
		Recive_Time = 5;
		memset(K_tdata_buf, '\0', K_MAX_FRAME);
		memset(K_rdata_buf, '\0', K_MAX_FRAME);
		R_Data_CRC = 0;
		i = 0;
		/*
		if(memorySize >= 32){
			for(i = 0; i < memorySize-32; i += 32){
				for(j = 0; j < 32; j++){
					K_tdata_buf[j] = p_bin_hex[memoryAddress + i + j];
				}
				len = 32;
				bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
				Sleep(5);
				OnReceive((LPVOID)this);
				memset(K_tdata_buf, '\0', K_MAX_FRAME);
				memset(K_rdata_buf, '\0', K_MAX_FRAME);
			}
		}
		//*/
		if(memorySize >= 96)
		{
			for(i = 0; i < memorySize-96; i += 96)
			{
				if(memorySize > 3 * 96)
				{
					if(i > (memorySize - 3 * 96))
						Recive_Time = 50;
				}
				else
				{
					Recive_Time = 50;
				}
				for(j = 0; j < 96; j++)
				{
					K_tdata_buf[j] = p_bin_hex[memoryAddress + i + j];
					R_Data_CRC += K_tdata_buf[j];
				}
				len = 96;
				bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
				Sleep(5);
				OnReceive((LPVOID)this);
				memset(K_tdata_buf, '\0', K_MAX_FRAME);
				memset(K_rdata_buf, '\0', K_MAX_FRAME);
			}
		}
		Recive_Time = 50;
		if(i < memorySize)
		{
			int z = 0;
			if ((memorySize - i) % 4 == 0 )
				z = memorySize - i;
			else
				z = ((memorySize - i) / 4 + 1) * 4;
			for(j = 0; j < z; j++)
			{
				K_tdata_buf[j] = p_bin_hex[memoryAddress + i + j];
				R_Data_CRC += K_tdata_buf[j];
			}
			len = z;
			bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
			Sleep(50);
			OnReceive((LPVOID)this);
			if ((len + 3) == K_rdata_len &&
				K_rdata_buf[K_rdata_len - 3] == (R_Data_CRC % 256) &&
				K_rdata_buf[K_rdata_len - 2] == ((R_Data_CRC >> 8) % 256) &&
				K_rdata_buf[K_rdata_len - 1] == 0x5a)
			{
			}
			else
			{
				err_ifo = "Trans Data CRC Error ";
				for(i=0; i<K_rdata_len; i++){
					err_ifo_temp.Format(_T("%02X "), K_rdata_buf[i]);
					err_ifo += err_ifo_temp;
				}
				err_ifo_temp = " CRC ";
				err_ifo += err_ifo_temp;
				err_ifo_temp.Format(_T("%04X "),R_Data_CRC);
				err_ifo += err_ifo_temp;

				MessageBox(err_ifo);
				return 0;
			}
			memset(K_tdata_buf, '\0', K_MAX_FRAME);
			memset(K_rdata_buf, '\0', K_MAX_FRAME);
		}
	}
	return 1;
	
	/*Sleep(5);//将要编程00010000~0240
	memoryAddress = DataDistribution[k][0];
	memorySize = DataDistribution[k][1];

	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	//
		
	for(i = 0; (i + 1)*28 < startchipid_len;i++)
	{
		len = 28;
		bWriteStat = WriteFile(hCom, &K_Flasher_startchipid[i*28], len, &len, NULL);
		Sleep(1);
		OnReceive((LPVOID)this);
		Sleep(1);
	}
	len = startchipid_len - 28 * i;
	bWriteStat = WriteFile(hCom, &K_Flasher_startchipid[28 * (i)], len, &len, NULL);

	OnReceive((LPVOID)this);
	//			

	len = 21;
	bWriteStat = WriteFile(hCom, K_Flasher_startchipid, len, &len, NULL);
	m_NeedTesterPresent = 0;



	do{
	// ************* requestDownload

		i = 0;
		j = 0;
		T_end = 0;

		// 如果已经到了列表的结尾
		if((memoryAddress == 0) && (memorySize == 0))
			break;

		result = fun_requestDownload(memoryAddress, memorySize);
		if((result & 0xFF) != 1){
			fun_setButtonStatus(1);
			return 0;
		}

		MNROBL = result >> 8;  // 得到maxNumberOfBlockLength
		
	// ************* transferData
		while(T_end == 0){
			// 如果还未到结尾
			if((i + MNROBL) <= memorySize){
				result = fun_transferData(i+memoryAddress-DataDistribution[0][0], MNROBL);
				if(result != 1){
					fun_setButtonStatus(1);
					return 0;
				}
				i += MNROBL;
			}
			// 如果已经到了结尾
			else{
				result = fun_transferData(i+memoryAddress-DataDistribution[0][0], (BYTE)(memorySize - i));
				if(result != 1){
					fun_setButtonStatus(1);
					return 0;
				}
				T_end = 1;
			}

			m_progress.SetPos(i+memoryAddress);
			Sleep(20);
		}

	// ************* requestTransferExit
		result = fun_requestTransferExit();
		if(result != 1){
			fun_setButtonStatus(1);
			return 0;
		}

		k++;
	}while(1);
	return false;*/
}
WORD CSTFlash::fun_testerPresent(void)
{
	BOOL bWriteStat;
	BYTE* PK_rdata_buf;
	DWORD len;
	WORD result=1;

	memset(K_tdata_buf, '\0', K_MAX_FRAME);
	memset(K_rdata_buf, '\0', K_MAX_FRAME);
	if(ST_Flasher_Flag == 1)
	{
		len = 1;
		K_tdata_buf[0] = 0x00;
		bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);
		if(bWriteStat){
			m_CommandNow = KSID::TesterPresent;
			OnReceive((LPVOID)this);

			if(K_rdata_buf[0] == 0x00 && K_rdata_buf[1] != 0xa5)
			{
				result = K_rdata_buf[1];
				if(result == 0)
					result = 0xFF;
			}
		}
		else
		{
			result = 0;
		}

	}
	else
	{
		len = KProtocol::ComposeCmd(KSID::TesterPresent, K_tdata_buf, 1);
		bWriteStat = WriteFile(hCom, K_tdata_buf, len, &len, NULL);

		if(bWriteStat)
		{
			m_CommandNow = TesterPresent;
			OnReceive((LPVOID)this);

			if(T_ReceiveSended)
				PK_rdata_buf = K_rdata_buf + 5;
			else
				PK_rdata_buf = K_rdata_buf;

			if(PK_rdata_buf[3] != (KSID::TesterPresent + 0x40))
			{
				result = PK_rdata_buf[5];
				if(result == 0)
					result = 0xFF;
			}
		}
		else
		{
			result = 0;
		}
	}
	
	
	if(result != 1)
		fun_errorHandler(KSID::TesterPresent, result);

	return result;
}
void CSTFlash::OnTimer(UINT_PTR nIDEvent)
{
	WORD result = 1;

	if(nIDEvent == 1)
	{
		if(m_NeedTesterPresent == 1)
		{
			KillTimer(nIDEvent);
			result = fun_testerPresent();
		}
		if(result == 1)
		{
			m_NeedTesterPresent = 1;
			SetTimer(1, 2000, NULL);
		}
		else
		{
			KillTimer(nIDEvent);
		}
	}
	else if(nIDEvent == 2)
	{
		if(m_nStatusECU != 1)
		{
			m_edit_ECUNumber = "";
			B_ECU_Number_ok = 0;
			UpdateData(FALSE);
		}
		KillTimer(nIDEvent);
	}
	CDialog::OnTimer(nIDEvent);
}