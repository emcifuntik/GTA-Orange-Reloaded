/**********************************************************************

Filename    :   FileOpener.cpp
Content     :   
Created     :   April 15, 2008
Authors     :   

Notes       :   Redesigned to use inherited state objects for GFx 2.2.

Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFx/GFx_Loader.h"
#include "Kernel/SF_SysFile.h"
#include "GFx/GFx_Log.h"

namespace Scaleform { namespace GFx {

// ***** FileOpener implementation

// Default implementation - use SysFile.
File* FileOpener::OpenFile(const char *purl, int flags, int modes)
{
    // Buffered file wrapper is faster to use because it optimizes seeks.
    return new SysFile(purl, flags, modes);
}

SInt64  FileOpener::GetFileModifyTime(const char *purl)
{
    FileStat fileStat;
    if (SysFile::GetFileStat(&fileStat, purl))
        return fileStat.ModifyTime; 
    return -1;
}


// Implementation that allows us to override the log.
File*      FileOpener::OpenFileEx(const char *pfilename, Log *plog, 
                                  int  flags, int modes)
{   
    File*  pin = OpenFile(pfilename, flags, modes);
    int  errCode = 16;
    if (pin)
        errCode = pin->GetErrorCode();

    if (!pin || errCode)
    {
        if (plog)
            plog->LogError("Loader failed to open '%s'", pfilename);
        if (pin)
        {
            //plog->LogError("Code '%d'", errCode);
            pin->Release();
        }
        return 0;
    }
    return pin;
}

}} // namespace Scaleform { namespace GFx {

