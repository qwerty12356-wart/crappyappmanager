#include "CreateShortcut.h"
#include "tools.h"

HRESULT CreateLink(LPCWSTR lpszApppath, LPCWSTR name ,LPCWSTR lpszArgs, LPCWSTR lpszShortcutPos, LPCWSTR lpszDesc, LPCWSTR icondir, int niconindex, bool includextention)
{
    HRESULT hres;
    IShellLink* psl;

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf;

        psl->SetPath(mytools::AutoExpandEnviromentVariable(lpszApppath).c_str());
        psl->SetDescription(lpszDesc);
        psl->SetArguments(lpszArgs);
        psl->SetIconLocation(icondir, niconindex);
        
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

        if (SUCCEEDED(hres))
        {
            //WCHAR wsz[MAX_PATH];
            std::wstring internalbuffer;
            internalbuffer = mytools::AutoExpandEnviromentVariable(lpszShortcutPos);
            boost::filesystem::wpath internalpathbuff(lpszApppath);
            mytools::ClearNullTerm(internalbuffer);
            internalbuffer = internalbuffer + L'\\';
            if (name != NULL)
            {
                internalbuffer.append(std::wstring(name) + L".lnk");
            }
            else
            {
                if (includextention)
                {
                    internalbuffer.append(internalpathbuff.filename().wstring() + L".lnk");

                }
                else
                {
                    internalbuffer.append(internalpathbuff.stem().wstring() + L".lnk");

                }
            }

            // Ensure that the string is Unicode. 
           // MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);

            // Save the link by calling IPersistFile::Save. 
            hres = ppf->Save(internalbuffer.c_str(), TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    return hres;
}