/**********************************************************************

Filename    :   SysFile.cpp
Content     :   File wrapper class implementation (Win32)

Created     :   April 5, 1999
Authors     :   Michael Antonov

History     :   8/27/2001 MA    Reworked file and directory interface

Copyright   :   (c) 1999-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#define  GFILE_CXX

// Standard C library (Captain Obvious guarantees!)
#include <stdio.h>

#include "SF_SysFile.h"
#include "SF_HeapNew.h"

namespace Scaleform {

// This is - a dummy file that fails on all calls.

class UnopenedFile : public File
{
public:
    UnopenedFile()  { }
    ~UnopenedFile() { }

    virtual const char* GetFilePath()               { return 0; }

    // ** File Information
    virtual bool        IsValid()                   { return 0; }
    virtual bool        IsWritable()                { return 0; }
    //virtual bool      IsRecoverable()             { return 0; }
    // Return position / file size
    virtual int         Tell()                      { return 0; }
    virtual SInt64      LTell()                     { return 0; }
    virtual int         GetLength()                 { return 0; }
    virtual SInt64      LGetLength()                { return 0; }

//  virtual bool        Stat(FileStats *pfs)        { return 0; }
    virtual int         GetErrorCode()              { return Error_FileNotFound; }

    // ** Stream implementation & I/O
    virtual int         Write(const UByte *pbuffer, int numBytes)     { return -1; SF_UNUSED2(pbuffer, numBytes); }
    virtual int         Read(UByte *pbuffer, int numBytes)            { return -1; SF_UNUSED2(pbuffer, numBytes); }
    virtual int         SkipBytes(int numBytes)                       { return 0;  SF_UNUSED(numBytes); }
    virtual int         BytesAvailable()                              { return 0; }
    virtual bool        Flush()                                       { return 0; }
    virtual int         Seek(int offset, int origin)                  { return -1; SF_UNUSED2(offset, origin); }
    virtual SInt64      LSeek(SInt64 offset, int origin)              { return -1; SF_UNUSED2(offset, origin); }

    virtual bool        ChangeSize(int newSize)                       { return 0;  SF_UNUSED(newSize); }
    virtual int         CopyFromStream(File *pstream, int byteSize)   { return -1; SF_UNUSED2(pstream, byteSize); }
    virtual bool        Close()                                       { return 0; }
    virtual bool        CloseCancel()                                 { return 0; }
};



// ***** System File

// System file is created to access objects on file system directly
// This file can refer directly to path

// ** Constructor
SysFile::SysFile() : DelegatedFile(0)
{
    pFile = *new UnopenedFile;
}

#ifdef SF_OS_WII
File* FileWiiDvdOpen(const char *ppath, int flags=File::Open_Read);
#elif defined(SF_OS_3DS)
File* File3DSOpen(const char *ppath, int flags=File::Open_Read);
#else
File* FileFILEOpen(const String& path, int flags, int mode);
#endif

// Opens a file
SysFile::SysFile(const String& path, int flags, int mode) : DelegatedFile(0)
{
    Open(path, flags, mode);
}


// ** Open & management
// Will fail if file's already open
bool    SysFile::Open(const String& path, int flags, int mode)
{
#ifdef SF_OS_WII
    pFile = *FileWiiDvdOpen(path.ToCStr());
#elif defined(SF_OS_3DS)
    pFile = *File3DSOpen(path.ToCStr(), flags);
#else
    pFile = *FileFILEOpen(path, flags, mode);
#endif
    if ((!pFile) || (!pFile->IsValid()))
    {
        pFile = *SF_NEW UnopenedFile;
        return 0;
    }
#if defined(SF_OS_WII)
    pFile = *SF_NEW BufferedFile(pFile);
#else    
    //pFile = *SF_NEW DelegatedFile(pFile); // MA Testing
    if (flags & Open_Buffered)
        pFile = *SF_NEW BufferedFile(pFile);
#endif
    return 1;
}


// ** Overrides

int     SysFile::GetErrorCode()
{
    return pFile ? pFile->GetErrorCode() : Error_FileNotFound;
}


// Overrides to provide re-open support
bool    SysFile::IsValid()
{
    return pFile && pFile->IsValid();
}
bool    SysFile::Close()
{
    if (IsValid())
    {
        DelegatedFile::Close();
        pFile = *SF_NEW UnopenedFile;
        return 1;
    }
    return 0;
}

/*
bool    SysFile::CloseCancel()
{
    if (IsValid())
    {
        BufferedFile::CloseCancel();
        pFile = *SF_NEW UnopenedFile;
        return 1;
    }
    return 0;
}
*/

} // Scaleform
