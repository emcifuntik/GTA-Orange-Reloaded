/**********************************************************************

Filename    :   Log.cpp
Content     :   Logging support
Created     :   July 18, 2010
Authors     :   Brendan Iribe, Michael Antonov

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "SF_Log.h"
#include "SF_Std.h"
#include <stdarg.h>
#include <stdio.h>

#if defined(SF_OS_WIN32)
#include <windows.h>
#elif defined(SF_OS_XBOX360)
#include <xtl.h>
#elif defined(SF_OS_WII)
#include <revolution/os.h>
#elif defined(SF_OS_3DS)
#include <nn/dbg.h>
#elif defined(SF_OS_ANDROID)
#include <android/log.h>
#endif

namespace Scaleform {

// Global Log pointer.
Log* volatile SF_GlobalLog = 0;

// ***** Log Implementation

Log::~Log()
{
    // Clear out global log
    if (this == SF_GlobalLog)
    {
        // TBD: perhaps we should ASSERT if this happens before system shutdown?
        SF_GlobalLog = 0;
    }
}

void Log::LogMessageVarg(LogMessageId messageId, const char* fmt, va_list argList)
{
    DefaultLogMessageVarg(messageId, fmt, argList);
}

void Log::LogMessageById(LogMessageId messageId, const char* pfmt, ...)
{
    va_list argList;
    va_start(argList, pfmt);
    LogMessageVarg(messageId, pfmt, argList);
    va_end(argList);
}

void Log::FormatLog(char* buffer, unsigned bufferSize, LogMessageId messageId,
                    const char* fmt, va_list argList)
{
    // Generate a format string.
    switch(messageId.GetMessageType())
    {
    case LogMessage_Error:     SFstrcpy(buffer, bufferSize, "Error: ");     break;
    case LogMessage_Warning:   SFstrcpy(buffer, bufferSize, "Warning: ");   break;
    case LogMessage_Assert:    SFstrcpy(buffer, bufferSize, "Assert: ");    break;
    case LogMessage_Text:      buffer[0] = 0;
    }

    UPInt prefixLength = SFstrlen(buffer);
    char *buffer2      = buffer + prefixLength;
    SFvsprintf(buffer2, bufferSize - prefixLength, fmt, argList);

    if (messageId.GetMessageType() != LogMessage_Text)
        SFstrcat(buffer, bufferSize, "\n");
}

void Log::DefaultLogMessageVarg(LogMessageId messageId, const char* fmt, va_list argList)
{
#ifdef SF_BUILD_DEBUG
    char buffer[MaxLogBufferMessageSize];
    FormatLog(buffer, MaxLogBufferMessageSize, messageId, fmt, argList);

#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360)
    ::OutputDebugStringA(buffer);
#elif defined(SF_OS_WII)
    OSReport(buffer);
#elif defined(SF_OS_3DS)
    nn::dbg::detail::PutString(buffer);
#elif defined(SF_OS_ANDROID)
    __android_log_write(ANDROID_LOG_INFO,"GFx",buffer);
#else
    fputs(buffer, stdout);
#endif

#else
    SF_UNUSED3(messageId, fmt, argList);
#endif
}


//static
void Log::SetGlobalLog(Log *log)
{
    SF_GlobalLog = log;
}
//static
Log* Log::GetGlobalLog()
{
// No global log by default?
//    if (!SF_GlobalLog)
//        SF_GlobalLog = GetDefaultLog();
    return SF_GlobalLog;
}

//static
Log* Log::GetDefaultLog()
{
    // Create default log pointer statically so that it can be used
    // even during startup.
    static Log defaultLog;
    return &defaultLog;
}

} // Scaleform 
