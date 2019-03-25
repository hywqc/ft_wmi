#pragma once

#include <string>
using std::wstring;

namespace CloudCare
{

	struct installCfg {
		wstring imagePath;
	};

	class CProgressDialog
	{
		friend DWORD WINAPI routine(LPVOID param);

	private:
		installCfg cfg;
	public:

		CProgressDialog(wstring path)
			:m_bUpgrade(FALSE)
		{
			cfg.imagePath = path;
		}

		INT_PTR show(HWND hParent);		
		void setPos(int pos, LPCTSTR status=NULL);

		BOOL m_bUpgrade;
	private:
		HWND hDlgWnd;
		HWND hCancelBtn;
		HWND hProgressCtrl;
		HWND hStateCtl;
	protected:
		static INT_PTR CALLBACK dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

		INT_PTR onInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
		void onStartInstall(WPARAM wParam, LPARAM lParam);
		void onUpdateState(WPARAM wParam, LPARAM lParam);
	};
};