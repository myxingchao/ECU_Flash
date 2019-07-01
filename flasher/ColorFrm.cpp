//ColorFrm.cpp
#include "stdafx.h"
#include "ColorFrm.h"
#include "Rgb2Hsl.h"

CColorFrm::CColorFrm()
{
}
CColorFrm::~CColorFrm()
{
}
BEGIN_MESSAGE_MAP(CColorFrm, CStatic)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()
//
void CColorFrm::SetColor(DWORD color)
{
	m_dwColor = color;
}
void CColorFrm::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	VERIFY(lpDIS->CtlType == ODT_STATIC);
	//CDC dc;
	//dc.Attach(lpDrawItemStruct->hDC);
	//dc.Detach();
	CDC* pdc = CDC::FromHandle(lpDIS->hDC);

	//���Ʊ�����
	//CBrush brush;
	//if (lpDrawItemStruct->itemState & ODS_SELECTED)//����״̬
	//	pdc->FillRect(&(lpDrawItemStruct->rcItem), &CBrush(m_dwColor));
	//else//δ����״̬
	//	pdc->FillRect(&(lpDrawItemStruct->rcItem), &CBrush(m_dwColor + RGB(40, 40, 40)));

	/*/���ñ�����ˢ��������ɫ��
	CPen	pen;
	CBrush	brush;
	if (::IsWindowEnabled(::GetParent(m_hWnd)) == FALSE)
	{
		pen.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));
		brush.CreateSolidBrush(::GetSysColor(COLOR_GRAYTEXT));//COLOR_3DFACE
		pdc->SetTextColor(::GetSysColor(COLOR_BTNFACE));//COLOR_BTNTEXT
	}
	else if ((lpDIS->itemState & ODS_DISABLED) != 0)
	{
		pen.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));
		brush.CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));//COLOR_3DFACE
		pdc->SetTextColor(::GetSysColor(COLOR_MENUTEXT));//COLOR_BTNTEXT
	}
	else if ((lpDIS->itemState & ODS_SELECTED) == 0)//���á�δ������δѡ��״̬
	{
		pen.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));
		brush.CreateSolidBrush(HLS_TRANSFORM(m_dwColor, -10, 0));//COLOR_3DFACE
		pdc->SetTextColor(HLS_TRANSFORM(m_dwColor, 70, 0));
	}
	else//ѡ�С�����״̬(û�õ�)
	{
		pen.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_HIGHLIGHT));
		if (lpDIS->itemState & ODS_SELECTED)//ѡ��
			brush.CreateSolidBrush(HLS_TRANSFORM(m_dwColor, +50, -50));//COLOR_HIGHLIGHT
		else//����
			brush.CreateSolidBrush(HLS_TRANSFORM(m_dwColor, +70, -70));//COLOR_HIGHLIGHT
		pdc->SetTextColor(HLS_TRANSFORM(m_dwColor, +80, -66));
	}
	CBrush*	oldb = pdc->SelectObject(&brush);
	CPen*	oldp = pdc->SelectObject(&pen);*/

	//���Ʊ߿�
	CRect rtc(lpDIS->rcItem);
	rtc.DeflateRect(1, 1);
	pdc->Rectangle(rtc);

	//ѡ��״̬
	//if ((lpDrawItemStruct->itemState & ODS_SELECTED) && (lpDrawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	//	pdc->FrameRect(&(lpDrawItemStruct->rcItem), &CBrush(RGB(255, 255, 100)));
	//	//COLORREF fc = RGB(255-GetRValue(m_UpColor), 255-GetGValue(m_UpColor), 255-GetBValue(m_UpColor));
	//ȡ��ѡ��
	//if (!(lpDrawItemStruct->itemState & ODS_SELECTED) && (lpDrawItemStruct->itemAction & ODA_SELECT))
	//	pdc->FrameRect(&lpDrawItemStruct->rcItem, &CBrush(255 - m_dwColor));
}