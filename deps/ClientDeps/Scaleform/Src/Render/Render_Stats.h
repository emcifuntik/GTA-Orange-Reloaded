/**********************************************************************

PublicHeader:   Render
Filename    :   Render_Stats.h
Content     :   Color transform for renderer.
Created     :   April 27, 2010
Authors     :   Michael Antonov

Notes       :   
History     :   

Copyright   :   (c) 2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#ifndef INC_SF_Render_Stats_H
#define INC_SF_Render_Stats_H

#include "Kernel/SF_Stats.h"

namespace Scaleform {

enum GStatRenderer
{
    StatRender_Default = StatGroup_Renderer,

    StatRender_Mem,
        StatRender_Buffers_Mem,
        StatRender_RenderBatch_Mem,
        StatRender_Primitive_Mem,
        StatRender_Fill_Mem,
        StatRender_Mesh_Mem,
        StatRender_MeshBatch_Mem,
        StatRender_Context_Mem,
        StatRender_NodeData_Mem,
        StatRender_TreeCache_Mem,
        StatRender_TextureManager_Mem,
        StatRender_MatrixPool_Mem,
        StatRender_MatrixPoolHandle_Mem,
        StatRender_Text_Mem
};

void    Link_RenderStats();

} // Scaleform


#endif
