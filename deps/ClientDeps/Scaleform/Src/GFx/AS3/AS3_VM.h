/**********************************************************************

Filename    :   AS3_VM.h
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

#ifndef INC_AS3_VM_H
#define INC_AS3_VM_H


#include "Obj/AS3_Obj_Function.h"
#include "Obj/AS3_Obj_Activation.h"
#include "Obj/AS3_Obj_Namespace.h" // Because of MakePublickNamespace().
#include "Obj/AS3_Obj_ClassClass.h"

#include "AS3_FlashUI.h" // for UI ...

#include "AS3_StringManager.h"
#include "GFx/AMP/Amp_ViewStats.h"
#include "GFx/AMP/Amp_Server.h"
#include <math.h>

namespace Scaleform { namespace GFx { namespace AMP { class ViewStats; } } }

///////////////////////////////////////////////////////////////////////////////
namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
// Forward declaration.
namespace Classes
{
    class Boolean;
    class Number;
    class int_;
    class uint;
    class String;
    class Array;
    class Vector;
    class Vector_int;
    class Vector_uint;
    class Vector_double;
    class Vector_String;
    class Vector_object;
    class Catch;
    class Error;
    class Date;
    class Math;
    class RegExp;
    class QName;
    class XML;
    class XMLList;
}

namespace ClassTraits 
{
    class UserDefined;
    class Boolean;
    class Number;
    class int_;
    class uint;
    class String;
    class Array;
    class Vector;
    class Vector_int;
    class Vector_uint;
    class Vector_double;
    class Vector_String;
    class Vector_object;
    class Catch;
}

///////////////////////////////////////////////////////////////////////////

// MultinameHash is used to map {Name, Namespace} -> Traits/Class
// VM should maintain a list of all registered ClassTraits so that it
// can look them up based on multiname for coerce/astype operations.
//
// Currently used by VM::ClassSet, but that should be replaced with traits.

template<class C>
class MultinameHash
{
    struct Key
    {
        ASString                         Name;
        SPtr<const Instances::Namespace> pNs;

        Key(const ASString& name, const Instances::Namespace* ns) : Name(name), pNs(ns) { }
        Key(const Key& other) : Name(other.Name), pNs(other.pNs) { }

        bool operator == (const Key& other) const
        { return (Name == other.Name) && (*pNs == *other.pNs); }
        
        struct HashFunctor
        {
            UPInt operator()(const Key& key) const
            { return key.Name.GetHash() ^ key.pNs->GetHash(); }
        };
    };

    HashLH<Key, C, typename Key::HashFunctor>  Entries;

public:
    typedef C ValueType;
    typedef MultinameHash<C> SelfType;

public:
    ~MultinameHash()
    {
        Clear();
    }

public:
    C*  Get(const ASString& name, const Instances::Namespace& ns)
    {
        return Entries.Get(Key(name, &ns));
    }

    C*  Get(const Multiname& mn)
    {
        if (!mn.IsMultiname())
            return Get(mn.GetName(), mn.GetNamespace());
        
        const ValueArray &nsArray = mn.GetNamespaceSet().GetNamespaces();

        C* p = 0;
        for (UPInt i =0; !p && (i< nsArray.GetSize()); i++)
            p = Get(mn.GetName(), nsArray[i]);
        return p;        
    }

    void Add(const ASString& name, const Instances::Namespace& ns, const C& val)
    {       
        Entries.Add(Key(name, &ns), val);
    }
    void Remove(const ASString& name, const Instances::Namespace& ns)
    {
        Entries.Remove(Key(name, &ns));
    }

    void Clear()
    {
       Entries.Clear();
    }
};

template<class C>
class AbcMultinameHash
{
    struct Key
    {
        SInt32 NsInd;
        SInt32 NextMnInd;

        Key(SInt32 ind, SInt32 next_ind = -1) : NsInd(ind), NextMnInd(next_ind) { }
        Key(const Key& other) : NsInd(other.NsInd), NextMnInd(other.NextMnInd) { }

        bool operator == (const Key& other) const
        { return (NsInd == other.NsInd) && (NextMnInd == other.NextMnInd); }
    };

public:
    typedef C ValueType;
    typedef AbcMultinameHash<C> SelfType;

public:
    ~AbcMultinameHash()
    {
        Clear();
    }

public:
    ///
    C*  Get(SInt32 ind, SInt32 next_ind = -1)
    {
        return Entries.Get(Key(ind, next_ind));
    }
    C*  Get(const Abc::Multiname& mn)
    {
        if (mn.IsQName())
            return Get(mn.GetNamespaceInd(), mn.GetNextMultinameIndex());

        return NULL;
    }

    ///
    void Add(SInt32 nsInd, const C& val)
    {       
        Entries.Add(Key(nsInd), val);
    }
    void Add(const Abc::Multiname& mn, const C& val)
    {       
        Entries.Add(Key(mn.GetNamespaceInd(), mn.GetNextMultinameIndex()), val);
    }

    ///
    void Clear()
    {
        Entries.Clear();
    }

    ///
    void ForEachChild_GC(RefCountBaseGC<Mem_Stat>::GcOp op) const
    {
        typename ContainerType::ConstIterator it = Entries.Begin();
        while (!it.IsEnd())
        {
            const C& v = it->Second;
            // Fr some reason AS3::ForEachChild_GC<C, Mem_Stat>(it->Second, op); doesn't want to compile.
            if (v)
                (*op)(const_cast<const AS3::RefCountBaseGC<Mem_Stat>**>(reinterpret_cast<AS3::RefCountBaseGC<Mem_Stat>**>(v.GetRawPtr())));
            ++it;
        }
    }

private:
    typedef HashLH<Key, C> ContainerType;
    ContainerType Entries;
};

// VMAbcFile wraps the use of Abc::File in the VM and stores extra VM-specific
// data associated with it, such as string pool and {Multiname -> Class} mapping tables.

class VMAbcFile : public GASRefCountBase
{
public:

    VMAbcFile(VM& vm, const Abc::File& file);        
    ~VMAbcFile();

    // Obtains interned Namespace/NamespaceSets for the file.
    // nsIndex is zero-based.
    Instances::Namespace&   GetInternedNamespace(const Abc::Multiname& mn);
    Instances::Namespace&   GetInternedNamespace(UPInt nsIndex);
    NamespaceSet&           GetInternedNamespaceSet(UPInt nsSetIndex);

    // strIndex is zero based.
    ASString                GetInternedString(UPInt strIndex);

    // TBD: Move String pool here, so that it is VM-specific (and
    // so that Abc file can be shared between VMs safely).
    // 

    // TBD: We should store instanceTraits/classes for private typenames
    // of the file here, since they shouldn't be added to the vm-wide
    // multiname type table.

    // Research: MN index -> Class table here? Could allow for
    // faster coerce implementation... but could there be potential lookup conflicts?
    // This may not be worth it, since we'd need a hash-table (there could be
    // way too many multinames in file).


    // Delegates to original file
    VM&                         GetVM() const         { return VMRef; }
    const Abc::File&            GetAbcFile() const    { return File; }
    const Abc::ConstPool&       GetConstPool() const  { return File.GetConstPool(); }
    const Abc::MethodTable&     GetMethods() const    { return File.GetMethods(); }
    const Abc::ClassTable&      GetClasses() const    { return File.GetClasses(); }
    const Abc::TraitTable&      GetTraits() const     { return File.GetTraits(); }
    const Abc::MethodBodyTable& GetMethodBody() const { return File.GetMethodBody(); }
    const Abc::MethodBodyInfo&  GetMethodBodyInfo(int i) const { return File.GetMethodBodyInfo(i); }

private:
    virtual void ForEachChild_GC(GcOp op) const;
    // Avoid warning; no assignment operator.
    VMAbcFile& operator = (const VMAbcFile&);

private:
    VM&              VMRef;
    const Abc::File& File;

    // Interned namespaces and namespace-sets arrays are file-specific.    
    //ArrayLH<SPtr<Instances::Namespace> > IntNamespaces;
    AbcMultinameHash<SPtr<Instances::Namespace> >   IntNamespaces;
    ArrayLH<SPtr<NamespaceSet> >                    IntNamespaceSets;
    ArrayLH<SPtr<ASStringNode> >                    IntStrings;
};


///////////////////////////////////////////////////////////////////////////
typedef ArrayPagedLH<Value> RegistersType;
//typedef ValueArray RegistersType;
//typedef ValueArrayDH RegistersType;
// There is no ArrayPagedDH.
//typedef ArrayPagedDH<Value> RegistersType;

///////////////////////////////////////////////////////////////////////////
class CallFrame
{
    friend class VM;
    
public:
    CallFrame(MemoryHeap* heap);
    CallFrame(
        const Value& invoker, // invoker can be NULL if there is nothing to track.
        VMAbcFile& file,
        const Abc::MethodBodyInfo& _mbi,
        const Abc::HasTraits* _ht,
        const ScopeStackType* _saved_scope,
        Abc::TCodeOffset _offset,
        bool discard_result,
        const Traits* ot // Origination Traits
        DEBUG_ARG(const ASString& name)
        DEBUG_ARG(UInt64 startTicks)
        );
        CallFrame(const CallFrame& other);
    ~CallFrame();

    CallFrame& operator =(const CallFrame& other);

public:
    const Abc::MethodBodyInfo& GetMethodBodyInfo() const
    {
        SF_ASSERT(pMBI);
        return *pMBI;
    }
    const Abc::Multiname& GetReturnType() const
    {
        return GetMethodBodyInfo().GetMethodInfo(GetFile().GetMethods()).
               GetReturnType(GetFile().GetConstPool());
    }
    // Can throw exceptions.
    const Abc::TOpCode& GetCode() const
    {
        return GetMethodBodyInfo().GetOpCode(*this);
    }
    Abc::TCodeOffset GetCodeOffset() const
    {
        return Offset;
    }
    const ScopeStackType* GetSavedScope() const
    {
        return pSavedScope;
    }
    const Abc::HasTraits* GetHasTraits() const
    {
        return pHT;
    }
    // Register's index starts with ZERO ...
    Value& GetRegister(UPInt ind) const
    {
        return pRegisterFile->At(RegBaseInd + ind);
    }
    UPInt GetRegBaseInd() const
    {
        return RegBaseInd;
    }
    UPInt GetScopeStackBaseInd() const
    {
        return ScopeStackBaseInd;
    }
    UPInt GetOriginalStackSize() const
    {
        return OriginalStackSize;
    }
    bool NeedToDiscardResult() const
    {
        return DiscardResult;
    }
    const Traits* GetOriginationTraits() const
    {
        return OriginationTraits;
    }
    VM& GetVM() const
    {
        return GetFile().GetVM();
    }
    VMAbcFile& GetFile() const
    {
        SF_ASSERT(pFile);
        return *pFile;
    }

    DEBUG_CODE(UInt32 GetCurrFileInd() const { return CurrFileInd; } )
    DEBUG_CODE(ASString GetCurrFileName() const { return GetFile().GetInternedString(CurrFileInd); } )
    DEBUG_CODE(UInt32 GetCurrLineNumber() const { return CurrLineNumber; } )

    DEBUG_CODE(ASStringNode* GetName() const { return Name; } )
    DEBUG_CODE(UInt64 GetStartTicks() const { return StartTicks; } )
    DEBUG_CODE(bool IsRegisteredFunction() const { return RegisteredFunction; } )
    
public:
    void ClearOpStack();
    void ClearScopeStack()
    {
        pScopeStack->Resize(ScopeStackBaseInd);
    }
    void AdjustOriginalStackSize()
    {
        AdjustedStack = true;
        ++OriginalStackSize;
    }
    bool IsAdjustedStack() const
    {
        return AdjustedStack;
    }
    
private:
    void SetCodeOffset(Abc::TCodeOffset _offset)
    {
        Offset = _offset;
    }
    void SetRegister(UPInt ind, const Value& value)
    {
        pRegisterFile->At(RegBaseInd + ind) = value;
    }
    void SetRegisterPickUnsafe(UPInt ind, Value& value)
    {
        pRegisterFile->At(RegBaseInd + ind).PickUnsafe(value);
    }

    void SetupRegisters(const Abc::MethodInfo& mi, const Value& _this, unsigned argc, const Value* argv);
    DEBUG_CODE(void SetRegisteredFunction(bool registered) { RegisteredFunction = registered; } )
    
private:
    const bool                  DiscardResult;
    bool                        AdjustedStack;
    bool                        ACopy;
    DEBUG_CODE(bool             RegisteredFunction;)
    // Stack size before execution of this frame. We need to know that in case we need to clear the stack.
    UPInt                       OriginalStackSize;
    UPInt                       RegBaseInd;
    UPInt                       ScopeStackBaseInd;
    Abc::TCodeOffset            Offset;
    DEBUG_CODE(UInt32           CurrFileInd;)
    DEBUG_CODE(UInt32           CurrLineNumber;)
    DEBUG_CODE(UInt64           StartTicks;)
    // We need this member to prevent destruction of invoker, which may happen if, for example, function
    // was created dynamically on stack by the "newfunction" opcode.
    MemoryHeap*                 pHeap;
    VMAbcFile*                  pFile;
    const Abc::MethodBodyInfo*  pMBI;
    const Abc::HasTraits*       pHT;
    // This member shouldn't be const because objects in saved scope may be modified later ...
    const ScopeStackType*       pSavedScope;
    DEBUG_CODE(SPtr<ASStringNode> Name;)
    RegistersType*              pRegisterFile;
    const Traits*               OriginationTraits;
    ScopeStackType*             pScopeStack;
    Value                       Invoker;
}; // class CallFrame

///////////////////////////////////////////////////////////////////////////
template <typename T>
struct Efficient
{
    typedef T type;
};
template <>
struct Efficient<Value>
{
    typedef Value& type;
};

///////////////////////////////////////////////////////////////////////////
class FileLoader
{
public:
    virtual ~FileLoader() {}

public:
//     virtual const Abc::File& GetFile(const ASString& name) = 0;
//     virtual const Abc::File& GetMainFile() = 0;
    virtual UPInt GetSize() const = 0;
    // Last file in the list is the main file to run.
    virtual const Abc::File& GetFile(UPInt n) = 0;
    virtual const Abc::File* GetFile(const ASString& name) = 0;
};

///////////////////////////////////////////////////////////////////////////
namespace Classes
{
    class System;
}

namespace InstanceTraits
{
    class GlobalObject;
    class GlobalObjectScript;
}

namespace TR
{
    class State;
}

///////////////////////////////////////////////////////////////////////////
// int Read(VM& vm, Multiname& obj);

CheckResult GetArrayInd(ASStringNode* sn, UInt32& ind);
CheckResult GetArrayInd(const Multiname& prop_name, UInt32& ind);

CheckResult GetVectorInd(ASStringNode* sn, UInt32& ind);
CheckResult GetVectorInd(const Multiname& prop_name, UInt32& ind);

///////////////////////////////////////////////////////////////////////////
class VM : public NewOverrideBase<Mem_Stat>
{
    friend class VTable; // Because of Execute().
    friend class CallFrame; // Because of Strings& GetStrings().
    friend class Object; // Because of Strings& GetStrings().
    friend class Traits; // Because of different class traits.
    friend class StackReader; // Because of op_stack, e.t.c.
    friend class SlotInfo; // Because of Execute().
    friend class InitializerGuard; // Because of EnterInitializer()/LeaveInitializer()
    friend class Instances::ThunkFunction; // Because of Execute().
    
    friend class Instances::GlobalObjectCPP; // Because it is a part of VM.

    friend class Classes::Function; // Because of GetFunctionInstanceTraits().
    friend class Classes::Activation; // Because of GetActivationInstanceTraits().
    friend class Classes::Object; // Because of different data type classes.
    friend class Classes::System; // Because of the Garbage Collector.
    friend class Classes::Vector; // Because of GetGlobalObjectCPP()

    friend class InstanceTraits::UserDefined; // Because of TraitsHistory.
    friend class InstanceTraits::GlobalObjectScript; // Because of RegisterClassTraits().

    friend class Tracer; // It needs to access internal data because it generates new code..
    friend class TR::State;

    friend int Read(VM& vm, Multiname& obj);    
    
public:
    VM(FlashUI& _ui, FileLoader& loader, AS3::StringManager& sm, ASRefCountCollector& gc);
    virtual ~VM();

    virtual AMP::ViewStats* GetAdvanceStats() const { return NULL; }
    
public:
    enum Error
    {
        eNotImplementedYet                              = 0,
        eOutOfMemoryError                               = 1000,
        eNotImplementedError                            = 1001,
        eInvalidPrecisionError                          = 1002,
        eInvalidRadixError                              = 1003,
        eInvokeOnIncompatibleObjectError                = 1004,
        eArrayIndexNotIntegerError                      = 1005,
        eCallOfNonFunctionError                         = 1006,
        eConstructOfNonFunctionError                    = 1007,
        eAmbiguousBindingError                          = 1008,
        eConvertNullToObjectError                       = 1009,
        eConvertUndefinedToObjectError                  = 1010,
        eIllegalOpcodeError                             = 1011,
        eLastInstExceedsCodeSizeError                   = 1012,
        eFindVarWithNoScopeError                        = 1013,
        eClassNotFoundError                             = 1014,
        eIllegalSetDxns                                 = 1015,
        eDescendentsError                               = 1016,
        eScopeStackOverflowError                        = 1017,
        eScopeStackUnderflowError                       = 1018,
        eGetScopeObjectBoundsError                      = 1019,
        eCannotFallOffMethodError                       = 1020,
        eInvalidBranchTargetError                       = 1021,
        eIllegalVoidError                               = 1022,
        eStackOverflowError                             = 1023,
        eStackUnderflowError                            = 1024,
        eInvalidRegisterError                           = 1025,
        eSlotExceedsCountError                          = 1026,
        eMethodInfoExceedsCountError                    = 1027,
        eDispIdExceedsCountError                        = 1028,
        eDispIdUndefinedError                           = 1029,
        eStackDepthUnbalancedError                      = 1030,
        eScopeDepthUnbalancedError                      = 1031,
        eCpoolIndexRangeError                           = 1032,
        eCpoolEntryWrongTypeError                       = 1033,
        eCheckTypeFailedError                           = 1034,
        eIllegalSuperCallError                          = 1035,
        eCannotAssignToMethodError                      = 1037,
        eRedefinedError                                 = 1038,
        eCannotVerifyUntilReferencedError               = 1039,
        eCantUseInstanceofOnNonObjectError              = 1040,
        eIsTypeMustBeClassError                         = 1041,
        eInvalidMagicError                              = 1042,
        eInvalidCodeLengthError                         = 1043,
        eInvalidMethodInfoFlagsError                    = 1044,
        eUnsupportedTraitsKindError                     = 1045,
        eMethodInfoOrderError                           = 1046,
        eMissingEntryPointError                         = 1047,
        ePrototypeTypeError                             = 1049,
        eConvertToPrimitiveError                        = 1050,
        eIllegalEarlyBindingError                       = 1051,
        eInvalidURIError                                = 1052,
        eIllegalOverrideError                           = 1053,
        eIllegalExceptionHandlerError                   = 1054,
        eWriteSealedError                               = 1056,
        eIllegalSlotError                               = 1057,
        eIllegalOperandTypeError                        = 1058,
        eClassInfoOrderError                            = 1059,
        eClassInfoExceedsCountError                     = 1060,
        eNumberOutOfRangeError                          = 1061,
        eWrongArgumentCountError                        = 1063,
        eCannotCallMethodAsConstructor                  = 1064,
        eUndefinedVarError                              = 1065,
        eFunctionConstructorError                       = 1066,
        eIllegalNativeMethodBodyError                   = 1067,
        eCannotMergeTypesError                          = 1068,
        eReadSealedError                                = 1069,
        ePropertyNotFoundError                          = 1069, //manually added duplicated number to match actual error
        eCallNotFoundError                              = 1070,
        eAlreadyBoundError                              = 1071,
        eZeroDispIdError                                = 1072,
        eDuplicateDispIdError                           = 1073,
        eConstWriteError                                = 1074,
        eMathNotFunctionError                           = 1075,
        eMathNotConstructorError                        = 1076,
        eWriteOnlyError                                 = 1077,
        eIllegalOpMultinameError                        = 1078,
        eIllegalNativeMethodError                       = 1079,
        eIllegalNamespaceError                          = 1080,
        eReadSealedErrorNs                              = 1081,
        eNoDefaultNamespaceError                        = 1082,
        eXMLPrefixNotBound                              = 1083,
        eXMLBadQName                                    = 1084,
        eXMLUnterminatedElementTag                      = 1085,
        eXMLOnlyWorksWithOneItemLists                   = 1086,
        eXMLAssignmentToIndexedXMLNotAllowed            = 1087,
        eXMLMarkupMustBeWellFormed                      = 1088,
        eXMLAssigmentOneItemLists                       = 1089,
        eXMLMalformedElement                            = 1090,
        eXMLUnterminatedCData                           = 1091,
        eXMLUnterminatedXMLDecl                         = 1092,
        eXMLUnterminatedDocTypeDecl                     = 1093,
        eXMLUnterminatedComment                         = 1094,
        eXMLUnterminatedAttribute                       = 1095,
        eXMLUnterminatedElement                         = 1096,
        eXMLUnterminatedProcessingInstruction           = 1097,
        eXMLNamespaceWithPrefixAndNoURI                 = 1098,
        eRegExpFlagsArgumentError                       = 1100,
        eNoScopeError                                   = 1101,
        eIllegalDefaultValue                            = 1102,
        eCannotExtendFinalClass                         = 1103,
        eXMLDuplicateAttribute                          = 1104,
        eCorruptABCError                                = 1107,
        eInvalidBaseClassError                          = 1108,
        eDanglingFunctionError                          = 1109,
        eCannotExtendError                              = 1110,
        eCannotImplementError                           = 1111,
        eCoerceArgumentCountError                       = 1112,
        eInvalidNewActivationError                      = 1113,
        eNoGlobalScopeError                             = 1114,
        eNotConstructorError                            = 1115,
        eApplyError                                     = 1116,
        eXMLInvalidName                                 = 1117,
        eXMLIllegalCyclicalLoop                         = 1118,
        eDeleteTypeError                                = 1119,
        eDeleteSealedError                              = 1120,
        eDuplicateMethodBodyError                       = 1121,
        eIllegalInterfaceMethodBodyError                = 1122,
        eFilterError                                    = 1123,
        eInvalidHasNextError                            = 1124,
        eOutOfRangeError                                = 1125,
        eVectorFixedError                               = 1126,
        eTypeAppOfNonParamType                          = 1127,
        eWrongTypeArgCountError                         = 1128,
        eFileOpenError                                  = 1500,
        eFileWriteError                                 = 1501,
        eScriptTimeoutError                             = 1502,
        eScriptTerminatedError                          = 1503,
        eEndOfFileError                                 = 1504,
        eStringIndexOutOfBoundsError                    = 1505,
        eInvalidRangeError                              = 1506,
        eNullArgumentError                              = 1507,
        eInvalidArgumentError                           = 1508,
        eShellCompressedDataError                       = 1509,
        eArrayFilterNonNullObjectError                  = 1510,
        eIndexOutOfBoundsError                          = 2006,
        eChildNullError                                 = 2007,
        eParameterValueArgumentError                    = 2008,
        eAddObjectItselfError                           = 2024,
        eMustBeChildError                               = 2025,
        eSceneNotFound                                  = 2108,
        eFrameLabelNotFoundInScene                      = 2109
    };

public:
    bool CheckObject(const Value& v);

    void exec_nop() 
    { 
        // Do nothing 
    }
    int exec_throw(SInt32 cp, CallFrame& cf) 
    { 
        // Get exception object from the stack ...
        SF_ASSERT(OpStack.GetSize() > 0);
        ExceptionObj = OpStack.Pop();
        
        return OnException(cp, cf);
    }
    void exec_getsuper(VMAbcFile& file, UInt32 v);
    void exec_setsuper(VMAbcFile& file, UInt32 index);
    void exec_dxns(UInt32 /*index*/) 
    {
        // Sets the default XML namespace.
        
        // Not implemented yet ...
        SF_ASSERT(false);
        
        // index is a u30 that must be an index into the string constant pool. The string at index is used
        // as the uri for the default XML namespace for this method.
        
        // A VerifyError is thrown if dxns is used in a method that does not have the SETS_DXNS flag set.
    }
    void exec_dxnslate() 
    {
        // Sets the default XML namespace with a value determined at runtime.

        SF_ASSERT(OpStack.GetSize() > 0);
        Value value = OpStack.Pop();
        
        // Not implemented yet ...
        SF_ASSERT(false);
        
        // The top value on the stack is popped, converted to a string, and that string is used as the uri
        // for the default XML namespace for this method.
        
        // A VerifyError is thrown if dxnslate is used in a method that does not have the SETS_DXNS flag set.
    }
    void exec_kill(UInt32 v) 
    {
        SetRegister(AbsoluteIndex(v), Value::GetUndefined());
    }
    void exec_label() 
    { 
        // Do nothing 
    }
    int exec_ifnlt(SInt32 v) 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        Value result;
        int r = 0;

        if (AbstractLessThan(result, stack._1, stack._2))
        {
            // If the result of the comparison is false ...
            // kUndefined is not true ...
            if (result.GetKind() == Value::kUndefined || result.AsBool() == false)
                r = v;
        }
        
        return r;
    }
    int exec_ifnlt_i(SInt32 v) 
    {
        VTSH2<2, SInt32> stack(OpStack);
        int r = 0;

        if (stack._1 >= stack._2)
            r = v;
        
        return r;
    }
    int exec_ifnle(SInt32 v) 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        Value result;
        int r = 0;

        if (AbstractLessThan(result, stack._2, stack._1))
        {
            // If the result of the comparison is true ...
            // kUndefined is not false ...
            if (result.GetKind() == Value::kUndefined || result.AsBool() == true)
                r = v;
        }
        
        return r;
    }
    int exec_ifnle_i(SInt32 v) 
    {
        VTSH2<2, SInt32> stack(OpStack);
        int r = 0;

        if (stack._1 > stack._2)
            r = v;
        
        return r;
    }
    int exec_ifngt(SInt32 v) 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        Value result;
        int r = 0;

        if (AbstractLessThan(result, stack._2, stack._1))
        {
            // If the result of the comparison is not true ...
            if (result.GetKind() == Value::kUndefined || result.AsBool() == false)
                r = v;
        }
        
        return r;
    }
    int exec_ifngt_i(SInt32 v) 
    {
        VTSH2<2, SInt32> stack(OpStack);
        int r = 0;

        if (stack._1 <= stack._2)
            r = v;
        
        return r;
    }
    int exec_ifnge(SInt32 v) 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        Value result;
        int r = 0;

        if (AbstractLessThan(result, stack._1, stack._2))
        {
            // If the result of the comparison is not false ...
            if (result.GetKind() == Value::kUndefined || result.AsBool() == true)
                r = v;
        }
        
        return r;
    }
    int exec_ifnge_i(SInt32 v) 
    {
        VTSH2<2, SInt32> stack(OpStack);
        int r = 0;

        if (stack._1 < stack._2)
            r = v;
        
        return r;
    }
    int exec_jump(SInt32 v) 
    {
        return v;
    }
    int exec_iftrue(SInt32 v) 
    {
        SH1<1, bool> stack(OpStack);
        int r = 0;

        if (stack.IsOK() && stack._1 == true)
            r = v;
        
        return r;
    }
    int exec_iftrue_b(SInt32 v) 
    {
        TSH1<1, bool> stack(OpStack);
        int r = 0;

        if (stack._1 == true)
            r = v;
        
        return r;
    }
    int exec_iffalse(SInt32 v)
    {
        SH1<1, bool> stack(OpStack);
        int r = 0;

        if (stack.IsOK() && stack._1 == false)
            r = v;
        
        return r;
    }
    int exec_iffalse_b(SInt32 v)
    {
        TSH1<1, bool> stack(OpStack);
        int r = 0;

        if (stack._1 == false)
            r = v;
        
        return r;
    }
    int exec_ifeq(SInt32 v) 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        int r = 0;
        bool result;
        
        // Order of parameter evaluation is not important here ...
        if (AbstractEqual(result, stack._1, stack._2))
            if (result)
                r = v;
        
        return r;
    }
    int exec_ifeq_i(SInt32 v) 
    {
        VTSH2<2, SInt32> stack(OpStack);
        int r = 0;
        
        // Order of parameter evaluation is not important here ...
        if (stack._1 == stack._2)
            r = v;
        
        return r;
    }
    int exec_ifne(SInt32 v) 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        int r = 0;
        bool result;
        
        // Order of parameter evaluation is not important here ...
        if (AbstractEqual(result, stack._1, stack._2))
            if (!result)
                r = v;
        
        return r;
    }
    int exec_ifne_i(SInt32 v) 
    {
        VTSH2<2, SInt32> stack(OpStack);
        int r = 0;
        
        // Order of parameter evaluation is not important here ...
        if (stack._1 != stack._2)
            r = v;
        
        return r;
    }
    int exec_iflt(SInt32 v)
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        int r = 0;
        bool result;
        
        if (LessThan(result, stack._1, stack._2))
            if (result)
                r = v;
        
        return r;
    }
    int exec_iflt_i(SInt32 v)
    {
        VTSH2<2, SInt32> stack(OpStack);
        int r = 0;
        
        if (stack._1 < stack._2)
            r = v;
        
        return r;
    }
    int exec_ifle(SInt32 v)
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        int r = 0;
        bool result;
        
        if (LessThanOrEqual(result, stack._1, stack._2))
            if (result)
                r = v;
        
        // RVO ...
        return r;

//          SH2<2> stack(op_stack);
//          return (LessThanOrEqual(stack._1, stack._2) ? v : 0);

        // Below is an example of code in C-stile, which is almost 10% slower ...
//          const Value& _2 = op_stack.Top();
//          const Value& _1 = op_stack[op_stack.GetSize() - 2];
//          int r = 0;
//          
//          if (LessThanOrEqual(_1, _2))
//              r = v;
//          
//          op_stack.Drop(2);
//          return r;
    }
    int exec_ifle_i(SInt32 v)
    {
        VTSH2<2, SInt32> stack(OpStack);
        int r = 0;
        
        if (stack._1 <= stack._2)
            r = v;
        
        return r;
    }
    int exec_ifgt(SInt32 v) 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        int r = 0;
        bool result;
        
        if (GreaterThan(result, stack._1, stack._2))
            if (result)
                r = v;
        
        return r;
    }
    int exec_ifgt_i(SInt32 v)
    {
        VTSH2<2, SInt32> stack(OpStack);
        int r = 0;
        
        if (stack._1 > stack._2)
            r = v;
        
        return r;
    }
    int exec_ifge(SInt32 v) 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        int r = 0;
        bool result;
        
        if (GreaterThanOrEqual(result, stack._1, stack._2))
            if (result)
                r = v;
        
        return r;
    }
    int exec_ifge_i(SInt32 v)
    {
        VTSH2<2, SInt32> stack(OpStack);
        int r = 0;
        
        if (stack._1 >= stack._2)
            r = v;
        
        return r;
    }
    int exec_ifstricteq(SInt32 v) 
    {
        int r = 0;
        
        // Order of parameter evaluation is not important here ...
        SF_ASSERT(OpStack.GetSize() > 1);
        if (StrictEqual(OpStack.Pop(), OpStack.Pop()))
            r = v;
        
        return r;
    }
    int exec_ifstrictne(SInt32 v) 
    {
        int r = 0;
        
        // Order of parameter evaluation is not important here ...
        SF_ASSERT(OpStack.GetSize() > 1);
        if (!StrictEqual(OpStack.Pop(), OpStack.Pop()))
            r = v;
        
        return r;
    }
    int exec_lookupswitch() 
    {
        // Value on the stack must be of type integer ...
        SF_ASSERT(OpStack.GetSize() > 0);
        return OpStack.Pop().AsInt();
    }
    void exec_pushwith() 
    {
        // A TypeError is thrown if scope_obj is null or undefined.
        if (OpStack.Back().IsNullOrUndefined())
        {            
            return ThrowTypeError(
                OpStack.Back().IsNull() ? eConvertNullToObjectError :  eConvertUndefinedToObjectError
                 DEBUG_ARG("With scope object is null or undefined")
                );
        }                    
        SF_ASSERT(OpStack.GetSize() > 0);
        GetScopeStack().PushBack(OpStack.Pop());
        // Set up the *scope* flag.
        GetScopeStack().Back().SetWith();
    }
    void exec_popscope() 
    {
        GetScopeStack().PopBack();
    }
    void exec_nextname() 
    {
        SH2<1, Value, SInt32> stack(OpStack);

        if (stack.IsOK())
            stack._1 = Value(stack._1.GetObject()->GetNextPropertyName(GlobalSlotIndex(stack._2)));
    }
    void exec_hasnext();
    void exec_pushnull()
    {
        OpStack.PushBack(Value::GetNull());
    }
    void exec_pushundefined()
    {
        OpStack.PushBack(Value::GetUndefined());
    }
    void exec_nextvalue();
    void exec_pushbyte(UInt8 v) 
    {
        // The value is promoted to an int.
        OpStack.PushBack(Value(SInt32(SInt8(v))));
    }
    void exec_pushshort(SInt32 v)
    {
        OpStack.PushBack(Value(v));
    }
    void exec_pushtrue()
    {
        OpStack.PushBack(Value(true));
    }
    void exec_pushfalse()
    {
        OpStack.PushBack(Value(false));
    }
    void exec_pushnan()
    {
        OpStack.PushBack(Value(NumberUtil::NaN()));
    }
    void exec_pop()
    {
        SF_ASSERT(OpStack.GetSize() > 0);
        OpStack.Pop();
    }
    void exec_dup() 
    {
        OpStack.PushBack(OpStack.Back());
    }
    void exec_swap() 
    {
        // Optimized version ...
        
        SPInt num      = OpStack.GetSize();
        const Value _2 = OpStack[--num];
        
        OpStack[num] = OpStack[num - 1];
        OpStack[--num] = _2;
    }
    void exec_pushstring(VMAbcFile& file, UInt32 v) 
    {
        OpStack.PushBack(
            GetStringManager().CreateString(
                file.GetConstPool().GetString(AbsoluteIndex(v))
                )
            );
    }
    void exec_pushint(VMAbcFile& file, UInt32 v) 
    {
        OpStack.PushBack(Value(file.GetConstPool().GetInt(v)));
    }
    void exec_pushuint(VMAbcFile& file, UInt32 v) 
    {
        OpStack.PushBack(Value(file.GetConstPool().GetUInt(v)));
    }
    void exec_pushdouble(VMAbcFile& file, UInt32 v) 
    {
        OpStack.PushBack(Value(MakeValueNumber(file.GetConstPool().GetDouble(v))));
    }
    void exec_pushscope() 
    {
        // A TypeError is thrown if value is null or undefined.
        if (OpStack.Back().IsNullOrUndefined())
        {            
            return ThrowTypeError(
                OpStack.Back().IsNull() ? eConvertNullToObjectError :  eConvertUndefinedToObjectError
                 DEBUG_ARG("Scope value is null or undefined")
                );
        }
        
        SF_ASSERT(OpStack.GetSize() > 0);
        GetScopeStack().PushBack(OpStack.Pop());
    }
    void exec_pushnamespace(VMAbcFile& file, UInt32 v) 
    {
        OpStack.PushBack(Value(&file.GetInternedNamespace(v)));
    }
    void exec_hasnext2(UInt32 object_reg, UInt32 index_reg);
    void exec_li8() 
    {
        // Load data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_li16() 
    {
        // Load data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_li32() 
    {
        // Load data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_lf32() 
    {
        // Load data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_lf64() 
    {
        // Load data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_si8() 
    {
        // Store data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_si16() 
    {
        // Store data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_si32() 
    {
        // Store data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_sf32() 
    {
        // Store data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_sf64() 
    {
        // Store data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_newfunction(UInt32 method_ind);
    void exec_call(UInt32 arg_count);
    void exec_construct(UInt32 arg_count);
    void exec_callmethod(UInt32 method_index, UInt32 arg_count);
    void exec_callsupermethod(UInt32 method_index, UInt32 arg_count);
    void exec_callstatic(VMAbcFile& file, UInt32 method_index, UInt32 arg_count);
    void exec_callsuper(VMAbcFile& file, UInt32 mn_index, UInt32 arg_count);
    void exec_callproperty(VMAbcFile& file, UInt32 mn_index, UInt32 arg_count);
    void exec_returnvoid();
    void exec_returnvalue();
    void exec_constructsuper(const CallFrame& cf, UInt32 arg_count);
    void exec_constructprop(VMAbcFile& file, UInt32 mn_index, UInt32 arg_count);
    void exec_callsuperid()
    {
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_callproplex(VMAbcFile& file, UInt32 mn_index, UInt32 arg_count);
    void exec_callinterface()
    {
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_callsupervoid(VMAbcFile& file, UInt32 mn_index, UInt32 arg_count);
    void exec_callpropvoid(VMAbcFile& file, UInt32 mn_index, UInt32 arg_count);
    void exec_sxi1() 
    {
        // Sign extend ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_sxi8() 
    {
        // Sign extend ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_sxi16() 
    {
        // Sign extend ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_applytype(UInt32 arg_count);
    void exec_newobject(UInt32 arg_count);
    void exec_newarray(UInt32 arr_size);
    void exec_newactivation(const CallFrame& cf)
    {
        OpStack.PushBack(GetClassActivation().MakeInstance(cf.GetFile(), cf.GetMethodBodyInfo() DEBUG_ARG(ASString(cf.GetName()))));

        // No exceptions in this method;
        // Tamarin throws exceptions in this opcode.
    }
    void exec_newclass(VMAbcFile& file, UInt32 v);
    void exec_getdescendants(VMAbcFile& file, UInt32 index);
    void exec_newcatch(VMAbcFile& file, const Abc::MethodBodyInfo::ExceptionInfo& e);
    void exec_findpropglobalstrict(UInt32 /*v*/) 
    {
        // Not documented ...
        // Not implemented ...
        SF_ASSERT(false);
    }
    void exec_findpropglobal(UInt32 /*v*/)
    {
        // Not documented ...
        // Not implemented ...
        SF_ASSERT(false);
    }
    void exec_findpropstrict(VMAbcFile& file, UInt32 mn_index);
    void exec_findproperty(VMAbcFile& file, UInt32 mn_index);
    void exec_finddef(UInt32 /*v*/) 
    {
        // Not documented ...
        // Not implemented ...
        SF_ASSERT(false);
    }
    void exec_getlex(VMAbcFile& file, UInt32 mn_index);
    void exec_setproperty(VMAbcFile& file, UInt32 mn_index);
    void exec_getlocal(UInt32 v) 
    {
        OpStack.PushBack(GetRegister(AbsoluteIndex(v)));
    }
    void exec_setlocal(UInt32 v) 
    {
        SF_ASSERT(OpStack.GetSize() > 0);
        SetRegister(AbsoluteIndex(v), OpStack.Pop());
    }
    void exec_getglobalscope() 
    {
        // Gets the global scope.
        
        // DO NOT delete this code !!!
        // !!! This is definitely not correct.
        // Version 1.
        // op_stack.Add(GetScopeStack()[0]);
        
        // Version 2.
        // First object in a first frame.
        // (Doesn't seem to work)
        //OpStack.PushBack(CallStack[0].ScopeStack[0]);
        
        // Version 3.
        // It works fine with non-flash code.
        /*
        if (CallStack.GetSize() > 0)
        {
            if (CallStack[0].ScopeStack.GetSize() > 0)
            {
                OpStack.PushBack(CallStack[0].ScopeStack[0]);
                return;
            } 
            else if (GetScopeStack().GetSize() > 0)
            {
                OpStack.PushBack(GetScopeStack().At(0));
                return;
            }
        }

        // ???
        // Last chance.
        // GetGlobalObjectCPP() is correct here.
        OpStack.PushBack(GetGlobalObjectCPPValue());
        */

        // Version 4.
        OpStack.PushBack(GetGlobalObject());

        // Version 5.
        //OpStack.PushBack(GetRegister(AbsoluteIndex(0)));
    }
    void exec_getscopeobject(UInt32 scope_index) 
    {
        // Get a scope object.
        const UPInt index = GetCurrCallFrame().GetScopeStackBaseInd() + scope_index;

        if (index >= GetScopeStack().GetSize())
        {
            SF_ASSERT(false);
            // index must be less than the current depth of the scope stack.
            return ThrowReferenceError(VM::eNotImplementedYet);
        }
        
        OpStack.PushBack(GetScopeStack()[index]);
        // Clean up the *with* flag.
        OpStack.Back().SetWith(false);
    }
    void exec_getproperty(VMAbcFile& file, UInt32 mn_index);
    void exec_getouterscope(CallFrame& cf, UInt32 scope_index) 
    {
        // Get a stored scope object.
        // This opcode is generated by VM. It is not supposed to be generated by a compiler.

        const ScopeStackType* ss = cf.GetSavedScope();
        if (ss == NULL || scope_index >= ss->GetSize())
        {
            SF_ASSERT(false);
            // index must be less than the current depth of the scope stack.
            return ThrowReferenceError(VM::eNotImplementedYet);
        }

        OpStack.PushBack((*ss)[scope_index]);
    }
    void exec_initproperty(VMAbcFile& file, UInt32 mn_index);
    void exec_deleteproperty(VMAbcFile& file, UInt32 mn_index);
    void exec_getslot(UInt32 slot_index);
    void exec_setslot(UInt32 slot_index);
    void exec_getglobalslot(UInt32 slot_index) 
    {
        // Get the value of a slot on the global scope.
        
        OpStack.PushBack(Value::GetUndefined());
        GetGlobalObject().GetObject()->GetSlotValueUnsafe(SlotIndex(slot_index), OpStack.Back()).DoNotCheck();
    }
    void exec_setglobalslot(UInt32 slot_index) 
    {
        // Set the value of a slot on the global scope.
        
        //GetRegister(AbsoluteIndex(0)).GetObject()->SetSlotValue(SlotIndex(slot_index), OpStack.Back());
        if (GetGlobalObject().GetObject()->SetSlotValue(SlotIndex(slot_index), OpStack.Back()))
            OpStack.PopBack();
    }
    void exec_convert_s() 
    {
        OpStack.Back().ToStringValue(GetStringManager()).DoNotCheck();
        // Can throw exceptions.
    }
    void exec_esc_xelem() 
    {
        // Escape an xml element.
        // This uses the ToXmlString algorithm as described in the E4X specification,
        // ECMA-357 section 10.2, to perform the conversion.
        
        SF_ASSERT(OpStack.GetSize() > 0);
        const Value value = OpStack.Pop();
        
        OpStack.PushBack(value);
        
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_esc_xattr() 
    {
        // Escape an xml attribute.
        // This uses the EscapeAttributeValue algorithm as described in the E4X
        // specification, ECMA-357 section 10.2.1.2, to perform the conversion.
        
        SF_ASSERT(OpStack.GetSize() > 0);
        const Value value = OpStack.Pop();
        
        OpStack.PushBack(value);
        
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_convert_i() 
    {
        // Extra check to satisfy postcondition checker.
        OpStack.Back().ToInt32Value().DoNotCheck();
        // Can throw exceptions.
    }
    void exec_convert_u()
    {
        // Extra check to satisfy postcondition checker.
        OpStack.Back().ToUInt32Value().DoNotCheck();
        // Can throw exceptions.
    }
    void exec_convert_d() 
    {
        // Extra check to satisfy postcondition checker.
        OpStack.Back().ToNumberValue().DoNotCheck();
        // Can throw exceptions.
    }
    void exec_convert_b() 
    {
        OpStack.Back().ToBooleanValue();
        // Doesn't throw exceptions.
    }
    void exec_convert_o() 
    {
        const Value& value = OpStack.Back();
        
        if (value.IsNullOrUndefined())
        {
            // A TypeError is thrown if obj is null or undefined.
            //SF_ASSERT(false);
            return ThrowTypeError(eNotImplementedYet DEBUG_ARG("Object is null or undefined"));
        }
        
        if (!value.IsObjectStrict())
        {
            // If value is an Object then nothing happens. Otherwise an exception is thrown.
            //SF_ASSERT(false);
            return ThrowError(eNotImplementedYet DEBUG_ARG("Cannot convert to Object"));
        }
    }
    void exec_checkfilter() 
    {
        // XML-related ...
        
        // If value is of type XML or XMLList then nothing happens. 
        // If value is of any other type a TypeError is thrown.

        /*
        SF_ASSERT(OpStack.GetSize() > 0);
        const Value value = OpStack.Pop();
        OpStack.PushBack(value);
        */
        
        // Not implemented yet ...
        // SF_ASSERT(false);
    }
    void exec_coerce(VMAbcFile& file, UInt32 mn_index);
    void exec_coerce_b() 
    {
        // Not documented ...
        // Not implemented ...
        SF_ASSERT(false);
    }
    void exec_coerce_a() 
    {
        // Coerce a value to the any type.
        // Indicates to the verifier that the value on the stack is of the any type (*). Does nothing to value.
    }
    void exec_coerce_i() 
    {
        // Not documented ...
        // Not implemented ...
        SF_ASSERT(false);
    }
    void exec_coerce_d() 
    {
        // Not documented ...
        // Not implemented ...
        SF_ASSERT(false);
    }
    void exec_coerce_s() 
    {
        Value& v = OpStack.Back();

        if (v.IsNullOrUndefined())
            v = Value::GetNull();
        else
            v.ToStringValue(GetStringManager()).DoNotCheck();
        // Can throw exceptions.
    }
    void exec_astype(VMAbcFile& file, UInt32 mn_index);
    void exec_astypelate();
    void exec_coerce_u() 
    {
        // Not documented ...
        // Not implemented ...
        SF_ASSERT(false);
    }
    void exec_coerce_o() 
    {
        // Not documented ...
        // Not implemented ...
        SF_ASSERT(false);
    }
    void exec_negate() 
    {
        SH1<0> stack(OpStack);
        // No exceptions at this point.
		Negate(stack._1);
        // Negate() can throw exceptions.
    }
    void exec_increment()
    {
        SH1<0> stack(OpStack);
        // No exceptions at this point.
		Increment(stack._1);
        // Increment() can throw exceptions.
    }
    void exec_inclocal(UInt32 v)
    {
		Increment(GetRegister(AbsoluteIndex(v)));
        // Increment() can throw exceptions.
    }
    void exec_decrement()
    {
		SH1<0> stack(OpStack);
        // No exceptions at this point.
		Decrement(stack._1);
        // Decrement() can throw exceptions.
    }
    void exec_declocal(UInt32 v)
    {
		Decrement(GetRegister(AbsoluteIndex(v)));
        // Decrement() can throw exceptions.
    }
    void exec_typeof();
    void exec_not() 
    {
        SH1<0, bool> stack(OpStack);

        if (stack.IsOK())
            stack._1 = !stack._1;

        // No exceptions.
    }
    void exec_bitnot()
    {
        SInt32 v;
        
        if (OpStack.Back().Convert2Int32(v))
            OpStack.Back() = Value(~v);

        // Can throw exceptions.
    }
    void exec_concat() 
    {
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_add_d() 
    {
        // Version of "add", which works with numbers ...
        SH2<1, Value::Number> stack(OpStack);

        if (stack.IsOK())
            stack._1 += stack._2;
    }
    void exec_add() 
    {
        SH2<1> stack(OpStack);
        // No exceptions at this point.
        Add(GetStringManager(), stack._1, stack._1, stack._2).DoNotCheck();
        // Add() can throw exceptions.
    }
    void exec_subtract() 
    {
		SH2<1> stack(OpStack);
        // No exceptions at this point.
		Subtract(stack._1, stack._1, stack._2).DoNotCheck();
        // Subtract() can throw exceptions.
    }
    void exec_multiply() 
    {
		SH2<1> stack(OpStack);
        // No exceptions at this point.
		Multiply(stack._1, stack._1, stack._2).DoNotCheck();
        // Multiply() can throw exceptions.
    }
    void exec_divide() 
    {
        SH2<1, Value::Number> stack(OpStack);

        if (stack.IsOK())
            stack._1 /= stack._2;
    }
    void exec_modulo() 
    {
        SH2<1, Value::Number> stack(OpStack);

        if (stack.IsOK())
            stack._1 = fmod(stack._1, stack._2);
    }
    void exec_lshift() 
    {
        // (ECMA-262 section 11.7.1)
		// Documentation says that stack._1 should be SInt32
        SH2<1, SInt32, UInt32> stack(OpStack);

        if (stack.IsOK())
            stack._1 <<= (stack._2 & 0x1F);
    }
    void exec_rshift()
    {
        // The right shift is sign extended, resulting in a signed 32-bit integer. See
        // ECMA-262 section 11.7.2
        // (C++) The type of the result is the same as the type of the left operand.
        SH2<1, SInt32, UInt32> stack(OpStack);

        if (stack.IsOK())
            stack._1 = stack._1 >> (stack._2 & 0x1F);
    }
    void exec_urshift() 
    {
        // !!! May be implemented incorrectly ...
        
        // The right shift is sign extended, resulting in a signed 32-bit integer. See
        // ECMA-262 section 11.7.3
        // (C++) The type of the result is the same as the type of the left operand.
        SH2<1, UInt32> stack(OpStack);

        if (stack.IsOK())
            stack._1 = stack._1 >> (stack._2 & 0x1F);
    }
    void exec_bitand() 
    {
        SH2<1, SInt32> stack(OpStack);

        if (stack.IsOK())
            stack._1 &= stack._2;
    }
    void exec_bitor() 
    {
        SH2<1, SInt32> stack(OpStack);

        if (stack.IsOK())
            stack._1 |= stack._2;
    }
    void exec_bitxor()
    {
        SH2<1, SInt32> stack(OpStack);

        if (stack.IsOK())
            stack._1 ^= stack._2;
    }
    void exec_equals() 
    {
        SH2<1> stack(OpStack);
        // No exceptions at this point.
        bool result;
        if (AbstractEqual(result, stack._1, stack._2))
            stack._1.SetBool(result);
    }
    void exec_strictequals() 
    {
        SH2<1> stack(OpStack);
        // No exceptions at this point.
        stack._1 = Value(StrictEqual(stack._1, stack._2));
    }
    void exec_lessthan() 
    {
        SH2<1> stack(OpStack);
        // No exceptions at this point.
        bool result;
        if (LessThan(result, stack._1, stack._2))
            stack._1.SetBool(result);
    }
    void exec_lessequals() 
    {
        SH2<1> stack(OpStack);
        // No exceptions at this point.
        bool result;
        if (LessThanOrEqual(result, stack._1, stack._2))
            stack._1.SetBool(result);
    }
    void exec_greaterthan()
    {
        SH2<1> stack(OpStack);
        // No exceptions at this point.
        bool result;
        if (GreaterThan(result, stack._1, stack._2))
            stack._1.SetBool(result);
    }
    void exec_greaterequals() 
    {
        SH2<1> stack(OpStack);
        // No exceptions at this point.
        bool result;
        if (GreaterThanOrEqual(result, stack._1, stack._2))
            stack._1.SetBool(result);
    }
    void exec_instanceof();
    void exec_istype(VMAbcFile& file, UInt32 mn_index);
    void exec_istypelate() 
    {
        // Check whether an Object is of a certain type.
        SH2<1> stack(OpStack);
        // No exceptions at this point.

        // stack._1 - value
        // stack._2 - type
        
        // A TypeError is thrown if type is not a Class.
        if (!stack._2.IsClass())
            return ThrowTypeError(eIsTypeMustBeClassError DEBUG_ARG("Type must be a Class"));
        
        const ClassTraits::Traits& type = stack._2.AsClass().GetClassTraits();
        
        stack._1 = Value(IsOfType(stack._1, type));
    }
    void exec_in();
    void exec_getabsobject(UPInt addr)
    {
        OpStack.PushBack(GetAbsObject(addr));
    }
    // slot_ind is ONE based.
    // slot_ind - 1 is an index in a slot table.
//     void exec_getabsslot(UPInt slot_ind);
    void exec_setabsslot(UPInt slot_ind);
    void exec_initabsslot(UPInt slot_ind);
    void exec_increment_i()
    {
        // Conversion is necessary here.
        SH1<0, SInt32> stack(OpStack);
        
        if (stack.IsOK())
            stack._1 += 1;
    }
    void exec_decrement_i() 
    {
        // Conversion is necessary here.
        SH1<0, SInt32> stack(OpStack);
        
        if (stack.IsOK())
            stack._1 -= 1;
    }
    void exec_increment_i2()
    {
        SInt32& i = OpStack.Back();
        ++i;
    }
    void exec_decrement_i2() 
    {
        SInt32& i = OpStack.Back();
        --i;
    }
    void exec_inclocal_i(UInt32 v)
    {
        Value& value = GetRegister(AbsoluteIndex(v));
        
        if (!value.ToInt32Value())
            // Exception
            return;

        SInt32& _1 = value;
        ++_1;
    }
    void exec_inclocal_i2(UInt32 v)
    {
        SInt32& i = GetRegister(AbsoluteIndex(v));
        ++i;
    }
    void exec_declocal_i(UInt32 v) 
    {
        Value& value = GetRegister(AbsoluteIndex(v));
        
        if (!value.ToInt32Value())
            // Exception
            return;

        SInt32& _1 = value;
        --_1;
    }
    void exec_declocal_i2(UInt32 v) 
    {
        SInt32& i = GetRegister(AbsoluteIndex(v));
        --i;
    }
    void exec_negate_i() 
    {
        // Conversion is necessary here.
        SH1<0, SInt32> stack(OpStack);
        
        if (stack.IsOK())
            stack._1 = -stack._1;
    }
    void exec_add_i() 
    {
        SH2<1, SInt32> stack(OpStack);

        if (stack.IsOK())
            stack._1 += stack._2;
    }
    void exec_subtract_i() 
    {
        SH2<1, SInt32> stack(OpStack);

        if (stack.IsOK())
            stack._1 -= stack._2;
    }
    void exec_multiply_i() 
    {
        SH2<1, SInt32> stack(OpStack);

        if (stack.IsOK())
            stack._1 *= stack._2;
    }
    void exec_getlocal0() 
    {
        OpStack.PushBack(GetRegister(AbsoluteIndex(0)));
    }
    void exec_getlocal1()
    {
        OpStack.PushBack(GetRegister(AbsoluteIndex(1)));
    }   
    void exec_getlocal2()
    {
        OpStack.PushBack(GetRegister(AbsoluteIndex(2)));
    }
    void exec_getlocal3()
    {
        OpStack.PushBack(GetRegister(AbsoluteIndex(3)));
    }
    void exec_setlocal0() 
    {
        SF_ASSERT(OpStack.GetSize() > 0);
        SetRegister(AbsoluteIndex(0), OpStack.Pop());
    }
    void exec_setlocal1() 
    {
        SF_ASSERT(OpStack.GetSize() > 0);
        SetRegister(AbsoluteIndex(1), OpStack.Pop());
    }
    void exec_setlocal2() 
    {
        SF_ASSERT(OpStack.GetSize() > 0);
        SetRegister(AbsoluteIndex(2), OpStack.Pop());
    }
    void exec_setlocal3() 
    {
        SF_ASSERT(OpStack.GetSize() > 0);
        SetRegister(AbsoluteIndex(3), OpStack.Pop());
    }
    void exec_abs_jump(UInt32 /*v1*/, UInt32 /*v2*/) 
    {
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }
    void exec_debug() 
    {
        // Not implemented yet ...
    }
    void exec_debugline(CallFrame& cf, UInt32 v);
    void exec_debugfile(CallFrame& cf, UInt32 v);
    void exec_timestamp() 
    {
        // Not documented ...
        // Not implemented yet ...
        SF_ASSERT(false);
    }

public:
    CheckResult LoadFile(const Abc::File& file, bool to_execute = false);
    bool LoadPackage(const ASString& name);

public:
    // GetRegisteredClassTraits() returns a hashed value if any.
    const ClassTraits::Traits* GetRegisteredClassTraits(const Multiname& mn);
    const ClassTraits::Traits* GetRegisteredClassTraits(const ASString& name, const Instances::Namespace& ns)
    {
        SPtr<ClassTraits::Traits>* pptraits = ClassTraitsSet.Get(name, ns);
        return pptraits ? *pptraits : NULL;
    }

    const ClassTraits::Traits* Resolve2ClassTraits(const Multiname& mn);
    const ClassTraits::Traits* Resolve2ClassTraits(const ASString& name, const Instances::Namespace& ns);

    const ClassTraits::Traits* Resolve2ClassTraits(VMAbcFile& file, const Abc::Multiname& mn);

    void RegisterClassTraits(const ASString& name, const Instances::Namespace& ns, ClassTraits::Traits& ctr)
    {
        ClassTraitsSet.Add(name, ns, &ctr);
    }

public:
    // Get Class of the value ...
    // kUndefined and kNull should be handled outside of this method ...
    const Class& GetClass(const Value& v) const;

    // Utility method.
    // Not const because of GetStringManager().
    // Non-const because of Resolve2Class().
    bool IsOfType(const Value& v, const char* type_name);
    // Handles primitive values correctly.
    bool IsOfType(const Value& v, const ClassTraits::Traits& ctr) const;

public:
    // By default execute only one top stack frame (including all function calls).
    // Return number of stack frames left to execute. 
    // Return ZERO if execution is finished completely.
    int ExecuteCode(unsigned max_stack_depth = 1);
    void RetrieveResult(Value& result)
    {
        SF_ASSERT(OpStack.GetSize() > 0);
        result = OpStack.Pop();
    }
    bool IsFinished() const
    {
        return CallStack.GetSize() == 0;
    }
    // saved_scope shouldn't be *const*.
    void AddFrame(
        const Value& invoker, // invoker can be NULL if there is nothing to track.
        VMAbcFile& file,
        const Abc::MethodInfo& mi,
        const Abc::MethodBodyInfo& mbi,
        const Abc::HasTraits* ht,
        const Value& _this,
        const ScopeStackType* saved_scope,
        int arg_count, 
        const Value* args,
        bool discard_result,
        const SPtr<const Traits>& ot // Origination Traits
        DEBUG_ARG(const ASString& name)
        )
    {
        if (CallStack.GetSize() == MaxCallStackSize)
            return ThrowError(eNotImplementedYet DEBUG_ARG("Reached MaxCallStackSize"));

        CallFrame cf(
            invoker,
            file,
            mbi,
            ht,
            saved_scope, 
            0, 
            discard_result,
            ot
            DEBUG_ARG(name)
            DEBUG_ARG(Timer::GetRawTicks())
            );
        
        cf.SetupRegisters(mi, _this, arg_count, args);
        
        if (IsException())
            return;
        
#ifdef SF_AMP_SERVER
        if (GetAdvanceStats() != NULL)
        {
            GetAdvanceStats()->PushCallstack(file.GetAbcFile().GetSwdHandle(), mbi.GetMethodInfoInd());
        }
#endif
        CallStack.PushBack(cf);
    }
    
    Pickable<Instances::Object> MakeObject() const;
    Pickable<Instances::Array> MakeArray() const;

    PropRef FindScopePropertyByName(const UPInt baseSSInd, const ScopeStackType& scope_stack, const char* name);
    PropRef FindValuePropertyByName(const Value& value, const char* name);
    
public:
    // typedef ValueArray OpStackType;
    typedef ArrayPagedLH<Value> OpStackType;
    //typedef ArrayCC<CallFrame> CallStackType;
    typedef ArrayPagedCC<CallFrame> CallStackType;

public:
    // Return Traits for a given value.

    const ClassTraits::Traits& GetClassTraits(const Value& v) const;
    InstanceTraits::Traits& GetInstanceTraits(const Value& v) const;
    Traits& GetValueTraits(const Value& v) const;

public:
    StringManager& GetStringManager() const
    {
        return StringManagerRef;
    }

    // This is a wrapper, which will/should call "toString"
    ASString AsString(const Value& value);
    
    // Needed only to display OpStack ...
    const OpStackType& GetOpStack() const
    {
        return OpStack;
    }
    UPInt GetScopeStackBaseInd() const
    {
        if (CallStack.GetSize() == 0)
            return 0;
        
        return GetCurrCallFrame().GetScopeStackBaseInd();
    }
    UPInt GetOpStackBaseInd() const
    {
        if (CallStack.GetSize() == 0)
            return 0;

        return GetCurrCallFrame().GetOriginalStackSize();
    }
    const CallFrame& GetCurrCallFrame() const
    {
        SF_ASSERT(CallStack.GetSize() > 0);
        return CallStack.Back();
    }
    const CallStackType& GetCallStack() const
    {
        return CallStack;
    }
    void AdjustOriginalStackSize()
    {
        SF_ASSERT(CallStack.GetSize() > 0);
        CallStack.Back().AdjustOriginalStackSize();
    }
    bool IsInitialized() const
    {
        return Initialized;
    }

    void GetStackTraceASString(ASString& result, const char* line_pref = "\t");

public:
    //////
    ASString GetASString(const char *pstr) const
    {
        return GetStringManager().CreateString(pstr);
    }
    ASString GetASString(const char *pstr, UPInt length) const
    {
        return GetStringManager().CreateString(pstr, length);
    }
    ASString GetASString(const String& str) const
    {
        return GetStringManager().CreateString(str);
    }

public:
    const RegistersType& GetRegisters() const
    {
        return RegisterFile;
    }

    const ScopeStackType& GetScopeStack() const
    {
        return ScopeStack;
    }

    // Register's index starts with ZERO ...
    const Value& GetRegister(AbsoluteIndex ind) const
    {
        return CallStack.Back().GetRegister(ind.Get());
    }
    
private:
    RegistersType& GetRegisters()
    {
        return RegisterFile;
    }

    ScopeStackType& GetScopeStack()
    {
        return ScopeStack;
    }

    // Register's index starts with ZERO ...
    Value& GetRegister(AbsoluteIndex ind)
    {
        return CallStack.Back().GetRegister(ind.Get());
    }

    void SetRegister(AbsoluteIndex ind, const Value& value)
    {
        CallStack.Back().SetRegister(ind.Get(), value);
    }
    
public:
    // this method is not private to let raw function trace their results.
    FlashUI& GetUI() const
    {
        return UI;
    }
    
    //
    ASRefCountCollector& GetGC() const
    {
        return GC.GetGC();
    }

    //
    const ScopeStackType& GetGlobalObjects() const
    {
        return GlobalObjects;
    }

    MemoryHeap* GetMemoryHeap() const
    {
        return MHeap;
    }

public:
    //
    bool IsClassClass(const Class& c) const
    {
        return &c == &GetClassClass();
    }
    bool IsClassClass(const ClassTraits::Traits& c) const
    {
        return &c == &GetClassTraitsClassClass();
    }
    bool IsClassClass(const Value& v) const
    {
        return IsClassClass(GetClass(v));
    }

    //
    bool IsObject(const Class& c) const
    {
        return &c == &GetClassObject();
    }
    bool IsObject(const ClassTraits::Traits& c) const
    {
        return &c == &GetClassTraitsObject();
    }
    bool IsObject(const Value& v) const
    {
        return IsObject(GetClass(v));
    }

    //
    bool IsFunction(const Class& c) const
    {
        return &c == &GetClassFunction();
    }
    bool IsFunction(const ClassTraits::Traits& c) const
    {
        return &c == &GetClassTraitsFunction();
    }
    bool IsFunction(const Value& v) const
    {
        return IsFunction(GetClass(v));
    }

    //
    bool IsMethodClosure(const Class& c) const
    {
        return &c == &GetClassMethodClosure();
    }
    bool IsMethodClosure(const ClassTraits::Traits& c) const
    {
        return &c == &GetClassTraitsMethodClosure();
    }
    bool IsMethodClosure(const Value& v) const
    {
        return IsMethodClosure(GetClass(v));
    }

    //
    bool IsNamespace(const Class& c) const
    {
        return &c == &GetClassNamespace();
    }
    bool IsNamespace(const ClassTraits::Traits& c) const
    {
        return &c == &GetClassTraitsNamespace();
    }
    bool IsNamespace(const Value& v) const
    {
        return IsNamespace(GetClass(v));
    }

    //
    bool IsBoolean(const Class& c) const
    {
        return &c == (Class*)&GetClassBoolean();
    }
    bool IsBoolean(const ClassTraits::Traits& c) const
    {
        return &c == (const ClassTraits::Traits*)&GetClassTraitsBoolean();
    }

    //
    bool IsNumber(const Class& c) const
    {
        return &c == (Class*)&GetClassNumber();
    }
    bool IsNumber(const ClassTraits::Traits& c) const
    {
        return &c == (const ClassTraits::Traits*)&GetClassTraitsNumber();
    }

    //
    bool IsInt(const Class& c) const
    {
        return &c == (Class*)&GetClassInt();
    }
    bool IsInt(const ClassTraits::Traits& c) const
    {
        return &c == (const ClassTraits::Traits*)&GetClassTraitsInt();
    }

    //
    bool IsUint(const Class& c) const
    {
        return &c == (Class*)&GetClassUInt();
    }
    bool IsUint(const ClassTraits::Traits& c) const
    {
        return &c == (const ClassTraits::Traits*)&GetClassTraitsUInt();
    }

    //
    bool IsString(const Class& c) const
    {
        return &c == (Class*)&GetClassString();
    }
    bool IsString(const ClassTraits::Traits& c) const
    {
        return &c == (const ClassTraits::Traits*)&GetClassTraitsString();
    }

    //
    bool IsArray(const Class& c) const
    {
        return &c == (Class*)&GetClassArray();
    }
    bool IsArray(const ClassTraits::Traits& c) const
    {
        return &c == (const ClassTraits::Traits*)&GetClassTraitsArray();
    }
    bool IsArray(const Value& v) const
    {
        return IsArray(GetClass(v));
    }

    //
    bool IsActivation(const Class& c) const
    {
        return &c == (Class*)&GetClassActivation();
    }
    bool IsActivation(const ClassTraits::Traits& c) const
    {
        return &c == (const ClassTraits::Traits*)&GetClassTraitsActivation();
    }
    bool IsActivation(const Value& v) const
    {
        return IsActivation(GetClass(v));
    }

    //
    bool IsQName(const Class& c) const;
    bool IsQName(const Value& v) const
    {
        return IsQName(GetClass(v));
    }

public:
	// Raw access to classes for performance reason.

	// Class.
	Classes::ClassClass& GetClassClass() const
	{
		SF_ASSERT(TraitsClassClass.Get());
        return (Classes::ClassClass&)TraitsClassClass->GetInstanceTraits().GetClass();
	}
    const ClassTraits::ClassClass& GetClassTraitsClassClass() const
    {
        SF_ASSERT(TraitsClassClass.Get());
        return *TraitsClassClass;
    }
    InstanceTraits::Traits& GetITraitsClass() const
    {
        return GetClassTraitsClassClass().GetInstanceTraits();
    }

    // Activation.
    Classes::Activation& GetClassActivation() const
    {
        SF_ASSERT(TraitsActivation.Get());
        return (Classes::Activation&)TraitsActivation->GetInstanceTraits().GetClass();
    }
    ClassTraits::Activation& GetClassTraitsActivation() const
    {
        SF_ASSERT(TraitsActivation.Get());
        return *TraitsActivation;
    }

    // Catch.
    Classes::Catch& GetClassCatch() const;

	// Object.
	Classes::Object& GetClassObject() const
	{
        SF_ASSERT(TraitsObject.Get());
        return (Classes::Object&)TraitsObject->GetInstanceTraits().GetClass();
	}
    ClassTraits::Object& GetClassTraitsObject() const
    {
        SF_ASSERT(TraitsObject.Get());
        return *TraitsObject;
    }
    InstanceTraits::Traits& GetITraitsObject() const
    {
        return GetClassTraitsObject().GetInstanceTraits();
    }

	// Namespace.
	Classes::Namespace& GetClassNamespace() const
	{
        SF_ASSERT(TraitsNamespace.Get());
        return (Classes::Namespace&)TraitsNamespace->GetInstanceTraits().GetClass();
	}
    const ClassTraits::Namespace& GetClassTraitsNamespace() const
    {
        SF_ASSERT(TraitsNamespace.Get());
        return *TraitsNamespace;
    }
    InstanceTraits::Traits& GetITraitsNamespace() const
    {
        return GetClassTraitsNamespace().GetInstanceTraits();
    }

	// Function.
	Classes::Function& GetClassFunction() const
	{
        SF_ASSERT(TraitsFunction.Get());
        return (Classes::Function&)TraitsFunction->GetInstanceTraits().GetClass();
	}
    const ClassTraits::Function& GetClassTraitsFunction() const
    {
        SF_ASSERT(TraitsFunction.Get());
        return *TraitsFunction;
    }
    InstanceTraits::Traits& GetITraitsFunction() const
    {
        return GetClassTraitsFunction().GetInstanceTraits();
    }

	// MethodClosure.
	Classes::MethodClosure& GetClassMethodClosure() const
	{
        SF_ASSERT(TraitsMethodClosure.Get());
        return (Classes::MethodClosure&)TraitsMethodClosure->GetInstanceTraits().GetClass();
	}
    const ClassTraits::MethodClosure& GetClassTraitsMethodClosure() const
    {
        SF_ASSERT(TraitsMethodClosure.Get());
        return *TraitsMethodClosure;
    }

	// Boolean.
	Classes::Boolean& GetClassBoolean() const;
    const ClassTraits::Boolean& GetClassTraitsBoolean() const
    {
        SF_ASSERT(TraitsBoolean.Get());
        return *TraitsBoolean;
    }
    InstanceTraits::Traits& GetITraitsBoolean() const;

	// Number.
	Classes::Number& GetClassNumber() const;
    const ClassTraits::Number& GetClassTraitsNumber() const
    {
        SF_ASSERT(TraitsNumber.Get());
        return *TraitsNumber;
    }
    InstanceTraits::Traits& GetITraitsNumber() const;

	// int.
	Classes::int_& GetClassInt() const;
    const ClassTraits::int_& GetClassTraitsInt() const
    {
        SF_ASSERT(TraitsInt.Get());
        return *TraitsInt;
    }
    InstanceTraits::Traits& GetITraitsSInt() const;

	// uint.
	Classes::uint& GetClassUInt() const;
    const ClassTraits::uint& GetClassTraitsUInt() const
    {
        SF_ASSERT(TraitsUint.Get());
        return *TraitsUint;
    }
    InstanceTraits::Traits& GetITraitsUInt() const;

	// String.
	Classes::String& GetClassString() const;
    const ClassTraits::String& GetClassTraitsString() const
    {
        SF_ASSERT(TraitsString.Get());
        return *TraitsString;
    }
    InstanceTraits::Traits& GetITraitsString() const;

	// Array.
	Classes::Array& GetClassArray() const;
    const ClassTraits::Array& GetClassTraitsArray() const
    {
        SF_ASSERT(TraitsArray.Get());
        return *TraitsArray;
    }
    InstanceTraits::Traits& GetITraitsArray() const;

    // Vector.
    Classes::Vector& GetClassVector() const;
    const ClassTraits::Vector& GetClassTraitsVector() const
    {
        SF_ASSERT(TraitsVector.Get());
        return *TraitsVector;
    }

    // Vector$int.
    Classes::Vector_int& GetClassVectorSInt() const;
    const ClassTraits::Vector_int& GetClassTraitsVectorSInt() const
    {
        SF_ASSERT(TraitsVector_int.Get());
        return *TraitsVector_int;
    }
    InstanceTraits::Traits& GetITraitsVectorSInt() const;

    // Vector$uint.
    Classes::Vector_uint& GetClassVectorUInt() const;
    const ClassTraits::Vector_uint& GetClassTraitsVectorUInt() const
    {
        SF_ASSERT(TraitsVector_uint.Get());
        return *TraitsVector_uint;
    }
    InstanceTraits::Traits& GetITraitsVectorUInt() const;

    // Vector$double.
    Classes::Vector_double& GetClassVectorNumber() const;
    const ClassTraits::Vector_double& GetClassTraitsVectorNumber() const
    {
        SF_ASSERT(TraitsVector_Number.Get());
        return *TraitsVector_Number;
    }
    InstanceTraits::Traits& GetITraitsVectorNumber() const;

    // Vector$String.
    Classes::Vector_String& GetClassVectorString() const;
    const ClassTraits::Vector_String& GetClassTraitsVectorString() const
    {
        SF_ASSERT(TraitsVector_String.Get());
        return *TraitsVector_String;
    }
    InstanceTraits::Traits& GetITraitsVectorString() const;

    // Vector$Object.
//     Classes::Vector_object& GetClassVectorObject() const
//     {
//         SF_ASSERT(TraitsVector_Object.Get());
//         return *TraitsVector_Object;
//     }

    InstanceTraits::Traits& GetITraitsNull() const
    {
        return *TraitsNull;
    }

    InstanceTraits::Traits& GetITraitsVoid() const
    {
        return *TraitsVoid;
    }

private:
    // Return default value for a given class.
    Value GetDefaultValue(const ClassTraits::Traits& ctr) const;
    // Return default value for a given multiname.
    Value GetDefaultValue(VMAbcFile& file, const Abc::Multiname& mn);

public:
    // Exceptions ...
    void Throw(const Value& e)
    {
        HandleException = true;
        ExceptionObj = e;
    }

    // Creates error message for Throw. "Error #:" needs to be a part of message.
    ASString FormatErrorMessage(Error id, const char* msg);
    ASString FormatErrorMessage(Error id, const ASString& msg);

    // Outputs the error to log.
    void OutputError(const Value& e);

    // This is called for all NOT_IMPLEMENTED functions/classes.
    // An extra 'forceThrow' means that an exception should be triggered even
    // if we are just tracing messages. Typically it makes sense to throw an
    // exception when unimplemented methods is expected to return an class object.
    void HandleNotImplemented(const char* msg, bool forceThrow = false);    

    // Expects compile-time string.
    void ThrowError(Error id, const char* msg = NULL);
    void ThrowError(Error id, const ASString& msg);

    // Expects compile-time string.
    void ThrowDefinitionError(Error id, const char* msg = NULL);
    void ThrowDefinitionError(Error id, const ASString& msg);

    // Expects compile-time string.
    void ThrowEvalError(Error id, const char* msg = NULL);
    void ThrowEvalError(Error id, const ASString& msg);

    // Expects compile-time string.
    void ThrowRangeError(Error id, const char* msg = NULL);
    void ThrowRangeError(Error id, const ASString& msg);

    // Expects compile-time string.
    void ThrowReferenceError(Error id, const char* msg = NULL);
    void ThrowReferenceError(Error id, const ASString& msg);

    // Expects compile-time string.
    void ThrowSecurityError(Error id, const char* msg = NULL);
    void ThrowSecurityError(Error id, const ASString& msg);

    // Expects compile-time string.
    void ThrowSyntaxError(Error id, const char* msg = NULL);
    void ThrowSyntaxError(Error id, const ASString& msg);

    // Expects compile-time string.
    void ThrowTypeError(Error id, const char* msg = NULL);
    void ThrowTypeError(Error id, const ASString& msg);

    // Expects compile-time string.
    void ThrowURIError(Error id, const char* msg = NULL);
    void ThrowURIError(Error id, const ASString& msg);

    // Expects compile-time string.
    void ThrowVerifyError(Error id, const char* msg = NULL);
    void ThrowVerifyError(Error id, const ASString& msg);

    // Expects compile-time string.
    void ThrowUninitializedError(Error id, const char* msg = NULL);
    void ThrowUninitializedError(Error id, const ASString& msg);

    // Expects compile-time string.
    void ThrowArgumentError(Error id, const char* msg = NULL);
    // argc - actual number of arguments.
    void ThrowArgumentError(Error id, unsigned argc, unsigned MinArgNum, unsigned MaxArgNum);

    bool IsException() const
    {
        return HandleException;
    }
    const Value& GetExceptionValue() const
    {
        return ExceptionObj;
    }
    void IgnoreException()
    {
        SF_ASSERT(IsException());
        HandleException = false;
    }

    void OutputAndIgnoreException()
    {   // Ignore before Output so that toString() succeeds.
        IgnoreException();
        OutputError(ExceptionObj);
    }

    
public:
    // Namespace related methods.
    
    Pickable<Instances::Namespace> MakeNamespace(Abc::NamespaceKind kind)
    {
        return MakeNamespace(kind, GetStringManager().CreateEmptyString());
    }
    Pickable<Instances::Namespace> MakeNamespace(Abc::NamespaceKind kind, const ASString& uri,
                                        const Value& prefix = Value::GetUndefined()) const;
    
    Pickable<Instances::Namespace> MakeInternedNamespace(Abc::NamespaceKind kind, const char* name) const;
    Pickable<Instances::Namespace> MakeInternedNamespace(Abc::NamespaceKind kind, const ASString& uri) const;               
    Pickable<Instances::Namespace> MakeInternedNamespace(const Abc::NamespaceInfo& ni) const
    {
        return MakeInternedNamespace(ni.GetKind(), ni.GetNameURI());
    }

    Instances::Namespace& GetInternedNamespace(Abc::NamespaceKind kind, const char* name) const;
    Instances::Namespace& GetInternedNamespace(Abc::NamespaceKind kind, const ASString& uri) const;               
    Instances::Namespace& GetInternedNamespace(const Abc::NamespaceInfo& ni) const
    {
        return GetInternedNamespace(ni.GetKind(), ni.GetNameURI());
    }

    Instances::Namespace& GetPublicNamespace() const
    {
        SF_ASSERT(PublicNamespace.Get());
        return *PublicNamespace;
    }
    Pickable<Instances::Namespace> MakePublicNamespace() const
    {
        SF_ASSERT(PublicNamespace.Get());
        return Pickable<Instances::Namespace>(PublicNamespace.Get(), PickValue);
    }
    Instances::Namespace& GetAS3Namespace() const
    {
        SF_ASSERT(AS3Namespace.Get());
        return *AS3Namespace;
    }

    
public:
    // Get Class using qualified name gname.
    // Return false in case of exception or if a class cannot be found.
    Class* GetClass(const char* gname);
    bool GetClassUnsafe(const char* gname, Value& result);

    // Create an instance of a class with the qualified name gname.
    // Returns *true* if ExecuteCode() should be called after this method.
    // Basically, it returns *false* if a constructed object is a pure C++ object.
    // Can throw exceptions.
    bool Construct(const char* gname, Value& result, unsigned argc = 0, const Value* argv = NULL, 
                    bool extCall = true);

    // Constructs a builtin class such as "flash.display.Graphics" and fills
    // in a SPtr to it. Returns false of constructor failed, which is different from
    // regular Construct since ExecuteCode is not needed for builtins.
    template<class C>
    bool ConstructBuiltinObject(SPtr<C> &pobj, const char* gname,
        unsigned argc = 0, const Value* argv = NULL)
    {
        SF_ASSERT(((UPInt)static_cast<Object*>((C*)0)) == 0);
        return constructBuiltinObject(
            reinterpret_cast<SPtr<Object>&>(pobj), gname, argc, argv);
    }

    // Constructs a builtin class such as "flash.display.Graphics" and fills
    // in a Value to it. Returns false of constructor failed, which is different from
    // regular Construct since ExecuteCode is not needed for builtins.
    CheckResult ConstructBuiltinValue(Value& v, const char* gname,
                                      unsigned argc = 0, const Value* argv = NULL)
    {
        Construct(gname, v, argc, argv);

        return (!IsException() && !v.IsNullOrUndefined());
    }

private:
    // Private helper to ConstructBuiltinObject
    CheckResult constructBuiltinObject(SPtr<Object> &pobj, const char* gname,
                                       unsigned argc, const Value* argv);    

private:
    // Get C++ Global Object, which holds all buildin classes.
    Instances::GlobalObjectCPP& GetGlobalObjectCPP() const
    {
        return *GlobalObject;
    }
    const Value& GetGlobalObjectCPPValue() const
    {
        return GlobalObjectValue;
    }
    // Get current ActionScript Global Object.
    const Value& GetGlobalObject() const;
    ASStringNode* CreateConstStringNode(const char* msg);

private:
    bool IsInInitializer() const
    {
        return InInitializer > 0;
    }
    void EnterInitializer()
    {
        ++InInitializer;
    }
    void LeaveInitializer()
    {
        --InInitializer;
    }

private:
    enum State {sExecute, sStepInto, sReturn};

    bool ProcessException(Abc::TCodeOffset& cp, CallFrame& cf, State& state)
    {
        int position = OnException(cp, cf);
        if (position < 0)
        {
            // Exception handling code is not in this frame ...
            state = sReturn;
            return true;
        } else
            cp = position;

        return false;
    }
    bool CheckException(Abc::TCodeOffset& cp, CallFrame& cf, State& state)
    {
        return IsException() ? ProcessException(cp, cf, state) : false;
    }
    
    bool NeedToStepInto(UPInt call_stack_size, UInt32 offset, State& state)
    {
        // Check if we have a new call_frame to execute ...
        if (call_stack_size != CallStack.GetSize())
        {
            // Store current offset in a right frame ...
            CallStack[call_stack_size - 1].SetCodeOffset(offset);
            
            state = sStepInto;
            return true;
        }
        
        return false;
    }
    
private:
    int OnException(SInt32 cp, CallFrame& cf);

	void DropCallFrame();

    // Result is put on the stack.
    void Execute(const Value& func, const Value& _this, unsigned argc, const Value* argv, bool discard_result = false);
    // Can throw exceptions.
    // Non-const because of Resolve2Class.
    void Coerce2ReturnType(const Value& value, Value& result);

public:
    // Result is returned as an argument.
    // It is hardly possible that you can screw up VM this way.
    void Execute(const Value& func, const Value& _this,
                 Value& result, unsigned argc, const Value* argv);

    // Execute call and ignore exception, if any, after tracing it to output.
    // This is a helper method for external use.
    void Execute_IgnoreException(const Value& func, const Value& _this,
                                 Value& result, unsigned argc, const Value* argv);

private:
    VM& GetSelf()
    {
        return *this;
    }

    Classes::Error& GetClassError() const;

private:
    // Stack helper, which reads ONE argument form stack ...
    
    ///
    // This stack helper will convert value on stack to the T1 type.
    template <int N, typename T1 = Value>
    struct SH1
    {
        typedef SH1<N, T1> SelfType;

        SH1(OpStackType& _stack)
        : Success(true)
        , _1(Value::ToType<T1>(_stack.Back(), Success))
        , stack(_stack)
        {
        }
        ~SH1()
        {
            stack.PopBack(N);
        }
        
    private:
        bool Success;

    public:
        T1& _1;
        bool IsOK() const { return Success; }
        bool IsException() const { return !Success; }
        
    private:
        SelfType& operator =(const SelfType&);

    private:
        OpStackType& stack;
    };
    // This stack helper will convert value on stack to the T1 type.
    template <typename T1>
    struct SH1<1, T1>
    {
        typedef SH1<1, T1> SelfType;

        SH1(OpStackType& _stack)
        : Success(true)
        , _1(Value::ToType<T1>(_stack.Back(), Success))
        , stack(_stack)
        {
        }
        ~SH1()
        {
            stack.PopBack();
        }
        
    private:
        bool Success;

    public:
        typename Efficient<T1>::type _1;
        bool IsOK() const { return Success; }
        bool IsException() const { return !Success; }
        
    private:
        SelfType& operator =(const SelfType&);

    private:
        OpStackType& stack;
    };
    
    ///
    // This stack helper expects value of type T1 on stack.
    template <int N, typename T1 = Value>
    struct TSH1
    {
        typedef TSH1<N, T1> SelfType;

        TSH1(OpStackType& _stack)
        : _1(_stack.Back())
        , stack(_stack)
        {
        }
        ~TSH1()
        {
            stack.PopBack(N);
        }
        
    public:
        T1& _1;
        
    private:
        SelfType& operator =(const SelfType&);

    private:
        OpStackType& stack;
    };
    // This stack helper expects value of type T1 on stack.
    template <typename T1>
    struct TSH1<1, T1>
    {
        typedef TSH1<1, T1> SelfType;

        TSH1(OpStackType& _stack)
        : _1(_stack.Back())
        , stack(_stack)
        {
        }
        ~TSH1()
        {
            stack.PopBack();
        }
        
        //typename Efficient<T1>::type _1;
        T1 _1;
        
    private:
        SelfType& operator =(const SelfType&);

    private:
        OpStackType& stack;
    };
    
    
    // Stack helper, which reads TWO arguments form stack ...
    // This stack helper will convert values on stack to T1 and T2 types accordingly.
    template <int N, typename T1 = Value, typename T2 = T1>
    struct SH2
    {
        typedef SH2<N, T1, T2> SelfType;

        SH2(OpStackType& _stack)
        : Success(true)
        , _2(Value::ToType<T2>(_stack.Back(), Success))
        , _1(Value::ToType<T1>(_stack[_stack.GetSize() - 2], Success))
        , stack(_stack)
        {
        }
        ~SH2()
        {
            stack.PopBack(N);
        }
        
    private:
        bool Success;

    public:
        T2& _2;
        T1& _1;
        bool IsOK() const { return Success; }
        bool IsException() const { return !Success; }
        
    private:
        SelfType& operator =(const SelfType&);

    private:
        OpStackType& stack;
    };
    // This stack helper will convert values on stack to T1 and T2 types accordingly.
    template <typename T1, typename T2>
    struct SH2<1, T1, T2>
    {
        typedef SH2<1, T1, T2> SelfType;

        SH2(OpStackType& _stack)
        : Success(true)
        , _2(Value::ToType<T2>(_stack.Back(), Success))
        , _1(Value::ToType<T1>(_stack[_stack.GetSize() - 2], Success))
        , stack(_stack)
        {
        }
        ~SH2()
        {
            stack.PopBack();
        }
        
    private:
        bool Success;

    public:
        //typename Efficient<T2>::type _2;
        T2  _2;
        T1& _1;

        bool IsOK() const { return Success; }
        bool IsException() const { return !Success; }
        
    private:
        SelfType& operator =(const SelfType&);

    private:
        OpStackType& stack;
    };
    
    ///
    // This stack helper expects values of types T1 and T2 on stack.
    template <int N, typename T1 = Value, typename T2 = T1>
    struct TSH2
    {
        typedef TSH2<N, T1, T2> SelfType;

        TSH2(OpStackType& _stack)
        : _2(_stack.Back())
        , _1(_stack[_stack.GetSize() - 2])
        , stack(_stack)
        {
        }
        ~TSH2()
        {
            stack.PopBack(N);
        }
        
        T1& _1;
        T2& _2;
        
    private:
        SelfType& operator =(const SelfType&);

    private:
        OpStackType& stack;
    };
    // This stack helper expects values of types T1 and T2 on stack.
    template <typename T1, typename T2>
    struct TSH2<1, T1, T2>
    {
        typedef TSH2<1, T1, T2> SelfType;

        TSH2(OpStackType& _stack)
        : _2(_stack.Back())
        , _1(_stack[_stack.GetSize() - 2])
        , stack(_stack)
        {
        }
        ~TSH2()
        {
            stack.PopBack();
        }
        
        T1& _1;
        //typename Efficient<T2>::type  _2;
        // It is faster this way ...
        T2  _2;
        
    private:
        SelfType& operator =(const SelfType&);

    private:
        OpStackType& stack;
    };
    
    ///
    // This stack helper expects values of types T1 and T2 on stack.
    template <int N, typename T1 = Value, typename T2 = T1>
    struct VTSH2
    {
        typedef VTSH2<N, T1, T2> SelfType;

        VTSH2(OpStackType& _stack)
        : _2(_stack.Back())
        , _1(_stack[_stack.GetSize() - 2])
        , stack(_stack)
        {
        }
        ~VTSH2()
        {
            stack.PopBack(N);
        }
        
        T2 _2;
        T1 _1;
        
    private:
        SelfType& operator =(const SelfType&);

    private:
        OpStackType& stack;
    };
    
private:
    class GCGuard
    {
    public:
        GCGuard(ASRefCountCollector& gc)
            : GC(gc)
        {

        }
        ~GCGuard()
        {
            GC.ForceCollect();
        }

    public:
        ASRefCountCollector& GetGC() const { return GC; }

    private:
        GCGuard& operator =(const GCGuard&);

    private:
        ASRefCountCollector& GC;
    };

private:
    ClassTraits::Traits* GetClassTraits(const ASString& name, const Instances::Namespace& ns);
    ClassTraits::UserDefined& GetUserDefinedTraits(VMAbcFile& file, const Abc::ClassInfo& ci);
    InstanceTraits::Function& GetFunctionInstanceTraits(VMAbcFile& file, const Abc::MethodBodyInfo& mbi);
    InstanceTraits::Activation& GetActivationInstanceTraits(VMAbcFile& file, const Abc::MethodBodyInfo& mbi DEBUG_ARG(const ASString& name));

    // Utility methods.

    void RegisterClassTraits(ClassTraits::Traits& tr);
    // Register all ClassTraits from a file.
    // Return true if a file should be loaded.
    bool RegisterUserDefinedClassTraits(VMAbcFile& file);
    void InitUserDefinedClassTraits(Instances::GlobalObjectScript& gos);

    void RegisterClassTraitsVector(ClassTraits::Traits& tr);

private:
    //enum {MaxCallStackSize = 1024};
    enum {MaxCallStackSize = 128};

    typedef MultinameHash<SPtr<ClassTraits::Traits> > TClassTraitsCache;
    typedef HashLH<const Abc::MethodBodyInfo*, SPtr<InstanceTraits::Function> > TFunctionTraitsCache;
    typedef HashLH<const Abc::MethodBodyInfo*, SPtr<InstanceTraits::Activation> > TActivationTraitsCache;

    AS3::StringManager& StringManagerRef;
    // Garbage Collector;
    GCGuard             GC;

    FlashUI&            UI;
    FileLoader&         Loader;
    int                 InInitializer;
    MemoryHeap* const   MHeap;

    OpStackType         OpStack;
    CallStackType       CallStack;
    RegistersType       RegisterFile;
    ScopeStackType      ScopeStack;
    
    // Exception-handling related ...
    bool  HandleException;
    Value ExceptionObj;

    ASStringHash<SPtr<VMAbcFile> >  Files;
    ScopeStackType                  GlobalObjects;
    bool                            Initialized;

    SF_AMP_CODE(UInt64 ActiveLineTimestamp;)
    SF_AMP_CODE(void SetActiveLine(UInt64 fileId, UInt32 lineNumber);)

    ///////////////////////////////////////////////////////////////////////

    // Public namespace.
    SPtr<Instances::Namespace> PublicNamespace;
    SPtr<Instances::Namespace> AS3Namespace;
    
    // ClassClass.
    SPtr<ClassTraits::ClassClass> TraitsClassClass;
    
    // Object.
    SPtr<ClassTraits::Object> TraitsObject;
    
    // Namespace.
    SPtr<ClassTraits::Namespace> TraitsNamespace;

    // Function.
    SPtr<ClassTraits::Function> TraitsFunction;
    
    // MethodClosure.
    SPtr<ClassTraits::MethodClosure> TraitsMethodClosure;
    
    // Boolean.
    SPtr<ClassTraits::Boolean> TraitsBoolean;
    
    // Number.
    SPtr<ClassTraits::Number> TraitsNumber;
    
    // int.
    SPtr<ClassTraits::int_> TraitsInt;
    
    // uint.
    SPtr<ClassTraits::uint> TraitsUint;
    
    // String.
    SPtr<ClassTraits::String> TraitsString;

    // Array.
    SPtr<ClassTraits::Array> TraitsArray;
    
    // Activation.
    SPtr<ClassTraits::Activation> TraitsActivation;
    
    // Catch.
    SPtr<ClassTraits::Catch> TraitsCatch;

    // Vector.
    SPtr<ClassTraits::Vector> TraitsVector;

    // Vector$int.
    SPtr<ClassTraits::Vector_int> TraitsVector_int;

    // Vector$uint.
    SPtr<ClassTraits::Vector_uint> TraitsVector_uint;

    // Vector$double.
    SPtr<ClassTraits::Vector_double> TraitsVector_Number;

    // Vector$String.
    SPtr<ClassTraits::Vector_String> TraitsVector_String;

    // Vector$Object.
//     SPtr<ClassTraits::Vector_object> TraitsVector_object;
//     SPtr<Classes::Vector_object> ClassVector_Object;
    
    SPtr<InstanceTraits::Traits> TraitsNull;
    SPtr<InstanceTraits::Traits> TraitsVoid;

    TClassTraitsCache ClassTraitsSet;

    SPtr<InstanceTraits::Function> NoFunctionTraits;
    TFunctionTraitsCache FunctionTraitsCache;
    
    SPtr<InstanceTraits::Activation> NoActivationTraits;
    TActivationTraitsCache ActivationTraitsCache;
    
    // CPP GlobalObject ...
    SPtr<InstanceTraits::GlobalObject> TraitaGlobalObject;
    SPtr<Instances::GlobalObjectCPP> GlobalObject;
    // This is the same GlobalObject. We just need it for convenience to return
    // in GetGlobalObject().
    Value GlobalObjectValue;
}; // class VM


///////////////////////////////////////////////////////////////////////////
class InitializerGuard
{
public:
    InitializerGuard(VM& vm)
        : Resource(vm)
    {
        Resource.EnterInitializer();
    }
    ~InitializerGuard()
    {
        Resource.LeaveInitializer();
    }

private:
    InitializerGuard& operator =(const InitializerGuard&);

private:
    VM& Resource;
};

///////////////////////////////////////////////////////////////////////////
template <typename T>
inline
const Class* GetSlotDataType(const VM& vm, const T&)
{
	//vm.Resolve2Class()
	return vm.IsInitialized() ? &vm.GetClassObject() : NULL;
}
template <>
inline
const Class* GetSlotDataType<bool>(const VM& vm, const bool&)
{
	return vm.IsInitialized() ? (const Class*)&vm.GetClassBoolean() : NULL;
}
#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_OS_WII) && !defined(SF_CC_ARM) 
template <>
inline
const Class* GetSlotDataType<int>(const VM& vm, const int&)
{
	return vm.IsInitialized() ? (const Class*)&vm.GetClassInt() : NULL;
}
#endif
template <>
inline
const Class* GetSlotDataType<SInt32>(const VM& vm, const SInt32&)
{
	return vm.IsInitialized() ? (const Class*)&vm.GetClassInt() : NULL;
}
#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_OS_WII) && !defined(SF_CC_ARM)
template <>
inline
const Class* GetSlotDataType<unsigned int>(const VM& vm, const unsigned int&)
{
	return vm.IsInitialized() ? (const Class*)&vm.GetClassUInt() : NULL;
}
#endif
template <>
inline
const Class* GetSlotDataType<UInt32>(const VM& vm, const UInt32&)
{
	return vm.IsInitialized() ? (const Class*)&vm.GetClassUInt() : NULL;
}
template <>
inline
const Class* GetSlotDataType<Value::Number>(const VM& vm, const Value::Number&)
{
	return vm.IsInitialized() ? (const Class*)&vm.GetClassNumber() : NULL;
}
template <>
inline
const Class* GetSlotDataType<Value>(const VM& vm, const Value&)
{
	return vm.IsInitialized() ? (const Class*)&vm.GetClassObject() : NULL;
}
template <>
inline
const Class* GetSlotDataType<String>(const VM& vm, const String&)
{
	return vm.IsInitialized() ? (const Class*)&vm.GetClassString() : NULL;
}
template <>
inline
const Class* GetSlotDataType<ASString>(const VM& vm, const ASString&)
{
    return vm.IsInitialized() ? (const Class*)&vm.GetClassString() : NULL;
}


///////////////////////////////////////////////////////////////////////////
inline
ASString Object::GetASString(const char *pstr) const
{
    return GetVM().GetStringManager().CreateString(pstr);
}

inline
ASString Object::GetASString(const char *pstr, UPInt length) const
{
    return GetVM().GetStringManager().CreateString(pstr, length);
}

inline
ASString Object::GetASString(const String& str) const
{
    return GetVM().GetStringManager().CreateString(str);
}

inline
bool Object::IsException() const
{
    return GetVM().IsException();
}

inline
StringManager& Object::GetStringManager() const
{
    return GetVM().GetStringManager();
}

inline
const Value& Object::GetGlobalObject() const
{
    return GetVM().GetGlobalObject();
}

inline
void Object::ExecuteValue(Value& value, Value& result, unsigned argc, const Value* argv)
{
	GetVM().Execute(value, Value(this), result, argc, argv);
}

inline
CheckResult Object::GetSlotValue(const ASString& prop_name, Instances::Namespace& ns, Value& value)
{
	PropRef prop = AS3::FindObjProperty(GetVM(), Value(this), Multiname(ns, Value(prop_name)));

	return prop && prop.GetSlotValue(GetVM(), value);
}

inline
CheckResult Object::GetSlotValueUnsafe(const ASString& prop_name, Instances::Namespace& ns, Value& value)
{
    PropRef prop = AS3::FindObjProperty(GetVM(), Value(this), Multiname(ns, Value(prop_name)));

    return prop && prop.GetSlotValueUnsafe(GetVM(), value);
}

inline
void Object::AddDynamicFunc(const ThunkInfo& func)
{
    // We do not need namespace here. 
    // It is always public for dynamic properties.

    // Can we use Getters/Setters as dynamic properties?
    SF_ASSERT(func.GetCodeType() == CT_Method);
    SF_ASSERT(func.NamespaceName == NULL || *func.NamespaceName == 0);

    AddDynamicSlotValuePair(
        GetStringManager().CreateConstString(func.Name), 
        func, 
        SlotInfo::aDontEnum
        );
}

///////////////////////////////////////////////////////////////////////////
inline
StringManager& Traits::GetStringManager() const
{
    return GetVM().GetStringManager();
}

inline
const Value& Traits::GetGlobalObject() const
{
    return GetVM().GetGlobalObject();
}

inline
Classes::ClassClass& Traits::GetClassClass() const
{
	return GetVM().GetClassClass();
}

inline
ASString Traits::GetASString(const char *pstr) const
{
    return GetVM().GetStringManager().CreateString(pstr);
}

inline
ASString Traits::GetASString(const char *pstr, UPInt length) const
{
    return GetVM().GetStringManager().CreateString(pstr, length);
}

inline
ASString Traits::GetASString(const String& str) const
{
    return GetVM().GetStringManager().CreateString(str);
}

inline
const SlotInfo* Traits::FindSlotInfo(VMAbcFile& file, const Abc::Multiname& mn) const 
{
    const Instances::Namespace& ns = GetVM().GetInternedNamespace(mn.GetNamespace(file.GetConstPool()));
    return Slots::FindSlotInfo(file.GetInternedString(mn.GetNameInd()), ns);
}

///////////////////////////////////////////////////////////////////////////
inline
SlotInfo::DataType GetDataType(TCodeType ct)
{
    SlotInfo::DataType dt = SlotInfo::DT_Code;
    
    switch (ct)
    {
    case CT_Method:
        dt = SlotInfo::DT_Code;
        break;
    case CT_Get:
        dt = SlotInfo::DT_Get;
        break;
    case CT_Set:
        dt = SlotInfo::DT_Set;
        break;
    }

    return dt;
}

///////////////////////////////////////////////////////////////////////////
inline
CallFrame::CallFrame(
     const Value& invoker,
     VMAbcFile& file,
     const Abc::MethodBodyInfo& _mbi,
     const Abc::HasTraits* _ht,
     const ScopeStackType* _saved_scope,
     Abc::TCodeOffset _offset,
     bool discard_result,
     const Traits* ot
     DEBUG_ARG(const ASString& name)
     DEBUG_ARG(UInt64 startTicks)
 )
     : DiscardResult(discard_result)
     , AdjustedStack(false)
     , ACopy(false)
     DEBUG_ARG(RegisteredFunction(false))
     , OriginalStackSize(file.GetVM().GetOpStack().GetSize())
     , RegBaseInd(file.GetVM().GetRegisters().GetSize())
     , ScopeStackBaseInd(file.GetVM().GetScopeStack().GetSize())
     , Offset(_offset)
     DEBUG_ARG(CurrFileInd(0))
     DEBUG_ARG(CurrLineNumber(0))
     DEBUG_ARG(StartTicks(startTicks))
     , pHeap(file.GetVM().GetMemoryHeap())
     , pFile(&file)
     , pMBI(&_mbi)
     , pHT(_ht)
     , pSavedScope(_saved_scope)
     DEBUG_ARG(Name(name.GetNode()))
     , pRegisterFile(&file.GetVM().GetRegisters())
     , OriginationTraits(ot)
     , pScopeStack(&file.GetVM().GetScopeStack())
     , Invoker(invoker)
{
    pRegisterFile->Resize(pRegisterFile->GetSize() + pMBI->GetMaxLocalRegisterIndex());
}

inline
void CallFrame::ClearOpStack()
{
    GetVM().OpStack.Resize(OriginalStackSize);
}


///////////////////////////////////////////////////////////////////////////
////

template <typename A0>
struct DefArgs1
{
    DefArgs1(A0 a0 = A0())
        : _0(a0)
    {
    }
    DefArgs1& operator=(const DefArgs1&);

    typename Clean<A0>::type _0;
};

template <typename A0, typename A1>
struct DefArgs2 : public DefArgs1<A0>
{
    DefArgs2(A0 a0 = A0(), A1 a1 = A1())
        : DefArgs1<A0>(a0)
        , _1(a1)
    {
    }
    DefArgs2& operator=(const DefArgs2&);

    typename Clean<A1>::type _1;
};

template <typename A0, typename A1, typename A2>
struct DefArgs3 : public DefArgs2<A0, A1>
{
    DefArgs3(A0 a0 = A0(), A1 a1 = A1(), A2 a2 = A2())
        : DefArgs2<A0, A1>(a0, a1)
        , _2(a2)
    {
    }
    DefArgs3& operator=(const DefArgs3&);

    typename Clean<A2>::type _2;
};

template <typename A0, typename A1, typename A2, typename A3>
struct DefArgs4 : public DefArgs3<A0, A1, A2>
{
    DefArgs4(A0 a0 = A0(), A1 a1 = A1(), A2 a2 = A2(), A3 a3 = A3())
        : DefArgs3<A0, A1, A2>(a0, a1, a2)
        , _3(a3)
    {
    }
    DefArgs4& operator=(const DefArgs4&);

    typename Clean<A3>::type _3;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4>
struct DefArgs5 : public DefArgs4<A0, A1, A2, A3>
{
    DefArgs5(A0 a0 = A0(), A1 a1 = A1(), A2 a2 = A2(), A3 a3 = A3(), A4 a4 = A4())
        : DefArgs4<A0, A1, A2, A3>(a0, a1, a2, a3)
        , _4(a4)
    {
    }
    DefArgs5& operator=(const DefArgs5&);

    typename Clean<A4>::type _4;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct DefArgs6 : public DefArgs5<A0, A1, A2, A3, A4>
{
    DefArgs6(A0 a0 = A0(), A1 a1 = A1(), A2 a2 = A2(), A3 a3 = A3(), A4 a4 = A4(), A5 a5 = A5())
        : DefArgs5<A0, A1, A2, A3, A4>(a0, a1, a2, a3, a4)
        , _5(a5)
    {
    }
    DefArgs6& operator=(const DefArgs6&);

    typename Clean<A5>::type _5;
};


///////////////////////////////////////////////////////////////////////////
template <class C, int M, typename A0>
inline
DefArgs1<A0> MakeDefArgs(AS3::StringManager& sm)
{
    return DefArgs1<A0>(
        Impl::GetMethodDefArg<C, M, 0, A0>(sm)
        );
}

template <class C, int M, typename A0, typename A1>
inline
DefArgs2<A0, A1> MakeDefArgs(AS3::StringManager& sm)
{
    return DefArgs2<A0, A1>(
        Impl::GetMethodDefArg<C, M, 0, A0>(sm),
        Impl::GetMethodDefArg<C, M, 1, A1>(sm)
        );
}

template <class C, int M, typename A0, typename A1, typename A2>
inline
DefArgs3<A0, A1, A2> MakeDefArgs(AS3::StringManager& sm)
{
    return DefArgs3<A0, A1, A2>(
        Impl::GetMethodDefArg<C, M, 0, A0>(sm),
        Impl::GetMethodDefArg<C, M, 1, A1>(sm),
        Impl::GetMethodDefArg<C, M, 2, A2>(sm)
        );
}

template <class C, int M, typename A0, typename A1, typename A2, typename A3>
inline
DefArgs4<A0, A1, A2, A3> MakeDefArgs(AS3::StringManager& sm)
{
    return DefArgs4<A0, A1, A2, A3>(
        Impl::GetMethodDefArg<C, M, 0, A0>(sm),
        Impl::GetMethodDefArg<C, M, 1, A1>(sm),
        Impl::GetMethodDefArg<C, M, 2, A2>(sm),
        Impl::GetMethodDefArg<C, M, 3, A3>(sm)
        );
}

template <class C, int M, typename A0, typename A1, typename A2, typename A3, typename A4>
inline
DefArgs5<A0, A1, A2, A3, A4> MakeDefArgs(AS3::StringManager& sm)
{
    return DefArgs5<A0, A1, A2, A3, A4>(
        Impl::GetMethodDefArg<C, M, 0, A0>(sm),
        Impl::GetMethodDefArg<C, M, 1, A1>(sm),
        Impl::GetMethodDefArg<C, M, 2, A2>(sm),
        Impl::GetMethodDefArg<C, M, 3, A3>(sm),
        Impl::GetMethodDefArg<C, M, 4, A4>(sm)
        );
}

template <class C, int M, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
inline
DefArgs6<A0, A1, A2, A3, A4, A5> MakeDefArgs(AS3::StringManager& sm)
{
    return DefArgs6<A0, A1, A2, A3, A4, A5>(
        Impl::GetMethodDefArg<C, M, 0, A0>(sm),
        Impl::GetMethodDefArg<C, M, 1, A1>(sm),
        Impl::GetMethodDefArg<C, M, 2, A2>(sm),
        Impl::GetMethodDefArg<C, M, 3, A3>(sm),
        Impl::GetMethodDefArg<C, M, 4, A4>(sm),
        Impl::GetMethodDefArg<C, M, 5, A5>(sm)
        );
}

///////////////////////////////////////////////////////////////////////////
// Args does not depend on T and N template arguments.

template <typename R>
class Args0
{
public:
    Args0(VM& vm, Value& result)
        : Vm(vm)
        , Result(result)
        , r(DefaultValue<R>(GetStringManager()))
    {
    }
    ~Args0()
    {
        if (GetVM().IsException())
            return;

        AS3::Convert(Result, r, GetStringManager());
    }

public:
    VM& GetVM() const
    {
        return Vm;
    }
    StringManager& GetStringManager() const
    {
        return GetVM().GetStringManager();
    }

private:
    Args0& operator =(const Args0&);

private:
    VM& Vm;

public:
    Value& Result;
    R r;
};

template <>
class Args0<Value>
{
public:
    Args0(VM& vm, Value& result)
        : Vm(vm)
        , r(result)
    {
    }
    ~Args0()
    {
        // No conversion is necessary.
    }

public:
    VM& GetVM() const
    {
        return Vm;
    }
    StringManager& GetStringManager() const
    {
        return GetVM().GetStringManager();
    }

private:
    Args0& operator =(const Args0&);

private:
    VM& Vm;

public:
    Value& r;
};

////
template <typename R, typename A0>
class Args1 : public Args0<R>
{
public:
    Args1(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs1<A0>& da)
        : Args0<R>(vm, result)
        , a0(da._0)
    {
        // !!! Use ReadArg here.
        ReadArgValue<A0>(0, a0, argc, argv, vm.GetStringManager());
    }

private:
    Args1& operator =(const Args1&);

public:
    typename Clean<A0>::type a0;
};

template <typename R>
class Args1<R, Value> : public Args0<R>
{
public:
    Args1(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs1<Value>& da)
        : Args0<R>(vm, result)
        , a0(argc > 0 ? argv[0] : da._0)
    {
    }

private:
    Args1& operator =(const Args1&);

public:
    const Value& a0;
};

template <typename R>
class Args1<R, const Value&> : public Args0<R>
{
public:
    Args1(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs1<const Value&>& da)
        : Args0<R>(vm, result)
        , a0(argc > 0 ? argv[0] : da._0)
    {
    }

private:
    Args1& operator =(const Args1&);

public:
    const Value& a0;
};

////
template <typename R, typename A0, typename A1>
class Args2 : public Args1<R, A0>
{
public:
    Args2(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs2<A0, A1>& da)
        : Args1<R, A0>(vm, result, argc, argv, da)
        , a1(da._1)
    {
        if (vm.IsException())
            return;

        ReadArgValue<A1>(1, a1, argc, argv, vm.GetStringManager());
    }

private:
    Args2& operator =(const Args2&);

public:
    typename Clean<A1>::type a1;
};

template <typename R, typename A0>
class Args2<R, A0, Value> : public Args1<R, A0>
{
public:
    Args2(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs2<A0, Value>& da)
        : Args1<R, A0>(vm, result, argc, argv, da)
        , a1(argc > 1 ? argv[1] : da._1)
    {
    }

private:
    Args2& operator =(const Args2&);

public:
    const Value& a1;
};

template <typename R, typename A0>
class Args2<R, A0, const Value&> : public Args1<R, A0>
{
public:
    Args2(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs2<A0, const Value&>& da)
        : Args1<R, A0>(vm, result, argc, argv, da)
        , a1(argc > 1 ? argv[1] : da._1)
    {
    }

private:
    Args2& operator =(const Args2&);

public:
    const Value& a1;
};

////
template <typename R, typename A0, typename A1, typename A2>
class Args3 : public Args2<R, A0, A1>
{
public:
    Args3(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs3<A0, A1, A2>& da)
        : Args2<R, A0, A1>(vm, result, argc, argv, da)
        , a2(da._2)
    {
        if (vm.IsException())
            return;

        ReadArgValue<A2>(2, a2, argc, argv, vm.GetStringManager());
    }

private:
    Args3& operator =(const Args3&);

public:
    typename Clean<A2>::type a2;
};

template <typename R, typename A0, typename A1>
class Args3<R, A0, A1, Value> : public Args2<R, A0, A1>
{
public:
    Args3(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs3<A0, A1, Value>& da)
        : Args2<R, A0, A1>(vm, result, argc, argv, da)
        , a2(argc > 2 ? argv[2] : da._2)
    {
    }

private:
    Args3& operator =(const Args3&);

public:
    const Value& a2;
};

template <typename R, typename A0, typename A1>
class Args3<R, A0, A1, const Value&> : public Args2<R, A0, A1>
{
public:
    Args3(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs3<A0, A1, const Value&>& da)
        : Args2<R, A0, A1>(vm, result, argc, argv, da)
        , a2(argc > 2 ? argv[2] : da._2)
    {
    }

private:
    Args3& operator =(const Args3&);

public:
    const Value& a2;
};

////
template <typename R, typename A0, typename A1, typename A2, typename A3>
class Args4 : public Args3<R, A0, A1, A2>
{
public:
    Args4(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs4<A0, A1, A2, A3>& da)
        : Args3<R, A0, A1, A2>(vm, result, argc, argv, da)
        , a3(da._3)
    {
        if (vm.IsException())
            return;

        ReadArgValue<A3>(3, a3, argc, argv, vm.GetStringManager());
    }

private:
    Args4& operator =(const Args4&);

public:
    typename Clean<A3>::type a3;
};

////
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
class Args5 : public Args4<R, A0, A1, A2, A3>
{
public:
    Args5(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs5<A0, A1, A2, A3, A4>& da)
        : Args4<R, A0, A1, A2, A3>(vm, result, argc, argv, da)
        , a4(da._4)
    {
        if (vm.IsException())
            return;

        ReadArgValue<A4>(4, a4, argc, argv, vm.GetStringManager());
    }

private:
    Args5& operator =(const Args5&);

public:
    typename Clean<A4>::type a4;
};

////
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
class Args6 : public Args5<R, A0, A1, A2, A3, A4>
{
public:
    Args6(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs6<A0, A1, A2, A3, A4, A5>& da)
        : Args5<R, A0, A1, A2, A3, A4>(vm, result, argc, argv, da)
        , a5(da._5)
    {
        if (vm.IsException())
            return;

        ReadArgValue<A5>(5, a5, argc, argv, vm.GetStringManager());
    }

private:
    Args6& operator =(const Args6&);

public:
    typename Clean<A5>::type a5;
};

///////////////////////////////////////////////////////////////////////////
// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R>
void ThunkFunc0<T, N, R>::Func(VM& vm, const Value& obj, Value& result, unsigned /*argc*/, const Value* /*argv*/)
{
    T& _this = static_cast<T&>(*(obj.IsClosure() ? obj.GetClosure() : obj.GetObject()));
    Args0<R> args(vm, result);

    (_this.*Method)(args.r);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0>
void ThunkFunc1<T, N, R, A0>::Func(VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*(obj.IsClosure() ? obj.GetClosure() : obj.GetObject()));
    Args1<R, A0> args(vm, result, argc, argv, MakeDefArgs<T, N, A0>(vm.GetStringManager()));

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1>
void ThunkFunc2<T, N, R, A0, A1>::Func(VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*(obj.IsClosure() ? obj.GetClosure() : obj.GetObject()));
    Args2<R, A0, A1> args(vm, result, argc, argv, MakeDefArgs<T, N, A0, A1>(vm.GetStringManager()));

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2>
void ThunkFunc3<T, N, R, A0, A1, A2>::Func(VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*(obj.IsClosure() ? obj.GetClosure() : obj.GetObject()));
    Args3<R, A0, A1, A2> args(vm, result, argc, argv, MakeDefArgs<T, N, A0, A1, A2>(vm.GetStringManager()));

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1, args.a2);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2, typename A3>
void ThunkFunc4<T, N, R, A0, A1, A2, A3>::Func(VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*(obj.IsClosure() ? obj.GetClosure() : obj.GetObject()));
    Args4<R, A0, A1, A2, A3> args(vm, result, argc, argv, MakeDefArgs<T, N, A0, A1, A2, A3>(vm.GetStringManager()));

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1, args.a2, args.a3);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
void ThunkFunc5<T, N, R, A0, A1, A2, A3, A4>::Func(VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*(obj.IsClosure() ? obj.GetClosure() : obj.GetObject()));
    Args5<R, A0, A1, A2, A3, A4> args(vm, result, argc, argv, MakeDefArgs<T, N, A0, A1, A2, A3, A4>(vm.GetStringManager()));

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1, args.a2, args.a3, args.a4);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
void ThunkFunc6<T, N, R, A0, A1, A2, A3, A4, A5>::Func(VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*(obj.IsClosure() ? obj.GetClosure() : obj.GetObject()));
    Args6<R, A0, A1, A2, A3, A4, A5> args(vm, result, argc, argv, MakeDefArgs<T, N, A0, A1, A2, A3, A4, A5>(vm.GetStringManager()));

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1, args.a2, args.a3, args.a4, args.a5);
}


}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_VM_H
