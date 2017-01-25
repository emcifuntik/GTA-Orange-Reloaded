/**********************************************************************

Filename    :   GFxASBlurFilter.cpp
Content     :   BlurFilter reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva
Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFxConfig.h"

#ifdef GFX_AS2_ENABLE_FILTERS

#include "AS2/AS2_BlurFilter.h"

#include "AS2/AS2_Action.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// BlurFilterObject::BlurFilterObject()
// : BitmapFilterObject(FilterDesc::Filter_Blur)
// {
// }
// 
BlurFilterObject::BlurFilterObject(Environment* penv)
: BitmapFilterObject(penv, FilterDesc::Filter_Blur)
{
}

BlurFilterObject::~BlurFilterObject()
{
}

bool BlurFilterObject::SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags /* = PropFlags */)
{
    if (name == "blurX")
    {
        SetBlurX((float)val.ToNumber(penv));
        return true;
    }
    else if (name == "blurY")
    {
        SetBlurY((float)val.ToNumber(penv));
        return true;
    }
    else if (name == "quality")
    {
        Log* plog = penv->GetLog();
        if (plog)
            plog->LogWarning("BlurFilter.quality property is not supported!");
        return true;
    }
    return Object::SetMember(penv, name, val, flags);
}

bool BlurFilterObject::GetMember(Environment* penv, const ASString& name, Value* val)
{
    if (name == "blurX")
    {
        val->SetNumber(GetBlurX());
        return true;
    }
    else if (name == "blurY")
    {
        val->SetNumber(GetBlurY());
        return true;
    }
    else if (name == "quality")
    {
        Log* plog = penv->GetLog();
        if (plog)
            plog->LogWarning("BlurFilter.quality property is not supported!");
        return true;
    }
    return Object::GetMember(penv, name, val);
}


// --------------------------------------------------------------------

const NameFunction BlurFilterProto::FunctionTable[] = 
{
    { "clone",         &BlurFilterProto::Clone },
    { 0, 0 }
};

BlurFilterProto::BlurFilterProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor) :
Prototype<BlurFilterObject>(psc, prototype, constructor)
{
    InitFunctionMembers(psc, FunctionTable, 0);
}

void BlurFilterProto::Clone(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, BlurFilter);
    BlurFilterObject* pthis = (BlurFilterObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    Ptr<Object> pfilter = *fn.Env->OperatorNew(fn.Env->GetGC()->FlashFiltersPackage, 
        fn.Env->GetBuiltin(ASBuiltin_BlurFilter), 0);
    BlurFilterObject* pnew = static_cast<BlurFilterObject*>(pfilter.GetPtr());
    pnew->SetFilter(pthis->GetFilter());
    fn.Result->SetAsObject(pfilter);
}


// --------------------------------------------------------------------


BlurFilterCtorFunction::BlurFilterCtorFunction(ASStringContext* psc) :
CFunctionObject(psc, BlurFilterCtorFunction::GlobalCtor)
{
    SF_UNUSED(psc);
}

void BlurFilterCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<BlurFilterObject> pnode;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_BlurFilter)
        pnode = static_cast<BlurFilterObject*>(fn.ThisPtr);
    else
        pnode = *SF_HEAP_NEW(fn.Env->GetHeap()) BlurFilterObject(fn.Env);
    fn.Result->SetAsObject(pnode.GetPtr());

    pnode->SetAlpha(1.0f);
    pnode->SetBlurX(4.0f);
    pnode->SetBlurY(4.0f);
    // the blur AS2 class does not support this property,
    // but the internal representation requires it to be set to 1
    pnode->SetStrength(1.0f);

    if (fn.NArgs > 0)
    {
        // process blurX
        pnode->SetBlurX((float)fn.Arg(0).ToNumber(fn.Env));

        if (fn.NArgs > 1)
        {
            // process blurY
            pnode->SetBlurY((float)fn.Arg(1).ToNumber(fn.Env));

            if (fn.NArgs > 2)
            {
                // process quality 
                // NOT SUPPORTED
            }
        }
    }

    ASStringContext* psc = fn.Env->GetSC();
    pnode->SetMemberRaw(psc, psc->CreateConstString("blurX"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("blurY"), Value::UNSET);
    // props not yet supported (returns as undefined)
    pnode->SetMemberRaw(psc, psc->CreateConstString("quality"), Value::UNSET);
}

Object* BlurFilterCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) BlurFilterObject(penv);
}

FunctionRef BlurFilterCtorFunction::Register(GlobalContext* pgc)
{
    // Register BitmapFilter here also
    if (!pgc->GetBuiltinClassRegistrar(pgc->GetBuiltin(ASBuiltin_BitmapFilter)))
        BitmapFilterCtorFunction::Register(pgc);

    ASStringContext sc(pgc, 8);
    FunctionRef ctor(*SF_HEAP_NEW(pgc->GetHeap()) BlurFilterCtorFunction(&sc));
    Ptr<BlurFilterProto> proto = 
        *SF_HEAP_NEW(pgc->GetHeap()) BlurFilterProto(&sc, pgc->GetPrototype(ASBuiltin_BitmapFilter), ctor);
    pgc->SetPrototype(ASBuiltin_BlurFilter, proto);
    pgc->FlashFiltersPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_BlurFilter), Value(ctor));
    return ctor;
}

}}} //SF::GFx::AS2

#endif  // SF_NO_FXPLAYER_AS_FILTERS
