#pragma once

#include <string>
#include <vector>

enum
{
	InstallType_Install = 0,
	InstallType_Upgrade
};

struct ArgumentItem
{
	ArgumentItem(LPCTSTR k, LPCTSTR lpdesc,BOOL must = TRUE)
		:key(k)
		,desc(lpdesc)
		,bmust(must)
	{

	}
	std::wstring key;
	std::wstring val;
	std::wstring desc;
	SIZE keyTextSize;
	BOOL bmust;
	RECT rcTtitle;
	RECT rcEdit;
	RECT rcEditWrapper;
	HWND hEdit;
	HWND hStaticDesc;
};


class CMainWindow
{
private:
	HFONT m_hTitleFont;
public:
	HWND m_hWnd;

	HWND m_radioCluster;
	HWND m_radioSingle;
	HWND m_staticCfg;
	HWND m_btnOk;
	HWND m_btnCancel;

	std::vector<ArgumentItem> m_args;

	int m_width;
	int m_height;

	int m_installType;

	CMainWindow(int w, int h)
		:m_hWnd(NULL)
		,m_hTitleFont(NULL)
		,m_radioSingle(NULL)
		,m_radioCluster(NULL)
		,m_width(w)
		,m_height(h)
		,m_installType(InstallType_Install)
	{

	}

	BOOL create(LPCTSTR name);
	void show(int nCmdShow)
	{
		::ShowWindow(m_hWnd, nCmdShow);
	}
	void update()
	{
		::UpdateWindow(m_hWnd);
	}
	void center();

	void updateEditDisp(int id);

	LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam);

	void onCreate();
	void onPaint(HDC hDC);
	void onCommand(int cmdID, int code);
	void onNotify(WPARAM wParam, LPARAM lParam);
};