#pragma once
#define UTF_CPP_CPLUSPLUS 201703L
#include <toml.hpp>
#include <boost/variant.hpp>
#include <iostream>
#include <utf8.h>
#include <locale>
#include "tools.h"

#define DEF_WIDTH 499+5
#define DEF_HEIGHT 373

/*
#ifndef MS_STDLIB_BUGS
#  if ( _MSC_VER || __MINGW32__ || __MSVCRT__ )
#    define MS_STDLIB_BUGS 1
#  else
#    define MS_STDLIB_BUGS 0
#  endif
#endif

#if MS_STDLIB_BUGS
#  include <io.h>
#  include <fcntl.h>
#endif
*/

//to a tools header 
struct ProgramConfigs
{
	boost::filesystem::wpath AppLists;
	bool forcerunAsAdministrator = false;
	bool createshortcuts = false;
	UINT width = DEF_WIDTH;
	UINT height = DEF_HEIGHT;
	std::wstring selficonparam;
	bool locksize = false;
	bool selfcreateicon = false;
	bool gui = true;
	boost::filesystem::wpath pathtoconf = L"%CURDIR%\\AppConf.toml";

};

 struct AppEntry
{
	bool RunAsAdministrator = false;
	bool docreateshortcut = false;
	int icoindex = 0;
	std::wstring AppName = L"";
	std::wstring AppDir = L"";
	std::wstring iconDir;
	std::string tablename;
	std::string tableparent = "";
	bool ishidden = false;
};

void GetAppList(std::wstring validapplistpath);



inline std::vector<AppEntry> g_appslist;
bool GetConfigFile(boost::filesystem::wpath conffilepath, ProgramConfigs* progconfOUT);
//LRESULT CALLBACK dialogconsole(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);