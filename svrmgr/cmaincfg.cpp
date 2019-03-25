#include "stdafx.h"
#include "cmaincfg.h"
#include "resource.h"
#include <WindowsX.h>
#include "csvr.h"
#include "util.h"
#include "cmetricsel.h"
#include <CommCtrl.h>
#include "ctrayicon.h"
#include "cprogress.h"
#include <fstream>

namespace CloudCare
{

	INT_PTR CALLBACK CMainCfgDialog::dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
	{
		CMainCfgDialog *mc = (CMainCfgDialog *)GetWindowLongPtr(hDlg, DWLP_USER);
		switch(message) {
		case WM_COMMAND:
			if (mc)
			{
				return mc->onCommond(wParam, lParam);
			}
			break;
		case WM_INITDIALOG:
			{
				CMainCfgDialog *mc = (CMainCfgDialog *)lParam;
				return mc->onInitDialog(hDlg, wParam, lParam);
			}
			break;
		case WM_CLOSE:
			{
				mc->onClose();
				return TRUE;
			}
			break;
		case WM_DESTROY:
			mc->onDestroy();
			return TRUE;
			break;
		case WM_DETECT_NEW_VERSION:
			if (mc)
			{
				mc->onNewVersion();
			}
			break;
		//case WM_CTLCOLORDLG:
		//	{
		//		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
		//	}
		//	break;
		default:
			break;
		}
		return (INT_PTR)FALSE;
	}

	BOOL CMainCfgDialog::show(BOOL hide/* =FALSE */)
	{
		if (!hDlg)
		{
			hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_CONFIG), NULL, CMainCfgDialog::dlgProc,LPARAM(this));
		}
		if (!hide)
		{
			fillEditsFromCfg(CSvrManager::shareInstance().service.argsInfo);
			ShowWindow(hDlg, SW_SHOW);
		}
		return hDlg != NULL;
	}

	void CMainCfgDialog::switchType(int typ)
	{
		HWND hApplyBtn = GetDlgItem(hDlg, IDC_BUTTON_APPLY_CHANGE);
		HWND hCancelBtn = GetDlgItem(hDlg, IDCANCEL);
		HWND hInstallBtn = GetDlgItem(hDlg, IDOK);
		HWND hUpgradeBtn = GetDlgItem(hDlg, IDC_BUTTON_UPGRADE);
		HWND hEditTeamID = GetDlgItem(hDlg, IDC_EDIT_TEAMID);
		EnableWindow(hEditTeamID, TRUE);

		type = typ;
		if (typ == FOR_CONFIG)
		{
			EnableWindow(hEditTeamID, FALSE);
			SetWindowText(hDlg, (std::wstring(AppName)+_T(" 配置")).c_str());
			
			::ShowWindow(hInstallBtn, SW_HIDE);
			::ShowWindow(hUpgradeBtn, SW_HIDE);
			::ShowWindow(hApplyBtn, SW_SHOW);
			::ShowWindow(hCancelBtn, SW_SHOW);

			fillEditsFromCfg(CSvrManager::shareInstance().service.argsInfo);

			CC::startCheckVersion();

		} else if(typ == FOR_INSTALL) {
			SetWindowText(hDlg, (std::wstring(AppName)+_T(" 安装")).c_str());
			::ShowWindow(hInstallBtn, SW_SHOW);
			::ShowWindow(hUpgradeBtn, SW_HIDE);
			::ShowWindow(hApplyBtn, SW_HIDE);
			::ShowWindow(hCancelBtn, SW_HIDE);
		} else {
			SetWindowText(hDlg, (std::wstring(AppName)+_T(" 更新")).c_str());
			::ShowWindow(hUpgradeBtn, SW_SHOW);
			::ShowWindow(hInstallBtn, SW_HIDE);
			::ShowWindow(hApplyBtn, SW_HIDE);
			::ShowWindow(hCancelBtn, SW_HIDE);

			fillEditsFromCfg(CSvrManager::shareInstance().service.argsInfo);
		}
	}

	INT_PTR CMainCfgDialog::onInitDialog(HWND hDlgWnd, WPARAM wParam, LPARAM lParam)
	{

		HICON hicon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SVRMGR));
		if (hicon)
		{
			SendMessage(hDlgWnd, WM_SETICON, ICON_BIG, (LPARAM)hicon);
		}

		hDlg = hDlgWnd;
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG)this);


		RECT rcClient;
		GetClientRect(hDlg, &rcClient); 

		//RECT rcWindow;
		//GetClientRect(hDlg, &rcWindow);
		//SetWindowPos(hDlg, NULL, 0, 0, 500, rcWindow.bottom - rcWindow.top, SWP_NOZORDER|SWP_NOMOVE);

		HWND hStatic = CreateWindowEx(0, WC_STATIC, _T(""),WS_CHILD|WS_VISIBLE|SS_BITMAP,0,0,rcClient.right-rcClient.left, 64,hDlg,NULL,hInst,NULL);
		HBITMAP hBanner = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_BANNER));
		SendMessage(hStatic, STM_SETIMAGE, WPARAM(IMAGE_BITMAP), LPARAM(hBanner));

#if _DEBUG
		//HWND hRemoteEdit = GetDlgItem(hDlg, IDC_EDIT_REMOTEHOST);
		//HWND hRemoteStaic = GetDlgItem(hDlg, IDC_STATIC_REMOTEHOST);

		//ShowWindow(hRemoteEdit, SW_SHOW);
		//ShowWindow(hRemoteStaic, SW_SHOW);

#endif

		switchType(type);

		return (INT_PTR)FALSE;
	}

	INT_PTR CMainCfgDialog::onCommond(WPARAM wParam, LPARAM lParam)
	{
		switch(LOWORD(wParam)) {
		case  IDCANCEL:
			onCancel();
			return TRUE;
			break;
		case IDOK:
			onInstall();
			return TRUE;
			break;
		case IDC_BUTTON_METRIC_ITEMS:
			{
				CC::CMetricSelect dlg;
				dlg.show(hDlg);
			}
			break;
		case IDC_BUTTON_APPLY_CHANGE:
			{
				onApplyChange();
			}
			break;
		case IDC_BUTTON_UPGRADE:
			{
				onUpgrade();
			}
			break;
		default:
			break;
		}
		return (INT_PTR)FALSE;
	}

	BOOL CMainCfgDialog::action(ActionType type)
	{
		ServiceArgsInfo info;
		fillInfoFromEdits(info);
		if (info.teamid.empty()
			//|| info.assetid.empty()
			|| info.ak.empty()
			|| info.sk.empty())
		{
			::MessageBox(hDlg, _T("带 * 的为必填项, 不能为空"), MB_TITLE_TEXT, MB_OKCANCEL);
			return FALSE;
		}

		CSvrManager::shareInstance().service.argsInfo = info;


		switch(type) {
		case ActionType_UpdateCfg:
			{
				wstring cmdline = info.generateUpdateCfgCmdline(CSvrManager::shareInstance().service.exePath);
				UINT nret = runCmd(cmdline.c_str()) ;

				std::wstring errmsg;
				if (nret != 0)
				{
					if (nret == 1024)
					{
						errmsg = getServiceErrText();
					}
					if (errmsg.empty())
					{
						errmsg = _T("修改失败");
					}
					::MessageBoxEx(hDlg, errmsg.c_str(), MB_TITLE_TEXT, MB_OK|MB_ICONWARNING, 0);
					return FALSE;
				}
				
				return TRUE;
			}
			break;
		case ActionType_Install:
		case ActionType_Upgrade:
			{
				wstring prestr = (type == ActionType_Install ? _T("安装失败: ") : _T("更新失败: "));

				wstring destInstallPath = CC::getTargetExePath();
				wstring destQuerydPath = CC::getTargetInstallDir()+_T("\\osqueryd.exe");
				wstring destEnvCfgYml = CC::getTargetInstallDir()+_T("\\kv.json");
				wstring destFileinfoCfgYml = CC::getTargetInstallDir()+_T("\\fileinfo.json");
				wstring destVersionTxt = CC::getTargetInstallDir()+_T("\\version");

				wstring errmsg = installFromRes(destInstallPath, IDR_EXE_CORSAIR, _T("EXE"));
				if (!errmsg.empty())
				{
					if(CC::CSvrManager::shareInstance().checkServiceExist(_T("Alibaba Security Aegis Detect Service")))
					{
						errmsg = _T("Alibaba Security Aegis Detect Service 服务可能会导致更新失败，请先停止该服务再进行更新操作");
					}
				}
				if (!errmsg.empty())
				{
					::MessageBox(hDlg, (prestr+errmsg).c_str(),  MB_TITLE_TEXT, MB_OK);
					return FALSE;
				}
				errmsg = installFromRes(destQuerydPath, IDR_EXE_OSQUERY, _T("EXE"));
				if (!errmsg.empty())
				{
					if(CC::CSvrManager::shareInstance().checkServiceExist(_T("Alibaba Security Aegis Detect Service")))
					{
						errmsg = _T("Alibaba Security Aegis Detect Service 服务可能会导致更新失败，请先停止该服务再进行更新操作");
					}
				}
				if (!errmsg.empty())
				{
					::MessageBox(hDlg, (prestr+errmsg).c_str(),  MB_TITLE_TEXT, MB_OK);
					return FALSE;
				}
				errmsg = installFromRes(destEnvCfgYml, IDR_ENV_CFG, _T("TXT"));
				if (!errmsg.empty())
				{
					::MessageBox(hDlg, (prestr+errmsg).c_str(),  MB_TITLE_TEXT, MB_OK);
					return FALSE;
				}
				errmsg = installFromRes(destFileinfoCfgYml, IDR_FILEINFO_CFG, _T("TXT"));
				if (!errmsg.empty())
				{
					::MessageBox(hDlg, (prestr+errmsg).c_str(),  MB_TITLE_TEXT, MB_OK);
					return FALSE;
				}
				errmsg = installFromRes(destVersionTxt, IDR_VERSION_INFO, _T("TXT"));
				if (!errmsg.empty())
				{
					::MessageBox(hDlg, (prestr+errmsg).c_str(),  MB_TITLE_TEXT, MB_OK);
					return FALSE;
				}
				/*std::wfstream of;
				of.open(destVersionTxt.c_str(), std::ios::out);
				if (of)
				{
				of << versionFromRes.c_str();
				of.close();
				}*/

				if (type == ActionType_Install)
				{
					wstring cmdline = info.generateInstallCmdline(destInstallPath);

					UINT nret = runCmd(cmdline.c_str());
					if ( nret != 0)
					{
						if (nret == 1024)
						{
							std::wstring errmsg = getServiceErrText();
							
							if (errmsg.empty())
							{
								errmsg = _T("安装失败");
							}
							::MessageBoxEx(hDlg, errmsg.c_str(), MB_TITLE_TEXT, MB_OK|MB_ICONWARNING, 0);
						} else {
							::MessageBox(hDlg, CC::getLastErrorString(GetLastError(), _T("安装失败:")).c_str(), MB_TITLE_TEXT, MB_OK|MB_ICONWARNING);
						}
						return FALSE;
					}
				}


				CProgressDialog progressDlg(destInstallPath);
				progressDlg.m_bUpgrade = (type == ActionType_Upgrade);
				INT_PTR installRet = progressDlg.show(hDlg);
				if (installRet == IDCANCEL)
				{
					if (type == ActionType_Upgrade)
					{
						::MessageBox(hDlg, _T("更新失败"), MB_TITLE_TEXT, MB_OK|MB_ICONWARNING); 
					}
					return FALSE;

				} else if(installRet == 1) {
					if (type == ActionType_Upgrade)
					{
						::MessageBox(hDlg, _T("更新失败"), MB_TITLE_TEXT, MB_OK|MB_ICONWARNING); 
					}
					return FALSE;
				}
			}
			break;
		}


		return TRUE;
	}


	 void CMainCfgDialog::onInstall() 
	 {
		 if (!checkRequiredArgs())
		 {
			 return;
		 }
		
		 Button_Enable(GetDlgItem(hDlg, IDOK), FALSE);
		if(!action(ActionType_Install)){
			Button_Enable(GetDlgItem(hDlg, IDOK), TRUE);
			return;
		}

		Button_Enable(GetDlgItem(hDlg, IDOK), TRUE);

		CSvrManager::shareInstance().getServiceInfo();
		switchType(FOR_CONFIG);
		ShowWindow(hDlg, SW_HIDE);
		CTrayIcon::shareInstance().show();
		CTrayIcon::shareInstance().showTrayMsg(_T("探针已启动, 点击此处管理"), _T("安装成功"));
	}

	 void CMainCfgDialog::onUpgrade()
	 {
		 if (!checkRequiredArgs())
		 {
			 return;
		 }

		 if (::MessageBox(hDlg, _T("更新版本将重启服务, 是否继续?"), MB_TITLE_TEXT, MB_YESNOCANCEL) == IDYES)
		 {
			 CSvrManager::shareInstance().stop();
			 //CSvrManager::shareInstance().uninstall();
			 Sleep(1000);
			 if(!action(ActionType_Upgrade))
			 {
				 return;
			 }

			 CSvrManager::shareInstance().getServiceInfo();
			 switchType(FOR_CONFIG);
			 ShowWindow(hDlg, SW_HIDE);
			 CTrayIcon::shareInstance().show();
			 wstring msg = _T("已更新至最新版本");
			 msg += versionFromRes;
			 CTrayIcon::shareInstance().showTrayMsg(msg.c_str(), _T("更新成功"));
		 }
	 }

	 BOOL CMainCfgDialog::checkRequiredArgs()
	 {
		 ServiceArgsInfo info;
		 fillInfoFromEdits(info);
		 if (info.teamid.empty()
			 //|| info.assetid.empty()
			 || info.ak.empty()
			 || info.sk.empty())
		 {
			 ::MessageBox(hDlg, _T("带 * 的为必填项, 不能为空"), MB_TITLE_TEXT, MB_OKCANCEL);
			 return FALSE;
		 }

		 long port = wcstol(info.port.c_str(), nullptr, 10);
		 if (info.port.length()>5 || port <=0 || port > 65535)
		 {
			 ::MessageBox(hDlg, _T("无效的端口号"), MB_TITLE_TEXT, MB_OKCANCEL);
			 return FALSE;
		 }
		 return TRUE;
	 }


	 void CMainCfgDialog::onApplyChange()
	 {
		 if (!checkRequiredArgs())
		 {
			 return;
		 }

		 TCHAR portbuf[20]={0};
		 Edit_GetText(GetDlgItem(hDlg, IDC_EDIT_PORT), portbuf, _countof(portbuf));
		 std::wstring portstr= CC::trim(portbuf);
		 int newport = wcstol(portstr.c_str(), 0, 10);
		 if(portstr != CSvrManager::shareInstance().service.argsInfo.port && checkPortIsUsed(newport)) {
			 MessageBox(hDlg, _T("端口被占用"), MB_TITLE_TEXT, MB_OK|MB_ICONWARNING);
			 return;
		 }

		 int nret;
		 if ((nret = ::MessageBox(hDlg, _T("更改配置将重启服务, 是否继续?"), MB_TITLE_TEXT, MB_YESNOCANCEL)) == IDYES)
		 {
			 if(!CSvrManager::shareInstance().stop())
			 {
				 ::MessageBox(hDlg, CSvrManager::shareInstance().status.second.c_str(), MB_TITLE_TEXT, MB_OK);
				 return;
			 }
			 if(!action(ActionType_UpdateCfg))
			 {
				 //::MessageBox(hDlg, CSvrManager::shareInstance().status.second.c_str(), _T("Error"), MB_OK);
				 return;
			 }
			 
			 if (!CSvrManager::shareInstance().start())
			 {
				 ::MessageBox(hDlg, CSvrManager::shareInstance().status.second.c_str(), MB_TITLE_TEXT, MB_OK);
				 return;
			 }
			 ::MessageBox(hDlg, _T("更改完成, 服务已重启."), MB_TITLE_TEXT, MB_OKCANCEL);
		 }
	 }

	 void CMainCfgDialog::onCancel()
	 {
		 SendMessage(hDlg, WM_CLOSE,0,0);
	 }

	 void CMainCfgDialog::onClose()
	 {
		 if (type == FOR_CONFIG)
		 {
			 ShowWindow(hDlg, SW_HIDE);
		 } else {
			 DestroyWindow(hDlg);
		 }
	 }

	 void CMainCfgDialog::onDestroy()
	 {
		 if (type != FOR_CONFIG)
		 {
			 PostQuitMessage(0);
		 }
	 }

	 void CMainCfgDialog::fillInfoFromEdits(ServiceArgsInfo &info) 
	 {
		 TCHAR buf[1000]={0};

		 HWND hEditTeamID = GetDlgItem(hDlg, IDC_EDIT_TEAMID);
		 //HWND hEditAssetID = GetDlgItem(hDlg, IDC_EDIT_ASSETID);
		 HWND hEditAK = GetDlgItem(hDlg, IDC_EDIT_AK);
		 HWND hEditSK = GetDlgItem(hDlg, IDC_EDIT_SK);
		 HWND hEditIP = GetDlgItem(hDlg, IDC_EDIT_IP);
		 HWND hEditPort = GetDlgItem(hDlg, IDC_EDIT_PORT);

		 Edit_GetText(hEditTeamID, buf, _countof(buf));
		 info.teamid = CC::trim(buf);

		 //Edit_GetText(hEditAssetID, buf, _countof(buf));
		 //info.assetid = CC::trim(buf);

		 Edit_GetText(hEditAK, buf, _countof(buf));
		 info.ak = CC::trim(buf);

		 Edit_GetText(hEditSK, buf, _countof(buf));
		 info.sk = CC::trim(buf);

		 Edit_GetText(hEditIP, buf, _countof(buf));
		 info.ip = CC::trim(buf);

		 Edit_GetText(hEditPort, buf, _countof(buf));
		 info.port = CC::trim(buf);

		 HWND hEditRemoteHost = GetDlgItem(hDlg, IDC_EDIT_REMOTEHOST);
		 if (IsWindowVisible(hEditRemoteHost))
		 {
			 Edit_GetText(hEditRemoteHost, buf, _countof(buf));
			 info.remotehost = CC::trim(buf);
		 }
	 }

	 void CMainCfgDialog::fillEditsFromCfg(const ServiceArgsInfo &cfg)
	 {
		 HWND hEditTeamID = GetDlgItem(hDlg, IDC_EDIT_TEAMID);
		 HWND hEditAssetID = GetDlgItem(hDlg, IDC_EDIT_ASSETID);
		 HWND hEditAK = GetDlgItem(hDlg, IDC_EDIT_AK);
		 HWND hEditSK = GetDlgItem(hDlg, IDC_EDIT_SK);
		 HWND hEditIP = GetDlgItem(hDlg, IDC_EDIT_IP);
		 HWND hEditPort = GetDlgItem(hDlg, IDC_EDIT_PORT);
		 HWND hEditRemoteHost = GetDlgItem(hDlg, IDC_EDIT_REMOTEHOST);

		 Edit_SetText(hEditTeamID, cfg.teamid.c_str());
		 Edit_SetText(hEditAssetID, cfg.assetid.c_str());
		 Edit_SetText(hEditAK, cfg.ak.c_str());
		 Edit_SetText(hEditSK, cfg.sk.c_str());
		 if (cfg.ip != _T("default"))
		 {
			 Edit_SetText(hEditIP, cfg.ip.c_str());
		 }
		 Edit_SetText(hEditPort, cfg.port.c_str());
		 if (cfg.port.empty())
		 {
			 Edit_SetText(hEditPort, _T("9100"));
		 }
		 Edit_SetText(hEditRemoteHost, cfg.remotehost.c_str());
	 }

	 void CMainCfgDialog::onNewVersion()
	 {
		 CTrayIcon::shareInstance().showTrayMsg(_T("发现新版本"), _T(""));
	 }
};