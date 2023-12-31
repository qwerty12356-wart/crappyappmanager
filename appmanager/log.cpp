#include "GUI.h"
#include "log.h"

void initCustomLog(HINSTANCE hinst)
{
	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hInstance = hinst;
	wcex.lpszClassName = L"LogDlg";
	wcex.lpfnWndProc = (WNDPROC)dialoglog;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
	RegisterClassEx(&wcex);
}


HWND CreateDialogLog(HINSTANCE hinst, HWND mainhwnd)
{
	//initCustomConsole(hinst);
	// WS_EX_WINDOWEDGE
	HWND dlgcons = CreateWindowExW(WS_EX_DLGMODALFRAME, L"LogDlg", L"Logs",
		WS_VISIBLE | WS_SYSMENU | WS_CAPTION | WS_POPUP, 100, 100, 800, 600,
		mainhwnd, NULL, hinst, NULL);
	return dlgcons;
}
RECT logwindowrect = {};
LRESULT CALLBACK dialoglog(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_CREATE:
	{
		
		HFONT hf = {};
		hf = CreateFont(16, 0, NULL, NULL, NULL, 0, 0, 0, 0, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas");
		RECT windowsize = {};
		GetClientRect(hwnd, &windowsize);
		int width = windowsize.right;
		int height = windowsize.bottom;
		HWND logwindow = CreateWindow(RICHEDIT_CLASSW, L"", ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL | WS_CHILD | WS_VISIBLE| WS_VSCROLL, 10, 10, width - 20, height - 50, hwnd, NULL, NULL, NULL);
		synclog(logwindow);
		g_logwndhandle = logwindow;
		GetWindowRect(logwindow, &logwindowrect);
		MapWindowPoints(HWND_DESKTOP, GetParent(logwindow), (LPPOINT)&logwindowrect, 2);
		SendMessageW(logwindow, WM_SETFONT, (WPARAM)hf, NULL);
		HWND OKButton = CreateWindowW(L"button", L"Ok",WS_VISIBLE | WS_CHILD,width -90, height -35, 80, 25, hwnd, (HMENU)OK_BUTTON, NULL, NULL);
		if (!logwindow) {
			std::cout << "Abort mission" << std::endl;
		}
		GETTEXTLENGTHEX gex = {};
		gex.codepage = 1200;
		gex.flags = GTL_DEFAULT;
		
		//std::cout << logwndcurrlen << std::endl;
		//writelog(L"Mission success", WL_INFO);
		DeleteObject(hf);
		break;

	}

	case WM_COMMAND: {
		//std::cout << (HWND*)GetWindowLongW(hwnd, GWLP_USERDATA) << std::endl;
		switch (wp)
		{
		case OK_BUTTON: {
			//EnableWindow(GetParent(hwnd), TRUE);
			DestroyWindow(hwnd);
			g_logwndhandle = NULL;
			break;
		}
		break;
		}
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HPEN pen;
		HBRUSH brush;
		HGDIOBJ oldP, oldB;
		HDC hdc;
		hdc = BeginPaint(hwnd, &ps);
		pen = ::CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		brush = (HBRUSH) ::GetStockObject(HOLLOW_BRUSH);
		oldP = ::SelectObject(hdc, pen);
		oldB = ::SelectObject(hdc, brush);
		std::cout << logwindowrect.left << std::endl;
		std::cout << logwindowrect.right << std::endl;
		std::cout << logwindowrect.bottom << std::endl;
		std::cout << logwindowrect.top << std::endl;
		if (!::Rectangle(hdc, logwindowrect.left-1, logwindowrect.top-1, logwindowrect.right+1, logwindowrect.bottom+1))
		{
			std::cout << "Rectangle creation failed" << std::endl;
		}
		::SelectObject(hdc, oldP);
		::SelectObject(hdc, oldB);
		::DeleteObject(pen);
		::EndPaint(hwnd, &ps);
		//return DefWindowProc(hwnd, msg, wp, lp);
		return 0;
	}
	case WM_CLOSE:
		EnableWindow(GetParent(hwnd), TRUE);
		DestroyWindow(hwnd);
		g_logwndhandle = NULL;
		break;

	}
	return (DefWindowProcW(hwnd, msg, wp, lp));
}
//the real sync
bool quicksync(HWND logwindow)
{
	
	std::wstring buff;
	GETTEXTLENGTHEX gex = {};
	gex.codepage = 1200;
	gex.flags = GTL_DEFAULT;
	int logwndcurrlen = SendMessage(logwindow, EM_GETTEXTLENGTHEX, (WPARAM)&gex, NULL);
	if (logwndcurrlen == 0)
	{
		synclog(logwindow);
		return TRUE;
	}
	else
	{
		buff = g_logs->substr(logwndcurrlen, g_logs->length());
		std::wstring secbuff;
		std::wistringstream wiss(buff);
		while (std::getline(wiss, secbuff))
		{
			secbuff = secbuff + L'\n';
			SendMessage(logwindow, EM_SETSEL, -2, -1);
			SendMessage(logwindow, EM_REPLACESEL, 0, (LPARAM)secbuff.c_str());
			SendMessage(logwindow, WM_VSCROLL, SB_BOTTOM, 0);
		}
		return TRUE;
	}
	return FALSE;
}

bool synclog(HWND logwindow)
{
	SetWindowText(logwindow, g_logs->c_str());
	return TRUE;
}

bool writelog(std::wstring const& value, int logmode) {
	std::wstring buff = value;
	while (buff.back() == L'\0')
	{
		buff.pop_back();
	}
	bool isERROR = false; //use to color text in rich edit, unused for now.
	switch (logmode)
	{
	case WL_DEBUG:
	{
	
		auto time = std::time(nullptr);
		auto timep = *std::localtime(&time);
		std::wostringstream timestream;
		timestream << std::put_time(&timep, L"%Y-%m-%d %H:%M:%S ");
		buff = timestream.str() + L"[DEBUG]: " + buff + L'\n';
		
		break;
	}
	case WL_ERROR:
	{
		auto time = std::time(nullptr);
		auto timep = *std::localtime(&time);
		std::wostringstream timestream;
		timestream << std::put_time(&timep, L"%Y-%m-%d %H:%M:%S ");
		buff = timestream.str() + L"[ERROR]: " + buff +L'\n';
		isERROR = true;
		break;
	}
	case WL_WARNINGS:
	{
		auto time = std::time(nullptr);
		auto timep = *std::localtime(&time);
		std::wostringstream timestream;
		timestream << std::put_time(&timep, L"%Y-%m-%d %H:%M:%S ");
		buff = timestream.str() + L"[WARN]: " + buff + L'\n';
		break;
	}
	case WL_INFO:
	{
		auto time = std::time(nullptr);
		auto timep = *std::localtime(&time);
		std::wostringstream timestream;
		timestream << std::put_time(&timep, L"%Y-%m-%d %H:%M:%S ");
		buff = timestream.str() + L"[INFO]: " + buff + L'\n';
		break;
	}
	default:
	{
		//assume info
		auto time = std::time(nullptr);
		auto timep = *std::localtime(&time);
		std::wostringstream timestream;
		timestream << std::put_time(&timep, L"%Y-%m-%d %H:%M:%S ");
		buff = timestream.str() + L"[INFO]: " + buff + L'\n';
		break;
	}
	}
	g_logs->append(buff);
	if (g_logwndhandle != NULL)
	{
		quicksync(g_logwndhandle);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


bool writeWindowsErrorCode(std::wstring pre, DWORD errcode)
{

}