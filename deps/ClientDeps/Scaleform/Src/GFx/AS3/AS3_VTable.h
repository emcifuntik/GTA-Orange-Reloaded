/**********************************************************************

Filename    :   AS3_VTable.h
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

#ifndef INC_AS3_VTable_H
#define INC_AS3_VTable_H

#include "AS3_Value.h"
#include "AS3_Slot.h"

namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
// Forward declaration
namespace InstanceTraits
{
//     class Traits;
    class GlobalObject;
}

class VM;

///////////////////////////////////////////////////////////////////////////
// Virtual table is not associated with particular traits or object on reason ... 
class VTable : public NewOverrideBase<Mem_Stat>
{
    friend class Traits;
    friend class InstanceTraits::GlobalObject; // Because of ClearInitScope().

    typedef VTable SelfType;
    
public:
    VTable(VM& _vm);
    VTable(const VTable& parent);
    
public:
    void Execute(AbsoluteIndex ind, const Value& _this,
                 unsigned argc, const Value* argv, bool discard_result = false);
    void GetMethod(Value& value, AbsoluteIndex ind, Object& _this) const;
    // Return a raw value. It is not correct to use this method to create 
    // callable objects. This method should be used only for displaying of internal information.
    const Value& Get(AbsoluteIndex ind) const
    {
        return VTMethods[ind.Get()];
    }
    const ScopeStackType& GetStoredScopeStack() const
    {
        return InitScope;
    }
    VM& GetVM() const
    {
        SF_ASSERT(pVM);
        return *pVM;
    }

public:
    void ForEachChild_GC(RefCountBaseGC<Mem_Stat>::GcOp op) const
    {
        AS3::ForEachChild_GC(VTMethods, op);
        AS3::ForEachChild_GC(InitScope, op);
    }

private:
    void SetMethod(
        AbsoluteIndex ind, 
        const Value& m,
        SlotInfo::DataType dt, 
        const Traits& ot 
        DEBUG_ARG(const ASString& name)
        );
    AbsoluteIndex AddMethod(
        const Value& m, 
        SlotInfo::DataType dt,
        const Traits& ot 
        DEBUG_ARG(const ASString& name)
        );

    void StoreScopeStack(const UPInt baseSSInd, const ScopeStackType& ss);
    void Add2StoredScopeStack(const Value& o);
    void ClearInitScope()
    {
        InitScope.Clear();
    }

    SelfType& GetSelf() { return *this; }

private:
    VM* pVM;
    ValueArray VTMethods;
    // We keep a copy of the scope stack ...
    ScopeStackType InitScope;
};

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_VTable_H

