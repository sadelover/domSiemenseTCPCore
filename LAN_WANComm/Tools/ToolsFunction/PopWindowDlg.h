#ifndef __POP_WINDOW_H__
#define __POP_WINDOW_H__


#define ID_TIMER_POP_WINDOW		10
#define ID_TIMER_CLOSE_WINDOW	11
#define ID_TIMER_DISPLAY_DELAY	12
#define WIN_WIDTH	            181
#define WIN_HEIGHT	            116
#define CLOSE_BITMAP_WIDTH      41
#define CLOSE_BITMAP_HEIGHT     25
#define CLOSE_BITMAP_PATH       TEXT("Images\\popup_close.bmp")

#include "ToolsFunction.h"


class CPopWindow : public CWnd
{
public:
	CPopWindow();                                 // default constructor
	virtual ~CPopWindow();                        // default destructor

	void ShowPopWindow();                         // show the pop window
	void ClosePopWindow();                        // close the pop window

protected:
	//{{AFX_MSG(CPopWindow)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void CreatePopWindow();                       // create the pop window

private:
	inline void SetText(LPCTSTR lpszTxt);         // set the text
	inline void SetCaption(LPCTSTR lpszCaption);  // set the caption

	inline CRect GetRect() const;                       // get the text rect
	inline void SetRect(CRect rect);                    // set the text rect

	inline COLORREF GetTxtColor() const;                // get the text color
	inline void SetTxtColor(COLORREF rgbColorRef);      // set the text color

public:
	CWnd* m_pMainWnd;

private:
	CToolsFunction m_toolsFunction;

	LPTSTR m_strText;
	LPTSTR m_strCaption;
	CRect m_rect;
	COLORREF m_rgbColorRef;
	bool m_bUnderline;

	HBITMAP m_hBitmap;
	BOOL m_bFlag;

	bool m_bPopWindowExist;                             // verify whether the pop window exists?
};

#endif  // __POP_WINDOW_H__