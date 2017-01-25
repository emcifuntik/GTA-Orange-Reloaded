/**********************************************************************

PublicHeader:   Kernel
Filename    :   AllocInfo.h
Content     :   Allocator Debugging Support
Created     :   October 1, 2008
Authors     :   Michael Antonov, Maxim Shemanarev

Copyright   :   (c) 2006-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Kernel_AllocInfo_H
#define INC_SF_Kernel_AllocInfo_H

#include "SF_Stats.h"

namespace Scaleform {


// ***** Allocator Debugging Support
//
// Debugging support is optionally stored with every allocation.
// This consists of the following pieces of info:
//  - StatId, used to identify and group allocation for statistic reporting.
//  - Line + FileName, used to report the memory leaks when heap
//    is released as a whole.
//------------------------------------------------------------------------
struct AllocInfo
{
    // User-specified identifier for allocation.
    unsigned    StatId;

#if defined(SF_BUILD_DEBUG)
    // Track location in code where allocation took place
    // so that it can be reported to debug leaks.
    unsigned    Line;
    const char* pFileName;

    AllocInfo()
        : StatId(Stat_Default_Mem), Line(0), pFileName("")
    { }
    AllocInfo(unsigned statId, const char* pfile, unsigned line)
        : StatId(statId), Line(line), pFileName(pfile)
    { }
    AllocInfo(const AllocInfo& src)
        : StatId(src.StatId), Line(src.Line), pFileName(src.pFileName)
    { }

#else

    AllocInfo() : StatId(Stat_Default_Mem)
    { }
    AllocInfo(unsigned statId, const char*, unsigned) : StatId(statId)
    { }
    AllocInfo(const AllocInfo& src) : StatId(src.StatId)
    { }

#endif

    // Note that we don't store the original Size here, since it is 
    // passed as a separate argument and we want to avoid such overhead.
};

} // Scaleform

#endif
