#pragma once
#include <boost/filesystem.hpp>
#include <Windows.h>
#include <boost/program_options.hpp>
#include <vector>
#include <string>
#include "AppList.h"

inline boost::filesystem::wpath g_ConfigFilePath = L"%CURDIR%\\AppConf.toml";
//inline boost::filesystem::wpath g_AppListPath(L"%CURDIR%\\AppList.toml");
int ParseCommandLine(PWSTR cmdline, ProgramConfigs* progconf = NULL);