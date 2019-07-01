// Draw.h : header file
#pragma once

///////////////////////////////////////////////////////////////////////////////
typedef DWORD HLSCOLOR;
#define HLS(h,l,s) ((HLSCOLOR)(((BYTE)(h)|((WORD)((BYTE)(l))<<8))|(((DWORD)(BYTE)(s))<<16)))

///////////////////////////////////////////////////////////////////////////////
#define HLS_H(hls) ((BYTE)(hls))
#define HLS_L(hls) ((BYTE)(((WORD)(hls)) >> 8))
#define HLS_S(hls) ((BYTE)((hls)>>16))

#define BMP_BACK     0
#define BMP_NORMAL   1
#define BMP_FOCUS    2
#define BMP_DOWN     3
#define BMP_DISABLE  4
#define BMP_ACTIVE   1
#define BMP_INACTIVE 2
#define BMP_SLIDER   3
#define BMP_CHECK    1
#define BMP_UNCHECK  2

//��ָ����λ�û�����Ӱ
#define SM_CXSHADOW 10

///////////////////////////////////////////////////////////////////////////////
HLSCOLOR RGB2HLS (COLORREF rgb);
COLORREF HLS2RGB (HLSCOLOR hls);
// Performs some modifications on the specified color : luminance and saturation
COLORREF HLS_TRANSFORM(COLORREF rgb, int percent_L, int percent_S);

///////////////////////////////////////////////////////////////////////////////
HBITMAP WINAPI GetScreenBitmap(LPCRECT pRect);

class CClientRect : public CRect
{
public:
	CClientRect(HWND hWnd)
	{
		::GetClientRect(hWnd, this);
	};
	CClientRect(const CWnd* pWnd)
	{
		::GetClientRect(pWnd->GetSafeHwnd(), this);
	};
};
///////////////////////////////////////////////////////////////////////////////
class CWindowRect : public CRect
{
public:
	CWindowRect(HWND hWnd)
	{
		::GetWindowRect(hWnd, this);
	};
	CWindowRect(const CWnd* pWnd)
	{
		::GetWindowRect(pWnd->GetSafeHwnd(), this);
	};
};
///////////////////////////////////////////////////////////////////////////////
class CWindowText : public CString
{
public:
	CWindowText(HWND hWnd)
	{
		CWnd::FromHandle(hWnd)->GetWindowText(*this);
	};
	CWindowText(const CWnd* pWnd)
	{
		pWnd->GetWindowText(*this);
	};
};
//////////////////////////////////////////////////////////////////////////
class CMemDC : public CDC 
{
private:
	CBitmap*    m_bitmap;
	CBitmap*    m_oldBitmap;
	CDC*        m_pDC;
	CRect		m_rcBounds;
public:
	CMemDC(CDC* pDC, const CRect& rcBounds) : CDC()
	{
		CreateCompatibleDC(pDC);
		m_bitmap = new CBitmap;
		m_bitmap->CreateCompatibleBitmap(pDC, rcBounds.right, rcBounds.bottom);
		m_oldBitmap = SelectObject(m_bitmap);
		m_pDC = pDC;
		m_rcBounds = rcBounds;
	}
	~CMemDC() 
	{
		m_pDC->BitBlt(m_rcBounds.left, m_rcBounds.top, m_rcBounds.Width(), m_rcBounds.Height(), 
					this, m_rcBounds.left, m_rcBounds.top, SRCCOPY);
		SelectObject(m_oldBitmap);
		if (m_bitmap != NULL) 
			delete m_bitmap;
	}
	CMemDC* operator->() 
	{
		return this;
	}
};
///////////////////////////////////////////////////////////////////////////////
class CBufferDC : public CDC
{
	HDC     m_hDestDC;
	CBitmap m_bitmap;     // Bitmap in Memory DC
	CRect   m_rect;
	HGDIOBJ m_hOldBitmap; // Previous Bitmap

public:
	CBufferDC(HDC hDestDC, const CRect& rcPaint = CRect(0,0,0,0));
   ~CBufferDC();
};
///////////////////////////////////////////////////////////////////////////////
class CPenDC
{
protected:
	CPen m_pen;
	HDC m_hDC;
	HPEN m_hOldPen;

public:
	CPenDC(HDC hDC, COLORREF crColor = CLR_NONE);
   ~CPenDC();

	void Color(COLORREF crColor);
	COLORREF Color () const;
};
///////////////////////////////////////////////////////////////////////////////
class CBrushDC
{
protected:
	CBrush m_brush;
	HDC m_hDC;
	HBRUSH m_hOldBrush;

public:
	CBrushDC(HDC hDC, COLORREF crColor = CLR_NONE);
   ~CBrushDC();

	void Color(COLORREF crColor);
	COLORREF Color () const;
};
///////////////////////////////////////////////////////////////////////////////
class CFontDC
{
protected:
	HFONT m_hFont;
	HDC m_hDC;
	HFONT m_hDefFont;
	COLORREF m_crTextOld;

public:
	CFontDC (HDC hDC, LPCTSTR sFaceName, COLORREF crText = CLR_DEFAULT);
	CFontDC (HDC hDC, BYTE nStockFont, COLORREF crText = CLR_DEFAULT);
	CFontDC (HDC hDC, HFONT hFont, COLORREF crText = CLR_DEFAULT);
   ~CFontDC ();

	const CFontDC& operator = (LPCTSTR sFaceName);
	const CFontDC& operator = (BYTE nStockFont);
	const CFontDC& operator = (HFONT hFont);
	const CFontDC& operator = (COLORREF crText);
	operator LPCTSTR ();
	operator COLORREF ();
};
///////////////////////////////////////////////////////////////////////////////
class CBoldDC
{
protected:
	CFont m_fontBold;
	HDC m_hDC;
	HFONT m_hDefFont;

public:
	CBoldDC (HDC hDC, bool bBold);
   ~CBoldDC ();
};
//////////////////////////////////////////////////////////////////////////
class CPHBitmap : public CBitmap  
{
public:
	CPHBitmap();
	virtual ~CPHBitmap();

	BOOL LoadImage(LPCTSTR szImagePath, COLORREF crBack = 0);
	BOOL LoadImage(UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst = NULL, COLORREF crBack = 0); 
	BOOL LoadBitmap (UINT nIDResource)
	{
		return CBitmap::LoadBitmap(nIDResource);
	}

	// helpers
	static BOOL GetResource(LPCTSTR lpName, LPCTSTR lpType, HMODULE hInst, void* pResource, int& nBufSize);
	static IPicture* LoadFromBuffer(BYTE* pBuff, int nSize);


	BOOL Draw(CDC *pDC, LPRECT prt);
	//draw sub bmp to special point
	BOOL Draw(CDC *pDC, int x, int y, LPRECT sr);
	BOOL Draw(CDC *pDC, int x, int y, LPRECT sr, COLORREF colTrans, BOOL bTrans);

	BOOL StretchDraw(CDC *pDC, LPRECT tr, LPRECT sr);
	BOOL StretchDraw(CDC *pDC, LPRECT r);

	int Width() { return GetWidth(); }
	int Height(){ return GetHeight();}
	int GetWidth() 
	{
		if (!GetSafeHandle())
			return 0;
		BITMAP bm;
		memset( &bm, 0, sizeof(bm) );
		GetBitmap(&bm);
		return bm.bmWidth;
	}
	int GetHeight()
	{
		if (!GetSafeHandle())
			return 0;
		BITMAP bm;
		memset( &bm, 0, sizeof(bm) );
		GetBitmap(&bm);
		return bm.bmHeight;
	}

	BOOL Attach(HBITMAP hbmp) { return CBitmap::Attach( hbmp );}
	BOOL LoadBitmap(LPCTSTR szFilename) 
	{ 
		ASSERT(szFilename);
		DeleteObject();
		return LoadImage(szFilename);
	}

	BOOL DrawTransparent(CDC * pDC, int x, int y, COLORREF crColour);    
	HRGN CreateRgnFromFile(COLORREF color);

	static void MKTransparentBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, 
						   int nHeight, HBITMAP hBitmap, int nXSrc, int nYSrc,
						   COLORREF colorTransparent, HPALETTE hPal);

protected:
	BOOL Attach(IPicture* pPicture, COLORREF crBack);

};
//////////////////////////////////////////////////////////////////////////
class CPHDraw
{
public:
	typedef enum _tagDrawMode
	{
		DrawModeFill = 0,
		DrawModeWidthStretch = 1,
		DrawModeHeightStretch = 2,
		DrawModeAllStretch = 3,
		DrawMode3D = 4,
		DrawModeState = 5,
		DrawModeHeightCenter3D = 6,
		DrawModeHeightStretch3D = 7
	} DrawMode;

	static BOOL DrawBitmap(CDC* pDC, LPCRECT lpRect, CBitmap* pBitmap, UINT Mode = DrawModeHeightStretch);
	static BOOL DrawVerticalText(CDC* pDC, LPCRECT lpRect, LPCTSTR lpText, int length, UINT format, LOGFONT *lf);
	static HRGN BitmapToRegion (HBITMAP hBmp, COLORREF cTransparentColor = 0, COLORREF cTolerance = 0x101010);
	static BOOL FillGradient(CDC *pDC, CRect rect, COLORREF colorStart, COLORREF colorFinish, BOOL bHorz = TRUE);
	static void DrawShadow(HDC hDCIn, HDC hDCOut, RECT& rc);
};