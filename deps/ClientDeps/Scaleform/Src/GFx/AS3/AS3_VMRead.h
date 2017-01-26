/**********************************************************************

Filename    :   AS3_VMRead.h
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

#ifndef INC_AS3_VMRead_H
#define INC_AS3_VMRead_H

#include "AS3_VM.h"


namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
class StackReader
{
public:
    typedef StackReader SelfType;

    StackReader(VM& vm)
    : Num(0)
    , VMRef(vm)
    , OpStack(vm.OpStack)
    {
    }
    
protected:
    // How many extra records we have read ...
    void HaveRead(int _n)
    {
        Num += _n;
    }
    int GetRead() const
    {
        return Num;
    }
    
protected:
    VM& GetVM() const
    {
        return VMRef;
    }

    //
    VM::OpStackType& GetOpStack()
    {
        return OpStack;
    }
    const VM::OpStackType& GetOpStack() const
    {
        return OpStack;
    }

    //
    Value Pop()
    {
        // ArrayPagedLH<> doesn't have assert in the Pop() method.
        // So, we have to check for empty array by ourself.
        SF_ASSERT(GetOpStack().GetSize() > 0);
        return GetOpStack().Pop();
    }

    //
    void PopBack()
    {
        GetOpStack().PopBack();
    }
    void PopBack(UPInt n)
    {
        GetOpStack().PopBack(n);
    }

    //
    Value& Back()
    {
        // ArrayPagedLH<> doesn't have assert in the Pop() method.
        // So, we have to check for empty array by ourself.
        SF_ASSERT(GetOpStack().GetSize() > 0);
        return GetOpStack().Back();
    }
    const Value& Back() const
    {
        // ArrayPagedLH<> doesn't have assert in the Pop() method.
        // So, we have to check for empty array by ourself.
        SF_ASSERT(GetOpStack().GetSize() > 0);
        return GetOpStack().Back();
    }

    void CheckObject(const Value& v) const;

    int Read(Multiname& obj);

private:
    SelfType& operator =(const SelfType&);

private:
    int Num;
    VM& VMRef;
    VM::OpStackType& OpStack;
};

class MnReader
{
public:
    MnReader(VMAbcFile& file)
        : File(file)
    {
    }

public:
    VMAbcFile& GetFile() const
    {
        return File;
    }
    const Abc::Multiname& GetMultiname(UPInt ind) const
    {
        return GetFile().GetConstPool().GetMultiname(ind);
    }

private:
    MnReader& operator =(const MnReader&);

private:
    VMAbcFile& File;
};

// ReadArgs classes are used to read arguments from stack in various call opcodes,
// including callproperty, callmethod, callsuper, construct, etc.

struct ReadArgs : public StackReader
{
    typedef ReadArgs SelfType;

    ReadArgs(VM& vm, UInt32 arg_count);
    
    const Value* GetCallArgs() const
    {
        if (ArgNum > CallArgs.GetSize())
            return FixedArr;
        else
            return CallArgs.GetDataPtr();
    }
    unsigned GetCallArgsNum() const
    {
        return ArgNum;
    }

private:
    SelfType& operator =(const SelfType&);

private:
    enum {eFixedArrSize = 8};

    const unsigned ArgNum;
    Value FixedArr[eFixedArrSize];
    ValueArrayDH CallArgs;
};

struct ReadArgsObject : public ReadArgs
{
    typedef ReadArgsObject SelfType;

    ReadArgsObject(VM& vm, UInt32 arg_count)
    : ReadArgs(vm, arg_count)
    , ArgObject(Back(), PickValue)
    {
        PopBack();
        HaveRead(1);
        
        CheckObject(ArgObject);
    }
    
    // !!! TBD: Should be const.
    Value ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadArgsObjectRef : public ReadArgs
{
    typedef ReadArgsObjectRef SelfType;

    ReadArgsObjectRef(VM& vm, UInt32 arg_count)
        : ReadArgs(vm, arg_count)
        , ArgObject(Back())
    {
        HaveRead(1);

        CheckObject(ArgObject);
    }

    Value& ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadArgsObjectValue : public ReadArgsObject
{
    typedef ReadArgsObjectValue SelfType;

    ReadArgsObjectValue(VM& vm, UInt32 arg_count)
    : ReadArgsObject(vm, arg_count)
    , value(Back(), PickValue)
    {
        PopBack();
        HaveRead(1);
    }
    
    const Value value;
};

struct ReadArgsMn : public ReadArgs, public MnReader
{
    typedef ReadArgsMn SelfType;

    ReadArgsMn(VMAbcFile& file, UInt32 arg_count, UInt32 mn_index)
    : ReadArgs(file.GetVM(), arg_count)
    , MnReader(file)
    , ArgMN(file, GetMultiname(mn_index))
    {
        HaveRead(Read(ArgMN));
    }
    
    Multiname ArgMN;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadArgsMnObject : public ReadArgsMn
{
    typedef ReadArgsMnObject SelfType;

    ReadArgsMnObject(VMAbcFile& file, UInt32 arg_count, UInt32 mn_index)
    : ReadArgsMn(file, arg_count, mn_index)
    , ArgObject(Back(), PickValue)
    {
        PopBack();
        HaveRead(1);
        
        CheckObject(ArgObject);
    }
    
    const Value ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadArgsMnObjectRef : public ReadArgsMn
{
    typedef ReadArgsMnObjectRef SelfType;

    ReadArgsMnObjectRef(VMAbcFile& file, UInt32 arg_count, UInt32 mn_index)
        : ReadArgsMn(file, arg_count, mn_index)
        , ArgObject(Back())
    {
        HaveRead(1);

        CheckObject(ArgObject);
    }

    Value& ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadMn : public StackReader, public MnReader
{
    typedef ReadMn SelfType;

    ReadMn(VMAbcFile& file, UInt32 mn_index)
    : StackReader(file.GetVM())
    , MnReader(file)
    , ArgMN(file, GetMultiname(mn_index))
    {
        HaveRead(Read(ArgMN));
    }
    
    Multiname ArgMN;

private:
    SelfType& operator =(const SelfType&);
};

// Read compile-time multiname ...
struct ReadMnCt : public StackReader, public MnReader
{
    typedef ReadMnCt SelfType;

    ReadMnCt(VMAbcFile& file, UInt32 mn_index)
    : StackReader(file.GetVM())
    , MnReader(file)
    , ArgMN(GetMultiname(mn_index))
    {
        if (!ArgMN.IsCompileTime())
        {
            // The multiname at index must not be a runtime multiname.
            GetVM().ThrowTypeError(VM::eNotImplementedYet);
        }
    }
    
    const Abc::Multiname& ArgMN;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadValue : public StackReader
{
    typedef ReadValue SelfType;

    ReadValue(VM& vm)
    : StackReader(vm)
    , ArgValue(Back(), PickValue)
    {
        PopBack();
        HaveRead(1);
    }
    
    const Value ArgValue;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadObjectRef : public StackReader
{
    typedef ReadObjectRef SelfType;

    ReadObjectRef(VM& vm)
        : StackReader(vm)
        , ArgObject(Back())
    {
        HaveRead(1);

        CheckObject(ArgObject);
    }

    Value& ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadMnCtValue : public ReadMnCt
{
    typedef ReadMnCtValue SelfType;

    ReadMnCtValue(VMAbcFile& file, UInt32 mn_index)
    : ReadMnCt(file, mn_index)
    , value(Back(), PickValue)
    {
        PopBack();
        HaveRead(1);
    }
    
    const Value value;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadMnCtValueRef : public ReadMnCt
{
    typedef ReadMnCtValueRef SelfType;

    ReadMnCtValueRef(VMAbcFile& file, UInt32 mn_index)
        : ReadMnCt(file, mn_index)
        , value(Back())
    {
        HaveRead(1);
    }

    Value& value;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadValueObject : public ReadValue
{
    typedef ReadValueObject SelfType;

    ReadValueObject(VM& vm)
    : ReadValue(vm)
    , ArgObject(Back(), PickValue)
    {
        PopBack();
        HaveRead(1);
        
        CheckObject(ArgObject);
    }
    
    const Value ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadValueMn : public ReadValue, public MnReader
{
    typedef ReadValueMn SelfType;

    ReadValueMn(VMAbcFile& file, UInt32 mn_index)
    : ReadValue(file.GetVM())
    , MnReader(file)
    , ArgMN(file, GetMultiname(mn_index))
    {
        HaveRead(Read(ArgMN));
    }
    
    Multiname ArgMN;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadValueMnObject : public ReadValueMn
{
    typedef ReadValueMnObject SelfType;

    ReadValueMnObject(VMAbcFile& file, UInt32 mn_index)
    : ReadValueMn(file, mn_index)
    , ArgObject(Back(), PickValue)
    {
        PopBack();
        HaveRead(1);
        
        CheckObject(ArgObject);
    }
    
    const Value ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadMnObjectRef : public ReadMn
{
    typedef ReadMnObjectRef SelfType;

    ReadMnObjectRef(VMAbcFile& file, UInt32 mn_index)
    : ReadMn(file, mn_index)
    , ArgObject(Back())
    {
        HaveRead(1);

        CheckObject(ArgObject);
    }
    
    Value& ArgObject;

private:
    SelfType& operator =(const SelfType&);
};
    
}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_VMRead_H
