#include "CommandLine.h"
#include <iostream>
#include "tools.h"
#include "log.h"
#include "AppList.h"
namespace po = boost::program_options;
int ParseCommandLine(PWSTR cmdline, ProgramConfigs* progconf = NULL)
{
	if (!progconf)
	{
		return 0;
	}
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
		writelog(L"BEGIN GETTING FILE", WL_ERROR);
		//boost::filesystem::wpath confpath(vm["configFile"].as<std::wstring>().c_str());
		
		boost::filesystem::wpath confpath(mytools::AutoExpandEnviromentVariable(mytools::u16stringtowstring(utf8::utf8to16(vm["configfile"].as<std::string>()))));
		if (!confpath.empty())
		{
			if (GetFileAttributes(confpath.c_str()) != INVALID_FILE_ATTRIBUTES)
			{
				progconf->pathtoconf = confpath;
				
			};
		}
		else {
			return 0;
		}
	}
	int carry = 0;
	if (vm.count("noshortcut"))
	{
		carry = 1;
	}
	if (vm.count("nogui"))
	{
		if (carry)
		{
			return 0;
		}
		return 2 + carry;
	}
	
	
	return 6+carry;
}
