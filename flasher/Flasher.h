// Flasher.h 
#pragma once
#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
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
ˢдʱ���������ʱ���ᵼ����ˢдʱ������1���ӣ�����ʱ�Ļ��ֻ�ռ��100%CPU��Դ��
����ͨ��Ӧ�ôӷ�����ͨ�Ÿ�Ϊ����ʽͨ�ţ��Լ���CPU��Դռ�á�
������½�����Ӧ��ͬ����Ϊ�첽����Ӧռ��ˢд�̵߳�ʱ�䡣
*/