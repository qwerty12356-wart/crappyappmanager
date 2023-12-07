#include "AppList.h"
#include "GUI.h"
#include "log.h"
#include <boost/variant.hpp>

#define APPSETTINGS_ADMINISTRATOR 1
#define APPSETTINGS_APPNAME 2
#define APPSETTINGS_APPDIR 3
#define APPSETTINGS_ICONDIR 4
#define APPSETTINGS_TABLEPARENT 5
#define APPSETTINGS_CREATESHORTCUT 6
#define APPSETTINGS_ICONINDEX 7
#define APPSETTINGS_HIDE 8

#define strcasecmp _stricmp

namespace detail
{

	struct CaseInsensitiveComparator
	{
		bool operator()(const std::string& a, const std::string& b) const noexcept
		{
			return ::strcasecmp(a.c_str(), b.c_str()) < 0;
		}
	};

}   // namespace detail


template <typename A,typename T>
using CaseInsensitiveMap = std::map<A, T, detail::CaseInsensitiveComparator>;

//true if succees, false if not

bool populateAppsettingsentry(AppEntry* app, int option, const toml::value maptosearch)
{
	CaseInsensitiveMap<toml::key, toml::value> temptab;
	try
	{
		temptab = toml::get<CaseInsensitiveMap<toml::key, toml::value>>(maptosearch);
	}
	catch (toml::type_error e)
	{
		
		writelog(std::wstring(L"TOML Get error: ") + mytools::ConvertstringTowstring(e.what()), WL_ERROR);
		return false;
	}

	try
	{
			switch (option)
			{
			case APPSETTINGS_ADMINISTRATOR:
			{
				if (temptab.at("administrator").as_boolean() == true)
				{
					app->RunAsAdministrator = true;
					return true;
				}
				else
				{
					return true;
				}

			}
			case APPSETTINGS_APPNAME:
			{
				
					std::wstring newwstring = mytools::ConvertstringTowstring(toml::get<std::string>(temptab.at("name")));
					app->AppName = newwstring;
					return true;
				

			}
			case APPSETTINGS_APPDIR:
			{
				std::wstring newwstring = mytools::ConvertstringTowstring(toml::get<std::string>(temptab.at("path")));
				
				app->AppDir = newwstring;
				return true;
			}
			case APPSETTINGS_ICONDIR:
			{
				std::wstring newwstring = mytools::ConvertstringTowstring(toml::get<std::string>(temptab.at("iconpath")));
				app->iconDir = newwstring;
				return true;
			}
			case APPSETTINGS_TABLEPARENT:
			{				
				app->tableparent = toml::get<std::string>(temptab.at("parent"));
				return true;
			}
			case APPSETTINGS_CREATESHORTCUT:
			{
				if (temptab.at("shortcut").as_boolean() == true)
				{
					app->docreateshortcut = true;
					return true;
				}
				else
				{
					return true;
				}
			}
			case APPSETTINGS_ICONINDEX:
			{
				app->icoindex = toml::get<int>(temptab.at("iconindex"));
				return true;
			}
			case APPSETTINGS_HIDE:
			{
				app->ishidden = toml::get<bool>(temptab.at("hide"));
				return true;
			}
		default:
				return false;
		}
		}
		catch (std::out_of_range)
		{
			return false;
		}
		catch (toml::type_error e)
		{
			
			writelog(L"Failed to translate types");
		}
		return false;
}


void ParseAppList(std::unordered_map<toml::key, toml::value>& table, std::vector<AppEntry>* apps = NULL)
{
	for (const auto& [key, submap] : table)
	{
		AppEntry tempapp;
		tempapp.tablename = key;
		if (!populateAppsettingsentry(&tempapp, APPSETTINGS_APPDIR, submap))
		{
			//add some debug messages here.
		
		}
		if (!populateAppsettingsentry(&tempapp, APPSETTINGS_ADMINISTRATOR, submap))
		{
			
		}
		
		if (!populateAppsettingsentry(&tempapp, APPSETTINGS_APPNAME, submap))
		{
			
		}
		if (!populateAppsettingsentry(&tempapp, APPSETTINGS_ICONDIR, submap))
		{
			
		}
		if (!populateAppsettingsentry(&tempapp, APPSETTINGS_TABLEPARENT, submap))
		{
			
		}
		if (!populateAppsettingsentry(&tempapp, APPSETTINGS_CREATESHORTCUT, submap))
		{

		}
		if (!populateAppsettingsentry(&tempapp, APPSETTINGS_ICONINDEX, submap))
		{

		}
		populateAppsettingsentry(&tempapp, APPSETTINGS_HIDE, submap);
		apps->push_back(tempapp);
	
	}
}



void GetAppList(std::wstring validapplistpath)
{
	try
	{
		g_appslist.clear();
		

		
		


		const auto tomlsettings = toml::parse(mytools::AutoExpandEnviromentVariable(validapplistpath));
		
		const auto& AppLists = tomlsettings;
		
		std::unordered_map<toml::key, toml::value> tab;
		try
		{
			tab = toml::get<std::unordered_map<toml::key, toml::value>>(AppLists);
			
		}
		catch (toml::type_error e)
		{
			writelog(L"Invalid AppList file", WL_ERROR);
		}

		
		
		ParseAppList(tab, &g_appslist);

	}
	catch (std::runtime_error)
	{
		writelog(L"File AppSettings.toml not found!", WL_ERROR);
	}
	
	catch (toml::syntax_error& e)
	{
		writelog(L"Failed to parse AppList: " + mytools::ConvertstringTowstring(e.what()), WL_ERROR);
	};
	
}

#define SETTING_FORCERUNASADMINISTRATOR 1
#define SETTING_CREATESHORTCUTS 2
#define SETTING_PATHTOAPPLIST 3
#define SETTING_WIDTH 4
#define SETTING_HEIGHT 5
#define SETTING_LOCKSIZE 6
#define SETTING_SELFCREATEICON 7
#define SETTING_SELFICONPARAM 8
bool GetSetting(short Option, CaseInsensitiveMap<toml::key, toml::value> table, ProgramConfigs* ptrtostruct )
{
	if (ptrtostruct == NULL)
	{
		return 0;
	}
	try
	{
		switch (Option)
		{
		case SETTING_PATHTOAPPLIST:
		{
			
			ptrtostruct->AppLists.assign(mytools::ConvertstringTowstring(toml::get<std::string>(table.at("applistpath"))));
			if (!mytools::IsFileExist(mytools::AutoExpandEnviromentVariable(ptrtostruct->AppLists.wstring()).c_str()))
			{
				writelog(L"Applist not found.", WL_ERROR);
				return 0;
			}
			return 1;
		}
		case SETTING_FORCERUNASADMINISTRATOR:
		{
			ptrtostruct->forcerunAsAdministrator = toml::get<bool>(table.at("mustadministrator"));
			return 1;
		}
		case SETTING_CREATESHORTCUTS:
		{
			ptrtostruct->createshortcuts = toml::get<bool>(table.at("createshortcuts"));
			return 1;
		}
		case SETTING_WIDTH:
		{
			ptrtostruct->width = toml::get<unsigned int>(table.at("menuwidth"));
			return 1;
		}
		case SETTING_HEIGHT:
		{
			ptrtostruct->height = toml::get<unsigned int>(table.at("menuheight"));
			return 1;
		}
		case SETTING_LOCKSIZE:
		{
			ptrtostruct->locksize = toml::get<bool>(table.at("locksize"));
			return 1;
		}
		case SETTING_SELFCREATEICON:
		{
			ptrtostruct->selfcreateicon = toml::get<bool>(table.at("createicontoself"));
			return 1;
		}
		case SETTING_SELFICONPARAM:
		{
			ptrtostruct->selficonparam = mytools::ConvertstringTowstring(toml::get<std::string>(table.at("selficonparam")));
			return 1;
		}
		default:
			return 0;
		}
	}
	catch (std::out_of_range)
	{
		return 0;
	}
	return 0;
}


bool GetConfigFile(boost::filesystem::wpath conffilepath, ProgramConfigs* progconfOUT)
{
	ProgramConfigs configs;
	
	try
	{
		const auto configfiletab = toml::parse<toml::discard_comments, std::map>(mytools::AutoExpandEnviromentVariable(conffilepath.wstring()));
		

		try
		{
			CaseInsensitiveMap<toml::key, toml::value> tab = toml::get<CaseInsensitiveMap<toml::key, toml::value>>(configfiletab);
			GetSetting(SETTING_CREATESHORTCUTS, tab, &configs);
			GetSetting(SETTING_FORCERUNASADMINISTRATOR, tab, &configs);
			GetSetting(SETTING_PATHTOAPPLIST, tab, &configs);
			GetSetting(SETTING_WIDTH, tab, &configs);
			GetSetting(SETTING_HEIGHT, tab, &configs);
			GetSetting(SETTING_LOCKSIZE, tab, &configs);
			GetSetting(SETTING_SELFCREATEICON, tab, &configs);
			GetSetting(SETTING_SELFICONPARAM, tab, &configs);

			*progconfOUT = configs;
			return 1;
			
		}
		catch (toml::type_error e)
		{
			writelog(L"Invalid config file", WL_ERROR);
			return 0;
		}
	}
	
	catch (std::runtime_error)
	{
		writelog(L"ConfigFile not found", WL_ERROR);
		return false;
	}
	
	catch (toml::syntax_error& e)
	{
		writelog(L"Failed to parse ConfigFile: " + mytools::ConvertstringTowstring(e.what()), WL_ERROR);
		return false;
	}
}