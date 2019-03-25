#pragma once

#include <string>
#include <vector>

using std::wstring;

namespace CloudCare
{
	struct MetricItem {
		wstring name;
		wstring desc;
		BOOL enabled;

		MetricItem(wstring n, wstring d, BOOL e=TRUE):name(n),desc(d),enabled(e) {

		}
	};


	extern MetricItem MetricItemList[];
	wstring enabledMetricsStringValue();

	class CMetricSelect {

	public:
		CMetricSelect()
			:hDlgWnd(NULL)
			,hSelectAllBtn(NULL)
			,hListView(NULL)
		{
		}

	private:
		HWND hDlgWnd;
		HWND hSelectAllBtn;
		HWND hListView;


	protected:
		static INT_PTR CALLBACK metricsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

		INT_PTR onInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
		INT_PTR onCommond(WPARAM wParam, LPARAM lParam);
		INT_PTR onNotify(WPARAM wParam, LPARAM lParam);
		void onOK();
		void onCancel();
		void onSelectAll();
		void onClose();

	public:
		INT_PTR show(HWND hParent);
	};
};
