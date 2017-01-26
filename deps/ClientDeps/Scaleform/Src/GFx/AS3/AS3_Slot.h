/**********************************************************************

Filename    :   AS3_Slot.h
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

#ifndef INC_AS3_Slot_H
#define INC_AS3_Slot_H

#include "AS3_Index.h"
#include "AS3_GC.h"
#include "Abc/AS3_Abc.h"


namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
// Forward declaration.
class VTable;

namespace Instances 
{
    class Dictionary;
}

namespace InstanceTraits
{
    class Traits;
}

namespace ClassTraits
{
    class Traits;
}

///////////////////////////////////////////////////////////////////////////
class SlotInfo : public NewOverrideBase<Mem_Stat>
{
    friend class Traits;
    friend class InstanceTraits::Traits; // Because of setNamespace().
    friend class Object; // ??? Temporarily ...
    friend class Instances::Dictionary; // ??? Temporarily ...
    
public:
    enum Attribute
    {
        aNone       = 0,
        aReadOnly   = 1 << 0, // Constant ???
        aDontEnum   = 1 << 1,
        aDontDelete = 1 << 2, // Slot ???
        aInternal   = 1 << 3
    };
    
public:
    enum DataType
    {
        // DO NOT reorder.
        DT_Unknown = 0,
        DT_ValueArray,
        DT_Value,
        DT_Object,
        DT_Boolean,
        DT_Int,
        DT_UInt,
        DT_Number,
        DT_String,
        DT_Code,
        DT_Get,
        DT_Set,
        DT_GetSet
    };
    
public:
    // !!! We should get rid of this constructor ...
    SlotInfo();
    SlotInfo(
        Pickable<const Instances::Namespace> ns,
        const ClassTraits::Traits* ctraits,
        int a = aDontEnum 
        DEBUG_ARG(const Ptr<ASStringNode>& name = NULL)
        );
    SlotInfo(const SlotInfo& other);
    ~SlotInfo();

    SlotInfo& operator =(const SlotInfo& other);

private:
    // This constructor should be available only to Traits.
    SlotInfo(
        Pickable<const Instances::Namespace> ns,
        VMAbcFile& file,
        const Abc::TraitInfo& ti,
        int a = aDontEnum 
        DEBUG_ARG(const Ptr<ASStringNode>& name = NULL)
        );

public:
    //
    int GetAttrs() const
    {
        return Attr;
    }
    //
    bool IsBound2Value() const
    {
        return ValueInd.IsValid() && GetBindingType() != DT_Unknown;
    }
    //
    bool IsReadonly() const
    {
        return GetAttrs() & aReadOnly;
    }
    //
    bool IsDontEnum() const
    {
        return (GetAttrs() & aDontEnum) != 0;
    }
    void SetDontEnum(bool flag = true)
    {
        Attr = (flag ? Attr | aDontEnum : Attr & ~aDontEnum);
    }
    //
    bool IsDontDelete() const
    {
        return (GetAttrs() & aDontDelete) != 0;
    }
    //
    bool IsInternal() const
    {
        return (GetAttrs() & aInternal) != 0;
    }
    //
    bool IsCode() const
    {
        return BindingType > 8;
    }

    //
    bool IsConst() const;
    bool IsClass() const;
    bool IsClassOrConst() const;
    
public:
    // We need "data" for instances ...
    // Can throw exceptions.
    CheckResult GetSlotValue(Value& value, Object* obj) const;
    CheckResult GetSlotValueUnsafe(Value& value, Object* obj) const;
    // Can throw exceptions.
    CheckResult GetSlotValue(VM& vm, Value& value, const Value& _this, const VTable* vt = NULL) const;
    CheckResult GetSlotValueUnsafe(VM& vm, Value& value, const Value& _this, const VTable* vt = NULL) const;
    // Can throw exceptions.        
    CheckResult SetSlotValue(VM& vm, const Value& v, Object* const obj);
    // Can throw exceptions.        
    CheckResult SetSlotValue(VM& vm, const Value& v, const Value& _this, const VTable* vt = NULL);

    const ClassTraits::Traits& GetDataType(VM& vm);
    
    // Delete dynamic slot value.
    void DeleteDynamic(Object& obj) const;

public:
    void ForEachChild_GC(const Object& obj, RefCountBaseGC<Mem_Stat>::GcOp op) const;
    void ForEachChild_GC(RefCountBaseGC<Mem_Stat>::GcOp op) const;
    
private:
    // Bind() is made separate from SlotInfo() constructor because of logic 
    // in Traits::Add2VT(), which can reset offset and/or DataType.
    void Bind(DataType dt, AbsoluteIndex offset)
    {
        BindingType = dt;
        ValueInd = offset;
    }

public:
    const Instances::Namespace& GetNamespace() const
    {
        SF_ASSERT(pNs);
        return *pNs;
    }
    DataType GetBindingType() const
    {
        return BindingType;
    }
    AbsoluteIndex GetValueInd() const
    {
        return ValueInd;
    }
    DEBUG_CODE(ASString GetName() const { return ASString(Name.GetPtr()); })
    
private:
    void setNamespace(const Instances::Namespace& ns);
    const ClassTraits::Traits* GetCTraits() const
    {
        return CTraits;
    }

private:
    // Combination of *enum Attribute* values ...
    int Attr;
    SPtr<const Instances::Namespace> pNs;
    
    SPtr<const ClassTraits::Traits> CTraits;
    VMAbcFile* File;
    const Abc::TraitInfo* TI;

    DataType BindingType;
    
    AbsoluteIndex ValueInd; // Index in Values or an offset.
    DEBUG_CODE(Ptr<ASStringNode> Name;)
};

///////////////////////////////////////////////////////////////////////////
class SlotContainerType
{
public:
    typedef ArrayDH_POD<UPInt> IndSetType;
    typedef HashLH<ASStringNode*, IndSetType> SetType;
    typedef SlotContainerType SelfType;
    typedef SlotInfo ValueType;

public:
    SlotContainerType();

public:
    // Operators.
    SelfType& operator =(const SelfType& other);
    ValueType& operator [](AbsoluteIndex ind) { return VArray[ind.Get()]; }
    const ValueType& operator [](AbsoluteIndex ind) const { return VArray[ind.Get()]; }

public:
    //
    UPInt GetSize() const { return Size; }

    //
    void Clear();

    //
    AbsoluteIndex FindFirst(const ASString& k) const
    {
        const SetType::ValueType* pArr = KVHashSet.Get(k.GetNode());
        return (pArr == NULL) ? AbsoluteIndex(-1) : (pArr->IsEmpty() ? AbsoluteIndex(-1) : AbsoluteIndex(pArr->Front()));
    }

    //
    AbsoluteIndex FindLast(const ASString& k) const
    {
        const SetType::ValueType* pArr = KVHashSet.Get(k.GetNode());
        return (pArr == NULL) ? AbsoluteIndex(-1) : (pArr->IsEmpty() ? AbsoluteIndex(-1) : AbsoluteIndex(pArr->Back()));
    }

    //
    AbsoluteIndex Find(const ASString& k) const { return FindFirst(k); }
    const SetType::ValueType* FindValues(const ASString& k) const
    {
        return KVHashSet.Get(k.GetNode());
    }
    SetType::ValueType* FindValues(const ASString& k)
    {
        return KVHashSet.Get(k.GetNode());
    }

    // Get first value.
    ValueType& Get(const ASString& k)
    {
        const AbsoluteIndex ind = Find(k);
        SF_ASSERT(ind.IsValid());

        return VArray[ind.Get()];
    }
    const ValueType& Get(const ASString& k) const
    {
        const AbsoluteIndex ind = Find(k);
        SF_ASSERT(ind.IsValid());

        return VArray[ind.Get()];
    }

    //
    ASStringNode* GetKey(AbsoluteIndex ind) const
    {
        return KArray[ind.Get()].GetPtr();
    }

    void SetKey(AbsoluteIndex ind, const ASString& k);

    //
    ValueType& Back() { return VArray.Back(); }
    const ValueType& Back() const { return VArray.Back(); }

    // Return number of removed values.
    UPInt RemoveKey(const ASString& k);

    // Return index of a newly added value.
    AbsoluteIndex Add(const ASString& k, const ValueType& v);

    //
    void Swap(SelfType& other);

    //
    AbsoluteIndex FindAdd(const ASString& k, const ValueType& v);

    //
    bool IsEmpty() const { return GetSize() == 0; }

public:
    void ForEachChild_GC(RefCountBaseGC<Mem_Stat>::GcOp op) const;

private:
    // Using separate value for size helps solve problems with Garbage Collector.
    UPInt                           Size;
    UPInt                           FirstFreeInd;
    ArrayLH<ValueType>              VArray;
    ArrayLH<Ptr<ASStringNode> >     KArray;
    SetType                         KVHashSet;
};

///////////////////////////////////////////////////////////////////////////////
inline
void ForEachChild_GC(const SlotContainerType& v, RefCountBaseGC<Mem_Stat>::GcOp op)
{
    v.ForEachChild_GC(op);
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_Slot_H

