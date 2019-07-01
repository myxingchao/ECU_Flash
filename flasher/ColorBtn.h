#pragma once

class CColorBtn : public CButton
{
public:
	CColorBtn();
	virtual ~CColorBtn();

public:
	void SetColor(DWORD color);
	//void DrawXPTheme(LPDRAWITEMSTRUCT lpDrawItemStruct);
protected:
	DWORD	m_dwColor;

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
protected:
	DECLARE_MESSAGE_MAP()
};