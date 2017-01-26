/**********************************************************************

Filename    :   AS3_Instance.cpp
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

#include "AS3_Instance.h"
#include "AS3_VM.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
const char* NS_AS3 = "http://adobe.com/AS3/2006/builtin";
const char* NS_flash_proxy = "http://www.adobe.com/flash/proxy";
const char* NS_Vector = "__AS3__.vec";

///////////////////////////////////////////////////////////////////////////
namespace InstanceTraits
{
    Traits::Traits(VM& vm, const ASString& n, Pickable<Instances::Namespace> ns, Class& c, const Traits* pt, ObjectType ot)
    : AS3::Traits(vm, c, pt, ot)
    , Name(n)
    , Ns(ns)
    // Future development.
    //: AS3::Traits(vm, c, (c.GetClassTraits().GetParent() ? &c.GetClassTraits().GetParent()->GetInstanceTraits() : NULL), ot)
    {
        // Precondition ...
        SF_ASSERT(SlotsAreBound2Values());
        
        // Parent traits are also InstanceTraits.
        // Copy parent's traits.
        if (GetParent())
        {
            if (!GetParent()->GetSlots().IsEmpty())
            {
                // Copy parent's slots ...
//                 SlotContainerType tmp(GetParent()->GetSlots()); // Deep copy ...
//                 Swap(tmp);

                Assign(*GetParent());
                SF_ASSERT(SlotsAreBound2Values());
                
                FirstOwnSlotInd = AbsoluteIndex(GetSlotInfoNum());
                OwnSlotNumber = GetParent()->OwnSlotNumber;
                FixedValueSlotNumber = GetParent()->GetFixedValueSlotNumber();
            }
        }
    }
    
    Traits::Traits(VM& vm, const ASString& n, Pickable<Instances::Namespace> ns, Class& c, ObjectType ot)
    : AS3::Traits(vm, c, (c.GetParentClass() ? &c.GetParentClass()->GetNewObjectTraits() : NULL), ot)
    , Name(n)
    , Ns(ns)
    // Future development.
    //: AS3::Traits(vm, c, (c.GetClassTraits().GetParent() ? &c.GetClassTraits().GetParent()->GetInstanceTraits() : NULL), ot)
    {
        // Precondition ...
        SF_ASSERT(SlotsAreBound2Values());

        // Parent traits are also InstanceTraits.
        // Copy parent's traits.
        if (GetParent())
        {
            if (!GetParent()->GetSlots().IsEmpty())
            {
                // Copy parent's slots ...
                //                 SlotContainerType tmp(GetParent()->GetSlots()); // Deep copy ...
                //                 Swap(tmp);

                Assign(*GetParent());
                SF_ASSERT(SlotsAreBound2Values());

                FirstOwnSlotInd = AbsoluteIndex(GetSlotInfoNum());
                OwnSlotNumber = GetParent()->OwnSlotNumber;
                FixedValueSlotNumber = GetParent()->GetFixedValueSlotNumber();
            }
        }
    }

    Traits::~Traits()
    {
    }

    void Traits::Super(const AS3::Traits& /*ot*/, const Value& _this, unsigned argc, const Value* argv) const
    {
        SF_ASSERT(!_this.IsNull());
        _this.GetObject()->Super(argc, argv);
    }

    // Obtain class Info, but only if UserDefined - null otherwise.
    const Abc::ClassInfo* Traits::GetClassInfo() const
    {
        return 0;
    }

    ASString Traits::GetName() const
    {
        return Name;
    }

    void Traits::ForEachChild_GC(GcOp op) const
    {
        AS3::Traits::ForEachChild_GC(op);

        AS3::ForEachChild_GC<Instances::Namespace, Mem_Stat>(Ns, op);
    }

    bool Traits::SupportsInterface(const ClassTraits::Traits& itraits) const
    {
        VM& vm = GetVM();
        const ArrayLH<Multiname>& interfaces = GetImplementedInterfaces();

        for (UPInt i = 0; i < interfaces.GetSize(); ++i)
        {
            const ClassTraits::Traits* ptraits = vm.GetRegisteredClassTraits(interfaces[i]);
            if (ptraits == &itraits)
                return true;

            // Recursive handling of nested interfaces.
            if (ptraits)
            {
                InstanceTraits::Traits& itr = ptraits->GetInstanceTraits();
                if (itr.SupportsInterface(itraits))
                    return true;
            }            
        }

        return false;
    }

    ASString Traits::GetQualifiedName(QNameFormat f) const
    {
        const ASString& nsUri = GetNamespace().GetUri();
        if (nsUri.IsEmpty())
            return Name;

        if (f == qnfWithColons)
            return nsUri + "::" + Name;

        return nsUri + "." + Name;
    }

    ///////////////////////////////////////////////////////////////////////
    void Prototype::MakeObject(Value& /*result*/)
    {
        SF_ASSERT(false);
    }

    Prototype::Prototype(VM& vm, const ASString& n, Class& c ) 
        : Traits(vm, n, Pickable<Instances::Namespace>(&vm.GetPublicNamespace(), PickValue), c, (c.GetParentClass() ? &c.GetParentClass()->GetNewObjectTraits() : NULL), DynamicObject) // No VMAbcFile because it is supposed to be used with C++ classes only.
    {

    }

    ///////////////////////////////////////////////////////////////////////
    Anonimous::Anonimous(VM& vm)
        : Prototype(vm, vm.GetStringManager().GetBuiltin(AS3Builtin_empty_), vm.GetClassClass())
    {
    }

} // namespace InstanceTraits

void Instance::Construct(Value& /*result*/, unsigned /*argc*/, const Value* /*argv*/, bool /*extCall*/)
{
    // Just do nothing. That seems to be a right behavior.
}

ASString Instance::GetName() const
{
    return GetClass().GetName();
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

