//
#pragma once
#include "Resource.h"

class CUserID : public CDialog
{
public:
	CUserID(CWnd* pParent = NULL);
	virtual ~CUserID();
public:
	void	Set(CHAR* user);
	CHAR*	Get();

protected:
	enum { IDD = IDD_USERID };
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

protected:
	CHAR m_szUser[_MAX_PATH];

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOK();
};
