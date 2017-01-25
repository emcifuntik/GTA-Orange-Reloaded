/**********************************************************************

PublicHeader:   GFx
Filename    :   GFx/GFx_PlayerStats.h
Content     :   Definitions of GFxPlayer Stat constants.
Created     :   June 21, 2008
Authors     :   Michael Antonov

Notes       :

Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_PLAYERSTATS_H
#define INC_SF_GFX_PLAYERSTATS_H

#include "GFxConfig.h"
#include "Kernel/SF_Types.h"
#include "GFx/GFx_Stats.h"

namespace Scaleform { namespace GFx {

// ***** Movie Statistics Identifiers

// Specific statistics.
enum StatMovieData
{
    StatMD_Default = StatGroup_GFxMovieData,

    // Memory for a movie.
    StatMD_Mem,
      StatMD_CharDefs_Mem,
      StatMD_ShapeData_Mem,
      StatMD_Tags_Mem,
      StatMD_Fonts_Mem,
      StatMD_Images_Mem,
#ifdef GFX_ENABLE_SOUND
      StatMD_Sounds_Mem,
#endif
      StatMD_ActionOps_Mem,
      StatMD_Other_Mem,

    // Different
    StatMD_Time,
      StatMD_Load_Tks,
      StatMD_Bind_Tks
};

enum StatMovieView
{
    StatMV_Default = StatGroup_GFxMovieView,

    // MovieView memory.
    StatMV_Mem,
      StatMV_MovieClip_Mem,
      StatMV_ActionScript_Mem,
      StatMV_Text_Mem,
      StatMV_XML_Mem,
      StatMV_Other_Mem,

    // MovieView Timings.
    StatMV_Tks,
      StatMV_Advance_Tks,
        StatMV_Action_Tks,
          StatMV_Seek_Tks,
        StatMV_Timeline_Tks,
        StatMV_Input_Tks,
            StatMV_Mouse_Tks,
      StatMV_ScriptCommunication_Tks,
        StatMV_GetVariable_Tks,
        StatMV_SetVariable_Tks,
        StatMV_Invoke_Tks,
          StatMV_InvokeAction_Tks,
      StatMV_Display_Tks,
        StatMV_Tessellate_Tks,
        StatMV_GradientGen_Tks,

    // Counters.
    StatMV_Counters,
      StatMV_Invoke_Cnt,
      StatMV_MCAdvance_Cnt,
      StatMV_Tessellate_Cnt
};


enum StatIME
{
    StatIME_Default = StatGroup_GFxIME,

    // IME memory.
    StatIME_Mem
};


enum StatFontCache
{
    StatFC_Default = StatGroup_GFxFontCache,

    // Font Cache memory.
    StatFC_Mem,
        StatFC_Batch_Mem,
        StatFC_GlyphCache_Mem,
        StatFC_Other_Mem
};

}} // Scaleform::GFx

#endif // ! INC_SF_GFX_PLAYERSTATS_H
