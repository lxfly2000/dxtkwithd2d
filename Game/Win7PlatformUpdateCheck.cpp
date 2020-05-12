#include "pch.h"
#include "Win7PlatformUpdateCheck.h"
#include "ProgressDialog.h"
#include <VersionHelpers.h>
#include<shellapi.h>
#include<urlmon.h>
#include<sstream>
#include<thread>
#include<string>

#pragma comment(lib,"urlmon.lib")

static const TCHAR patch_url_x64[] = TEXT("https://download.microsoft.com/download/1/4/9/14936FE9-4D16-4019-A093-5E00182609EB/Windows6.1-KB2670838-x64.msu");
static const TCHAR patch_url_x86[] = TEXT("https://download.microsoft.com/download/1/4/9/14936FE9-4D16-4019-A093-5E00182609EB/Windows6.1-KB2670838-x86.msu");
static std::thread tSub;

BOOL Is64bitSystem()
{
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
		return TRUE;
	else
		return FALSE;
}

class StatusCallback :public IBindStatusCallback
{
private:
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
	{
		*ppv = NULL;
		if (riid == IID_IUnknown || riid == IID_IBindStatusCallback) {
			*ppv = this;
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
		return m_cRef++;
	}

	STDMETHODIMP_(ULONG) Release()
	{
		if (--m_cRef == 0) {
			delete this;
			return 0;
		}
		return m_cRef;
	}

	STDMETHODIMP GetBindInfo(DWORD* grfBINDF, BINDINFO* bindinfo)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP GetPriority(LONG* nPriority)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC* formatetc, STGMEDIUM* stgmed)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP OnLowResource(DWORD reserved)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP OnObjectAvailable(REFIID iid, IUnknown* punk)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP OnStartBinding(DWORD dwReserved, IBinding* pib)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP OnStopBinding(HRESULT hresult, LPCWSTR szError)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
	{
		if (downloadStatus == 2 || downloadStatus == 3)
			return E_ABORT;
		if (ulProgressMax)
			SetProgressDialogBarValue(ulProgress * 100 / ulProgressMax);
		return S_OK;
	}

	DWORD m_cRef;
public:
	int downloadStatus;//0=下载完成 1=下载中 2=下载出错 3=下载中断
	StatusCallback():m_cRef(1),downloadStatus(1){}
};

static StatusCallback dcb;
static TCHAR retFilePath[MAX_PATH]{};
static LPCTSTR retFileName = NULL;
static int downloadRetryLeft = 3;

void Subthread_Download(BOOL arg)
{
	LPCTSTR tURL = arg ? patch_url_x64 : patch_url_x86;
	retFileName = wcsrchr(tURL, '/') + 1;
	std::wstring title = TEXT("正在下载：");
	title.append(retFileName);
	SetProgressDialogTitle(title.c_str());
	dcb.downloadStatus = 1;
	ProgressSetEnableClose(TRUE);
	downloadRetryLeft--;
	if (URLDownloadToCacheFile(NULL, tURL, retFilePath, ARRAYSIZE(retFilePath) - 1, 0, &dcb) == S_OK)
	{
		ProgressSetEnableClose(FALSE);
		title = TEXT("正在安装：");
		title.append(retFileName);
		title.append(TEXT(", 安装完成后系统将重启"));
		SetProgressDialogTitle(title.c_str());
		SetProgressDialogBarValue(-1);
		std::wstring param = TEXT("\"");
		param.append(retFilePath);
		param.append(TEXT("\" /quiet"));
		SHELLEXECUTEINFO se{};
		se.cbSize = sizeof se;
		se.lpVerb = TEXT("open");
		se.lpFile = TEXT("wusa.exe");
		se.lpParameters = param.c_str();
		se.fMask = SEE_MASK_NOCLOSEPROCESS;
		se.nShow = SW_NORMAL;
		ShellExecuteEx(&se);
		WaitForSingleObject(se.hProcess, INFINITE);
		DWORD retcode;
		GetExitCodeProcess(se.hProcess, &retcode);
		CloseHandle(se.hProcess);
		if (retcode == 0x570 && downloadRetryLeft)
		{
			DeleteFile(retFilePath);
			Subthread_Download(arg);
			return;
		}
		EndProgressDialog(retcode);
	}
}

void DownloadPatch(HWND hwnd)
{
	ProgressSetOnClickCancel([]()
		{
			dcb.downloadStatus = 3;
			EndProgressDialog(E_ABORT);//不知道为啥这句放到Subthread_Download中去会在程序退出时报错
		});
	ProgressSetOnShow([]()
		{
			tSub = std::thread(Subthread_Download, Is64bitSystem());
		});
	switch (ProgressDialog(hwnd, TEXT("取消(&C)")))
	{
	case E_ABORT:default:
		MessageBox(hwnd, L"下载被中断，您需要稍后手动下载更新以完成安装。\n"
			"Download was interrupted, please download this update manually later to finish setup.", retFileName, MB_ICONEXCLAMATION);
		break;
	case 0x570:
		MessageBox(hwnd, L"下载的文件不完整或无法读取，您需要稍后手动下载更新以完成安装。\n"
			"File corrupted or unable to read, please download this update manually later to finish setup.", retFileName, MB_ICONEXCLAMATION);
		break;
	case S_OK:
		break;
	}
	tSub.join();
}

BOOL Win7PlatformUpdateCheck(HWND hwnd, HRESULT hr)
{
	if (hr == E_NOINTERFACE)
	{
		if (IsWindows7OrGreater())
		{
			if (IsWindows7SP1OrGreater())
			{
				if (!IsWindows8OrGreater())
				{
					if (MessageBox(hwnd, TEXT("您需要安装 Windows 7 平台更新 (KB2670838) 才能运行本程序，是否现在下载安装？\n"
						"You need to install Platform Update for Windows 7 (KB2670838) to run this program, do you want to download and install?"), NULL, MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
						DownloadPatch(hwnd);
					return TRUE;
				}
			}
			else
			{
				MessageBox(hwnd, TEXT("您需要安装 Windows 7 SP1 更新来运行本程序。\n"
					"You need to install Windows 7 Service Pack 1 to run this program."), NULL, MB_ICONEXCLAMATION);
				return TRUE;
			}
		}
	}
	return FALSE;
}
