#include "stdafx.h"
#include "cmetricsel.h"
#include <sstream>
#include <vector>
#include "resource.h"
#include <CommCtrl.h>
#include <WindowsX.h>

namespace CloudCare {

	MetricItem MetricItemList[] = {
		MetricItem(_T("ad"), _T("Active Directory Domain Services")),
		MetricItem(_T("cpu"), _T("CPU usage")),
		MetricItem(_T("cs"), _T("\"Computer System\" metrics (system properties, num cpus/total memory)")),
		MetricItem(_T("dns"), _T("DNS Server")),
		MetricItem(_T("hyperv"), _T("Hyper-V hosts")),
		MetricItem(_T("iis"), _T("IIS sites and applications")),
		MetricItem(_T("logical_disk"), _T("Logical disks, disk I/O")),
		MetricItem(_T("memory"), _T("Memory usage metrics")),
		MetricItem(_T("msmq"), _T("MSMQ queues")),
		MetricItem(_T("mssql"), _T("SQL Server Performance Objects metrics")),
		MetricItem(_T("netframework_clrexceptions"), _T(".NET Framework CLR Exceptions")),
		MetricItem(_T("netframework_clrinterop"), _T(".NET Framework Interop Metrics")),
		MetricItem(_T("netframework_clrjit"), _T(".NET Framework JIT metrics")),
		MetricItem(_T("netframework_clrloading"), _T(".NET Framework CLR Loading metrics")),
		MetricItem(_T("netframework_clrlocksandthreads"), _T(".NET Framework locks and metrics threads")),
		MetricItem(_T("netframework_clrmemory"), _T(".NET Framework Memory metrics")),
		MetricItem(_T("netframework_clrremoting"), _T(".NET Framework Remoting metrics")),
		MetricItem(_T("netframework_clrsecurity"), _T(".NET Framework Security Check metrics")),
		MetricItem(_T("net"), _T("Network interface I/O")),
		MetricItem(_T("os"), _T("OS metrics (memory, processes, users)")),
		MetricItem(_T("process"), _T("Per-process metrics")),
		MetricItem(_T("service"), _T("Service state metrics")),
		MetricItem(_T("system"), _T("	System calls")),
		MetricItem(_T("tcp"), _T("TCP connections")),
		//MetricItem(_T("textfile"), _T("Read prometheus metrics from a text file"), FALSE),
		MetricItem(_T("vmware"), _T("Performance counters installed by the Vmware Guest agent")),
		MetricItem(_T("null"), _T("")),
	};

	wstring enabledMetricsStringValue() {
		wstring result;
		int index = 0;
		std::vector<UINT32> vals;
		UINT32 n = 0;
		while(TRUE) {
			MetricItem item = MetricItemList[index];
			if (item.name == _T("null"))
			{
				break;
			}

			if (item.enabled)
			{
				n |= (1 << (index % 32));
			}
			if ((index % 32) == 31)
			{
				vals.push_back(n);
				n = 0;
			}
			index++;
		}

		if (vals.size() == 0)
		{
			vals.push_back(n);
		}

		std::wostringstream ss;
		ss << std::hex;
		for (std::vector<UINT32>::iterator it=vals.begin(); it!=vals.end();it++)
		{
			if (it == vals.begin())
			{
				ss << *it;
			} else {
				ss << _T("-") << *it;
			}
		}

		return ss.str();
	}

	INT_PTR CALLBACK CMetricSelect::metricsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{

		CMetricSelect *ms = (CMetricSelect *)GetWindowLongPtr(hDlg, DWLP_USER);
		
		switch(message) {
		case WM_COMMAND:
			if (ms)
			{
				return ms->onCommond(wParam, lParam);
			}
			break;
		case WM_NOTIFY:
			if (ms)
			{
				return ms->onNotify(wParam, lParam);
			}
			break;
		case  WM_INITDIALOG:
			{
				CMetricSelect *ms = (CMetricSelect *)lParam;
				SetWindowLongPtr(hDlg, DWLP_USER,(LONG)ms);
				return ms->onInitDialog(hDlg, wParam, lParam);
			}
			return (INT_PTR)TRUE;
			break;
		case WM_CLOSE:
			if (ms)
			{
				ms->onClose();
				return TRUE;
			}
			break;
		default:
			break;
		}
		return (INT_PTR)FALSE;
	}

	INT_PTR CMetricSelect::show(HWND hParent)
	{
		return DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_DIALOG_METRICS), hParent, CMetricSelect::metricsDlgProc, (LPARAM)this);
	}

	INT_PTR CMetricSelect::onInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam) {

		hDlgWnd = hDlg;

		SetWindowText(hDlgWnd, _T("¿ªÆô/¹Ø±Õ¼ì²âÏî"));

		hSelectAllBtn = GetDlgItem(hDlgWnd, IDC_CHECK_SELECTALL);
		hListView = GetDlgItem(hDlgWnd, IDC_LIST_METRICS);

		BOOL selectAll = TRUE;
		if (hListView)
		{
			ListView_SetExtendedListViewStyle(hListView,LVS_EX_GRIDLINES|LVS_EX_CHECKBOXES);

			LVCOLUMN lvc={0};
			lvc.mask = LVCF_FMT|LVCF_WIDTH|LVCF_TEXT;
			lvc.cx = 180;
			lvc.fmt = LVCFMT_LEFT;
			lvc.pszText = _T("Ãû³Æ");
			ListView_InsertColumn(hListView,0,&lvc);
			lvc.fmt = LVCFMT_CENTER;
			lvc.cx = 220;
			lvc.pszText = _T("ÃèÊö");
			ListView_InsertColumn(hListView,1,&lvc);

			//HIMAGELIST hImageList = ImageList_Create(1,30,ILC_COLOR32,1,0);
			//ListView_SetImageList(hListView, hImageList,LVSIL_SMALL);

			int index = 0;

			while (TRUE)
			{
				MetricItem item = MetricItemList[index];
				if (item.name == _T("null"))
				{
					break;
				}

				LVITEM lvi = {0};
				lvi.iItem = index;
				lvi.mask = LVIF_TEXT;
				lvi.pszText = const_cast<LPWSTR>(item.name.c_str());
				ListView_InsertItem(hListView, &lvi);

				ListView_SetItemText(hListView,index,1, const_cast<LPWSTR>(item.desc.c_str()));

				BOOL bEnabled = item.enabled;

				ListView_SetCheckState(hListView,index, bEnabled);

				if (!bEnabled && selectAll)
				{
					selectAll = FALSE;
				}

				index++;
			}

		}

		Button_SetCheck(hSelectAllBtn, selectAll);

		return (INT_PTR)TRUE;
	}

	INT_PTR CMetricSelect::onCommond(WPARAM wParam, LPARAM lParam) {
		switch(LOWORD(wParam)) {
		case  IDCANCEL:
			onCancel();
			return TRUE;
			break;
		case IDOK:
			onOK();
			return TRUE;
			break;
		case IDC_CHECK_SELECTALL:
			onSelectAll();
			return TRUE;
			break;
		default:
			break;
		}
		return (INT_PTR)FALSE;
	}

	INT_PTR CMetricSelect::onNotify(WPARAM wParam, LPARAM lParam)
	{
		NMHDR *phdr = (NMHDR *)lParam;
		switch(phdr->code) {
		case LVN_ITEMCHANGED:
			{
				LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
				BOOL bcheck = ListView_GetCheckState(hListView, pnmv->iItem);
				int nc = ListView_GetItemCount(hListView);

				BOOL selall = TRUE;
				for (int i=0;i<nc;i++)
				{
					if(!ListView_GetCheckState(hListView, i)) {
						selall = FALSE;
						break;
					}
				}

				Button_SetCheck(hSelectAllBtn, selall);
				return TRUE;
			}
			break;
		default:
			break;
		}
		return (INT_PTR)FALSE;
	}


	void CMetricSelect::onCancel()
	{
		SendMessage(hDlgWnd, WM_CLOSE,0,0);
	}

	void CMetricSelect::onOK()
	{
		int ncount = ListView_GetItemCount(hListView);
		for (int i=0;i<ncount;i++)
		{
			MetricItemList[i].enabled = ListView_GetCheckState(hListView, i);
		}
		SendMessage(hDlgWnd, WM_CLOSE,0,0);
	}

	void CMetricSelect::onSelectAll()
	{
		BOOL bSelAll = Button_GetCheck(hSelectAllBtn);
		int nc = ListView_GetItemCount(hListView);
		for (int i=0;i<nc;i++)
		{
			ListView_SetCheckState(hListView,i,bSelAll);
		}
	}

	void CMetricSelect::onClose()
	{
		EndDialog(hDlgWnd, 0);
	}

};