//ColorBtn.cpp
#include "stdafx.h"
#include "ColorBtn.h"
#include "Rgb2Hsl.h"

CColorBtn::CColorBtn()
{
}
CColorBtn::~CColorBtn()
{
}
BEGIN_MESSAGE_MAP(CColorBtn, CButton)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()
//
void CColorBtn::SetColor(DWORD color)
{
	m_dwColor = color;
}
void CColorBtn::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	VERIFY(lpDIS->CtlType == ODT_BUTTON);
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

	//���ñ�����ˢ��������ɫ��
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
	CPen*	oldp = pdc->SelectObject(&pen);
	pdc->Rectangle(&lpDIS->rcItem);

	/*/���Ʊ߿�
	CRect rtc(lpDIS->rcItem);
	if (lpDIS->itemState & ODS_FOCUS)
		rtc.DeflateRect(1, 1);

	//�������ߡ�
	if ((lpDIS->itemState & ODS_FOCUS) != 0 && (lpDIS->itemState & ODS_DISABLED) != 0)
	{
		rtc.DeflateRect(3, 3);
		pdc->DrawFocusRect(rtc);
	}*/
	pdc->SelectObject(oldp);
	pdc->SelectObject(oldb);

	//ѡ��״̬
	//if ((lpDrawItemStruct->itemState & ODS_SELECTED) && (lpDrawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	//	pdc->FrameRect(&(lpDrawItemStruct->rcItem), &CBrush(RGB(255, 255, 100)));
	//	//COLORREF fc = RGB(255-GetRValue(m_UpColor), 255-GetGValue(m_UpColor), 255-GetBValue(m_UpColor));
	//ȡ��ѡ��
	//if (!(lpDrawItemStruct->itemState & ODS_SELECTED) && (lpDrawItemStruct->itemAction & ODA_SELECT))
	//	pdc->FrameRect(&lpDrawItemStruct->rcItem, &CBrush(255 - m_dwColor));

	//�������塣
	/*HFONT hf = ::CreateFont(
		50,							// nHeight
		0,							// nWidth
		0,							// nEscapement
		0,							// nOrientation
		FW_MEDIUM,					// nWeight
		FALSE,						// bItalic
		FALSE,						// bUnderline
		0,							// cStrikeOut
		DEFAULT_CHARSET,			// nCharSet
		OUT_DEFAULT_PRECIS,			// nOutPrecision
		CLIP_DEFAULT_PRECIS,		// nClipPrecision
		ANTIALIASED_QUALITY,		// nQuality
		DEFAULT_PITCH | FF_SWISS,	// nPitchAndFamily
		"MS Gothic");				// lpszFacename
	pdc->SelectObject(hf);*/
	LOGFONT	logf;
	GetFont()->GetLogFont(&logf);
	logf.lfHeight = 50;
	CFont newf, *oldf = NULL;
	if (newf.CreateFontIndirect(&logf) == TRUE)
		oldf = pdc->SelectObject(&newf);

	//�������֡�
	CString	txt;
	GetWindowText(txt);
	pdc->SetBkMode(TRANSPARENT);
	pdc->DrawText(txt, &lpDIS->rcItem, DT_SINGLELINE |DT_CENTER |DT_VCENTER);

	/*/�ָ����塣
	if (oldf != NULL)
	{
		pdc->SelectObject(oldf);
		newf.DeleteObject();
	}*/
}
/*void CColorBtn::DrawXPTheme(LPDRAWITEMSTRUCT lpDIS) 
{
	CDC* pdc = CDC::FromHandle(lpDIS->hDC);
}*/
/*void CColorBtn::DrawXPTheme(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect rcClient = lpDrawItemStruct->rcItem; 
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);

	char WIN_PATH[1024];
	::GetWindowsDirectory(WIN_PATH, 1023);
	strcpy(WIN_PATH + strlen(WIN_PATH), "\\system32\\uxtheme.dll");
	HMODULE hm = LoadLibrary(WIN_PATH);

	typedef BOOL (DRAW_BKGND) (HTHEME, HDC, int, int, const RECT*, const RECT*);
	typedef void (CLOSE_THEME) (HTHEME);
	typedef HTHEME (OPEN_THEME) (HWND, LPCWSTR);
	OPEN_THEME*		PFOpenTheme		= (OPEN_THEME*)		::GetProcAddress(hm, "OpenThemeData");
	DRAW_BKGND*		PFDrawBkgnd		= (DRAW_BKGND*)		::GetProcAddress(hm, "DrawThemeBackground");
	CLOSE_THEME*	PFCloseTheme	= (CLOSE_THEME*)	::GetProcAddress(hm, "CloseThemeData");

	if (m_bOver)
	{
		if (PFOpenTheme == NULL || PFDrawBkgnd == NULL || PFCloseTheme == NULL)
		{
			dc.DrawFrameControl(&rcClient, DFC_CAPTION, DFCS_CAPTIONCLOSE);
			dc.DrawFocusRect(&rcClient);
		}
		else
		{
			HTHEME ht = PFOpenTheme(this->GetSafeHwnd(), L"WINDOW");
			PFDrawBkgnd(ht, lpDrawItemStruct->hDC, WP_CLOSEBUTTON, CBS_HOT, &lpDrawItemStruct->rcItem, NULL);//&lpDrawItemStruct->rcItem
			PFCloseTheme(ht);
			dc.DrawFocusRect(&rcClient);
		}
	}
	else
	{
		if (PFOpenTheme == NULL || PFDrawBkgnd == NULL || PFCloseTheme == NULL)
		{
			dc.DrawFrameControl(&rcClient,DFC_CAPTION,DFCS_CAPTIONCLOSE);
		}
		else
		{
			HTHEME ht = PFOpenTheme(this->GetSafeHwnd(), L"WINDOW");
			PFDrawBkgnd(ht, lpDrawItemStruct->hDC, WP_CLOSEBUTTON, CBS_NORMAL, &lpDrawItemStruct->rcItem, NULL);//&lpDrawItemStruct->rcItem
			PFCloseTheme(ht);
		}
	}
	::FreeLibrary(hm);
	dc.Detach();
}*/
