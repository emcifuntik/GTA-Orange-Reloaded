/**********************************************************************

Filename    :   SF_String_PathUtil.cpp
Content     :   String filename/url helper function
Created     :   Audust 26, 2007
Authors     :   Ankur Mohan, Michael Antonov

Notes       :   
History     :   

Copyright   :   (c) 1998-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "SF_String.h"
#include "SF_Debug.h"
#include "SF_UTF8Util.h"

namespace Scaleform {

//--------------------------------------------------------------------
// ***** Path-Scanner helper function 

// Scans file path finding filename start and extension start, fills in their addess.
void ScanFilePath(const char* url, const char** pfilename, const char** pext)
{ 
    const char *filename = 0;
    const char *lastDot = 0;

    UInt32 charVal = UTF8Util::DecodeNextChar(&url);

    while (charVal != 0)
    {
        if ((charVal == '/') || (charVal == '\\'))
        {
            filename = url;
            lastDot  = 0;
        }
        else if (charVal == '.')
        {
            lastDot = url - 1;
        }
        
        charVal = UTF8Util::DecodeNextChar(&url);
    }

    if (filename != url)
        filename = 0;

    if (pfilename)
        *pfilename = filename;
    if (pext)
        *pext = lastDot;
}

// Scans till the end of protocol. Returns first character past protocol,
// 0 if not found.
//  - protocol: 'file://', 'http://'
const char* ScanPathProtocol(const char* url)
{    
    UInt32 charVal = UTF8Util::DecodeNextChar(&url);
    UInt32 charVal2;
   
    while (charVal != 0)
    {
        // Treat a colon followed by a slash as absolute.
        if (charVal == ':')
        {
            charVal2 = UTF8Util::DecodeNextChar(&url);
            charVal  = UTF8Util::DecodeNextChar(&url);
            if ((charVal == '/') && (charVal2 == '\\'))
                return url;
        }
        charVal = UTF8Util::DecodeNextChar(&url);
    }
    return 0;
}


//--------------------------------------------------------------------
// ***** String Path API implementation

bool String::HasAbsolutePath(const char* url)
{
    // Absolute paths can star with:
    //  - protocols:        'file://', 'http://'
    //  - windows drive:    'c:\'
    //  - UNC share name:   '\\share'
    //  - unix root         '/'

    // On the other hand, relative paths are:
    //  - directory:        'directory/file'
    //  - this directory:   './file'
    //  - parent directory: '../file'
    // 
    // For now, we don't parse '.' or '..' out, but instead let it be concatenated
    // to string and let the OS figure it out. This, however, is not good for file
    // name matching in library/etc, so it should be improved.

    if (!url || !*url)
        return true; // Treat empty strings as absolute.    

    UInt32 charVal = UTF8Util::DecodeNextChar(&url);

    // Fist character of '/' or '\\' means absolute url.
    if ((charVal == '/') || (charVal == '\\'))
        return true;

    while (charVal != 0)
    {
        // Treat a colon followed by a slash as absolute.
        if (charVal == ':')
        {
            charVal = UTF8Util::DecodeNextChar(&url);
            // Protocol or windows drive. Absolute.
            if ((charVal == '/') || (charVal == '\\'))
                return true;
        }
        else if ((charVal == '/') || (charVal == '\\'))
        {
            // Not a first character (else 'if' above the loop would have caught it).
            // Must be a relative url.
            break;
        }

        charVal = UTF8Util::DecodeNextChar(&url);
    }

    // We get here for relative paths.
    return false;    
}


bool String::HasExtension(const char* path)
{
    const char* ext = 0;
    ScanFilePath(path, 0, &ext);
    return ext != 0;
}
bool String::HasProtocol(const char* path)
{
    return ScanPathProtocol(path) != 0;
}


String  String::GetPath() const
{
    const char* filename = 0;
    ScanFilePath(ToCStr(), &filename, 0);

    // Technically we can have extra logic somewhere for paths,
    // such as enforcing protocol and '/' only based on flags,
    // but we keep it simple for now.
    return String(ToCStr(), filename ? (filename-ToCStr()) : GetSize());
}

String  String::GetProtocol() const
{
    const char* protocolEnd = ScanPathProtocol(ToCStr());
    return String(ToCStr(), protocolEnd ? (protocolEnd-ToCStr()) : 0);
}

String  String::GetFilename() const
{
    const char* filename = 0;
    ScanFilePath(ToCStr(), &filename, 0);
    return String(filename);
}
String  String::GetExtension() const
{
    const char* ext = 0;
    ScanFilePath(ToCStr(), 0, &ext);
    return String(ext);
}

void    String::StripExtension()
{
    const char* ext = 0;
    ScanFilePath(ToCStr(), 0, &ext);
    if (ext)
        AssignString(ext, SFstrlen(ext));
}

void    String::StripProtocol()
{
    const char* protocol = ScanPathProtocol(ToCStr());
    if (protocol)
        AssignString(protocol, SFstrlen(protocol));
}

} // Scaleform
