#pragma once
#include <windows.h>
#include <string.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#define WL_ERROR 1
#define WL_WARNINGS 2
#define WL_INFO 3
#define WL_DEBUG 4
#define MENU_LOG 2002
inline std::wstring* g_logs;
bool writelog(std::wstring const& value, int logmode = WL_INFO);

bool synclog(HWND logwnd);

bool quicksync(HWND logwnd);

bool writeWindowsErrorCode(std::wstring pre, DWORD errcode);