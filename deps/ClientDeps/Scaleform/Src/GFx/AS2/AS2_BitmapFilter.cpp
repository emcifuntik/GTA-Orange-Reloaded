/**********************************************************************

Filename    :   GFxASBitmapFilter.cpp
Content     :   BitmapFilter reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva
Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFxConfig.h"

#ifdef GFX_AS2_ENABLE_FILTERS

#include "AS2/AS2_BitmapFilter.h"

#include "AS2/AS2_Action.h"

namespace Scaleform { namespace GFx { namespace AS2 {

//BitmapFilterObject::BitmapFilterObject(FilterDesc::FilterType ft)
//{
//    Filter.Flags = (UInt8)ft;
//}

BitmapFilterObject::BitmapFilterObject(Environment* penv, FilterDesc::FilterType ft)
: Object(penv)
{
    SF_UNUSED(penv);
    Filter.Flags = (UInt8)ft;
}

BitmapFilterObject::~BitmapFilterObject()
{

}


// --------------------------------------------------------------------


BitmapFilterProto::BitmapFilterProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor) :
Prototype<BitmapFilterObject>(psc, prototype, constructor)
{
}


// --------------------------------------------------------------------


BitmapFilterCtorFunction::BitmapFilterCtorFunction(ASStringContext* psc)
:   CFunctionObject(psc, NULL)
{
    SF_UNUSED(psc);
}


FunctionRef BitmapFilterCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef ctor(*SF_HEAP_NEW(pgc->GetHeap()) BitmapFilterCtorFunction(&sc));
    Ptr<BitmapFilterProto> proto = 
        *SF_HEAP_NEW(pgc->GetHeap()) BitmapFilterProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_BitmapFilter, proto);
    pgc->FlashFiltersPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_BitmapFilter), Value(ctor));
    return ctor;
}

}}} //SF::GFx::AS2
#endif  // SF_NO_FXPLAYER_AS_FILTERS
