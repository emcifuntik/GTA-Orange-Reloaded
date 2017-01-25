/**********************************************************************

Filename    :   FileFILE.cpp
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

#include "SF_Types.h"
#include "SF_Debug.h"

// Standard C library (Captain Obvious guarantees!)
#include <stdio.h>
#ifndef SF_OS_WINCE
#include <sys/stat.h>
#endif

#include "SF_SysFile.h"

#ifndef SF_OS_WINCE
#include <errno.h>
#endif

// unistd provides stat() function
#ifdef SF_OS_PS3
#include <unistd.h>
#endif

#include "SF_HeapNew.h"

namespace Scaleform {

// ***** File interface

// ***** FILEFile - C streams file

static int SFerror ()
{
#if !defined(SF_OS_WINCE) && !defined(SF_CC_RENESAS)
    if (errno == ENOENT)
        return FileConstants::Error_FileNotFound;
    else if (errno == EACCES || errno == EPERM)
        return FileConstants::Error_Access;
    else if (errno == ENOSPC)
        return FileConstants::Error_DiskFull;
    else
#endif
        return FileConstants::Error_IOError;
};

#ifdef SF_OS_WIN32
#include "windows.h"
// A simple helper class to disable/enable system error mode, if necessary
// Disabling happens conditionally only if a drive name is involved
class SysErrorModeDisabler
{
    BOOL    Disabled;
    UINT    OldMode;
public:
    SysErrorModeDisabler(const char* pfileName)
    {
        if (pfileName && (pfileName[0]!=0) && pfileName[1]==':')
        {
            Disabled = 1;
            OldMode = ::SetErrorMode(SEM_FAILCRITICALERRORS);
        }
        else
            Disabled = 0;
    }

    ~SysErrorModeDisabler()
    {
        if (Disabled) ::SetErrorMode(OldMode);
    }
};
#else
class SysErrorModeDisabler
{
public:
    SysErrorModeDisabler(const char* pfileName) { }
};
#endif // SF_OS_WIN32


// This macro enables verification of I/O results after seeks against a pre-loaded
// full file buffer copy. This is generally not necessary, but can been used to debug
// memory corruptions; we've seen this fail due to EAX2/DirectSound corrupting memory
// under FMOD with XP64 (32-bit) and Realtek HA Audio driver.
//#define GFILE_VERIFY_SEEK_ERRORS


// This is the simplest possible file implementation, it wraps around the descriptor
// This file is delegated to by SysFile.

class FILEFile : public File
{
protected:

    // Allocated filename
    String      FileName;

    // File handle & open mode
    bool        Opened;
    FILE*       fs;
    int         OpenFlags;
    // Error code for last request
    int         ErrorCode;

    int         LastOp;

#ifdef SF_FILE_VERIFY_SEEK_ERRORS
    UByte*      pFileTestBuffer;
    unsigned    FileTestLength;
    unsigned    TestPos; // File pointer position during tests.
#endif

public:

    FILEFile()
    {
        Opened = 0; FileName = "";

#ifdef SF_FILE_VERIFY_SEEK_ERRORS
        pFileTestBuffer =0;
        FileTestLength  =0;
        TestPos         =0;
#endif
    }
    // Initialize file by opening it
    FILEFile(const String& fileName, int flags, int Mode);
    // The 'pfileName' should be encoded as UTF-8 to support international file names.
    FILEFile(const char* pfileName, int flags, int Mode);

    ~FILEFile()
    {
        if (Opened)
            Close();
    }

    virtual const char* GetFilePath();

    // ** File Information
    virtual bool        IsValid();
    virtual bool        IsWritable();
    //virtual bool      IsRecoverable();
    // Return position / file size
    virtual int         Tell();
    virtual SInt64      LTell();
    virtual int         GetLength();
    virtual SInt64      LGetLength();

//  virtual bool        Stat(FileStats *pfs);
    virtual int         GetErrorCode();

    // ** Stream implementation & I/O
    virtual int         Write(const UByte *pbuffer, int numBytes);
    virtual int         Read(UByte *pbuffer, int numBytes);
    virtual int         SkipBytes(int numBytes);
    virtual int         BytesAvailable();
    virtual bool        Flush();
    virtual int         Seek(int offset, int origin);
    virtual SInt64      LSeek(SInt64 offset, int origin);

    virtual bool        ChangeSize(int newSize);
    virtual int         CopyFromStream(File *pStream, int byteSize);
    virtual bool        Close();
    //virtual bool      CloseCancel();
private:
    void                init();
};


// Initialize file by opening it
FILEFile::FILEFile(const String& fileName, int flags, int mode)
  : FileName(fileName), OpenFlags(flags)
{
    SF_UNUSED(mode);
    init();
}

// The 'pfileName' should be encoded as UTF-8 to support international file names.
FILEFile::FILEFile(const char* pfileName, int flags, int mode)
  : FileName(pfileName), OpenFlags(flags)
{
    SF_UNUSED(mode);
    init();
}

void FILEFile::init()
{
    // Open mode for file's open
    const char *omode = "rb";

    if (OpenFlags & Open_Truncate)
    {
        if (OpenFlags & Open_Read)
            omode = "w+b";
        else
            omode = "wb";
    }
    else if (OpenFlags & Open_Create)
    {
        if (OpenFlags & Open_Read)
            omode = "a+b";
        else
            omode = "ab";
    }
    else if (OpenFlags & Open_Write)
        omode = "r+b";

#ifdef SF_OS_WIN32
    SysErrorModeDisabler disabler(FileName.ToCStr());
#endif

#if defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400) && !defined(SF_OS_WINCE)
    wchar_t womode[16];
    wchar_t *pwFileName = (wchar_t*)SF_ALLOC((UTF8Util::GetLength(FileName.ToCStr())+1) * sizeof(wchar_t), Stat_Default_Mem);
    UTF8Util::DecodeString(pwFileName, FileName.ToCStr());
    SF_ASSERT(strlen(omode) < sizeof(womode)/sizeof(womode[0]));
    UTF8Util::DecodeString(womode, omode);
    _wfopen_s(&fs, pwFileName, womode);
    SF_FREE(pwFileName);
#else
    fs = fopen(FileName.ToCStr(), omode);
#endif
#ifdef SF_OS_WINCE
    if (fs)
        fseek(fs, 0, SEEK_SET);
#else
    if (fs)
        rewind (fs);
#endif
    Opened = (fs != NULL);
    // Set error code
    if (!Opened)
        ErrorCode = SFerror();
    else
    {
        // If we are testing file seek correctness, pre-load the entire file so
        // that we can do comparison tests later.
#ifdef SF_FILE_VERIFY_SEEK_ERRORS        
        TestPos         = 0;
        fseek(fs, 0, SEEK_END);
        FileTestLength  = ftell(fs);
        fseek(fs, 0, SEEK_SET);
        pFileTestBuffer = (UByte*)SF_HEAP_AUTO_ALLOC(this, FileTestLength);        
        if (pFileTestBuffer)
        {
            SF_ASSERT(FileTestLength == (unsigned)Read(pFileTestBuffer, FileTestLength));
            Seek(0, Seek_Set);
        }        
#endif

        ErrorCode = 0;
    }
    LastOp = 0;
}


const char* FILEFile::GetFilePath()
{
    return FileName.ToCStr();
}


// ** File Information
bool    FILEFile::IsValid()
{
    return Opened;
}
bool    FILEFile::IsWritable()
{
    return IsValid() && (OpenFlags&Open_Write);
}
/*
bool    FILEFile::IsRecoverable()
{
    return IsValid() && ((OpenFlags&SF_FO_SAFETRUNC) == SF_FO_SAFETRUNC);
}
*/

// Return position / file size
int     FILEFile::Tell()
{
    int pos = ftell (fs);
    if (pos < 0)
        ErrorCode = SFerror();
    return pos;
}

SInt64  FILEFile::LTell()
{
    int pos = ftell(fs);
    if (pos < 0)
        ErrorCode = SFerror();
    return pos;
}

int     FILEFile::GetLength()
{
    int pos = Tell();
    if (pos >= 0)
    {
        Seek (0, Seek_End);
        int size = Tell();
        Seek (pos, Seek_Set);
        return size;
    }
    return -1;
}
SInt64  FILEFile::LGetLength()
{
    return GetLength();
}

int     FILEFile::GetErrorCode()
{
    return ErrorCode;
}

// ** Stream implementation & I/O
int     FILEFile::Write(const UByte *pbuffer, int numBytes)
{
    if (LastOp && LastOp != Open_Write)
        fflush(fs);
    LastOp = Open_Write;
    int written = (int) fwrite(pbuffer, 1, numBytes, fs);
    if (written < numBytes)
        ErrorCode = SFerror();

#ifdef SF_FILE_VERIFY_SEEK_ERRORS
    if (written > 0)
        TestPos += written;
#endif

    return written;
}

int     FILEFile::Read(UByte *pbuffer, int numBytes)
{
    if (LastOp && LastOp != Open_Read)
        fflush(fs);
    LastOp = Open_Read;
    int read = (int) fread(pbuffer, 1, numBytes, fs);
    if (read < numBytes)
        ErrorCode = SFerror();

#ifdef SF_FILE_VERIFY_SEEK_ERRORS
    if (read > 0)
    {
        // Read-in data must match our pre-loaded buffer data!
        UByte* pcompareBuffer = pFileTestBuffer + TestPos;
        for (int i=0; i< read; i++)
        {
            SF_ASSERT(pcompareBuffer[i] == pbuffer[i]);
        }

        //SF_ASSERT(!memcmp(pFileTestBuffer + TestPos, pbuffer, read));
        TestPos += read;
        SF_ASSERT(ftell(fs) == (int)TestPos);
    }
#endif

    return read;
}

// Seeks ahead to skip bytes
int     FILEFile::SkipBytes(int numBytes)
{
    SInt64 pos    = LTell();
    SInt64 newPos = LSeek(numBytes, Seek_Cur);

    // Return -1 for major error
    if ((pos==-1) || (newPos==-1))
    {
        return -1;
    }
    //ErrorCode = ((NewPos-Pos)<numBytes) ? errno : 0;

    return int (newPos-(int)pos);
}

// Return # of bytes till EOF
int     FILEFile::BytesAvailable()
{
    SInt64 pos    = LTell();
    SInt64 endPos = LGetLength();

    // Return -1 for major error
    if ((pos==-1) || (endPos==-1))
    {
        ErrorCode = SFerror();
        return 0;
    }
    else
        ErrorCode = 0;

    return int (endPos-(int)pos);
}

// Flush file contents
bool    FILEFile::Flush()
{
    return !fflush(fs);
}

int     FILEFile::Seek(int offset, int origin)
{
    int newOrigin = 0;
    switch(origin)
    {
    case Seek_Set: newOrigin = SEEK_SET; break;
    case Seek_Cur: newOrigin = SEEK_CUR; break;
    case Seek_End: newOrigin = SEEK_END; break;
    }

    if (newOrigin == SEEK_SET && offset == Tell())
        return Tell();

    if (fseek (fs, offset, newOrigin))
    {
#ifdef SF_FILE_VERIFY_SEEK_ERRORS
        SF_ASSERT(0);
#endif
        return -1;
    }
    
#ifdef SF_FILE_VERIFY_SEEK_ERRORS
    // Track file position after seeks for read verification later.
    switch(origin)
    {
    case Seek_Set:  TestPos = offset;       break;
    case Seek_Cur:  TestPos += offset;      break;    
    case Seek_End:  TestPos = FileTestLength + offset; break;
    }
    SF_ASSERT((int)TestPos == Tell());
#endif

    return (int)Tell();
}

SInt64  FILEFile::LSeek(SInt64 offset, int origin)
{
    return Seek((int)offset,origin);
}

bool    FILEFile::ChangeSize(int newSize)
{
    SF_UNUSED(newSize);
    SF_DEBUG_WARNING(1, "File::ChangeSize not supported");
    ErrorCode = Error_IOError;
    return 0;
}

int     FILEFile::CopyFromStream(File *pstream, int byteSize)
{
    UByte   buff[0x4000];
    int     count = 0;
    int     szRequest, szRead, szWritten;

    while (byteSize)
    {
        szRequest = (byteSize > int(sizeof(buff))) ? int(sizeof(buff)) : byteSize;

        szRead    = pstream->Read(buff, szRequest);
        szWritten = 0;
        if (szRead > 0)
            szWritten = Write(buff, szRead);

        count    += szWritten;
        byteSize -= szWritten;
        if (szWritten < szRequest)
            break;
    }
    return count;
}


bool    FILEFile::Close()
{
#ifdef SF_FILE_VERIFY_SEEK_ERRORS
    if (pFileTestBuffer)
    {
        SF_FREE(pFileTestBuffer);
        pFileTestBuffer = 0;
        FileTestLength  = 0;
    }
#endif

    bool closeRet = !fclose(fs);

    if (!closeRet)
    {
        ErrorCode = SFerror();
        return 0;
    }
    else
    {
        Opened    = 0;
        fs        = 0;
        ErrorCode = 0;
    }

    // Handle safe truncate
    /*
    if ((OpenFlags & SF_FO_SAFETRUNC) == SF_FO_SAFETRUNC)
    {
        // Delete original file (if it existed)
        DWORD oldAttributes = FileUtilWin32::GetFileAttributes(FileName);
        if (oldAttributes!=0xFFFFFFFF)
            if (!FileUtilWin32::DeleteFile(FileName))
            {
                // Try to remove the readonly attribute
                FileUtilWin32::SetFileAttributes(FileName, oldAttributes & (~FILE_ATTRIBUTE_READONLY) );
                // And delete the file again
                if (!FileUtilWin32::DeleteFile(FileName))
                    return 0;
            }

        // Rename temp file to real filename
        if (!FileUtilWin32::MoveFile(TempName, FileName))
        {
            //ErrorCode = errno;
            return 0;
        }
    }
    */
    return 1;
}

/*
bool    FILEFile::CloseCancel()
{
    bool closeRet = (bool)::CloseHandle(fd);

    if (!closeRet)
    {
        //ErrorCode = errno;
        return 0;
    }
    else
    {
        Opened    = 0;
        fd        = INVALID_HANDLE_VALUE;
        ErrorCode = 0;
    }

    // Handle safe truncate (delete tmp file, leave original unchanged)
    if ((OpenFlags&SF_FO_SAFETRUNC) == SF_FO_SAFETRUNC)
        if (!FileUtilWin32::DeleteFile(TempName))
        {
            //ErrorCode = errno;
            return 0;
        }
    return 1;
}
*/

File *FileFILEOpen(const String& path, int flags, int mode)
{
    return SF_NEW FILEFile(path, flags, mode);
}

#if !defined(SF_OS_WII) 
// Helper function: obtain file information time.
bool    SysFile::GetFileStat(FileStat* pfileStat, const String& path)
{
#if defined(SF_OS_WINCE)
    return false;

#else
#if defined(SF_OS_WIN32)
    // 64-bit implementation on Windows.
    struct __stat64 fileStat;
    // Stat returns 0 for success.
    wchar_t *pwpath = (wchar_t*)SF_ALLOC((UTF8Util::GetLength(path.ToCStr())+1)*sizeof(wchar_t), Stat_Default_Mem);
    UTF8Util::DecodeString(pwpath, path.ToCStr());

    int ret = _wstat64(pwpath, &fileStat);
    SF_FREE(pwpath);
    if (ret) return false;
#else
    struct stat fileStat;
    // Stat returns 0 for success.
    if (stat(path, &fileStat) != 0)
        return false;
#endif
    pfileStat->AccessTime = fileStat.st_atime;
    pfileStat->ModifyTime = fileStat.st_mtime;
    pfileStat->FileSize   = fileStat.st_size;
    return true;
#endif
}
#endif

} // Scaleform
