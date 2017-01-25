/**********************************************************************

Filename    :   AS3_Instance.h
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

#ifndef INC_AS3_Instance_H
#define INC_AS3_Instance_H


#include "AS3_Traits.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
extern const char* NS_AS3;
extern const char* NS_flash_proxy;
extern const char* NS_Vector;

///////////////////////////////////////////////////////////////////////////
namespace InstanceTraits
{
    class Traits : public AS3::Traits
    {
    public:
        Traits(VM& vm, const ASString& n, Pickable<Instances::Namespace> ns, Class& c, const Traits* pt, ObjectType ot = SealedObject);
        Traits(VM& vm, const ASString& n, Pickable<Instances::Namespace> ns, Class& c, ObjectType ot = SealedObject);
        ~Traits();
        
    public:
        virtual void Super(const AS3::Traits& /*ot*/, const Value& _this, unsigned argc, const Value* argv) const;

        // Determines whether these traits implement the specified
        // interface class; implemented in InstanceTraits::UserDefined
        // This implementation is very similar to UserDefined::SupportsInterface()
        virtual bool SupportsInterface(const ClassTraits::Traits&) const;
        // Obtain class Info, but only if UserDefined - null otherwise.
        virtual const Abc::ClassInfo* GetClassInfo() const;

        virtual ASString GetName() const;
        virtual ASString GetQualifiedName(QNameFormat f = qnfWithColons) const;

        virtual void ForEachChild_GC(GcOp op) const;

        const ArrayLH<Multiname>& GetImplementedInterfaces() const
        {
            return ImplementsInterfaces;
        }
        const Instances::Namespace& GetNamespace() const 
        { 
            return *Ns; 
        }
        const Traits* GetParent() const
        {
            return static_cast<const Traits*>(AS3::Traits::GetParent());
        }

    protected:
        void RegisterImplementedInterface(const Multiname& mn)
        {
            return ImplementsInterfaces.PushBack(mn);
        }

    private:
        ASString Name;
        SPtr<Instances::Namespace> Ns;
        // This member is used only by C++ Traits. AS3 Traits use Abc::Instance
        // to get info about implemented interfaces.
        ArrayLH<Multiname> ImplementsInterfaces;
    };

    // This is Traits, which cannot create objects.
    // Such paradigm is used at several places.
    class Prototype : public Traits
    {
    public:
        Prototype(VM& vm, const ASString& n, Class& c);

    public:
        virtual void MakeObject(Value& result);
    };

    // Doesn't belong to any class.
    // It is more like the ClassTraits::Traits.
    class Anonimous : public Prototype
    {
    public:
        Anonimous(VM& vm);
    };

} // namespace InstanceTraits

inline
void ClassTraits::Traits::SetInstanceTraits(InstanceTraits::Traits& itr)
{
    ITraits.SetPtr(&itr);
}

///////////////////////////////////////////////////////////////////////////
// Instance of a class ...

class Instance : public AS3::Object
{
    friend class Instances::Object; // Because of Instance(VM& vm).
    
public:
    Instance(InstanceTraits::Traits& t)
    : AS3::Object(t)
    {
    }
    
private:
    Instance(VM& vm)
    : AS3::Object(vm)
    {
    }
    
public:
    virtual void Construct(Value& result, unsigned argc, const Value* argv, bool extCall = false);
    virtual ASString GetName() const;
    
    InstanceTraits::Traits& GetInstanceTraits()
    {
        return static_cast<InstanceTraits::Traits&>(GetTraits());
    }
};

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_Instance_H

