#include "stdafx.h"
#include "csvr.h"
#include <WinSvc.h>
#include <Shlwapi.h>
#include "util.h"
#include <strsafe.h>
#include "cmetricsel.h"
#include <WinReg.h>
#include <fstream>
#define YAML_DECLARE_STATIC
#include "yaml.h"


extern EditionType gEditionType;


namespace CloudCare {

	/*wstring ServiceArgsInfo::generateImagePath(const wstring &exepath)
	{
	wstring path = exepath;
	path += (_T(" --team-id ") + teamid);
	path += (_T(" --uploader-uid ") + assetid);
	path += (_T(" --ak ") + ak);
	path += (_T(" --sk ") + sk);
	if (!ip.empty())
	{
	path += (_T(" --host ") + ip);
	}
	if (!port.empty())
	{
	path += (_T(" --port ") + port);
	}
	if (!remotehost.empty())
	{
	path += (_T(" --remote-host ") + remotehost);
	}
	path += (_T(" --enabled ") + enabledMetricsStringValue());

	return path;
	} */

	wstring ServiceArgsInfo::generateInstallCmdline(const wstring &exepath)
	{
		wstring line = exepath;
		line += _T(" --init ");
		line += (_T(" --team-id ") + teamid);
		//path += (_T(" --uploader-uid ") + assetid);
		line += (_T(" --ak ") + ak);
		line += (_T(" --sk ") + sk);
		if (!ip.empty())
		{
			line += (_T(" --host ") + ip);
		}
		if (!port.empty())
		{
			line += (_T(" --port ") + port);
		}
		switch(gEditionType)
		{
		case EditionType_Test:
			line += _T(" --remote-host http://testing.kodo.cloudcare.cn:10401 ");
			break;
		case EditionType_Propord:
			line += _T(" --remote-host https://preprod-kodo.cloudcare.cn ");
			break;
		case EditionType_Release:
			line += _T(" --remote-host https://kodo.cloudcare.cn ");
			break;
		default:
			break;
		}

		//if (!remotehost.empty())
		//{
		//	line += (_T(" --remote-host ") + remotehost);
		//}
		line += (_T(" --enabled ") + enabledMetricsStringValue());

		return line;
	} 

	wstring ServiceArgsInfo::generateGetDownloadURLCmdline(const wstring &exepath)
	{
		wstring line = exepath;
		line += _T(" --get-download-url");

		return line;
	} 

	wstring ServiceArgsInfo::generateUpdateCfgCmdline(const wstring &exepath)
	{
		wstring line = exepath;
		line += _T(" --update-cfg ");
		line += (_T(" --team-id ") + teamid);
		//path += (_T(" --uploader-uid ") + assetid);
		line += (_T(" --ak ") + ak);
		line += (_T(" --sk ") + sk);
		if (!ip.empty())
		{
			line += (_T(" --host ") + ip);
		}
		if (!port.empty())
		{
			line += (_T(" --port ") + port);
		}
		//if (!remotehost.empty())
		//{
		//	line += (_T(" --remote-host ") + remotehost);
		//}
		line += (_T(" --enabled ") + enabledMetricsStringValue());

		return line;
	}

	BOOL ServiceArgsInfo::parseFromYaml(LPCTSTR path)
	{
		yaml_parser_t parser;
		yaml_token_t token;

		FILE *pf = fopen(wide2string(path).c_str(), "r");
		if (!pf)
		{
			return FALSE;
		}
		char currentKey[100] = {0};

		int mapindex = 0;
		int keyindex = 0;

		char cbuf[1000] = {0};

		int state = 0;
		if(!yaml_parser_initialize(&parser))
		{
			return FALSE;
		}

		yaml_parser_set_input_file(&parser, pf);

		do 
		{
			if(yaml_parser_scan(&parser, &token)==0)
			{
				break;
			}
			switch(token.type)
			{
			case YAML_STREAM_START_TOKEN:
				//OutputDebugString(_T("Stream Start\n"));
				break;
			case YAML_STREAM_END_TOKEN:
				//OutputDebugString(_T("Stream End\n"));
				break;
			case YAML_BLOCK_MAPPING_START_TOKEN:
				//OutputDebugString(_T("[Block mapping]\n"));
				mapindex++;
				keyindex=0;
				break;
			//case YAML_BLOCK_SEQUENCE_START_TOKEN:
			//	OutputDebugString(_T("Start Block (Sequence)\n"));
			//	break;
			case YAML_BLOCK_ENTRY_TOKEN:
				//OutputDebugString(_T("Start Block (Entry)\n"));
				break;
			case YAML_BLOCK_END_TOKEN:
				//OutputDebugString(_T("End Block\n"));
				break;
			case YAML_KEY_TOKEN:
				state = 0;
				keyindex++;
				break;
			case YAML_VALUE_TOKEN:
				state = 1;
				break;
			case YAML_SCALAR_TOKEN:
				if (state == 0)
				{
					strcpy_s(currentKey, sizeof(currentKey), (char *)token.data.scalar.value);
				} else {

					if (mapindex == 1)
					{
						if (StrCmpNA(currentKey, "team_id", strlen(currentKey)) == 0)
						{
							teamid = string2wstring((char *)token.data.scalar.value);
						} else if (StrCmpNA(currentKey, "ak", strlen(currentKey)) == 0) {
							ak = string2wstring((char *)token.data.scalar.value); 
						} else if (StrCmpNA(currentKey, "sk", strlen(currentKey)) == 0) {
							char ensk[1000] = {0};
							strcpy_s(ensk, sizeof(ensk), (char *)token.data.scalar.value);
							sk = xorDecode((const char *)ensk, strlen(ensk));

						} else if (StrCmpNA(currentKey, "host", strlen(currentKey)) == 0) {
							ip = string2wstring((char *)token.data.scalar.value); 
						} else if (StrCmpNA(currentKey, "port", strlen(currentKey)) == 0) {
							port = string2wstring((char *)token.data.scalar.value); 
						} else if (StrCmpNA(currentKey, "remote_host", strlen(currentKey)) == 0) {
							remotehost = string2wstring((char *)token.data.scalar.value); 
						}
					} else if (mapindex == 2) {
						std::wstring enabled = string2wstring((char *)token.data.scalar.value);

						std::wstring ck = string2wstring(currentKey);
						
						int index = 0;
						while(TRUE)
						{
							MetricItem & item = MetricItemList[index];
							if (item.name == ck)
							{
								item.enabled = (enabled == _T("true"));
								break;
							}
							index++;
						}
					}
					
				}
				break;
			}

			if (token.type != YAML_STREAM_END_TOKEN)
			{
				yaml_token_delete(&token);
			}
		} while (token.type != YAML_STREAM_END_TOKEN);

		yaml_token_delete(&token);

		yaml_parser_delete(&parser);
		fclose(pf);

		return TRUE;
	}


	/*void CSvr::setImagePath(const wstring &path)
	{

	std::size_t nfind = path.find(_T("\\corsair.exe"));
	if (nfind != wstring::npos)
	{
	exePath = path.substr(0, nfind+lstrlen(_T("\\corsair.exe")));
	}

	imagePath = path;
	argsInfo.teamid = getFlagValueFromPath(path, _T("--team-id"));
	argsInfo.assetid = getFlagValueFromPath(path, _T("--uploader-uid"));
	argsInfo.ak = getFlagValueFromPath(path, _T("--ak"));
	argsInfo.sk = getFlagValueFromPath(path, _T("--sk"));
	argsInfo.ip = getFlagValueFromPath(path, _T("--host"));
	argsInfo.port = getFlagValueFromPath(path, _T("--port"));
	argsInfo.enabled = getFlagValueFromPath(path, _T("--enabled"));
	argsInfo.remotehost = getFlagValueFromPath(path, _T("--remote-host"));

	std::vector<wstring> enable_parts = splitString(argsInfo.enabled, _T("-"));
	std::vector<UINT32> enable_bits;
	for (size_t i=0; i<enable_parts.size(); i++)
	{
	UINT32 n = (UINT32)wcstol(enable_parts[i].c_str(), NULL, 16);
	enable_bits.push_back(n);
	}

	int index = 0;
	while(TRUE)
	{
	MetricItem & item = MetricItemList[index];
	if (item.name == _T("null"))
	{
	break;
	}

	size_t n = (size_t)index / 32;
	UINT32 u = enable_bits[n];

	UINT32 cmp = (1 << (index % 32));

	item.enabled = ((u & cmp) == cmp);

	index++;
	}
	}*/

	CSvrManager::CSvrManager():hSCM(nullptr), status(std::pair<DWORD, wstring>(0, _T(""))) {

	}


	CSvrManager::~CSvrManager() {
		if (hSCM != nullptr)
		{
			CloseServiceHandle(hSCM);
		}
	}

	BOOL CSvrManager::connectSCM() {
		hSCM = OpenSCManager(NULL,  SERVICES_ACTIVE_DATABASE,  SC_MANAGER_ALL_ACCESS );
		if (hSCM == nullptr)
		{
			DWORD dwErr = GetLastError();
			wstring errMsg;
			if (dwErr == ERROR_ACCESS_DENIED)
			{
				errMsg = _T("访问被拒绝，请以管理员身份运行");
			} else {
				errMsg = getLastErrorString(dwErr, _T("[error] Fail to connect SCM:"));
			}
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}
		status = std::pair<DWORD, wstring>(0, _T(""));
		return TRUE;
	}


	BOOL CSvrManager::checkServiceExist(LPCTSTR name)
	{
		SC_HANDLE hsvr = OpenService(hSCM, name, SERVICE_QUERY_CONFIG);
		BOOL bhave = (hsvr != NULL);
		if (hsvr)
		{
			CloseServiceHandle(hsvr);
		}
		return bhave;
	}

	BOOL CSvrManager::getServiceInfo()
	{

		if (!hSCM)
		{
			return FALSE;
		}

		if (service.name.empty())
		{
			service.name = ServiceName;
			service.displayName = ServiceName;
			service.autoStart = TRUE;
		}

		if (!service.hService)
		{
			service.hService = OpenService(hSCM, service.name.c_str(), SERVICE_ALL_ACCESS);
			if (service.hService == nullptr)
			{
				DWORD dwErr = GetLastError();
				wstring errMsg = getLastErrorString(dwErr, _T("[error] OpenService fail:"));
				status = make_pair(dwErr, errMsg);
				return FALSE;
			}
		}

		LPQUERY_SERVICE_CONFIG lpsc = nullptr;
		DWORD dwBytesNeeded, cbBufSize, dwError;
		if(!QueryServiceConfig(service.hService, NULL, 0, &dwBytesNeeded))
		{
			dwError = GetLastError();
			if (ERROR_INSUFFICIENT_BUFFER == dwError)
			{
				cbBufSize = dwBytesNeeded;
				lpsc = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_FIXED, cbBufSize);
			} else {
				wstring errMsg = getLastErrorString(dwError, _T("[error] Query service config fail:"));
				status = make_pair(dwError, errMsg);
				return FALSE;
			}
		}

		if(!QueryServiceConfig(service.hService, lpsc, cbBufSize, &dwBytesNeeded)) {
			DWORD dwErr = GetLastError();
			wstring errMsg = getLastErrorString(dwErr, _T("[error] Query service config fail:"));
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}

		service.argsInfo.parseFromYaml(getTargetYamPath().c_str());
		service.exePath = getTargetExePath();
		service.displayName = lpsc->lpDisplayName;
		service.autoStart = (lpsc->dwStartType == SERVICE_AUTO_START);
		LocalFree(lpsc);

		if(!QueryServiceStatus(service.hService, &service.state)) {
			DWORD dwErr = GetLastError();
			wstring errMsg = getLastErrorString(dwErr, _T("[error] Query service status fail:"));
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}

		wstring versionFile = CC::getTargetVersionPath();
		if (PathFileExists(versionFile.c_str()))
		{
			std::ifstream inf;
			inf.open(wide2string(versionFile).c_str(), std::ios::in|std::ios::binary);
			if (inf)
			{
				inf.seekg(0, std::ios::end);
				int len = (int)inf.tellg();
				inf.seekg(std::ios::beg);
				char *pbuf = new char[len+1];
				ZeroMemory(pbuf, len+1);
				inf.read(pbuf, len);
				std::string sv(pbuf);
				delete [] pbuf;
				inf.close();
				service.version = parseVersion(sv);
			}
		}


// 		HKEY hregkey = NULL;
// 		LSTATUS lstatus = RegOpenKey(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\services\\corsair"), &hregkey);
// 		if (lstatus == ERROR_SUCCESS)
// 		{
// 			TCHAR buf[100]={0};
// 			DWORD dwsize = sizeof(buf);
// 			RegGetValue(hregkey, NULL, _T("Version"), RRF_RT_ANY, NULL, (PVOID)buf, &dwsize);
// 			RegCloseKey(hregkey);
// 			service.version = buf;
// 		}

		status = std::pair<DWORD, wstring>(0, _T(""));
		return TRUE;
	}

	BOOL CSvrManager::updateState()
	{
		if(!QueryServiceStatus(service.hService, &service.state)) {
			DWORD dwErr = GetLastError();
			wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}

		status = std::pair<DWORD, wstring>(0, _T(""));
		return TRUE;
	}

	void CSvrManager::uninit() {
		if (hSCM)
		{
			CloseServiceHandle(hSCM);
			hSCM = nullptr;
		}
	}

	wstring CSvrManager::getStatusString() const {
		switch(service.state.dwCurrentState) {
		case SERVICE_STOPPED:
			return _T("停止");
			break;
		case SERVICE_RUNNING:
			return _T("正在运行");
			break;
		case SERVICE_PAUSED:
			return _T("暂停");
			break;
		default:
			return _T("未知");
			break;
		}
	}

	
	BOOL CSvrManager::install(const wstring &imagePath) {
		if (!hSCM)
		{
			return FALSE;
		}

		SC_HANDLE scHandler = CreateService(hSCM, service.name.c_str(),  service.displayName.c_str(), SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,  SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, imagePath.c_str(), NULL, NULL, NULL, NULL, NULL);
		if (!scHandler)
		{
			DWORD dwErr = GetLastError();
			wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}
		service.hService = scHandler;

		SERVICE_DESCRIPTION sdec;
		sdec.lpDescription = const_cast<LPTSTR>(ServiceDescription);
		ChangeServiceConfig2(scHandler, SERVICE_CONFIG_DESCRIPTION, &sdec);

		//HKEY hkey = NULL;
		//LSTATUS lret = RegOpenKey(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\services\\corsair"), &hkey);
		//if (hkey)
		//{
		//	DWORD bufsize=(versionFromRes.length()+1)*sizeof(TCHAR);
		//	lret = RegSetKeyValue(hkey, NULL, _T("Version"), REG_SZ, versionFromRes.c_str(), bufsize);
		//	RegCloseKey(hkey);
		//}

		status = std::pair<DWORD, wstring>(0, _T(""));
		return TRUE;
	}


	BOOL CSvrManager::start()
	{
		SERVICE_STATUS_PROCESS ssStatus;
		DWORD dwBytesNeeded ;
		DWORD dwStartTickCount;
		DWORD dwOldCheckPoint;
		DWORD dwWaitTime;
		DWORD dwTimeOut;

		if(!QueryServiceStatusEx(service.hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
		{
			DWORD dwErr = GetLastError();
			wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}

		if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
		{
			status.first = 1;
			status.second = _T("Cannot start the service because it is already running");
			return FALSE;
		}

		dwStartTickCount  = GetTickCount();
		dwOldCheckPoint = ssStatus.dwCheckPoint;

		//如果服务正在停止，则等待直到服务停止
		while(ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
		{
			dwWaitTime = ssStatus.dwWaitHint / 10;
			if (dwWaitTime < 1000)
			{
				dwWaitTime = 1000;
			}
			if (dwWaitTime > 10000)
			{
				dwWaitTime = 10000;
			}

			Sleep(dwWaitTime);

			if(!QueryServiceStatusEx(service.hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
			{
				DWORD dwErr = GetLastError();
				wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
				status = make_pair(dwErr, errMsg);
				return FALSE;
			}

			if (ssStatus.dwCheckPoint > dwOldCheckPoint)
			{
				dwStartTickCount = GetTickCount();
				dwOldCheckPoint = ssStatus.dwCheckPoint;

			} else {
				if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
				{
					status.first = 1;
					status.second = _T("Timeout waiting for service to stop");
					return FALSE;
				}
			}
		}


		if(!StartService(service.hService, 0, NULL)) {
			DWORD dwErr = GetLastError();
			wstring errMsg = getServiceErrText();
			if (errMsg.empty())
			{
				errMsg = getLastErrorString(dwErr, _T("[error] StartService fail:"));
			}
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}

		if(!QueryServiceStatusEx(service.hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
		{
			DWORD dwErr = GetLastError();
			wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}

		if (ssStatus.dwCurrentState == SERVICE_RUNNING)
		{
			Sleep(500);
			status = std::pair<DWORD, wstring>(0, _T(""));
			return TRUE;
		}

		dwStartTickCount = GetTickCount();
		dwOldCheckPoint = ssStatus.dwCheckPoint;

		dwTimeOut = ssStatus.dwWaitHint;
		if (dwTimeOut == 0)
		{
			dwTimeOut = 30000;
		}

		while(ssStatus.dwCurrentState == SERVICE_START_PENDING)
		{
			dwWaitTime = ssStatus.dwWaitHint / 10;
			if (dwWaitTime < 1000)
			{
				dwWaitTime = 1000;
			}
			if (dwWaitTime > 10000)
			{
				dwWaitTime = 10000;
			}

			Sleep(dwWaitTime);

			if(!QueryServiceStatusEx(service.hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
			{
				DWORD dwErr = GetLastError();
				wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
				status = make_pair(dwErr, errMsg);
				return FALSE;
			}

			if (ssStatus.dwCheckPoint > dwOldCheckPoint)
			{
				dwStartTickCount = GetTickCount();
				dwOldCheckPoint = ssStatus.dwCheckPoint;
			} else {

				if (GetTickCount() - dwStartTickCount > dwTimeOut)
				{
					status.first = 1;
					status.second = _T("Timeout waiting for service to start");
					return FALSE;
				}
			}
		}

		if (ssStatus.dwCurrentState == SERVICE_RUNNING)
		{

		}

		status = std::pair<DWORD, wstring>(0, _T(""));
		return TRUE;
	}

	BOOL CSvrManager::stop()
	{
		SERVICE_STATUS_PROCESS ssStatus;
		DWORD dwBytesNeeded ;
		DWORD dwStartTickCount;
		DWORD dwWaitTime;
		DWORD dwTimeout = 30000;

		if(!QueryServiceStatusEx(service.hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
		{
			DWORD dwErr = GetLastError();
			wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}

		if (ssStatus.dwCurrentState == SERVICE_STOPPED)
		{
			return TRUE;
		}

		dwStartTickCount = GetTickCount();

		while(ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
		{
			dwWaitTime = ssStatus.dwWaitHint / 10;
			if (dwWaitTime < 1000)
			{
				dwWaitTime = 1000;
			}
			if (dwWaitTime > 10000)
			{
				dwWaitTime = 10000;
			}

			Sleep(dwWaitTime);

			if(!QueryServiceStatusEx(service.hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
			{
				DWORD dwErr = GetLastError();
				wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
				status = make_pair(dwErr, errMsg);
				return FALSE;
			}

			if (ssStatus.dwCurrentState == SERVICE_STOPPED)
			{
				return TRUE;

			} 

			if (GetTickCount() - dwStartTickCount > dwTimeout)
			{
				status.first = 1;
				status.second = _T("Service stop timed out");
				return FALSE;
			}
		}

		if (!ControlService(service.hService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssStatus))
		{
			DWORD dwErr = GetLastError();
			wstring errMsg = getLastErrorString(dwErr, _T("[error] ControlService fail:"));
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}

		while(ssStatus.dwCurrentState != SERVICE_STOPPED)
		{
			Sleep(ssStatus.dwWaitHint);

			if(!QueryServiceStatusEx(service.hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
			{
				DWORD dwErr = GetLastError();
				wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
				status = make_pair(dwErr, errMsg);
				return FALSE;
			}

			if (ssStatus.dwCurrentState == SERVICE_STOPPED)
			{
				break;
			}

			if (GetTickCount() - dwStartTickCount > dwTimeout)
			{
				status.first = 1;
				status.second = _T("Wait stoppint timed out");
				return FALSE;
			}
		}

		status = std::pair<DWORD, wstring>(0, _T(""));
		return TRUE;
	}

	BOOL CSvrManager::pause()
	{
		SERVICE_STATUS_PROCESS ssStatus;
		DWORD dwBytesNeeded ;
		DWORD dwStartTickCount;
		DWORD dwWaitTime;
		DWORD dwTimeout = 30000;

		if(!QueryServiceStatusEx(service.hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
		{
			DWORD dwErr = GetLastError();
			wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}

		if (ssStatus.dwCurrentState == SERVICE_PAUSED)
		{
			return TRUE;
		}

		dwStartTickCount = GetTickCount();

		while(ssStatus.dwCurrentState == SERVICE_PAUSE_PENDING)
		{
			dwWaitTime = ssStatus.dwWaitHint / 10;
			if (dwWaitTime < 1000)
			{
				dwWaitTime = 1000;
			}
			if (dwWaitTime > 10000)
			{
				dwWaitTime = 10000;
			}

			Sleep(dwWaitTime);

			if(!QueryServiceStatusEx(service.hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
			{
				DWORD dwErr = GetLastError();
				wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
				status = make_pair(dwErr, errMsg);
				return FALSE;
			}

			if (ssStatus.dwCurrentState == SERVICE_PAUSED)
			{
				return TRUE;

			} 

			if (GetTickCount() - dwStartTickCount > dwTimeout)
			{
				status.first = 1;
				status.second = _T("Service pause timed out");
				return FALSE;
			}
		}

		if (!ControlService(service.hService, SERVICE_CONTROL_PAUSE, (LPSERVICE_STATUS)&ssStatus))
		{
			DWORD dwErr = GetLastError();
			wstring errMsg = getLastErrorString(dwErr, _T("[error] ControlService fail:"));
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}

		while(ssStatus.dwCurrentState != SERVICE_PAUSED)
		{
			Sleep(ssStatus.dwWaitHint);

			if(!QueryServiceStatusEx(service.hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
			{
				DWORD dwErr = GetLastError();
				wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
				status = make_pair(dwErr, errMsg);
				return FALSE;
			}

			if (ssStatus.dwCurrentState == SERVICE_PAUSED)
			{
				break;
			}

			if (GetTickCount() - dwStartTickCount > dwTimeout)
			{
				status.first = 1;
				status.second = _T("Wait pausing timed out");
				return FALSE;
			}
		}

		status = std::pair<DWORD, wstring>(0, _T(""));
		return TRUE;
	}

	BOOL CSvrManager::resume()
	{
		SERVICE_STATUS_PROCESS ssStatus;
		DWORD dwBytesNeeded ;
		DWORD dwStartTickCount;
		DWORD dwWaitTime;
		DWORD dwTimeout = 30000;

		if(!QueryServiceStatusEx(service.hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
		{
			DWORD dwErr = GetLastError();
			wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}

		if (ssStatus.dwCurrentState == SERVICE_RUNNING)
		{
			return TRUE;
		}

		dwStartTickCount = GetTickCount();

		while(ssStatus.dwCurrentState == SERVICE_CONTINUE_PENDING)
		{
			dwWaitTime = ssStatus.dwWaitHint / 10;
			if (dwWaitTime < 1000)
			{
				dwWaitTime = 1000;
			}
			if (dwWaitTime > 10000)
			{
				dwWaitTime = 10000;
			}

			Sleep(dwWaitTime);

			if(!QueryServiceStatusEx(service.hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
			{
				DWORD dwErr = GetLastError();
				wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
				status = make_pair(dwErr, errMsg);
				return FALSE;
			}

			if (ssStatus.dwCurrentState == SERVICE_RUNNING)
			{
				return TRUE;
			}

			if (GetTickCount() - dwStartTickCount > dwTimeout)
			{
				status.first = 1;
				status.second = _T("Service continue timed out");
				return FALSE;
			}
		}

		if (!ControlService(service.hService, SERVICE_CONTROL_CONTINUE, (LPSERVICE_STATUS)&ssStatus))
		{
			DWORD dwErr = GetLastError();
			wstring errMsg = getLastErrorString(dwErr, _T("[error] ControlService fail:"));
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}

		while(ssStatus.dwCurrentState != SERVICE_RUNNING)
		{
			Sleep(ssStatus.dwWaitHint);

			if(!QueryServiceStatusEx(service.hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
			{
				DWORD dwErr = GetLastError();
				wstring errMsg = getLastErrorString(dwErr, _T("[error] QueryServiceStatus fail:"));
				status = make_pair(dwErr, errMsg);
				return FALSE;
			}

			if (ssStatus.dwCurrentState == SERVICE_RUNNING)
			{
				break;
			}

			if (GetTickCount() - dwStartTickCount > dwTimeout)
			{
				status.first = 1;
				status.second = _T("Wait resume timed out");
				return FALSE;
			}
		}

		status = std::pair<DWORD, wstring>(0, _T(""));
		return TRUE;
	}

	BOOL CSvrManager::uninstall()
	{
		BOOL bret = DeleteService(service.hService);
		CloseServiceHandle(service.hService);
		service.hService = NULL;
		return bret;
	}

	BOOL CSvrManager::updateImagePath(const wstring &imagePath)
	{
		HKEY hkey = NULL;
		LSTATUS lret = RegOpenKey(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\services\\corsair"), &hkey);
		if (!hkey)
		{
			DWORD dwErr = GetLastError();
			wstring errMsg = getLastErrorString(dwErr, _T("[error] "));
			status = make_pair(dwErr, errMsg);
			return FALSE;
		}
		DWORD bufsize=(imagePath.length()+1)*sizeof(TCHAR);
		lret = RegSetKeyValue(hkey, NULL, _T("ImagePath"), REG_EXPAND_SZ, imagePath.c_str(), bufsize);
		if (lret != ERROR_SUCCESS)
		{
			DWORD dwErr = GetLastError();
			wstring errMsg = getLastErrorString(dwErr, _T("[error] "));
			status = make_pair(dwErr, errMsg);
			RegCloseKey(hkey);
			return FALSE;
		}

		RegCloseKey(hkey);
		return TRUE;
	}

};