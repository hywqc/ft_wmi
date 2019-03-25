#pragma once

#include <string>

namespace CloudCare
{
	class CVersionDialog
	{
	public:
		CVersionDialog(const std::wstring &curver, const std::wstring &newver)
			:m_curVer(curver)
			,m_newVer(newver)
		{

		}

		INT_PTR show(HWND hParent);

	public:

		std::wstring m_curVer;
		std::wstring m_newVer;

		HWND hDlgWnd;
		HWND hDownloadBtn;
		HWND hCurrentVer;
		HWND hNewVer;
	protected:
		static INT_PTR CALLBACK dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

		INT_PTR onInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
	};
};