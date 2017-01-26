/*****************************************************************

PublicHeader:   Render
Filename    :   RenderTreeLog.h
Content     :   Rendering log used to esitmate sorting efficiency
Created     :   
Authors     :   Michael Antonov
Copyright   :   (c) 1999-2007 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_RenderTreeLog_H
#define INC_RenderTreeLog_H

#include "Render/Render_Types2D.h"
#include "Render/Render_Matrix2x4.h"

#define GFX_RTREELOG_ENABLED

namespace Scaleform { 
namespace Render {

// This RenderTreeLog is used for debugging only, allowing object
// tree with bounds information to be dumped so as to allow sorter
// testing for Gfx 4.0 renderer

#ifdef GFX_RTREELOG_ENABLED

class RenderTreeLog
{
    static void    IndentLine();
public:
    static void    StartLog(const char* pfilename);
    static bool    IsLogging();
    static void    EndLog();

    static void    StartObject(const char* pname,
        const RectF& bounds, const Matrix2F& matrix);
    static void    EndObject();

    static void    LogRender(const char* pcmdName, unsigned key);
};

// Macros to allow easy compile-out of logging calls (and their arguments)
#define GFX_RTREELOG_STARTOBJECT(name, b, m)  \
    if (Render::RenderTreeLog::IsLogging()) Render::RenderTreeLog::StartObject(name, b, m)
#define GFX_RTREELOG_ENDOBJECT() Render::RenderTreeLog::EndObject()

#else

#define GFX_RTREELOG_STARTOBJECT(name, b, m)
#define GFX_RTREELOG_ENDOBJECT()

#endif

}}

#endif // INC_RenderTreeLog_H

