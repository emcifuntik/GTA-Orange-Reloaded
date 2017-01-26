/**********************************************************************

Filename    :   Render_VertexPath.cpp
Content     :   
Created     :
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#include "Render_TessDefs.h"

namespace Scaleform { namespace Render {

void VertexPath::Clear() 
{ 
    Vertices.ClearAndRelease(); 
    Paths.ClearAndRelease();
    LastVertex = 0;
}

void VertexPath::AddVertex(CoordType x, CoordType y) 
{ 
    VertexBasic v = {x, y}; 
    Vertices.PushBack(v); 
}

void VertexPath::ClosePath()
{
    if (Vertices.GetSize() - LastVertex > 2 && 
        (Vertices[LastVertex].x != Vertices.Back().x || 
         Vertices[LastVertex].y != Vertices.Back().y))
    {
        Vertices.PushBack(Vertices[LastVertex]);
    }
}

void VertexPath::FinalizePath(unsigned, unsigned, bool, bool)
{
    if (Vertices.GetSize() - LastVertex < 3) 
    {
        Vertices.CutAt(LastVertex);
    }
    else
    {
        PathBasic p = { LastVertex, (unsigned)Vertices.GetSize() - LastVertex };
        Paths.PushBack(p);
        LastVertex = (unsigned)Vertices.GetSize();
    }
}


}} // Scaleform::Render
