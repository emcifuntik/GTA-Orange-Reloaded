/**********************************************************************

Filename    :   AS2_FunctionRefImpl.h
Content     :   ActionScript implementation classes
Created     :   
Authors     :   

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/


#ifndef INC_SF_GFX_FUNCTIONREFIMPL_H
#define INC_SF_GFX_FUNCTIONREFIMPL_H

#include "AS2/AS2_Action.h"

namespace Scaleform { namespace GFx { namespace AS2 {

//
//////////// FunctionRefBase //////////////////
//
SF_INLINE void FunctionRefBase::Init(FunctionObject* funcObj, UByte flags)
{
    Flags = flags;
    Function = funcObj; 
    if (!(Flags & FuncRef_Weak) && Function)
        Function->AddRef();
    pLocalFrame = 0; 
}

SF_INLINE void FunctionRefBase::Init(FunctionObject& funcObj, UByte flags)
{
    Flags = flags;
    Function = &funcObj; 
    pLocalFrame = 0; 
}

SF_INLINE void FunctionRefBase::Init(const FunctionRefBase& orig, UByte flags)
{
    Flags = flags;
    Function = orig.Function; 
    if (!(Flags & FuncRef_Weak) && Function)
        Function->AddRef();
    pLocalFrame = 0; 
    if (orig.pLocalFrame != 0) 
        SetLocalFrame (orig.pLocalFrame, orig.Flags & FuncRef_Internal);
}

SF_INLINE void FunctionRefBase::DropRefs()
{
    if (!(Flags & FuncRef_Weak) && Function)
        Function->Release ();
    Function = 0;
    if (!(Flags & FuncRef_Internal) && pLocalFrame != 0)
        pLocalFrame->Release ();
    pLocalFrame = 0;

}

SF_INLINE void FunctionRefBase::Invoke(const FnCall& fn, const char* pmethodName) const
{
    SF_ASSERT (Function);
    Function->Invoke(fn, pLocalFrame, pmethodName);
}

SF_INLINE bool FunctionRefBase::operator== (const FunctionRefBase& f) const
{
    return Function == f.Function;
}

SF_INLINE bool FunctionRefBase::operator!= (const FunctionRefBase& f) const
{
    return Function != f.Function;
}

SF_INLINE bool FunctionRefBase::operator== (const CFunctionPtr f) const
{
    if (f == NULL)
        return Function == NULL;
    return IsCFunction() && static_cast<CFunctionObject*>(Function)->pFunction == f;
}

SF_INLINE bool FunctionRefBase::operator!= (const CFunctionPtr f) const
{
    if (f == NULL)
        return Function != NULL;
    return IsCFunction() && static_cast<CFunctionObject*>(Function)->pFunction == f;
}

SF_INLINE bool FunctionRefBase::IsCFunction () const
{
    SF_ASSERT (Function);
    return Function->IsCFunction ();
}

SF_INLINE bool FunctionRefBase::IsAsFunction () const
{
    SF_ASSERT (Function);
    return Function->IsAsFunction ();
}

}}} // SF::GFx::AS2

#endif // INC_SF_GFX_FUNCTIONREFIMPL_H

