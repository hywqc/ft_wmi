#pragma once

#include <string>
using std::wstring;

#define DEFAULT_LISTEN_PORT _T("9100")

namespace CloudCare
{
	struct ServiceArgsInfo;

	enum {
		FOR_INSTALL,
		FOR_CONFIG,
		FOR_UPGRADE,
	};

	enum ActionType {
		ActionType_UpdateCfg = 0,
		ActionType_Install,
		ActionType_Upgrade,
	};
	

	class CMainCfgDialog
	{
	private:
		HWND hDlg;
		int type;
	public:
		CMainCfgDialog(int typ)
			:type(typ)
			,hDlg(NULL)
		{}
		BOOL show(BOOL hide=FALSE);
		HWND getDlgWnd() const
		{
			return hDlg;
		}

		void switchType(int typ);

	protected:
		static INT_PTR CALLBACK dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

		INT_PTR onInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
		INT_PTR onCommond(WPARAM wParam, LPARAM lParam);

		BOOL action(ActionType type);

		void onInstall(); 
		void onClose();
		void onDestroy();
		void onCancel();
		void onApplyChange();
		void onUpgrade();
		void onNewVersion();


		void fillInfoFromEdits(ServiceArgsInfo &info);
		void fillEditsFromCfg(const ServiceArgsInfo &cfg);

		BOOL checkRequiredArgs();

	};

};

extern CloudCare::CMainCfgDialog *pCfgDialog;
