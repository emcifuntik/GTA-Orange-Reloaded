/**********************************************************************

Filename    :   AS3_GFxValueImpl.cpp
Content     :   Complex Objects API Implementation
Created     :   
Authors     :   Prasad Silva, Michael Antonov

Copyright   :   (c) 2009 Scaleform Corp. All Rights Reserved.

Notes       :   Contains MovieImpl, Value::ObjectInterface and 
                Value method definitions.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_TextField.h"
#include "Obj/AS3_Obj_Array.h"
#include "AS3_Slot.h"
#include "AS3_MovieRoot.h"

#include "AS3_AvmSprite.h"
#include "AS3_AvmInteractiveObj.h"
#include "AS3_AvmTextField.h"
#include "AS3_AvmDisplayObjContainer.h"
#include "Obj/Display/AS3_Obj_Display_DisplayObjectContainer.h"
#include "Obj/AS3_Obj_Function.h"
#include "Kernel/SF_MsgFormat.h"

//#include "Render/Render_Renderer.h"


namespace Scaleform { namespace GFx {

#define GFX_VALUE_OFFSETOF(st, m) ((size_t) ( (char *)&((st *)(16))->m - (char *)16 ))


// ***** Custom AS2 function object

namespace AS3 {

// Custom FunctionObject that wraps the function context as well as
// the user data.
class UserDefinedFunction : public Instances::FunctionBase
{
protected:
    mutable Ptr<FunctionHandler>    pContext;
    void*                           pUserData; 

public:
    UserDefinedFunction(InstanceTraits::Traits& t, FunctionHandler* pcontext, void* puserData) 
        : FunctionBase(t)
    {
        pContext = pcontext;
        pUserData = puserData;
    }

    // *argv* does not include *this* ...
    // These Execute() methods should be const. It is reasonable.
    // Function call doesn't change function itself.
    virtual void Execute(const AS3::Value& _this, AS3::Value& result, unsigned argc, const AS3::Value* argv) const
    {
        ExecuteImpl(_this, result, argc, argv);
    }
    virtual void Execute(const AS3::Value& _this, unsigned argc, const AS3::Value* argv, bool discard_result = false) const
    {
        SF_UNUSED(discard_result);
        AS3::Value result;
        ExecuteImpl(_this, result, argc, argv);
    }

    virtual UInt32 GetMaxParamNum() const { return 1023; }

private:
    void    ExecuteImpl(const AS3::Value& _this, AS3::Value& result, unsigned argc, const AS3::Value* argv) const;
};


void UserDefinedFunction::ExecuteImpl(const AS3::Value& _this, AS3::Value& result, 
                                      unsigned argc, const AS3::Value* argv) const
{
    ArrayCPP<GFx::Value> args;
    GFx::Value thisVal, retVal;

    SF_ASSERT(_this.IsObject());    // Must be at least the global object
    ASVM& vm = (ASVM&)(GetVM());
    MovieRoot* pmovieRoot = vm.GetMovieRoot();
    SF_ASSERT(pmovieRoot);

    pmovieRoot->ASValue2GFxValue(_this, &thisVal);
    args.PushBack(thisVal); // Used to pack 'this' on callback

    for (unsigned i = 0; i < argc; i++)
    {
        GFx::Value arg;
        pmovieRoot->ASValue2GFxValue(argv[i], &arg);
        args.PushBack(arg);
    }

    FunctionHandler::Params params;
    params.pMovie               = vm.GetMovieImpl();
    params.pRetVal              = &retVal;
    params.pThis                = &thisVal;
    params.pArgs                = argc > 0 ? &args[1] : NULL;
    params.pArgsWithThisRef     = &args[0];
    params.ArgCount             = (unsigned)args.GetSize() - 1;
    params.pUserData            = pUserData;
    pContext->Call(params);

    if (!retVal.IsUndefined()) 
        pmovieRoot->GFxValue2ASValue(retVal, &result);
}

} // namespace AS3



// If both VMs are supported, Value::Object interface is implemented
// through virtual functions; if not, it is statically linked.

#if defined(GFX_AS2_SUPPORT) && defined(GFX_AS3_SUPPORT)

class AS3ValueObjectInterface : public MovieImpl::ValueObjectInterface
{
    typedef Value::DisplayInfo DisplayInfo;

    // Implementation of Value::ObjectInterface
    void    ObjectAddRef(Value* val, void* pobj);
    void    ObjectRelease(Value* val, void* pobj);

    bool    HasMember(void* pdata, const char* name, bool isdobj) const;
    bool    GetMember(void* pdata, const char* name, Value* pval, bool isdobj) const;
    bool    SetMember(void* pdata, const char* name, const Value& value, bool isdobj);
    bool    Invoke(void* pdata, Value* presult, const char* name,
                   const Value* pargs, UPInt nargs, bool isdobj);
    bool    DeleteMember(void* pdata, const char* name, bool isdobj);
    void    VisitMembers(void* pdata, ObjVisitor* visitor, bool isdobj) const;

    unsigned GetArraySize(void* pdata) const;
    bool    SetArraySize(void* pdata, unsigned sz);
    bool    GetElement(void* pdata, unsigned idx, Value *pval) const;
    bool    SetElement(void* pdata, unsigned idx, const Value& value);
    void    VisitElements(void* pdata, ArrVisitor* visitor,
                         unsigned idx, int count) const;
    bool    PushBack(void* pdata, const Value& value);
    bool    PopBack(void* pdata, Value* pval);
    bool    RemoveElements(void* pdata, unsigned idx, int count);

    bool    IsDisplayObjectActive(void* pdata) const;
    bool    GetDisplayInfo(void* pdata, DisplayInfo* pinfo) const;
    bool    SetDisplayInfo(void* pdata, const DisplayInfo& info);
    bool    GetDisplayMatrix(void* pdata, Render::Matrix2F* pmat) const;
    bool    SetDisplayMatrix(void* pdata, const Render::Matrix2F& mat);
    bool    GetMatrix3D(void* pdata, Render::Matrix3F* pmat) const;
    bool    SetMatrix3D(void* pdata, const Render::Matrix3F& mat);
    bool    GetCxform(void* pdata, Render::Cxform* pcx) const;
    bool    SetCxform(void* pdata, const Render::Cxform& cx);

    bool    GetText(void* pdata, Value* pval, bool ishtml) const;
    bool    SetText(void* pdata, const char* ptext, bool ishtml);
    bool    SetText(void* pdata, const wchar_t*, bool ishtml);
    bool    GotoAndPlay(void* pdata, const char* frame, bool stop);
    bool    GotoAndPlay(void* pdata, unsigned frame, bool stop);

    bool    CreateEmptyMovieClip(void* pdata, GFx::Value* pmc, 
                                    const char* instanceName, SInt32 depth);
    bool    AttachMovie(void* pdata, GFx::Value* pmc, 
                            const char* symbolName, const char* instanceName, 
                            SInt32 depth, const MemberValueSet* initArgs);

    void    ToString(String* pstr, const GFx::Value& thisVal) const;

public:
    AS3ValueObjectInterface(MovieImpl* pmovieRoot)
        : MovieImpl::ValueObjectInterface(pmovieRoot) { }

};
#define GFX_Value_ObjectInterface_CLASS AS3ValueObjectInterface

#else
#define GFX_Value_ObjectInterface_CLASS Value::ObjectInterface
#endif


// ***** GFxValue::ObjectInterface or AS3ValueObjectInterface definitions


void GFX_Value_ObjectInterface_CLASS::ObjectAddRef(Value* val, void* pobj)
{
    // Addref complex object
    switch (val->GetType())
    {
    case Value::VT_String:
        {
            // pobj points to const char**
            size_t memberOffset = GFX_VALUE_OFFSETOF(ASStringNode, pData);
            ASStringNode* ps = (ASStringNode*)(((UByte*)pobj) - memberOffset);
            ps->AddRef();
        }
        break;
    case Value::VT_StringW:
        {
            // pobj points to const wchar_t*
            size_t memberOffset = GFX_VALUE_OFFSETOF(MovieImpl::WideStringStorage, pData);
            MovieImpl::WideStringStorage* pws = (MovieImpl::WideStringStorage*)(((UByte*)pobj) - memberOffset);
            pws->AddRef();
        }
        break;
    case Value::VT_DisplayObject:
    case Value::VT_Object:
    case Value::VT_Array:
        ((AS3::Object*)pobj)->AddRef_Unsafe();
        break;
    default:
        SF_ASSERT(0);
    }

#ifdef SF_BUILD_DEBUG
    ExternalObjRefs.PushBack(val);
#endif
}

void GFX_Value_ObjectInterface_CLASS::ObjectRelease(Value* val, void* pobj)
{
    // NOTE: The next statements may cause an access violation exception
    // if the GFxMovie containing the AS object has been released. To
    // avoid this issue, make sure to release any GFx::Values holding
    // references to AS objects before the movie that holds those objects
    // is released.

    switch (val->GetType())
    {
    case Value::VT_String:
        {
            // pobj points to const char**
            size_t memberOffset = GFX_VALUE_OFFSETOF(ASStringNode, pData);
            ASStringNode* ps = (ASStringNode*)(((UByte*)pobj) - memberOffset);
            ps->Release();
        }
        break;
    case Value::VT_StringW:
        {
            // pobj points to const wchar_t*
            size_t memberOffset = GFX_VALUE_OFFSETOF(MovieImpl::WideStringStorage, pData);
            MovieImpl::WideStringStorage* pws = 
                (MovieImpl::WideStringStorage*)(((UByte*)pobj) - memberOffset);
            pws->Release();
        }
        break;
    case Value::VT_DisplayObject:
    case Value::VT_Object:
    case Value::VT_Array:
        ((AS3::Object*)pobj)->Release_Unsafe();
        break;
    default:
        SF_ASSERT(0);
    }

#ifdef SF_BUILD_DEBUG
    ExternalObjRefs.Remove(val);
#endif
}


bool GFX_Value_ObjectInterface_CLASS::HasMember(void* pdata, const char* name,
                                                bool isdobj) const 
{        
    SF_UNUSED(isdobj);
    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);
    AS3::VM*        vm   = root->GetAVM();
    AS3::Object*    obj  = (AS3::Object*)pdata;
    
    AS3::Multiname mn(root->GetAVM()->GetPublicNamespace(),
                      root->GetStringManager()->CreateString(name));
    AS3::PropRef prop = obj->FindProperty(mn);
    SF_ASSERT(!vm->IsException());
    
    if (!prop)
    {
        // Check if object is a display object container
        if (AreDisplayObjectContainerTraits(obj->GetTraitsType()))
        {
            AS3::AvmDisplayObjContainer* pcurr = 
                AS3::ToAvmDisplayObjContainer(static_cast<AS3::Instances::DisplayObjectContainer*>(
                obj)->pDispObj->CharToDisplayObjContainer());
            ASString propName = root->GetStringManager()->CreateString(name);
            AS3::Instances::DisplayObject* pchild = pcurr->GetAS3ChildByName(propName);
            if (pchild)
            {
                SF_ASSERT(pchild->pDispObj);
                return true;
            }
            else
            {
                if (vm->IsException()) vm->IgnoreException();
                return false;
            }
        }
    }

    return (bool)prop;
}


bool GFX_Value_ObjectInterface_CLASS::GetMember(void* pdata, const char* name,
                                                Value* pval, bool isdobj) const
{
    SF_UNUSED(isdobj);
    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);
    AS3::VM*        vm   = root->GetAVM();
    AS3::Object*    obj  = (AS3::Object*)pdata;

    AS3::Multiname mn(vm->GetPublicNamespace(),
        root->GetStringManager()->CreateString(name));

    AS3::Value asval;
    if (!obj->GetProperty(mn, asval))
    {
        // Check if object is a display object container
        if (AreDisplayObjectContainerTraits(obj->GetTraitsType()))
        {
            AS3::AvmDisplayObjContainer* pcurr = 
                AS3::ToAvmDisplayObjContainer(static_cast<AS3::Instances::DisplayObjectContainer*>(
                obj)->pDispObj->CharToDisplayObjContainer());
            ASString propName = root->GetStringManager()->CreateString(name);
            AS3::Instances::DisplayObject* pchild = pcurr->GetAS3ChildByName(propName);
            if (pchild)
            {
                SF_ASSERT(pchild->pDispObj);
                asval = AS3::Value(pchild);
                root->ASValue2GFxValue(asval, pval);
                return true;
            }
            else
            {
                if (vm->IsException()) vm->IgnoreException();
                pval->SetUndefined();
                return false;
            }
        }

        if (vm->IsException())
            vm->OutputAndIgnoreException();
        pval->SetUndefined();
        return false;
    }

    root->ASValue2GFxValue(asval, pval);
    SF_ASSERT(!vm->IsException());
    return true;
}


bool GFX_Value_ObjectInterface_CLASS::SetMember(void* pdata, const char* name,
                                                const Value& value, bool isdobj)
{
    SF_UNUSED(isdobj);
    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);
    AS3::VM*        vm   = root->GetAVM();
    AS3::Object*    obj  = (AS3::Object*)pdata;

    AS3::Multiname  mn(vm->GetPublicNamespace(),
                       root->GetStringManager()->CreateString(name));

    // Check if display list contains a child with same name, and bail if that is the case
    // This is done for DAPI consistency
    if (AreDisplayObjectContainerTraits(obj->GetTraitsType()))
    {
        AS3::AvmDisplayObjContainer* pcurr = 
            AS3::ToAvmDisplayObjContainer(static_cast<AS3::Instances::DisplayObjectContainer*>(
            obj)->pDispObj->CharToDisplayObjContainer());
        ASString propName = root->GetStringManager()->CreateString(name);
        AS3::Instances::DisplayObject* pchild = pcurr->GetAS3ChildByName(propName);
        if (pchild)
        {
            SF_ASSERT(pchild->pDispObj);
            String errMsg;
            Format(errMsg, "Property '{0}' already exists as a DisplayObject. SetMember aborted.", 
                name);
            root->Output(AS3::FlashUI::Output_Error, errMsg);
            return false;
        }
    }

    AS3::Value asval;
    root->GFxValue2ASValue(value, &asval);
    if (!obj->SetProperty(mn, asval))
    {
        if (vm->IsException())
            vm->OutputAndIgnoreException();
        return false;
    }

    SF_ASSERT(!vm->IsException());
    return true;
}


bool GFX_Value_ObjectInterface_CLASS::Invoke(void* pdata, GFx::Value* presult, const char* name,
                                             const GFx::Value* pargs, UPInt nargs, bool isdobj)
{
    SF_UNUSED(isdobj);
    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);
    AS3::VM*        vm   = root->GetAVM();

    AS3::Multiname  mn(vm->GetPublicNamespace(),
                       root->GetStringManager()->CreateString(name));
    AS3::PropRef    prop = ((AS3::Object*)pdata)->FindProperty(mn);
    
    if (!prop)
        return false;
   
    AS3::Value asfn;
    AS3::Value asresult;

    if (!prop.GetSlotValueUnsafe(*vm, asfn))
    {
        vm->OutputAndIgnoreException();
        return false;
    }
    
    if (nargs)
    {
        Array<AS3::Value> args(nargs);
        for (UPInt i = 0; i< nargs; i++)
            root->GFxValue2ASValue(pargs[i], &args[i]);
        vm->Execute(asfn, AS3::Value((AS3::Object*)pdata), asresult, nargs, &args[0]);
    }
    else
    {
        vm->Execute(asfn, AS3::Value((AS3::Object*)pdata), asresult, 0, 0);
    }
       
    if (vm->IsException())
    {
        vm->OutputAndIgnoreException();
        return false;
    }

    if (presult)
        root->ASValue2GFxValue(asresult, presult);
    SF_ASSERT(!vm->IsException());
    return true;
}


bool GFX_Value_ObjectInterface_CLASS::DeleteMember(void* pdata, const char* name, bool isdobj)
{
    SF_UNUSED(isdobj);
    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);
    AS3::VM*        vm   = root->GetAVM();

    AS3::Multiname  mn(vm->GetPublicNamespace(),
                       root->GetStringManager()->CreateString(name));
    
    SF_ASSERT(!vm->IsException());
    return ((AS3::Object*)pdata)->DeleteProperty(mn);
}


void GFX_Value_ObjectInterface_CLASS::VisitMembers(void* pdata, Value::ObjectVisitor* visitor,
                                                   bool isdobj) const
{
    SF_UNUSED(isdobj);
    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);        
    AS3::VM*        vm   = root->GetAVM();

    AS3::Object*    obj  = (AS3::Object*)pdata;

    // Iterate through all dynamic slots
    const AS3::SlotContainerType& dynSlots = obj->GetDynamicSlots();
    for (unsigned i = 0; i < dynSlots.GetSize(); i++)
    {
        AS3::AbsoluteIndex idx(i);
        const ASStringNode* strnode = dynSlots.GetKey(idx);
        if (!strnode) continue;  // !PPS if strnode==NULL, then dynslot was deleted and is invalid; skip over
        const AS3::SlotInfo& slotinfo = dynSlots[idx];
        AS3::Value asval;
        slotinfo.GetSlotValueUnsafe(asval, obj).DoNotCheck();
        GFx::Value  val;
        root->ASValue2GFxValue(asval, &val);
        visitor->Visit(strnode->pData, val);
    }
    
    // !PPS Iterating through fixed slots is not comparable to an AS3 iteration, as the AS3
    // version only looks at dyn slots. The describeType() method - that dumps XML - is used
    // to reflect on the type/fixed traits.
    /*
    const AS3::SlotContainerType& fixedSlots = ((const AS3::Object*)obj)->GetTraits().GetSlots();
    for (unsigned i = 0; i < fixedSlots.GetSize(); i++)
    {
        AS3::AbsoluteIndex idx(i);
        const ASStringNode* strnode = fixedSlots.GetKey(idx);
        SF_ASSERT(strnode);
        const AS3::SlotInfo& slotinfo = fixedSlots[idx];
        if (slotinfo.IsDontEnum()) continue;
        // !PPS We avoid getter/setter for now. TBD
        if (slotinfo.GetBindingType() != AS3::SlotInfo::DT_Get &&
            slotinfo.GetBindingType() != AS3::SlotInfo::DT_Set &&
            slotinfo.GetBindingType() != AS3::SlotInfo::DT_GetSet)
        {
            AS3::Value asval;
            slotinfo.GetSlotValueUnsafe(*vm, asval, obj).DoNotCheck();
            GFx::Value  val;
            root->ASValue2GFxValue(asval, &val);

            if (vm->IsException())
                vm->OutputAndIgnoreException();

            visitor->Visit(strnode->pData, val);
        }
    }
    */

    // Iterate through all child movieclips if obj is a DisplayObjectContainer
    if (AreDisplayObjectContainerTraits(obj->GetTraitsType()))
    {
        AS3::AvmDisplayObjContainer* pcurr = 
            AS3::ToAvmDisplayObjContainer(static_cast<AS3::Instances::DisplayObjectContainer*>(
            obj)->pDispObj->CharToDisplayObjContainer());
        for (unsigned i = 0; i < pcurr->GetNumChildren(); i++) 
        {
            AS3::Instances::DisplayObject* pchild = pcurr->GetAS3ChildAt(i);
            SF_ASSERT(pchild->pDispObj);            
            // Avoid dobjs that weren't dynamically created, a
            if (!pchild->pDispObj->IsTimelineObjectFlagSet())
            {
                ASString name = pchild->pDispObj->GetName();
                AS3::Value asval(pchild);
                GFx::Value val;
                root->ASValue2GFxValue(asval, &val);
                visitor->Visit(name.ToCStr(), val);
            }
        }
    }

    SF_ASSERT(!vm->IsException());
}


unsigned GFX_Value_ObjectInterface_CLASS::GetArraySize(void* pdata) const
{
    AS3::Instances::Array* arr = (AS3::Instances::Array*) (AS3::Object*) pdata;
    return (unsigned)arr->GetSize();
}


bool GFX_Value_ObjectInterface_CLASS::SetArraySize(void* pdata, unsigned sz)
{
    AS3::Instances::Array* arr = (AS3::Instances::Array*) (AS3::Object*) pdata;
    arr->Resize(sz);
    return true;
}


bool GFX_Value_ObjectInterface_CLASS::GetElement(void* pdata, unsigned idx, Value *pval) const
{
    AS3::Instances::Array* arr = (AS3::Instances::Array*) (AS3::Object*) pdata;
        
    pval->SetUndefined();
    if (idx >= (unsigned)arr->GetSize())
        return false;

    AS3::MovieRoot*   root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);
    const AS3::Value& asval = arr->At(idx);
    root->ASValue2GFxValue(asval, pval);
    return true;
}


bool GFX_Value_ObjectInterface_CLASS::SetElement(void* pdata, unsigned idx, const Value& value)
{
    AS3::Instances::Array* arr  = (AS3::Instances::Array*) (AS3::Object*) pdata;
    AS3::MovieRoot*        root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);
    
    AS3::Value asval;
    root->GFxValue2ASValue(value, &asval);
    arr->Set(idx, asval);
    return true;
}


void GFX_Value_ObjectInterface_CLASS::VisitElements(void* pdata, Value::ArrayVisitor* visitor,
                                                    unsigned idx, int count) const
{
    AS3::Instances::Array* arr  = (AS3::Instances::Array*) (AS3::Object*) pdata;
    AS3::MovieRoot*        root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);
    
    unsigned sz = (unsigned)arr->GetSize();
    if (idx >= sz)
        return;
    if (count < 0)
        count = (sz - idx);

    GFx::Value val;
    unsigned   eidx = Alg::Min(sz, idx + count);

    for (unsigned i=idx; i < eidx; i++)
    {
        const AS3::Value& asval = arr->At(i);
        root->ASValue2GFxValue(asval, &val);
        visitor->Visit(i, val);
    }
}


bool GFX_Value_ObjectInterface_CLASS::PushBack(void *pdata, const Value &value)
{
    AS3::Instances::Array* arr  = (AS3::Instances::Array*) (AS3::Object*) pdata;
    AS3::MovieRoot*        root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);

    AS3::Value asval;
    root->GFxValue2ASValue(value, &asval);
    arr->PushBack(asval);
    return true;
}


bool GFX_Value_ObjectInterface_CLASS::PopBack(void* pdata, Value* pval)
{
    AS3::Instances::Array* arr  = (AS3::Instances::Array*) (AS3::Object*) pdata;
    AS3::MovieRoot*        root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);

    int sz = arr->GetSize();
    if (sz <= 0) 
    {
        if (pval) pval->SetUndefined();
        return false;
    }
    if (pval)
    {
        const AS3::Value& asval = arr->At(sz-1);
        root->ASValue2GFxValue(asval, pval);
    }
    arr->Resize(sz-1);
    return true;
}


bool GFX_Value_ObjectInterface_CLASS::RemoveElements(void *pdata, unsigned idx, int count)
{
    AS3::Instances::Array* arr  = (AS3::Instances::Array*) (AS3::Object*) pdata; 

    unsigned sz = (unsigned)arr->GetSize();
    if (idx >= sz) return false;
    if (count < 0) { count = (sz - idx); }

    arr->RemoveMultipleAt(idx, Alg::Min(sz - idx, (unsigned)count));
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::GetDisplayMatrix(void* pdata, Render::Matrix2F* pmat) const
{
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj->GetTraitsType())) return false;
    DisplayObject* pd = static_cast<AS3::Instances::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    Render::Matrix2F m = pd->GetMatrix();
    m.Ty() = TwipsToPixels(m.Tx());
    m.Tx() = TwipsToPixels(m.Ty());
    *pmat = m;

    return true;
}


bool GFX_Value_ObjectInterface_CLASS::SetDisplayMatrix(void* pdata, const Render::Matrix2F& mat)
{
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj->GetTraitsType())) return false;
    DisplayObject* pd = static_cast<AS3::Instances::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    // Extra checks
    if (!mat.IsValid())
    {
        SF_DEBUG_WARNING(1, "GFxValue::SetDisplayMatrix called with an invalid matrix.\n"
            "At least one of the matrix values is invalid (non-finite).\n");
        return false;
    }

    Render::Matrix2F m = mat;
    m.Tx() = PixelsToTwips(m.Tx());
    m.Ty() = PixelsToTwips(m.Ty());
    pd->SetMatrix(m);

    // Also set the geom data structure to reflect changes for the runtime
    DisplayObject::GeomDataType geomData;
    pd->GetGeomData(geomData);
    geomData.X = int(mat.Tx());
    geomData.Y = int(mat.Ty());
    geomData.XScale = mat.GetXScaleDouble()*(Double)100.;
    geomData.YScale = mat.GetYScaleDouble()*(Double)100.;
    geomData.Rotation = (mat.GetRotationDouble()*(Double)180.)/SF_MATH_PI;
    pd->SetGeomData(geomData);

    return true;
}

bool GFX_Value_ObjectInterface_CLASS::GetMatrix3D(void* pdata, Render::Matrix3F* pmat) const
{
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj->GetTraitsType())) return false;
    DisplayObject* pd = static_cast<AS3::Instances::DisplayObject*>(obj)->pDispObj; 
    SF_ASSERT(pd);

    Render::Matrix3F m = pd->GetMatrix3D();
    m.Tx() = TwipsToPixels(m.Tx());
    m.Ty() = TwipsToPixels(m.Ty());
    m.Tz() = TwipsToPixels(m.Tz());
    *pmat = m;
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::SetMatrix3D(void* pdata, const Render::Matrix3F& mat)
{
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj->GetTraitsType())) return false;
    DisplayObject* pd = static_cast<AS3::Instances::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    // Extra checks
    if (!mat.IsValid())
    {
        SF_DEBUG_WARNING(1, "GFxValue::SetDisplayMatrix called with an invalid matrix.\n"
            "At least one of the matrix values is invalid (non-finite).\n");
        return false;
    }

    Render::Matrix3F m = mat;
    m.Tx() = PixelsToTwips(m.Tx());
    m.Ty() = PixelsToTwips(m.Ty());
    m.Tz() = PixelsToTwips(m.Tz());
    pd->SetMatrix3D(m);

    // Also set the geom data structure to reflect changes for the runtime
    DisplayObject::GeomDataType geomData;
    pd->GetGeomData(geomData);
    geomData.Z = mat.Tz();
    geomData.ZScale = mat.GetZScale()*(Double)100.;

    float xr, yr;
    mat.GetRotation(&xr, &yr, NULL);
    geomData.XRotation = SF_RADTODEG(xr);
    geomData.YRotation = SF_RADTODEG(yr);

    pd->SetGeomData(geomData);
    return true;
}

bool    GFX_Value_ObjectInterface_CLASS::IsDisplayObjectActive(void* pdata) const
{
    AS3::Object*    obj = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj->GetTraitsType())) return false;
    DisplayObject* pd = static_cast<AS3::Instances::DisplayObject*>(obj)->pDispObj;    
    return pd != NULL;
}

bool GFX_Value_ObjectInterface_CLASS::GetDisplayInfo(void* pdata, Value::DisplayInfo* pinfo) const
{
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj->GetTraitsType())) return false;
    DisplayObject* pd = static_cast<AS3::Instances::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);
    if (!pd) return false;

    DisplayObject::GeomDataType geomData;
    pd->GetGeomData(geomData);    
    if (pd->GetType() == CharacterDef::TextField)
    {
        // Special case for TextField
        ((TextField*)pd)->GetPosition(pinfo);
    }
    else
    {
        Double x = TwipsToPixels(Double(geomData.X));
        Double y = TwipsToPixels(Double(geomData.Y));
        Double rotation = geomData.Rotation;
        Double xscale = geomData.XScale;
        Double yscale = geomData.YScale;
        Double alpha = pd->GetCxform().M[3][0] * 100.F;
        bool visible = pd->GetVisible();
        Double xrotation = geomData.XRotation;
        Double yrotation = geomData.YRotation;
        Double zscale = geomData.ZScale;
        Double z = TwipsToPixels(geomData.Z);
        pinfo->Set(x, y, rotation, xscale, yscale, alpha, visible, z, xrotation, yrotation, zscale);
        pinfo->SetFOV(pd->GetFOV());        

        Matrix4F matProj;
        if (!pd->GetProjectionMatrix3D(&matProj))
            matProj = *pd->GetMovieImpl()->GetProjectionMatrix3D();
        pinfo->SetProjectionMatrix3D(&matProj);

        Matrix3F matView;
        if (!pd->GetViewMatrix3D(&matView))
            matView = *pd->GetMovieImpl()->GetViewMatrix3D();
        pinfo->SetViewMatrix3D(&matView);
    }    
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::SetDisplayInfo(void *pdata, const Value::DisplayInfo &cinfo)
{
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj->GetTraitsType())) return false;
    DisplayObject* pd = static_cast<AS3::Instances::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    bool Update3DTransforms = false;

    // Check for TextField special case:
    bool istf = (pd->GetType() == CharacterDef::TextField);
    TextField* ptf = static_cast<TextField*>(pd);

    if (cinfo.IsFlagSet(DisplayInfo::V_alpha))
    {
        if (!NumberUtil::IsNaN(cinfo.GetAlpha()))
        {
            // Set alpha modulate, in percent.
            InteractiveObject::Cxform  cx = pd->GetCxform();
            cx.M[3][0] = float(cinfo.GetAlpha() / 100.);
            pd->SetCxform(cx);
            pd->SetAcceptAnimMoves(0);
        }
    }

    if (cinfo.IsFlagSet(DisplayInfo::V_visible))
    {
        pd->SetVisible(cinfo.GetVisible());
    }


    if (cinfo.IsFlagSet(DisplayInfo::V_z))
    {
        Double zval = PixelsToTwips(cinfo.GetZ());
        if (NumberUtil::IsNaN(zval))
            zval = 0;
        if (NumberUtil::IsNEGATIVE_INFINITY(zval) || NumberUtil::IsPOSITIVE_INFINITY(zval))
            zval = 0;

        // pd->EnsureGeomDataCreated(); // let timeline anim continue
        DisplayObjectBase::GeomDataType* tfgeomData = pd->GetGeomDataPtr();
        SF_ASSERT(tfgeomData);

        if (zval != tfgeomData->Z)
        {
            tfgeomData->Z = zval;
            Update3DTransforms = true;
        }
    }

    if (cinfo.IsFlagSet(DisplayInfo::V_zscale))
    {
        Double newZScale = cinfo.GetZScale();
        if (NumberUtil::IsNaN(newZScale) ||
            NumberUtil::IsNEGATIVE_INFINITY(newZScale) || NumberUtil::IsPOSITIVE_INFINITY(newZScale))
        {
            newZScale = 100.;
        }

        // pd->EnsureGeomDataCreated(); // let timeline anim continue
        DisplayObjectBase::GeomDataType* tfgeomData = pd->GetGeomDataPtr();
        SF_ASSERT(tfgeomData);

        if (tfgeomData->ZScale != newZScale)
        {
            tfgeomData->ZScale = newZScale;
            Update3DTransforms = true;
        }
    }

    if (cinfo.IsFlagSet(DisplayInfo::V_xrotation))
    {
        Double rval = cinfo.GetXRotation();

        // pd->EnsureGeomDataCreated(); // let timeline anim continue
        DisplayObjectBase::GeomDataType* tfgeomData = pd->GetGeomDataPtr();
        SF_ASSERT(tfgeomData);

        if (rval != tfgeomData->XRotation)
        {       
            Double r = fmod((Double)rval, (Double)360.);
            if (r > 180)
                r -= 360;
            else if (r < -180)
                r += 360;

            tfgeomData->XRotation = r;
            Update3DTransforms = true;
        }
    }

    if (cinfo.IsFlagSet(DisplayInfo::V_yrotation))
    {
        Double rval = cinfo.GetYRotation();

        // pd->EnsureGeomDataCreated(); // let timeline anim continue
        DisplayObjectBase::GeomDataType* tfgeomData = pd->GetGeomDataPtr();
        SF_ASSERT(tfgeomData);

        if (rval != tfgeomData->YRotation)
        {      
            Double r = fmod((Double)rval, (Double)360.);
            if (r > 180)
                r -= 360;
            else if (r < -180)
                r += 360;

            tfgeomData->YRotation= r;
            Update3DTransforms = true;
        }
    }

    if (Update3DTransforms)
        pd->UpdateTransform3D();

    if (cinfo.IsFlagSet(DisplayInfo::V_FOV))
    {
        Double rval = cinfo.GetFOV();
        if (rval != pd->GetFOV())
        {       
            Double r = fmod((Double)rval, (Double)180.);
            pd->SetFOV(r);
        }
    }

    if (cinfo.IsFlagSet(DisplayInfo::V_projMatrix3D))
    {
        pd->SetProjectionMatrix3D(*cinfo.GetProjectionMatrix3D());
    }

    if (cinfo.IsFlagSet(DisplayInfo::V_viewMatrix3D))
    {
        pd->SetViewMatrix3D(*cinfo.GetViewMatrix3D());
    }

    if (cinfo.IsFlagSet(DisplayInfo::V_x|DisplayInfo::V_y|DisplayInfo::V_rotation|
                       DisplayInfo::V_xscale|DisplayInfo::V_yscale))
    {       
        // Special case for TextField:
        if (istf && cinfo.IsFlagSet(DisplayInfo::V_xscale|DisplayInfo::V_yscale|
                                   DisplayInfo::V_rotation))
            ptf->SetNeedUpdateGeomData();

        pd->SetAcceptAnimMoves(0);
        DisplayObjectBase::GeomDataType* tfgeomData = pd->GetGeomDataPtr();
        SF_ASSERT(tfgeomData);

        InteractiveObject::Matrix m = pd->GetMatrix();

        Render::PointD pt;
        // Special case for TextField:
        if (istf && cinfo.IsFlagSet(DisplayInfo::V_x|DisplayInfo::V_y))
        {
            Render::PointF tpt = ptf->TransformToTextRectSpace(cinfo);
            pt.x = tpt.x;
            pt.y = tpt.y;
        }
        else
        {
            if (cinfo.IsFlagSet(DisplayInfo::V_x))
                pt.x = cinfo.GetX();
            if (cinfo.IsFlagSet(DisplayInfo::V_y))
                pt.y = cinfo.GetY();
        }

        // Perform scale/rotation before x/y is set.
        // If order should be changed, call this method twice in expected order.

        if (cinfo.IsFlagSet(DisplayInfo::V_rotation|DisplayInfo::V_xscale|DisplayInfo::V_yscale))
        {
            InteractiveObject::Matrix om = tfgeomData->OrigMatrix;
            om.Tx() = m.Tx();
            om.Ty() = m.Ty();

            Double origRotation = om.GetRotationDouble();
            Double origXScale = om.GetXScaleDouble();
            Double origYScale = om.GetYScaleDouble();
            
            Double newXScale = tfgeomData->XScale / 100;
            Double newYScale = tfgeomData->YScale / 100;
            Double newRotation = SF_DEGTORAD(tfgeomData->Rotation);

            // _rotation
            Double rval = cinfo.IsFlagSet(DisplayInfo::V_rotation) ? cinfo.GetRotation() : NumberUtil::NaN();
            if (!NumberUtil::IsNaN(rval))
            {
                newRotation = fmod((Double)rval, (Double)360.);
                if (newRotation > 180)
                    newRotation -= 360;
                else if (newRotation < -180)
                    newRotation += 360;
                tfgeomData->Rotation = newRotation;
                newRotation = SF_DEGTORAD(newRotation); // Convert to rads
            }
            // _xscale
            Double nx = cinfo.IsFlagSet(DisplayInfo::V_xscale) ? (cinfo.GetXScale()/100) : NumberUtil::NaN();
            if ((nx != newXScale) && !NumberUtil::IsNaNOrInfinity(nx))
            {
                tfgeomData->XScale = cinfo.GetXScale();
                if (origXScale == 0 || nx > 1E+16)
                {
                    nx = 0;
                    origXScale = 1;
                }
                newXScale = nx;
            }
            // _yscale
            Double ny = cinfo.IsFlagSet(DisplayInfo::V_yscale) ? (cinfo.GetYScale()/100) : NumberUtil::NaN();
            if ((ny != origYScale) && !NumberUtil::IsNaNOrInfinity(ny))
            {                
                tfgeomData->YScale = cinfo.GetYScale();
                if (origYScale == 0 || ny > 1E+16)
                {
                    ny = 0;
                    origYScale = 1;
                }
                newYScale = ny;
            }              

            // remove old rotation by rotate back and add new one

            ASCharacter_MatrixScaleAndRotate2x2(om,
                float(newXScale/origXScale), 
                float(newYScale/origYScale),
                float(newRotation - origRotation));

            m = om;
        }

        // _x
        Double xval = cinfo.IsFlagSet(DisplayInfo::V_x) ? pt.x : NumberUtil::NaN();
        if (!NumberUtil::IsNaN(xval))
        {
            if (NumberUtil::IsNEGATIVE_INFINITY(xval) || NumberUtil::IsPOSITIVE_INFINITY(xval))
                xval = 0;
            tfgeomData->X = int(floor(PixelsToTwips(xval)));
            m.Tx() = (float) tfgeomData->X;
        }
        // _y
        Double yval = cinfo.IsFlagSet(DisplayInfo::V_y) ? pt.y : NumberUtil::NaN();
        if (!NumberUtil::IsNaN(yval))
        {
            if (NumberUtil::IsNEGATIVE_INFINITY(yval) || NumberUtil::IsPOSITIVE_INFINITY(yval))
                yval = 0;
            tfgeomData->Y = int(floor(PixelsToTwips(yval)));
            m.Ty() = (float) tfgeomData->Y;
        }

        if (m.IsValid())
            pd->SetMatrix(m);

        // Special case for TextField:
        if (istf) 
        {
            if (cinfo.IsFlagSet(Value::DisplayInfo::V_x))
                tfgeomData->X = Alg::IRound(PixelsToTwips(pt.x));
            if (cinfo.IsFlagSet(Value::DisplayInfo::V_y))
                tfgeomData->Y = Alg::IRound(PixelsToTwips(pt.y));
        }
    }

    return true;
}

bool GFX_Value_ObjectInterface_CLASS::SetText(void *pdata, const char *ptext, bool reqHtml)
{
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj->GetTraitsType())) return false;
    DisplayObject* pd = static_cast<AS3::Instances::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    if (pd->GetType() != CharacterDef::TextField)
        return SetMember(pdata, (reqHtml) ? "htmlText" : "text", ptext, true);
    else 
    {
        TextField* ptf = static_cast<TextField*>(pd);
        ptf->SetText(ptext, reqHtml);
        return true;
    }
}

bool GFX_Value_ObjectInterface_CLASS::SetText(void *pdata, const wchar_t *ptext, bool reqHtml)
{
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj->GetTraitsType())) return false;
    DisplayObject* pd = static_cast<AS3::Instances::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    if (pd->GetType() != CharacterDef::TextField)
        return SetMember(pdata, (reqHtml) ? "htmlText" : "text", ptext, true);
    else 
    {
        TextField* ptf = static_cast<TextField*>(pd);
        ptf->SetText(ptext, reqHtml);
        return true;
    }
}

bool GFX_Value_ObjectInterface_CLASS::GetText(void* pdata, Value* pval, bool reqHtml) const
{
    AS3::MovieRoot*     root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);
    AS3::Object*        obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj->GetTraitsType())) return false;
    DisplayObject* pd = static_cast<AS3::Instances::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    if (pd->GetType() != CharacterDef::TextField)
        return GetMember(pdata, reqHtml ? "htmlText" : "text", pval, true);
    else
    {
        TextField* ptf = static_cast<TextField*>(pd);
        ASString text = ptf->GetText(reqHtml);
        root->ASValue2GFxValue(text, pval);
        return true;
    }
}

bool GFX_Value_ObjectInterface_CLASS::GotoAndPlay(void *pdata, const char *frame, bool stop)
{
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj->GetTraitsType())) return false;
    DisplayObject* pd = static_cast<AS3::Instances::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    if (!pd->IsSprite()) return false;

    unsigned frameNum;
    Sprite* pspr = pd->CharToSprite_Unsafe();
    if (pspr->GetLabeledFrame(frame, &frameNum))
    {
        pspr->GotoFrame(frameNum);
        pspr->SetPlayState(stop ? State_Stopped : State_Playing);
        return true;
    }
    return false;
}

bool GFX_Value_ObjectInterface_CLASS::GotoAndPlay(void *pdata, unsigned frame, bool stop)
{
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj->GetTraitsType())) return false;
    DisplayObject* pd = static_cast<AS3::Instances::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    if (!pd->IsSprite()) return false;

    // Incoming frame number is 1-based, but internally GFx is 0 based.
    Sprite* pspr = pd->CharToSprite_Unsafe();
    pspr->GotoFrame(frame-1);
    pspr->SetPlayState(stop ? State_Stopped : State_Playing);
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::GetCxform(void *pdata, Render::Cxform *pcx) const
{
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj->GetTraitsType())) return false;
    DisplayObject* pd = static_cast<AS3::Instances::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    *pcx = pd->GetCxform();
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::SetCxform(void *pdata, const Render::Cxform &cx)
{
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj->GetTraitsType())) return false;
    DisplayObject* pd = static_cast<AS3::Instances::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    pd->SetCxform(cx);
    pd->SetAcceptAnimMoves(false);
    return true;
}

static DisplayObjContainer* AS3ToGFxDisplayObjContainer(AS3::Object* po)
{
    return static_cast<DisplayObjContainer*>(
        static_cast<AS3::Instances::DisplayObjectContainer*>(po)->pDispObj.GetPtr()
        );
}

bool GFX_Value_ObjectInterface_CLASS::CreateEmptyMovieClip(void* pdata, GFx::Value* pmc, 
                                                     const char* instanceName, SInt32 depth)
{
    AS3::MovieRoot*        root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);
    AS3::Object*    parentObj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectContainerTraits(parentObj->GetTraitsType())) return false;
    DisplayObjContainer* parentDispObj = AS3ToGFxDisplayObjContainer(parentObj);
    SF_ASSERT(parentDispObj);

    AS3::Value asObj;
    AS3::ASVM* pAVM = root->GetAVM();
    SF_ASSERT(pAVM);
    const bool need2execute = pAVM->Construct("flash.display.Sprite", asObj, 0, 0, false); 
    if (!pAVM->IsException()) 
    {
        if (need2execute)
            pAVM->ExecuteCode();

        SF_ASSERT(asObj.IsObject());
        if (!AreDisplayObjectTraits(asObj.GetObject()->GetTraitsType())) return false;
        AS3::Instances::DisplayObject* pmcObj = static_cast<AS3::Instances::DisplayObject*>(asObj.GetObject());
        AS3::Value undefVal;
        pmcObj->nameSet(undefVal, root->GetStringManager()->CreateString(instanceName));

        // Add new display obj to parent
        AS3::AvmDisplayObjContainer* container = AS3::ToAvmDisplayObjContainer(parentDispObj);
        unsigned index = (depth < 0) ? container->GetNumChildren() : depth;
        container->AddChildAt(pmcObj->pDispObj, index);

        root->ASValue2GFxValue(asObj, pmc);
        return true;
    }
    return false;
}

bool GFX_Value_ObjectInterface_CLASS::AttachMovie(void* pdata, GFx::Value* pmc, 
                                            const char* symbolName, const char* instanceName, 
                                            SInt32 depth, const MemberValueSet* initArgs)
{
    AS3::MovieRoot*        root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);
    AS3::Object*    parentObj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectContainerTraits(parentObj->GetTraitsType())) return false;
    DisplayObjContainer* parentDispObj = AS3ToGFxDisplayObjContainer(parentObj);
    SF_ASSERT(parentDispObj);

    AS3::Value asObj;
    AS3::ASVM* pAVM = root->GetAVM();
    SF_ASSERT(pAVM);
    const bool need2execute = pAVM->Construct(symbolName, asObj, 0, 0, false); 
    if (!pAVM->IsException()) 
    {
        if (need2execute)
            pAVM->ExecuteCode();

        // ** Simulate attachMovie
        SF_ASSERT(asObj.IsObject());
        if (!AreDisplayObjectTraits(asObj.GetObject()->GetTraitsType())) return false;
        AS3::Instances::DisplayObject* pmcObj = static_cast<AS3::Instances::DisplayObject*>(asObj.GetObject());
        AS3::Value undefVal;
        pmcObj->nameSet(undefVal, root->GetStringManager()->CreateString(instanceName));
        
        // Load initArgs into obj
        if (initArgs)
        {
            for (unsigned i = 0; i < initArgs->GetSize(); i++)
            {
                const MemberValue& mv = (*initArgs)[i];
                AS3::Multiname propname(root->GetAVM()->GetPublicNamespace(),
                    root->GetStringManager()->CreateString(mv.Key));
                AS3::Value propval;
                root->GFxValue2ASValue(mv.mValue, &propval);
                pmcObj->SetProperty(propname, propval).DoNotCheck();
            }
        }

        // Add new display obj to parent
        AS3::AvmDisplayObjContainer* container = AS3::ToAvmDisplayObjContainer(parentDispObj);
        if (depth > (int)container->GetNumChildren())
        {
            LogState* log = root->GetLogState();
            if (log)
                log->LogScriptWarning("DAPI AttachMovie() depth requested (%d) for symbol \"%s\" "
                                      "is too large. Using next highest index (%d) instead.", 
                                      depth, symbolName, container->GetNumChildren());
        }
        unsigned index = (depth < 0 || depth > (int)container->GetNumChildren()) ? container->GetNumChildren() : depth;
        container->AddChildAt(pmcObj->pDispObj, index);

        root->ASValue2GFxValue(asObj, pmc);
        return true;
    }

    LogState* log = root->GetLogState();
    if (log)
        log->LogScriptWarning("attachMovie() failed - export name \"%s\" is not found.", symbolName);
    pAVM->IgnoreException();
    return false;
}

void GFX_Value_ObjectInterface_CLASS::ToString(String* pstr, const GFx::Value& thisVal) const
{
    AS3::MovieRoot*       proot= static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot);

    AS3::Value asVal;
    proot->GFxValue2ASValue(thisVal, &asVal);

    *pstr = String(asVal.AsString().ToCStr());
}


// ***** MovieRoot definitions

namespace AS3 {

// Value conversion for external communication
void MovieRoot::GFxValue2ASValue(const GFx::Value& gfxVal, Value* pdestVal)
{
    
    SF_ASSERT(pdestVal);
    switch(gfxVal.GetType())
    {
    case GFx::Value::VT_Undefined: pdestVal->SetUndefined(); break;
    case GFx::Value::VT_Boolean:   pdestVal->SetBool(gfxVal.GetBool()); break;
    case GFx::Value::VT_Null:      pdestVal->SetNull(); break;
    case GFx::Value::VT_Number:    pdestVal->SetNumber(gfxVal.GetNumber()); break;
    case GFx::Value::VT_String:    
        {
            if (gfxVal.IsManagedValue())
            {
                size_t memberOffset = OFFSETOF(ASStringNode, pData);
                // Must be an assignment of a string from the same movie root
                SF_ASSERT(gfxVal.pObjectInterface->IsSameContext(pMovieImpl->pObjectInterface));
                ASStringNode* strnode = (ASStringNode*)(((UByte*)gfxVal.mValue.pStringManaged) - memberOffset);                
                *pdestVal = strnode;                
            }
            else
                *pdestVal = GetStringManager()->CreateString(gfxVal.GetString());
        }
        break;
    case GFx::Value::VT_StringW:    
        {
            if (gfxVal.IsManagedValue())
            {                
                size_t memberOffset = OFFSETOF(MovieImpl::WideStringStorage, pData);
                // Must be an assignment of a string from the same movie root
                SF_ASSERT(gfxVal.pObjectInterface->IsSameContext(pMovieImpl->pObjectInterface));
                MovieImpl::WideStringStorage* wstr = 
                    (MovieImpl::WideStringStorage*)(((UByte*)gfxVal.mValue.pStringW) - memberOffset);
                ASString str(wstr->pNode);
                *pdestVal = str;
            }
            else 
                *pdestVal = GetStringManager()->CreateString(gfxVal.GetStringW());
        }
        break;
    
    case GFx::Value::VT_Array:
    case GFx::Value::VT_Object:
    case GFx::Value::VT_DisplayObject:
        {            
            // Must be an assignment of an object from the same movie root
            SF_ASSERT(gfxVal.pObjectInterface->IsSameContext(pMovieImpl->pObjectInterface));
            *pdestVal = (Object*)gfxVal.mValue.pData;
        }
        break;    
    default:
        break;
    }    
}

void MovieRoot::ASValue2GFxValue(const Value& value, GFx::Value* pdestVal) const
{
    SF_ASSERT(pdestVal);
    int toType;
    if (pdestVal->GetType() & GFx::Value::VTC_ConvertBit)
        toType = pdestVal->GetType() & (~GFx::Value::VTC_ConvertBit);
    else
    {
        switch(value.GetKind())
        {
        case Value::kUndefined: toType = GFx::Value::VT_Undefined; break;
        case Value::kBoolean:   toType = GFx::Value::VT_Boolean; break;
        case Value::kInt:
        case Value::kUInt:
        case Value::kNumber:    toType = GFx::Value::VT_Number; break;

            // All treated as objects:
        case Value::kObject:
        case Value::kClass:
        case Value::kFunction:
        case Value::kThunkFunction:
        //case Value::kMethodClosure: !PPS: We cannot support closures yet without storing two ptrs
            
                toType = value.GetObject() ? GFx::Value::VT_Object : GFx::Value::VT_Null;
            break;               
            // All other types are treated as strings.
        default:
            toType = GFx::Value::VT_String; break;
        }
        // Special cases
        if (value.IsNull()) toType = GFx::Value::VT_Null;
    }

    // Clear existing ref
    if (pdestVal->IsManagedValue())
        pdestVal->ReleaseManagedValue();

    switch(toType)
    {
    case GFx::Value::VT_Undefined:
    case GFx::Value::VT_Null:
        {
            pdestVal->Type = GFx::Value::ValueType(toType);
        }
        break;
    case GFx::Value::VT_Boolean:
        {
            pdestVal->Type = GFx::Value::ValueType(toType);
            pdestVal->mValue.BValue = value.AsBool();
        }
        break;
    case GFx::Value::VT_Number:
        {
            pdestVal->Type = GFx::Value::ValueType(toType);            
            if (!value.Convert2Number(pdestVal->mValue.NValue))
            {
				pAVM->OutputAndIgnoreException();
                pdestVal->mValue.NValue = NumberUtil::NaN();
            } 
        }
        break;
        
    case GFx::Value::VT_String:    
        {
            ASString str(pAVM->GetStringManager().GetBuiltin(AS3Builtin_empty_));
            if (!value.Convert2String(str, pAVM->GetStringManager()))
                pAVM->OutputAndIgnoreException();

            size_t        memberOffset = GFX_VALUE_OFFSETOF(ASStringNode, pData);                        
            ASStringNode* strNode = str.GetNode();
            pdestVal->Type = GFx::Value::ValueType(GFx::Value::VT_String | GFx::Value::VTC_ManagedBit);
            pdestVal->mValue.pStringManaged = (const char**)(((UByte*)strNode) + memberOffset);
            pdestVal->pObjectInterface = pMovieImpl->pObjectInterface;
			pMovieImpl->pObjectInterface->ObjectAddRef(pdestVal, pdestVal->mValue.pData);
        }
        break;        
    case GFx::Value::VT_StringW:    
        {               
            ASString str(pAVM->GetStringManager().GetBuiltin(AS3Builtin_empty_));
            if (!value.Convert2String(str, pAVM->GetStringManager()))
                pAVM->OutputAndIgnoreException();
            
            size_t memberOffset = GFX_VALUE_OFFSETOF(MovieImpl::WideStringStorage, pData);
            ASStringNode* strNode = str.GetNode();
            unsigned len = str.GetLength() + 1;
            void* pdata = SF_HEAP_ALLOC(GetMovieHeap(), sizeof(MovieImpl::WideStringStorage) - sizeof(UByte) + (sizeof(wchar_t) * len), StatMV_Other_Mem);
            pdestVal->Type = GFx::Value::ValueType(GFx::Value::VT_StringW | GFx::Value::VTC_ManagedBit);
            Ptr<MovieImpl::WideStringStorage> wstr = *::new(pdata) MovieImpl::WideStringStorage(strNode, str.GetSize());
            pdestVal->mValue.pStringW = (const wchar_t*)(((UByte*)wstr.GetPtr()) + memberOffset);
            pdestVal->pObjectInterface = pMovieImpl->pObjectInterface;
			pMovieImpl->pObjectInterface->ObjectAddRef(pdestVal, pdestVal->mValue.pData);
        }
        break;
        
    case GFx::Value::VT_Object:
        {
            GFx::Value::ValueType type = GFx::Value::VT_Object;
            Object* object = value.GetObject();
            
             if (AreDisplayObjectTraits(object->GetTraitsType()))
                type = GFx::Value::VT_DisplayObject;
             else if (object->GetTraitsType() == Traits_Array)
                type = GFx::Value::VT_Array;

             pdestVal->Type = GFx::Value::ValueType(type | GFx::Value::VTC_ManagedBit);
             pdestVal->mValue.pData = object;
             pdestVal->pObjectInterface = pMovieImpl->pObjectInterface;
             pMovieImpl->pObjectInterface->ObjectAddRef(pdestVal, pdestVal->mValue.pData);
        }
        break;
    }
}


void MovieRoot::CreateObjectInterface(MovieImpl* movie)
{
    SF_ASSERT(!movie->pObjectInterface);
    movie->pObjectInterface = 
        SF_HEAP_NEW(GetMovieHeap()) GFX_Value_ObjectInterface_CLASS(movie);
}

void MovieRoot::CreateString(GFx::Value* pvalue, const char* pstring)
{        
    ASString asstr = GetStringManager()->CreateString(pstring);
    ASValue2GFxValue(asstr, pvalue);
}

void MovieRoot::CreateStringW(GFx::Value* pvalue, const wchar_t* pstring)
{
    ASString asstr = GetStringManager()->CreateString(pstring);
    ASValue2GFxValue(asstr, pvalue);
}

void MovieRoot::CreateObject(GFx::Value* pvalue, const char* className,
                             const GFx::Value* pargs, unsigned nargs)
{
    // Setup invoke parameters
    // ** Create storage
    enum { NumValuesOnStack = 10 };
    void* argArrayOnStack[NumValuesOnStack*((sizeof(GFx::Value)+sizeof(void*))/sizeof(void*))];
    Value* pargArray;
    if (nargs > NumValuesOnStack)
        pargArray = (Value*)
        SF_HEAP_AUTO_ALLOC_ID(this, sizeof(Value) * nargs, GFx::StatMV_ActionScript_Mem);
    else
        pargArray = (Value*)argArrayOnStack;
    // ** Convert to AS3::Value
    for (unsigned i = 0, n = nargs; i < n; ++i)
    {
        const GFx::Value& val = pargs[i];
        Value* pdestVal = Scaleform::Construct<Value>(&pargArray[i]);
        GFxValue2ASValue(val, pdestVal);
    }

    Value obj;
    const bool need2execute = pAVM->Construct(className ? className : "Object", obj, nargs, pargArray); 
    if (!pAVM->IsException()) {
        if (need2execute)
            pAVM->ExecuteCode();
    } else {
        printf("Exception in CreateObject(\"%s\"):\n\t", className);
        pAVM->OutputAndIgnoreException();
    }
    ASValue2GFxValue(obj, pvalue);

    // Destruct elements in AS3::Value params array
    for (unsigned i = 0, n = nargs; i < n; ++i)
        pargArray[i].~Value();
    if (nargs > sizeof(argArrayOnStack)/sizeof(argArrayOnStack[0]))
        SF_FREE(pargArray);
}

void MovieRoot::CreateArray(GFx::Value* pvalue)
{
    Value arr(pAVM->MakeArray());

    ASValue2GFxValue(arr, pvalue);
}

void MovieRoot::CreateFunction(GFx::Value* pvalue, GFx::FunctionHandler* pfc, 
                               void* puserData /* = NULL */)
{
    AS3::Value asval(MakePickable(SF_HEAP_NEW(GetMovieHeap()) 
                UserDefinedFunction(pAVM->GetClassTraitsFunction().GetThunkFunctionTraits(), pfc, puserData))
        );
    ASValue2GFxValue(asval, pvalue);
}

} // namespace AS3


// In case someone decides to combine sources
#undef GFX_Value_ObjectInterface_CLASS

}} // Scaleform::GFx
