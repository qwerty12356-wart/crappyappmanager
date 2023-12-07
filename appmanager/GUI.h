#pragma once
#pragma comment(lib,"comctl32.lib")
#include <Windows.h>
#include <map>
#include <unordered_map>
#include <iostream>
#include <CommCtrl.h>
//#include <io.h>
//#include <fcntl.h>
//#include <cstdio>
#include <string>
#include <Richedit.h>
#include <math.h>
#include <Uxtheme.h>
//#include <gdiplus.h>
#include <windowsx.h>
//#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib, "uxtheme.lib")
#define MENU_QUIT 1001
#define BACK_BUTTON 2000
#define OK_BUTTON 2001
#define MAINWND_BUTTONACTION 2005
#define LOAD_CONFIGFILEMENU 2006
#define REFRESH_APPLIST 2007
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
inline std::wstring currapplistdir;
inline std::unordered_map<std::string, std::string> g_parentmap;


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
int InitializeMenu(HWND hwnd);
HWND CreateDialogLog(HINSTANCE hinst, HWND mainhwnd);
void initCustomLog(HINSTANCE hinst);
LRESULT CALLBACK dialoglog(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
inline HWND g_logwndhandle = 0;
BOOL initCustomButtonsWnd(HINSTANCE hinst);

