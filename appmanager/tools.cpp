
#define UTF_CPP_CPLUSPLUS 201703L
#include "tools.h"
#include <vector>
#include "log.h"
#include "utf8.h"

std::wstring mytools::ConvertstringTowstring(std::string a)
{
	return mytools::u16stringtowstring(utf8::utf8to16(a));
}

std::enable_if_t<sizeof(wchar_t) == 2, std::wstring> mytools::u16stringtowstring(std::u16string utf16)
{
	std::wstring buffer;
	buffer.resize(utf16.size());
	memcpy_s(buffer.data(), buffer.size() * sizeof(char16_t), utf16.data(), utf16.size() * sizeof(wchar_t));

	return buffer;
}




std::wstring mytools::AutoExpandEnviromentVariable(std::wstring relpath)
{
	
	std::wstring tempbuff;
	
	
	DWORD strlen = ExpandEnvironmentStrings(relpath.c_str(), NULL, 0);
	tempbuff.resize(strlen);
	ExpandEnvironmentStrings(relpath.c_str(), &tempbuff[0], strlen);
	return tempbuff;
}

struct EXTICONSTRUCT
{
	int IconIndex = 0;
	int cxIcon = 0;
	int cyIcon = 0;
	HICON* phicon = nullptr;
	UINT* piconID = nullptr;
	UINT maxIcon = 0;
	UINT flags = NULL;
	int64_t nIconCount = 0;
	int64_t loopcount = 0;
};;;



HMODULE g_hmd;


bool ExtractIconById(HMODULE hmodule, EXTICONSTRUCT* exticoparam, LPCWSTR lpName, LPCWSTR lpType)
{
	
	if (exticoparam->phicon == NULL)
	{
		return false;
	}
	int cxIcon = exticoparam->cxIcon;
	int cyIcon = exticoparam->cyIcon;


	HRSRC hRes = FindResource(hmodule, lpName, lpType);
	if (hRes == NULL)
	{
		return false;
	}
	HGLOBAL hhRes = LoadResource(hmodule, hRes);
	if (hhRes == NULL)
	{
		return false;
	}
	if (exticoparam->piconID == NULL)
	{
		UINT buffericonid;
		while (cxIcon != 0)
		{
			buffericonid = NULL;
			
			buffericonid = LookupIconIdFromDirectoryEx((PBYTE)hhRes, TRUE, LOWORD(cxIcon), LOWORD(cyIcon), exticoparam->flags);
			if (buffericonid == NULL)
			{
				return false;
			}
			HRSRC icoRes = FindResource(hmodule, MAKEINTRESOURCE(buffericonid), MAKEINTRESOURCE(RT_ICON));
			DWORD awd = GetLastError();
			if (icoRes == NULL)
			{
				return false;
			}
			if (icoRes == NULL)
			{
				return false;
			}
			
			HGLOBAL hIconRes = LoadResource(hmodule, icoRes);
			DWORD icoressize = SizeofResource(hmodule, icoRes);
			if (hIconRes == NULL || icoressize == NULL)
			{
				return false;
			}
			exticoparam->phicon[exticoparam->nIconCount] = CreateIconFromResourceEx((PBYTE)hIconRes, icoressize, TRUE, 0x00030000, cxIcon, cyIcon, exticoparam->flags);
			awd = GetLastError();
			if (exticoparam->phicon[exticoparam->nIconCount] == NULL)
			{
				return false;
			}
			cxIcon = cxIcon >> 16;
			cyIcon = HIWORD(cyIcon);
			exticoparam->nIconCount = exticoparam->nIconCount + 1;
		}

	}
	else
	{
		while (cxIcon != 0)
		{
			exticoparam->piconID[exticoparam->nIconCount] = LookupIconIdFromDirectoryEx((PBYTE)hhRes, TRUE, LOWORD(cxIcon), LOWORD(cyIcon), exticoparam->flags);
			if (exticoparam->piconID[exticoparam->nIconCount] == NULL)
			{
				return false;
			}
			HRSRC icoRes = FindResource(hmodule, MAKEINTRESOURCE(exticoparam->piconID[exticoparam->nIconCount]), MAKEINTRESOURCE(RT_ICON));
			if (icoRes == NULL)
			{
				return false;
			}
			HGLOBAL hIconRes = LoadResource(hmodule, icoRes);
			DWORD icoressize = SizeofResource(hmodule, icoRes);
			if (hIconRes == NULL || icoressize == NULL)
			{
				return false;
			}
			exticoparam->phicon[exticoparam->nIconCount] = CreateIconFromResourceEx((PBYTE)hIconRes, icoressize, TRUE, 0x00030000, cxIcon, cyIcon, exticoparam->flags);
			if (exticoparam->phicon[exticoparam->nIconCount] == NULL)
			{
				return false;
			}
			cxIcon = HIWORD(cxIcon);
			cyIcon = HIWORD(cyIcon);
			exticoparam->nIconCount = exticoparam->nIconCount + 1;
			
		}

	}
	return true;
}


BOOL EmumNameResCallBack(HMODULE hmodule, LPCWSTR lptype, LPWSTR lpName, LONG_PTR lParam)
{
	
	EXTICONSTRUCT* ptrIcoParams = (EXTICONSTRUCT*)lParam;
	if (ptrIcoParams->phicon == NULL && ptrIcoParams->IconIndex == -1)
	{
		ptrIcoParams->nIconCount = ptrIcoParams->nIconCount + 1;
		return 1;
	}

	if (ptrIcoParams->IconIndex < 0)
	{
		if (ptrIcoParams->maxIcon == 1)
		{
			
			ExtractIconById(hmodule, ptrIcoParams, MAKEINTRESOURCE(abs(ptrIcoParams->IconIndex)), MAKEINTRESOURCE(RT_GROUP_ICON));
			
		}
		else
		{
			DWORD currpointedid = NULL;
			if (*lpName == L'#')
			{
				currpointedid = _wtoi(&lpName[1]);
			}
			else if (IS_INTRESOURCE(lpName))
			{
				currpointedid = _wtoi(lpName);
			}
			if (currpointedid >= abs(ptrIcoParams->IconIndex) /* && currpointedid < (abs(ptrIcoParams->IconIndex) + ptrIcoParams->maxIcon)*/ && currpointedid != NULL)
			{
				ExtractIconById(hmodule, ptrIcoParams, lpName, MAKEINTRESOURCE(RT_GROUP_ICON));

			}
			else
			{
				goto endnoinc;
			}
		}
	}
	else
	{
		if (ptrIcoParams->loopcount >= ptrIcoParams->IconIndex && ptrIcoParams->loopcount < (ptrIcoParams->maxIcon + ptrIcoParams->IconIndex))
		{
			
				ExtractIconById(hmodule, ptrIcoParams, lpName, lptype);
		}
	}






	ptrIcoParams->loopcount = ptrIcoParams->loopcount + 1;
	endnoinc:
	if (ptrIcoParams->loopcount >= ptrIcoParams->maxIcon)
	{
		return 0;
	}
	return 1;
}

#define MAGIC_ICON       (WORD)0
#define MAGIC_ICO1       (WORD)1
#define MAGIC_CUR        (WORD)2
#define MAGIC_BMP        ((WORD)'B'+((WORD)'M'<<8))

#define MAGIC_ANI1       ((WORD)'R'+((WORD)'I'<<8))
#define MAGIC_ANI2       ((WORD)'F'+((WORD)'F'<<8))
#define MAGIC_ANI3       ((WORD)'A'+((WORD)'C'<<8))
#define MAGIC_ANI4       ((WORD)'O'+((WORD)'N'<<8))


UINT _ExtractFromICO(LPCTSTR pFileName, int iconIndex,
	int cxIcon, int cyIcon,
	HICON* phicon, UINT flags)
{


	if (iconIndex >= 1)
		return 0;

	flags |= LR_LOADFROMFILE;
	HICON hicon = (HICON)LoadImage(NULL, pFileName, IMAGE_ICON, cxIcon, cyIcon, flags);
	if (hicon == NULL)
		return 0;

	
	if (phicon == NULL)
		DestroyIcon(hicon);
	else
		*phicon = hicon;

	return 1;
}


UINT _ExtractFromBMP(LPCTSTR pFileName, int iconIndex,
	int cxIcon, int cyIcon,
	HICON* phicon, UINT flags)
{
	if (iconIndex >= 1)
		return 0;

	flags |= LR_LOADFROMFILE;
	HBITMAP hbm = (HBITMAP)LoadImage(NULL, pFileName, IMAGE_BITMAP,
		cxIcon, cyIcon, flags);
	if (hbm == NULL)
		return 0;

	if (phicon == NULL)
	{
		DeleteObject(hbm);
		return 1;
	}

	HBITMAP hbmMask = CreateBitmap(cxIcon, cyIcon, 1, 1, NULL);

	HDC hdc = CreateCompatibleDC(NULL);
	SelectObject(hdc, hbm);

	HDC hdcMask = CreateCompatibleDC(NULL);
	SelectObject(hdcMask, hbmMask);

	SetBkColor(hdc, GetPixel(hdc, 0, 0));
	
#define DSTERASE 0x00220326
	BitBlt(hdcMask, 0, 0, cxIcon, cyIcon, hdc, 0, 0, SRCCOPY);
	BitBlt(hdc, 0, 0, cxIcon, cyIcon, hdcMask, 0, 0, DSTERASE);

	ICONINFO ii;
	ii.fIcon = TRUE;
	ii.xHotspot = 0;
	ii.yHotspot = 0;
	ii.hbmColor = hbm;
	ii.hbmMask = hbmMask;
	HICON hicon = CreateIconIndirect(&ii);

	DeleteObject(hdc);
	DeleteObject(hbm);
	DeleteObject(hdcMask);
	DeleteObject(hbmMask);

	*phicon = hicon;
	return 1;
}


UINT mytools::MyPrivateExtractIcon(boost::filesystem::wpath filename, int nIconIndex, int cxIcon, int cyIcon, HICON* phicon, UINT* piconid, UINT nMaxIcon, UINT flags)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	
	
	
	filename.assign(mytools::AutoExpandEnviromentVariable(filename.wstring()));
	boost::filesystem::wpath interFilename = filename;
	if (filename.parent_path() == L"")
	{
		
		std::wstring buffer;
		//buffer.resize(32767);
		DWORD size = SearchPath(NULL, filename.filename().c_str(), NULL, 0, NULL, NULL);
		buffer.resize(size);
		DWORD searchres = SearchPath(NULL, filename.filename().c_str(), NULL, size, &buffer[0], (LPWSTR*)0x0);
		if (searchres == 0)
		{
			return 0;
		}
		interFilename = buffer;
		buffer.resize(0);
	}
	else
	{
		interFilename = mytools::AutoExpandEnviromentVariable(interFilename.wstring());
	}
	

	
	HMODULE hModule = NULL;
	hModule = LoadLibraryEx(interFilename.c_str(), (HANDLE)NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE | LOAD_LIBRARY_AS_DATAFILE);
	if (hModule != NULL)
	{
		EXTICONSTRUCT exic;
		exic.IconIndex = nIconIndex;
		exic.maxIcon = nMaxIcon;
		exic.phicon = phicon;
		exic.piconID = piconid;
		exic.cxIcon = cxIcon;
		exic.cyIcon = cyIcon;
		exic.flags = flags;
		exic.loopcount = 0;
		EnumResourceNamesExW((HMODULE)hModule, MAKEINTRESOURCE(RT_GROUP_ICON), (ENUMRESNAMEPROC)EmumNameResCallBack, (LONG_PTR)&exic, RESOURCE_ENUM_MUI | RESOURCE_ENUM_LN | RESOURCE_ENUM_VALIDATE, NULL);
		FreeLibrary(hModule);
		return exic.nIconCount;
	}
	HANDLE hf = CreateFile(interFilename.c_str(), GENERIC_READ | FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hf == INVALID_HANDLE_VALUE)
	{
		hf = CreateFile(interFilename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (hf == INVALID_HANDLE_VALUE)
			return 0;
	}
	else
	{
		FILETIME ftAccess;
		if (GetFileTime(hf, NULL, &ftAccess, NULL))
			SetFileTime(hf, NULL, &ftAccess, NULL);

	}
	WORD magic[6];
	DWORD read = 0;
	if (!ReadFile(hf, &magic, sizeof(magic), &read, NULL))
	{
		return 0;
	}
	if (read != sizeof(magic))
	{
		return 0;

	}
	UINT res = 0;
	switch (magic[0])
	{
	case MAGIC_ANI1:
		if (magic[1] == MAGIC_ANI2 &&
			magic[4] == MAGIC_ANI3 &&
			magic[5] == MAGIC_ANI4)
			res = _ExtractFromICO(interFilename.c_str(), nIconIndex, cxIcon, cyIcon, phicon, flags);
		break;

	case MAGIC_BMP:
		res = _ExtractFromBMP(interFilename.c_str(), nIconIndex, cxIcon, cyIcon, phicon, flags);
		break;

	case MAGIC_ICON:
		
		if ((magic[1] == 1 || magic[1] == MAGIC_CUR) &&
			magic[2] >= 1)
		{
			res = _ExtractFromICO(interFilename.c_str(), nIconIndex, cxIcon, cyIcon, phicon, flags);


		}
		break;
	}
	return res;

}


void mytools::ClearNullTerm(std::wstring& nullterminatedstring)
{
	while (nullterminatedstring.back() == L'\0')
	{
		nullterminatedstring.pop_back();
	}
}

bool mytools::IsFileExist(LPCWSTR path)
{
	if (path == NULL)
	{
		return false;
	}
	DWORD dwAttrib = GetFileAttributes(path);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
BOOL mytools::IsElevated() {
	BOOL fRet = FALSE;
	HANDLE hToken = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		TOKEN_ELEVATION Elevation;
		DWORD cbSize = sizeof(TOKEN_ELEVATION);
		if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
			fRet = Elevation.TokenIsElevated;
		}
	}
	if (hToken) {
		CloseHandle(hToken);
	}
	return fRet;
}
