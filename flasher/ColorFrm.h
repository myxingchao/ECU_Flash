#pragma once

class CColorFrm : public CStatic
{
public:
	CColorFrm();
	virtual ~CColorFrm();

public:
	void SetColor(DWORD color);
protected:
	DWORD	m_dwColor;

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
protected:
	DECLARE_MESSAGE_MAP()
};