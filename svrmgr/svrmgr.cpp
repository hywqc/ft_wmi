// svrmgr.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "svrmgr.h"
#include "csvr.h"
#include "util.h"
#include "cmetricsel.h"
#include "ctrayicon.h"
#include "cmaincfg.h"
#include <strsafe.h>
#include "MainWindow.h"
#include <fstream>

#pragma comment(lib,"Comctl32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "winhttp")

#ifdef _DEBUG
#pragma comment(lib, "yaml_staticd.lib")
#else
#pragma comment(lib, "yaml_static.lib")
#endif

LPCTSTR AppName = _T("王教授主机诊断探针");
LPCTSTR AppNameEn = _T("ProfWang Probe");
extern LPCTSTR ServiceName = _T("ProfWang Probe Service"); 
extern LPCTSTR ServiceDescription = _T("支持收集主机相关监控指标并上报到王教授中进行诊断"); 


EditionType gEditionType = EditionType_Propord;

HINSTANCE hInst;
HWND hMainDlg = NULL;

wstring versionFromRes;
wstring remoteVersion;

wstring gRemoteHostUrl;

extern CC::CMainCfgDialog *pCfgDialog = nullptr;


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	

	hInst = hInstance;

	versionFromRes = CC::parseVersion(CC::loadVersionRes());

	switch(gEditionType)
	{
	case EditionType_Propord:
		AppName = _T("王教授主机诊断探针(预发)");
		break;
	case EditionType_Test:
		AppName = _T("王教授主机诊断探针(测试)");
		break;
	default:
		break;
	}


	HANDLE hMutex = CreateMutex(nullptr, FALSE, _T("profwangprobe"));
	DWORD dwerr = GetLastError();
	if (hMutex)
	{
		if (ERROR_ALREADY_EXISTS  == dwerr)
		{
			CloseHandle(hMutex);
			::MessageBox(NULL, _T("当前已存在运行的实例"), MB_TITLE_TEXT, MB_OK);
			return 1;
		}
	}

	//__argc
	//GetCommandLine()
	//CommandLineToArgv()

	MSG msg;
	HACCEL hAccelTable;
	BOOL ret;


#if 1

	if (!CC::CSvrManager::shareInstance().connectSCM())
	{
		::MessageBox(NULL, CC::CSvrManager::shareInstance().status.second.c_str(), MB_TITLE_TEXT, MB_OK);
		return FALSE;
	}

	CC::CSvrManager::shareInstance().getServiceInfo();

	if (CC::CSvrManager::shareInstance().haveNotInstalled())
	{
		pCfgDialog = new CC::CMainCfgDialog(CC::FOR_INSTALL);
		if(!pCfgDialog->show())
		{
			return FALSE;
		}
		hMainDlg = pCfgDialog->getDlgWnd();

	} else {

		BOOL bUpgrade =  (versionFromRes != CC::CSvrManager::shareInstance().service.version);
		if (bUpgrade)
		{
			pCfgDialog = new CC::CMainCfgDialog(CC::FOR_UPGRADE);
			if(!pCfgDialog->show())
			{
				return FALSE;
			}
			hMainDlg = pCfgDialog->getDlgWnd();

		} else {


			pCfgDialog = new CC::CMainCfgDialog(CC::FOR_CONFIG);
			if(!pCfgDialog->show(TRUE))
			{
				return FALSE;
			}

			hMainDlg = pCfgDialog->getDlgWnd();
			ShowWindow(hMainDlg, SW_HIDE);

			CC::CTrayIcon::shareInstance().show();
			CC::CTrayIcon::shareInstance().showTrayMsg(_T("点击此处进行管理"), _T("王教授主机诊断探针"));
		}
	}

#else

	CMainWindow mainWnd(570,560);
	if(!mainWnd.create(AppName))
	{
		return 1;
	}

	mainWnd.center();
	mainWnd.show(nCmdShow);
	mainWnd.update();

#endif


	
 	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SVRMGR));

	while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (ret == -1)
		{
			return -1;
		}

		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



