/**********************************************************************

PublicHeader:   None
Filename    :   GFxConfig.h
Content     :   GFx configuration file - contains #ifdefs for
                the optional components of the library
Created     :   June 15, 2005
Authors     :   All Scaleform Engineers

Copyright   :   (c) 2005-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_Config_H
#define INC_SF_GFX_Config_H

#include "Kernel/SF_Types.h" // reqd for SF_OS_*** macros

// ***** Prerequisites
//---------------------------------------------------------------------------

// Determine if we are using debug configuration. This needs to be done
// here because some of the later #defines rely on it.
#ifndef SF_BUILD_DEBUG
# if defined(_DEBUG) || defined(_DEBUGOPT)
#   define SF_BUILD_DEBUG
# endif
#endif

#ifndef SF_BUILD_DEBUGOPT
# ifdef _DEBUGOPT
#   define SF_BUILD_DEBUGOPT
# endif
#endif

// Allow developers to replace the math.h
//#define SF_MATH_H  <math.h>

#define GFX_MAX_MICE_SUPPORTED          4
#define GFX_MAX_KEYBOARD_SUPPORTED      4

// Default mouse support enable state for GFx. Enabling mouse is also
// dynamically controlled by GFxMovieView::EnableMouseSupport().
#if defined(SF_OS_XBOX) || defined(SF_OS_XBOX360) || \
    defined(SF_OS_PSP) || defined(SF_OS_PS2) || defined(SF_OS_PS3) || \
    defined(SF_OS_GAMECUBE)
#define GFX_MOUSE_SUPPORT_ENABLED   0
#else
#define GFX_MOUSE_SUPPORT_ENABLED   1
#endif

// ***** Install-time configurable add-ons
//---------------------------------------------------------------------------

// Enable/disable addons for Msvc builds
#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX360)
    #include "GFxConfigAddons.h"

    #if defined(SF_OS_WIN32) && defined(GFX_ENABLE_VIDEO_WIN32)
//        #define GFX_ENABLE_VIDEO
    #elif defined(SF_OS_XBOX360) && defined(GFX_ENABLE_VIDEO_XBOX360)
//        #define GFX_ENABLE_VIDEO
    #endif
#endif



// ***** General Scaleform options
//---------------------------------------------------------------------------

// This macro needs to be defined if it is necessary to avoid the use of Double.
// In that case Double in defined as float and thus extra #ifdef checks on
// overloads need to be done.
// NOTE: By default, PS2 and PSP define this if not manually defined here.
//#define SF_NO_DOUBLE

// Define this macro to eliminate custom wctype tables for functions such as
// G_iswspace, SFtowlower, g_towupper and so on. If this macro is defined GFx 
// will use system Unicode functions (which are incredibly slow on Microsoft 
// Windows).
//#define SF_NO_WCTYPE

// Enable thread support
#define SF_ENABLE_THREADS

// Define this to disable statistics tracking; this is useful for the final build.
#define SF_ENABLE_STATS

// Enable use of LIBJPEG. Wjen disabled, JPEGUtil becomes a no-op stub,
// and SWF JPEG image loading will stop functioning.
// Disabling saves roughly 60k in release build.
#define SF_ENABLE_LIBJPEG

// Define this macro if whole JPEGLIB is compiled as C++ code (CPP-ize). 
// By default libjpeg is pure C library and public names are not mangled. 
// Though, it might be necessary to mangle jpeglib's names in order to 
// resolve names clashing issues (for example, with XBox360's xmedia.lib).
//#define SF_CPP_LIBJPEG

// Enable use of ZLIB and GZLibFile class.
// When disabled, compressed SWF files will no longer load.
// Disabling saves roughly 20k in release build.
#define SF_ENABLE_ZLIB

// If disabled, SWF PNG image loading is not functioning.
// Note: beside the uncommenting the line below, it is necessary
// to provide the path and the library name for the linker.
// The name of the library is libpng.lib for all configurations;
// the path is different. For example, VC8, 64-bit, Release Static
// library is located at %(GFX)\3rdParty\libpng\lib\x64\msvc80\release_mt_static
#define SF_ENABLE_LIBPNG

// Enable the use of SIMD optimized functions, if available. If the platform does
// not support a SIMD instruction set, then this option will be disabled in SF_SIMD.h.
// SIMD instructions are disabled in debug builds, because without function inlining,
// they are considerably slower.
#ifndef SF_BUILD_DEBUG
    #define SF_ENABLE_SIMD
#endif


// ***** Memory/Allocation Configuration
//---------------------------------------------------------------------------

// These defines control memory tracking used for statistics and leak detection.
// By default, memory debug functionality is enabled automatically by SF_BUILD_DEBUG.
#ifdef SF_BUILD_DEBUG
    // Enable debug memory tracking. This passes __FILE__ and __LINE__ data into
    // allocations and enabled DebugStorage in the memory heap used to track
    // statistics and memory leaks.
    #define SF_MEMORY_ENABLE_DEBUG_INFO

    // Wipe memory in the allocator; this is useful for debugging.
    #define SF_MEMORY_MEMSET_ALL

    // Check for corrupted memory in the free chunks. May significantly
    // slow down the allocations.
    //#define SF_MEMORY_CHECK_CORRUPTION
#else
    // Enable debug memory tracking even in the release mode.
    //#define SF_MEMORY_ENABLE_DEBUG_INFO
#endif

// Use for extreme debug only! This define enforces the use of system 
// memory allocator as much as possible. That is: no granulator; absolute 
// minimal heap and system granularities; no reserve; no segment caching.
// It increases the chances of early memory corruption detection on 
// segmentation fault. Works only with GSysAllocWinAPI and GSysAllocMMAP.
//#define SF_MEMORY_FORCE_SYSALLOC

// Use for extreme debug only! Enforces the use of _aligned_malloc/_aligned_free
// No memory heaps functionality, no statistics, no heap memory limits.
//#define SF_MEMORY_FORCE_MALLOC

// Trace all essential operations: CreateHeap, DestroyHeap, Alloc, Free, Realloc.
// See MemoryHeap::HeapTracer for details.
//#define SF_MEMORY_TRACE_ALL




// ***** Renderer Specific options
//---------------------------------------------------------------------------

// Enable Stroker to generate lines; Adds roughly 20K in release build.
// Note, the stroker is required to produce faux bold glyphs. If it is 
// disabled then faux bold fonts will not be generated.
#define SF_RENDER_ENABLE_STROKER

// Enable StrokerAA to generate thick lines if they are opaque or almost opaque. 
// StrokerAA generates meshes about 10 times faster than the regular one, but 
// self-intersecting strokes will produce overlapping meshes.
#define SF_RENDER_ENABLE_STROKERAA

// If this macro is defined the renderer will use Hairliner to generate thin
// lines. The Hairliner generates meshes about 3-4 times faster than the regular 
// stroker and produces no overlaps in meshes. It also generates about 33% 
// less triangles. Hairliner serves only 1-pixel anti-aliased strokes.
#define SF_RENDER_ENABLE_HAIRLINER

// Tessellator produces better triangles for the price of 2-4% of slowdown
#define SF_RENDER_ENABLE_MOUNTAINS

// Enable EdgeAA; adds roughly 30K in release build.
#define SF_RENDER_ENABLE_EDGEAA

// Enable Render::GlyphPacker
//#define SF_RENDER_ENABLE_GLYPH_PACKER //@FONT

// Enable Render::GlyphCache
#define SF_RENDER_ENABLE_GLYPH_CACHE //@FONT

// Enable gradient generation
#define SF_RENDER_ENABLE_GRADIENT_GEN

// Enable morphing (shape tween)
#define SF_RENDER_ENABLE_MORPHING

// Enable scale9grid
//#define SF_RENDER_ENABLE_SCALE9GRID //@TODO, the macro is not used!

#define SF_NO_GRADIENT_GEN 


// ***** GFx Logging options
//---------------------------------------------------------------------------

// Define this macro enables support for verbose parsing of input files.
// If this option is set, verbose parse options are available, and the
// GFxLoader::SetVerboseParse call will be able to produce text logs.
// Game production release builds should probably undefine this option.
#define GFX_VERBOSE_PARSE

#ifdef GFX_VERBOSE_PARSE
    // Define this macro to enable support for verbose parsing of actions
    // (support for disassembly during loading).
    #define GFX_VERBOSE_PARSE_ACTION

    // Define this macro to enable support for verbose parsing
    // of shape character structures.
    #define GFX_VERBOSE_PARSE_SHAPE

    // Define this macro to enable support for verbose parsing
    // of morph shape character structures.
    #define GFX_VERBOSE_PARSE_MORPHSHAPE
#endif




// ***** GFx Add-on Options
//---------------------------------------------------------------------------

// Define this macro to enable sound support (including core and ActionScript)
#define GFX_ENABLE_SOUND

// Define this macro to exclude video support (including core and ActionScript)
//#define GFX_ENABLE_VIDEO

#if !defined(SF_ENABLE_THREADS)
    // Video can be used only with multithreading support
    #undef GFX_ENABLE_VIDEO
#endif // SF_ENABLE_THREADS

// Enable core CSS support
//#define GFX_ENABLE_CSS //@TODO

// Enable core XML support
#define GFX_ENABLE_XML

// Enable core IME support
//#define SF_ENABLE_IME //@TODO!
#define SF_NO_IME_SUPPORT

// Enable built-in Korean IME logic
//#define GFX_ENABLE_BUILTIN_KOREAN_IME




// ***** Other GFx Options
//---------------------------------------------------------------------------

// Enable DrawText API
//#define GFX_ENABLE_DRAWTEXT

// Enable keyboard support. When disabled, no Key AS class will be provided, 
// HandleEvent with KeyEvent will not be supported, PAD keys on consoles 
// will not work.
#define GFX_ENABLE_KEYBOARD

// Enable mouse support. When disabled, this option also disables Mouse AS class.
#define GFX_ENABLE_MOUSE

// Default mouse support enable state for GFx. Enabling mouse is also
// dynamically controlled by GFxMovieView::EnableMouseSupport().
#if ( defined(SF_OS_XBOX) || defined(SF_OS_XBOX360) || \
      defined(SF_OS_PSP) || defined(SF_OS_PS2) || defined(SF_OS_PS3) || \
      defined(SF_OS_GAMECUBE) ) && defined(GFX_ENABLE_MOUSE)
#undef GFX_ENABLE_MOUSE
#endif

// Enable font compactor (compaction during the run-time). When disabled, 
// fonts compacted by the gfxexport (with option -fc) remain working.
//#define GFX_ENABLE_FONT_COMPACTOR //@FONT

// Enable use of compacted fonts (fonts, compacted by gfxexport (option -fc))
// #define GFX_ENABLE_COMPACTED_FONTS //@FONT

// Enable TextField ActionScript extension functions. When disabled,
// standard GFxPlayer's HUD will not work.
#define GFX_ENABLE_TEXTFIELD_EXTENSIONS

// Enable text editing. When disabled, text selection will be disabled 
// as well (since it is a part of text editing).
#define GFX_ENABLE_TEXT_INPUT

// Define this macro to throw assertion if any font texture is generated during
// the runtime.
//#define GFX_ASSERT_ON_FONT_BITMAP_GEN

// Define this macro to throw assertion if any gradient texture is generated during
// the runtime.
//#define GFX_ASSERT_ON_GRADIENT_BITMAP_GEN

// Define this macro to throw assertion if any re-sampling occurred in renderer during
// the runtime.
//#define GFX_ASSERT_ON_RENDERER_RESAMPLING

// Define this macro to throw assertion if any mipmap levels generation occurred in
// renderer during the runtime.
//#define GFX_ASSERT_ON_RENDERER_MIPMAP_GEN





// ***** ActionScript Options
//---------------------------------------------------------------------------

// ActionScript support macros enable AS2 and/or AS3 vms. It is not
// critical to disable these macros since if one of the GFx::ASSupport classes
// is not defined, the majority of the associated VM code isn't linked in.
// However, having ony GFX_AS?_SUPPORT_one macro defined is slightly more efficient
// as it eliminates some extra overhead and virtual calls.
// Lib-only customers who need only one VM shouldn't change these macros and instead
// initialize GFx only one of the supporting class.

// Defined if AS2 support is available, so you can create GFx::AS2Support class.
#define GFX_AS2_SUPPORT
// Defined if AS3 support is available, so you can create GFx::AS3Support class.
#define GFX_AS3_SUPPORT

// Do not change this, as it's controlled by macros above.
#if defined(GFX_AS2_SUPPORT) && defined(GFX_AS3_SUPPORT)
    #define GFX_VM_ABSTRACT(arg) virtual arg = 0
#else
    #define GFX_VM_ABSTRACT(arg) arg
#endif


// Define this macro to enable support for verbose logging of executed ActionScript opcodes.
// If this macro is undefined, GFxMovie::SetVerboseAction will have no effect.
// Game production release builds should probably #undef this option.
#define GFX_AS2_VERBOSE
// Same thing for AS3
#define GFX_AS3_VERBOSE

#if !defined(SF_BUILD_DEBUG) && !defined(SF_BUILD_DEBUGOPT)
    // Turn verbose action OFF for Release builds
    #undef GFX_AS2_VERBOSE
    #undef GFX_AS3_VERBOSE
#endif

// Enable support for verbose logging of ActionScript run-time errors.
// If this macro is undefined, GFxMovie::SetVerboseActionErrors will have no effect.
// Game production release builds should probably #undef this option.
#define GFX_AS2_VERBOSE_ERRORS

// Enable garbage collection
#define GFX_AS_ENABLE_GC

// Enable *Filter class support.
#define GFX_AS2_ENABLE_FILTERS

// Enable 'Date' ActionScript class support.
#define GFX_AS2_ENABLE_DATE

// Enable 'Point' ActionScript class support.
#define GFX_AS2_ENABLE_POINT

// Enable 'Rectangle' ActionScript class support
#define GFX_AS2_ENABLE_RECTANGLE

// Enable 'Transform' ActionScript class support
#define GFX_AS2_ENABLE_TRANSFORM

// Enable 'ColorTransform' ActionScript class support
#define GFX_AS2_ENABLE_COLORTRANSFORM

// Enable 'Matrix' ActionScript class support
#define GFX_AS2_ENABLE_MATRIX

// Enable 'TextSnapshot' class support
#define GFX_AS_ENABLE_TEXTSNAPSHOT

// Enable 'SharedObject' class support
//#define GFX_AS2_ENABLE_SHAREDOBJECT

// Enable 'MovieClipLoader' ActionScript class support
#define GFX_AS2_ENABLE_MOVIECLIPLOADER

// Enable 'LoadVars' ActionScript class support
#define GFX_AS2_ENABLE_LOADVARS
//#define GFX_AS3_ENABLE_LOADVARS

// Enable 'BitmapData' ActionScript class support. Note, if BitmapData
// is disabled then textfield does not support <IMG> HTML tags and image 
// substitutions.
#define GFX_AS2_ENABLE_BITMAPDATA

// Enable 'System.capabilites' ActionScript class support
#define GFX_AS2_ENABLE_CAPABILITES

// Enable 'Color' ActionScript class support
#define GFX_AS2_ENABLE_COLOR

// Enable 'TextFormat' ActionScript class support
#define GFX_AS2_ENABLE_TEXTFORMAT

// Enable 'Selection' ActionScript class support.
#define GFX_AS2_ENABLE_SELECTION

// Enable 'Stage' ActionScript class support. Stage.height and Stage.width 
// will not be supported as well.
#define GFX_AS2_ENABLE_STAGE

// Enable 'Mouse' ActionScript class support. 
#define GFX_AS2_ENABLE_MOUSE



// *** Example defines for shipping (final) GFX build

#ifdef SF_BUILD_SHIPPING
#undef SF_ENABLE_STATS
#undef GFX_VERBOSE_PARSE
#undef GFX_AS2_VERBOSE_ERRORS 
#endif


// ***** Example defines for lightweight GFX on small consoles
//---------------------------------------------------------------------------

#ifdef SF_BUILD_LITE
    #undef SF_ENABLE_LIBJPEG
    #undef SF_ENABLE_ZLIB
    #undef SF_ENABLE_LIBPNG

    #undef SF_ENABLE_STATS

    #undef GFX_VERBOSE_PARSE
    #undef GFX_VERBOSE_PARSE_ACTION
    #undef GFX_VERBOSE_PARSE_SHAPE
    #undef GFX_VERBOSE_PARSE_MORPHSHAPE
    #undef GFX_AS2_VERBOSE
    #undef GFX_AS2_VERBOSE_ERRORS

    #undef GFX_ENABLE_GLYPH_PACKER
    #undef GFX_ENABLE_GRADIENT_GEN

    //#ifdef SF_OS_PSP
        #undef GFX_ENABLE_VIDEO
    //#endif

    #undef GFX_ENABLE_CSS
    #undef GFX_ENABLE_XML
    #undef SF_ENABLE_IME
    #undef GFX_ENABLE_BUILTIN_KOREAN_IME

    // *** Disabling ActionScript Options
    #undef GFX_AS2_ENABLE_FILTERS
    #undef GFX_AS2_ENABLE_DATE
    #undef GFX_AS2_ENABLE_POINT
    #undef GFX_AS2_ENABLE_RECTANGLE
    #undef GFX_AS2_ENABLE_TRANSFORM
    #undef GFX_AS2_ENABLE_COLORTRANSFORM
    #undef GFX_AS2_ENABLE_MATRIX
    #undef GFX_AS2_ENABLE_TEXTSNAPSHOT
    #undef GFX_AS2_ENABLE_SHAREDOBJECT
    #undef GFX_AS2_ENABLE_MOVIECLIPLOADER
    #undef GFX_AS2_ENABLE_LOADVARS
    #undef GFX_AS2_ENABLE_BITMAPDATA
    #undef GFX_AS2_ENABLE_CAPABILITES
    //#undef GFX_AS2_ENABLE_COLOR
    #undef GFX_AS2_ENABLE_TEXTFORMAT
    #undef GFX_AS2_ENABLE_SELECTION
    //#define GFX_AS2_ENABLE_STAGE
    #undef GFX_ENABLE_DRAWTEXT

    #undef GFX_ENABLE_FONT_COMPACTOR
    #undef GFX_ENABLE_TEXTFIELD_EXTENSIONS

    // no text input support on consoles
    #if defined(SF_OS_XBOX) || defined(SF_OS_XBOX360) || \
        defined(SF_OS_PSP) || defined(SF_OS_PS2) || defined(SF_OS_PS3) || \
        defined(SF_OS_GAMECUBE) || defined(SF_OS_WII)
        #undef GFX_ENABLE_TEXT_INPUT
    #endif

    // no mouse support on consoles
    #if defined(SF_OS_XBOX) || defined(SF_OS_XBOX360) || \
        defined(SF_OS_PSP) || defined(SF_OS_PS2) || defined(SF_OS_PS3) || \
        defined(SF_OS_GAMECUBE)
        #undef GFX_ENABLE_MOUSE
    #endif
#endif

// ***** Remote memory and performance profiling
//---------------------------------------------------------------------------

#if !defined(SF_BUILD_SHIPPING) && defined(SF_ENABLE_THREADS)

#define SF_AMP_SERVER

// Enable debug memory tracking when AMP is enabled
//#define SF_MEMORY_ENABLE_DEBUG_INFO

#define SF_RENDERER_PROFILE

#endif


// ***** Automatic class dependency checks (DO NOT EDIT THE CODE BELOW)
//---------------------------------------------------------------------------

#ifndef GFX_ENABLE_MOUSE
    #undef GFX_AS2_ENABLE_MOUSE
#endif
    
#ifndef GFX_AS2_ENABLE_POINT
    // GASRectangle is heavily dependent on GASPoint
    #undef GFX_AS2_ENABLE_RECTANGLE
#else
    #if !defined(GFX_AS2_ENABLE_RECTANGLE) || !defined(GFX_AS2_ENABLE_MATRIX)
        // GASPoint is useless without GASRectangle or GASMatrix
        #undef GFX_AS2_ENABLE_POINT
    #endif
#endif


#ifndef GFX_AS2_ENABLE_TRANSFORM
    // GASColorTransform is useless without GASTransform
    #undef GFX_AS2_COLORTRANSFORM
#endif


#if !defined(GFX_AS2_ENABLE_MATRIX) || \
    !defined(GFX_AS2_ENABLE_RECTANGLE) || \
    !defined(GFX_AS2_ENABLE_COLORTRANSFORM)
    // GASTransform is useless without GASMatrix, GASRectangle, GASColorTransform
    #undef GFX_AS2_ENABLE_TRANSFORM
#endif


#endif
