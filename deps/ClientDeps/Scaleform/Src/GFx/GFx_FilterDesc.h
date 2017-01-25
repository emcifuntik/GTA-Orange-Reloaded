/**********************************************************************

Filename    :   FilterDesc.h
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_FilterDesc_H
#define INC_SF_GFX_FilterDesc_H

#include "Kernel/SF_Types.h"

#include "GFx/GFx_PlayerStats.h"
#include "GFx/GFx_StreamContext.h"
#include "Render/Text/Text_FilterDesc.h"

namespace Scaleform { 
namespace GFx {

class Stream;


// Helper function used to load filters.
template <class T> 
unsigned LoadFilters(T* pin, Render::Text::FilterDesc* filters, UPInt filterssz);

}} // Scaleform::GFx

#endif
