/**********************************************************************

Filename    :   AS3_CodeExecute.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_AS3_CodeExecute_H
#define INC_AS3_CodeExecute_H


///////////////////////////////////////////////////////////////////////////////
namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc
{
    ///////////////////////////////////////////////////////////////////////////
    // Forward declaration.
    class Code;
    class OpCodeCallback;

    ///////////////////////////////////////////////////////////////////////////
    // Return *true* if execution is finished completely ...
    /*
    bool ExecuteCode(
        const Code& code, 
        AS3::VM& vm,
        int start_offset = 0
        );
    */
}}}} // namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc {

#endif // INC_AS3_CodeExecute_H
