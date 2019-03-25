#include "stdafx.h"
#include "util.h"
#include <strsafe.h>
#include <Shlwapi.h>
#include <ShlObj.h>
#include <Windows.h>
#include <stdlib.h>
#include "resource.h"
#include "cJSON.h"
#include <winhttp.h>
#include <fstream>
#include <winsock2.h>  

#pragma comment(lib,"ws2_32.lib") 

extern EditionType gEditionType;

void _trace(LPCTSTR fmt,...) 
{
	va_list args;
	va_start(args, fmt);
	TCHAR buf[1000]={0};
	StringCchPrintf(buf, _countof(buf), fmt, args);
	buf[lstrlen(buf)]=L'\n';
	OutputDebugString(buf);
	va_end(args);
}


namespace CloudCare {

	wstring getLastErrorString(DWORD dwErr, LPCTSTR prefix) {
		wstring result;
		_TCHAR buf[2048]={0};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErr, 0,buf, _countof(buf), NULL);
		if (prefix != nullptr)
		{
			_TCHAR buf2[2048]={0};
			StringCchPrintf(buf2, _countof(buf2), _T("%s %s"), prefix, buf);
			result = std::wstring(buf2);
		} else {
			result = std::wstring(buf);
		}
		return result;
	}

	wstring getAbsExePath(LPCTSTR name) {
		TCHAR buf[2000]={0};
		StringCchCopy(buf, _countof(buf), __targv[0]);
		PathRemoveFileSpec(buf);
		PathCombine(buf, buf, name);
		return wstring(buf);
	}

	wstring getTargetInstallDir() {
		TCHAR buf[MAX_PATH]={0};
		::SHGetSpecialFolderPath(NULL,buf,CSIDL_PROGRAM_FILES,FALSE);
		::PathCombine(buf,buf,_T("CloudCare\\"));
		::PathCombine(buf, buf, _T("ProfWangProbe"));
		::SHCreateDirectory(NULL, buf);
		return wstring(buf);
	}

	wstring getTargetExePath() {
		return getTargetInstallDir()+_T("\\probe.exe");
	}

	wstring getTargetYamPath() {
		return getTargetInstallDir()+_T("\\probe.yml");
	}

	wstring getTargetVersionPath() {
		return getTargetInstallDir()+_T("\\version");
	}

	wstring getTempDir() {
		TCHAR buf[MAX_PATH]={0};
		::GetTempPath(_countof(buf), buf);
		PathCombine(buf,buf,_T("CloudCare"));
		return wstring(buf);
	}


	wstring ltrim(LPCTSTR str) {
		int nc = 0;
		for (int i=0; str[i]!=L'\0'; i++)
		{
			if (str[i] == L' ')
			{
				nc++;
			} else {
				break;
			}
		}
		return wstring(str+nc);
	}

	wstring rtrim(LPCTSTR str) {
		int l = (int)lstrlen(str);
		int nc = 0;
		auto c = str[l-1];
		for (int i=l; i>=0; i--)
		{
			if (str[i] == L'\0')
			{
				continue;
			}
			if (str[i] == L' ')
			{
				nc++;
			} else {
				break;
			}
		}
		return wstring(str, str+l-nc);
	}

	wstring trim(LPCTSTR str) {
		wstring s = ltrim(str);
		return rtrim(s.c_str());
	}


	wstring getFlagValueFromPath(const wstring &str, const wstring &flag) 
	{
		wstring result;
		std::size_t nfind = str.find(flag);
		if (nfind != wstring::npos)
		{
			wstring sub = str.substr(nfind+flag.length());
			if (!sub.empty())
			{
				wstring s = ltrim(sub.c_str());
				nfind = s.find(_T(" "));
				if (nfind != wstring::npos)
				{
					result = s.substr(0, nfind);
				} else {
					result = rtrim(s.c_str());
				}
			}
		}
		return result;
	}


	std::vector<wstring> splitString(const wstring &str, const wstring &splitter)
	{
		wstring s(str);
		std::size_t npos;
		std::vector<wstring> splits;
		while((npos = s.find(_T("-"))) != wstring::npos)
		{
			splits.push_back(s.substr(0,npos));
			s.erase(0, npos+1);
		}
		if (!s.empty())
		{
			splits.push_back(s);
		}
		return splits;
	}


	std::wstring installFromRes(wstring dest, UINT resid, LPCTSTR  type)
	{
		TCHAR buf[100] = {0};
		HRSRC hexe = FindResource(hInst, MAKEINTRESOURCE(resid),  type);
		if (!hexe)
		{
			StringCchPrintf(buf, _countof(buf), _T("FindResource %d failed"), resid);
			return std::wstring(buf);
		}
		DWORD dwsize = SizeofResource(hInst, hexe);
		HGLOBAL hres = LoadResource(hInst, hexe);
		if (!hres)
		{
			StringCchPrintf(buf, _countof(buf), _T("LoadResource %d failed"), resid);
			return buf;
		}
		LPVOID resBytes = LockResource(hres);
		PBYTE pb = new BYTE[dwsize];
		memcpy_s(pb, dwsize,resBytes,dwsize);
		if(!GlobalUnlock(hres))
		{
			StringCchPrintf(buf, _countof(buf), _T("GlobalUnlock %d failed"), resid);
			return buf;
		}

		if (PathFileExists(dest.c_str()))
		{
			if(!DeleteFile(dest.c_str()))
			{
				StringCchPrintf(buf, _countof(buf), _T("DeleteFile %d failed"), resid);
				return buf;
			}
		}

		HANDLE hfile = CreateFile(dest.c_str(), GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (!hfile || hfile == INVALID_HANDLE_VALUE)
		{
			StringCchPrintf(buf, _countof(buf), _T("CreateFile %d failed"), resid);
			return buf;
		}
		DWORD dwWrite=0;
		if(!::WriteFile(hfile, pb, dwsize,&dwWrite, NULL)) {
			delete [] pb;
			CloseHandle(hfile);
			StringCchPrintf(buf, _countof(buf), _T("WriteFile %d failed"), resid);
			return buf;
		}
		delete [] pb;
		CloseHandle(hfile);

		return _T("");
	}

	wstring  string2wstring(const std::string &str)
	{
		int needlen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		TCHAR *pws = new TCHAR[needlen];
		MultiByteToWideChar(CP_UTF8,0,str.c_str(), -1, pws, needlen);
		wstring ws(pws);
		delete [] pws;
		return ws;
	}

	std::string  wide2string(const std::wstring &str)
	{
		int needlen = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, nullptr, nullptr);
		char *ps = new char[needlen];
		WideCharToMultiByte(CP_UTF8,0, str.c_str(), -1, ps, needlen, nullptr, nullptr);
		std::string s(ps);
		delete [] ps;
		return s;
	}

	UINT runCmd(LPCTSTR cmdline)
	{
		STARTUPINFO si = {0};
		si.cb = sizeof(si);
		PROCESS_INFORMATION pi={0};
		BOOL bret = CreateProcess(nullptr, const_cast<LPTSTR>(cmdline), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
		DWORD ec = 1;

		if (bret)
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
			GetExitCodeProcess(pi.hProcess, &ec);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
		return ec;
	}


	static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/'};
	static char *decoding_table = NULL;
	static int mod_table[] = {0, 2, 1};


	char *base64_encode(const unsigned char *data,
		size_t input_length,
		size_t *output_length) {

			*output_length = 4 * ((input_length + 2) / 3);

			char *encoded_data = (char *)malloc(*output_length);
			if (encoded_data == NULL) return NULL;

			for (int i = 0, j = 0; i < input_length;) {

				UINT32 octet_a = i < input_length ? (unsigned char)data[i++] : 0;
				UINT32 octet_b = i < input_length ? (unsigned char)data[i++] : 0;
				UINT32 octet_c = i < input_length ? (unsigned char)data[i++] : 0;

				UINT32 triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

				encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
				encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
				encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
				encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
			}

			for (int i = 0; i < mod_table[input_length % 3]; i++)
				encoded_data[*output_length - 1 - i] = '=';

			return encoded_data;
	}

	void build_decoding_table() {

		decoding_table = (char *)malloc(256);

		for (int i = 0; i < 64; i++)
			decoding_table[(unsigned char) encoding_table[i]] = i;
	}

	unsigned char *base64_decode(const char *data,
		size_t input_length,
		size_t *output_length) {

			if (decoding_table == NULL) build_decoding_table();

			if (input_length % 4 != 0) return NULL;

			*output_length = input_length / 4 * 3;
			if (data[input_length - 1] == '=') (*output_length)--;
			if (data[input_length - 2] == '=') (*output_length)--;

			unsigned char *decoded_data = (unsigned char*)malloc(*output_length);
			if (decoded_data == NULL) return NULL;

			for (int i = 0, j = 0; i < input_length;) {

				UINT32 sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
				UINT32 sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
				UINT32 sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
				UINT32 sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

				UINT32 triple = (sextet_a << 3 * 6)
					+ (sextet_b << 2 * 6)
					+ (sextet_c << 1 * 6)
					+ (sextet_d << 0 * 6);

				if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
				if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
				if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
			}

			return decoded_data;
	}


	void base64_cleanup() {
		free(decoding_table);
	}


	// 对用户的 SecretKey 进行加密
	static unsigned char xorkeys[] = {
		0xbb, 0x74, 0x24, 0xa5,
			0xba, 0x5a, 0x0a, 0x8c,
			0x65, 0x61, 0xdf, 0x57,
			0xa1, 0x3c, 0xfb, 0xe9,
			0x89, 0x12, 0xcb, 0x5a,
			0xd2, 0x70, 0xf3, 0x82,
			0x67, 0xdd, 0x5c, 0x8a,
			0xec, 0x77, 0xcf, 0x48,
			0x39, 0x1c, 0x0e, 0xab,
			0xee, 0xe, 0x16, 0xe8,
			0x2c, 0xab, 0xf2, 0x61,
			0xfc, 0xc7, 0xfd, 0x1c,
			0x58, 0xfc, 0xe7, 0x4f,
			0x70, 0xed, 0xc8, 0xf1,
			0x5f, 0x36, 0x18, 0x3c,
			0x29, 0x38, 0x27, 0xc1,
			0xbc, 0x29, 0x3, 0x89,
			0xcb, 0xbe, 0xc7, 0xc8,
			0xce, 0xb3, 0x7d, 0x7d,
			0xe1, 0x84, 0x74, 0xd, 0x1c, 0x66, 0xb6, 0x86, 0xbc, 0xb, 0x33, 0x1, 0x17, 0x93, 0xd3, 0x82, 0xb7, 0xb0, 0x96, 0xe3, 0xd6, 0xef, 0xc4, 0xa1, 0xf7, 0xb0, 0x6e, 0xd, 0x55, 0x2e, 0x3e, 0x25, 0x4c, 0xf7, 0xc6, 0xeb, 0x63, 0x8c, 0x88, 0x69, 0xf5, 0x86, 0x6a, 0x56, 0xc1, 0xaf, 0x46, 0xbf, 0x6f, 0x35, 0xfc, 0x90};


		std::wstring xorDecode(const char *pdata, size_t len)
		{
			size_t outlen=0;
			unsigned char *data = base64_decode(pdata, len, &outlen);
			if (!data)
			{
				return _T("");
			}
			int length = data[0] ^ xorkeys[0];

			unsigned char dedata[1000] = {0};
			for(int index = 0; index < 128; index++) {
				dedata[index] = (data[index] ^ xorkeys[index]);
			}
			std::string s((const char *)(dedata+1), length);

			return string2wstring(s);
		}

		std::string loadVersionRes()
		{
			HRSRC hexe = FindResource(hInst, MAKEINTRESOURCE(IDR_VERSION_INFO), _T("TXT"));
			DWORD dwsize = SizeofResource(hInst, hexe);
			HGLOBAL hres = LoadResource(hInst, hexe);
			LPVOID resBytes = LockResource(hres);
			PBYTE pb = new BYTE[dwsize+1];
			ZeroMemory(pb, dwsize+1);
			memcpy_s(pb, dwsize,resBytes,dwsize);
			std::string sv((LPCSTR)pb);
			delete [] pb;
			return sv;
		}

		std::wstring parseVersion(const std::string & str)
		{
			std::wstring result;
			cJSON *json = cJSON_Parse(str.c_str());
			const cJSON *version = nullptr;

			if (!json)
			{
				const char *error_ptr = cJSON_GetErrorPtr();
				if (error_ptr)
				{
					fprintf(stderr, "Error: %s\n", error_ptr);
				}
			} else {
				version = cJSON_GetObjectItemCaseSensitive(json, "version");
				if (cJSON_IsString(version) && version->valuestring != nullptr)
				{
					result = string2wstring(version->valuestring);
				}
			}
			if (json)
			{
				cJSON_Delete(json);
			}
			return result;
		}

		BOOL checkVersion(std::string &response)
		{
			LPCTSTR versionUrl;

			switch(gEditionType)
			{
			case EditionType_Test:
				versionUrl = _T("/profwang_probe/windows/test/version");
				break;
			case EditionType_Propord:
				versionUrl = _T("/profwang_probe/windows/preprod/version");
				break;
			case EditionType_Release:
				versionUrl = _T("/profwang_probe/windows/release/version");
				break;
			}

			HINTERNET hSession = NULL;
			HINTERNET hConnect = NULL;
			HINTERNET hRequest = NULL;
			BOOL bResult = FALSE;
			DWORD dwSize = 0, dwReaded = 0;
			char *pBuffer = nullptr;
			BOOL bRet = FALSE;

			hSession = WinHttpOpen(L"A WinHTTP Example Program/1.0", 
				WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, 
				WINHTTP_NO_PROXY_NAME, 
				WINHTTP_NO_PROXY_BYPASS, 0);

			if (!hSession)
			{
				goto end;
			}

			hConnect = WinHttpConnect(hSession, _T("cloudcare-kodo.oss-cn-hangzhou.aliyuncs.com"), INTERNET_DEFAULT_HTTP_PORT, 0);
			if (!hConnect)
			{
				goto end;
			}

			hRequest = WinHttpOpenRequest(hConnect, _T("GET"),  versionUrl, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
			if (!hRequest)
			{
				goto end;
			}

			bResult = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
			if (!bResult)
			{
				goto end;
			}

			bResult = WinHttpReceiveResponse(hRequest, 0);
			if (!bResult)
			{
				goto end;
			}


			do 
			{
				dwSize = 0;
				if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
				{
					bRet = FALSE;
					goto end;
				}
				if (dwSize == 0)
				{
					break;
				}
				pBuffer = new char[dwSize+1];
				ZeroMemory(pBuffer, dwSize+1);

				if (!WinHttpReadData(hRequest, pBuffer, dwSize, &dwReaded))
				{
					delete []  pBuffer;
					bRet = FALSE;
					goto end;
				} else {
					bRet = TRUE;
					response += pBuffer;
					delete [] pBuffer;
				}

			} while (dwSize > 0);


end:
			if(hRequest) WinHttpCloseHandle(hRequest);
			if(hConnect) WinHttpCloseHandle(hConnect);
			if(hSession) WinHttpCloseHandle(hSession);

			return bRet;
		}


		static DWORD WINAPI versionRoutine(LPVOID param)
		{
			Sleep(5*1000);

			while(1)
			{
				std::string remoteVersion;
				BOOL bCheck = checkVersion(remoteVersion);
				if (bCheck)
				{
					std::wstring v = parseVersion(remoteVersion);
					remoteVersion = wide2string(v);
					if (v != versionFromRes)
					{
						if (hMainDlg)
						{
							PostMessage(hMainDlg, WM_DETECT_NEW_VERSION, NULL, NULL);
						}
					}
				}
				Sleep(1000 * 60 * 60);
			}

			return 1;
		}


		void startCheckVersion()
		{
			static HANDLE hthread = NULL;
			if (!hthread)
			{
				hthread = CreateThread(nullptr, 0, versionRoutine, nullptr, 0, nullptr);
			}
		}

		std::wstring getServiceErrText()
		{
			std::wstring errmsg;
			std::wstring errpath = getTargetInstallDir()+ _T("\\install_error");
			std::ifstream inf(wide2string(errpath).c_str(), std::ios::in|std::ios::binary);
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
				errmsg = string2wstring(sv);
				DeleteFile(errpath.c_str());
			}

			if (errmsg == _T("carrier.kodo.invalidHttpAuthorization"))
			{
				errmsg = _T("签名认证失败");
			} else if (errmsg == _T("carrier.kodo.rejected")) {
				errmsg = _T("拒绝访问");
			} else if (errmsg == _T("carrier.kodo.tooManyMetrics")) {
				errmsg = _T("请求的指标过多");
			} else if (errmsg == _T("carrier.kodo.issueSourceNotSet")) {
				errmsg = _T("无情报源");
			} else if (errmsg == _T("carrier.kodo.teamNotFound") || errmsg == _T("carrier.kodo.invalidX-Team-Id")) {
				errmsg = _T("不存在该团队");
			} else if (errmsg == _T("carrier.kodo.invalidJson")) {
				errmsg = _T("无效的JSON数据");
			} else if (errmsg == _T("carrier.kodo.uploaderNotFound")) {
				errmsg = _T("不存在该主机诊断或集群诊断");
			} else if (errmsg == _T("carrier.kodo.issueSourceNotFound")) {
				errmsg = _T("不存在该情报源或该情报源已经删除");
			} else if (errmsg == _T("carrier.kodo.portused")) {
				errmsg = _T("端口被占用");
			} else if (errmsg == _T("carrier.kodo.exceedMaxLimit")) {
				errmsg = _T("安装失败：超过购买的额度限制");
			} else if (errmsg == _T("carrier.kodo.invalidJson")) {
				errmsg = _T("无效的JSON数据");
			} else if (errmsg == _T("carrier.kodo.invalidApiArgs")) {
				errmsg = _T("无效的请求参数");
			} else if (errmsg == _T("carrier.kodo.invalidTsdbTime")) {
				errmsg = _T("无效的时间戳");
			}
			return errmsg;
		}

		BOOL checkPortIsUsed(int port)
		{
			WORD sockVersion = MAKEWORD(2,2);  
			WSADATA wsaData;  
			if(WSAStartup(sockVersion, &wsaData)!=0)  
			{  
				getLastErrorString(GetLastError(), _T(""));
				return FALSE; 
			}

			SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if(sclient == INVALID_SOCKET)
			{
				getLastErrorString(GetLastError(), _T(""));
				return FALSE;
			}

			sockaddr_in serAddr;
			serAddr.sin_family = AF_INET;
			serAddr.sin_port = htons(port);
			serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
			if(connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
			{
				closesocket(sclient);
				return FALSE;
			}
			closesocket(sclient);
			return TRUE;
		}

		/*BOOL checkProbeAvaiable()
		{
			if (gRemoteHostUrl.empty())
			{
				gRemoteHostUrl = _T("http://kodo.cloudcare.com");
			}
			
			HINTERNET hSession = NULL;
			HINTERNET hConnect = NULL;
			HINTERNET hRequest = NULL;
			BOOL bResult = FALSE;
			DWORD dwSize = 0, dwReaded = 0;
			char *pBuffer = nullptr;
			BOOL bRet = FALSE;

			hSession = WinHttpOpen(L"A WinHTTP Example Program/1.0", 
				WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, 
				WINHTTP_NO_PROXY_NAME, 
				WINHTTP_NO_PROXY_BYPASS, 0);

			if (!hSession)
			{
				goto end;
			}

			hConnect = WinHttpConnect(hSession, gRemoteHostUrl.c_str(), INTERNET_DEFAULT_HTTP_PORT, 0);
			if (!hConnect)
			{
				goto end;
			}

			hRequest = WinHttpOpenRequest(hConnect, _T("POST"),  _T("/v1/issue-source"), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
			if (!hRequest)
			{
				goto end;
			}

			bResult = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
			if (!bResult)
			{
				goto end;
			}

			bResult = WinHttpReceiveResponse(hRequest, 0);
			if (!bResult)
			{
				goto end;
			}


			do 
			{
				dwSize = 0;
				if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
				{
					bRet = FALSE;
					goto end;
				}
				if (dwSize == 0)
				{
					break;
				}
				pBuffer = new char[dwSize+1];
				ZeroMemory(pBuffer, dwSize+1);

				if (!WinHttpReadData(hRequest, pBuffer, dwSize, &dwReaded))
				{
					delete []  pBuffer;
					bRet = FALSE;
					goto end;
				} else {
					bRet = TRUE;
					response += pBuffer;
					delete [] pBuffer;
				}

			} while (dwSize > 0);


end:
			if(hRequest) WinHttpCloseHandle(hRequest);
			if(hConnect) WinHttpCloseHandle(hConnect);
			if(hSession) WinHttpCloseHandle(hSession);

			return bRet;
		}*/

};