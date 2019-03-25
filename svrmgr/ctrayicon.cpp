#include "stdafx.h"
#include "ctrayicon.h"
#include <ShellAPI.h>
#include "resource.h"
#include "csvr.h"
#include "cmetricsel.h"
#include "cmaincfg.h"
#include "util.h"
#include "VersionDialog.h"

#define WM_SVRCTRL_RESULT WM_USER+200

namespace CloudCare
{

	static LRESULT WINAPI wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
	{

		CTrayIcon *ti = (CTrayIcon *)GetWindowLongPtr(hWnd, GWL_USERDATA);

		switch(message) {
		case WM_TRAY:
			if (ti)
			{
				ti->onTray(wParam, lParam);
			}
			break;
		case WM_SVRCTRL_RESULT:
			if (ti)
			{
				ti->onSvrCtrlResult(wParam, lParam);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

	BOOL CTrayIcon::show()
	{
		WNDCLASSEX wcx = {0};
		wcx.cbSize = sizeof(wcx);
		wcx.hInstance = hInst;
		wcx.lpszClassName = _T("traywnd");
		wcx.lpfnWndProc = wndproc;
		RegisterClassEx(&wcx);
		HWND hWnd = CreateWindowEx(0, _T("traywnd"), _T(""), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,0,1,1,NULL,NULL,hInst,NULL);
		if (!hWnd)
		{
			return FALSE;
		}
		SetWindowLongPtr(hWnd, GWL_USERDATA,(LONG)this);

		trayWnd = hWnd;

		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = hWnd;
		nid.uID = IDI_SVRMGR;
		nid.uFlags =  NIF_MESSAGE|NIF_ICON|NIF_TIP|NIF_INFO;
		nid.uCallbackMessage = WM_TRAY;
		nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SVRMGR));
		lstrcpy(nid.szTip, _T("王教授主机诊断探针"));

		return Shell_NotifyIcon(NIM_ADD, &nid);
	}

	BOOL CTrayIcon::remove()
	{
		NOTIFYICONDATA nid={0};
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = trayWnd;
		nid.uID = IDI_SVRMGR;
		nid.uFlags = NIF_ICON;
		BOOL bret = Shell_NotifyIcon(NIM_DELETE, &nid);
		return bret;
	}



	void CTrayIcon::showTrayMsg(LPCTSTR info, LPCTSTR title/* =_T("corsair") */)
	{
		lstrcpy(nid.szInfoTitle,  title);
		lstrcpy(nid.szInfo, info);
		nid.uTimeout = 2000;
		Shell_NotifyIcon(NIM_MODIFY, &nid);
	}

	LRESULT CTrayIcon::onTray(WPARAM wParam, LPARAM lParam) {
		switch(lParam){
		case WM_RBUTTONUP:
		case WM_LBUTTONUP:
			{
				CSvrManager::shareInstance().getServiceInfo();

				HMENU hMenu = CreatePopupMenu();
				switch(CSvrManager::shareInstance().service.state.dwCurrentState) {
				case SERVICE_STOPPED:
					AppendMenu(hMenu, MF_STRING, ID_TRAY_STAUS, _T("状态: 停止"));
					AppendMenu(hMenu, MF_STRING, ID_TRAY_START, _T("启动"));
					break;
				case SERVICE_RUNNING:
					AppendMenu(hMenu, MF_STRING, ID_TRAY_STAUS, _T("状态: 正在运行"));
					AppendMenu(hMenu, MF_STRING, ID_TRAY_STOP, _T("停止"));
					//AppendMenu(hMenu, MF_STRING, ID_TRAY_PAUSE, _T("暂停"));
					break;
				case  SERVICE_PAUSED:
					AppendMenu(hMenu, MF_STRING, ID_TRAY_STAUS, _T("状态: 暂停"));
					AppendMenu(hMenu, MF_STRING, ID_TRAY_RESUME, _T("恢复"));
					AppendMenu(hMenu, MF_STRING, ID_TRAY_STOP, _T("停止"));
					break;
				case SERVICE_START_PENDING:
					AppendMenu(hMenu, MF_STRING, ID_TRAY_STARTING, _T("状态: 正在启动..."));
					break;
				case SERVICE_STOP_PENDING:
					AppendMenu(hMenu, MF_STRING, ID_TRAY_STARTING, _T("状态: 正在停止..."));
					break;
				case SERVICE_PAUSE_PENDING:
					AppendMenu(hMenu, MF_STRING, ID_TRAY_STARTING, _T("状态: 正在暂停..."));
					break;
				case SERVICE_CONTINUE_PENDING:
					AppendMenu(hMenu, MF_STRING, ID_TRAY_STARTING, _T("状态: 正在恢复..."));
					break;
				default:
					break;
				}

				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(hMenu, MF_STRING, ID_TRAY_CONFIG, _T("配置"));
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

				wstring v = _T("版本(");
				v += versionFromRes;
				v += _T(")");

				AppendMenu(hMenu, MF_STRING, ID_TRAY_VERSION, v.c_str());
				//AppendMenu(hMenu, MF_STRING, ID_TRAY_SHOW, _T("检查更新"));
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

				AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, _T("退出"));

				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(hMenu, MF_STRING, ID_TRAY_UNINSTALL, _T("卸载"));


				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(trayWnd);
				int cmd =TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, trayWnd, NULL);
				switch(cmd)
				{
				case ID_TRAY_CONFIG:
					onShowConfig();
					break;
				case ID_TRAY_START:
					onStartSvr();
					break;
				case ID_TRAY_STOP:
					onStopSvr();
					break;
				case ID_TRAY_PAUSE:
					onPauseSvr();
					break;
				case ID_TRAY_RESUME:
					onResumeSvr();
					break;
				case ID_TRAY_EXIT:
					remove();
					PostQuitMessage(0);
					break;
				case ID_TRAY_UNINSTALL:
					uninstall();
					break;
				case ID_TRAY_VERSION:
					{
						std::string remoteVersion;
						if (checkVersion(remoteVersion))
						{
							std::wstring v = parseVersion(remoteVersion);
							remoteVersion = wide2string(v);
							if (v != versionFromRes)
							{
								PostMessage(trayWnd, WM_NULL,0,0);
								//pCfgDialog->show();
								CVersionDialog dlg(versionFromRes, v);
								dlg.show(pCfgDialog->getDlgWnd());
								return FALSE;
								//MessageBox(NULL, _T("发现新版本, 请前往下载"), MB_TITLE_TEXT, MB_OK);
							} else {
								MessageBox(NULL, _T("当前已是最新版本"), MB_TITLE_TEXT, MB_OK);
							}
						} else {
							MessageBox(NULL, _T("检测版本失败"), MB_TITLE_TEXT, MB_OK);
						}
					}
					break;
				default: 
					break;
				}

				PostMessage(trayWnd, WM_NULL,0,0);
			}
			break;
		case NIN_BALLOONUSERCLICK:
			{
				//_trace(_T("NIN_BALLOONUSERCLICK"));
				//onShowConfig();
			}
			break;
		default:
			break;
		}
		return 0;
	}

	void CTrayIcon::onSvrCtrlResult(WPARAM wParam, LPARAM lParam)
	{
		BOOL bok = (BOOL)wParam;
		BOOL forUninstall = FALSE;
		std:wstring errmsg;
		if (isStoping)
		{
			isStoping = FALSE;
		} else if (isStarting) 
		{
			isStarting = FALSE;
			if (!bok)
			{
				errmsg = getServiceErrText();
			}

		} else if (isPausing) 
		{
			isPausing = FALSE;

		} else if (isResuming)
		{
			isResuming = FALSE;
		} else if (isUninstalling)
		{
			isUninstalling = FALSE;
			forUninstall = TRUE;
		}
		if (bok)
		{
			if (forUninstall)
			{
				::MessageBox(NULL, _T("卸载完成"), MB_TITLE_TEXT, MB_OK|MB_ICONINFORMATION);
				PostQuitMessage(0);

				return;
			}
		} else {
			if (!errmsg.empty())
			{
				::MessageBox(NULL, errmsg.c_str(), MB_TITLE_TEXT, MB_OK|MB_ICONWARNING);				
			} else {
				::MessageBox(NULL, CSvrManager::shareInstance().status.second.c_str(), MB_TITLE_TEXT, MB_OK|MB_ICONWARNING);
			}
		}

		CSvrManager::shareInstance().getServiceInfo();
	}

	void CTrayIcon::onShowConfig()
	{
		pCfgDialog->show();
	}

	DWORD WINAPI svrControlRoutine(LPVOID param)
	{
		CTrayIcon *ti = (CTrayIcon *)param;
		BOOL bok = FALSE;
		if (ti->isStoping)
		{
			bok = CSvrManager::shareInstance().stop();
		} else if (ti->isStarting)
		{
			bok = CSvrManager::shareInstance().start();
		} else if (ti->isPausing)
		{
			bok = CSvrManager::shareInstance().pause();
		} else if (ti->isResuming)
		{
			bok = CSvrManager::shareInstance().resume();
		} else if (ti->isUninstalling)
		{
			bok = CSvrManager::shareInstance().stop();
			bok = CSvrManager::shareInstance().uninstall();

		}
		PostMessage(ti->trayWnd, WM_SVRCTRL_RESULT, (WPARAM)bok, NULL);
		return 0;
	}

	void CTrayIcon::onStartSvr()
	{
		isStarting = TRUE;
		CreateThread(NULL, 0, svrControlRoutine, (LPVOID)this, 0, NULL);
	}


	void CTrayIcon::onStopSvr()
	{
		isStoping = TRUE;
		CreateThread(NULL, 0, svrControlRoutine, (LPVOID)this, 0, NULL);
	}

	void CTrayIcon::onResumeSvr()
	{
		isResuming = TRUE;
		CreateThread(NULL, 0, svrControlRoutine, (LPVOID)this, 0, NULL);
	}

	void CTrayIcon::onPauseSvr()
	{
		isPausing = TRUE;
		CreateThread(NULL, 0, svrControlRoutine, (LPVOID)this, 0, NULL);
	}

	BOOL CTrayIcon::uninstall()
	{
		if(::MessageBox(NULL, _T("是否卸载探针服务?"), MB_TITLE_TEXT, MB_YESNOCANCEL|MB_ICONQUESTION) != IDYES)
		{
			return TRUE;
		}
		isUninstalling = TRUE;
		CreateThread(NULL, 0, svrControlRoutine, (LPVOID)this, 0, NULL);
		return TRUE;
	}

};