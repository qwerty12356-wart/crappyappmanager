
#include "extracticon.h"
#include <crtdbg.h>
#include "log.h"
#include <DbgHelp.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
//TODO: Revamp This library due to it having several skill issue
#define MAX_PATH 32767
class SmartHANDLE
{
public:
    SmartHANDLE( HANDLE h ) { m_h = h; }
    ~SmartHANDLE( void ) { 
        if( m_h != INVALID_HANDLE_VALUE )
            ::CloseHandle( m_h ); 
    }
    operator HANDLE( void )  { return m_h; }
    HANDLE operator = ( HANDLE h )  
        { 
            if( m_h != INVALID_HANDLE_VALUE ) 
                ::CloseHandle( m_h );
            m_h = h;
            return m_h;
        }
    
private:
    HANDLE m_h;
};

class SmartFileMapping
{
public:
    SmartFileMapping( void* v ) { m_v = v; }
    ~SmartFileMapping( void ) { if( m_v != NULL ) ::UnmapViewOfFile( m_v ); }
    operator LPVOID( void )  { return m_v; }
    LPVOID ExtHandle()
    {
        return m_v;
    }
private:
    void* m_v;
};

const IMAGE_RESOURCE_DIRECTORY* find_entry_by_id(const IMAGE_RESOURCE_DIRECTORY* dir,
    WORD id, const void* root)
{
    
    const IMAGE_RESOURCE_DIRECTORY_ENTRY* entry;
    int min, max, pos;

    entry = (const IMAGE_RESOURCE_DIRECTORY_ENTRY*)(dir + 1);
    min = dir->NumberOfNamedEntries;
    max = min + dir->NumberOfIdEntries - 1;
    
    while (min <= max)
    {
        
        
        pos = (min + max) / 2;
        if (entry[pos].Id == id)
        {
            
            return (const IMAGE_RESOURCE_DIRECTORY*)((const char*)root + entry[pos].OffsetToDirectory);
        }
        if (entry[pos].Id > id) max = pos - 1;
        else min = pos + 1;
    }
    return NULL;
}
ExtractIcons::ExtractIcons(void)
{
}

ExtractIcons::~ExtractIcons(void)
{
}


#define MAGIC_ICON       (WORD)0
#define MAGIC_ICO1       (WORD)1
#define MAGIC_CUR        (WORD)2
#define MAGIC_BMP        ((WORD)'B'+((WORD)'M'<<8))

#define MAGIC_ANI1       ((WORD)'R'+((WORD)'I'<<8))
#define MAGIC_ANI2       ((WORD)'F'+((WORD)'F'<<8))
#define MAGIC_ANI3       ((WORD)'A'+((WORD)'C'<<8))
#define MAGIC_ANI4       ((WORD)'O'+((WORD)'N'<<8))

#define VER30            0x00030000
#define _T(a) L##a

int ExtractIcons::_HasExtension( LPCTSTR pPath )
{
    LPCTSTR p = wcsrchr( pPath, L'.' );
    if( p == NULL )
        return 0;
    if( lstrcmpi( p, _T(".com")) == 0 ) return COM_FILE;
    if( lstrcmpi( p, _T(".bat")) == 0 ) return BAT_FILE;
    if( lstrcmpi( p, _T(".cmd")) == 0 ) return CMD_FILE;
    if( lstrcmpi( p, _T(".pif")) == 0 ) return PIF_FILE;
    if( lstrcmpi( p, _T(".lnk")) == 0 ) return LNK_FILE;
    if( lstrcmpi( p, _T(".ico")) == 0 ) return ICO_FILE;
    if( lstrcmpi( p, _T(".exe")) == 0 ) return EXE_FILE;
    
    return 0;
}

const IMAGE_RESOURCE_DIRECTORY* find_entry_default(const IMAGE_RESOURCE_DIRECTORY* dir,
    const void* root)
{
    const IMAGE_RESOURCE_DIRECTORY_ENTRY* entry;
    entry = (const IMAGE_RESOURCE_DIRECTORY_ENTRY*)(dir + 1);
    return (const IMAGE_RESOURCE_DIRECTORY*)((const char*)root + entry->OffsetToDirectory);
}

PIMAGE_SECTION_HEADER WINAPI RtlImageRvaToSection(const IMAGE_NT_HEADERS* nt,
    HMODULE module, DWORD rva)
{
    int i;
    const IMAGE_SECTION_HEADER* sec;

    sec = (const IMAGE_SECTION_HEADER*)((const char*)&nt->OptionalHeader +
        nt->FileHeader.SizeOfOptionalHeader);
    for (i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++)
    {
        if ((sec->VirtualAddress <= rva) && (sec->VirtualAddress + sec->SizeOfRawData > rva))
            return (PIMAGE_SECTION_HEADER)sec;
    }
    return NULL;
}


PVOID WINAPI RtlImageRvaToVa(const IMAGE_NT_HEADERS* nt, HMODULE module,
    DWORD rva, IMAGE_SECTION_HEADER** section)
{
    IMAGE_SECTION_HEADER* sec;

    if (section && *section)  /* try this section first */
    {
        sec = *section;
        if ((sec->VirtualAddress <= rva) && (sec->VirtualAddress + sec->SizeOfRawData > rva))
            goto found;
    }
    if (!(sec = RtlImageRvaToSection(nt, module, rva))) return NULL;
found:
    if (section) *section = sec;
    return (char*)module + sec->PointerToRawData + (rva - sec->VirtualAddress);
}



PVOID WINAPI RtlImageDirectoryEntryToData(HMODULE module, BOOL image, WORD dir, ULONG* size)
{
    IMAGE_NT_HEADERS* nt;
    DWORD addr;

    if ((ULONG_PTR)module & 1) image = FALSE;  /* mapped as data file */
    module = (HMODULE)((ULONG_PTR)module & ~3);
    if (!(nt = ImageNtHeader(module))) return NULL;
    if (nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    {
        const IMAGE_NT_HEADERS64* nt64 = (const IMAGE_NT_HEADERS64*)nt;

        if (dir >= nt64->OptionalHeader.NumberOfRvaAndSizes) return NULL;
        if (!(addr = nt64->OptionalHeader.DataDirectory[dir].VirtualAddress)) return NULL;
        *size = nt64->OptionalHeader.DataDirectory[dir].Size;
        if (image || addr < nt64->OptionalHeader.SizeOfHeaders) return (char*)module + addr;
    }
    else if (nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
    {
        const IMAGE_NT_HEADERS32* nt32 = (const IMAGE_NT_HEADERS32*)nt;

        if (dir >= nt32->OptionalHeader.NumberOfRvaAndSizes) return NULL;
        if (!(addr = nt32->OptionalHeader.DataDirectory[dir].VirtualAddress)) return NULL;
        *size = nt32->OptionalHeader.DataDirectory[dir].Size;
        if (image || addr < nt32->OptionalHeader.SizeOfHeaders) return (char*)module + addr;
    }
    else return NULL;

    /* not mapped as image, need to find the section containing the virtual address */
    return RtlImageRvaToVa(nt, module, addr, NULL);
}



UINT ExtractIcons::_ExtractFromExe( HANDLE hFile, int iconIndex, 
                                   int cxIconSize, int cyIconSize, 
                                   HICON *phicon, UINT *piconid, 
                                   UINT maxIcons, UINT flags )
{
    
    SmartHANDLE hFileMap(CreateFileMapping(hFile, NULL, PAGE_READONLY | SEC_COMMIT, 0,0, NULL));
    SmartFileMapping peImgroot(MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0));
    UINT cx1 = LOWORD(cxIconSize);
    UINT cx2 = HIWORD(cxIconSize);
    UINT cy1 = LOWORD(cyIconSize);
    UINT cy2 = HIWORD(cyIconSize);
    if (piconid) 
        *piconid = 0xFFFFFFFF;

    if (!piconid) 
        piconid = (UINT*)phicon;
    UINT16 iconamount;
    const IMAGE_RESOURCE_DATA_ENTRY* idataentry, * igdataentry;
    const IMAGE_RESOURCE_DIRECTORY* rootresourcedir, * iconresourcedir = nullptr, * icongroupresourcedir ;
    ULONG size;
    ULONG i;
    BYTE* idata, * igdata;
    rootresourcedir = (IMAGE_RESOURCE_DIRECTORY*)ImageDirectoryEntryToDataEx(peImgroot.ExtHandle(), FALSE, IMAGE_DIRECTORY_ENTRY_RESOURCE, &size, NULL);
    if (!rootresourcedir)
    {
       
        return 0;
    }
    icongroupresourcedir = find_entry_by_id(rootresourcedir, LOWORD(RT_GROUP_ICON), rootresourcedir);
    if (!icongroupresourcedir)
    {
       
        return 0;
    }
    iconamount = icongroupresourcedir->NumberOfNamedEntries + icongroupresourcedir->NumberOfIdEntries;
    if (!phicon)
    {
        return iconamount;
    }
    if (iconIndex < 0)
    {
        int n = 0;
        int iId = abs(iconIndex);
        const IMAGE_RESOURCE_DIRECTORY_ENTRY* xprdeTmp = (const IMAGE_RESOURCE_DIRECTORY_ENTRY*)(icongroupresourcedir + 1);

        while (n < iconamount && xprdeTmp)
        {
            if (xprdeTmp->Id == iId)
            {
                iconIndex = n;
                break;
            }
            n++;
            xprdeTmp++;
        }
        if (iconIndex < 0)
        {
            return 0;
        }
    }
    else
    {
        if (iconIndex >= iconamount)
        {
            return 0;
        }
    }
    if (maxIcons > iconamount - iconIndex)
    {
        maxIcons = iconamount - iconIndex;
    }
    const IMAGE_RESOURCE_DIRECTORY_ENTRY* rediren = (const IMAGE_RESOURCE_DIRECTORY_ENTRY*)(icongroupresourcedir + 1) + iconIndex;
    for ( i = 0; i < maxIcons; i++, rediren++)
    {
        const IMAGE_RESOURCE_DIRECTORY* resdir = (const IMAGE_RESOURCE_DIRECTORY*)((const char*)rootresourcedir + rediren->OffsetToDirectory);
        resdir = find_entry_default(resdir, rootresourcedir);
        igdataentry = (const IMAGE_RESOURCE_DATA_ENTRY*)resdir;
        igdata = (BYTE*)ImageRvaToVa(ImageNtHeader((HMODULE)peImgroot.ExtHandle()), (HMODULE)peImgroot.ExtHandle(), igdataentry->OffsetToData, NULL);
        if (!igdata)
        {
            return 0;
        }

        piconid[i] = LookupIconIdFromDirectoryEx(igdata, TRUE, cx1, cy1, flags);
        if (cx2 && cy2) piconid[++ i] = LookupIconIdFromDirectoryEx(igdata, TRUE, cx2, cy2, flags);
    }
    
    if (!(iconresourcedir = find_entry_by_id(rootresourcedir, LOWORD(RT_ICON), rootresourcedir)))
    {
        return 0;
    }
    
    for ( i = 0; i < maxIcons; i++)
    {
        const IMAGE_RESOURCE_DIRECTORY* xresdir;
        xresdir = find_entry_by_id(iconresourcedir, LOWORD(piconid[i]), rootresourcedir);
        if (!xresdir)
        {
            
            phicon[i] = 0;
            continue;
        }
        xresdir = find_entry_default(xresdir, rootresourcedir);
        idataentry = (const IMAGE_RESOURCE_DATA_ENTRY*)xresdir;
        idata = (BYTE*)ImageRvaToVa(ImageNtHeader((HMODULE)peImgroot.ExtHandle()), (HMODULE)peImgroot.ExtHandle(), idataentry->OffsetToData, NULL);

        if (!idata)
        {
            
            phicon[i] = 0;
            continue;
        }
        
        phicon[i] = CreateIconFromResourceEx(idata, idataentry->Size, TRUE, 0x00030000, cx1, cy1, flags);
        
        if (cx2 && cy2)
            phicon[++i] = CreateIconFromResourceEx(idata, idataentry->Size, TRUE, 0x00030000, cx2, cy2, flags);
    
    }
    return i;
}

UINT ExtractIcons::_ExtractFromBMP( LPCTSTR pFileName, int iconIndex, 
                                   int cxIcon, int cyIcon, 
                                   HICON* phicon, UINT flags )
{
    if( iconIndex >= 1 )
        return 0;

    flags |= LR_LOADFROMFILE;
    HBITMAP hbm = (HBITMAP)LoadImage( NULL, pFileName, IMAGE_BITMAP,
                                      cxIcon, cyIcon, flags );
    if( hbm == NULL )
        return 0;

    if( phicon == NULL )
    {
        DeleteObject(hbm);
        return 1;
    }

    HBITMAP hbmMask = CreateBitmap( cxIcon, cyIcon, 1, 1, NULL );

    HDC hdc = CreateCompatibleDC(NULL);
    SelectObject( hdc, hbm );

    HDC hdcMask = CreateCompatibleDC(NULL);
    SelectObject(hdcMask, hbmMask);

    SetBkColor( hdc, GetPixel(hdc, 0, 0) );
// this ROP Code will leave bits in the destination bitmap the same color if the
// corresponding source bitmap's bit are black.
// all other bits in the destination (where source bits are not black)
// are turned to black.
#define DSTERASE 0x00220326 // dest = dest & (~src) :
    BitBlt( hdcMask, 0, 0, cxIcon, cyIcon, hdc, 0, 0, SRCCOPY );
    BitBlt( hdc, 0, 0, cxIcon, cyIcon, hdcMask, 0, 0, DSTERASE );

    ICONINFO ii;
    ii.fIcon    = TRUE;
    ii.xHotspot = 0;
    ii.yHotspot = 0;
    ii.hbmColor = hbm;
    ii.hbmMask  = hbmMask;
    HICON hicon = CreateIconIndirect( &ii );

    DeleteObject(hdc);
    DeleteObject(hbm);
    DeleteObject(hdcMask);
    DeleteObject(hbmMask);

    *phicon = hicon;
    return 1;
}

UINT ExtractIcons::_ExtractFromICO( LPCTSTR pFileName, int iconIndex, 
                                   int cxIcon, int cyIcon, 
                                   HICON* phicon, UINT flags )
{


    if( iconIndex >= 1 )
        return 0;

    flags |= LR_LOADFROMFILE;
    HICON hicon = (HICON)LoadImage( NULL, pFileName, IMAGE_ICON, cxIcon, cyIcon, flags );
    if( hicon == NULL )
        return 0;

    //  do we just want a count?
    if( phicon == NULL )
        DestroyIcon( hicon );
    else
        *phicon = hicon;

    return 1;
}

UINT ExtractIcons::_ExtractIcons( LPCTSTR pFileName, int iconIndex, 
                                 int cxIcon, int cyIcon, 
                                 HICON* phicon, UINT* piconid, UINT maxIcons, UINT flags )
{
    SmartHANDLE hFile( INVALID_HANDLE_VALUE );

    if( phicon != NULL )
        *phicon = NULL;
    if( piconid != NULL )
        *piconid = (UINT)-1;

    std::wstring fileName(pFileName);
    fileName.resize(MAX_PATH);
    

  
    switch( _HasExtension( fileName.c_str() ) )
    {
        case COM_FILE:
        case BAT_FILE:
        case CMD_FILE:
        case PIF_FILE:
        case LNK_FILE:
            return 0;

        default:
            break;
    }

   
    std::wstring filenames;
    filenames.resize(MAX_PATH);
    ExpandEnvironmentStrings(fileName.c_str(), &filenames[0], MAX_PATH);


    std::wstring expFileName(filenames);
    
    if( SearchPath( NULL, expFileName.c_str(), NULL, MAX_PATH, &fileName[0], NULL) == 0)
        return 0; // error...
    expFileName.clear();
    hFile = CreateFile( filenames.c_str(), GENERIC_READ | FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if( hFile == INVALID_HANDLE_VALUE )
    {
        hFile = CreateFile( filenames.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

        if( hFile == INVALID_HANDLE_VALUE )
            return 0;
    }
    else
    {
        // Restore the Access Date
        FILETIME ftAccess;
        if( GetFileTime( hFile, NULL, &ftAccess, NULL ) )
            SetFileTime( hFile, NULL, &ftAccess, NULL );
    }


    WORD magic[6];
    DWORD read = 0;
    if( ! ReadFile( hFile,&magic, sizeof(magic), &read, NULL ) )
        return 0;
    if( read != sizeof(magic) )
        return 0;

    UINT res = 0;
    switch( magic[0] )
    {
        case IMAGE_DOS_SIGNATURE:
            res = _ExtractFromExe( hFile, iconIndex, cxIcon, cyIcon, phicon, piconid, maxIcons, flags );
           
        break;

        case MAGIC_ANI1: //  ani cursors are RIFF file of type 'ACON'
            if( magic[1] == MAGIC_ANI2 && 
                magic[4] == MAGIC_ANI3 &&
                magic[5] == MAGIC_ANI4 )
                res = _ExtractFromICO( fileName.c_str(), iconIndex, cxIcon, cyIcon, phicon, flags);
        break;

        case MAGIC_BMP:    
            res = _ExtractFromBMP( fileName.c_str(), iconIndex, cxIcon, cyIcon, phicon, flags);
        break;

        case MAGIC_ICON:    
            //  icons and cursors format:
            //      reserved       : always zero
            //      resourceType   : 1 for icons 2 cor cursor.
            //      countIcons       : images count
            // we want countIcons >= 1
            if( (magic[1] == 1 || magic[1] == MAGIC_CUR ) &&
                magic[2] >= 1 )
            {
                res = _ExtractFromICO( fileName.c_str(), iconIndex, cxIcon, cyIcon, phicon, flags);

                
            }
        break;
    }

    return res;
}

UINT ExtractIcons::Get( LPCTSTR pFileName, int iconIndex, 
                    int cxIcon, int cyIcon, 
                    HICON* phicon, UINT* piconid, 
                    UINT maxIcons, UINT flags )
{
    return _ExtractIcons( pFileName, iconIndex, 
                          cxIcon, cyIcon, 
                          phicon, piconid, 
                          maxIcons, flags );
}
