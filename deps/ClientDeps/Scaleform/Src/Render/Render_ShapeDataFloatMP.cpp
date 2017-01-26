/**********************************************************************

Filename    :   Render_ShapeDataFloatMP.cpp
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#include "Render_ShapeDataFloatMP.h"

#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {

ShapeDataFloatMP::ShapeDataFloatMP() : 
    ShapeMeshProvider()
{
    pData = *SF_HEAP_AUTO_NEW(this) ShapeDataFloat();
}

void ShapeDataFloatMP::CountLayers()
{
    pData->EndShape();
    AttachShape(pData);
}

}} // Scaleform::Render
