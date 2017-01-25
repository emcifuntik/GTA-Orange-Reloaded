/**********************************************************************

Filename    :   AS3_Abc_ConnstPool.cpp
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

#include "AS3_Abc_ConstPool.h"

namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc 
{

///////////////////////////////////////////////////////////////////////////
ConstPool::ConstPool()
: const_ns_set(Memory::GetHeapByAddress(&GetSelf()))
, any_namespace(NS_Public, "")
, any_type(MN_QName, 0, 0)
{
    // We do not need initialize zero records of all arrays in const_pool.
    // That will be done by Stream& operator >>(Stream& s, ConstPool& obj)
}

}}}} // namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc {

