/**********************************************************************

PublicHeader:   None
Filename    :   AS3/AS3_StringManager.h
Content     :   Implementation of AS StringManager and builtins.
Created     :   March, 2010
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   This built-in StringManager is separated from the one
                used by AS2, but they can share a common
                ASStringManager instance.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#ifndef SF_GFx_AS3_StringManager_H
#define SF_GFx_AS3_StringManager_H

#include "GFx/GFx_ASString.h"

namespace Scaleform { namespace GFx { namespace AS3 {


/* ***** AS3 String Builtin Notes

Built-ins are strings that can be accessed very efficiently based on
a constant. They are usually obtained from the built-in manager through
an inline function such as

// Returns a string for "undefined"
pManager->GetBuiltin(ASBuiltin_undefined)

Builtins are pre-created ahead of time, so that GetBuiltin() can be
inlined to a single array access. Built-in strings are used heavily
throughout AS so performance of their references and compares is of
vital importance.

AS3::StringManager builtins are separate from the ones used in AS2
since there is a smaller number of them needed. For now, we combine
AS3 implementation (Top-Level) and Flash strings here; however,
they shouldn't cause a dependency problem.

*/

enum BuiltinType
{
    // ***** AS3 Core/TopLevel Strings

    AS3Builtin_empty_,      // ""
    
    AS3Builtin_undefined,
    AS3Builtin_null,
    AS3Builtin_void,
    AS3Builtin_true,
    AS3Builtin_false,
    // Types for typeof operator
    AS3Builtin_boolean,
    AS3Builtin_number,
    AS3Builtin_string,
    AS3Builtin_function,
    AS3Builtin_object,
    AS3Builtin_xml,
    AS3Builtin_unknown,
    AS3Builtin_hitArea,

    AS3Builtin_zero,
    AS3Builtin_NaN,
    AS3Builtin_Infinity,
    AS3Builtin_minusInfinity_, // "-Infinity"

    AS3Builtin_namespace,
    AS3Builtin_global,
    AS3Builtin_length,
    AS3Builtin_toString,
    AS3Builtin_valueOf,
    AS3Builtin_Class,
    AS3Builtin_Date,
    
       
    // ***** Flash Strings
    // These can be taken out if TopLevel-only build is needed.

    // Events    
    AS3Builtin_activate,
    AS3Builtin_added,
    AS3Builtin_addedToStage,
    AS3Builtin_removed,
    AS3Builtin_removedFromStage,
    AS3Builtin_deactivate,
    AS3Builtin_enterFrame,
    AS3Builtin_timer,
    AS3Builtin_timerComplete,
    AS3Builtin_complete,
    AS3Builtin_progress,
    AS3Builtin_open,
    AS3Builtin_init,
    AS3Builtin_render,
	AS3Builtin_resize,

    // Mouse events
    AS3Builtin_click,
    AS3Builtin_doubleClick,
    AS3Builtin_mouseDown,
    AS3Builtin_mouseMove,
    AS3Builtin_mouseOut,
    AS3Builtin_mouseOver,
    AS3Builtin_mouseUp,
    AS3Builtin_mouseWheel,
    AS3Builtin_rollOut,
    AS3Builtin_rollOver,
    AS3Builtin_keyDown,
    AS3Builtin_keyUp,

    AS3Builtin_textInput,
    AS3Builtin_change,
    AS3Builtin_scroll,
    
    AS3BuiltinConst_Count_
};

// StringManager - String manager and builtin table for AS3.

class StringManager
    : public ASStringBuiltinManagerT<BuiltinType, AS3BuiltinConst_Count_>
{
    typedef ASStringBuiltinManagerT<BuiltinType, AS3BuiltinConst_Count_> Base;
public:
    StringManager(ASStringManager* pstrMgr);
    ~StringManager() { }
};


}}} // namespace Scaleform::GFx::AS3

#endif //SF_GFx_AS3_StringManager_H
