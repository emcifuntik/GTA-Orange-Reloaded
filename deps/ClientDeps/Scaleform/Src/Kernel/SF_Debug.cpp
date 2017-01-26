/**********************************************************************

Filename    :   Kernel/SF_Debug.h
Content     :   General purpose debugging support
Created     :   July 18, 2001
Authors     :   Brendan Iribe, Michael Antonov

Copyright   :   (c) 2001-2006 Scaleform Corp. All Rights Reserved.

Notes       :   GFC Debug warning functionality is enabled
                if and only if SF_BUILD_DEBUG is defined.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "SF_Debug.h"

namespace Scaleform {

void LogDebugMessage(LogMessageId id, const char* fmt, ...)
{
    Log* log = Log::GetGlobalLog();
    va_list argList;
    va_start(argList, fmt);
    if (log)
        log->LogMessageVarg(id, fmt, argList);
    else
        Log::DefaultLogMessageVarg(id, fmt, argList);
    va_end(argList);
}
    

} // Scaleform
