#include "stdafx.h"
#include "VersionDialog.h"
#include "util.h"
#include "resource.h"
#include <ShellAPI.h>
#include "csvr.h"
#include <fstream>
#include <strsafe.h>

namespace CloudCare
{

	INT_PTR CVersionDialog::onInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
	{
		SetWindowText(hDlg, MB_TITLE_TEXT);
		hDlgWnd = hDlg;
		SetWindowLongPtr(hDlg, DWLP_USER,(LONG)this);

		hDownloadBtn = GetDlgItem(hDlg, IDC_DOWNLOAD_NEWVER);
		hCurrentVer = GetDlgItem(hDlg, IDC_CURRENT_VER);
		hNewVer = GetDlgItem(hDlg, IDC_NEW_VERSION);

		SetWindowText(hCurrentVer, m_curVer.c_str());
		SetWindowText(hNewVer, m_newVer.c_str());

		return FALSE;
	}

	INT_PTR CVersionDialog::show(HWND hParent)
	{
		return DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_VERSION_CHECK), hParent, CVersionDialog::dlgProc,(LPARAM)this);
	}

	INT_PTR CALLBACK CVersionDialog::dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
	{
		CVersionDialog *pd = (CVersionDialog *)GetWindowLongPtr(hDlg, DWLP_USER);

		switch(message) {
		case WM_COMMAND:
			{
				switch(LOWORD(wParam)) {
				case  IDC_DOWNLOAD_NEWVER:
					{

						EnableWindow(pd->hDownloadBtn, FALSE);

						wstring cmdline = CSvrManager::shareInstance().service.argsInfo.generateGetDownloadURLCmdline(getTargetExePath());
	
						UINT nret = runCmd(cmdline.c_str());

						TCHAR durl[1000]={0};
						StringCchPrintf(durl, _countof(durl),  _T("http://%s/ft_wmi_exporter/test/ft_wmi_exporter-%s.exe"), getOssBucket().c_str(), pd->m_newVer.c_str());

						//std::wstring durl = "http://" + getOssBucket() + _T("/ft_wmi_exporter/test/ft_wmi_exporter-") + m_newVer + _T(".exe");
						//std::wstring filepath = getTargetInstallDir()+ _T("\\dowload_url");
						//std::ifstream inf(wide2string(filepath).c_str(), std::ios::in|std::ios::binary);
						//if (inf)
						//{
						//	inf.seekg(0, std::ios::end);
						//	int len = (int)inf.tellg();
						//	inf.seekg(std::ios::beg);
						//	char *pbuf = new char[len+1];
						//	ZeroMemory(pbuf, len+1);
						//	inf.read(pbuf, len);
						//	std::string sv(pbuf);
						//	delete [] pbuf;
						//	inf.close();
						//	durl = string2wstring(sv);
						//	DeleteFile(filepath.c_str());
						//}

						if (durl[0] == _T('\0'))
						{
							EnableWindow(pd->hDownloadBtn, TRUE);
							MessageBox(NULL, _T("获取下载地址失败"), MB_TITLE_TEXT, MB_OK|MB_ICONWARNING);
							return FALSE;
						}

						//MessageBox(NULL, durl.c_str(), MB_TITLE_TEXT, MB_OK);

						ShellExecute(nullptr, _T("open"), durl, NULL, NULL, SW_SHOWNORMAL);
						EndDialog(hDlg, 0);
					}
					break;
				default:
					break;
				}
			}
			break;
		case  WM_INITDIALOG:
			{
				CVersionDialog *dd = (CVersionDialog *)lParam;
				dd->onInitDialog(hDlg, wParam, lParam);
				return TRUE;
			}
			break;
		case WM_CLOSE:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
			break;
		default: 
			break;
		}

		return (INT_PTR)FALSE;

	}
};