#pragma once
#include "Picture.h"

class CWait : public CDialog
{
public:
	CWait(CWnd* pParent = NULL);
	virtual ~CWait();

	enum { IDD = IDD_WAITTING };
	CPicture	m_arPicture;

public:
	void	SetTargetProc(LPVOID process, LPVOID param);
	LPVOID	GetParam();
	DWORD	GetRetValue();

protected:
	AFX_THREADPROC	m_pProcess;
	LPVOID			m_pParam;
	CWinThread*		m_pWorkThread;
	DWORD			m_dwRet;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClose();
};
