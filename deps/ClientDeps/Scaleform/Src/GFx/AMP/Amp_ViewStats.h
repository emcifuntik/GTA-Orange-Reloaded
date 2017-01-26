/**********************************************************************

Filename    :   Amp_ViewStats.h
Content     :   Performance statistics for a MovieView
Created     :   February, 2010
Authors     :   Alex Mantzaris

Copyright   :   (c) 2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef GFX_AMP_SCRIPT_PROFILE_STATS_H
#define GFX_AMP_SCRIPT_PROFILE_STATS_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Hash.h"
#include "Kernel/SF_Random.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_StringHash.h"
#include "Kernel/SF_AmpInterface.h"
#include "Amp_ProfileFrame.h"

namespace Scaleform {
    class String;

namespace GFx {
    class MovieDef;

namespace AMP {

// For mixing ActionScript and C++ call stack 
enum AmpHandleType
{ 
    NativeCodeSwdHandle = 1,
};



class ViewStats : public AmpStats
{
public:

    // Struct for holding function execution metrics
    struct AmpFunctionStats
    {
		UInt32 TimesCalled;
		UInt64 TotalTime;    // Microseconds
    };

    // This struct holds timing information for one GASActionBuffer
    // The times are stored in an array of size equal to the size of the buffer for fast access
    // Many entries will be zero, as instructions are typically several bytes long
    struct BufferInstructionTimes : public RefCountBase<BufferInstructionTimes, Stat_Default_Mem>
    {
        BufferInstructionTimes(UInt32 size) : Times(size)
        {
            memset(&Times[0], 0, size * sizeof(Times[0])); // 
        }
        ArrayLH<UInt64> Times;
    };

    // Function executions are tracked per function called for each caller
    // This means a timings map will have a caller-callee pair as a key
    // ParentChildFunctionPair is this key
    struct ParentChildFunctionPair
    {
        UInt64 CallerId;    // SWF handle and byte offset
        UInt64 FunctionId;  // SWF handle and byte offset
        bool operator==(const ParentChildFunctionPair& kRhs) const 
        { 
            return (CallerId == kRhs.CallerId && FunctionId == kRhs.FunctionId); 
        }
    };
    typedef HashLH<ParentChildFunctionPair, AmpFunctionStats> FunctionStatMap;

    // Struct for holding per-line timings
    struct FileLinePair
    {
        UInt64 FileId;
        UInt32 LineNumber;
        bool operator==(const FileLinePair& rhs) const
        {
            return (FileId == rhs.FileId && LineNumber == rhs.LineNumber);
        }
    };
    typedef HashLH<FileLinePair, UInt64> SourceLineStatMap;

    // Constructor
    ViewStats();

    // virtual method overrides for native method profiling
    virtual void        NativePushCallstack(const char* functionName, AmpNativeFunctionId functionId);
    virtual void        NativePopCallstack(UInt64 time);

    // Methods for ActionScript profiling
    void                RegisterScriptFunction(UInt32 swdHandle, UInt32 swfOffset, const char* name, unsigned length, UInt32 asVersion);
    void                RegisterSourceFile(UInt32 swdHandle, UInt32 index, const char* name);
    void                PushCallstack(UInt32 swdHandle, UInt32 swfOffset);
    void                PopCallstack(UInt32 swdHandle, UInt32 swfOffset, UInt64 funcTime);
    void                GetActiveLine(UInt64* activeFile, UInt32* activeLine) const;
    void                SetActiveLine(UInt64 fileId, UInt32 lineNumber);
    ArrayLH<UInt64>&    LockBufferInstructionTimes(UInt32 swdHandle, UInt32 swfBufferOffset, unsigned length);
    void                ReleaseBufferInstructionTimes();
    void                RecordSourceLineTime(UInt64 lineTime);
    
    // Methods for AMP stats collecting
    void                CollectAmpInstructionStats(MovieProfile* movieProfile);
    void                ClearAmpInstructionStats();
    void                CollectAmpFunctionStats(MovieProfile* movieProfile);
    void                ClearAmpFunctionStats();
    void                CollectAmpSourceLineStats(MovieProfile* movieProfile);
    void                ClearAmpSourceLineStats();
    void                CollectTimingStats(ProfileFrame* frameProfile);
    void                CollectMarkers(MovieProfile* movieProfile);
    void                ClearMarkers();
    void                CollectMemoryStats(ProfileFrame* frameProfile);
    void                GetStats(StatBag* bag, bool reset);

    // View information
    void            SetName(const char* pcName);
    const String&   GetName() const;
    UInt32          GetViewHandle() const;
    void            SetMovieDef(MovieDef* movieDef);
    UInt32          GetVersion() const;
    float           GetWidth() const;
    float           GetHeight() const;
    float           GetFrameRate() const;
    UInt32          GetFrameCount() const;

    // Flash frame for this view
    UInt32      GetCurrentFrame() const;
    void        SetCurrentFrame(UInt32 frame);

    // For instruction profiling with random sampling
    UInt64      GetInstructionTime(UInt32 samplePeriod);

    // Frame markers
    void        AddMarker(const char* markerType);

    // Debugger
    void        DebugStep(int depth);
    void        DebugGo();
    void        DebugPause();
    void        DebugWait();
    bool        IsDebugPaused() const;

private:

    // ActionScript function profiling
    FunctionStatMap                                             FunctionTimingMap;
    FunctionDescMap                                             FunctionInfoMap;
    StringHashLH<UInt32>                                        NativeFunctionIdMap;
    UInt32                                                      NextNativeFunctionId;

    typedef ArrayConstPolicy<0, 4, true> NeverShrinkPolicy;
    struct CallInfo 
    {
        CallInfo() : FunctionId(0), FileId(0), LineNumber(0) {}
        CallInfo(UInt64 functionId, UInt64 fileId, UInt32 lineNumber) 
            : FunctionId(functionId), FileId(fileId), LineNumber(lineNumber) {}
        UInt64  FunctionId;
        UInt64  FileId;
        UInt32  LineNumber;
    };
    ArrayLH<CallInfo, Stat_Default_Mem, NeverShrinkPolicy>      Callstack;
    int                                                         CallstackDepthPause;
    UInt64                                                      ActiveFileId;
    UInt32                                                      ActiveLineNumber;
    mutable Lock                                                ActiveLock;                                    

    // ActionScript instruction timing
    typedef HashLH<UInt64, Ptr<BufferInstructionTimes> >        InstructionTimingMap;
    InstructionTimingMap                                        InstructionTimingsMap;

    // ActionScript source line timing
    SourceLineStatMap                                           SourceLineTimingsMap;
    SourceFileDescMap                                           SourceLineInfoMap;

    // View information
    mutable Lock    ViewLock;
    UInt32          ViewHandle;   // Unique GFxMovieRoot ID so that AMP can keep track of each view
    StringLH        ViewName;  
    unsigned        CurrentFrame;
    UInt32          Version;
    float           Width;
    float           Height;
    float           FrameRate;
    UInt32          FrameCount;

    // Instruction sampling
    Alg::Random::Generator  RandomGen;
    UInt32                  SkipSamples;
    UInt64                  LastTimer;

    // Markers
    StringHashLH<UInt32>    Markers;

    Scaleform::Event        DebugEvent;

    // Helper method for key generation
    static UInt64 MakeSwdOffsetKey(UInt32 swdHandle, UInt32 iOffset)
    {
        UInt64 iKey = swdHandle;
        iKey <<= 32;
        iKey += iOffset;
        return iKey;
    }

    void RefreshActiveLine();
};

// This class keeps track of function execution time and call stack
// Time starts counting in constructor and stops in destructor
// Updates the view stats object with the results
class ScopeFunctionTimer
{
public:
    ScopeFunctionTimer(ViewStats* viewStats, UInt32 swdHandle, unsigned pc) : 
          Stats(viewStats), 
          SwdHandle(swdHandle), 
          PC(pc)
      { 
          if (Stats != NULL)
          {
              Stats->PushCallstack(swdHandle, pc);
              StartTicks = Timer::GetRawTicks();
          }
          else
          {
              StartTicks = 0;
          }
      }
      ~ScopeFunctionTimer()
      {
          if (Stats != NULL)
          {
              Stats->PopCallstack(SwdHandle, PC, Timer::GetRawTicks() - StartTicks);
          }
      }
      UInt64                GetStartTicks() const   { return StartTicks; }
private:
    UInt64              StartTicks;
    ViewStats*          Stats;
    UInt32              SwdHandle;
    UInt32              PC;
};

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif
