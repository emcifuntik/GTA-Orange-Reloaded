/********************************************************************** 
Filename    :   GFx_Kernel.h 
Content     :   Convenience header collection for  
Created     :   July 2010 
Authors     :   Automatically generated 
 
Copyright   :   (c) 2006-2010 Scaleform Corp. All Rights Reserved. 
 
Licensees may use this file in accordance with the valid Scaleform 
Commercial License Agreement provided with the software. 
 
This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE. 
 
**********************************************************************/ 
 
#ifndef INC_GFx_Kernel_H 
#define INC_GFx_Kernel_H 
 
#include "../Src/Kernel/SF_AllocAddr.h" 		
#include "../Src/Kernel/SF_Allocator.h" 		
#include "../Src/Kernel/SF_AllocInfo.h" 		
#include "../Src/Kernel/SF_AmpInterface.h" 		
#include "../Src/Kernel/SF_ArrayStaticBuff.h" 		
#include "../Src/Kernel/SF_Debug.h" 		
#include "../Src/Kernel/SF_File.h" 		
#include "../Src/Kernel/SF_HeapNew.h" 		
#include "../Src/Kernel/SF_KeyCodes.h" 		
#include "../Src/Kernel/SF_Locale.h" 		
#include "../Src/Kernel/SF_Log.h" 		
#include "../Src/Kernel/SF_Math.h" 		
#include "../Src/Kernel/SF_Memory.h" 		
#include "../Src/Kernel/SF_MemoryHeap.h" 		
#include "../Src/Kernel/SF_RadixTree.h" 		
#include "../Src/Kernel/SF_Random.h" 		
#include "../Src/Kernel/SF_Range.h" 		
#include "../Src/Kernel/SF_RefCount.h" 		
#include "../Src/Kernel/SF_StackMemPool.h" 		
#include "../Src/Kernel/SF_Stats.h" 		
#include "../Src/Kernel/SF_Std.h" 		
#include "../Src/Kernel/SF_SysAlloc.h" 		
#include "../Src/Kernel/SF_SysFile.h" 		
#include "../Src/Kernel/SF_System.h" 		
#include "../Src/Kernel/SF_Types.h" 		
#include "../Src/Kernel/HeapMH/HeapMH_MemoryHeap.h" 		
#include "../Src/Kernel/HeapMH/HeapMH_SysAllocMalloc.h" 		
#include "../Src/Kernel/HeapPT/HeapPT_MemoryHeap.h" 		
#ifdef SF_OS_3DS
	#include "../Src/Kernel/HeapPT/HeapPT_SysAlloc3DS.h" 		
#endif
#include "../Src/Kernel/HeapPT/HeapPT_SysAllocMalloc.h" 		
#ifdef SF_OS_LINUX
	#include "../Src/Kernel/HeapPT/HeapPT_SysAllocMMAP.h" 		
#endif
#ifdef SF_OS_PS3
	#include "../Src/Kernel/HeapPT/HeapPT_SysAllocPS3.h" 		
#endif
#ifdef SF_OS_WII
	#include "../Src/Kernel/HeapPT/HeapPT_SysAllocWii.h" 		
#endif
#ifdef SF_OS_WIN32
	#include "../Src/Kernel/HeapPT/HeapPT_SysAllocWinAPI.h" 		
#endif
#endif     // INC_GFx_Kernel_H 
