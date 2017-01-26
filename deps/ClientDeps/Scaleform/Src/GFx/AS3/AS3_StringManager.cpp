/**********************************************************************

Filename    :   AS3/AS3_StringManager.cpp
Content     :   Implementation of AS StringManager and builtins.
Created     :   March, 2010
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "AS3_StringManager.h"

namespace Scaleform { namespace GFx { namespace AS3 {

// ***** StringManager

// Build in string table
static const char* AS3BuiltinTable[] = 
{
    // ***** AS3 Core/TopLevel Strings
    "",
    "undefined",
    "null",
    "void",
    "true",
    "false",
    // Types for typeof operator.
    "boolean",
    "number",
    "string",
    "function",
    "object",
    "xml",
    "unknown",
    "hitArea",

    "0",
    "NaN",
    "Infinity",
    "-Infinity",

    "namespace",
    "global",
    "length",
    "toString",
    "valueOf",
    "Class",
    "Date",
    
    // *** Flash Strings
    // Events
    "activate",
    "added",
    "addedToStage",
    "removed",
    "removedFromStage",
    "deactivate",
    "enterFrame",
    "timer",
    "timerComplete",
    "complete",
    "progress",
    "open",
    "init",
    "render",
	"resize",

    "click",
    "doubleClick",
    "mouseDown",
    "mouseMove",
    "mouseOut",
    "mouseOver",
    "mouseUp",
    "mouseWheel",
    "rollOut",
    "rollOver",
    "keyDown",
    "keyUp",

    "textInput",
    "change",
    "scroll"
};

StringManager::StringManager(ASStringManager* pstrMgr)
: StringManager::Base(pstrMgr, AS3BuiltinTable)
{
    SF_COMPILER_ASSERT((sizeof(AS3BuiltinTable)/sizeof(AS3BuiltinTable[0])) == AS3BuiltinConst_Count_);

    // If this hits, there is some kind of ordering issue.
    SF_ASSERT(GetBuiltin(AS3Builtin_activate) == "activate");
    SF_ASSERT(GetBuiltin(AS3Builtin_mouseOver) == "mouseOver");
    SF_ASSERT(GetBuiltin(AS3Builtin_enterFrame) == "enterFrame");
    SF_ASSERT(GetBuiltin(AS3Builtin_rollOver) == "rollOver");
}

}}} // namespace Scaleform::GFx::AS3
