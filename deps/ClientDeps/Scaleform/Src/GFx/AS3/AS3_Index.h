/**********************************************************************

Filename    :   AS3_Index.h
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

#ifndef INC_AS3_Index_H
#define INC_AS3_Index_H


#include "Kernel/SF_Types.h"

///////////////////////////////////////////////////////////////////////////
#include "Abc/AS3_Abc_Type.h"


namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
class Traits;

// Begins with ZERO ...
// AbsoluteIndex is used as a dynamic slot index and/or object data item
// offset in SlotInfo.

class AbsoluteIndex
{
public:
	typedef AbsoluteIndex SelfType;

    explicit AbsoluteIndex(SPInt ind)
    : Index(ind)
    {
    }
    AbsoluteIndex(class RelativeIndex ind, const Traits& t);
	explicit AbsoluteIndex(class SlotIndex ind, const Traits& t);
    explicit AbsoluteIndex(class GlobalSlotIndex ind);
    
public:
    SelfType& operator =(const SelfType& other)
    {
        Index = other.Get();
        return *this;
    }

    //
    bool operator <(const SelfType& other)
    {
        return Index < other.Index;
    }
    bool operator <(UPInt other)
    {
        return Index < 0 ? true : (UPInt)Index < other;
    }

    ///
    void operator +=(const SelfType& other)
    {
        Index += other.Get();
    }
    void operator +=(SPInt num)
    {
        Index += num;
    }
    SelfType operator +(UPInt v)
    {
        return SelfType(Index + v);
    }
    SelfType operator +(const SelfType& other)
    {
        return SelfType(Index + other.Get());
    }
    SelfType& operator ++()
    {
        ++Index;
        return *this;
    }
    SelfType operator ++(int)
    {
        SPInt tmp = Index;
        ++Index;
        return SelfType(tmp);
    }

    ///
    void operator -=(const SelfType& other)
    {
        Index -= other.Get();
    }
    void operator -=(SPInt num)
    {
        Index -= num;
    }
    SelfType operator -(UPInt v)
    {
        return SelfType(Index - v);
    }
    SelfType operator -(const SelfType& other)
    {
        return SelfType(Index - other.Get());
    }
    SelfType& operator --()
    {
        --Index;
        return *this;
    }
    SelfType operator --(int)
    {
        SPInt tmp = Index;
        --Index;
        return SelfType(tmp);
    }
    
public:
    UPInt Get() const
    {
        return static_cast<UPInt>(Index);
    }
    bool IsValid() const
    {
        return Index >= 0;
    }
    
protected:
    SPInt Index;
};

///////////////////////////////////////////////////////////////////////////
// Begins with ZERO ...
class RelativeIndex
{
public:
	typedef RelativeIndex SelfType;

    explicit RelativeIndex(SPInt ind)
    : Index(ind)
    {
    }
    explicit RelativeIndex(class SlotIndex ind);
    RelativeIndex(AbsoluteIndex ind, const Traits& t);
    
public:
    SelfType& operator =(const SelfType& other)
    {
        Index = other.Get();
        return *this;
    }
    void operator +=(const SelfType& other)
    {
        Index += other.Get();
    }
    bool operator <(const SelfType& other)
    {
        return Index < other.Index;
    }
    bool operator <(UPInt other)
    {
        return Index < 0 ? true : (UPInt)Index < other;
    }
    SelfType operator +(UPInt v)
    {
        return SelfType(Index + v);
    }
    SelfType operator +(const SelfType& other)
    {
        return SelfType(Index + other.Get());
    }
    SelfType& operator ++()
    {
        ++Index;
        return *this;
    }
    SelfType operator ++(int)
    {
        SPInt tmp = Index;
        ++Index;
        return SelfType(tmp);
    }
    
public:
    UPInt Get() const
    {
        return static_cast<UPInt>(Index);
    }
    bool IsValid() const
    {
        return Index >= 0;
    }
    
protected:
    SPInt Index;
};

///////////////////////////////////////////////////////////////////////////
// Begins with ONE ...
// It is a relative index + one.
class SlotIndex
{
public:
	typedef SlotIndex SelfType;

    explicit SlotIndex(UPInt ind)
    : Index(ind)
    {
    }
    explicit SlotIndex(RelativeIndex ind)
    : Index(ind.Get() + 1)
    {
        SF_ASSERT(ind.IsValid());
    }
	SlotIndex(class GlobalSlotIndex ind, const Traits& t);

public:
    SelfType& operator =(const SelfType& other)
    {
        Index = other.Get();
        return *this;
    }

	//
	bool operator <(UPInt other)
	{
		return Index < other;
	}
    bool operator <(const SelfType& other)
    {
        return Index < other.Index;
    }

	//
	bool operator >(UPInt other)
	{
		return Index > other;
	}
	bool operator >(const SelfType& other)
	{
		return Index > other.Index;
	}

	//
	SelfType operator -(UPInt v)
	{
		SF_ASSERT(Index >= v);
		return SelfType(Index - v);
	}
	SelfType operator -(const SelfType& other)
	{
		SF_ASSERT(Index >= other.Get());
		return SelfType(Index - other.Get());
	}

	//
    SelfType operator +(UPInt v)
    {
        return SelfType(Index + v);
    }
    SelfType operator +(const SelfType& other)
    {
        return SelfType(Index + other.Get());
    }
    SelfType& operator ++()
    {
        ++Index;
        return *this;
    }
    SelfType operator ++(int)
    {
        UPInt tmp = Index;
        ++Index;
        return SelfType(tmp);
    }
	void operator +=(const SelfType& other)
	{
		Index += other.Get();
	}
    
public:
    UPInt Get() const
    {
        return Index;
    }
    bool IsValid() const
    {
        return Index > 0;
    }
    
protected:
    UPInt Index;
};

///////////////////////////////////////////////////////////////////////////
// Begins with ONE ...
// It is an absolute index + one.
class GlobalSlotIndex
{
public:
	typedef GlobalSlotIndex SelfType;

	explicit GlobalSlotIndex(UPInt ind)
		: Index(ind)
	{
	}
	explicit GlobalSlotIndex(AbsoluteIndex ind)
		: Index(ind.Get() + 1)
	{
		SF_ASSERT(ind.IsValid());
	}

public:
	SelfType& operator =(const SelfType& other)
	{
		Index = other.Get();
		return *this;
	}

	//
	bool operator <(UPInt other)
	{
		return Index < other;
	}
	bool operator <(const SelfType& other)
	{
		return Index < other.Index;
	}

	//
	bool operator >(UPInt other)
	{
		return Index > other;
	}
	bool operator >(const SelfType& other)
	{
		return Index > other.Index;
	}

	//
	SelfType operator -(UPInt v)
	{
		SF_ASSERT(Index >= v);
		return SelfType(Index - v);
	}
	SelfType operator -(const SelfType& other)
	{
		SF_ASSERT(Index >= other.Get());
		return SelfType(Index - other.Get());
	}
    SelfType& operator --()
    {
        --Index;
        return *this;
    }
    SelfType operator --(int)
    {
        UPInt tmp = Index;
        --Index;
        return SelfType(tmp);
    }
    void operator -=(UPInt n)
    {
        Index -= n;
    }
    void operator -=(const SelfType& other)
    {
        Index -= other.Get();
    }

	//
	SelfType operator +(UPInt v)
	{
		return SelfType(Index + v);
	}
	SelfType operator +(const SelfType& other)
	{
		return SelfType(Index + other.Get());
	}
	SelfType& operator ++()
	{
		++Index;
		return *this;
	}
	SelfType operator ++(int)
	{
		UPInt tmp = Index;
		++Index;
		return SelfType(tmp);
	}
    void operator +=(UPInt n)
    {
        Index += n;
    }
	void operator +=(const SelfType& other)
	{
		Index += other.Get();
	}

public:
	UPInt Get() const
	{
		return Index;
	}
	bool IsValid() const
	{
		return Index > 0;
	}

protected:
	UPInt Index;
};

inline
RelativeIndex::RelativeIndex(SlotIndex ind)
: Index(ind.Get() - 1)
{
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_Index_H

