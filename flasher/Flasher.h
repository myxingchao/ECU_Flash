// Flasher.h 
#pragma once
#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif
#include "resource.h"

class CFlasherApp : public CWinApp
{
public:
	CFlasherApp();

	void LoadIni(LPCSTR filename);
	void SaveIni(LPCSTR filename);
public:
	virtual BOOL InitInstance();
	DECLARE_MESSAGE_MAP()
};
extern CFlasherApp theApp;

/*	TODO:
刷写时如果增加延时，会导致总刷写时间增加1分钟，不延时的话又会占用100%CPU资源，
串口通信应该从非阻塞通信改为阻塞式通信，以减少CPU资源占用。
另外更新进度条应从同步改为异步，不应占用刷写线程的时间。
*/