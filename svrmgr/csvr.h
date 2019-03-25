#pragma once

#include <vector>
#include <string>

using std::wstring;
using std::vector;

extern wstring versionFromRes;

extern LPCTSTR ServiceName;
extern LPCTSTR ServiceDescription; 

namespace CloudCare {

	//serviceÆô¶¯²ÎÊý
	struct ServiceArgsInfo {
		wstring teamid;
		wstring assetid;
		wstring ak;
		wstring sk;
		wstring ip;
		wstring port;
		wstring enabled;
		wstring remotehost; //for test

		BOOL operator==(const ServiceArgsInfo & cfg) {
			if (cfg.teamid == teamid
				&& cfg.assetid == assetid
				&& cfg.ak == ak
				&& cfg.sk == sk
				&& cfg.ip == ip
				&& cfg.port == port
				&& cfg.remotehost == remotehost)
			{
				return TRUE;
			}
			return FALSE;
		}

		//wstring generateImagePath(const wstring &exepath);
		wstring generateInstallCmdline(const wstring &exepath);
		wstring generateUpdateCfgCmdline(const wstring &exepath);
		wstring generateGetDownloadURLCmdline(const wstring &exepath);

		BOOL parseFromYaml(LPCTSTR path);
	};


	class CSvr {
	private:
		wstring imagePath;
	public:
		CSvr()
			:hService(NULL)
		{
			state.dwCurrentState = SERVICE_STOPPED;
		}

		wstring exePath;
		SC_HANDLE hService;
		wstring name;
		wstring displayName;
		BOOL autoStart;
		SERVICE_STATUS state;
		std::wstring version;

		ServiceArgsInfo argsInfo;

		//void setImagePath(const wstring &path);
	};

	class CSvrManager {

	public:
		static CSvrManager & shareInstance() {
			static CSvrManager svr;
			return svr;
		}

		CSvrManager();
		~CSvrManager();

	private:
		SC_HANDLE hSCM;

	public:
		CSvr service;

	public:

		void uninit();

		BOOL checkServiceExist(LPCTSTR name);
		BOOL getServiceInfo();
		BOOL updateState();

		BOOL haveNotInstalled() const {
			if(service.hService == nullptr){
				return status.first == ERROR_SERVICE_DOES_NOT_EXIST;
			}
			return FALSE;
		}

		wstring getStatusString() const;

		BOOL connectSCM();
		BOOL install(const wstring &imagePath);
		BOOL start();
		BOOL stop();
		BOOL pause();
		BOOL resume();
		BOOL uninstall();

		BOOL updateImagePath(const wstring &imagePath);

		std::pair<DWORD, wstring> status;

	};

};

namespace CC = CloudCare;