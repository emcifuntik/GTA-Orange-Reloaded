/**********************************************************************

PublicHeader:   AMP
Filename    :   Amp_ProfileFrame.h
Content     :   Profile information sent to AMP
Created     :   February 2010
Authors     :   Alex Mantzaris, Ben Mowery

Copyright   :   (c) 2005-2010 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef GFX_AMP_PROFILE_FRAME
#define GFX_AMP_PROFILE_FRAME

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_String.h"
#include "Kernel/SF_Hash.h"


namespace Scaleform {
class File;
namespace GFx {
namespace AMP {

// This struct contains the stats for each ActionScript instruction for a given movie
//
// The stats are organized in an array with one element per ActionScript buffer
// The buffer stats include execution times for each instruction, but also the buffer length,
// which is used by the client to map source code lines to execution time
//
// Each buffer is uniquely identified by a SWD handle (unique to each SWF file),
// and the offset into the SWF in bytes.
// 
struct MovieInstructionStats : public RefCountBase<MovieInstructionStats, Stat_Default_Mem>
{
    struct InstructionTimePair
    {
        UInt32      Offset;  // Byte offset from beginning of buffer
        UInt64      Time;    // Microseconds
    };

    // ActionScript buffer data collected during execution
    struct ScriptBufferStats : RefCountBase<ScriptBufferStats, Stat_Default_Mem>
    {
        UInt32                          SwdHandle;      // uniquely defines the SWF
        UInt32                          BufferOffset;   // Byte offset from beginning of SWF
        UInt32                          BufferLength;
        ArrayLH<InstructionTimePair>    InstructionTimesArray;

        // Serialization
        void                            Read(File& str, UInt32 version);
        void                            Write(File& str, UInt32 version) const;
    };

    // Array of pointers, so all data can be allocated on the same heap
    ArrayLH< Ptr<ScriptBufferStats> > BufferStatsArray;

    // operators
    MovieInstructionStats& operator/=(unsigned numFrames);
    MovieInstructionStats& operator*=(unsigned num);
    void        Merge(const MovieInstructionStats& other);

    // Serialization
    void        Read(File& str, UInt32 version);
    void        Write(File& str, UInt32 version) const;
};


// Struct for mapping a byte offset to a function name and length
struct FunctionDesc : public RefCountBase<FunctionDesc, Stat_Default_Mem>
{
    StringLH    Name;
    unsigned    Length;
    UInt64      FileId;
    UInt32      FileLine;
    UInt32      ASVersion;
};
typedef HashLH<UInt64, Ptr<FunctionDesc> > FunctionDescMap;

typedef HashLH<UInt64, String> SourceFileDescMap;


// This struct contains the execution times and numbers for each ActionScript function 
// for a given movie
//
// The stats are organized in an array with one element per ActionScript function
// and a hash map from function ID to function name
//
struct MovieFunctionStats : public RefCountBase<MovieFunctionStats, Stat_Default_Mem>
{
    struct FuncStats
    {
        UInt64      FunctionId;     // SWF handle and offset
        UInt64      CallerId;       // Caller SWF handle and offset
        UInt32      TimesCalled;
        UInt64      TotalTime;      // microseconds
    };

    ArrayLH<FuncStats>              FunctionTimings;
    FunctionDescMap                 FunctionInfo;

    // operators
    MovieFunctionStats& operator/=(unsigned numFrames);
    MovieFunctionStats& operator*=(unsigned numFrames);
    void        Merge(const MovieFunctionStats& other);

    // Serialization
    void        Read(File& str, UInt32 version);
    void        Write(File& str, UInt32 version) const;

    // For debugging
    void DebugPrint() const;
};

// This struct contains the execution times and numbers for each ActionScript line 
// for a given movie
//
// The stats are organized in an array with one element per ActionScript line
// and a hash map from file ID to filename
//
struct MovieSourceLineStats : public RefCountBase<MovieSourceLineStats, Stat_Default_Mem>
{
    struct SourceStats
    {
        UInt64      FileId;         // SWF handle and file ID
        UInt32      LineNumber;
        UInt64      TotalTime;      // microseconds
    };

    ArrayLH<SourceStats>                SourceLineTimings;
    SourceFileDescMap                   SourceFileInfo;

    // operators
    MovieSourceLineStats& operator/=(unsigned numFrames);
    MovieSourceLineStats& operator*=(unsigned numFrames);
    void        Merge(const MovieSourceLineStats& other);

    // Serialization
    void        Read(File& str, UInt32 version);
    void        Write(File& str, UInt32 version) const;
};

// This struct contains all the profile statistics for a view
// That includes function statistics and instruction statistics
// It is part of the profile sent to AMP every frame
struct MovieProfile : public RefCountBase<MovieProfile, Stat_Default_Mem>
{
    struct MarkerInfo : public RefCountBase<FunctionDesc, Stat_Default_Mem>
    {
        StringLH        Name;
        UInt32          Number;
    };

    UInt32              ViewHandle;
    UInt32              MinFrame;
    UInt32              MaxFrame;
    StringLH            ViewName;
    UInt32              Version;
    float               Width;
    float               Height;
    float               FrameRate;
    UInt32              FrameCount;
    ArrayLH< Ptr<MarkerInfo> >   Markers;

    Ptr<MovieInstructionStats>  InstructionStats;
    Ptr<MovieFunctionStats>     FunctionStats;
    Ptr<MovieSourceLineStats>   SourceLineStats;

    // Initialization
    MovieProfile();
    
    // operators
    void            Merge(const MovieProfile& rhs);
    MovieProfile&   operator=(const MovieProfile& rhs);
    MovieProfile&   operator/=(unsigned numFrames);
    MovieProfile&   operator*=(unsigned num);

    // Serialization
    void            Read(File& str, UInt32 version);
    void            Write(File& str, UInt32 version) const;
};

// Image information
struct ImageInfo : public RefCountBase<ImageInfo, Stat_Default_Mem>
{
    UInt32      Id;
    StringLH    Name;
    StringLH    HeapName;
    UInt32      Bytes;
    bool        External;
    UInt32      AtlasId;
    UInt32      AtlasTop;
    UInt32      AtlasBottom;
    UInt32      AtlasLeft;
    UInt32      AtlasRight;  

    ImageInfo() : Id(0), Bytes(0), External(false), 
        AtlasId(0), AtlasTop(0), AtlasBottom(0), AtlasLeft(0), AtlasRight(0) { }

    bool operator<(const ImageInfo& rhs) const
    {
        if (Bytes == rhs.Bytes)
        {
            return Id < rhs.Id;
        }
        return Bytes > rhs.Bytes;
    }

    // Serialization
    void            Read(File& str, UInt32 version);
    void            Write(File& str, UInt32 version) const;
};

// The data in this class consists of all the metrics reported by AMP
// and displayed by the client in various ways (graphs, tables, etc)
//
// The AMP server updates one such object every frame and sends it to the client
// as part of a MessageProfileFrame
// 
class ProfileFrame : public RefCountBase<ProfileFrame, Stat_Default_Mem>
{
public:
    // Frame timestamp (microseconds)
    UInt64  TimeStamp;

    UInt32  FramesPerSecond;

    // CPU graph
    UInt32  AdvanceTime;
    UInt32      ActionTime;
    UInt32          SeekTime;
    UInt32      TimelineTime;
    UInt32      InputTime;
    UInt32          MouseTime;
    UInt32  GetVariableTime;
    UInt32  SetVariableTime;
    UInt32  InvokeTime;
    UInt32  DisplayTime;
    UInt32      TesselationTime;
    UInt32      GradientGenTime;
    UInt32  UserTime;

    // Rendering graph
    UInt32  LineCount;
    UInt32  MaskCount;
    UInt32  FilterCount;
    UInt32  MeshCount;
    UInt32  TriangleCount;
    UInt32  DrawPrimitiveCount;
    UInt32  StrokeCount;
    UInt32  GradientFillCount;
    UInt32  MeshThrashing;
    UInt32  RasterizedGlyphCount;
    UInt32  FontTextureCount;
    UInt32  NumFontCacheTextureUpdates;
    UInt32  FontThrashing;
    UInt32  FontFill;
    UInt32  FontFail;

    // Memory graph
    UInt32  TotalMemory;
    UInt32  ImageMemory;
    UInt32  MovieDataMemory;
    UInt32  MovieViewMemory;
    UInt32  MeshCacheMemory;
    UInt32  FontCacheMemory;
    UInt32  VideoMemory;
    UInt32  SoundMemory;
    UInt32  OtherMemory;

    ArrayLH< Ptr<MovieProfile> >    MovieStats;
    Ptr<MovieFunctionStats>         DisplayStats;
    ArrayLH<UInt32>                 SwdHandles;
    ArrayLH<UInt64>                 FileHandles;

    Ptr<MemItem>                    MemoryByStatId;
    Ptr<MemItem>                    MemoryByHeap;
    Ptr<MemItem>                    Images;
    Ptr<MemItem>                    Fonts;

    ArrayLH< Ptr<ImageInfo> >       ImageList;

    // Initialization
    ProfileFrame();

    // Operators for multiple frame reporting
    ProfileFrame& operator+=(const ProfileFrame& rhs);
    ProfileFrame& operator/=(unsigned numFrames);
    ProfileFrame& operator*=(unsigned num);

    // Serialization
    void        Read(File& str, UInt32 version);
    void        Write(File& str, UInt32 version) const;
};

// This struct holds the current state of AMP
// For feedback to the AMP client
//
class ServerState : public RefCountBase<ServerState, Stat_Default_Mem>
{
public:
    UInt32                  StateFlags;
    StringLH                ConnectedApp;
    StringLH                ConnectedFile;
    StringLH                AaMode;
    StringLH                StrokeType;
    StringLH                CurrentLocale;
    ArrayLH<String>         Locales;
    float                   CurveTolerance;
    float                   CurveToleranceMin;
    float                   CurveToleranceMax;
    float                   CurveToleranceStep;
    UInt64                  CurrentFileId;
    UInt32                  CurrentLineNumber;

    ServerState();
    ServerState& operator=(const ServerState& rhs);
    bool operator!=(const ServerState& rhs) const;

    // Serialization
    void        Read(File& str, UInt32 version);
    void        Write(File& str, UInt32 version) const;
};

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif
