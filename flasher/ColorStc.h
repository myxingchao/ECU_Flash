#pragma once

class CColorStc : public CStatic
{
public:
	CColorStc();
	virtual ~CColorStc();

public:
	void SetColor(DWORD color);
protected:
	DWORD	m_dwColor;
	BOOL	m_bOver;

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
protected:
	DECLARE_MESSAGE_MAP()
};