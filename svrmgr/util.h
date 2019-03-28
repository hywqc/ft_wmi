#pragma once

#include <string>
#include <vector>
using std::wstring;

extern wstring versionFromRes;
extern wstring remoteVersion;
extern BOOL TestEdition;
extern HWND hMainDlg;
extern LPCTSTR AppName;
extern wstring gRemoteHostUrl;


enum EditionType {
	EditionType_Test = 0,
	EditionType_Propord,
	EditionType_Release,
};

#define WM_DETECT_NEW_VERSION (WM_USER+1)

#define MB_TITLE_TEXT _T("FT WMI Exporter")

namespace CloudCare {

	wstring getLastErrorString(DWORD dwErr, LPCTSTR prefix);

	std::vector<wstring> splitString(const wstring &str, const wstring &splitter);
	wstring ltrim(LPCTSTR str);
	wstring rtrim(LPCTSTR str);
	wstring trim(LPCTSTR str);
	wstring getFlagValueFromPath(const wstring &str, const wstring &flag);

	wstring getAbsExePath(LPCTSTR exename);
	wstring getTargetInstallDir();
	wstring getTempDir();

	wstring getTargetExePath();
	wstring getTargetYamPath();
	wstring getTargetVersionPath();
	wstring getTargetVersionPath();

	wstring installFromRes(wstring dest, UINT resid, LPCTSTR  type);

	wstring  string2wstring(const std::string &str);
	std::string  wide2string(const std::wstring &str);

	std::wstring xorDecode(const char *pdata, size_t len);

	UINT runCmd(LPCTSTR cmdline);

	std::string loadVersionRes();

	std::wstring parseVersion(const std::string & str);

	void startCheckVersion();

	BOOL checkVersion(std::string &response);

	std::wstring getOssBucket();

	std::wstring getServiceErrText();

	BOOL checkPortIsUsed(int port);

	//BOOL checkProbeAvaiable();

};


