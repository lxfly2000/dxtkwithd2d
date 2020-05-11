#pragma once
#include<Windows.h>

HRESULT ProgressDialog(HWND hwndParent, LPCTSTR strCancel);
BOOL SetProgressDialogTitle(LPCTSTR title);
LONG SetProgressDialogBarValue(int percentage);
BOOL EndProgressDialog(int retCode);
void ProgressSetOnClickCancel(void(*fCallback)());
void ProgressSetOnShow(void(*fCallback)());
