#include "stdafx.h"
#include "cprogress.h"
#include "resource.h"
#include <CommCtrl.h>
#include "csvr.h"
#include "util.h"

#define WM_START_INSTALL WM_USER+100
#define WM_UPDATE_STATE WM_USER+101

namespace CloudCare
{
	INT_PTR CALLBACK CProgressDialog::dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
	{
		CProgressDialog *pd = (CProgressDialog *)GetWindowLongPtr(hDlg, DWLP_USER);

		switch(message) {
		case WM_COMMAND:
			{
				switch(LOWORD(wParam)) {
				case  IDCANCEL:
					{
						int ret = MessageBox(hDlg, _T("是否确定取消安装?"), MB_TITLE_TEXT, MB_YESNO);
						if (ret != IDYES)
						{
							return TRUE;
						}
						SendMessage(hDlg, WM_CLOSE,0,0);
						return TRUE;
					}
					break;
				default:
					break;
				}
			}
			break;
		case  WM_INITDIALOG:
			{
				CProgressDialog *dd = (CProgressDialog *)lParam;
				dd->onInitDialog(hDlg, wParam, lParam);
				return TRUE;
			}
			break;
		case WM_START_INSTALL:
			if (pd)
			{
				pd->onStartInstall(wParam, lParam);
				return TRUE;
			}
			break;
		case WM_UPDATE_STATE:
			if(pd)
			{
				pd->onUpdateState(wParam, lParam);
			}
			return TRUE;
		case WM_CLOSE:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
			break;
		default: 
			break;
		}

		return (INT_PTR)FALSE;

	}

	INT_PTR CProgressDialog::onInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam) {
		SetWindowText(hDlg, _T("安装"));
		hDlgWnd = hDlg;
		SetWindowLongPtr(hDlg, DWLP_USER,(LONG)this);
		hProgressCtrl = GetDlgItem(hDlg, IDC_PROGRESS_DOWNLOAD);
		hStateCtl = GetDlgItem(hDlg, IDC_STATIC_STATE);

		PostMessage(hDlg, WM_START_INSTALL, NULL, NULL);
		
		return (INT_PTR)TRUE;
	}

	DWORD WINAPI routine(LPVOID param)
	{
		Sleep(500);

		CProgressDialog *dlg = (CProgressDialog *)param;

		if (!dlg->m_bUpgrade)
		{
			if(!CSvrManager::shareInstance().install(dlg->cfg.imagePath)) {
				PostMessage(dlg->hDlgWnd, WM_UPDATE_STATE, (WPARAM)101, NULL);
				return 1;
			}
		}

		SendMessage(dlg->hDlgWnd, WM_UPDATE_STATE, (WPARAM)10, LPARAM(_T("正在启动服务...")));

		if (!CSvrManager::shareInstance().start())
		{
			if (!dlg->m_bUpgrade)
			{
				CSvrManager::shareInstance().uninstall();
			}
			PostMessage(dlg->hDlgWnd, WM_UPDATE_STATE, (WPARAM)101, NULL);
			return 1;
		}

		SendMessage(dlg->hDlgWnd, WM_UPDATE_STATE, (WPARAM)99, NULL);

		Sleep(100);

		PostMessage(dlg->hDlgWnd, WM_UPDATE_STATE, (WPARAM)100, NULL);

		return 0;
	}

	void CProgressDialog::onStartInstall(WPARAM wParam, LPARAM lParam)
	{
		CreateThread(NULL, 0,routine, (LPVOID)this, 0, NULL);
	}

	void CProgressDialog::onUpdateState(WPARAM wParam, LPARAM lParam)
	{
		int n = (int)wParam;
		if (n > 100)
		{
			MessageBox(NULL, CSvrManager::shareInstance().status.second.c_str(), MB_TITLE_TEXT, MB_OK);
			EndDialog(hDlgWnd, n-100);
		} else {
			setPos((int)wParam, LPCTSTR(lParam));
			if (n == 100)
			{
				EndDialog(hDlgWnd, 0);
			}
		}
	}

	INT_PTR CProgressDialog::show(HWND hParent)
	{
		return DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_DIALOG_DOWNLOAD), hParent, CProgressDialog::dlgProc,(LPARAM)this);
	}

	void CProgressDialog::setPos(int pos, LPCTSTR status/* =NULL */)
	{
		pos = min(100, pos);
		SendMessage(hProgressCtrl, PBM_SETPOS, (WPARAM)pos, NULL);
		if (status)
		{
			SetWindowText(hStateCtl, status);
		}
	}
};