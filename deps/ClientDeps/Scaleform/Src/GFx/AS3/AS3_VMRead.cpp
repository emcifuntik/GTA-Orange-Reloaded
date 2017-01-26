/**********************************************************************

Filename    :   AS3_VMRead.cpp
Content     :   
Created     :   September, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "AS3_VMRead.h"

namespace Scaleform { namespace GFx { namespace AS3 
{
    ///////////////////////////////////////////////////////////////////////////
    void StackReader::CheckObject(const Value& v) const
    {
        if (v.IsNull())
        {
            // A TypeError is thrown if obj is null or undefined.
            GetVM().ThrowTypeError(VM::eConvertNullToObjectError DEBUG_ARG("Cannot access a property or method of a null object reference"));
        }
        else if (v.IsUndefined())
        {
            GetVM().ThrowTypeError(VM::eConvertUndefinedToObjectError DEBUG_ARG("A term is undefined and has no properties"));
        }
    }

    int StackReader::Read(Multiname& obj)
    {
        switch (obj.GetKind())
        {
        case Abc::MN_QName:
        case Abc::MN_QNameA:
            // QName is not read from stack; both name and namespase known at compile-time.
            break;
        case Abc::MN_RTQName:
        case Abc::MN_RTQNameA:
            // Name is known at compile time, Namespace is resolved at runtime.

            // For maniacs.
            CheckObject(Back());
            if (GetVM().IsException())
                break;

            obj.SetRTNamespace(
                const_cast<Instances::Namespace&>(Pop().AsNamespace()));
            return 1;
        case Abc::MN_RTQNameL:
        case Abc::MN_RTQNameLA:
            // Both name and namespace are resolved at runtime.
            obj.SetRTName(Pop());

            // For maniacs.
            CheckObject(Back());
            if (GetVM().IsException())
                break;

            obj.SetRTNamespace(
                const_cast<Instances::Namespace&>(Pop().AsNamespace()));
            return 2;
        case Abc::MN_Multiname:
        case Abc::MN_MultinameA:
            // MInfo is not supposed to be read from stack.
            break;
        case Abc::MN_MultinameL:
        case Abc::MN_MultinameLA:
            // Namespace set is known at compile time, Name is resolved at runtime.        
            // name_ind is a string, and we will get an int or an uint as an index instead;
            obj.SetRTName(Pop());         
            return 1;
        case Abc::MN_Typename:
            // Recursive!!!?
            GetVM().GetUI().Output(FlashUI::Output_Warning, "Reading chained multiname in itself.");
            return Read(obj);
        default:
            SF_ASSERT(false);
            break;
        }

        return 0;
    }

    ///////////////////////////////////////////////////////////////////////////
    ReadArgs::ReadArgs( VM& vm, UInt32 arg_count ) : StackReader(vm)
        , ArgNum(arg_count)
        , CallArgs(vm.GetMemoryHeap())
    {
        if (arg_count)
        {
            VM::OpStackType& opstack = GetOpStack();
            const UPInt size = opstack.GetSize();
            unsigned j = 0;
            UPInt i = size - arg_count;

            if (arg_count <= eFixedArrSize)
            {
                for (; i < size; ++i, ++j)
                    //FixedArr[j].Swap(opstack[i]);
                    FixedArr[j].PickUnsafe(opstack[i]);
            }
            else
            {
                CallArgs.Resize(arg_count);

                for (; i < size; ++i, ++j)
                    //CallArgs[j].Swap(opstack[i]);
                    CallArgs[j].PickUnsafe(opstack[i]);
            }

            opstack.Resize(size - arg_count);
        }

        HaveRead(arg_count);
    }


}}} // namespace Scaleform { namespace GFx { namespace AS3 {
