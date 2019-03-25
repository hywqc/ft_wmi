#include "stdafx.h"
#include "MainWindow.h"
#include <CommDlg.h>
#include <CommCtrl.h>
#include <WindowsX.h>

#define RGB_RED RGB(255,0,0)
static  const int kRadioClustID = 2000;
static  const int kRadioSingleID = 2001;
static  const int kStaticCfg = 2002;

static LRESULT WINAPI wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	CMainWindow *pthis = (CMainWindow *)GetWindowLong(hWnd, GWL_USERDATA);
	if (pthis)
	{
		return pthis->wndProc(message, wParam, lParam);
	}
	switch(message)
	{
	case WM_NCCREATE:
		{
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
			CMainWindow *pthis = (CMainWindow *)pcs->lpCreateParams;
			pthis->m_hWnd = hWnd;
			SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG)pthis);
		}
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL CMainWindow::create(LPCTSTR name)
{
	static BOOL breg = FALSE;
	if (!breg)
	{
		WNDCLASSEX wcx={0};
		wcx.cbSize = sizeof(WNDCLASSEX);
		//wcx.hbrBackground = HBRUSH(COLOR_WINDOW+1);
		wcx.lpfnWndProc = wndproc;
		wcx.lpszClassName = _T("svrmgrmain");
		wcx.style = CS_VREDRAW|CS_HREDRAW;
		wcx.hCursor = LoadCursor(NULL, IDC_ARROW);

		breg = RegisterClassEx(&wcx);
	}
	if (!breg)
	{
		return FALSE;
	}
	m_hWnd = CreateWindowEx(0, _T("svrmgrmain"), name, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION,0,0,m_width, m_height, NULL, NULL, hInst, this);
	return (m_hWnd != NULL);
}

void CMainWindow::center()
{
	int cxScreen = GetSystemMetrics(SM_CXFULLSCREEN);
	int cyScreen = GetSystemMetrics(SM_CYFULLSCREEN);

	int x = (cxScreen - m_width) / 2;
	int y = (cyScreen - m_height) / 2;

	SetWindowPos(m_hWnd, NULL, x, y, m_width, m_height, SWP_NOSIZE | SWP_NOZORDER);
}

LRESULT CMainWindow::wndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	switch(message)
	{
	case WM_CREATE:
		onCreate();
		break;
	case WM_PAINT:
		hDC = BeginPaint(m_hWnd, &ps);
		onPaint(hDC);
		EndPaint(m_hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	case WM_COMMAND:
		{
			int cmdID = LOWORD(wParam);
			int code = HIWORD(wParam);
			onCommand(cmdID, code);
		}
		break;
	case WM_NOTIFY:
		onNotify(wParam, lParam);
		break;
	case WM_CTLCOLORSTATIC:
		{
			HWND hwnd = (HWND)lParam;
			HDC hdc = (HDC)wParam;

			if (m_staticCfg != NULL && hwnd == m_staticCfg)
			{
				SetTextColor(hdc, RGB(0,0,200));
			}
			else
			{
				BOOL bhave = FALSE;
				for (size_t i=0;i<m_args.size();i++)
				{
					if (m_args[i].hStaticDesc == hwnd)
					{
						bhave = TRUE;
						break;
					}
				}
				if (bhave)
				{
					SetTextColor(hdc, RGB(200,200,200));
				}
			}
			
			return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
			
		}
		break;
	case WM_CTLCOLORBTN:
		{
			HWND hwnd = (HWND)lParam;
			if (hwnd == m_btnOk)
			{
				HBRUSH bru = CreateSolidBrush(RGB(0,0,200));
				return (LRESULT)bru;
			}
		}
		break;
	default:
		break;
	}
	return DefWindowProc(m_hWnd, message, wParam, lParam);
}


void CMainWindow::onCreate()
{
	//HWND hbtn = CreateWindowEx(0, WC_BUTTON, _T("click"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 300, 20, 100, 30, m_hWnd, (HMENU)666, hInst, nullptr);

	LOGFONT lf = {0};
	lf.lfHeight = -15;
	lf.lfWeight = 0x190;
	lf.lfOutPrecision = 0x3;
	lf.lfClipPrecision = 0x2;
	lf.lfQuality = 0x1;
	lf.lfPitchAndFamily = 0x22;
	_tcscpy(lf.lfFaceName, _T("Tahoma"));
	m_hTitleFont = CreateFontIndirect(&lf);

	m_args.push_back(ArgumentItem(_T("团队 ID : "), _T("请输入王教授的团队 ID")));
	m_args.push_back(ArgumentItem(_T("Upload ID : "), _T("请输入当前主机的云实例 ID")));
	m_args.push_back(ArgumentItem(_T("Access Key : "), _T("请输入 Access Key")));
	m_args.push_back(ArgumentItem(_T("Secret Key : "), _T("请输入 Secret Key")));
	m_args.push_back(ArgumentItem(_T("IP : "), _T("可选项, 当前主机的出口 IP"),FALSE));
	m_args.push_back(ArgumentItem(_T("监听端口 : "), _T("可选项, 监听端口, 默认9100"), FALSE));

	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);

	HDC hdc = GetWindowDC(m_hWnd);
	HFONT holdfont = (HFONT)SelectObject(hdc, m_hTitleFont);

	const int kIntervalV = 55;
	const int kTitleStartX = 60;
	const int kEditHeight = 30;

	int titleY = 60;
	RECT rcTtitle = {0};
	rcTtitle.left = kTitleStartX;
	rcTtitle.right = kTitleStartX+100;

	for (size_t i = 0; i < m_args.size(); i++)
	{
		ArgumentItem &item = m_args[i];

		rcTtitle.top = titleY;
		rcTtitle.bottom = titleY+20;
		item.rcTtitle = rcTtitle;

		SIZE sz = {0};
		GetTextExtentPoint(hdc, item.key.c_str(), item.key.length(), &sz);
		item.keyTextSize = sz;

		item.rcEditWrapper.left = rcTtitle.right + 20;
		item.rcEditWrapper.top = rcTtitle.top - (kEditHeight - sz.cy)/2;
		item.rcEditWrapper.right = rcClient.right - kTitleStartX-20;
		item.rcEditWrapper.bottom = item.rcEditWrapper.top + kEditHeight;

		item.rcEdit.left = item.rcEditWrapper.left+2;
		item.rcEdit.top = rcTtitle.top;
		item.rcEdit.right = item.rcEditWrapper.right-2;
		item.rcEdit.bottom = rcTtitle.bottom;

		item.hEdit = CreateWindowEx(0, WC_EDIT, _T(""), WS_CHILD|WS_VISIBLE, item.rcEdit.left , item.rcEdit.top, item.rcEdit.right - item.rcEdit.left, item.rcEdit.bottom - item.rcEdit.top,  m_hWnd, (HMENU)i, hInst, nullptr);
		SendMessage(item.hEdit, WM_SETFONT, WPARAM(m_hTitleFont), LPARAM(TRUE));
		item.hStaticDesc = CreateWindowEx(0, WC_STATIC, item.desc.c_str(), WS_CHILD|WS_VISIBLE|SS_NOTIFY, item.rcEdit.left+5 , item.rcEdit.top, item.rcEdit.right - item.rcEdit.left-5, item.rcEdit.bottom - item.rcEdit.top,  m_hWnd, HMENU(i+100), hInst, nullptr);
		SendMessage(item.hStaticDesc, WM_SETFONT, WPARAM(m_hTitleFont), LPARAM(TRUE));
		//SetClassLongPtr(item.hStaticDesc, GCL_HCURSOR,(LONG)LoadCursor(NULL, IDC_IBEAM));
		//SetWindowPos(item.hEdit, item.hStaticDesc, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOREDRAW);

		SetWindowLongPtr(item.hEdit, GWL_USERDATA, (LONG)&item);

		titleY+=kIntervalV;
	}

	SelectObject(hdc, holdfont);
	ReleaseDC(m_hWnd, hdc);

		m_radioCluster = CreateWindowEx(0, WC_BUTTON, _T("集群模式"), WS_CHILD | WS_VISIBLE |BS_AUTORADIOBUTTON, rcTtitle.right+20, titleY, 90, 30, m_hWnd, (HMENU)kRadioClustID, hInst, nullptr);
		SendMessage(m_radioCluster, WM_SETFONT, WPARAM(m_hTitleFont), LPARAM(TRUE));
		m_radioSingle = CreateWindowEx(0, WC_BUTTON, _T("单机模式"), WS_CHILD | WS_VISIBLE |BS_AUTORADIOBUTTON, rcTtitle.right+20+100, titleY, 90, 30, m_hWnd, (HMENU)kRadioSingleID, hInst, nullptr);
		SendMessage(m_radioSingle, WM_SETFONT, WPARAM(m_hTitleFont), LPARAM(TRUE));

		m_staticCfg = CreateWindowEx(0, WC_STATIC, _T("检测项配置>"), WS_CHILD|WS_VISIBLE|SS_NOTIFY, 400 , titleY+5, 100, 30,  m_hWnd, HMENU(kStaticCfg), hInst, nullptr);
		SendMessage(m_staticCfg, WM_SETFONT, WPARAM(m_hTitleFont), LPARAM(TRUE));


		int btnWidth = 120;
		int btnHeight = 40;
		int x = (rcClient.right - btnWidth*2 - 35)/2;
		int y = rcClient.bottom - btnHeight - 30;

		m_btnOk = CreateWindowEx(0, WC_BUTTON, _T("安装"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, btnWidth, btnHeight, m_hWnd, (HMENU)666, hInst, nullptr);
		SendMessage(m_btnOk, WM_SETFONT, WPARAM(m_hTitleFont), LPARAM(TRUE));
		m_btnCancel = CreateWindowEx(0, WC_BUTTON, _T("取消"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x+btnWidth+35, y, btnWidth, btnHeight, m_hWnd, (HMENU)667, hInst, nullptr);
		SendMessage(m_btnCancel, WM_SETFONT, WPARAM(m_hTitleFont), LPARAM(TRUE));


}

void CMainWindow::onNotify(WPARAM wParam, LPARAM lParam)
{
}

void CMainWindow::updateEditDisp(int id)
{
	if (id >= m_args.size())
	{
		return;
	}
	const ArgumentItem &item = m_args[id];
	if(Edit_GetTextLength(item.hEdit) <= 0)
	{
		ShowWindow(item.hStaticDesc, SW_SHOW);
	} else {
		ShowWindow(item.hStaticDesc, SW_HIDE);
	}
}

void CMainWindow::onCommand(int cmdID, int code)
{

	switch(code)
	{
	case STN_CLICKED:
		{
			if (cmdID == kStaticCfg)
			{

			}
			else
			{
				if (cmdID - 100 >= m_args.size())
				{
					return;
				}
				const ArgumentItem &item = m_args[cmdID-100];
				ShowWindow(item.hStaticDesc, SW_HIDE);
				SetFocus(item.hEdit);
			}
		}
		break;
	//case EN_SETFOCUS:

	//	break;
	case EN_KILLFOCUS:
		{
			OutputDebugString(_T("kill focus\n"));
			updateEditDisp(cmdID);
		}
		break;
	case EN_UPDATE:
	case EN_CHANGE:
		updateEditDisp(cmdID);
		break;

	default:
		break;
	}

	/*switch(cmdID)
	{
	case 666:
	{
	LOGFONT lf;
	CHOOSEFONT cf = {0};
	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hInstance = hInst;
	cf.hwndOwner = m_hWnd;
	cf.lpLogFont = &lf;
	cf.Flags = CF_SCREENFONTS;
	cf.nFontType = SCREEN_FONTTYPE;

	ChooseFont(&cf);

	m_hTitleFont = CreateFontIndirect(&lf);
	InvalidateRect(m_hWnd, nullptr, FALSE);
	}
	break;
	default:
	break;
	}*/
}


void CMainWindow::onPaint(HDC hDC)
{
	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);
	int cx = rcClient.right - rcClient.left;
	int cy = rcClient.bottom - rcClient.top;
	HDC hdcMem = CreateCompatibleDC(hDC);
	HBITMAP hbmpMem = CreateCompatibleBitmap(hDC, cx, cy);
	SelectObject(hdcMem, hbmpMem);
	FillRect(hdcMem, &rcClient, HBRUSH(COLOR_WINDOW+1));

	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(220,220,220));
	SelectObject(hdcMem, hPen);

	COLORREF oldTextClr;

	if (m_hTitleFont)
	{
		SelectObject(hdcMem, m_hTitleFont);
	}


	for (size_t i = 0; i < m_args.size(); i++)
	{
		ArgumentItem item = m_args[i];
		RECT &rcTtitle = item.rcTtitle;

		DrawText(hdcMem,item.key.c_str(), item.key.length(), &rcTtitle, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		if (item.bmust)
		{
			oldTextClr = SetTextColor(hdcMem, RGB_RED);
			RECT rcrisk={0};
			rcrisk.right = rcTtitle.right - item.keyTextSize.cx - 8;
			rcrisk.left = rcrisk.right-20;
			rcrisk.top = rcTtitle.top;
			rcrisk.bottom = rcTtitle.bottom;
			DrawText(hdcMem, _T("*"), 1, &rcrisk, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			SetTextColor(hdcMem, oldTextClr);
		}

		RoundRect(hdcMem, item.rcEditWrapper.left, item.rcEditWrapper.top, item.rcEditWrapper.right, item.rcEditWrapper.bottom, 5, 5);
	}

	BitBlt(hDC, 0, 0, cx, cy, hdcMem, 0, 0, SRCCOPY);

	DeleteObject(hdcMem);
	DeleteObject(hbmpMem);
	DeleteObject(hPen);
}

