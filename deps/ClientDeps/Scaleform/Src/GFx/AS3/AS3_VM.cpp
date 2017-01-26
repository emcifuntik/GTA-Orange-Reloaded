/**********************************************************************

Filename    :   AS3_VM.cpp
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

#include "AS3_VM.h"
//#include "../abc/abc_asstring.h"
#include "AS3_AsString.h"
#include "AS3_VMRead.h"
#include "AS3_VTable.h"

// For VM::FormatErrorMessage
#include "../Kernel/SF_MsgFormat.h"

#include "Obj/AS3_Obj_Function.h"
#include "Obj/AS3_Obj_Activation.h"
#include "Obj/AS3_Obj_Catch.h"
#include "Obj/AS3_Obj_Boolean.h"
#include "Obj/AS3_Obj_Number.h"
#include "Obj/AS3_Obj_int.h"
#include "Obj/AS3_Obj_uint.h"
#include "Obj/AS3_Obj_String.h"
#include "Obj/AS3_Obj_Array.h"
#include "Obj/AS3_Obj_Activation.h"
#include "Obj/AS3_Obj_UserDefined.h"
#include "Obj/AS3_Obj_Namespace.h"
#include "Obj/AS3_Obj_QName.h"
#include "Obj/AS3_Obj_Error.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_int.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_uint.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_double.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_String.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_object.h"
#include "Obj/AS3_Obj_Global.h"


namespace Scaleform { namespace GFx { namespace AS3 
{


///////////////////////////////////////////////////////////////////////////

// ***** VMAbcFile Implementation

VMAbcFile::VMAbcFile(VM& vm, const Abc::File& file)
    : GASRefCountBase(&vm.GetGC()), VMRef(vm), File(file)
{
    // String with index zero is empty.
    IntStrings.PushBack(vm.GetStringManager().GetEmptyStringNode());
}

VMAbcFile::~VMAbcFile()
{
    // Moved to source file due to the use of undeclared Namespace
    // in ArrayLH.
}

void VMAbcFile::ForEachChild_GC(GcOp op) const
{
    UPInt i = 0;

    IntNamespaces.ForEachChild_GC(op);

    for (i = 0; i< IntNamespaceSets.GetSize(); ++i)
        if (IntNamespaceSets[i])
            AS3::ForEachChild_GC<NamespaceSet, Mem_Stat>(IntNamespaceSets[i], op);
}

Instances::Namespace& VMAbcFile::GetInternedNamespace(const Abc::Multiname& mn)
{
    SPtr<Instances::Namespace>* pNs = IntNamespaces.Get(mn);

    if (pNs == NULL)
    {
        const Abc::ConstPool& cp = GetAbcFile().GetConstPool();
        const Abc::NamespaceInfo& info = mn.GetNamespace(cp);
        ASString URI = GetVM().GetStringManager().CreateString(info.GetNameURI());

        if (mn.GetNextMultinameIndex() >= 0)
        {
            const Abc::Multiname& next_mn = mn.GetNextMultiname(cp);
            const Abc::NamespaceInfo& next_info = next_mn.GetNamespace(cp);

            URI += "$";
            URI += GetVM().GetStringManager().CreateString(next_info.GetNameURI());
        }

        Pickable<Instances::Namespace> ns_ptr = GetVM().MakeInternedNamespace(info.GetKind(), URI);

        IntNamespaces.Add(mn, ns_ptr);
        pNs = IntNamespaces.Get(mn);
    }

    return **pNs;
}

Instances::Namespace& VMAbcFile::GetInternedNamespace(UPInt nsIndex)
{
    SPtr<Instances::Namespace>* pNs = IntNamespaces.Get(nsIndex);

    if (pNs == NULL)
    {
        const Abc::NamespaceInfo& info = GetAbcFile().GetConstPool().GetNamespace(nsIndex);
        Pickable<Instances::Namespace> ns_ptr = GetVM().MakeInternedNamespace(info.GetKind(), info.GetNameURI());

        IntNamespaces.Add(nsIndex, ns_ptr);
        pNs = IntNamespaces.Get(nsIndex);
    }

    return **pNs;
}

NamespaceSet& VMAbcFile::GetInternedNamespaceSet(UPInt nsSetIndex)
{    
    if (nsSetIndex >= IntNamespaceSets.GetSize())
        IntNamespaceSets.Resize(nsSetIndex + 1);
    
    // Initialize.
    if (!IntNamespaceSets[nsSetIndex])
    {
        Pickable<NamespaceSet> nss(SF_HEAP_NEW(GetVM().GetMemoryHeap()) NamespaceSet(VMRef));
        IntNamespaceSets[nsSetIndex] = nss;
        const Abc::NamespaceSetInfo& info = GetAbcFile().GetConstPool().GetNamespaceSet(nsSetIndex);
        
        // Fill it up with data.
        for(UPInt i = 0; i < info.GetSize(); ++i)
        {
            int nsi = info.GetNamespaceInd(i);
            nss->Add(GetInternedNamespace(nsi));
        }
    }
    
    return *IntNamespaceSets[nsSetIndex];
}

ASString VMAbcFile::GetInternedString(UPInt strIndex)
{
    if (strIndex >= IntStrings.GetSize())
        IntStrings.Resize(strIndex + 1);

    if (IntStrings[strIndex].Get() == NULL)
    {
        IntStrings[strIndex].SetPtr(GetVM().GetStringManager().CreateString(
            File.GetConstPool().GetString(AbsoluteIndex(strIndex))
            ).GetNode());
    }

    return ASString(IntStrings[strIndex]);
}

///////////////////////////////////////////////////////////////////////////
// int Read(AS3::VM& vm, Multiname& obj)
// {
//     switch (obj.GetKind())
//     {
//     case Abc::MN_QName:
//     case Abc::MN_QNameA:
//         // QName is not read from stack; both name and namespase known at compile-time.
//         break;
//     case Abc::MN_RTQName:
//     case Abc::MN_RTQNameA:
//         // Name is known at compile time, Namespace is resolved at runtime.
//         obj.SetRTNamespace(
//             const_cast<Instances::Namespace&>(vm.OpStack.Pop().AsNamespace()));
//         return 1;
//     case Abc::MN_RTQNameL:
//     case Abc::MN_RTQNameLA:
//         // Both name and namespace are resolved at runtime.
//         obj.SetRTName(vm.OpStack.Pop());
//         obj.SetRTNamespace(
//             const_cast<Instances::Namespace&>(vm.OpStack.Pop().AsNamespace()));
//         return 2;
//     case Abc::MN_Multiname:
//     case Abc::MN_MultinameA:
//         // MInfo is not supposed to be read from stack.
//         break;
//     case Abc::MN_MultinameL:
//     case Abc::MN_MultinameLA:
//         // Namespace set is known at compile time, Name is resolved at runtime.        
//         // name_ind is a string, and we will get an int or an uint as an index instead;
//         obj.SetRTName(vm.OpStack.Pop());         
//         return 1;
//     case Abc::MN_Typename:
//         // Recursive!!!?
//         return Read(vm, obj);
//     default:
//         SF_ASSERT(false);
//         break;
//     }
//     
//     return 0;
// }

///////////////////////////////////////////////////////////////////////////
CheckResult GetArrayInd(ASStringNode* sn, UInt32& ind)
{
    const unsigned ssize = sn->Size;

    if (ssize > 0)
    {
        const char* sdata = sn->pData;
        if (isdigit(sdata[0]) && sdata[0] != '0')
        {
            for (unsigned i = 1; i < ssize; ++i)
            {
                if (!isdigit(sdata[i]))
                    return false;
            }

            char* end;
            Value::Number num = strtod(sdata, &end);

            if (num <= SF_MAX_UINT32)
            {
                ind = (UInt32)num;
                return true;
            }
        } 
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
CheckResult GetArrayInd(const Multiname& prop_name, UInt32& ind)
{
    bool result = false;
    const Value& name = prop_name.GetName();

    if (name.IsString())
        result = GetArrayInd(name.GetStringNode(), ind);
    else if (name.IsNumeric())
        result = name.Convert2UInt32(ind);

    return result;
}

///////////////////////////////////////////////////////////////////////////
CheckResult GetVectorInd(ASStringNode* sn, UInt32& ind)
{
    const unsigned ssize = sn->Size;

    if (ssize > 0)
    {
        const char* sdata = sn->pData;
        if (isdigit(sdata[0]) && sdata[0] != '0')
        {
            for (unsigned i = 1; i < ssize; ++i)
            {
                if (!isdigit(sdata[i]))
                {
                    if (sdata[i] == '.')
                    {
                        for (unsigned j = i + 1; j < ssize; ++j)
                        {
                            if (sdata[j] != '0')
                                return false;
                        }

                        // Everything is fine.
                        break;
                    }

                    return false;
                }
            }

            char* end;
            Value::Number num = strtod(sdata, &end);

            if (num <= SF_MAX_UINT32)
            {
                ind = (UInt32)num;
                return true;
            }
        } 
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
CheckResult GetVectorInd(const Multiname& prop_name, UInt32& ind)
{
    bool result = false;
    const Value& name = prop_name.GetName();

    /* Old version
    if (name.IsString())
        result = GetArrayInd(name.GetStringNode(), ind);
    else if (name.IsNumeric())
        result = name.Convert2UInt32(ind);
    */

    switch (name.GetKind())
    {
    case Value::kString:
        result = GetVectorInd(name.GetStringNode(), ind);
        break;
    case Value::kUInt:
        ind = name.AsUInt();
        result = true;
        break;
    case Value::kInt:
        if (name.AsInt() >= 0)
        {
            ind = static_cast<UInt32>(name.AsInt());
            result = true;
        }
        break;
    case Value::kNumber:
        {
            Value::Number n;
            if (name.AsNumber() >= 0 && ::modf(name.AsNumber(), &n) == 0.0 && name.AsNumber() <= SF_MAX_UINT32)
            {
                ind = static_cast<UInt32>(name.AsNumber());
                result = true;
            }
        }
        break;
    default:
        break;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
CallFrame::CallFrame(MemoryHeap* heap)
: DiscardResult(false)
, AdjustedStack(false)
, ACopy(false)
DEBUG_ARG(RegisteredFunction(false))
, OriginalStackSize(0)
, RegBaseInd(0)
, ScopeStackBaseInd(0)
, Offset(0)
DEBUG_ARG(CurrFileInd(0))
DEBUG_ARG(CurrLineNumber(0))
DEBUG_ARG(StartTicks(0))
, pHeap(heap)
, pFile(NULL)
, pMBI(NULL)
, pHT(NULL)
, pSavedScope(NULL)
, pRegisterFile(NULL)
, OriginationTraits(NULL)
, pScopeStack(NULL)
{
}

CallFrame::CallFrame(const CallFrame& other)
: DiscardResult(other.DiscardResult)
, AdjustedStack(other.AdjustedStack)
, ACopy(true)
DEBUG_ARG(RegisteredFunction(other.RegisteredFunction))
, OriginalStackSize(other.OriginalStackSize)
, RegBaseInd(other.RegBaseInd)
, ScopeStackBaseInd(other.ScopeStackBaseInd)
, Offset(other.Offset)
DEBUG_ARG(CurrFileInd(0))
DEBUG_ARG(CurrLineNumber(0))
DEBUG_ARG(StartTicks(other.StartTicks))
, pHeap(other.pHeap)
, pFile(other.pFile)
, pMBI(other.pMBI)
, pHT(other.pHT)
, pSavedScope(other.pSavedScope)
DEBUG_ARG(Name(other.Name))
, pRegisterFile(other.pRegisterFile)
, OriginationTraits(other.OriginationTraits)
, pScopeStack(other.pScopeStack)
, Invoker(other.Invoker)
{
}

CallFrame::~CallFrame()
{
    if (pRegisterFile && pScopeStack && ACopy)
    {
        pRegisterFile->Resize(pRegisterFile->GetSize() - pMBI->GetMaxLocalRegisterIndex());
        ClearScopeStack();
    }
}

CallFrame& CallFrame::operator =(const CallFrame& other)
{
    if (this != &other)
    {
        *const_cast<bool*>(&DiscardResult) = other.DiscardResult;
        AdjustedStack = other.AdjustedStack;
        ACopy = true;
        DEBUG_CODE(RegisteredFunction = other.RegisteredFunction;)
        OriginalStackSize = other.OriginalStackSize;
        RegBaseInd = other.RegBaseInd;
        ScopeStackBaseInd = other.ScopeStackBaseInd;
        Offset = other.Offset;
        DEBUG_CODE(CurrFileInd = other.CurrFileInd;)
        DEBUG_CODE(CurrLineNumber = other.CurrLineNumber;)
        DEBUG_CODE(StartTicks = other.StartTicks;)
        pHeap = other.pHeap;
        pFile = other.pFile;
        pMBI = other.pMBI;
        pHT = other.pHT;
        pSavedScope = other.pSavedScope;
        DEBUG_CODE(Name = other.Name;)
        pRegisterFile = other.pRegisterFile;
        OriginationTraits = other.OriginationTraits;
        pScopeStack = other.pScopeStack;
        Invoker = other.Invoker;
    }

    return *this;
}

void CallFrame::SetupRegisters(const Abc::MethodInfo& mi, const Value& _this, unsigned argc, const Value* argv)
{
    // Register 0 holds the “this” object. This value is never null.
    int reg = 0;
    SF_ASSERT(!_this.IsNull());
    SetRegister(reg++, _this);
    
    const UPInt param_count = mi.GetParamCount();
    const UPInt first_opt_param_num = param_count - mi.GetOptionalParamCount();
    
    // Registers 1 through method_info.param_count holds parameter values coerced to the declared types
    // of the parameters. If fewer than method_body_info.local_count values are supplied to the call then
    // the remaining values are either the values provided by default value declarations (optional arguments)
    // or the value undefined.
    
    for(unsigned i = 0; i < param_count; ++i)
    {
        if (i < argc)
        {
#if 0
            // This simplified version breaks "as3/Definitions/Classes/Override/ExtDefaultOverRidePublic.abc"
            // This is the only problem so far.

            // TBD: Replace runtime coerce with Trace-time coerce.
            SetRegister(reg++, argv[i]);
#else
            // Coerce to the argument's type ...

            // Resolve multiname ...
            const Abc::Multiname& mn = mi.GetParamType(GetFile().GetConstPool(), i);
            const ClassTraits::Traits* to = GetVM().Resolve2ClassTraits(GetFile(), mn);

            if (to)
            {
                Value result;
                
                if (to->Coerce(argv[i], result))
                {
                    // Store value ...
                    SetRegisterPickUnsafe(reg++, result);
                } else
                {
                    // A TypeError is thrown if value cannot be coerced to the specified type.
                    //SF_ASSERT2(false, "Value " + AsString(args[i], GetVM()) + " cannot be coerced to " + AsString(Value(to), GetVM()));
                    return GetVM().ThrowTypeError(VM::eNotImplementedYet DEBUG_ARG("Value cannot be coerced to the specified type"));
                }
            } else
            {
                //SF_ASSERT2(false, "Couldn't resolve multiname " + AsString(mn));
                return GetVM().ThrowReferenceError(VM::eNotImplementedYet DEBUG_ARG("Couldn't resolve multiname."));
            }
#endif
        } else
        {
            // No more arguments from stack ...
            if(mi.HasOptionalParams() && i >= first_opt_param_num)
            {
                SetRegister(reg++, GetDetailValue(GetFile(), mi.GetOptionalParam(i - first_opt_param_num)));
            } else
            {
                SetRegister(reg++, Value::GetUndefined());
            }
        }
    }

    // TBD: Consider mi.HasParamNames() ?
    
    if (mi.NeedRest())
    {
        // If NEED_REST is set in method_info.flags, the method_info.param_count+1 register is set up to
        // reference an array that holds the superfluous arguments.

        Pickable<Instances::Array> arr = GetVM().MakeArray();

        if (argc > param_count)
            arr->Append((unsigned)(argc - param_count), argv + param_count);

        SetRegister(reg++, arr);
    } else if (mi.NeedArguments())
    {
        // If NEED_ARGUMENTS is set in method_info.flags, the method_info.param_count+1 register is set up
        // to reference an “arguments” object that holds all the actual arguments: see ECMA-262 for more
        // information. (The AVM2 is not strictly compatible with ECMA-262; it creates an Array object for
        // the “arguments” object, whereas ECMA-262 requires a plain Object.)

        // "arguments" is a regular Array. There is no an explicit "arguments" class.
        Pickable<Instances::Array> arr = GetVM().MakeArray();

        arr->Append(argc, argv);
        arr->AddDynamicSlotValuePair(
            GetVM().GetStringManager().CreateConstString("callee"), 
            // Make a method closure by storing "this".
            GetVM().GetClassFunction().MakeInstance2(Invoker, _this)
            //Value(_this.GetObject(), &Invoker.AsFunction())
            );

        SetRegister(reg++, arr);
    }
}

///////////////////////////////////////////////////////////////////////////
VM::VM(FlashUI& _ui, FileLoader& loader, StringManager& sm, ASRefCountCollector& gc)
: StringManagerRef(sm)
, GC(gc)
, UI(_ui)
, Loader(loader)
, InInitializer(0)
, MHeap(Memory::GetHeapByAddress(&GetSelf()))
, CallStack(GetMemoryHeap())
, ScopeStack(GetMemoryHeap())
, HandleException(false)
, GlobalObjects(GetMemoryHeap())

, Initialized(false)
#ifdef SF_AMP_SERVER
, ActiveLineTimestamp(0)
#endif

, PublicNamespace(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) Instances::Namespace(GetSelf(), Abc::NS_Public, "")))
, AS3Namespace(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) Instances::Namespace(GetSelf(), Abc::NS_Public, NS_AS3)))

, TraitsClassClass(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::ClassClass(GetSelf())))
, TraitsObject(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::Object(GetSelf())))
, TraitsNamespace(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::Namespace(GetSelf(), GetClassTraitsObject())))
, TraitsFunction(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::Function(GetSelf(), GetClassTraitsObject())))
, TraitsMethodClosure(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::MethodClosure(GetSelf(), GetClassTraitsObject())))
, TraitsBoolean(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::Boolean(GetSelf(), GetClassTraitsObject())))
, TraitsNumber(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::Number(GetSelf(), GetClassTraitsObject())))
, TraitsInt(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::int_(GetSelf(), GetClassTraitsObject())))
, TraitsUint(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::uint(GetSelf(), GetClassTraitsObject())))
, TraitsString(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::String(GetSelf(), GetClassTraitsObject())))
, TraitsArray(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::Array(GetSelf(), GetClassTraitsObject())))
, TraitsActivation(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::Activation(GetSelf())))
, TraitsCatch(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::Catch(GetSelf(), GetClassTraitsObject())))
, TraitsVector(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::Vector(GetSelf(), GetClassTraitsObject())))
, TraitsVector_int(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::Vector_int(GetSelf(), GetClassTraitsVector())))
, TraitsVector_uint(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::Vector_uint(GetSelf(), GetClassTraitsVector())))
, TraitsVector_Number(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::Vector_double(GetSelf(), GetClassTraitsVector())))
, TraitsVector_String(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::Vector_String(GetSelf(), GetClassTraitsVector())))

, TraitsNull(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) InstanceTraits::Anonimous(GetSelf())))
, TraitsVoid(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) InstanceTraits::Anonimous(GetSelf())))

, NoFunctionTraits(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) InstanceTraits::Function(GetSelf(), GetClassFunction())))

, NoActivationTraits(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) InstanceTraits::Activation(GetSelf(), GetClassActivation())))

, TraitaGlobalObject(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) InstanceTraits::GlobalObject(GetSelf(), GetClassObject())))
, GlobalObject(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) Instances::GlobalObjectCPP(GetSelf(), *TraitaGlobalObject)))
, GlobalObjectValue(GlobalObject)
{
    RegisterClassTraits(*TraitsClassClass);
    RegisterClassTraits(*TraitsObject);
    RegisterClassTraits(*TraitsNamespace);
    RegisterClassTraits(*TraitsFunction);
    RegisterClassTraits(*TraitsMethodClosure);
    RegisterClassTraits(*TraitsBoolean);
    RegisterClassTraits(*TraitsNumber);
    RegisterClassTraits(*TraitsInt);
    RegisterClassTraits(*TraitsUint);
    RegisterClassTraits(*TraitsString);
    RegisterClassTraits(*TraitsArray);
    //RegisterClassTraits(*TraitsActivation);
    //RegisterClassTraits(*TraitsCatch);
    RegisterClassTraitsVector(*TraitsVector);
    RegisterClassTraitsVector(*TraitsVector_int);
    RegisterClassTraitsVector(*TraitsVector_uint);
    RegisterClassTraitsVector(*TraitsVector_Number);
    RegisterClassTraitsVector(*TraitsVector_String);

    Initialized = true;

    GlobalObjects.PushBack(Value(GlobalObject.Get()));

    const UPInt n = Loader.GetSize();
    for (UPInt i = 0; i < n; ++i)
    {
        const Abc::File& file = Loader.GetFile(i);

        LoadFile(file, i == (n - 1)).DoNotCheck();
    }

    // Delayed initialization to prevent circular dependencies.
    {
        // Set up traits of the public namespace, which was created uninitialized.
        // !!! For some reason that causes memory leaks.
        //PublicNamespace->pTraits = &ClassNamespace->GetInstanceTraits();
        //AS3Namespace->pTraits = &ClassNamespace->GetInstanceTraits();

        // ClassClass delayed initialization.
        {
            TraitsClassClass->RegisterSlots();

            // Prototype of ClassClass cannot be set in constructor because
            // ClassObject is not initialized at this time.
            GetClassClass().SetupPrototype();
        }

        TraitsObject->RegisterSlots();
        TraitsNamespace->RegisterSlots();
    }

    // Register ClassVector with the Global Object.
    UPInt index = 0;
    GetGlobalObjectCPP().AddFixedSlot(GetClassVector(), MakeInternedNamespace(Abc::NS_Public, NS_Vector), index);
}

VM::~VM()
{
    // This situation should be handled by the garbage collector.
    // Because of dependencies.
    //ClassClass->ReleasePrototype();
}

void VM::RegisterClassTraits(ClassTraits::Traits& tr)
{
    const Instances::Namespace& ns = GetPublicNamespace();

    RegisterClassTraits(tr.GetInstanceTraits().GetClass().GetName(), ns, tr);
}

bool IsScaleformGFx(const Instances::Namespace& ns)
{
    static const StringDataPtr scaleform_gfx("scaleform.gfx");
    const ASString& uri = ns.GetUri();
    const StringDataPtr str(uri.ToCStr(), uri.GetSize());

    return (str.GetSize() >= scaleform_gfx.GetSize() && SFstrncmp(str.ToCStr(), scaleform_gfx.ToCStr(), scaleform_gfx.GetSize()) == 0);
}

bool VM::RegisterUserDefinedClassTraits(VMAbcFile& file)
{
    const Abc::ClassTable& classes = file.GetClasses();
    UPInt numOfClasses = 0;

    // Filter out scripts, which
    // 1) initialize classes, which are not registered;
    // 2) duplicate identical scripts from other files.

    // Register uninitialized traits,
    for (UPInt i = 0; i < classes.GetSize(); ++i)
    {
        const Abc::ClassInfo& class_info = classes.Get(i);
        const Abc::Multiname& name = class_info.GetName(file.GetConstPool());
        const ASString str_name = file.GetInternedString(name.GetNameInd());
        const Instances::Namespace& ns = file.GetInternedNamespace(name.GetNamespaceInd());

        // check for *scaleform extension* packages.
        if (IsScaleformGFx(ns))
            // Skip it.
            continue;

        if (Resolve2ClassTraits(str_name, ns) != NULL)
            // This class is already loaded from another file. Skip it.
            continue;

        Pickable<ClassTraits::UserDefined> ud(SF_HEAP_NEW(GetMemoryHeap()) ClassTraits::UserDefined(file, GetSelf(), class_info));
        ClassTraitsSet.Add(str_name, ns, TClassTraitsCache::ValueType(ud));
        ++numOfClasses;
    }

    if (numOfClasses == 0 && classes.GetSize() != 0)
        return false;

    bool uninitialize = false;

    // Initialize uninitialized traits,
    for (UPInt i = 0; i < classes.GetSize(); ++i)
    {
        const Abc::ClassInfo& class_info = classes.Get(i);
        const Abc::Multiname& name = class_info.GetName(file.GetConstPool());
        const ASString str_name = file.GetInternedString(name.GetNameInd());
        const Instances::Namespace& ns = file.GetInternedNamespace(name.GetNamespaceInd());

        if (IsScaleformGFx(ns))
            // Skip it.
            continue;

        /* Another way of doing the same thing.
        const ClassTraits::UserDefined* ud = static_cast<const ClassTraits::UserDefined*>(Resolve2ClassTraits(str_name, ns));

        // Filter out classes, which are defined in other files. This happens.
        if (ud && (&ud->GetFile() == &file))
            ud->Initialize();
        */

        ClassTraits::UserDefined& ud = GetUserDefinedTraits(file, class_info);
        // Filter out classes, which are defined in other files. This happens.
        if (&ud.GetFile() != &file)
            continue;

        ud.Initialize();

        // Initialize() can throw exceptions.
        if (IsException())
        {
            OutputAndIgnoreException();

            // Delete ClassTraits.
            ClassTraitsSet.Remove(str_name, ns);
            --numOfClasses;
            uninitialize = true;
            break;
        }
    }

    // Unload whole file.
    if (uninitialize)
    {
        // Remove traits, which may be not completely initialized because of an exception,
        for (UPInt i = 0; i < classes.GetSize(); ++i)
        {
            const Abc::ClassInfo& class_info = classes.Get(i);
            const Abc::Multiname& name = class_info.GetName(file.GetConstPool());
            const ASString str_name = file.GetInternedString(name.GetNameInd());
            const Instances::Namespace& ns = file.GetInternedNamespace(name.GetNamespaceInd());

            // check for *scaleform extension* packages.
            if (IsScaleformGFx(ns))
                // Skip it.
                continue;

            const ClassTraits::UserDefined* ctr = static_cast<const ClassTraits::UserDefined*>(Resolve2ClassTraits(str_name, ns));

            if (ctr == NULL)
                continue;

            // Filter out classes, which are defined in other files. This happens.
            if (&ctr->GetFile() != &file)
                continue;

            ClassTraitsSet.Remove(str_name, ns);
        }

        return false;
    }

    return (classes.GetSize() == 0 || (classes.GetSize() != 0 && numOfClasses != 0));
}

void VM::InitUserDefinedClassTraits(Instances::GlobalObjectScript& gos)
{
    using namespace Abc;

    const Abc::ScriptInfo& si = gos.GetScript();
    VMAbcFile& file = gos.GetFile();
    const TraitTable& tt = file.GetTraits();

    for (UPInt i = 0; i < si.GetTraitsCount(); ++i)
    {
        const TraitInfo& ti = si.GetTraitInfo(tt, AbsoluteIndex(i));

        if (ti.GetType() != TraitInfo::tClass)
            continue;

        const Abc::Multiname& name = ti.GetTypeName(file.GetAbcFile());
        const Instances::Namespace& ns = file.GetInternedNamespace(name.GetNamespaceInd());
        ASString str_name = file.GetInternedString(name.GetNameInd());

        // Skip Scaleform classes.
        if (IsScaleformGFx(ns))
            continue;

        const ClassTraits::UserDefined* cud = static_cast<const ClassTraits::UserDefined*>(Resolve2ClassTraits(str_name, ns));

        // Filter out classes, which are defined in other files. This happens.
        if (cud && (&cud->GetFile() == &file))
        {
            InstanceTraits::UserDefined& iud = static_cast<InstanceTraits::UserDefined&>(cud->GetInstanceTraits());

            if (!iud.HasScriptSetUp())
                iud.SetScript(gos);
        }
    }
}

void VM::RegisterClassTraitsVector(ClassTraits::Traits& tr)
{
    const Instances::Namespace& ns = GetInternedNamespace(Abc::NS_Public, NS_Vector);

    RegisterClassTraits(tr.GetInstanceTraits().GetClass().GetName(), ns, tr);
}

PropRef VM::FindScopePropertyByName(const UPInt baseSSInd, const ScopeStackType& scope_stack, const char* name)
{
    const Multiname mn(
        GetPublicNamespace(),
        // We could use CreateConstString() here.
        GetStringManager().CreateString(name)
        );

    return FindScopeProperty(*this, baseSSInd, scope_stack, mn);
}

PropRef VM::FindValuePropertyByName(const Value& value, const char* name)
{
    const Multiname mn(
        GetPublicNamespace(),
        // We could use CreateConstString() here.
        GetStringManager().CreateString(name)
        );

    /* For maniacs
    SF_ASSERT(!value.IsNullOrUndefined());
    */
    return FindObjProperty(*this, value, mn);
}

Pickable<Instances::Object> VM::MakeObject() const
{
    return static_cast<InstanceTraits::Object&>(GetClassTraitsObject().GetInstanceTraits()).MakeInstance();
}

Pickable<Instances::Array> VM::MakeArray() const
{
    return static_cast<InstanceTraits::Array&>(GetClassTraitsArray().GetInstanceTraits()).MakeInstance();
}

ASString VM::AsString(const Value& value)
{
    StringManager& sm = GetStringManager();
    ASString result = sm.CreateEmptyString();

    if (!value.Convert2String(result, sm))
    {
        // Exception.
        // Do nothing about it.
        ;
    }
    
    return result;
}
    
const ClassTraits::Traits* VM::GetRegisteredClassTraits(const Multiname& mn)
{
    const ClassTraits::Traits* result = NULL;

    if (mn.IsAnyType())
        // Code below worked fine.
        // But using GetClassTraitsObject() in Resolve2ClassTraits() causes several dozen of tests to fail.
        //result = &GetClassTraitsObject();
        result = &GetClassTraitsClassClass();
    else
    {
        SPtr<ClassTraits::Traits>* pptraits = ClassTraitsSet.Get(mn);
        result = pptraits ? *pptraits : NULL;
    }

    return result;
}

const ClassTraits::Traits* VM::Resolve2ClassTraits(const ASString& name, const Instances::Namespace& ns)
{
    const ClassTraits::Traits* result = GetRegisteredClassTraits(name, ns);

    if (result == NULL)
    {
        Pickable<ClassTraits::Traits> r = GetGlobalObjectCPP().MakeClassTraits(name, ns);
        result = r.GetPtr();

        if (result)
            ClassTraitsSet.Add(name, ns, r);
    }

    return result;
}

const ClassTraits::Traits* VM::Resolve2ClassTraits(const Multiname& mn)
{
    const ClassTraits::Traits* result = GetRegisteredClassTraits(mn);

    if (result == NULL)
    {
        StringManager& sm = GetStringManager();
        ASString name = sm.CreateEmptyString();

        if (!mn.GetName().Convert2String(name, sm))
            // Error
            return result;

        Pickable<ClassTraits::Traits> r = GetGlobalObjectCPP().MakeClassTraits(name, mn.GetNamespace());
        result = r.GetPtr();

        if (result)
            ClassTraitsSet.Add(name, mn.GetNamespace(), r);
    }

    return result;
}

const ClassTraits::Traits* VM::Resolve2ClassTraits(VMAbcFile& file, const Abc::Multiname& mn)
{
    if (&mn == &file.GetConstPool().GetAnyType())
    {
        // !!! ATTENSION !!!
        // This is a fundamental problem.
        // The major difference between Class and Object is that Class leaves
        // *undefined* unchanged, in opposite, Object will convert *undefined*
        // to *null*.

        // Object can represent any type.
        // Using code below will cause several dozen of tests to fail.
        //return &GetClassTraitsObject();
        return &GetClassTraitsClassClass();
    }

    /* DO NOT delete this code.
    Commented out because of the performance reason.
    */
    const ASString name = file.GetInternedString(mn.GetNameInd());

    if (name == GetStringManager().GetBuiltin(AS3Builtin_void))
        // Should this be Object?
        return &GetClassTraitsObject();

    SF_ASSERT(mn.IsQName());
    const Instances::Namespace& ns = file.GetInternedNamespace(mn.GetNamespaceInd());
    return Resolve2ClassTraits(name, ns);
}

const Class& VM::GetClass(const Value& v) const
{
    switch(v.GetKind())
    {
    case Value::kBoolean:
        return GetClassBoolean();
    case Value::kInt:
        return GetClassInt();
    case Value::kUInt:
        return GetClassUInt();
    case Value::kNumber:
        return GetClassNumber();
    case Value::kString:
        return GetClassString();
    case Value::kClass:
        return GetClassClass();
    case Value::kNamespace:
        return GetClassNamespace();
//     case Value::kObject:
//         if (v.GetObject())
//             return v.GetObject()->GetClass();
// 
//         break;
    case Value::kThunk:
    case Value::kThunkFunction:
    case Value::kFunction:
    case Value::kMethodInd:
    case Value::kMethodClosure:
    case Value::kThunkClosure:
        // This should work in case of null.
        return GetClassFunction();
    default:
//         SF_ASSERT2(false, "Cannot get class of " + AsString(v, GetStrings()) + " value.");
//         SF_ASSERT(false);
        break;
    }
    
    if (v.GetObject())
        return v.GetObject()->GetClass();
    else
        // Return Classes::Object for null.
        return GetClassObject();
}

bool VM::IsOfType(const Value& v, const char* type_name)
{
    const Multiname mn(*this, type_name);

    const ClassTraits::Traits* ctr = Resolve2ClassTraits(mn);
    return (ctr && IsOfType(v, *ctr));
}

inline
Value::Number GetFractionalPortion(Value::Number v)
{
    double r; // We cannot use Value::Number here. 

    return modf(v, &r);
}

bool VM::IsOfType(const Value& v, const ClassTraits::Traits& ctr) const
{
    switch (v.GetKind())
    {
    case Value::kBoolean:
        // Boolean is final.
        return IsBoolean(ctr) || IsObject(ctr);
    case Value::kInt:
        if (IsUint(ctr))
            return v.AsInt() >= 0; // It is not correct to use SF_MIN_UINT32 here.
        else if (IsNumber(ctr))
            return true;

        // int is final.
        return IsInt(ctr) || IsObject(ctr);
    case Value::kUInt:
        if (IsInt(ctr))
            return v.AsUInt() <= SF_MAX_SINT32;
        else if (IsNumber(ctr))
            return true;

        // uint is final.
        return IsUint(ctr) || IsObject(ctr);
    case Value::kNumber:
        {
            double r; // We cannot use Value::Number here.

            if (modf(v.AsNumber(), &r) == 0)
            {
                if (IsUint(ctr))
                    return r >= SF_MIN_UINT32 && r <= SF_MAX_UINT32;
                else if (IsInt(ctr))
                    return r >= SF_MIN_SINT32 && r <= SF_MAX_SINT32;
            }
        }

        // Number is final.
        return IsNumber(ctr) || IsObject(ctr);
    case Value::kString:
        // String is final.
        return IsString(ctr) || IsObject(ctr);

    case Value::kObject:
        if (v.IsNull())
            return false;
        // fall through.    
    case Value::kFunction:
    case Value::kNamespace:
    case Value::kThunkFunction:        
        return GetClassTraits(v).IsOfType(ctr);
    case Value::kClass:
        if (&ctr == &GetClassTraitsClassClass())
            // Special case. Each class is of type class.
            return true;
        else
            return GetClassTraits(v).IsOfType(ctr);
    default:
        break;
    }

    return false;
}

int VM::OnException(SInt32 cp, CallFrame& cf)
{
    int position = -1;
    const Abc::MethodBodyInfo& mbi = cf.GetMethodBodyInfo();
    const Abc::MethodBodyInfo::Exception& e = mbi.GetException();
    UPInt handler_num = 0;

    while (e.FindExceptionInfo(cp, handler_num))
    {
        const Abc::MethodBodyInfo::ExceptionInfo& ei = e.Get(handler_num);
        const int et_ind = ei.GetExceptionTypeInd();
        
        if (et_ind == 0)
        {
            // This handler can handle any type of exceptions ...
            
            // The value and scope stacks in the handling method are cleared before the handler is entered.
            // If a handler is found then the stack is cleared, the exception object is pushed onto the stack.
            cf.ClearOpStack();
            OpStack.PushBack(ExceptionObj);
            // Clean *exception* value ...
            ExceptionObj.Clean();
            cf.ClearScopeStack();
            
            position = ei.GetTargetPos();
            break;
        } else
        {
            // Undefined ExceptionObj should be ignored.
            if (!ExceptionObj.IsUndefined())
            {
                //const String& en = GetConstPool().GetString(Abc::StringID(et_ind));
                const ClassTraits::Traits& exc_ctr = GetClassTraits(ExceptionObj);

                HandleException = false;
                const ClassTraits::Traits* handler_tr = Resolve2ClassTraits(cf.GetFile(), cf.GetFile().GetConstPool().GetMultiname(et_ind));
                if (IsException())
                {
                    ; // Oops. Exception during exception handling.
                }
                HandleException = true;

                if (handler_tr && exc_ctr.IsOfType(*handler_tr))
                {
                    // This handler can handle this type of exceptions ...

                    // The value and scope stacks in the handling method are cleared before the handler is entered.
                    // If a handler is found then the stack is cleared, the exception object is pushed onto the stack.
                    cf.ClearOpStack();
                    OpStack.PushBack(ExceptionObj);
                    // Clean *exception* value ...
                    ExceptionObj.Clean();
                    cf.ClearScopeStack();

                    position = ei.GetTargetPos();
                    break;
                }
            }
        }

        ++handler_num;
    }
    
    HandleException = (position < 0);
        
    return position;
}

Classes::Error& VM::GetClassError() const
{
    return GlobalObject->GetClassError();
}

void VM::exec_typeof() 
{
    SH1<0> stack(OpStack);
    const Value::KindType kind = stack._1.GetKind();        
    AS3::BuiltinType type_name;

    switch(kind)
    {
    case Value::kUndefined:
        type_name = AS3Builtin_undefined;
        break;
    case Value::kBoolean:
        type_name = AS3Builtin_boolean;
        break;
    case Value::kInt:
    case Value::kUInt:
    case Value::kNumber:
        type_name = AS3Builtin_number;
        break;
    case Value::kString:
        type_name = AS3Builtin_string;
        break;
    case Value::kFunction:
    case Value::kThunk:
    case Value::kThunkFunction:
    case Value::kMethodClosure:
    case Value::kThunkClosure:
        type_name = AS3Builtin_function;
        break;
    case Value::kObject:
        // Null is also the object type.
        type_name = AS3Builtin_object;
        break;
    default:
        type_name = AS3Builtin_unknown;
        break;
    }

    stack._1 = Value(GetStringManager().GetBuiltin(type_name));
}

void VM::exec_getsuper(VMAbcFile& file, UInt32 v) 
{

    ReadMnObjectRef args(file, v);

    if (IsException())
        return;

    // MA: eReadSealedError or eWriteOplyError possible in property
    const Value value(args.ArgObject, PickValue);
    if (!GetSuperProperty(*this, GetCurrCallFrame().GetOriginationTraits(), args.ArgObject, value, args.ArgMN))
        return ThrowReferenceError(VM::eNotImplementedYet DEBUG_ARG("Couldn't find parent property"));
}

void VM::exec_setsuper(VMAbcFile& file, UInt32 index) 
{
    ReadValueMnObject args(file, index);
    
    if (IsException())
        return;

    // Semantic of this operation is not very clear to me ...
    // Let's do it this way ...
    
    if (!SetSuperProperty(*this, GetCurrCallFrame().GetOriginationTraits(), args.ArgObject, args.ArgMN, args.ArgValue))
        return ThrowReferenceError(VM::eNotImplementedYet DEBUG_ARG("Couldn't set parent property"));
}

void VM::exec_setproperty(VMAbcFile& file, UInt32 mn_index) 
{
    // Set a property.
    
    // Read arguments ...
    ReadValueMnObject args(file, mn_index);
    
    if (IsException())
        return;
    
    SetProperty(*this, args.ArgObject, args.ArgMN, args.ArgValue).DoNotCheck();
}

void VM::exec_callproperty(VMAbcFile& file, UInt32 mn_index, UInt32 arg_count)
{
    // Call a property.
    
    // Read arguments ...
    ReadArgsMnObject args(file, arg_count, mn_index);
    
    if (IsException())
        return;
    
    // Resolve property ...
    PropRef prop = FindObjProperty(*this, args.ArgObject, args.ArgMN);
    
    if (prop)
    {
        Value func;
        
        if (!prop.GetSlotValueUnsafe(*this, func))
            return;
        
        // A TypeError is thrown if the property specified by the
        // multiname is null or undefined.
        if (func.IsNullOrUndefined())
            return ThrowTypeError(VM::eCallOfNonFunctionError DEBUG_ARG("The property specified by the multiname is null or undefined"));
        
        // The [[Call]] property is invoked on the value of the resolved property with the arguments 
        // obj, arg1, ..., argn. 
        Execute(func, args.ArgObject, arg_count, args.GetCallArgs());
    } 
    else
    {
        // No property ...
        if (GetValueTraits(args.ArgObject).IsDynamic())
        {
            // In case of a dynamic object property might be created.
            // But we do not create it for the time being. We just throw
            // an exception.
            return ThrowTypeError(VM::eCallOfNonFunctionError);
        }
        else
        {
            DEBUG_CODE(ASString msg(GetStringManager().CreateConstString("Couldn't find the property "));)
#ifdef USE_DEBUGGER
            msg += AS3::AsString(args.ArgMN);
#endif

            return ThrowReferenceError(VM::ePropertyNotFoundError DEBUG_ARG(msg));
        }
    }
}

void VM::exec_returnvoid()
{
    // Return from a method.

    // Return from the currently executing method. This returns the value undefined. If the
    // method has a return type, then undefined is coerced to that type and then returned.

    /* DO NOT delete this code. It was original behavior.
    OpStack.PushBack(Value::GetUndefined());
    */
    if (!GetCurrCallFrame().NeedToDiscardResult())
        OpStack.PushBack(Value::GetUndefined());

    /* DO NOT delete this code !!!
    Commented out because of a performance reason.
    There was no problem with this so far.
    if (!GetCurrCallFrame().NeedToDiscardResult())
    // Can throw exceptions.
    Coerce2ReturnType(OpStack.Back(), OpStack.Back());
    */
}

void VM::exec_returnvalue()
{
    // Return a value from a method.

    // return_value is popped off of the stack, and coerced to the expected return type of the
    // method. The coerced value is what is actually returned from the method.

    // It is returned on stack, so, if we do not want to coerce it (for the time being), we can
    // just leave it there.

    // A TypeError is thrown if return_value cannot be coerced to the expected return type of the
    // executing method.

    /* DO NOT delete this code. It was original behavior.
    // Coerce to the argument's type ...
    // This "if" statements is a little bit tricky.
    if (!GetCurrCallFrame().NeedToDiscardResult())
    // Can throw exceptions.
    Coerce2ReturnType(OpStack.Back(), OpStack.Back());
    */

    if (GetCurrCallFrame().NeedToDiscardResult())
    {
	    // Below is the old code.
//         if (OpStack.Back().IsPrimitive())
//             // Primitive value cannot be cast to an object, so, just get rid of it.
//             OpStack.Pop();
//         else
//         {
//             // Can throw exceptions.
//             Coerce2ReturnType(OpStack.Back(), OpStack.Back());
//             if (IsException())
//                 return;
// 
//             if (OpStack.GetSize() > 1)
//             {
//                 Value v = OpStack.Pop();
//                 OpStack.Pop();
//                 OpStack.PushBack(v);
//             }
//         }

	/* Commenting out code below will result in a memory leak with the /Tamarin/acceptance/ecma3/ExecutionContexts/e10_1_6.abc test

        {
            if (OpStack.GetSize() > 0)
            {
		// Can throw exceptions.
		Coerce2ReturnType(OpStack.Back(), OpStack.Back());
		if (IsException())
                    return;

		if (OpStack.GetSize() > 1)
		{
			UPInt size = OpStack.GetSize();

			Alg::Swap(OpStack[size - 1], OpStack[size - 2]);
			OpStack.PopBack();
		}
            }
        }
	*/

        OpStack.PopBack();
    }
    else
        Coerce2ReturnType(OpStack.Back(), OpStack.Back());
}

void VM::exec_callproplex(VMAbcFile& file, UInt32 mn_index, UInt32 arg_count)
{
    // Call a property.
    
    // Read arguments ...
    ReadArgsMnObject args(file, arg_count, mn_index);
    
    if (IsException())
        return;
    
    // The property specified by the multiname at index is resolved on the object obj. The [[Call]]
    // property is invoked on the value of the resolved property with the arguments null, arg1, ...,
    // argn. The result of the call is pushed onto the stack.
    
    // Resolve a property ...
    
    PropRef prop = FindObjProperty(*this, args.ArgObject, args.ArgMN);
    if (prop)
    {
        Value func;
        
        if (!prop.GetSlotValueUnsafe(*this, func))
            return;
        
        // A TypeError is thrown if the property specified by the
        // multiname is null or undefined.
        if (func.IsNullOrUndefined())
            return ThrowTypeError(VM::eCallOfNonFunctionError DEBUG_ARG("The property specified by the multiname is null or undefined"));

        // The [[Call]]
        // property is invoked on the value of the resolved property with the arguments null, arg1, ...,
        // argn. The result of the call is pushed onto the stack.

        // This is the only difference from *callproperty* I could find so far ...
        Execute(func, Value::GetUndefined(), arg_count, args.GetCallArgs());
    }
    else
    {
        // No property ...
        if (GetValueTraits(args.ArgObject).IsDynamic())
        {
            // In case of a dynamic object property might be created.
            // But we do not create it for the time being. We just throw
            // an exception.
            return ThrowTypeError(VM::eCallOfNonFunctionError);
        }
        else
        {
            DEBUG_CODE(
                ASString msg(GetStringManager().CreateConstString("Couldn't find the property "));
            )
#ifdef USE_DEBUGGER
            msg += AS3::AsString(args.ArgMN);
#endif
            return ThrowReferenceError(VM::ePropertyNotFoundError DEBUG_ARG(msg));
        }
    }
}

void VM::exec_callpropvoid(VMAbcFile& file, UInt32 mn_index, UInt32 arg_count)
{
    // Call a property, discarding the return value.
    
    // Read arguments ...
    ReadArgsMnObject args(file, arg_count, mn_index);
    
    if (IsException())
        return;
    
    // Resolve property ...
    
    PropRef prop = FindObjProperty(*this, args.ArgObject, args.ArgMN);
    if (prop)
    {
        Value value;
        
        if (!prop.GetSlotValueUnsafe(*this, value))
            return;
        
        // A TypeError is thrown if the property specified by the
        // multiname is null or undefined.
        if (value.IsNullOrUndefined())
            return ThrowTypeError(VM::eCallOfNonFunctionError DEBUG_ARG("The property specified by the multiname is null or undefined"));

        // The [[Call]] property is invoked on the value of the resolved property with the arguments 
        // obj, arg1, ..., argn. 

        // We MUST discard result here.
        Execute(value, args.ArgObject, arg_count, args.GetCallArgs(), true);
    }
    else
    {
        // No property ...
        if (GetValueTraits(args.ArgObject).IsDynamic())
        {
            // In case of a dynamic object property might be created.
            // But we do not create it for the time being. We just throw
            // an exception.
            return ThrowTypeError(VM::eCallOfNonFunctionError);
        }
        else
        {
            DEBUG_CODE(
                ASString msg(GetStringManager().CreateConstString("Couldn't find the property "));
            )

#ifdef USE_DEBUGGER
            msg += AS3::AsString(args.ArgMN, dShort);
#endif
            return ThrowReferenceError(VM::ePropertyNotFoundError DEBUG_ARG(msg));
        }
    }
}

void VM::exec_callmethod(UInt32 method_index, UInt32 arg_count)
{
    // Call a method identified by index in the object's method table (virtual table).
    // This opcode cannot be generated by a compiler. Veryfier must throw
    // exception in this case.
    
    // Read arguments ...
    ReadArgsObject args(*this, arg_count);
    
    if (IsException())
        return;
    
    // method_index is the index of the method to invoke on receiver. arg_count is a u30 that is
    // the number of arguments present on the stack. receiver is the object to invoke the method on.
    
    const Traits& tr = GetValueTraits(args.ArgObject);
    const VTable& vt = tr.GetVT();
    const Value& funct = vt.Get(AbsoluteIndex(method_index));

    // This is my own check.
    if (funct.IsNullOrUndefined())
        return ThrowTypeError(VM::eCallOfNonFunctionError DEBUG_ARG("The property specified by the multiname is null or undefined"));

    // The [[Call]] property is invoked on the value of the resolved property with the arguments 
    // obj, arg1, ..., argn.
    Execute(funct, args.ArgObject, arg_count, args.GetCallArgs());
}

void VM::exec_callsupermethod(UInt32 method_index, UInt32 arg_count)
{
    // Call a method identified by index in the object's method table (virtual table).
    // This opcode cannot be generated by a compiler. Veryfier must throw
    // exception in this case.

    // Read arguments ...
    ReadArgsObject args(*this, arg_count);

    if (IsException())
        return;

    // method_index is the index of the method to invoke on receiver. arg_count is a u30 that is
    // the number of arguments present on the stack. receiver is the object to invoke the method on.

    const Traits* tr = GetCurrCallFrame().GetOriginationTraits();

    if (tr == NULL)
        tr = &GetValueTraits(args.ArgObject);

    if (tr->GetParent() == NULL)
        return ThrowReferenceError(VM::eNotImplementedYet DEBUG_ARG("Couldn't find parent property"));

    const VTable& vt = tr->GetParent()->GetVT();
    const Value& funct = vt.Get(AbsoluteIndex(method_index));

    // This is my own check.
    if (funct.IsNullOrUndefined())
        return ThrowTypeError(VM::eCallOfNonFunctionError DEBUG_ARG("The property specified by the multiname is null or undefined"));

    // The [[Call]] property is invoked on the value of the resolved property with the arguments 
    // obj, arg1, ..., argn. 
    Execute(funct, args.ArgObject, arg_count, args.GetCallArgs());
}

void VM::exec_callstatic(VMAbcFile& file, UInt32 method_index, UInt32 arg_count)
{
    // Call a method identified by index in the abcFile method table.
    
    // Read arguments ...
    ReadArgsObject args(*this, arg_count);
    
    if (IsException())
        return;
    
    // method_index is the index of the method_info of the method to invoke.
            
    // The receiver will be used as the “this” value for the method. 
    // The result of the method is pushed onto the stack.

    {
        const Abc::MethodInfo& mi = file.GetMethods().Get(method_index);
        const Abc::MethodBodyInfo& mbi = file.GetMethodBodyInfo(method_index);
        
        AddFrame(
            Value::GetUndefined(), // There is no object to track its life time.
            file,
            mi,
            mbi,
            NULL, // No traits available.
            args.ArgObject,
            NULL, // ??? No saved scope ???
            arg_count,
            args.GetCallArgs(),
            false, // discard_result
            NULL
            DEBUG_ARG(args.ArgObject.GetObject()->GetName() + " callstatic")
            );
    }
}

void VM::exec_newfunction(UInt32 method_ind)
{
    // When creating the new function object the scope stack used is the current scope stack when
    // this instruction is executed, and the body is the method_body entry that references the
    // specified method_info entry.

    // Let's use a stored scope if there is one.
    const ScopeStackType* sss = GetCurrCallFrame().GetSavedScope();
    // It is OK to take a file from a current call frame.
    VMAbcFile& file = GetCurrCallFrame().GetFile();
    if (sss && !sss->IsEmpty())
    {
        ScopeStackType ss(*sss);
        const ScopeStackType& vm_ss = GetScopeStack();
        const UPInt size = vm_ss.GetSize();

        for (UPInt i = GetScopeStackBaseInd(); i < size; ++i)
            ss.PushBack(vm_ss[i]);

        OpStack.PushBack(GetClassFunction().MakeInstance(file, method_ind, ss, GetValueTraits(GetGlobalObject())));
    }
    else
    {
        ScopeStackType ss(GetMemoryHeap());
        const ScopeStackType& vm_ss = GetScopeStack();
        const UPInt size = vm_ss.GetSize();

        for (UPInt i = GetScopeStackBaseInd(); i < size; ++i)
            ss.PushBack(vm_ss[i]);

        OpStack.PushBack(GetClassFunction().MakeInstance(file, method_ind, ss, GetValueTraits(GetGlobalObject())));
    }

    // No exceptions in this method;
    // Tamarin throws exceptions in this opcode.
}

void VM::exec_call(UInt32 arg_count)
{
    // Call a closure.
    
    // Read arguments ...
    ReadArgsObjectValue args(*this, arg_count);
    
    if (IsException())
        return;
    
    // function is the closure that is being called. receiver is the object to use for the “this” value. This will
    // invoke the [[Call]] property on function with the arguments receiver, arg1, ..., argn. 
    // The result of invoking the [[Call]] property will be pushed onto the stack.

    Execute(args.value, args.ArgObject, arg_count, args.GetCallArgs());
}

void VM::exec_callsuper(VMAbcFile& file, UInt32 mn_index, UInt32 arg_count)
{
    // Call a method on a base class.
    
    // Read arguments ...
    ReadArgsMnObject args(file, arg_count, mn_index);
    
    if (IsException())
        return;
    
    // The base class of receiver is determined and the method indicated by the multiname is
    // resolved in the declared traits of the base class.

    Value func;
    if (!GetSuperProperty(*this, GetCurrCallFrame().GetOriginationTraits(), func, args.ArgObject, args.ArgMN))
        // Not confirmed ...
        return ThrowReferenceError(VM::eCallNotFoundError DEBUG_ARG("Couldn't find the parent property"));

    // This is my own check.
    if (func.IsNullOrUndefined())
        return ThrowTypeError(VM::eCallOfNonFunctionError DEBUG_ARG("The parent property is null or undefined"));

    // The [[Call]] property is invoked on the value of the resolved property with the arguments 
    // obj, arg1, ..., argn. 
    // The first argument will be used as the “this” value for the method.
    // The result of the method call is pushed onto the stack.
    
    Execute(func, args.ArgObject, arg_count, args.GetCallArgs());
}

void VM::exec_callsupervoid(VMAbcFile& file, UInt32 mn_index, UInt32 arg_count)
{
    // Call a method on a base class, discarding the return value.
    
    // Read arguments ...
    ReadArgsMnObject args(file, arg_count, mn_index);
    
    if (IsException())
        return;
    
    // The base class of receiver is determined and the method indicated by the multiname is
    // resolved in the declared traits of the base class.

    Value value;
    if (!GetSuperProperty(*this, GetCurrCallFrame().GetOriginationTraits(), value, args.ArgObject, args.ArgMN))
        // Not confirmed ...
        return ThrowReferenceError(VM::eNotImplementedYet DEBUG_ARG("Couldn't find the parent property"));

    // This is my own check.
    if (value.IsNullOrUndefined())
        return ThrowTypeError(VM::eCallOfNonFunctionError DEBUG_ARG("The parent property is null or undefined"));

    // The [[Call]] property is invoked on the value of the resolved property with the arguments 
    // obj, arg1, ..., argn. 
    // The first argument will be used as the “this” value for the method.
    // The result of the method call is pushed onto the stack.
    
    // We MUST discard result here.
    Execute(value, args.ArgObject, arg_count, args.GetCallArgs(), true);
}

void VM::exec_construct(UInt32 arg_count)
{
    // Construct an instance.
    
    // Read arguments ...
    // ReadArgsObjectRef doesn't work here.
    ReadArgsObject args(*this, arg_count);
    
    if (IsException())
        return;
    
    // In case we can use more efficient code here.
    SF_ASSERT(!args.ArgObject.IsTraits());

    // "object" is the function that is being constructed. This will invoke the [[Construct]] property on object with
    // the given arguments. The new instance generated by invoking [[Construct]] will be pushed onto the stack.
    
    // TODO:
    // !!! Look up for a "Construct" property ...
    //A TypeError is thrown if object does not implement the [[Construct]] property.
    
    if (!args.ArgObject.IsObject())
        return ThrowTypeError(VM::eNotImplementedYet);

    Value result;

#ifdef CONSTRUCT_IN_TRAITS
    // GetInstanceTraits() can malfunction here.
    GetValueTraits(args.ArgObject).Construct(result, arg_count, args.GetCallArgs());
#else
    if (args.ArgObject.IsNullOrUndefined())
        return ThrowTypeError(VM::eConvertNullToObjectError);

    args.ArgObject.GetObject()->Construct(result, arg_count, args.GetCallArgs());
#endif

    if (IsException())
        return;

    OpStack.PushBack(result);
    AdjustOriginalStackSize();
}

void VM::exec_constructsuper(const CallFrame& cf, UInt32 arg_count)
{
    // Construct an instance of the base class.
    // This will invoke the constructor on the base class of object with 
    // the given arguments.
    
    // Read arguments ...
    ReadArgsObject args(*this, arg_count);
    
    if (IsException())
        return;
    
    const Traits* ot = cf.GetOriginationTraits();
    SF_ASSERT(ot);

    /* Old code.
    // Do not delete. Keep for reference.
    const Class& cl = ot->GetClass();

    SF_ASSERT(cl.GetParentClass());
    if (cl.GetParentClass())
    {
        Class& parent_class = *cl.GetParentClass();
        
        // This will invoke the constructor on the base class of object with the given arguments.
        parent_class.GetNewObjectTraits().Super(*ot, args.ArgObject, arg_count, args.GetCallArgs());
    }
    */

    if (ot->GetParent())
    {
        // Dirty trick.
        // constructsuper can be called only for instances, so we can cast
        // Traits to InstanceTraits.
        const InstanceTraits::Traits* ptr = static_cast<const InstanceTraits::Traits*>(ot->GetParent());

        ptr->Super(*ot, args.ArgObject, arg_count, args.GetCallArgs());
    }
}

void VM::exec_constructprop(VMAbcFile& file, UInt32 mn_index, UInt32 arg_count)
{
    // Construct a property.

    // The [[Construct]] property is invoked on the value of the resolved property with the arguments
    // obj, arg1, ..., argn. 
    // The new instance generated by invoking [[Construct]] will be pushed onto the stack.          
    
    // Read arguments ...
    ReadArgsMnObjectRef args(file, arg_count, mn_index);
    
    if (IsException())
        return;

    bool constructed = false;

    if (args.ArgMN.IsQName())
    {
        const ClassTraits::Traits* ctr = Resolve2ClassTraits(args.ArgMN);

        if (ctr)
        {
            InstanceTraits::Traits& itr = ctr->GetInstanceTraits();

#ifdef CONSTRUCT_IN_TRAITS
            itr.Construct(args.ArgObject, arg_count, args.GetCallArgs());
#else
            itr.GetClass().Construct(args.ArgObject, arg_count, args.GetCallArgs());
#endif
            constructed = true;
        }
    }

    if (!constructed)
    {
        // Fall back to the regular version.

        PropRef prop = FindObjProperty(*this, args.ArgObject, args.ArgMN);
        if (prop)
        {
            Value value;

            if (!prop.GetSlotValueUnsafe(*this, value))
                return;

            // TODO:
            // !!! Look up for a "Construct" property ...
            //A TypeError is thrown if object does not implement the [[Construct]] property.

#ifdef CONSTRUCT_IN_TRAITS
            // GetInstanceTraits() can malfunction here.
            GetValueTraits(value).Construct(args.ArgObject, arg_count, args.GetCallArgs());
#else
            if (value.IsNullOrUndefined())
                return ThrowTypeError(VM::eConvertNullToObjectError);

            value.GetObject()->Construct(args.ArgObject, arg_count, args.GetCallArgs());
#endif        
        } 
        else
            return ThrowReferenceError(VM::ePropertyNotFoundError);
    }

    if (IsException())
        return;

    // We had AdjustOriginalStackSize() call here earlier;
}

void VM::exec_getdescendants(VMAbcFile& file, UInt32 mn_index) 
{
    // XML - related ...
    
    // Read arguments ...
    ReadMnObjectRef args(file, mn_index);
    
    if (IsException())
        return;
    
    // Not finished yet ...
    //SF_ASSERT(false);
    
    // A TypeError is thrown if obj is not of type XML or XMLList.

    // !!! This is a wrong result !!!
    args.ArgObject.SetNull();
}

void VM::exec_newcatch(VMAbcFile& file, const Abc::MethodBodyInfo::ExceptionInfo& e)
{
    OpStack.PushBack(GetClassCatch().MakeInstance(file, e));

    // No exceptions in this method;
    // Tamarin throws exceptions in this opcode.
}

void VM::exec_findpropstrict(VMAbcFile& file, UInt32 mn_index) 
{
    ReadMn args(file, mn_index);
    
    if (IsException())
        return;

    // This searches the scope stack, and then the saved scope in the current method closure, for a
    // property with the name specified by the multiname at mn_index.
    
    // Find a property.
    PropRef prop = FindScopeProperty(*this, GetScopeStackBaseInd(), GetScopeStack(), args.ArgMN);

    // Look in Saved Scope.
    if (!prop)
    {
        const ScopeStackType* ss = CallStack.Back().GetSavedScope();

        if (ss)
            prop = FindScopeProperty(*this, 0, *ss, args.ArgMN);
    }

    // Look in Global Objects. This is correct behavior.
    if (!prop)
        prop = FindScopeProperty(*this, 0, GetGlobalObjects(), args.ArgMN);

    if (prop)
    {
        // Push found object ...
        OpStack.PushBack(prop.GetValue());
    } else
    {
        // A ReferenceError is thrown if the property is not resolved in any object on the scope stack.
        // return ThrowReferenceError("The property is not resolved in any object on the scope stack");

        DEBUG_CODE(
            ASString msg(GetStringManager().CreateConstString("Variable "));
            msg += args.ArgMN.GetName().AsString();
            msg += " is not defined.";
        )
        return ThrowReferenceError(VM::eUndefinedVarError DEBUG_ARG(msg));
    }
}

void VM::exec_findproperty(VMAbcFile& file, UInt32 mn_index) 
{
    // Search the scope stack for a property.
    
    ReadMn args(file, mn_index);

    if (IsException())
        return;

    // This searches the scope stack, and then the saved scope in the current method closure, for a
    // property with the name specified by the multiname at mn_index.
    
    // If the property is resolved then the object it was resolved in is pushed onto the stack. If the
    // property is unresolved in all objects on the scope stack then the global object is pushed onto
    // the stack.

    // Find a property.

    PropRef prop = FindScopeProperty(*this, GetScopeStackBaseInd(), GetScopeStack(), args.ArgMN);

    // Look in Saved Scope.
    if (!prop)
    {
        const ScopeStackType* ss = CallStack.Back().GetSavedScope();

        if (ss)
            prop = FindScopeProperty(*this, 0, *ss, args.ArgMN);
    }

    // Look in Global Objects. This is correct behavior.
    if (!prop)
        prop = FindScopeProperty(*this, 0, GetGlobalObjects(), args.ArgMN);

    if (prop)
    {
        // Push found object ...
        OpStack.PushBack(prop.GetValue());
    } else
    {
        // Push GlobalObject ...
        OpStack.PushBack(GetGlobalObject());
    }
}

void VM::exec_getlex(VMAbcFile& file, UInt32 mn_index) 
{
    // Find and get a property.
    
    ReadMnCt args(file, mn_index);
    
    if (IsException())
        return;
    
    // This is the equivalent of doing a "findpropstict" followed by a "getproperty". It will find the
    // object on the scope stack that contains the property, and then will get the value from that object.

    // Find a property.
    // The same as "findpropstict" ...

    const Multiname mn(file, args.ArgMN);

    PropRef prop = FindScopeProperty(*this, GetScopeStackBaseInd(), GetScopeStack(), mn);

    // Look in Saved Scope.
    if (!prop)
    {
        const ScopeStackType* ss = CallStack.Back().GetSavedScope();

        if (ss)
            prop = FindScopeProperty(*this, 0, *ss, mn);
    }

    // Look in Global Objects. This is correct behavior.
    if (!prop)
        prop = FindScopeProperty(*this, 0, GetGlobalObjects(), mn);

    if (prop)
    {
        // ... and then get the value from this object.

        // Efficient version.
//         OpStack.PushBack(Value::GetUndefined());
//         prop.GetValue(OpStack.Back());
//         if (IsException())
//             return OpStack.PopBack();

        // Current version.
        Value value;

        if (!prop.GetSlotValueUnsafe(*this, value))
            return;

        // DO NOT delete this code.
//         // That should be already handled by GetValue().
//         switch (value.GetKind())
//         {
//         case Value::kFunction:
//             value = GetClassFunction().MakeInstance2(value.AsFunction(), prop.GetObject() DEBUG_ARG(value.AsFunction().GetFunctionName()));
//             break;
//         case Value::kThunk:
//             value = GetClassFunction().MakeInstance(
//                 (InstanceTraits::Traits&)prop.GetObject()->GetValueTraits(),
//                 *prop.GetObject(), 
//                 value.AsThunk()
//                 );
//             break;
//         default:
//             break;
//         }

        OpStack.PushBack(value);
    } else
    {
        // A ReferenceError is thrown if the property is not resolved in any object on the scope stack.
        return ThrowReferenceError(VM::eNotImplementedYet DEBUG_ARG("The property is not resolved in any object on the scope stack"));
    }
}

void VM::exec_getproperty(VMAbcFile& file, UInt32 mn_index) 
{
    // Get a property.
    
    // Read arguments ...
    ReadMnObjectRef args(file, mn_index);
    
    if (IsException())
        return;
    
    // The property with the name specified by the multiname will be resolved in object, and the
    // value of that property will be pushed onto the stack. If the property is unresolved,
    // undefined is pushed onto the stack.
    
    Value _this(args.ArgObject, PickValue);
    if(!AS3::GetProperty(*this, _this, args.ArgMN, args.ArgObject))
    {
        if (IsException())
            return;

        // This logic seems to be specific to Tamarin.
        if (_this.IsPrimitive())
            return ThrowReferenceError(VM::ePropertyNotFoundError);
    }
}

void VM::exec_initproperty(VMAbcFile& file, UInt32 mn_index) 
{
    // Initialize a property.
    
    // Read arguments ...
    ReadValueMnObject args(file, mn_index);
    
    if (IsException())
        return;
    
    InitializerGuard __(*this);

    AS3::SetProperty(*this, args.ArgObject, args.ArgMN, args.ArgValue).DoNotCheck();
}

void VM::exec_deleteproperty(VMAbcFile& file, UInt32 mn_index) 
{
    // Delete a property.
    
    // Read arguments ...
    ReadMnObjectRef args(file, mn_index);

    // TODO:
    // A ReferenceError is thrown if object is null or undefined.
    // It is TypeError for now.

    if (IsException())
        return;

    // This will invoke the [[Delete]] method on object with the name specified by the multiname.
    // If object is not dynamic or the property is a fixed property then nothing happens, and false
    // is pushed onto the stack. If object is dynamic and the property is not a fixed property, it is
    // removed from object and true is pushed onto the stack.
    bool result = false;

    if (args.ArgObject.IsObject())
    {
        Object& obj = *args.ArgObject.GetObject();

        if (obj.IsDynamic())
            result = obj.DeleteProperty(args.ArgMN);

        if (IsException())
            return;
    }
    else if (args.ArgObject.IsPrimitive())
        // We want to bypass a thunk here.
        // This logic seems to be Tamarin-specific.
        return ThrowReferenceError(VM::eDeleteSealedError);

    // All primitive values are also sealed classes.

    args.ArgObject.SetBool(result);
}

void VM::exec_getslot(UInt32 slot_index) 
{
    // Get the value of a slot.
    
    // Read arguments ...
    ReadObjectRef args(*this);
    
    if (IsException())
        return;
    
    SF_ASSERT(args.ArgObject.IsObject());
    args.ArgObject.GetObject()->GetSlotValue(SlotIndex(slot_index), args.ArgObject).DoNotCheck();
}

// void VM::exec_getabsslot(UPInt slot_ind)
// {
//     // Get the value of a slot.
//     // slot_ind is ONE based.
//     // slot_ind - 1 is an index in a slot table.
// 
//     // Read arguments ...
//     ReadObjectRef args(*this);
//     
//     if (IsException())
//         return;
//     
//     SF_ASSERT(args.ArgObject.IsObject());
//     Object& obj = *args.ArgObject.GetObject();
//     obj.GetTraits().GetSlotValue(args.ArgObject, AbsoluteIndex(slot_ind - 1), &obj).DoNotCheck();
// }

void VM::exec_setabsslot(UPInt slot_ind)
{
    // Set the value of a slot.
    // slot_ind is ONE based.
    // slot_ind - 1 is an index in a slot table.

    // Read arguments ...
    ReadValueObject args(*this);

    if (IsException())
        return;

    SF_ASSERT(args.ArgObject.IsObject());
    Object& obj = *args.ArgObject.GetObject();

    obj.GetTraits().SetSlotValue(AbsoluteIndex(slot_ind - 1), args.ArgValue, &obj).DoNotCheck();
}

void VM::exec_initabsslot(UPInt slot_ind)
{
    // Set the value of a slot.
    // slot_ind is ONE based.
    // slot_ind - 1 is an index in a slot table.

    // Read arguments ...
    ReadValueObject args(*this);

    if (IsException())
        return;

    SF_ASSERT(args.ArgObject.IsObject());
    Object& obj = *args.ArgObject.GetObject();

    InitializerGuard __(*this);

    obj.GetTraits().SetSlotValue(AbsoluteIndex(slot_ind - 1), args.ArgValue, &obj).DoNotCheck();
}

void VM::exec_setslot(UInt32 slot_index) 
{
    // Set the value of a slot.
    
    // Read arguments ...
    ReadValueObject args(*this);
    
    if (IsException())
        return;
    
    args.ArgObject.GetObject()->SetSlotValue(SlotIndex(slot_index), args.ArgValue).DoNotCheck();
}

void VM::exec_coerce(VMAbcFile& file, UInt32 mn_index) 
{
    // Coerce a value to a specified type.
    
    // Read arguments ...
    ReadMnCtValueRef args(file, mn_index);
    
    if (IsException())
        return;
    
    // The type specified by the multiname is resolved, and value is coerced to that type. The
    // resulting value is pushed onto the stack. If any of value’s base classes, or implemented
    // interfaces matches the type specified by the multiname, then the conversion succeeds and the
    // result is pushed onto the stack.
    
    // Resolve multiname ...
    const ClassTraits::Traits* to = Resolve2ClassTraits(file, args.ArgMN);
    
    if (to)
    {
        if (to->Coerce(args.value, args.value))
            return;
        else
            // A TypeError is thrown if value cannot be coerced to the specified type.
            return ThrowTypeError(VM::eNotImplementedYet DEBUG_ARG("Value cannot be coerced to the specified type"));
    }
    
    return ThrowTypeError(VM::eNotImplementedYet DEBUG_ARG("Couldn't resolve multiname."));
}

void VM::exec_newobject(UInt32 arg_count) 
{
    Pickable<Instances::Object> o = MakeObject();
    
    for (UInt32 i = 0; i < arg_count; ++i)
    {
        Value value = OpStack.Pop();
        Value name  = OpStack.Pop();

        // All object properties are dynamic.
        o->AddDynamicSlotValuePair(name.AsString(), value);
    }
    
    OpStack.PushBack(o);

    // It looks like this function doesn't throw exceptions.
    // Tamarin throws exceptions in this opcode.
}

void VM::exec_newclass(VMAbcFile& file, UInt32 v) 
{
    const Abc::ClassInfo& class_info = file.GetClasses().Get(v);
    
    Value base_type = OpStack.Pop();
    
    // Get a base type if any ...
    Class*  base_class = NULL;

    // TBD:
    // The base_type argument must be coerced to Class and then must be a base class 
    // of class_info. This means that there is no automatic creation of base classes,
    // instead compiler is responsible for creating base class instance ahead of time
    // and passing it to us.
    // NOTE: newclass is also called for interfaces,
    // in which case base_type is typically null.

    if (base_type.IsNullOrUndefined())
    {
        if (class_info.HasSuperClass())
            return ThrowTypeError(VM::eConvertNullToObjectError DEBUG_ARG("Object is null or undefined"));
    }
    else
    {
        if (!base_type.IsClass())
            // Is this a correct exception type and a message?
            return ThrowTypeError(VM::eConvertNullToObjectError DEBUG_ARG("Object is null or undefined"));

        base_class  = &base_type.AsClass();
        // Double check ...
        //SF_ASSERT(base_class == Resolve2Class(file, class_info.GetSuperClassName(file.GetConstPool())));
    }

    // check for *scaleform extension* packages.
    const Abc::Multiname& name = class_info.GetName(file.GetConstPool());
    Instances::Namespace& t_ns = file.GetInternedNamespace(name.GetNamespaceInd());
    const ASString& t_uri = t_ns.GetUri();
    const StringDataPtr t_str(t_uri.ToCStr(), t_uri.GetSize());
    const StringDataPtr scaleform_gfx("scaleform.gfx");

    if (t_str.GetSize() >= scaleform_gfx.GetSize() && SFstrncmp(t_str.ToCStr(), scaleform_gfx.ToCStr(), scaleform_gfx.GetSize()) == 0)
    {
        // This is *scaleform.gfx extension* package.
        const ASString t_name = file.GetInternedString(name.GetNameInd());
        Value value;

        if (GetGlobalObjectCPP().GetProperty(Multiname(t_ns, t_name), value))
            OpStack.PushBack(value);
        else
            ThrowReferenceError(VM::ePropertyNotFoundError);
    }
    else
    {
        // When this instruction is executed, the scope stack must contain all the scopes of all base
        // classes, as the scope stack is saved by the created ClassClosure.

        ClassTraits::UserDefined& cud = GetUserDefinedTraits(file, class_info);

        if (cud.GetInstanceTraits().HasConstructorSetup())
            // This class was already created by another script. It happens.
            OpStack.PushBack(Value(&cud.GetInstanceTraits().GetConstructor()));
        else
            OpStack.PushBack(cud.MakeClass(base_class));

        AdjustOriginalStackSize();
    }

}

void VM::exec_astype(VMAbcFile& file, UInt32 mn_index) 
{
    // Return the same value, or null if not of the specified type.
    
    // Read arguments ...
    ReadMnCtValueRef args(file, mn_index);
    
    if (IsException())
        return;
    
    // Pop value off of the stack. If value is of the type specified by the multiname, push value back
    // onto the stack. If value is not of the type specified by the multiname, then push null onto
    // the stack.
    
    const ClassTraits::Traits* tr = Resolve2ClassTraits(file, args.ArgMN);
    
    // and must be the name of a type ...
    if (tr)
    {
        const ClassTraits::Traits& value_tr = GetClassTraits(args.value);
        
        if (tr != &value_tr)
        {
            args.value.SetNull();
            return;
        }
    }
    
    return ThrowTypeError(VM::eNotImplementedYet DEBUG_ARG("The multiname must be the name of a type"));
}

void VM::exec_instanceof() 
{
    // Check the prototype chain of an object for the existence of a type.

    Value type = OpStack.Pop();
    if (!type.IsClass() || !type.IsFunction())
        return ThrowTypeError(VM::eCantUseInstanceofOnNonObjectError DEBUG_ARG("RHS of instanceof must be a class or function"));

    Value& value = OpStack.Back();
    Value& result = OpStack.Back();
    
    const Traits* cur_tr = &GetValueTraits(value);
    while (cur_tr)
    {
        const AS3::Object& proto = const_cast<Class&>(cur_tr->GetClass()).GetPrototype();

        // "type" can be a primitive value.
        if (&proto == &GetClass(type))
            return result.SetBool(true);

        cur_tr = cur_tr->GetParent();
    }

    result.SetBool(false);

}

void VM::exec_istype(VMAbcFile& file, UInt32 mn_index) 
{
    // Check whether an Object is of a certain type.
    
    // Read arguments ...
    ReadMnCtValueRef args(file, mn_index);
    // No exceptions at this point.
    
    const ClassTraits::Traits* type = Resolve2ClassTraits(file, args.ArgMN);
    
    if (type == NULL)
        // Unresolved multiname ...
        return ThrowTypeError(VM::eNotImplementedYet DEBUG_ARG("The multiname must be the name of a type"));
    
    // Resolve the type specified by the multiname. Let indexType refer to that type. Compute the
    // type of value, and let valueType refer to that type. If valueType is the same as indexType, result
    // is true. If indexType is a base type of valueType, or an implemented interface of valueType,
    // then result is true. Otherwise result is set to false. Push result onto the stack.
    
    args.value.SetBool(IsOfType(args.value, *type));
}

void VM::exec_debugline(CallFrame& cf, UInt32 v)
{
    SF_UNUSED2(cf, v);

#ifdef SF_AMP_SERVER
    AMP::ViewStats* stats = GetAdvanceStats();
    if (stats != NULL)
    {
        UInt64 activeFile;
        stats->GetActiveLine(&activeFile, NULL);
        SetActiveLine(activeFile, v);
        if (!cf.IsRegisteredFunction())
        {
            cf.SetRegisteredFunction(true);
            stats->RegisterScriptFunction(cf.GetFile().GetAbcFile().GetSwdHandle(), 
                cf.GetMethodBodyInfo().GetMethodInfoInd(), cf.Name->pData, 0, 3);
        }
        if (stats->IsDebugPaused())
        {
            AmpServer::GetInstance().SendCurrentState();
        }
        stats->DebugWait();
    }

    cf.CurrLineNumber = v;
#endif
}

void VM::exec_debugfile(CallFrame& cf, UInt32 v)
{
    SF_UNUSED2(cf, v);

#ifdef SF_AMP_SERVER
    VMAbcFile& file = cf.GetFile();

    AMP::ViewStats* stats = GetAdvanceStats();
    if (stats != NULL)
    {
        stats->RegisterSourceFile(file.GetAbcFile().GetSwdHandle(), v, file.GetInternedString(v).ToCStr());

        UInt64 fileId = file.GetAbcFile().GetSwdHandle();
        fileId <<= 32;
        fileId += v;
        SetActiveLine(fileId, 0);
    }

    cf.CurrFileInd = v;
#endif
}

void VM::exec_astypelate() 
{
    // Return the same value, or null if not of the specified type.
    // (Similar to "astype") ...
    
    // Pop class and value off of the stack. class should be an object of type Class. If value is of the
    // type specified by class, push value back onto the stack. If value is not of the type specified by
    // class, then push null onto the stack.
    
    const Value vclass = OpStack.Pop();
    
    if (!vclass.IsClass())
    {
        // A TypeError is thrown if class is not of type Class.
        if (vclass.IsNullOrUndefined())
        {
            return ThrowTypeError(
                vclass.IsNull() ? eConvertNullToObjectError :  eConvertUndefinedToObjectError
                 DEBUG_ARG("Class value is null or undefined"));
        }
        return ThrowTypeError(VM::eIsTypeMustBeClassError DEBUG_ARG("Value is not of type Class"));
    }
    
    const ClassTraits::Traits& tr = vclass.AsClass().GetClassTraits();
    Value& value = OpStack.Back();
    
    if (!IsOfType(value, tr))
        value.SetNull();
}

void VM::exec_newarray(UInt32 arr_size) 
{
    Pickable<Instances::Array> a = MakeArray();
    
    a->Append(OpStack, OpStack.GetSize() - arr_size, arr_size);
    OpStack.Resize(OpStack.GetSize() - arr_size);
    
    OpStack.PushBack(a);

    // No exceptions in this method;
    // Tamarin throws exceptions in this opcode.
}

void VM::exec_hasnext2(UInt32 object_reg, UInt32 index_reg) 
{
    AbsoluteIndex reg_ind(index_reg);
    AbsoluteIndex obj_ind(object_reg);
    Value& obj_value = GetRegister(obj_ind);

    // Check for null/undefined
    if (obj_value.IsNullOrUndefined())
        // Push false.
        return OpStack.PushBack(Value(false));

    UInt32 v = 0;
    if (!GetRegister(reg_ind).Convert2UInt32(v))
        // Exception
        return;

    GlobalSlotIndex slot_ind(v);

    if (obj_value.IsObject())
    {
        Object& obj = obj_value;
        // This can be implemented as a virtual function.
        if (IsArray(obj.GetClass()))
        {
            Instances::Array& array = static_cast<Instances::Array&>(obj);

            // Set index.
            const GlobalSlotIndex next_ind = array.GetNextIndex(slot_ind);
            SetRegister(reg_ind, Value(next_ind.Get()));

            // Push value ...
            OpStack.PushBack(Value(next_ind.IsValid()));
        }
        else
        {
            GlobalSlotIndex ind = obj.GetNextPropertyIndex(slot_ind);
            Object* cur_obj = &obj;

            const Traits* cur_tr = &obj.GetTraits();
            while (cur_tr && !ind.IsValid())
            {
                cur_obj = &const_cast<Class&>(cur_tr->GetClass()).GetPrototype();

                ind = cur_obj->GetNextPropertyIndex(GlobalSlotIndex(0));
                cur_tr = cur_tr->GetParent();
            }

            // Set object.
            if (ind.IsValid())
            {
                if (cur_obj != &obj)
                    SetRegister(obj_ind, Value(cur_obj));
            }
            else
                // Set Object to NULL.
                SetRegister(obj_ind, Value::GetNull());

            // Set index.
            SetRegister(reg_ind, Value(ind.Get()));

            // Store boolean result on stack.
            OpStack.PushBack(Value(ind.IsValid()));
        }
    }
    else
    {
        // Primitive values.

        const Traits& t = GetValueTraits(obj_value);
        GlobalSlotIndex ind = t.GetNextPropertyIndex(slot_ind);

        Object* cur_obj = NULL;
        const Traits* cur_tr = &t;
        while (cur_tr && !ind.IsValid())
        {
            cur_obj = &const_cast<Class&>(cur_tr->GetClass()).GetPrototype();

            ind = cur_obj->GetNextPropertyIndex(GlobalSlotIndex(0));
            cur_tr = cur_tr->GetParent();
        }

        // Set object.
        if (ind.IsValid())
        {
            if (cur_obj != NULL)
                // Prototype object was found.
                SetRegister(obj_ind, Value(cur_obj));
        }
        else
            // Set Object to NULL.
            SetRegister(obj_ind, Value::GetNull());

        // Set index.
        SetRegister(reg_ind, Value(ind.Get()));

        // Store boolean result on stack.
        OpStack.PushBack(Value(ind.IsValid()));
    }
}

void VM::exec_hasnext() 
{
    SH2<1, Value, SInt32> stack(OpStack);

    if (stack.IsException())
        return;

    // This logic should work correctly.
    SF_ASSERT(stack._1.GetObject());
    stack._1 = Value(stack._1.GetObject()->GetNextPropertyIndex(GlobalSlotIndex(stack._2)).Get());
}

void VM::exec_nextvalue() 
{
    SH2<1, Value, SInt32> stack(OpStack);

    if (stack.IsException())
        return;

    SF_ASSERT(stack._1.GetObject());
    Object& obj = *stack._1.GetObject();

    if (IsArray(obj.GetClass()) && stack._2 >= 0)
    {
        Instances::Array& array = static_cast<Instances::Array&>(obj);
        stack._1 = array.At(AbsoluteIndex(GlobalSlotIndex(stack._2)).Get());
    }
    else
        obj.GetNextPropertyValue(stack._1, GlobalSlotIndex(stack._2));
}

void VM::exec_in() 
{
    // Determine whether an object has a named property.

    // name is converted to a String, and is looked up in obj. If no property is found, then the
    // prototype chain is searched by calling [[HasProperty]] on the prototype of obj. If the
    // property is found result is true. Otherwise result is false. Push result onto the stack.

    SH2<1> stack(OpStack);
    // No exceptions at this point.

    //stack._2 is obj ...
    //stack._1 is name_obj ...

    // name is converted to a String ...
//     if (!stack._1.ToStringValue(GetStringManager()))
//         return;

    // This check is necessary.
    if (stack._2.IsNullOrUndefined())
        return ThrowTypeError(VM::eConvertNullToObjectError DEBUG_ARG("A term is undefined and has no properties"));

    // Find property ...
    PropRef prop = 
        FindObjProperty(
            *this,
            stack._2, 
            Multiname(
                GetPublicNamespace(), 
                stack._1
            )
        );

    stack._1 = Value(prop.IsFound());
}

void VM::exec_applytype(UInt32 arg_count) 
{
    // Read arguments ...
    ReadArgsObjectRef args(*this, arg_count);

    if (IsException())
        return;

    if (!args.ArgObject.IsClass())
        return ThrowTypeError(eTypeAppOfNonParamType);

    args.ArgObject = &args.ArgObject.AsClass().ApplyTypeArgs(args.GetCallArgsNum(), args.GetCallArgs());
}

void VM::Execute(const Value& func, const Value& _this, unsigned argc, const Value* argv, bool discard_result)
{
    switch(func.GetKind())
    {
    case Value::kFunction:
        {
            Instances::Function& f = func;
            
            // The [[Call]] property is invoked on the value of the resolved property with the arguments 

            f.Execute(_this, argc, argv, discard_result);
            // Bytecode. Exception will be handled by bytecode.
        }
        break;
    case Value::kClass:
    case Value::kObject:
        {
            // It doesn't look like a function ...
            // Just call a "Call" method ...
            
            Value result;
            
            SF_ASSERT(func.GetObject());
            func.GetObject()->Call(_this, result, argc, argv);
            
            // C++. We need to handle exceptions ...
            if (IsException())
                return;
            
            if (!discard_result)
                OpStack.PushBack(result);
        }
        break;
    case Value::kThunk:
        {
            const ThunkInfo& thunk = func.AsThunk();
            Value result;
            
            // Check number of arguments.
            if (argc > thunk.GetMaxArgNum() || argc < thunk.GetMinArgNum())
                return ThrowArgumentError(VM::eWrongArgumentCountError, argc, thunk.GetMinArgNum(), thunk.GetMaxArgNum());

            // DO NOT delete this code.
            /* Disabled for the time being.
            // Check type of *this*.
            Value coerced_this;
            if (_this.IsClass())
                coerced_this = _this;
            else
            {
                const Class* cl = Resolve2Class(thunk);
                SF_ASSERT(cl);
                if (!cl->Coerce(_this, coerced_this, GetStringManager()))
                    return ThrowTypeError(VM::eNotImplementedYet);
            }

            // Run method.
            (*thunk.Method)(*this, coerced_this, result, argc, argv);
            */

            // Hack for Function::apply() and Function::call()
            if (_this.IsClosure() && argc > 0)
            {
                if (&thunk == &InstanceTraits::Function::GetApplyThunkInfo() ||
                    &thunk == &InstanceTraits::Function::GetCallThunkInfo())
                    const_cast<Value&>(argv[0]) = _this.GetClosure();

                // Run method.
                (*thunk.Method)(*this, Value(_this.GetObject()), result, argc, argv);
            }
            else
                // Run method.
                (*thunk.Method)(*this, _this, result, argc, argv);
            
            // C++. We need to handle exceptions ...
            if (IsException())
                return;
            
            if (!discard_result)
                OpStack.PushBack(result);
        }
        break;
    case Value::kThunkFunction:
        {
            // ThunkClosure behaves similar to MethodClosure.
            Instances::ThunkFunction& f = func;
            
            Value result;
            
            f.Execute(_this, result, argc, argv);
            
            // C++. We need to handle exceptions ...
            if (IsException())
                return;
            
            if (!discard_result)
                OpStack.PushBack(result);
        }
        break;
    case Value::kMethodClosure:
        {
            // The [[Call]] property is invoked on the value of the resolved property with the arguments 

            SF_ASSERT(func.GetFunct());
            func.GetFunct()->Execute(Value(func.GetClosure()), argc, argv, discard_result);
            // Bytecode. Exception will be handled by bytecode.
        }
        break;
    case Value::kThunkClosure:
        {
            const ThunkInfo& thunk = func.GetThunkFunct();
            Value result;

            // Check number of arguments.
            if (argc > thunk.GetMaxArgNum() || argc < thunk.GetMinArgNum())
                return ThrowArgumentError(VM::eWrongArgumentCountError, argc, thunk.GetMinArgNum(), thunk.GetMaxArgNum());

            // Hack for Function::apply() and Function::call()
            if (_this.IsClosure() && argc > 0)
            {
                if (&thunk == &InstanceTraits::Function::GetApplyThunkInfo() ||
                    &thunk == &InstanceTraits::Function::GetCallThunkInfo())
                    const_cast<Value&>(argv[0]) = _this.GetClosure();
            }

            (*thunk.Method)(*this, Value(func.GetClosure()), result, argc, argv);

            // C++. We need to handle exceptions ...
            if (IsException())
                return;

            if (!discard_result)
                OpStack.PushBack(result);
        }
        break;
        /* DO NOT delete this code
     case Value::kMethodInd:
         {
             const int method_ind = func.GetMethodInd();
             VMAbcFile& file = func.GetAbcFile();
             const Abc::MethodBodyInfo& mbi = file.GetMethodBodyInfo(method_ind);
             const Abc::MethodInfo& mi = file.GetMethods().Get(method_ind);
             
             AddFrame(
                 _this,
                 file,
                 mi,
                 mbi,
                 NULL, // No traits available.
                 (_this.IsNullOrUndefined() ? GetGlobalObject() : _this),
                 &GetScopeStack(), // This is not correct any more.
                 argc,
                 argv,
                 discard_result,
                 NULL // !!! This is definetely not correct. We should have real Traits here.
                 DEBUG_ARG(GetStringManager().CreateConstString("method id: ") + Scaleform::AsString(method_ind))
                 );
             // Bytecode. Exception will be handled by bytecode.
         }
         break;
         */
    default:
        //SF_ASSERT2(false, "Value is not a Function.");
        ThrowTypeError(VM::eCallOfNonFunctionError DEBUG_ARG("Value is not a Function"));
        break;
    }
}

void VM::Execute(const Value& func, const Value& _this, Value& result, unsigned argc, const Value* argv)
{
    switch(func.GetKind())
    {
    case Value::kFunction:
        // The [[Call]] property is invoked on the value of the resolved property with the arguments 
        func.AsFunction().Execute(_this, result, argc, argv);
        break;
    case Value::kClass:
    case Value::kObject:
        // It doesn't look like a function ...
        // Just call a "Call" method ...
        SF_ASSERT(func.GetObject());
        func.GetObject()->Call(_this, result, argc, argv);
        break;
    case Value::kThunk:
        {
            const ThunkInfo& thunk = func.AsThunk();

            // Check number of arguments.
            if (argc > thunk.GetMaxArgNum() || argc < thunk.GetMinArgNum())
                return ThrowArgumentError(VM::eWrongArgumentCountError, argc, thunk.GetMinArgNum(), thunk.GetMaxArgNum());

            // DO NOT delete this code.
            /* It causes recursive calls in case of Number.
            // Check type of *this*.
            Value coerced_this;
            if (_this.IsClass())
                coerced_this = _this;
            else
            {
                const Class* cl = Resolve2Class(thunk);
                SF_ASSERT(cl);
                if (!cl->Coerce(_this, coerced_this, GetStringManager()))
                    return ThrowTypeError(VM::eNotImplementedYet);
            }

            // Run method.
            (*thunk.Method)(*this, coerced_this, result, argc, argv);
            */

            // Hack for Function::apply() and Function::call()
            if (_this.IsClosure() && argc > 0)
            {
                if (&thunk == &InstanceTraits::Function::GetApplyThunkInfo() ||
                    &thunk == &InstanceTraits::Function::GetCallThunkInfo())
                    const_cast<Value&>(argv[0]) = _this.GetClosure();

                // Run method.
                (*thunk.Method)(*this, Value(_this.GetObject()), result, argc, argv);
            }
            else
                // Run method.
                (*thunk.Method)(*this, _this, result, argc, argv);
        }
        break;
    case Value::kThunkFunction:
        func.AsThunkFunction().Execute(_this, result, argc, argv);
        break;
    case Value::kMethodClosure:
        // The [[Call]] property is invoked on the value of the resolved property with the arguments 

        SF_ASSERT(func.GetFunct());
        func.GetFunct()->Execute(Value(func.GetClosure()), result, argc, argv);
        break;
    case Value::kThunkClosure:
        {
            const ThunkInfo& thunk = func.GetThunkFunct();

            // Check number of arguments.
            if (argc > thunk.GetMaxArgNum() || argc < thunk.GetMinArgNum())
                return ThrowArgumentError(VM::eWrongArgumentCountError, argc, thunk.GetMinArgNum(), thunk.GetMaxArgNum());

            // Hack for Function::apply() and Function::call()
            if (_this.IsClosure() && argc > 0)
            {
                if (&thunk == &InstanceTraits::Function::GetApplyThunkInfo() ||
                    &thunk == &InstanceTraits::Function::GetCallThunkInfo())
                    const_cast<Value&>(argv[0]) = _this.GetClosure();
            }

            (*thunk.Method)(*this, Value(func.GetClosure()), result, argc, argv);
        }
        break;
        /* DO NOT delete this code
     case Value::kMethodInd:
         {
             const int method_ind = func.GetMethodInd();
             VMAbcFile& file = func.GetAbcFile();
             const Abc::MethodBodyInfo& mbi = file.GetMethodBodyInfo(method_ind);
             const Abc::MethodInfo& mi = file.GetMethods().Get(method_ind);
             
             AddFrame(
                 _this,
                 file,
                 mi,
                 mbi,
                 NULL, // No traits available.
                 (_this.IsNullOrUndefined() ? GetGlobalObject() : _this),
                 &GetScopeStack(), // This is not correct any more.
                 argc,
                 argv,
                 false, // discard_result ...
                 NULL // !!! This is definitely not correct. We should have real Traits here.
                 DEBUG_ARG(GetStringManager().CreateConstString("method id: ") + Scaleform::AsString(method_ind))
                 );
                 
             // AddFrame can throw exceptions.
             if (IsException())
                 return;

             ExecuteCode();
             
             if (IsException())
                 return;

             RetrieveResult(result);
         }
         break;
         */
    default:
        //SF_ASSERT2(false, "Value is not a Function.");
        ThrowTypeError(VM::eCallOfNonFunctionError DEBUG_ARG("Value is not a Function"));
        break;
    }
}

void VM::Execute_IgnoreException(const Value& func, const Value& _this,
                                 Value& result, unsigned argc, const Value* argv)
{
    Execute(func, _this, result, argc, argv);
    if (IsException())    
        OutputAndIgnoreException();
}


void VM::Coerce2ReturnType(const Value& value, Value& result)
{
    // Resolve multiname ...
    const CallFrame& cf = GetCurrCallFrame();
    const Abc::Multiname& mn = cf.GetReturnType();
    const ClassTraits::Traits* to = Resolve2ClassTraits(cf.GetFile(), mn);

    if (to)
    {
        if (!to->Coerce(value, result))
        {
            // A TypeError is thrown if return_value cannot be coerced to the expected return type of the
            // executing method.
            return ThrowTypeError(VM::eCheckTypeFailedError DEBUG_ARG("Value cannot be coerced to the specified type"));
        }
    } else
    {
        //SF_ASSERT2(false, "Couldn't resolve multiname " + AsString(mn));
        return ThrowReferenceError(VM::eNotImplementedYet DEBUG_ARG("Couldn't resolve multiname."));
    }
}

ClassTraits::Traits* VM::GetClassTraits(const ASString& name, const Instances::Namespace& ns)
{
    TClassTraitsCache::ValueType* ptr = ClassTraitsSet.Get(name, ns);

    if (ptr == NULL)
        return NULL;

    return *ptr;
}

ClassTraits::UserDefined& VM::GetUserDefinedTraits(VMAbcFile& file, const Abc::ClassInfo& ci)
{
    const Abc::Multiname& name = ci.GetName(file.GetConstPool());
    const ASString t_name = file.GetInternedString(name.GetNameInd());
    const Instances::Namespace& t_ns = file.GetInternedNamespace(name.GetNamespaceInd());

    TClassTraitsCache::ValueType* ptr = ClassTraitsSet.Get(t_name, t_ns);

    if (ptr == NULL)
    {
        // check for *scaleform extension* packages.
        const ASString& t_uri = t_ns.GetUri();
        const StringDataPtr t_str(t_uri.ToCStr(), t_uri.GetSize());
        const StringDataPtr scaleform_gfx("scaleform.gfx");

        if (t_str.GetSize() >= scaleform_gfx.GetSize() && SFstrncmp(t_str.ToCStr(), scaleform_gfx.ToCStr(), scaleform_gfx.GetSize()) == 0)
        {
            // This is *scaleform.gfx extension* package.
            return const_cast<ClassTraits::UserDefined&>(
                static_cast<const ClassTraits::UserDefined&>(
                    *Resolve2ClassTraits(file, name)
                    )
                );
        }

        SF_ASSERT(false);
    }

    return static_cast<ClassTraits::UserDefined&>(*ptr->Get());
}

InstanceTraits::Function& VM::GetFunctionInstanceTraits(VMAbcFile& file, const Abc::MethodBodyInfo& mbi)
{
    if (mbi.GetTraitsCount())
    {
        TFunctionTraitsCache::ValueType* ptr = FunctionTraitsCache.Get(&mbi);

        if (ptr == NULL)
        {
            Pickable<InstanceTraits::Function> t(SF_HEAP_NEW(GetMemoryHeap()) InstanceTraits::Function(&file, GetSelf(), GetClassFunction(), mbi));
            FunctionTraitsCache.Add(&mbi, t);

            return *t;
        }

        return **ptr;
    }

    return *NoFunctionTraits;
}

InstanceTraits::Activation& VM::GetActivationInstanceTraits(VMAbcFile& file, const Abc::MethodBodyInfo& mbi DEBUG_ARG(const ASString& name))
{
    if (mbi.GetTraitsCount())
    {
        TActivationTraitsCache::ValueType* ptr = ActivationTraitsCache.Get(&mbi);

        if (ptr == NULL)
        {
            Pickable<InstanceTraits::Activation> t(SF_HEAP_NEW(GetMemoryHeap()) InstanceTraits::Activation(file, GetSelf(), GetClassActivation(), mbi DEBUG_ARG(name)));
            ActivationTraitsCache.Add(&mbi, t);

            return *t;
        }

        return **ptr;
    }

    return *NoActivationTraits;
}

void VM::DropCallFrame()
{
#ifdef SF_AMP_SERVER
    if (GetAdvanceStats() != NULL)
    {
        CallFrame& frame = CallStack.Back();
        GetAdvanceStats()->PopCallstack(frame.GetFile().GetAbcFile().GetSwdHandle(), frame.GetMethodBodyInfo().GetMethodInfoInd(), Timer::GetRawTicks() - frame.GetStartTicks());
    }
#endif

    // Restore stack in case of exception.
    if (IsException())
        CallStack.Back().ClearOpStack();
    /* DO NOT delete this code. This was original behavior.
    else if (CallStack.Back().NeedToDiscardResult())
    OpStack.PopBack();
    */

    CallStack.PopBack();
}

ASString VM::FormatErrorMessage(Error id, const char* msg)
{
    String fullMsg;
    if (msg)
        Format(fullMsg, "Error #{0}: {1}", (int)id, msg);
    else
        Format(fullMsg, "Error #{0}", (int)id);    
    return GetStringManager().CreateString(fullMsg);
}
ASString VM::FormatErrorMessage(Error id, const ASString& msg)
{
    return FormatErrorMessage(id, msg.ToCStr());
}

void VM::OutputError(const Value& e)
{
    Value r;
    StringManager& sm = GetStringManager();
    ASString errorString = sm.CreateEmptyString();

    const Multiname prop_name(GetPublicNamespace(), GetStringManager().CreateConstString("getStackTrace"));
    if (!e.IsNullOrUndefined() && AS3::ExecuteProperty(*this, prop_name, e, r, 0, NULL))
    {
        if (!r.Convert2String(errorString, sm))
            return;
    }

    if (e.IsNullOrUndefined() || errorString == GetStringManager().GetBuiltin(AS3Builtin_null))
    {
        // It looks like there is no getStackTrace() method.
        if (!e.Convert2String(errorString, sm))
            return;
    }

    UI.Output(FlashUI::Output_Error, errorString.ToCStr());
    UI.Output(FlashUI::Output_Error, "\n");
}


void VM::HandleNotImplemented(const char* msg, bool forceThrow)
{
    SF_UNUSED2(msg, forceThrow);
    // Ideally, we would just like a an output in the Flash player:
    //UI.Output(FlashUI::Output_Warning,
    //    String("Warning: ") + String(msg) + " is not implemented yet\n");

    ThrowError(VM::eNotImplementedYet
         DEBUG_ARG(GetStringManager().CreateString(msg) + " is not implemented yet"));
}


Pickable<Instances::Namespace> VM::MakeNamespace(Abc::NamespaceKind kind,
                                        const ASString& uri, const Value& prefix) const
{
    SF_UNUSED(uri);
    return static_cast<InstanceTraits::Namespace&>(GetClassTraitsNamespace().GetInstanceTraits()).MakeInstance(kind, uri, prefix);
}

Pickable<Instances::Namespace> VM::MakeInternedNamespace(Abc::NamespaceKind kind,
                                               const ASString& uri) const
{
    if (uri.IsEmpty() && kind == Abc::NS_Public)
        return Pickable<Instances::Namespace>(&GetPublicNamespace(), PickValue);

    InstanceTraits::Namespace& nsInstanceTraits = static_cast<InstanceTraits::Namespace&>(GetClassTraitsNamespace().GetInstanceTraits());
    return nsInstanceTraits.MakeInternedInstance(kind, uri, Value::GetUndefined());
}

Pickable<Instances::Namespace> VM::MakeInternedNamespace(Abc::NamespaceKind kind,
                                               const char* name) const
{
    if (!name) name = "";
    return MakeInternedNamespace(kind, GetStringManager().CreateString(name));    
}

Instances::Namespace& VM::GetInternedNamespace(Abc::NamespaceKind kind,
                                               const ASString& uri) const
{
    if (uri.IsEmpty() && kind == Abc::NS_Public)
        return GetPublicNamespace();

    InstanceTraits::Namespace& nsInstanceTraits = static_cast<InstanceTraits::Namespace&>(GetClassTraitsNamespace().GetInstanceTraits());
    return nsInstanceTraits.GetInternedInstance(kind, uri, Value::GetUndefined());
}

Instances::Namespace& VM::GetInternedNamespace(Abc::NamespaceKind kind,
                                               const char* name) const
{
    if (!name) name = "";
    return GetInternedNamespace(kind, GetStringManager().CreateString(name));    
}

CheckResult VM::LoadFile(const Abc::File& file, bool to_execute)
{
    // We need pCurrentFile to be able to initialize InstanceTraits.
    SPtr<VMAbcFile> vmfile(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) VMAbcFile(*this, file)));

    // Register file.
    Files.Add(GetASString(file.GetName()), vmfile);

    // Register class traits.
    if (!RegisterUserDefinedClassTraits(*vmfile))
        return false;

    if (IsException())
        return false;

    // Register scripts.
    const UPInt n = file.GetScripts().GetSize();
    for (UPInt i = 0; i < n; ++i)
    {
        const Abc::ScriptInfo& script = file.GetScripts().Get(i);
        bool load = true;

        if (!load)
            continue;

        SPtr<InstanceTraits::GlobalObjectScript> it;
        
        it.Pick(SF_HEAP_NEW(GetMemoryHeap()) InstanceTraits::GlobalObjectScript(
                *vmfile,
                GetSelf(), 
                GetClassObject(), 
                script
            ));

        Pickable<Instances::GlobalObjectScript> inst(
            SF_HEAP_NEW(GetMemoryHeap()) Instances::GlobalObjectScript(
                *vmfile,
                script,
                *it
                )
            );
        GlobalObjects.PushBack(inst);

        InitUserDefinedClassTraits(*inst);
    }

    // Execute last script if necessary.
    if (to_execute && n > 0)
        static_cast<Instances::GlobalObjectScript*>(GlobalObjects.Back().GetObject())->Execute();

    return true;
}

bool VM::LoadPackage(const ASString& name)
{
    const Abc::File* file = Loader.GetFile(name);

    return file && LoadFile(*file);
}

Class* VM::GetClass(const char* gname)
{
    // Create a parsed multiname
    const Multiname mn(*this, gname);
    Class* _class = NULL;

    const ClassTraits::Traits* ctr = Resolve2ClassTraits(mn);
    if (ctr && ctr->IsValid())
    {
        InstanceTraits::Traits& itr = ctr->GetInstanceTraits();

        if (itr.HasConstructorSetup())
            _class = &itr.GetClass();
    }

    return _class;
}

bool VM::GetClassUnsafe(const char* gname, Value& result)
{
    // Create a parsed multiname
    const Multiname mn(*this, gname);
    const PropRef obj_prop = FindScopeProperty(*this, 0, GetGlobalObjects(), mn);

    if (IsException())
        return false;

    if (!obj_prop)
    {
        ThrowReferenceError(VM::eNotImplementedYet DEBUG_ARG("Object not found"));
        return false;
    }

    // Get property value.
    if (!obj_prop.GetSlotValueUnsafe(*this, result))
        return false;

    return true;
}

bool VM::Construct(const char* gname, Value& result, unsigned argc, const Value* argv, bool extCall)
{
    const UPInt original_stack_size = CallStack.GetSize();

    // Get class.
    Value value;
    // !!! GetClass() MAY NOT be used here.
    if (!GetClassUnsafe(gname, value))
        return false;

    // TODO:
    // !!! Look up for a "Construct" property ...
    //A TypeError is thrown if object does not implement the [[Construct]] property.

    // Create instance.
#ifdef CONSTRUCT_IN_TRAITS
    // GetInstanceTraits() can malfunction here.
    GetValueTraits(value).Construct(result, argc, argv, extCall);
#else
    if (value.IsNullOrUndefined())
    {
        ThrowTypeError(VM::eConvertNullToObjectError);
        return false;
    }

    value.GetObject()->Construct(result, argc, argv, extCall);
#endif

    if (IsException())
        return false;

    return CallStack.GetSize() > original_stack_size;
}

// Private helper to ConstructBuiltinObject
CheckResult VM::constructBuiltinObject(SPtr<Object> &pobj, const char* gname,
                                unsigned argc, const Value* argv)
{
    Value v;
    Construct(gname, v, argc, argv);

    if (!IsException() && !v.IsNullOrUndefined())
    {
        pobj = v.GetObject();
        return true;
    }        
    pobj = 0;
    if (IsException())
        IgnoreException();
    return false;
}

Traits& VM::GetValueTraits(const Value& v) const
{
    switch (v.GetKind())
    {
    case Value::kUndefined: // = 0
        break;
    case Value::kBoolean:   // = 1
        return GetClassTraitsBoolean().GetInstanceTraits();
    case Value::kInt:       // = 2
        return GetClassTraitsInt().GetInstanceTraits();
    case Value::kUInt:      // = 3
        return GetClassTraitsUInt().GetInstanceTraits();
    case Value::kNumber:    // = 4
        return GetClassTraitsNumber().GetInstanceTraits();
    case Value::kString:    // = 5
        return GetClassTraitsString().GetInstanceTraits();
    case Value::kNamespace: // = 10
        return GetClassTraitsNamespace().GetInstanceTraits();
    case Value::kFunction:  // = 6
    case Value::kObject:    // = 8
    case Value::kClass:     // = 9
    case Value::kMethodClosure:  // = 14
    // ThunkFunction is a real Object. It just wraps a Thunk.
    case Value::kThunkFunction: // = 12
        if (v.IsNull())
            return GetClassTraitsObject().GetInstanceTraits();
        else
            return v.GetObject()->GetTraits();
    case Value::kThunk:     // = 11
    case Value::kThunkClosure:  // = 15
        return GetClassTraitsFunction().GetThunkTraits();
    case Value::kMethodInd:  // = 13
        break;
    case Value::kInstanceTraits:
        return v.GetInstanceTraits();
    case Value::kClassTraits:
        return v.GetClassTraits();
    }

    SF_ASSERT(false);
    return GetClassTraitsObject().GetInstanceTraits();
}

const ClassTraits::Traits& VM::GetClassTraits(const Value& v) const
{
    switch(v.GetKind())
    {
    case Value::kBoolean:
        return GetClassTraitsBoolean();
    case Value::kInt:
        return GetClassTraitsInt();
    case Value::kUInt:
        return GetClassTraitsUInt();
    case Value::kNumber:
        return GetClassTraitsNumber();
    case Value::kString:
        return GetClassTraitsString();
    case Value::kClass:
        return v.AsClass().GetClassTraits();
    case Value::kNamespace:
        return GetClassTraitsNamespace();
    case Value::kThunk:
    case Value::kThunkFunction:
    case Value::kFunction:
    case Value::kMethodInd:
    case Value::kMethodClosure:
    case Value::kThunkClosure:
        // This should work in case of null.
        return *TraitsFunction;
    case Value::kClassTraits:
        return v.GetClassTraits();
    default:
        break;
    }

    if (v.GetObject())
        return v.GetObject()->GetClass().GetClassTraits();
    else
        // Return ClassTraits::Object for null.
        return GetClassTraitsObject();
}

InstanceTraits::Traits& VM::GetInstanceTraits(const Value& v) const
{
    switch (v.GetKind())
    {
    case Value::kUndefined: // = 0
        break;
    case Value::kBoolean:   // = 1
        return GetClassTraitsBoolean().GetInstanceTraits();
    case Value::kInt:       // = 2
        return GetClassTraitsInt().GetInstanceTraits();
    case Value::kUInt:      // = 3
        return GetClassTraitsUInt().GetInstanceTraits();
    case Value::kNumber:    // = 4
        return GetClassTraitsNumber().GetInstanceTraits();
    case Value::kString:    // = 5
        return GetClassTraitsString().GetInstanceTraits();
    case Value::kNamespace: // = 10
        return GetClassTraitsNamespace().GetInstanceTraits();
    case Value::kFunction:  // = 6
    case Value::kObject:    // = 8
    case Value::kMethodClosure:  // = 14
        // ThunkFunction is a real Object. It just wraps a Thunk.
    case Value::kThunkFunction: // = 12
        if (v.IsNull())
            return GetClassTraitsObject().GetInstanceTraits();
        else
            return static_cast<InstanceTraits::Traits&>(v.GetObject()->GetTraits());
    case Value::kClass:     // = 9
        if (v.IsNull())
            // ??? Object?
            return GetClassTraitsObject().GetInstanceTraits();
        else
            return v.AsClass().GetClassTraits().GetInstanceTraits();
    case Value::kThunk:     // = 11
    case Value::kThunkClosure:  // = 15
        return GetClassTraitsFunction().GetThunkTraits();
    case Value::kMethodInd:  // = 13
        break;
    case Value::kInstanceTraits:
        return v.GetInstanceTraits();
    }

    SF_ASSERT(false);
    return GetClassTraitsObject().GetInstanceTraits();
}

const Value& VM::GetGlobalObject() const
{
    // Version 1.
    // Not correct. It should be a script global object.
    //OpStack.PushBack(GetGlobalObjectCPP().Get());

    // Version 2.
    // Not correct.
    //OpStack.PushBack(OpStack[0]);

    // Version 3.
    // Not correct. "this" is not a Global Object.
    // Found object should always be a dynamic object.
    // And "this" can be a Sealed class.
    //OpStack.PushBack(GetRegister(AbsoluteIndex(0)));

    // Version 4.
//         {
//             if (CallStack.GetSize() > 0)
//             {
//                 OpStack.PushBack(CallStack[0].GetRegister(0));
//             }
// 
//             // ???
//             // Last chance.
//             OpStack.PushBack(GetRegister(AbsoluteIndex(0)));
//         }

    // Version 5.
    // It is not correct to take object from the top of the Scope Stack.
    // Found object should always be a dynamic object.
//         {
//             if (CallStack.GetSize() > 0)
//             {
//                 const CallFrame& cf = CallStack.Back();
// 
//                 if (cf.ScopeStack.GetSize() > 0)
//                 {
//                     OpStack.PushBack(cf.ScopeStack[0]);
//                     return;
//                 }
//             }
// 
//             OpStack.PushBack(GetRegister(AbsoluteIndex(0)));
//         }

    // Version 6.
    // This works correctly so far.
//     {
//         if (CallStack.GetSize() > 0)
//         {
//             const CallFrame& cf = CallStack[0];
// 
//             if (cf.ScopeStack.GetSize() > 0)
//                 return cf.ScopeStack[0];
//         }
// 
//         return GetRegister(AbsoluteIndex(0));
//     }

    // Version 7.
//     {
//         const ScopeStackType& ss = GetValueTraits(GetRegister(AbsoluteIndex(0))).GetStoredScopeStack();
// 
//         SF_ASSERT(!ss.IsEmpty());
//         const Value go = ss[0];
// 
//         if (go.IsClass())
//             // Because we do not put it there explicitly.
//             return GetGlobalObjectCPPValue();
// 
//         return ss[0];
//     }

    // Version 8.
    {
        if (GetCallStack().GetSize() != 0)
        {
            const ScopeStackType* sss = GetCurrCallFrame().GetSavedScope();

            if (sss && !sss->IsEmpty())
                return sss->At(0);

            return GetRegister(AbsoluteIndex(0));
        }
        
        // Do we have a better solution?
        return GetGlobalObjectCPPValue();
    }
}

Value VM::GetDefaultValue(const ClassTraits::Traits& ctr) const
{
    if (IsClassClass(ctr))
        return Value::GetUndefined();
    else if (IsBoolean(ctr))
        return Value(false);
    else if (IsInt(ctr))
        return Value(SInt32(0));
    else if (IsUint(ctr))
        return Value(UInt32(0));
    else if (IsNumber(ctr))
        return Value(NumberUtil::NaN());

    return Value::GetNull();
}

Value VM::GetDefaultValue(VMAbcFile& file, const Abc::Multiname& mn)
{
    if (&mn != &file.GetConstPool().GetAnyType())
    {
        // AnyType gets converted to *undefined*.
        const ClassTraits::Traits* ctr = Resolve2ClassTraits(file, mn);

        if (ctr)
            return GetDefaultValue(*ctr);
    }

    return Value::GetUndefined();
}

bool VM::CheckObject(const Value& v)
{
    bool result = true;

    if (v.IsNull())
    {
        // A TypeError is thrown if obj is null or undefined.
        ThrowTypeError(VM::eConvertNullToObjectError DEBUG_ARG("Cannot access a property or method of a null object reference"));
        result = false;
    }
    else if (v.IsUndefined())
    {
        ThrowTypeError(VM::eConvertUndefinedToObjectError DEBUG_ARG("A term is undefined and has no properties"));
        result = false;
    }

    return result;
}

void VM::GetStackTraceASString(ASString& result, const char* line_pref)
{
#ifdef GFX_AS3_VERBOSE
    const CallStackType& cs = GetCallStack();

    for (UPInt i = cs.GetSize(), j = 0; i > 0; --i, ++j)
    {
        const CallFrame& cf = cs[i - 1];

        if (j > 0)
            result += "\n";

        result += line_pref;
        result += "at ";
        result += ASString(cf.GetName());
        result += "()";
        if (cf.GetCurrFileInd() != 0)
        {
            result += "[";
            result += cf.GetCurrFileName();
            result += ":";
            result += Scaleform::AsString(cf.GetCurrLineNumber());
            result += "]";
        }
    }
#else
    SF_UNUSED(line_pref);
    result = GetStringManager().GetBuiltin(AS3Builtin_null);
#endif
}

#ifdef SF_AMP_SERVER
void VM::SetActiveLine(UInt64 fileId, UInt32 lineNumber)
{
    AMP::ViewStats* stats = GetAdvanceStats();
    if (stats != NULL)
    {
        UInt64 activeFile;
        UInt32 activeLine;
        stats->GetActiveLine(&activeFile, &activeLine);

        if (fileId != activeFile || lineNumber != activeLine)
        {
            // Record time
            UInt64 newTimestamp = Timer::GetRawTicks();
            if (activeLine != 0)
            {
                stats->RecordSourceLineTime(newTimestamp - ActiveLineTimestamp);
            }
            ActiveLineTimestamp = newTimestamp;

            stats->SetActiveLine(fileId, lineNumber);
        }    
    }
}
#endif


Classes::Boolean& VM::GetClassBoolean() const
{
    SF_ASSERT(TraitsBoolean.Get());
    return (Classes::Boolean&)TraitsBoolean->GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsBoolean() const
{
    return GetClassTraitsBoolean().GetInstanceTraits();
}

Classes::Number& VM::GetClassNumber() const
{
    SF_ASSERT(TraitsNumber.Get());
    return (Classes::Number&)TraitsNumber->GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsNumber() const
{
    return GetClassTraitsNumber().GetInstanceTraits();
}

Classes::int_& VM::GetClassInt() const
{
    return (Classes::int_&)GetClassTraitsInt().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsSInt() const
{
    return GetClassTraitsInt().GetInstanceTraits();
}

Classes::uint& VM::GetClassUInt() const
{
    return (Classes::uint&)GetClassTraitsUInt().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsUInt() const
{
    return GetClassTraitsUInt().GetInstanceTraits();
}

Classes::String& VM::GetClassString() const
{
    return (Classes::String&)GetClassTraitsString().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsString() const
{
    return GetClassTraitsString().GetInstanceTraits();
}

Classes::Array& VM::GetClassArray() const
{
    return (Classes::Array&)GetClassTraitsArray().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsArray() const
{
    return GetClassTraitsArray().GetInstanceTraits();
}

Classes::Vector& VM::GetClassVector() const
{
    return (Classes::Vector&)GetClassTraitsVector().GetInstanceTraits().GetClass();
}

Classes::Vector_int& VM::GetClassVectorSInt() const
{
    return (Classes::Vector_int&)GetClassTraitsVectorSInt().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsVectorSInt() const
{
    return GetClassTraitsVectorSInt().GetInstanceTraits();
}

Classes::Vector_uint& VM::GetClassVectorUInt() const
{
    return (Classes::Vector_uint&)GetClassTraitsVectorUInt().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsVectorUInt() const
{
    return GetClassTraitsVectorUInt().GetInstanceTraits();
}

Classes::Vector_double& VM::GetClassVectorNumber() const
{
    return (Classes::Vector_double&)GetClassTraitsVectorNumber().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsVectorNumber() const
{
    return GetClassTraitsVectorNumber().GetInstanceTraits();
}

Classes::Vector_String& VM::GetClassVectorString() const
{
    return (Classes::Vector_String&)GetClassTraitsVectorString().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsVectorString() const
{
    return GetClassTraitsVectorString().GetInstanceTraits();
}

Classes::Catch& VM::GetClassCatch() const
{
    SF_ASSERT(TraitsCatch.Get());
    return (Classes::Catch&)TraitsCatch->GetInstanceTraits().GetClass();
}

///////////////////////////////////////////////////////////////////////////
// We keep implementation here because of Array.
CheckResult SetProperty(VM& vm, const Value& _this, const Multiname& prop_name, const Value& value)
{
    /* For maniacs
    if (_this.IsNullOrUndefined())
    {
        vm.ThrowTypeError(VM::eConvertUndefinedToObjectError DEBUG_ARG("A term is undefined and has no properties"));
        return false;
    }
    */

    const Traits& tr = vm.GetValueTraits(_this);
    if (tr.IsArrayLike())
        return _this.GetObject()->SetProperty(prop_name, value);

    // Find a property.
    // Find + Set shouldn't mutate a prototype object.
    // Search in own properties only. Ignore prototype.
    PropRef prop = AS3::FindObjProperty(vm, _this, prop_name, FindSet);
    if (prop)
        return prop.SetSlotValue(vm, value);
    else if (_this.IsObject())
    {
        Object& obj = *_this.GetObject();

        // Dynamic properties can only be assigned to public namespace.
        // TBD: Add Multiname::AsValidDynamicName()?. Empty names shouldn't be allowed...
        if (obj.IsDynamic() &&
            prop_name.ContainsNamespace(vm.GetPublicNamespace()))
        {
            obj.AddDynamicSlotValuePair(prop_name, value);
            return true;
        }
    }

    // A ReferenceError is thrown if the property is unresolved and obj is not dynamic.
    vm.ThrowReferenceError(
        VM::eWriteSealedError 
        DEBUG_ARG("Property is unresolved and object is not dynamic"));

    return false;
}

///////////////////////////////////////////////////////////////////////////
CheckResult GetProperty(VM& vm, const Value& _this, const Multiname& prop_name, Value& value)
{
    /* For maniacs
    if (_this.IsNullOrUndefined())
    {
        vm.ThrowTypeError(VM::eConvertUndefinedToObjectError DEBUG_ARG("A term is undefined and has no properties"));
        return false;
    }
    */

    const Traits& tr = vm.GetValueTraits(_this);
    if (tr.IsArrayLike())
        return _this.GetObject()->GetProperty(prop_name, value);

    // Find a property.
    PropRef prop = FindObjProperty(vm, _this, prop_name);
    if (prop)
        return prop.GetSlotValue(vm, value);

    return false;
}
CheckResult GetPropertyUnsafe(VM& vm, const Value& _this, const Multiname& prop_name, Value& value)
{
    /* For maniacs
    if (_this.IsNullOrUndefined())
    {
        vm.ThrowTypeError(VM::eConvertUndefinedToObjectError DEBUG_ARG("A term is undefined and has no properties"));
        return false;
    }
    */

    const Traits& tr = vm.GetValueTraits(_this);
    if (tr.IsArrayLike())
        return _this.GetObject()->GetProperty(prop_name, value);

    // Find a property.
    PropRef prop = FindObjProperty(vm, _this, prop_name);
    if (prop)
        return prop.GetSlotValueUnsafe(vm, value);

    return false;
}

///////////////////////////////////////////////////////////////////////////
// Keep it here for the time being.

CheckResult Object::SetProperty(const Multiname& prop_name, const Value& value)
{
    // Find a property.

    VM& vm = GetVM();

    // Find + Set shouldn't mutate a prototype object.
    PropRef prop = AS3::FindObjProperty(GetVM(), Value(this), prop_name, FindSet);
    if (prop)
        return prop.SetSlotValue(vm, value);
    else
    {
        Object& obj = *this;
        
        // Dynamic properties can only be assigned to public namespace.
        // TBD: Add Multiname::AsValidDynamicName()?. Empty names shouldn't be allowed...
        if (obj.IsDynamic() &&
            prop_name.ContainsNamespace(vm.GetPublicNamespace()))
        {
            obj.AddDynamicSlotValuePair(prop_name, value);
            return true;
        }
    }

    // A ReferenceError is thrown if the property is unresolved and obj is not dynamic.
    vm.ThrowReferenceError(
        VM::eWriteSealedError 
        DEBUG_ARG("Property is unresolved and object is not dynamic"));

    return false;
}

CheckResult Object::GetProperty(const Multiname& prop_name, Value& value)
{
    VM& vm = GetVM();

    // Find a property.
    PropRef prop = AS3::FindObjProperty(vm, Value(this), prop_name);
    if (prop)
        return prop.GetSlotValue(vm, value);

    return false;
}

CheckResult Object::DeleteProperty(const Multiname& prop_name)
{
    if (IsDynamic())
        return DeleteDynamicSlotValuePair(prop_name);

    return false;
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

