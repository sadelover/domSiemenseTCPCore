// PopWindowDlg.cpp the implement of PopWindowDlg.cpp

#include "stdafx.h"
#include "PopWindowDlg.h"

#pragma warning(disable:4996)

extern bool g_bAutoUpdateClose;


// Default constructor.
CPopWindow::CPopWindow()
: m_hBitmap(NULL),
  m_pMainWnd(NULL)
{
	// Initialize the text.
	m_strText = TEXT("有文件需要更新,\r\n")
		        TEXT("需要立即升级,请点击...\r\n");

	// Set the text rect.
	CRect rect;
	rect.left = 30;
	rect.top = 50;
	rect.right = 180;
	rect.bottom = 80;
	SetRect(rect);

	// Set the text color as the black color.
	SetTxtColor(RGB(0, 0, 0));

	// Whether the text has underline.
	m_bUnderline = false;

	// Get the path of the close buttom bitmap resource.
	LPTSTR lpszCurrentAppPath = m_toolsFunction.GetCurretAppPath();
	TCHAR tszCurrentAppPath[MAX_PATH] = {0};
	_tcscpy_s(tszCurrentAppPath, MAX_PATH, lpszCurrentAppPath);
	_tcscat_s(tszCurrentAppPath, MAX_PATH, CLOSE_BITMAP_PATH);
	// Load the close buttom bitmap resource.
	m_hBitmap = (HBITMAP)::LoadImage(NULL,  
		tszCurrentAppPath,
		IMAGE_BITMAP,
		0,
		0,  
		LR_LOADFROMFILE);

	if (m_hBitmap == NULL)
	{
		TRACE("CBitmap::LoadBitmap() failed, GetLastError() return: %d.  \r\n", GetLastError());
		// Free the bitmap resource.
		DeleteObject(m_hBitmap); 
	}
	m_bFlag = false;

	// Create the pop window.
	//CreatePopWindow();
}

// Default destructor.
CPopWindow::~CPopWindow()
{
	// Free the bitmap resource.
	if (m_hBitmap != NULL)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}

	if (m_bPopWindowExist)
	{
		// Non-update files completed, then write to the register.
		m_toolsFunction.RegWriteValue(HKEY_CURRENT_USER, REG_VALUE_PATH, "HasUpdateFiles", "yes");
	}
}

BEGIN_MESSAGE_MAP(CPopWindow, CWnd)
	//{{AFX_MSG_MAP(CPopWindow)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// Response the WM_PAINT message.
void CPopWindow::OnPaint() 
{
	CPaintDC dc(this);  // device context for painting
	CDC dcMemory;
	dcMemory.CreateCompatibleDC(&dc);

	// Draw the black rectangle border.
	CPen penBlack;
	penBlack.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* penOld = dc.SelectObject(&penBlack);
	CRect rectClient;
	GetClientRect(rectClient);
	dc.Rectangle(rectClient);
	dc.SelectObject(penOld);
	DeleteObject(penBlack);

	// Draw the text.
	CFont font;
	LOGFONT logFont;
	memset(&logFont, 0, sizeof(LOGFONT));
	logFont.lfHeight = 12;
	_tcscpy(logFont.lfFaceName, TEXT("宋体"));
	if (m_bUnderline)
	{
		logFont.lfUnderline = TRUE;
	}
	else
	{
		logFont.lfUnderline = FALSE;
	}
	// Create the indirect font for the text.
	font.CreateFontIndirect(&logFont);
	CFont* fontOld = dc.SelectObject(&font);

	// Get and set the text color.
	COLORREF rgb = GetTxtColor();
	dc.SetTextColor(rgb);
	// Set the background mode.
	dc.SetBkMode(TRANSPARENT);

	// Draw the text.
	CRect rect;
	rect = GetRect();
	dc.DrawText(m_strText, -1, &rect, DT_CENTER | DT_VCENTER);
	dc.SelectObject(fontOld);

	// Draw the close button bitmap.
	if (m_hBitmap != NULL)
	{
		dcMemory.SelectObject(m_hBitmap);
		dc.StretchBlt(rectClient.right - CLOSE_BITMAP_WIDTH,
			rectClient.top,
			CLOSE_BITMAP_WIDTH, 
			CLOSE_BITMAP_HEIGHT,    
			&dcMemory, 
			0,
			0,
			CLOSE_BITMAP_WIDTH,    
			CLOSE_BITMAP_HEIGHT,
			SRCCOPY);
	}

	DeleteObject(dcMemory);
}

// Response the WM_TIMER message.
void CPopWindow::OnTimer(UINT nIDEvent) 
{	
	// Get the screen position.
	static int nHeight = 0;
	int cy = GetSystemMetrics(SM_CYSCREEN);
	int cx = GetSystemMetrics(SM_CXSCREEN);

	// Get the client work area.
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
	int y = rect.bottom - rect.top;
	int x = rect.right - rect.left;
	x = x - WIN_WIDTH;

	switch(nIDEvent)
	{
	case ID_TIMER_POP_WINDOW:
		{
			if (nHeight <= WIN_HEIGHT)
			{
				nHeight += 3;
				MoveWindow(x, y-nHeight, WIN_WIDTH, WIN_HEIGHT);
				Invalidate(FALSE);
			}
			else
			{
				KillTimer(ID_TIMER_POP_WINDOW);
			}
		}
		break;
	case ID_TIMER_CLOSE_WINDOW:
		{
			if (nHeight >= 0)
			{
				nHeight -= 3;
				MoveWindow(x, y-nHeight, WIN_WIDTH, nHeight);
				Invalidate(FALSE);
			}
			else
			{
				KillTimer(ID_TIMER_CLOSE_WINDOW);
				SendMessage(WM_CLOSE);
			}
		}
		break;
	case ID_TIMER_DISPLAY_DELAY:
		{
			KillTimer(ID_TIMER_DISPLAY_DELAY);
			// Active the pop window closed timer.
			SetTimer(ID_TIMER_CLOSE_WINDOW, 20, NULL);
		}
		break;
	default:
		break;
	}

	CWnd::OnTimer(nIDEvent);
}

// Response the WM_CREATE message.
int CPopWindow::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	return 0;
}

// Response the WM_LBUTTONDOWN message.
void CPopWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rect = GetRect();
	// The point is within the text region.
	if (point.x > rect.left && point.x < rect.right && point.y > rect.top && point.y < rect.bottom)
	{
		// Underline text.
		m_bUnderline = true;
		// Set the text color as the gray color.
		SetTxtColor(RGB(128, 128, 128));
		Invalidate(TRUE);

		int nResult = ::MessageBox(NULL, TEXT("确定要进行升级吗?"), TEXT("提示"), MB_YESNO | MB_ICONINFORMATION | MB_TOPMOST);
		if (nResult == IDYES)
		{
			m_bPopWindowExist = false;
			g_bAutoUpdateClose = true;

			// Update files completed, then write to the register.
			m_toolsFunction.RegWriteValue(HKEY_CURRENT_USER, REG_VALUE_PATH, "HasUpdateFiles", "no");

			// Active the pop window closed timer.
			SetTimer(ID_TIMER_CLOSE_WINDOW, 0, NULL);
		}
	}

	CRect clientRect;
	GetClientRect(clientRect);
	// The point is within the close button region.
	if (point.x > clientRect.right - CLOSE_BITMAP_WIDTH && point.x < clientRect.right && point.y > clientRect.top && point.y < clientRect.top + CLOSE_BITMAP_HEIGHT)
	{
		m_bPopWindowExist = false;

		// Non-update files completed, then write to the register.
		m_toolsFunction.RegWriteValue(HKEY_CURRENT_USER, REG_VALUE_PATH, "HasUpdateFiles", "yes");

		// Active the pop window closed timer.
		SetTimer(ID_TIMER_CLOSE_WINDOW, 0, NULL);
	}
}

// Response the WM_MOUSEMOVE message.
void CPopWindow::OnMouseMove(UINT nFlags, CPoint point) 
{
	//// Firstly, modify the text color when the mouse is in the text region.

	// Get the text rectangle.
	CRect rect = GetRect();
	// The point is within the text region.
	if (point.x > rect.left && point.x < rect.right && point.y > rect.top && point.y < rect.bottom)
	{
		// Non-underline text.
		m_bUnderline = true;
		// Set the text color as the blue color.
		SetTxtColor(RGB(0, 0, 255));
		Invalidate(TRUE);
	}
	else
	{
		m_bUnderline = false;
		// Set the text color as the black color.
		SetTxtColor(RGB(0, 0, 0));
		Invalidate(TRUE);
	}

	//// Secondly, modify the cursor style when the mouse is in the text or the close button region.

	// Get the client rectangle.
	CRect clientRect;
	GetClientRect(clientRect);
	// The point is within the close button region.
	if (point.x > rect.left && point.x < rect.right && point.y > rect.top && point.y < rect.bottom
	 || point.x > clientRect.right - CLOSE_BITMAP_WIDTH && point.x < clientRect.right && point.y > clientRect.top && point.y < clientRect.top + CLOSE_BITMAP_HEIGHT)
	{
		// Show the hand cursor.
		HCURSOR hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND));
		SetCursor(hCursor);
	}
	else
	{
		// Show the arrow cursor.
		HCURSOR hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
		SetCursor(hCursor);
	}

	CWnd::OnMouseMove(nFlags, point);
}

// Response the WM_KILLFOCUS message.
void CPopWindow::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);

	if (m_bFlag)
	{
		// Active the pop window display delay timer.
		SetTimer(ID_TIMER_DISPLAY_DELAY, 3000, NULL);
	}	
}

// Show the pop window.
void CPopWindow::ShowPopWindow()
{
	SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetActiveWindow();
	ShowWindow(SW_SHOWNORMAL);
	UpdateWindow();
}

// Close the pop window.
void CPopWindow::ClosePopWindow()
{
	// Free the bitmap resource.
	if (m_hBitmap != NULL)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}

 	if (m_bPopWindowExist)
 	{
 		// Non-update files completed, then write to the register.
 		m_toolsFunction.RegWriteValue(HKEY_CURRENT_USER, REG_VALUE_PATH, "HasUpdateFiles", "yes");
 	}
}

// Create the message active window.
void CPopWindow::CreatePopWindow()
{
	CreateEx(/*WS_EX_TRANSPARENT | */WS_EX_TOPMOST,
		AfxRegisterWndClass(0,
			::LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_ARROW)),(HBRUSH)GetStockObject(WHITE_BRUSH),NULL),
		TEXT(""),
		WS_POPUP,
		0,
		0,
		0,
		0,
		NULL,
		NULL,
		NULL);

	m_bPopWindowExist = true;

	// Active the pop window timer.
	SetTimer(ID_TIMER_POP_WINDOW, 0, NULL);
}

// Set the message text.
void CPopWindow::SetText(LPCTSTR lpszMsg)
{
	if (_tcscmp(lpszMsg, TEXT("")) != 0)
	{
		lstrcpy(m_strText, lpszMsg);
	}
}

// Set the message caption.
void CPopWindow::SetCaption(LPCTSTR lpszCaption)
{
	if (_tcscmp(lpszCaption, TEXT("")) != 0)
	{
		lstrcpy(m_strCaption, lpszCaption);
	}
}

// Get the message text rect.
CRect CPopWindow::GetRect() const
{
	return m_rect;
}

// Set the message text rect.
void CPopWindow::SetRect(CRect rect)
{
	memcpy(m_rect, rect, sizeof(rect));
}

// Get the text color.
COLORREF CPopWindow::GetTxtColor() const
{
	return m_rgbColorRef;
}

// Set the text color.
void CPopWindow::SetTxtColor(COLORREF rgbColorRef)
{
	memcpy(&m_rgbColorRef, &rgbColorRef, sizeof(rgbColorRef));
}