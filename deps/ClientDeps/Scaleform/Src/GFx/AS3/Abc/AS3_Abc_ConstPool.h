/**********************************************************************

Filename    :   AS3_Abc_ConstPool.h
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

#ifndef INC_AS3_Abc_ConstPool_H
#define INC_AS3_Abc_ConstPool_H


#include "AS3_Abc_Type.h"
#include "../AS3_Index.h"

///////////////////////////////////////////////////////////////////////////////
// Forward declaration.
namespace Scaleform { namespace GFx { namespace AS3 
{
    class VM;
}}} // namespace Scaleform { namespace GFx { namespace AS3 {

///////////////////////////////////////////////////////////////////////////////
namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc 
{

///////////////////////////////////////////////////////////////////////////
// Forward declaration.
class ConstPool;

///////////////////////////////////////////////////////////////////////////
enum AbcKind {
    AbcKind_unused              = 0x00,
    AbcKind_Utf8                = 0x01,
    AbcKind_Int                 = 0x03,
    AbcKind_UInt                = 0x04,
    AbcKind_PrivateNs           = 0x05, // non-shared namespace
    AbcKind_Double              = 0x06,
    AbcKind_Qname               = 0x07, // o.ns::name, ct ns, ct name
    AbcKind_Namespace           = 0x08,
    AbcKind_Multiname           = 0x09, // o.name, ct nsset, ct name
    AbcKind_False               = 0x0A,
    AbcKind_True                = 0x0B,
    AbcKind_Null                = 0x0C,
    AbcKind_QnameA              = 0x0D, // o.@ns::name, ct ns, ct attr-name
    AbcKind_MultinameA          = 0x0E, // o.@name, ct attr-name
    AbcKind_RTQname             = 0x0F, // o.ns::name, rt ns, ct name
    AbcKind_RTQnameA            = 0x10, // o.@ns::name, rt ns, ct attr-name
    AbcKind_RTQnameL            = 0x11, // o.ns::[name], rt ns, rt name
    AbcKind_RTQnameLA           = 0x12, // o.@ns::[name], rt ns, rt attr-name
    AbcKind_NamespaceSet        = 0x15,
    AbcKind_PackageNamespace    = 0x16,
    AbcKind_PackageInternalNs   = 0x17,
    AbcKind_ProtectedNamespace  = 0x18,
    AbcKind_ExplicitNamespace   = 0x19,
    AbcKind_StaticProtectedNs   = 0x1A,
    AbcKind_MultinameL          = 0x1B,
    AbcKind_MultinameLA         = 0x1C,
    AbcKind_TypeName            = 0x1D
};

///////////////////////////////////////////////////////////////////////////

// NamespaceInfo describes a namespace entry in a constant pool, it combines a NamespaceKind 
// with URIString. Non-public namespaces receive a non-empty compiler-generated NameURI.
// Namespaces are compared based on {NamespaceKind, NameURI}, with exception of Private namespaces
// which must be unique across ABCs.

class NamespaceInfo : public NewOverrideBase<Mem_Stat>
{
    friend class Reader;
    friend bool operator ==(const NamespaceInfo& l, const NamespaceInfo& r);
public:
    NamespaceInfo()    
    : Kind(NS_Undefined)
    {
    }
    NamespaceInfo(NamespaceKind k, const String& uri)    
    : Kind(k)
    , NameURI(uri)
    {
    }

public:

    bool HasKind() const
    {
        return Kind != NS_Undefined;
    }
    
    NamespaceKind GetKind() const
    {
        SF_ASSERT(IsValidKind());
        return Kind;
    }
    // User-defined namespaces have names. All other don't.
    bool IsUserDefined() const
    {
        return (GetKind() == NS_Public || GetKind() == NS_Explicit);
    }
    // This method is required for multiname resolution ...
    bool IsPublic() const
    {
        return IsUserDefined();
    }
    const String& GetNameURI() const
    {
        return NameURI;
    }
    
private:
    // For debugging ...
    bool IsValidKind() const
    {
        return Kind != NS_Undefined;
    }
    
private:    
    NamespaceKind   Kind;
    String          NameURI;
};

///////////////////////////////////////////////////////////////////////////
inline
bool operator ==(const NamespaceInfo& l, const NamespaceInfo& r)
{
    // MA TBD: This won't work for Privates
    return (l.Kind == r.Kind && l.NameURI == r.NameURI);
}

///////////////////////////////////////////////////////////////////////////
class NamespaceSetInfo : public NewOverrideBase<Mem_Stat>
{
    friend class Reader;

public:
    NamespaceSetInfo(MemoryHeap* heap)
        : NS(heap)
    {
    }

public:
    UPInt GetSize() const
    {
        return NS.GetSize();
    }
    const NamespaceInfo& Get(const ConstPool& cp, UPInt num) const;
    int GetNamespaceInd(UPInt num) const
    {
        return NS[num];
    }

private:
    ArrayDH_POD<int> NS;
};

///////////////////////////////////////////////////////////////////////////

// MultinameKindBits describe representation of MultinameKind.
// MultinameKind is broken down into Namespace, Name, and Attribute parts.
//  - Namespace can be qualified (Q), runtime-qualified (RTQ),
//    or a namespace-set (Multi).
//  - Name can be explicit (Name) or runtime/"late" (NameL).
//  - (A) suffix indicates that a name refers to an XML attribute,
//   as in "node.@attrib" access.
// TBD: MN_Typename is a separate value; need research what it's used for.
// Perhaps Typename describes parameterized type such as Vector.<int>?

enum MultinameKindBits
{
    MN_NS_Mask      = 0x03, // Namespace type bits.
    MN_NS_Qualified = 0x00,
    MN_NS_Runtime   = 0x01,
    MN_NS_Multi     = 0x02,
    MN_NameLate     = 0x04, // Late name binding flag.
    MN_Attr         = 0x08  // Attribute present in name flag.
};

// MultinameKind describes different types of Multiname.
enum MultinameKind
{
    MN_QName        = MN_NS_Qualified,
    MN_QNameA       = MN_NS_Qualified | MN_Attr,
    MN_RTQName      = MN_NS_Runtime,
    MN_RTQNameA     = MN_NS_Runtime | MN_Attr,
    MN_RTQNameL     = MN_NS_Runtime | MN_NameLate,
    MN_RTQNameLA    = MN_NS_Runtime | MN_NameLate | MN_Attr,
    MN_Multiname    = MN_NS_Multi,
    MN_MultinameA   = MN_NS_Multi | MN_Attr,
    MN_MultinameL   = MN_NS_Multi | MN_NameLate,
    MN_MultinameLA  = MN_NS_Multi | MN_NameLate | MN_Attr,
    MN_Typename     = 0x10,
    MN_Invalid      = 0x20 // Set on loading error.
};


// HasMultinameKind class holds MultinameKind and is a base class 
// for Abc::Multiname and AS3::Multiname, which is its runtime version.

class HasMultinameKind
{
public:
    HasMultinameKind(MultinameKind kind) : Kind(kind) { }

    MultinameKind GetKind() const
    {
        SF_ASSERT(isValidKind());
        return Kind;
    }
    
    bool IsNameLate() const
    {
        return (GetKind() & MN_NameLate) != 0;
    }

    bool IsQName() const
    {
        const MultinameKindBits bits = getNamespaceBits();
        // MN_NS_Runtime is also a qualified name.
        // Qualified name is an antonym to MN_NS_Multi.
        return bits == MN_NS_Qualified || bits == MN_NS_Runtime;
    }
    bool IsRunTime() const
    {
        return getNamespaceBits() == MN_NS_Runtime;
    }
    bool IsMultiname() const
    {
        return getNamespaceBits() == MN_NS_Multi;
    }
    
    bool IsCompileTime() const
    {
        return !IsRunTime() && !IsNameLate();
    }

    bool IsTypename() const
    {
        return GetKind() == MN_Typename;
    }

protected:
    bool isValidKind() const
    {
        return Kind != MN_Invalid;
    }
    MultinameKindBits getNamespaceBits() const
    {
        return (MultinameKindBits)(Kind & MN_NS_Mask);
    }
    void setNamespaceBits(MultinameKindBits bits)
    {
        Kind = (MultinameKind)((Kind & ~MN_NS_Mask) | bits);
    }    
    void setKind(MultinameKind kind)
    {
        Kind = kind;
    }

private:
    MultinameKind Kind;
};


// Multiname describes a name used for identifier lookup in the VM,
// combining Namespace/NamespaceSet and identifier name information.
//
// Information in Multiname may be not complete if it a Runtime Multiname
// (MN_NS_Runtime).

class Multiname : public NewOverrideBase<Mem_Stat>, public HasMultinameKind
{
    friend class Reader;
    friend int Read(AS3::VM& vm, Multiname& obj);

public:
    explicit Multiname(MultinameKind k = MN_QName,
                       SInt32 name_ind = 0, SInt32 namespace_ind = -1)
    : HasMultinameKind(k)
    , NsInd(namespace_ind)
    , NameIndex(name_ind)
    , NextIndex(-1)
    {
    }

    // Name is not always available.
    SInt32 GetNameInd() const
    {
        SF_ASSERT(NameIndex >= 0);
        return NameIndex;
    }
    const String& GetName(const ConstPool& cp) const;

    //
    const NamespaceInfo& GetNamespace(const ConstPool& cp) const;
    SPInt GetNamespaceInd() const
    {
        return NsInd;
    }

    //
    const NamespaceSetInfo& GetNamespaceSetInfo(const ConstPool& cp) const;
    SPInt GetNamespaceSetInd() const
    {
        return NsSetInd;
    }
    
    //
    const Multiname& GetNextMultiname(const ConstPool& cp) const;
    SPInt GetNextMultinameIndex() const
    {
        return NextIndex;
    }

private:
    // Accessor for Read()
    void setKind(MultinameKind kind)
    {
        HasMultinameKind::setKind(kind);
    }

    union 
    {
        SInt32 NsInd;
        SInt32 NsSetInd;
    };
    SInt32  NameIndex;
    SInt32  NextIndex;
};

///////////////////////////////////////////////////////////////////////////
class ConstPool : public NewOverrideBase<Mem_Stat> // Not Moveable !!!
{
    friend class Reader;

public:
    typedef ArrayLH<Multiname> MultinamesType;
    typedef ArrayLH<String> StringsType;
    
public:
    ConstPool();
    
public:
    ConstPool& GetSelf()
    {
        return *this;
    }
    const ConstPool& GetSelf() const
    {
        return *this;
    }
    
    SInt32 GetInt(UPInt ind) const
    {
        // The “0” entry of the integer array is not present in the abcFile; 
        // it represents the zero value for the purposes of providing values for 
        // optional parameters and field initialization.
        SF_ASSERT(ind < const_int.GetSize());

        // const_uint[0] == 0
        return const_int[ind];
    }
    UPInt GetIntCount() const
    {
        return const_int.GetSize();
    }
    
    UInt32 GetUInt(UPInt ind) const
    {
        // The “0” entry of the uinteger array is not present in the abcFile; 
        // it represents the zero value for the purposes of providing values 
        // for optional parameters and field initialization.
        SF_ASSERT(ind < const_uint.GetSize());

        // const_uint[0] == 0
        return const_uint[ind];
    }
    UPInt GetUIntCount() const
    {
        return const_uint.GetSize();
    }
    
    double GetDouble(UPInt ind) const
    {
        // The “0” entry of the double array is not present in the abcFile; 
        // it represents the NaN (Not-a-Number) value for the purposes
        // of providing values for optional parameters and field initialization.
        SF_ASSERT(ind < const_double.GetSize());

        // const_double[0] == NaN
        return const_double[ind];
    }
    UPInt GetDoubleCount() const
    {
        return const_double.GetSize();
    }
    
    const String& GetString(AbsoluteIndex ind) const
    {
        // Entry “0” of the string array is not present in the abcFile; it
        // represents the empty string in most contexts but is also used to 
        // represent the “any” name in others (known as “*” in ActionScript).
        SF_ASSERT(ind.IsValid() && ind.Get() < const_str.GetSize());

        // const_str[0] == empty string.
        return const_str[ind.Get()];
    }
    UPInt GetStringCount() const
    {
        return const_str.GetSize();
    }
    const StringsType& GetStrings() const
    {
        return const_str;
    }
    
    const NamespaceInfo& GetNamespace(UPInt ind) const
    {
        if (ind == 0)
            return GetAnyNamespace();
        
        // Entry “0” of the namespace array is not present in the abcFile; 
        // it represents the “any” namespace (known as “*” in ActionScript).
        SF_ASSERT(ind > 0 && ind < const_namespace.GetSize());
        return const_namespace[ind];
    }
    UPInt GetNamespaceCount() const
    {
        return const_namespace.GetSize();
    }
    
    const NamespaceSetInfo& GetNamespaceSet(UPInt ind) const
    {
        // The “0” entry of the ns_set array is not
        // present in the abcFile.
        SF_ASSERT(ind > 0 && ind < const_ns_set.GetSize());
        return const_ns_set[ind];
    }
    UPInt GetNamespaceSetCount() const
    {
        return const_ns_set.GetSize();
    }
    
    const MultinamesType& GetMultinames() const 
    {
        return const_multiname;
    }
    const Multiname& GetMultiname(UPInt ind) const
    {
        // The “0” entry of the multiname array is not
        // present in the abcFile.
        SF_ASSERT(ind > 0 && ind < const_multiname.GetSize());
        return const_multiname[ind];
    }
    UPInt GetMultinameCount() const
    {
        return const_multiname.GetSize();
    }
    
    const NamespaceInfo& GetAnyNamespace() const 
    {
        return any_namespace;
    }
    const Multiname& GetAnyType() const
    {
        return any_type;
    }

public:
    void Clear()
    {
        const_int.Clear();
        const_uint.Clear();
        const_double.Clear();
        const_str.Clear();
        const_namespace.Clear();
        const_ns_set.Clear();
        const_multiname.Clear();
    }

private:
    ConstPool& operator =(const ConstPool&);

private:
    ArrayLH_POD<SInt32> const_int;
    ArrayLH_POD<UInt32> const_uint;
    ArrayLH_POD<double> const_double;
    StringsType const_str;
    ArrayLH<NamespaceInfo> const_namespace;
    ArrayCC<NamespaceSetInfo> const_ns_set;
    MultinamesType const_multiname;
    const NamespaceInfo any_namespace;
    const Multiname any_type;
};

///////////////////////////////////////////////////////////////////////////
// Inline methods ...

inline
const String& Multiname::GetName(const ConstPool& cp) const
{
    return cp.GetString(AbsoluteIndex(NameIndex));
}

inline
const NamespaceInfo& Multiname::GetNamespace(const ConstPool& cp) const
{
    SF_ASSERT(IsQName());
    
    //if (!ns_ind)
    //  return any_name;

    return cp.GetNamespace(NsInd);
}

inline
const NamespaceSetInfo& Multiname::GetNamespaceSetInfo(const ConstPool& cp) const
{
    SF_ASSERT(IsMultiname());
    SF_ASSERT(NsSetInd > 0);
    return cp.GetNamespaceSet(NsSetInd);
}

inline
const Multiname& Multiname::GetNextMultiname(const ConstPool& cp) const
{
    SF_ASSERT(IsQName());
    SF_ASSERT(GetNextMultinameIndex() >= 0);

    return cp.GetMultiname(GetNextMultinameIndex());
}

///////////////////////////////////////////////////////////////////////////
inline
const NamespaceInfo& NamespaceSetInfo::Get(const ConstPool& cp, UPInt ind) const
{
    SF_ASSERT(ind < NS.GetSize());
    
    return cp.GetNamespace(GetNamespaceInd(ind));
}

}}}} // namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc {

#endif // INC_AS3_Abc_ConstPool_H
