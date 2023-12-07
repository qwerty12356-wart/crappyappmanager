#pragma once
#include <string>
#include <Windows.h>
#include <boost/filesystem.hpp>
namespace mytools
{
	std::enable_if_t<sizeof(wchar_t) == 2, std::wstring> u16stringtowstring(std::u16string utf16);
	std::wstring AutoExpandEnviromentVariable(std::wstring relpath);
	UINT MyPrivateExtractIcon(boost::filesystem::wpath filename, int nIconIndex, int cxIcon, int cyIcon, HICON* phicon, UINT* piconid, UINT nMaxIcon, UINT flags);
	void ClearNullTerm(std::wstring&);
	bool IsFileExist(LPCWSTR path);
	BOOL IsElevated();
	std::wstring ConvertstringTowstring(std::string a);
}



		

