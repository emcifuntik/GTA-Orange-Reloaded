/**********************************************************************

Filename    :   Render_Stats.cpp
Content     :   Rendering statistics declarations
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

#include "Render_Stats.h"
#include "Render/Render_HAL.h"
#include "Kernel/SF_Alg.h"
#include "Kernel/SF_Random.h"

namespace Scaleform {

// ***** Renderer Stat Constants

// GRenderer Memory.
SF_DECLARE_MEMORY_STAT_SUM_GROUP(StatRender_Mem,    "Renderer", Stat_Mem)

SF_DECLARE_MEMORY_STAT(StatRender_Buffers_Mem,      "Buffers", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_RenderBatch_Mem,  "RenderBatch", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_Primitive_Mem,    "Primitive", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_Fill_Mem,         "Fill", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_Mesh_Mem,         "Mesh", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_MeshBatch_Mem,    "MeshBatch", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_Context_Mem,      "Context", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_NodeData_Mem,     "NodeData", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_TreeCache_Mem,    "TreeCache", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_TextureManager_Mem, "TextureManager", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_MatrixPool_Mem,    "MatrixPool", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_MatrixPoolHandle_Mem, "MatrixPoolHandles", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_Text_Mem,          "Text", StatRender_Mem)

void Link_RenderStats()
{
}


#ifdef SF_RENDERER_PROFILE

namespace Render {

Color ProfileViews::GetColorForBatch(UPInt base, unsigned index) const
{
    UInt64 key = (UInt64(base & 0xffffffff) << 32) | index;

    Color batchColor;
    Color* batchLookupColor = BatchColorLookup.Get(key);
    if (batchLookupColor)
    {
        batchColor = *batchLookupColor;
    }
    else
    {
        // Create a new color
        // PPS: Use HSV space? Using hue may produce better color variance and has a higher probability of
        //      avoiding the same color for multiple batches.
        //      Technically, for the best color variance we should be using a spatial metric.
        batchColor.SetHSV(Alg::Random::NextRandom() / (float)SF_MAX_UINT32, 0.8f, 0.9f);
        //batchColor = Scaleform::Alg::Random::NextRandom();

        BatchColorLookup.Add(key, batchColor);
    }

    batchColor.SetAlpha(192);
    return batchColor;
}

void ProfileViews::SetProfileViews(UInt64 modes)
{
    DisableProfileViews();
    if (!modes)
        return;

    for (int i = 0; i < 3; i++)
    {
        memset (FillCxforms[i].M, 0, sizeof(FillCxforms[i].M));
        FillCxforms[i].M[3][1] = 255;
    }

    OverrideMasks = 1;

    for (int i = 0; i < 3; i++)
    {
        UInt64 mode = modes >> (i << 4);
        if (mode & Profile_Fill)
        {
            FillCxforms[0].M[i][1] += ((mode & 0xff) * (1.f/255.f));
            FillMode = 1;
        }
        if (mode & Profile_Mask)
        {
            FillCxforms[1].M[i][1] += ((mode & 0xff) * (1.f/255.f));
            FillMode = 1;
        }
        if (mode & Profile_Clear)
        {
            FillCxforms[2].M[i][1] += ((mode & 0xff) * (1.f/255.f));
            FillMode = 1;
        }
        if (mode & Profile_Batch)
            BatchMode |= (1 << i) | 8;
    }

    if (FillMode)
        OverrideBlend = Blend_Add;
}
}
#endif

} // Scaleform

