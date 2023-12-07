
#include "GUI.h"
#include "AppList.h"
#include "log.h"
#include "CreateShortcut.h"
#include "tools.h"
#include <boost/program_options.hpp>

void CreateLinkInAppEntryLoop(std::vector<AppEntry> aa, std::wstring fullpathtodesktop)
{
	
	for (auto& i : aa)
	{
		if (i.AppName == L"" && i.AppDir != L"")
		{
			i.AppName = boost::filesystem::wpath(i.AppDir).stem().wstring();
		}
		if (i.docreateshortcut == true)
		{
			CreateLink(i.AppDir.c_str(), i.AppName.c_str(), NULL, fullpathtodesktop.c_str(), L"", i.iconDir.c_str(), i.icoindex, true);
		}
	}
}
//inline boost::filesystem::wpath g_ConfigFilePath = L"%CURDIR%\\AppConf.toml";
namespace po = boost::program_options;

int WINAPI wWinMain(HINSTANCE hinst, HINSTANCE hiprevinst, PWSTR cmdline, int ncmdline)
{
	g_logs = new std::wstring();

	

	
	ProgramConfigs progconf;
	wchar_t* tempbuffer = new wchar_t[MAX_PATH];
	GetModuleFileName(NULL, tempbuffer, MAX_PATH);
	boost::filesystem::wpath* tempwpath = new boost::filesystem::wpath();
	tempwpath->assign(tempbuffer);
	//GetConfigFile(mytools::AutoExpandEnviromentVariable(g_ConfigFilePath.wstring()), &progconf);
	SetEnvironmentVariable(L"curdir", tempwpath->parent_path().c_str());
	//SetEnvironmentVariable(L"curconfdir", mytools::AutoExpandEnviromentVariable(g_ConfigFilePath.wstring()).c_str());
	wmemset(tempbuffer, 0, sizeof(tempbuffer));
	SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, tempbuffer);
	SetEnvironmentVariable(L"userprofile", tempbuffer);
	delete[] tempbuffer;
	
	//Sacrifice code readability for efficency
	po::options_description desc("All Options");
	po::positional_options_description p;
	p.add("configfile", -1);

	desc.add_options()
		("configfile", po::value<std::string>(), "Set config file for program")
		("nogui", "Run in cmd mode")
		("noshortcut", "Run without shortcut");
	po::variables_map vm;
	//po::store(po::parse_command_line(static_cast<int>(wcslen(cmdline)), *cmdline, desc), vm);
	std::vector<std::wstring> args = po::split_winmain(cmdline);
	po::store(po::wcommand_line_parser(args).options(desc).positional(p).allow_unregistered().run(), vm);
	po::notify(vm);
	if (vm.count("configfile"))
	{
		//boost::filesystem::wpath confpath(vm["configFile"].as<std::wstring>().c_str());

		boost::filesystem::wpath confpath(mytools::ConvertstringTowstring(vm["configfile"].as<std::string>()));
		if (!confpath.empty())
		{
			if (GetFileAttributes(confpath.c_str()) != INVALID_FILE_ATTRIBUTES)
			{
				progconf.pathtoconf = confpath;

			};
		}
		else
		{
			writelog(L"configfile parameter not specified, falling back to default", WL_ERROR);
		}
	}
	SetEnvironmentVariable(L"curconfdir", progconf.pathtoconf.c_str());
	GetConfigFile(progconf.pathtoconf, &progconf);
	

	if (vm.count("noshortcut"))
	{
		progconf.createshortcuts = false;
	}
	if (vm.count("nogui"))
	{
		progconf.gui = false;
	}

	(void)CoInitialize(NULL);
	
	
	
	

	

	//Init libraries
	INITCOMMONCONTROLSEX commex = {};
	commex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	commex.dwICC = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&commex);
	LoadLibrary(L"riched20.dll");

	//Init custom made stuff
	initCustomLog(hinst);
	initCustomButtonsWnd(hinst);
	GetAppList(progconf.AppLists.wstring());
	currapplistdir = progconf.AppLists.wstring();
	if (progconf.createshortcuts == true)
	{
		CreateLinkInAppEntryLoop(g_appslist, L"%USERPROFILE%\\Desktop");
	}
	if (progconf.selfcreateicon == true)
	{
		CreateLink(tempwpath->c_str(), tempwpath->stem().c_str(), progconf.selficonparam.c_str(), L"%USERPROFILE%\\Desktop", L"", NULL, NULL, false);
	}
	if (progconf.gui == false)
	{
		return 0;
	}
	delete tempwpath;
	






	DWORD style = WS_OVERLAPPEDWINDOW;
	if (progconf.locksize)
	{
		style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	}


	//Begin window creation
	
	const wchar_t* classname = L"MainWndName";
	WNDCLASS WC = { 0 };
	WC.hInstance = hinst;
	WC.lpfnWndProc = WndProc;
	WC.hCursor = LoadCursor(NULL, IDC_ARROW);
	WC.hbrBackground = (HBRUSH)COLOR_WINDOW;
		WC.lpszClassName = classname;
		RegisterClass(&WC);
		HWND mainwnd = CreateWindow(classname, L"USB Menu", style, CW_USEDEFAULT, CW_USEDEFAULT, progconf.width, progconf.height, NULL, NULL, hinst, NULL);
		if (!mainwnd)
		{
			MessageBox(NULL, L"Windows Creation Failed. Application halting", L"Crittcal Error Occurred", MB_OK);
			return 0;
		}
		ShowWindow(mainwnd, ncmdline);
		MSG msg;
		while (GetMessage(&msg, NULL, NULL, NULL) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		delete g_logs;
		//Gdiplus::GdiplusShutdown(gdipluscodes);
		CoUninitialize();
		return (int)msg.wParam;
	
}