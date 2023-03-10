// wintip.c (part of MinTTY)
// Copyright 2008 Andy Koppe
// Adapted from code from PuTTY-0.60 by Simon Tatham and team.
// Licensed under the terms of the GNU General Public License v3 or later.

#include "winpriv.h"

static ATOM tip_class;
static HFONT tip_font;
static COLORREF tip_bg;
static COLORREF tip_text;
static HWND tip_wnd;
static bool tip_enabled;

static LRESULT CALLBACK
tip_proc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  switch (nMsg) {
    when WM_DESTROY:
      DeleteObject(tip_font);
      tip_font = null;
    when WM_ERASEBKGND: return true;
    when WM_NCHITTEST:  return HTTRANSPARENT;
    when WM_SETTEXT: {
      LPCTSTR str = (LPCTSTR) lParam;
      HDC dc = CreateCompatibleDC(null);
      SelectObject(dc, tip_font);
      SIZE sz;
      GetTextExtentPoint32(dc, str, strlen(str), &sz);
      SetWindowPos(hWnd, null, 0, 0, sz.cx + 6, sz.cy + 6,
                   SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
      InvalidateRect(hWnd, null, false);
      DeleteDC(dc);
    }
    when WM_PAINT: {
      PAINTSTRUCT ps;
      HDC dc = BeginPaint(hWnd, &ps);

      SelectObject(dc, tip_font);
      SelectObject(dc, GetStockObject(BLACK_PEN));

      HBRUSH hbr = CreateSolidBrush(tip_bg);
      HGDIOBJ holdbr = SelectObject(dc, hbr);

      RECT cr;
      GetClientRect(hWnd, &cr);
      Rectangle(dc, cr.left, cr.top, cr.right, cr.bottom);

      int wtlen = GetWindowTextLength(hWnd);
      LPTSTR wt = (LPTSTR) newn(TCHAR, wtlen + 1);
      GetWindowText(hWnd, wt, wtlen + 1);

      SetTextColor(dc, tip_text);
      SetBkColor(dc, tip_bg);

      TextOut(dc, cr.left + 3, cr.top + 3, wt, wtlen);

      free(wt);

      SelectObject(dc, holdbr);
      DeleteObject(hbr);

      EndPaint(hWnd, &ps);
      return 0;
    }
  }
  return DefWindowProc(hWnd, nMsg, wParam, lParam);
}

void
win_update_tip(HWND src, int cx, int cy)
{
  if (!tip_enabled)
    return;

  if (!tip_wnd) {
    NONCLIENTMETRICS nci;

   /* First make sure the window class is registered */

    if (!tip_class) {
      WNDCLASS wc;
      wc.style = CS_HREDRAW | CS_VREDRAW;
      wc.lpfnWndProc = tip_proc;
      wc.cbClsExtra = 0;
      wc.cbWndExtra = 0;
      wc.hInstance = inst;
      wc.hIcon = null;
      wc.hCursor = null;
      wc.hbrBackground = null;
      wc.lpszMenuName = null;
      wc.lpszClassName = "SizeTipClass";
      tip_class = RegisterClass(&wc);
    }

   /* Prepare other GDI objects and drawing info */

    tip_bg = GetSysColor(COLOR_INFOBK);
    tip_text = GetSysColor(COLOR_INFOTEXT);
    memset(&nci, 0, sizeof (NONCLIENTMETRICS));
    nci.cbSize = sizeof (NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof (NONCLIENTMETRICS),
                         &nci, 0);
    tip_font = CreateFontIndirect(&nci.lfStatusFont);
  }

 /* Generate the tip text */

  char str[32];
  sprintf(str, "%dx%d", cx, cy);

  if (!tip_wnd) {
   /* calculate the tip's size */

    HDC dc = CreateCompatibleDC(null);
    SIZE sz;    
    GetTextExtentPoint32(dc, str, strlen(str), &sz);
    DeleteDC(dc);

    RECT wr;
    GetWindowRect(src, &wr);

    int ix = max(wr.left, 16);
    int iy = max(wr.top - sz.cy, 16);

   /* Create the tip window */
    tip_wnd =
      CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
                     MAKEINTRESOURCE(tip_class), str, WS_POPUP, ix, iy, sz.cx,
                     sz.cy, null, null, inst, null);
    ShowWindow(tip_wnd, SW_SHOWNOACTIVATE);
  }
  else {
   /* Tip already exists, just set the text */
    SetWindowText(tip_wnd, str);
  }
}

void
win_enable_tip(void)
{
  tip_enabled = true;
}

void
win_disable_tip(void)
{
  if (tip_wnd) {
    DestroyWindow(tip_wnd);
    tip_wnd = null;
  }
  tip_enabled = false;
}
