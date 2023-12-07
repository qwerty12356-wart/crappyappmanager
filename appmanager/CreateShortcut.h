#pragma once
#include <Windows.h>
#include <shlobj.h>
#include <winnls.h>
#include <shobjidl.h>
#include <objbase.h>
#include <objidl.h>
#include <shlguid.h>

HRESULT CreateLink(LPCWSTR lpszApppath,LPCWSTR name,LPCWSTR lpszArgs ,LPCWSTR lpszShortcutPos, LPCWSTR lpszDesc, LPCWSTR icondir, int ncionindex, bool includextention);

