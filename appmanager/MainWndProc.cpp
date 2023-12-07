#include "GUI.h"
#include "log.h"
#include "AppList.h"
#define TEST2BUTTON 2
#include <boost/filesystem.hpp>
#include <shlobj.h>
#include <objbase.h>      // For COM headers
#include <shobjidl.h>     // for IFileDialogEvents and IFileDialogControlEvents
#include <shlwapi.h>



#define MAINWND_CREATEBUTTONS 2006
#define BUTTON_DEFSIZE 120


#define CSTIMGBUTTON_PRESSSTATE 0
#define CSTIMGBUTTON_IMAGE (CSTIMGBUTTON_PRESSSTATE + sizeof(LONG))
#define CSTIMGBUTTON_LABEL (CSTIMGBUTTON_IMAGE + sizeof(LONG))
#define CSTIMGBUTTON_APPSETTINGS (CSTIMGBUTTON_LABEL + sizeof(std::wstring*))
#define CSTIMGBUTTON_FONT (CSTIMGBUTTON_APPSETTINGS + sizeof(AppEntry*))
HWND backbutton = nullptr;
HWND directorybar = nullptr;


BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lp)
{
	if (hwnd == backbutton || hwnd == directorybar || hwnd == g_logwndhandle)
	{

	}
	else
	{
		DestroyWindow(hwnd);
	}
	return TRUE;
}

int GetMaxFit(int width, int buttonsize)
{
	return floor((width - 10) / (buttonsize + 1));
}

std::string g_btntable;

const COMDLG_FILTERSPEC c_rgSaveTypes[] =
{
	{L"TOML config file (*.toml)",       L"*.toml"}
};

HRESULT TomlConfLoad(LPWSTR* outputPath)
{
	IFileDialog* pfd = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		DWORD dwFlags;
		hr = pfd->GetOptions(&dwFlags);
		if (SUCCEEDED(hr))
		{
			hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
			if (SUCCEEDED(hr))
			{
				hr = pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
				if (SUCCEEDED(hr))
				{
					hr = pfd->SetFileTypeIndex(1);
					if (SUCCEEDED(hr))
					{
						hr = pfd->SetDefaultExtension(L"toml");
						if (SUCCEEDED(hr))
						{
				
							pfd->Show(NULL);
							if (SUCCEEDED(hr))
							{
								IShellItem* psiResult;
								hr = pfd->GetResult(&psiResult);
								if (SUCCEEDED(hr))
								{
									 //psiResult->GetDisplayName();
									LPWSTR ptrtostring;
									psiResult->GetDisplayName(SIGDN_FILESYSPATH, &ptrtostring);
									*outputPath = ptrtostring;
								}
								psiResult->Release();
							}

						}
					}
				}
			}
		}
		pfd->Release();
	}
	return hr;
}




LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
		// wparam is Appsettings, lp is reserved for the back button
	case MAINWND_BUTTONACTION:
	{
		AppEntry* aps = (AppEntry*)wp;
		
		if (lp)
		{
			std::vector<AppEntry>::iterator i =std::find_if(g_appslist.begin(), g_appslist.end(), [&](const AppEntry val)
				{
					return val.tablename == g_btntable;
				}); //iterate thru a vector to find the appentry for the parent folder
			g_btntable = i->tableparent;
			PostMessage(hwnd, MAINWND_CREATEBUTTONS, NULL, NULL);
			if (i->tableparent == "")
			{
				EnableWindow(backbutton, FALSE);
				ShowWindow(backbutton, SW_HIDE);
			}
			return 0;
		}

		if (aps->AppDir == L"")
		{
			g_btntable = aps->tablename;
			PostMessage(hwnd, MAINWND_CREATEBUTTONS, NULL, NULL);
			EnableWindow(backbutton, TRUE);
			ShowWindow(backbutton, SW_SHOW);
			return 0;
		}

		
		if (aps->RunAsAdministrator)
		{
			std::wstring tempbuf = mytools::AutoExpandEnviromentVariable(aps->AppDir);
			if (tempbuf != L"")
			{
				ShellExecute(NULL, L"runas", tempbuf.c_str(), L"", boost::filesystem::wpath(tempbuf).parent_path().c_str(), 1);
				

			}
		}
		else
		{
			std::wstring tempbuf = mytools::AutoExpandEnviromentVariable(aps->AppDir);
			if (tempbuf != L"")
			{
				ShellExecute(NULL, L"open", tempbuf.c_str(), L"", boost::filesystem::wpath(tempbuf).parent_path().c_str(), 1);
				
			}
		}
		
		return 0;
	}
	
	case MAINWND_CREATEBUTTONS:
	{
		EnumChildWindows(hwnd, EnumChildProc, NULL);
		RECT backbtnr, dirbarr = {};
		if (backbutton != nullptr )
		{
			GetWindowRect(backbutton, &backbtnr);

			MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&backbtnr, 2);
			
		}
		else
		{
			return 0;
		}
		 uint64_t count = 0;
		 RECT wndrect = {};
		 GetWindowRect(hwnd, &wndrect);
		 int maxfit = GetMaxFit(wndrect.right - wndrect.left, BUTTON_DEFSIZE);
		
		 HFONT hf = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Helvetica");
		 if (backbtnr.bottom != 0)
		 {


			 for (auto& it : g_appslist)
			 {
				 if (it.tableparent == g_btntable && !it.ishidden)
				 {
					 if (it.AppName == L"" && it.AppDir != L"")
					 {
						 it.AppName = boost::filesystem::wpath(it.AppDir).stem().wstring();
					 }
					 HWND temp = CreateWindow(L"CustomButton", it.AppName.c_str(), WS_CHILD | WS_VISIBLE, 5 + (BUTTON_DEFSIZE * (count % maxfit)), backbtnr.bottom + 5 + BUTTON_DEFSIZE * (floor(count / maxfit)), BUTTON_DEFSIZE, BUTTON_DEFSIZE, hwnd, NULL, NULL, (LPVOID)&it);
					 PostMessage(temp, WM_SETFONT, (WPARAM)hf, TRUE);
					 
					 count++;
				 }

			 }
		 }
		 
		return 0;
	}
	case WM_CREATE:
	{
		InitializeMenu(hwnd);
		
		
		
		backbutton = CreateWindow(L"Button", L"Back", WS_CHILD | WS_VISIBLE, 5, 5, 150, 50, hwnd, (HMENU)BACK_BUTTON, NULL, NULL);
		HFONT hf = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Helvetica");
		SendMessage(backbutton, WM_SETFONT, (WPARAM)hf, TRUE);
		SetWindowTheme(backbutton, L"", L"");
		EnableWindow(backbutton, FALSE);
		ShowWindow(backbutton, SW_HIDE);

		PostMessage(hwnd, MAINWND_CREATEBUTTONS, NULL, NULL);
		return 0;
	}
	case WM_KEYUP:
	{
		if (wp == VK_F5)
		{
			SendMessage(hwnd, WM_COMMAND, REFRESH_APPLIST, NULL);
		}
		break;
	}
	case WM_COMMAND:
	{
		switch (wp)
		{
		
		case MENU_QUIT:
		{
			SendMessage(hwnd, WM_CLOSE, NULL, NULL);
			break;
		}
		case MENU_LOG:
		{
			std::cout << "Button OK!" << std::endl;
			HWND temp = CreateDialogLog(GetModuleHandle(NULL), hwnd);
			if (!temp)
			{
				std::cout << "Window Creation failure" << std::endl;
				std::cout << GetLastError() << std::endl;
			}
			else
			{
				//SetWindowLongPtrW(temp, GWLP_USERDATA, (LONG)&hwn/d);
				//EnableWindow(hwnd, FALSE);
			}
			break;
			//break;
		}
		case LOAD_CONFIGFILEMENU:
		{
			LPWSTR tempwstring = nullptr;
			TomlConfLoad(&tempwstring);
			currapplistdir = tempwstring;
			GetAppList(tempwstring);
			SendMessage(hwnd, MAINWND_CREATEBUTTONS, NULL, NULL);
			CoTaskMemFree(tempwstring);
			break;
		}
		case REFRESH_APPLIST:
		{
			GetAppList(currapplistdir);
			PostMessage(hwnd, MAINWND_CREATEBUTTONS, NULL, NULL);
			break;
		}
		case BACK_BUTTON:
		{
			PostMessage(hwnd, MAINWND_BUTTONACTION, NULL, TRUE);
			break;
		}

		default:
			break;
		}
		return 0;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}
	return false;
}

int InitializeMenu(HWND hwnd)
{
	HMENU ourmenu = CreateMenu();
	HMENU subfilemenu = CreateMenu();
	AppendMenu(ourmenu, MF_POPUP, (UINT_PTR)subfilemenu, L"File");
	AppendMenu(subfilemenu, MF_POPUP, LOAD_CONFIGFILEMENU, L"Load App Lists");
	AppendMenu(subfilemenu, MF_STRING, MENU_QUIT, L"Quit");
	HMENU helpsubmenu = CreateMenu();
	AppendMenu(ourmenu, MF_POPUP, (UINT_PTR)helpsubmenu, L"Help");
	AppendMenu(helpsubmenu, MF_STRING, MENU_LOG, L"Logs");
	AppendMenu(helpsubmenu, MF_STRING, REFRESH_APPLIST, L"Refresh");
	
	SetMenu(hwnd, ourmenu);
	return 0;
}

//wine implementation copying

LRESULT CALLBACK CustomImageButton(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		CREATESTRUCTW* cretstrct = (CREATESTRUCTW*)lp;
		
		std::wstring* buttonlabel = new std::wstring;
		bool TESTSUBJECT= false;
		if (sizeof(LONG) == sizeof(std::wstring*))
		{
			TESTSUBJECT = true;
		}
		if (cretstrct->lpszName != NULL){
			*buttonlabel = cretstrct->lpszName;
		}
		AppEntry* aps = new AppEntry;
		
		if (cretstrct->lpCreateParams != NULL)
		{
			*aps = *((AppEntry*)cretstrct->lpCreateParams);
			
		}
		HICON* bmp = new HICON;
		*bmp =(HICON)NULL;
		if (aps->iconDir == L"" && aps->AppDir == L"")
		{
			mytools::MyPrivateExtractIcon(L"%windir%\\System32\\imageres.dll.mun", -5382, 64, 64, bmp, NULL, 1,NULL);
			if (*bmp == NULL)
			{
				//ExtractIcons().Get(L"%windir%\\SystemResources\\imageres.dll.mun", -5382, 64, 64, bmp, NULL, 1,);
				mytools::MyPrivateExtractIcon(L"%windir%\\SystemResources\\imageres.dll.mun", -5382, 64, 64, bmp, NULL, 1, NULL);
			}
		}
		else if (aps->iconDir == L"")
		{
			
			mytools::MyPrivateExtractIcon(boost::filesystem::wpath(aps->AppDir), 0, 64, 64, bmp, NULL, 1, NULL);
			//PrivateExtractIcons(aps->AppDir.c_str(), 0, 64, 64, bmp, NULL, 1, NULL);
			//RT_GROUP_ICON
		}
		else
		{
			mytools::MyPrivateExtractIcon(aps->iconDir.c_str(), 0, 64, 64, bmp, NULL, 1, NULL);
			
		}
		if (*bmp == NULL)
		{
			//ExtractIcons().Get(L"%windir%\\SystemResources\\imageres.dll.mun", -15, 64, 64, bmp, NULL, 1, LR_LOADFROMFILE);
			mytools::MyPrivateExtractIcon(L"%windir%\\System32\\imageres.dll", -15, 64, 64, bmp, NULL, 1, NULL);
		}

		SetLastError(0);
		SetWindowLongPtr(hwnd, CSTIMGBUTTON_IMAGE, (LONG)bmp);
		SetWindowLongPtr(hwnd, CSTIMGBUTTON_APPSETTINGS, (LONG)aps);
		SetWindowLongPtr(hwnd, CSTIMGBUTTON_LABEL, (LONG)buttonlabel);
		
	
		break;
	}
	case WM_PAINT:
	{
		LONG style = GetWindowLongPtr(hwnd, GWL_STYLE);
		//HRGN hrgn;
		PAINTSTRUCT ps;
		RECT rect;
		HDC hdc = BeginPaint(hwnd, &ps);
		bool isclicked = NULL;
		GetClientRect(hwnd, &rect);
		UINT flags = IsWindowEnabled(hwnd) ? DSS_NORMAL : DSS_DISABLED;
		isclicked = (bool)GetWindowLongPtr(hwnd, CSTIMGBUTTON_PRESSSTATE);
		UINT state = DFCS_BUTTONPUSH;
		HFONT hf = (HFONT)GetWindowLongPtr(hwnd, CSTIMGBUTTON_FONT);
		HFONT oldhf = NULL;
		HBRUSH hbrs = (HBRUSH)SelectObject(hdc, GetSysColorBrush(COLOR_BTNFACE));
		
		if (!hf)
		{
			hf = (HFONT)GetStockObject(SYSTEM_FONT);
		}
		HICON* hbmp = (HICON*)GetWindowLongPtr(hwnd, CSTIMGBUTTON_IMAGE);
		if (isclicked == true)
		{
			state |= DFCS_PUSHED;
		}
		bool retval = DrawFrameControl(hdc, &rect, DFC_BUTTON, state);
		std::wstring controllabel = *(std::wstring*)GetWindowLongPtr(hwnd, CSTIMGBUTTON_LABEL);
		

		oldhf = (HFONT)SelectObject(hdc, hf);
		RECT windowstextrect, allocbtnclientrec;
		SIZE fontsz = {};
		GetTextExtentPoint32(hdc, controllabel.c_str(), controllabel.length(), &fontsz);
		int lines = 1;
		
		allocbtnclientrec.left = 0;
		allocbtnclientrec.top = 0;
		allocbtnclientrec.bottom = rect.bottom - rect.top;
		allocbtnclientrec.right = rect.right - rect.left;

		windowstextrect = allocbtnclientrec;
		//windowstextrect.right = rect.right - rect.left;
		windowstextrect.left += 5;
		windowstextrect.right += -5;
		windowstextrect.bottom = allocbtnclientrec.bottom - 5;
		//janky but it works, and thats all i need.
		if (((double)fontsz.cx / (double)(windowstextrect.right-windowstextrect.left)) > (double)1)
		{
			lines = 2;
		}
		windowstextrect.top = allocbtnclientrec.bottom -5 - (fontsz.cy) * lines  ;
		int prevbkmode = SetBkMode(hdc, TRANSPARENT);
		
		DrawText(hdc, controllabel.c_str(), -1, &windowstextrect, DT_CENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_MODIFYSTRING | DT_WORDBREAK | DT_EDITCONTROL);
		UINT posx = (allocbtnclientrec.right / 2) - 64 / 2;
		UINT posy = windowstextrect.top - 64- 5 ;

		UINT returnval = DrawState(hdc, GetSysColorBrush(COLOR_GRAYTEXT), NULL, (LPARAM)*hbmp, NULL, posx, posy, 64, 64, DSS_NORMAL | DST_ICON);


		SetBkMode(hdc, prevbkmode);
		SelectObject(hdc, hbrs);
		SelectObject(hdc, oldhf);
		
		
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_DESTROY:
	{
		
		HFONT hf = (HFONT)GetWindowLongPtr(hwnd, CSTIMGBUTTON_FONT);
		DeleteFont(hf);
		HICON* totrash3 = (HICON*)GetWindowLongPtr(hwnd, CSTIMGBUTTON_IMAGE);
		DestroyIcon(*totrash3);
		delete totrash3;
		std::wstring* totrash = (std::wstring*)GetWindowLongPtr(hwnd, CSTIMGBUTTON_LABEL);
		AppEntry* totrash2 = (AppEntry*)GetWindowLongPtr(hwnd, CSTIMGBUTTON_APPSETTINGS);
		delete totrash2;
		delete totrash;
		break;
	}

	case WM_SETTEXT:
	{

		std::wstring* t = (std::wstring*)GetWindowLongPtr(hwnd, CSTIMGBUTTON_LABEL);
		
		
		if (t != nullptr && lp != NULL)
		{

			*t = std::wstring((LPCWSTR)lp);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	case WM_GETTEXT:
	{
		std::wstring* temp = (std::wstring*)GetWindowLongPtr(hwnd, CSTIMGBUTTON_LABEL);
		if (temp != nullptr)
		{
			LPWSTR templp = (LPWSTR)lp;
			temp->copy(templp, (int)wp, 0);
			return TRUE;
		}
		return FALSE;
	}
	case WM_GETTEXTLENGTH:
	{
		std::wstring* tmp = (std::wstring*)GetWindowLongPtr(hwnd, CSTIMGBUTTON_LABEL);
		if (tmp != nullptr)
		{
			return tmp->length();
		}
		else {
			return 0;
		}
		
	}
	case WM_LBUTTONDOWN:
	{
		
		SetWindowLongPtr(hwnd, CSTIMGBUTTON_PRESSSTATE, true);
		SetCapture(hwnd);
		SetFocus(hwnd);
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	}
	case WM_LBUTTONUP:
	{
	    
		bool isclicked = (bool)GetWindowLongPtr(hwnd, CSTIMGBUTTON_PRESSSTATE);
		if (!isclicked) { break; }
		RECT rect = {};
		GetClientRect(hwnd, &rect);
		POINT pt = {};
		pt.x = GET_X_LPARAM(lp);
		pt.y = GET_Y_LPARAM(lp);
		if (PtInRect(&rect, pt))
		{
			AppEntry* aps = (AppEntry*)GetWindowLongPtr(hwnd, CSTIMGBUTTON_APPSETTINGS);
			PostMessage(GetParent(hwnd), MAINWND_BUTTONACTION, (WPARAM)aps, NULL);
		}
		
		SetWindowLongPtr(hwnd, CSTIMGBUTTON_PRESSSTATE, false);
		InvalidateRect(hwnd, 0, TRUE);
		ReleaseCapture();
		break;
	}
	case WM_CAPTURECHANGED:
	{
		bool ispushed = GetWindowLongPtr(hwnd, CSTIMGBUTTON_PRESSSTATE);
		if (ispushed)
		{
			SetWindowLongPtr(hwnd, CSTIMGBUTTON_PRESSSTATE, false);
			InvalidateRect(hwnd, 0, TRUE);
		}
		break;
	}
	case WM_SETFONT:
	{
		HFONT hf = (HFONT)wp;
		
		if (hf)
		{
			SetWindowLongPtr(hwnd, CSTIMGBUTTON_FONT, (LONG)hf);
			
		}
		if (lp)
		{
			InvalidateRect(hwnd, NULL, TRUE);
		}
		return 0;
	}

	}
	
	
	
	return DefWindowProc(hwnd, msg, wp, lp);
}

BOOL initCustomButtonsWnd(HINSTANCE hinst)
{
	WNDCLASSEX wcex = {};
	wcex.hInstance = hinst;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = L"CustomButton";
	wcex.lpfnWndProc = CustomImageButton;
	wcex.cbWndExtra = CSTIMGBUTTON_FONT + sizeof(LONG)+1;
	RegisterClassEx(&wcex);
	return TRUE;
}