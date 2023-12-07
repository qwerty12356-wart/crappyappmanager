


#pragma once
#include <Windows.h>

#pragma comment (lib,"Dbghelp.lib")

class ExtractIcons
{
public:
    ExtractIcons(void);
    ~ExtractIcons(void);

     
    static UINT Get( LPCTSTR pFileName, int iconIndex, 
                     int cxIcon, int cyIcon, 
                     HICON* phicon, UINT* piconid, 
                     UINT maxIcons, UINT flags /* LR_COLOR ( for full color )*/ );
private:
   

    enum
    {
        COM_FILE = 1,
        BAT_FILE,
        CMD_FILE,
        PIF_FILE,
        LNK_FILE,
        ICO_FILE,
        EXE_FILE,
    };

    static UINT _ExtractIcons( LPCTSTR pFileName, int iconIndex, 
                               int cxIcon, int cyIcon, 
                               HICON* phicon, UINT* piconid, 
                               UINT maxIcons, UINT flags );

    static UINT _ExtractFromExe( HANDLE hFile, int iconIndex, 
                                 int cxIconSize, int cyIconSize, 
                                 HICON *phicon, UINT *piconid, 
                                 UINT maxIcons, UINT flags );
    static UINT _ExtractFromBMP( LPCTSTR pFileName, int iconIndex, 
                                 int cxIcon, int cyIcon, 
                                 HICON* phicon, UINT flags );
    static UINT _ExtractFromICO( LPCTSTR pFileName, int iconIndex, 
                                 int cxIcon, int cyIcon, 
                                 HICON* phicon, UINT flags );


    static int _HasExtension( LPCTSTR pPath );
};
