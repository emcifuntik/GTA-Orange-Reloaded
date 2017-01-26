/**********************************************************************

Filename    :   AS3_Abc_Type.h
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

#ifndef INC_AS3_Abc_Type_H
#define INC_AS3_Abc_Type_H

#include "Kernel/SF_Array.h"
#include "Kernel/SF_ArrayPaged.h"
#include "Kernel/SF_StringHash.h"
#include "Kernel/SF_Alg.h"
#include "../AS3_StringManager.h"

#ifdef SF_BUILD_DEBUG
#define USE_DEBUGGER
#endif

// #define ENABLE_LOG_VM

///////////////////////////////////////////////////////////////////////////////
#if 0
    #define REPORT_NOT_IMPLEMENTED
#else
/*
    // Minimalistic version of VM.
    #define AS3_NO_FLASH
    #ifdef GFX_AS3_VERBOSE
        #undef GFX_AS3_VERBOSE
    #endif
*/
#endif

#ifdef REPORT_NOT_IMPLEMENTED
    #define NOT_IMPLEMENTED(x) GetVM().ThrowError(VM::eNotImplementedYet, x);
    #define NOT_IMPLEMENTED_STATIC(x) vm.ThrowError(VM::eNotImplementedYet, x);
#else
    #define NOT_IMPLEMENTED(x) GetVM().GetUI().Output(FlashUI::Output_Warning, "Not implemented " x);
    #define NOT_IMPLEMENTED_STATIC(x) vm.GetUI().Output(FlashUI::Output_Warning, "Not implemented " x);
#endif

///////////////////////////////////////////////////////////////////////////////
#if defined(USE_DEBUGGER) || defined (SF_AMP_SERVER)

#define DEBUG_ARG(x) , x
#define DEBUG_CODE(x) x

#else

#define DEBUG_ARG(x)
#define DEBUG_CODE(x)

#endif


namespace Scaleform
{

///////////////////////////////////////////////////////////////////////////
enum { Mem_Stat = Stat_Default_Mem };


///////////////////////////////////////////////////////////////////////////
// String utilities.

inline
String operator +(const char* l, const String& r)
{
    return String(l, r);
}

///////////////////////////////////////////////////////////////////////////
inline
String operator +(const char* l, const GFx::ASString& r)
{
    return String(l) + String(r.ToCStr(), r.GetSize());
}

inline
String operator +(const String& l, const GFx::ASString& r)
{
    return l + String(r.ToCStr(), r.GetSize());
}

///////////////////////////////////////////////////////////////////////////
template <typename T>
inline
String& operator <<(String& str, T v)
{
    const String sv = AsString(v);
    str.AppendString(sv.ToCStr(), sv.GetSize());
    return str;
}

template <>
inline
String& operator <<(String& str, const char* v)
{
    str.AppendString(v);
    return str;
}

template <>
inline
String& operator <<(String& str, const String& v)
{
    str.AppendString(v.ToCStr(), v.GetSize());
    return str;
}

template <>
inline
String& operator <<(String& str, const GFx::ASString& v)
{
    str.AppendString(v.ToCStr(), v.GetSize());
    return str;
}


} // namespace Scaleform


///////////////////////////////////////////////////////////////////////////
namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
// Forward declaration.
class Object;
class Class;
class Value;

///////////////////////////////////////////////////////////////////////////
template <typename T>
struct Clean
{
    typedef T type;
};

template <typename T>
struct Clean<const T>
{
    typedef T type;
};

template <typename T>
struct Clean<T&>
{
    typedef T type;
};

template <typename T>
struct Clean<const T&>
{
    typedef T type;
};

///////////////////////////////////////////////////////////////////////////
enum PickType { PickValue };

template <typename T>
class Pickable
{
public:
    Pickable() : pV(NULL) {}
    explicit Pickable(T* p) : pV(p) {}
    Pickable(T* p, PickType) : pV(p) 
    {
        SF_ASSERT(pV);
        if (pV)
            pV->AddRef();
    }
    template <typename OT>
    Pickable(const Pickable<OT>& other) : pV(other.GetPtr()) {}
    //Pickable(const Pickable<typename Clean<T>::type>& other) : pV(other.GetPtr()) {}

public:
    Pickable& operator =(const Pickable& other)
    {
        SF_ASSERT(pV == NULL);
        pV = other.pV;
        // Extra check.
        //other.pV = NULL;
        return *this;
    }

public:
    T* GetPtr() const { return pV; }
    T* operator->() const
    {
        return pV;
    }
    T& operator*() const
    {
        SF_ASSERT(pV);
        return *pV;
    }

private:
    T* pV;
};

template <typename T>
inline
Pickable<T> MakePickable(T* p)
{
    return Pickable<T>(p);
}

/////////////////////////////////////////////////////////////////////////// 

// ***** SPtr<>

// SPtr is a smart pointer used for garbage-collectible classes within the VM.
// A special aspect of this class is that it uses lower bit of the pointer
// value as a marker to disable the next Release() call as needed for
// garbage collection of cycles. This bit is only applied by the garbage
// collector, allowing object destructor to be used instead of having an
// explicit finalize method, which would be error-prone.
//
// All member SPtr<> references to child objects must be referenced from
// the ForEachChild_GC() virtual function of that class for collector to
// see it properly.

template <typename T>
class SPtr
{
public:
    friend class Value;
    template <typename AT> friend class APtr;

    typedef T       ObjType;
    typedef SPtr<T> SelfType;

public:
    // Constructors.

    SPtr(T* v = NULL)
        : pObject(v)
    {
        SF_ASSERT(v == NULL || (Scaleform::UPInt)v > 4);
        AddRef();
    }
    // Copy constructor.
    SPtr(const SPtr<T>& other)
        : pObject(other.pObject)
    {
        AddRef();
    }
    ~SPtr()
    {
        Release();
    }

public:
    // Pick constructors.
    // We save on AddRef().

    SPtr(SelfType& other, PickType)
        : pObject(other.pObject)
    {
        other.pObject = NULL;
        // No AddRef() on purpose.
    }
    SPtr(Pickable<T> v)
        : pObject(v.GetPtr())
    {
        SF_ASSERT(v.GetPtr() == NULL || (Scaleform::UPInt)v.GetPtr() > 4);
        // No AddRef() on purpose.
    }

public:
    // Assignment operator.
    SelfType& operator =(const SelfType& other)
    {
        return Set(other);
    }
    SelfType& operator =(T* p)
    {
        return SetPtr(p);
    }
    SelfType& operator =(Pickable<T> v)
    {
        return Pick(v);
    }

public:
    // Pickers.

    // Pick a value.
    SelfType& Pick(SelfType& other)
    {
        if (&other != this)
        {
            Release();

            pObject = other.pObject;
            other.pObject = NULL;
        }

        return *this;
    }

    SelfType& Pick(Pickable<T> v)
    {
        if (v.GetPtr() != pObject)
        {
            Release();

            pObject = v.GetPtr();
        }

        return *this;
    }

    SelfType& Pick(T* p)
    {
        if (p != pObject)
        {
            Release();

            pObject = p;
        }

        return *this;
    }

public:
    // Setters.

    SelfType& SetPtr(T* p)
    {
        if (p != pObject)
        {
            SF_ASSERT(p == NULL || (Scaleform::UPInt)p > 4);
            Release();
            if (p)
                p->AddRef();

            pObject = p;
        }

        return *this;
    }
    SelfType& Set(const SelfType& other)
    {
        if (&other != this)
        {
            Release();
            other.AddRef();

            pObject = other.pObject;
        }

        return *this;
    }

public:
    // Getters.

    T* operator->() const
    {
        return pObject;
    }
    T& operator*() const
    {
        SF_ASSERT(pObject);
        return *pObject;
    }

    // Obtain pointer reference directly, for D3D interfaces
    T*& GetRawRef() { return pObject; }
    T** GetRawPtr() const { return &pObject; }
    T* Get() const
    {
        return pObject;
    }
    // Conversion
    operator T*() const
    {
        return pObject;
    }

private:
    void AddRef() const
    {
        if (pObject)
            pObject->AddRef();
    }
    void Release()
    {
        if (pObject)
        {
            if ((Scaleform::UPInt)pObject & 1)
                pObject = (T*)((Scaleform::UPInt)pObject - 1);
            else
                pObject->Release();
        }
    }

private:
    mutable T* pObject;
};

///////////////////////////////////////////////////////////////////////////
static const char* LogFmt = "VM LOG: %s\n";

// LogVM seems to be used exclusively for FindProperty logging
// when GFX_AS3_VERBOSE is defined.
// TBD: Remove external printf references!
inline void LogVM(const String& s) { printf(LogFmt, s.ToCStr()); }
inline void LogVM(const ASString& s) { printf(LogFmt, s.ToCStr()); }
inline void LogVM(const char* s) { printf(LogFmt, s); }

///////////////////////////////////////////////////////////////////////////

#if defined(GFX_AS3_VERBOSE) && defined(ENABLE_LOG_VM)
#define LOG_VM(a) LogVM(a)
#else
#define LOG_VM(a)
#endif


namespace Abc
{
    ///////////////////////////////////////////////////////////////////////////
    // Instruction offset within TOpCode (which is an OpCodeBuffer)
    typedef UPInt TCodeOffset;

    ///////////////////////////////////////////////////////////////////////////
    // NamespaceKind describes the type of namespace NamespaceInfo represents.
    enum NamespaceKind
    {
        NS_Undefined = -1,
        NS_Public,
        NS_Protected,
        NS_StaticProtected,
        NS_Private,  // Private NS shouldn't be interned (private namespaces don't match).
        NS_Explicit, // What is this for? Perhaps it identifies NSs with explicit URIs?
        NS_PackageInternal
    };

}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {


#endif // INC_AS3_Abc_Type_H
