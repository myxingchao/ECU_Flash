#pragma once
#include "ServerThd.h"
#include "ClientThd.h"

class CChild;
class CMain : public CDialog
{
public:
	CMain(CWnd* pParent = NULL);
	virtual ~CMain();

protected:
	enum { IDD = IDD_MAIN };
	virtual BOOL OnInitDialog();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

protected:
	static	UINT OpenComThd(LPVOID param);
	void	SetControlInfo(CWnd* pwnd, LONG anchor);

protected:
	CServerThd*	m_pServer;
	CClientThd*	m_pClient;
	CChild*		m_pChild[DEF_CHILD_COUNT];
	HICON		m_hIcon;
	LONG		m_pAnchor[16][4];//control handle, anchor type, original width, original height.
	SIZE		m_szOld, m_szMin, m_szDelta;
	LONG		m_nProcRun;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnBnClickedAbout();
	afx_msg void OnBnClickedInit();
	afx_msg void OnBnClickedUserID();
	afx_msg void OnBnClickedConfig();
	afx_msg void OnBnClickedClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HCURSOR OnQueryDragIcon();
};
