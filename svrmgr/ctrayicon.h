#pragma once

#include <ShellAPI.h>

#define WM_TRAY WM_USER+1

#define ID_TRAY_SHOW 2000
#define ID_TRAY_QUIT (ID_TRAY_SHOW+1)
#define ID_TRAY_STAUS (ID_TRAY_SHOW+2)
#define ID_TRAY_VERSION (ID_TRAY_SHOW+3)
#define ID_TRAY_CONFIG (ID_TRAY_SHOW+4)
#define ID_TRAY_STOP (ID_TRAY_SHOW+5)
#define ID_TRAY_PAUSE (ID_TRAY_SHOW+6)
#define ID_TRAY_RESUME (ID_TRAY_SHOW+7)
#define ID_TRAY_START (ID_TRAY_SHOW+8)
#define ID_TRAY_EXIT (ID_TRAY_SHOW+9)
#define ID_TRAY_STARTING (ID_TRAY_SHOW+10)
#define ID_TRAY_STOPING (ID_TRAY_SHOW+11)
#define ID_TRAY_PAUSING (ID_TRAY_SHOW+12)
#define ID_TRAY_RESUMING (ID_TRAY_SHOW+13)
#define ID_TRAY_UNINSTALL (ID_TRAY_SHOW+14)

enum TrayActionType
{
	TrayActionType_Start = 0,
	TrayActionType_Stop,
	TrayActionType_Uninstall,
	TrayActionType_CheckVersion,
	TrayActionType_Config,
};

namespace CloudCare
{
	 
	class CTrayIcon
	{
		friend static LRESULT WINAPI wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		friend DWORD WINAPI svrControlRoutine(LPVOID param);

	public:
		static CTrayIcon & shareInstance()
		{
			static CTrayIcon ti;
			return ti;
		}

		CTrayIcon()
			:trayWnd(NULL)
			,isStarting(FALSE)
			,isStoping(FALSE)
			,isPausing(FALSE)
			,isResuming(FALSE)
			,isUninstalling(FALSE)
		{

		}
	private:
		HWND trayWnd;
		NOTIFYICONDATA nid;

		BOOL isStarting;
		BOOL isStoping;
		BOOL isPausing;
		BOOL isResuming;
		BOOL isUninstalling;

	protected:
		LRESULT onTray(WPARAM wParam, LPARAM lParam);
	public:
		BOOL show();
		BOOL remove();
		BOOL uninstall();
		void showTrayMsg(LPCTSTR info, LPCTSTR title=_T("corsair"));
		HWND getTrayWnd() const {
			return trayWnd;
		}

		void onShowConfig();
		void onStartSvr();
		void onStopSvr();
		void onResumeSvr();
		void onPauseSvr();
		void onSvrCtrlResult(WPARAM wParam, LPARAM lParam);
	};
};