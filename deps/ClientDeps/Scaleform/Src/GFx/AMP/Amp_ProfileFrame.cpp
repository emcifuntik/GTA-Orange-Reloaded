#include "Amp_ProfileFrame.h"
#include "Kernel/SF_MemItem.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_MsgFormat.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MemItem.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

// utility function
void readString(File& file, String* str)
{
    UInt32 stringLength = file.ReadUInt32();
    for (UInt32 i = 0; i < stringLength; ++i)
    {
        str->AppendChar(file.ReadSByte());
    }
}

// utility function
void writeString(File& file, const String& str)
{
    file.WriteUInt32(static_cast<UInt32>(str.GetLength()));
    for (UPInt i = 0; i < str.GetLength(); ++i)
    {
        file.WriteSByte(str[i]);
    }
}


// Constructor
ProfileFrame::ProfileFrame() :
    TimeStamp(0),
    FramesPerSecond(0),
    AdvanceTime(0),
    ActionTime(0),
    SeekTime(0),
    TimelineTime(0),
    InputTime(0),
    MouseTime(0),
    GetVariableTime(0),
    SetVariableTime(0),
    InvokeTime(0),
    DisplayTime(0),
    TesselationTime(0),
    GradientGenTime(0),
    UserTime(0),
    LineCount(0),
    MaskCount(0),
    FilterCount(0),
    MeshCount(0),
    TriangleCount(0),
    DrawPrimitiveCount(0),
    StrokeCount(0),
    GradientFillCount(0),
    MeshThrashing(0),
    RasterizedGlyphCount(0),
    FontTextureCount(0),
    NumFontCacheTextureUpdates(0),
    FontThrashing(0),
    FontFill(0),
    FontFail(0),
    TotalMemory(0),
    ImageMemory(0),
    MovieDataMemory(0),
    MovieViewMemory(0),
    MeshCacheMemory(0),
    FontCacheMemory(0),
    VideoMemory(0),
    SoundMemory(0),
    OtherMemory(0)
{
    MemoryByStatId = *SF_HEAP_AUTO_NEW(this) MemItem(0);
    MemoryByHeap = *SF_HEAP_AUTO_NEW(this) MemItem(0);
    Images = *SF_HEAP_AUTO_NEW(this) MemItem(0);
    Fonts = *SF_HEAP_AUTO_NEW(this) MemItem(0);
    DisplayStats = *SF_HEAP_AUTO_NEW(this) MovieFunctionStats();
}


ProfileFrame& ProfileFrame::operator+=(const ProfileFrame& rhs)
{
    FramesPerSecond += rhs.FramesPerSecond;
    AdvanceTime += rhs.AdvanceTime;
    TimelineTime += rhs.TimelineTime;
    ActionTime += rhs.ActionTime;
    SeekTime += rhs.SeekTime;
    InputTime += rhs.InputTime;
    MouseTime += rhs.MouseTime;
    GetVariableTime += rhs.GetVariableTime;
    SetVariableTime += rhs.SetVariableTime;
    InvokeTime += rhs.InvokeTime;
    DisplayTime += rhs.DisplayTime;
    TesselationTime += rhs.TesselationTime;
    GradientGenTime += rhs.GradientGenTime;
    UserTime += rhs.UserTime;
    LineCount += rhs.LineCount;
    MaskCount += rhs.MaskCount;
    FilterCount += rhs.FilterCount;
    MeshCount += rhs.MeshCount;
    TriangleCount += rhs.TriangleCount;
    DrawPrimitiveCount += rhs.DrawPrimitiveCount;
    StrokeCount += rhs.StrokeCount;
    GradientFillCount += rhs.GradientFillCount;
    MeshThrashing += rhs.MeshThrashing;
    RasterizedGlyphCount += rhs.RasterizedGlyphCount;
    FontTextureCount += rhs.FontTextureCount;
    NumFontCacheTextureUpdates += rhs.NumFontCacheTextureUpdates;
    FontThrashing += rhs.FontThrashing;
    FontFill += rhs.FontFill;
    FontFail += rhs.FontFail;
    TotalMemory += rhs.TotalMemory;
    ImageMemory += rhs.ImageMemory;
    MovieDataMemory += rhs.MovieDataMemory;
    MovieViewMemory += rhs.MovieViewMemory;
    MeshCacheMemory += rhs.MeshCacheMemory;
    FontCacheMemory += rhs.FontCacheMemory;
    VideoMemory += rhs.VideoMemory;
    SoundMemory += rhs.SoundMemory;
    OtherMemory += rhs.OtherMemory;

    for (UPInt i = 0; i < rhs.MovieStats.GetSize(); ++i)
    {
        bool found = false;
        for (UPInt j = 0; j < MovieStats.GetSize(); ++j)
        {
            if (MovieStats[j]->ViewHandle == rhs.MovieStats[i]->ViewHandle)
            {
                MovieStats[j]->Merge(*rhs.MovieStats[i]);
                found = true;
                break;
            }
        }
        
        if (!found)
        {
            MovieProfile* newStats = SF_HEAP_AUTO_NEW(this) MovieProfile();
            *newStats = *rhs.MovieStats[i];
            MovieStats.PushBack(*newStats);
        }
    }
    DisplayStats->Merge(*rhs.DisplayStats);

    for (UPInt i = 0; i < rhs.SwdHandles.GetSize(); ++i)
    {
        bool found = false;
        for (UPInt j = 0; j < SwdHandles.GetSize(); ++j)
        {
            if (SwdHandles[j] == rhs.SwdHandles[i])
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            SwdHandles.PushBack(rhs.SwdHandles[i]);
        }
    }

    for (UPInt i = 0; i < rhs.FileHandles.GetSize(); ++i)
    {
        bool found = false;
        for (UPInt j = 0; j < FileHandles.GetSize(); ++j)
        {
            if (FileHandles[j] == rhs.FileHandles[i])
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            FileHandles.PushBack(rhs.FileHandles[i]);
        }
    }

    MemoryByStatId->Merge(*rhs.MemoryByStatId);
    MemoryByHeap->Merge(*rhs.MemoryByHeap);
    Images->Merge(*rhs.Images);
    Fonts->Merge(*rhs.Fonts);

    for (UPInt i = 0; i < rhs.ImageList.GetSize(); ++i)
    {
        bool found = false;
        for (UPInt j = 0; j < ImageList.GetSize(); ++j)
        {
            if (ImageList[j]->Id == rhs.ImageList[i]->Id)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            ImageList.PushBack(rhs.ImageList[i]);
        }
    }

    return *this;
}

ProfileFrame& ProfileFrame::operator/=(unsigned numFrames)
{
    FramesPerSecond /= numFrames;
    AdvanceTime /= numFrames;
    TimelineTime /= numFrames;
    ActionTime /= numFrames;
    SeekTime /= numFrames;
    InputTime /= numFrames;
    MouseTime /= numFrames;
    GetVariableTime /= numFrames;
    SetVariableTime /= numFrames;
    InvokeTime /= numFrames;
    DisplayTime /= numFrames;
    TesselationTime /= numFrames;
    GradientGenTime /= numFrames;
    UserTime /= numFrames;
    LineCount /= numFrames;
    MaskCount /= numFrames;
    FilterCount /= numFrames;
    MeshCount /= numFrames;
    TriangleCount /= numFrames;
    DrawPrimitiveCount /= numFrames;
    StrokeCount /= numFrames;
    GradientFillCount /= numFrames;
    MeshThrashing /= numFrames;
    RasterizedGlyphCount /= numFrames;
    FontTextureCount /= numFrames;
    NumFontCacheTextureUpdates /= numFrames;
    FontThrashing /= numFrames;
    FontFill /= numFrames;
    FontFail /= numFrames;
    TotalMemory /= numFrames;
    ImageMemory /= numFrames;
    MovieDataMemory /= numFrames;
    MovieViewMemory /= numFrames;
    MeshCacheMemory /= numFrames;
    FontCacheMemory /= numFrames;
    VideoMemory /= numFrames;
    SoundMemory /= numFrames;
    OtherMemory /= numFrames;

    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        *MovieStats[i] /= numFrames;
    }
    *DisplayStats /= numFrames;

    *MemoryByStatId /= numFrames;
    *MemoryByHeap /= numFrames;
    *Images /= numFrames;
    *Fonts /= numFrames;

    return *this;
}


ProfileFrame& ProfileFrame::operator*=(unsigned num)
{
    FramesPerSecond *= num;
    AdvanceTime *= num;
    TimelineTime *= num;
    ActionTime *= num;
    SeekTime *= num;
    InputTime *= num;
    MouseTime *= num;
    GetVariableTime *= num;
    SetVariableTime *= num;
    InvokeTime *= num;
    DisplayTime *= num;
    TesselationTime *= num;
    GradientGenTime *= num;
    UserTime *= num;
    LineCount *= num;
    MaskCount *= num;
    FilterCount *= num;
    MeshCount *= num;
    TriangleCount *= num;
    DrawPrimitiveCount *= num;
    StrokeCount *= num;
    GradientFillCount *= num;
    MeshThrashing *= num;
    RasterizedGlyphCount *= num;
    FontTextureCount *= num;
    NumFontCacheTextureUpdates *= num;
    FontThrashing *= num;
    FontFill *= num;
    FontFail *= num;
    TotalMemory *= num;
    ImageMemory *= num;
    MovieDataMemory *= num;
    MovieViewMemory *= num;
    MeshCacheMemory *= num;
    FontCacheMemory *= num;
    VideoMemory *= num;
    SoundMemory *= num;
    OtherMemory *= num;

    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        *MovieStats[i] *= num;
    }
    *DisplayStats *= num;

    *MemoryByStatId *= num;
    *MemoryByHeap *= num;
    *Images *= num;
    *Fonts *= num;

    return *this;
}


// Serialization
void ProfileFrame::Read(File& str, UInt32 version)
{
    TimeStamp = str.ReadUInt64();
    FramesPerSecond = str.ReadUInt32();
    AdvanceTime = str.ReadUInt32();
    TimelineTime = str.ReadUInt32();
    ActionTime = str.ReadUInt32();
    SeekTime = str.ReadUInt32();
    InputTime = str.ReadUInt32();
    MouseTime = str.ReadUInt32();
    GetVariableTime = str.ReadUInt32();
    SetVariableTime = str.ReadUInt32();
    InvokeTime = str.ReadUInt32();
    DisplayTime = str.ReadUInt32();
    TesselationTime = str.ReadUInt32();
    GradientGenTime = str.ReadUInt32();
    UserTime = str.ReadUInt32();
    LineCount = str.ReadUInt32();
    MaskCount = str.ReadUInt32();
    FilterCount = str.ReadUInt32();
    if (version >= 16)
    {
        MeshCount = str.ReadUInt32();
    }
    TriangleCount = str.ReadUInt32();
    DrawPrimitiveCount = str.ReadUInt32();
    StrokeCount = str.ReadUInt32();
    GradientFillCount = str.ReadUInt32();
    MeshThrashing = str.ReadUInt32();
    RasterizedGlyphCount = str.ReadUInt32();
    FontTextureCount = str.ReadUInt32();
    NumFontCacheTextureUpdates = str.ReadUInt32();
    if (version >= 14)
    {
        FontThrashing = str.ReadUInt32();
        FontFill = str.ReadUInt32();
        FontFail = str.ReadUInt32();
    }
    TotalMemory = str.ReadUInt32();
    ImageMemory = str.ReadUInt32();
    MovieDataMemory = str.ReadUInt32();
    MovieViewMemory = str.ReadUInt32();
    MeshCacheMemory = str.ReadUInt32();
    FontCacheMemory = str.ReadUInt32();
    VideoMemory = str.ReadUInt32();
    SoundMemory = str.ReadUInt32();
    OtherMemory = str.ReadUInt32();
    UInt32 iSize = str.ReadUInt32();
    MovieStats.Resize(iSize);
    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        MovieStats[i] = *SF_HEAP_AUTO_NEW(this) MovieProfile();
        MovieStats[i]->Read(str, version);
    }
    if (version >= 15)
    {
        DisplayStats->Read(str, version);
    }
    iSize = str.ReadUInt32();
    SwdHandles.Resize(iSize);
    for (UPInt i = 0; i < SwdHandles.GetSize(); ++i)
    {
        SwdHandles[i] = str.ReadUInt32();
    }
    if (version >= 9)
    {
        iSize = str.ReadUInt32();
        FileHandles.Resize(iSize);
        for (UPInt i = 0; i < FileHandles.GetSize(); ++i)
        {
            FileHandles[i] = str.ReadUInt64();
        }
    }
    MemoryByStatId->Read(str, version);
    MemoryByHeap->Read(str, version);
    if (version >= 3)
    {
        Images->Read(str, version);
    }
    if (version >= 7)
    {
        Fonts->Read(str, version);
    }
    if (version >= 17)
    {
        UInt32 size = str.ReadUInt32();
        ImageList.Resize(size);
        for (UInt32 i = 0; i < size; ++i)
        {
            ImageList[i] = *SF_HEAP_AUTO_NEW(this) ImageInfo();
            ImageList[i]->Read(str, version);
        }
    }

    if (version < 8)
    {
        str.ReadUInt32();
        str.ReadUInt32();
    }
}

// Serialization
void ProfileFrame::Write(File& str, UInt32 version) const
{
    str.WriteUInt64(TimeStamp);
    str.WriteUInt32(FramesPerSecond);
    str.WriteUInt32(AdvanceTime);
    str.WriteUInt32(TimelineTime);
    str.WriteUInt32(ActionTime);
    str.WriteUInt32(SeekTime);
    str.WriteUInt32(InputTime);
    str.WriteUInt32(MouseTime);
    str.WriteUInt32(GetVariableTime);
    str.WriteUInt32(SetVariableTime);
    str.WriteUInt32(InvokeTime);
    str.WriteUInt32(DisplayTime);
    str.WriteUInt32(TesselationTime);
    str.WriteUInt32(GradientGenTime);
    str.WriteUInt32(UserTime);
    str.WriteUInt32(LineCount);
    str.WriteUInt32(MaskCount);
    str.WriteUInt32(FilterCount);
    if (version >= 16)
    {
        str.WriteUInt32(MeshCount);
    }
    str.WriteUInt32(TriangleCount);
    str.WriteUInt32(DrawPrimitiveCount);
    str.WriteUInt32(StrokeCount);
    str.WriteUInt32(GradientFillCount);
    str.WriteUInt32(MeshThrashing);
    str.WriteUInt32(RasterizedGlyphCount);
    str.WriteUInt32(FontTextureCount);
    str.WriteUInt32(NumFontCacheTextureUpdates);
    if (version >= 14)
    {
        str.WriteUInt32(FontThrashing);
        str.WriteUInt32(FontFill);
        str.WriteUInt32(FontFail);
    }
    str.WriteUInt32(TotalMemory);
    str.WriteUInt32(ImageMemory);
    str.WriteUInt32(MovieDataMemory);
    str.WriteUInt32(MovieViewMemory);
    str.WriteUInt32(MeshCacheMemory);
    str.WriteUInt32(FontCacheMemory);
    str.WriteUInt32(VideoMemory);
    str.WriteUInt32(SoundMemory);
    str.WriteUInt32(OtherMemory);

    str.WriteUInt32(static_cast<UInt32>(MovieStats.GetSize()));
    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        MovieStats[i]->Write(str, version);
    }
    if (version >= 15)
    {
        DisplayStats->Write(str, version);
    }
    str.WriteUInt32(static_cast<UInt32>(SwdHandles.GetSize()));
    for (UPInt i = 0; i < SwdHandles.GetSize(); ++i)
    {
        str.WriteUInt32(SwdHandles[i]);
    }
    if (version >= 9)
    {
        str.WriteUInt32(static_cast<UInt32>(FileHandles.GetSize()));
        for (UPInt i = 0; i < FileHandles.GetSize(); ++i)
        {
            str.WriteUInt64(FileHandles[i]);
        }
    }
    MemoryByStatId->Write(str, version);
    MemoryByHeap->Write(str, version);
    if (version >= 3)
    {
        Images->Write(str, version);
    }
    if (version >= 7)
    {
        Fonts->Write(str, version);
    }
    if (version >= 17)
    {
        str.WriteUInt32(ImageList.GetSize());
        for (UInt32 i = 0; i < ImageList.GetSize(); ++i)
        {
            ImageList[i]->Write(str, version);
        }
    }

    if (version < 8)
    {
        str.WriteUInt32(0);
        str.WriteUInt32(0);
    }
}

//////////////////////////////////////////////////////////

// Constructor
MovieProfile::MovieProfile() : 
    ViewHandle(0),
    MinFrame(0),
    MaxFrame(0)
{   
    InstructionStats = *SF_HEAP_AUTO_NEW(this) MovieInstructionStats();
    FunctionStats = *SF_HEAP_AUTO_NEW(this) MovieFunctionStats();
    SourceLineStats = *SF_HEAP_AUTO_NEW(this) MovieSourceLineStats();
}


void MovieProfile::Merge(const MovieProfile& rhs)
{
    SF_ASSERT(ViewHandle == rhs.ViewHandle);
    SF_ASSERT(ViewName == rhs.ViewName);
    MinFrame = Alg::Min(MinFrame, rhs.MinFrame);
    MaxFrame = Alg::Max(MaxFrame, rhs.MaxFrame);

    for (UPInt i = 0; i < rhs.Markers.GetSize(); ++i)
    {
        bool found = false;
        for (UPInt j = 0; j < Markers.GetSize(); ++j)
        {
            if (rhs.Markers[i]->Name == Markers[j]->Name)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Markers.PushBack(Markers[i]);
        }
    }

    InstructionStats->Merge(*rhs.InstructionStats);
    FunctionStats->Merge(*rhs.FunctionStats);
    SourceLineStats->Merge(*rhs.SourceLineStats);
}

MovieProfile& MovieProfile::operator=(const MovieProfile& rhs)
{
    ViewHandle = rhs.ViewHandle;
    MinFrame = rhs.MinFrame;
    MaxFrame = rhs.MaxFrame;
    ViewName = rhs.ViewName;
    Version = rhs.Version;
    Width = rhs.Width;
    Height = rhs.Height;
    FrameRate = rhs.FrameRate;
    FrameCount = rhs.FrameCount;
    Markers = rhs.Markers;
    *InstructionStats = *rhs.InstructionStats;
    *FunctionStats = *rhs.FunctionStats;
    *SourceLineStats = *rhs.SourceLineStats;
    return *this;
}

MovieProfile& MovieProfile::operator/=(unsigned numFrames)
{
    *InstructionStats /= numFrames;
    *FunctionStats /= numFrames;
    *SourceLineStats /= numFrames;
    return *this;
}

MovieProfile& MovieProfile::operator*=(unsigned num)
{
    *InstructionStats *= num;
    *FunctionStats *= num;
    *SourceLineStats *= num;
    return *this;
}

// Serialization
void MovieProfile::Read(File& str, UInt32 version)
{
    ViewHandle = str.ReadUInt32();
    MinFrame = str.ReadUInt32();
    MaxFrame = str.ReadUInt32();
    if (version >= 4)
    {
        readString(str, &ViewName);
        Version = str.ReadUInt32();
        Width = str.ReadFloat();
        Height = str.ReadFloat();
        FrameRate = str.ReadFloat();
        FrameCount = str.ReadUInt32();
    }
    if (version >= 6)
    {
        UInt32 numMarkers = str.ReadUInt32();
        Markers.Resize(numMarkers);
        for (UInt32 i = 0; i < numMarkers; ++i)
        {
            Markers[i] = *SF_HEAP_AUTO_NEW(this) MarkerInfo();
            if (version >= 11)
            {
                 readString(str, &Markers[i]->Name);
            }
            else
            {
                Markers[i]->Name = "Marker";
            }
            Markers[i]->Number = str.ReadUInt32();
        }
    }
    SF_ASSERT(InstructionStats);
    InstructionStats->Read(str, version);
    SF_ASSERT(FunctionStats);
    FunctionStats->Read(str, version);
    SF_ASSERT(SourceLineStats);
    SourceLineStats->Read(str, version);
}

// Serialization
void MovieProfile::Write(File& str, UInt32 version) const
{
    str.WriteUInt32(ViewHandle);
    str.WriteUInt32(MinFrame);
    str.WriteUInt32(MaxFrame);
    if (version >= 4)
    {
        writeString(str, ViewName);
        str.WriteUInt32(Version);
        str.WriteFloat(Width);
        str.WriteFloat(Height);
        str.WriteFloat(FrameRate);
        str.WriteUInt32(FrameCount);
    }
    if (version >= 6)
    {
        str.WriteUInt32(static_cast<UInt32>(Markers.GetSize()));
        for (UPInt i = 0; i < Markers.GetSize(); ++i)
        {
            if (version >= 11)
            {
                writeString(str, Markers[i]->Name);
            }
            str.WriteUInt32(Markers[i]->Number);
        }
    }
    SF_ASSERT(InstructionStats);
    InstructionStats->Write(str, version);
    SF_ASSERT(FunctionStats);
    FunctionStats->Write(str, version);
    SF_ASSERT(SourceLineStats);
    SourceLineStats->Write(str, version);
}

/////////////////////////////////////////////////////

void ImageInfo::Read(File& str, UInt32 version)
{
    SF_UNUSED(version);
    Id = str.ReadUInt32();
    readString(str, &Name);
    readString(str, &HeapName);
    Bytes = str.ReadUInt32();
    External = (str.ReadUByte() != 0);
    AtlasId = str.ReadUInt32();
    AtlasTop = str.ReadUInt32();
    AtlasBottom = str.ReadUInt32();
    AtlasLeft = str.ReadUInt32();
    AtlasRight = str.ReadUInt32();
}

void ImageInfo::Write(File& str, UInt32 version) const
{
    SF_UNUSED(version);
    str.WriteUInt32(Id);
    writeString(str, Name);
    writeString(str, HeapName);
    str.WriteUInt32(Bytes);
    str.WriteUByte(External ? 1 : 0);
    str.WriteUInt32(AtlasId);
    str.WriteUInt32(AtlasTop);
    str.WriteUInt32(AtlasBottom);
    str.WriteUInt32(AtlasLeft);
    str.WriteUInt32(AtlasRight);
}

/////////////////////////////////////////////////////

MovieInstructionStats& MovieInstructionStats::operator/=(unsigned numFrames)
{
    for (UPInt i = 0; i < BufferStatsArray.GetSize(); ++i)
    {
        ArrayLH<InstructionTimePair>& times = BufferStatsArray[i]->InstructionTimesArray;
        for (UPInt j = 0; j < times.GetSize(); ++j)
        {
            times[j].Time /= numFrames;
        }
    }
    return *this;
}

MovieInstructionStats& MovieInstructionStats::operator*=(unsigned num)
{
    for (UPInt i = 0; i < BufferStatsArray.GetSize(); ++i)
    {
        ArrayLH<InstructionTimePair>& times = BufferStatsArray[i]->InstructionTimesArray;
        for (UPInt j = 0; j < times.GetSize(); ++j)
        {
            times[j].Time *= num;
        }
    }
    return *this;
}


void MovieInstructionStats::Merge(const MovieInstructionStats& other)
{
    for (UPInt i = 0; i < other.BufferStatsArray.GetSize(); ++i)
    {
        bool buffersMerged = false;
        for (UPInt j = 0; j < BufferStatsArray.GetSize(); ++j)
        {
            if (BufferStatsArray[j]->SwdHandle == other.BufferStatsArray[i]->SwdHandle
                && BufferStatsArray[j]->BufferOffset == other.BufferStatsArray[i]->BufferOffset)
            {
                SF_ASSERT(BufferStatsArray[j]->BufferLength == other.BufferStatsArray[i]->BufferLength);
                ArrayLH<InstructionTimePair>& thisArray = BufferStatsArray[j]->InstructionTimesArray;
                const ArrayLH<InstructionTimePair>& otherArray = BufferStatsArray[i]->InstructionTimesArray;
                for (UPInt k = 0; k < otherArray.GetSize(); ++k)
                {
                    bool instructionsMerged = false;
                    for (UPInt l = 0; l < thisArray.GetSize(); ++l)
                    {
                        if (thisArray[l].Offset == otherArray[k].Offset)
                        {
                            thisArray[l].Time += otherArray[k].Time;
                            instructionsMerged = true;
                            break;
                        }
                    }

                    if (!instructionsMerged)
                    {
                        thisArray.PushBack(otherArray[k]);
                    }
                }
                buffersMerged = true;
                break;
            }
        }

        if (!buffersMerged)
        {
            ScriptBufferStats* newBufferStats = SF_HEAP_AUTO_NEW(this) ScriptBufferStats();
            *newBufferStats = *other.BufferStatsArray[i];
            BufferStatsArray.PushBack(*newBufferStats);
        }
    }
}


// Serialization
void MovieInstructionStats::Read(File& str, UInt32 version)
{
    UInt32 iSize = str.ReadUInt32();
    BufferStatsArray.Resize(iSize);
    for (UPInt i = 0; i < BufferStatsArray.GetSize(); ++i)
    {
        BufferStatsArray[i] = *SF_HEAP_AUTO_NEW(this) ScriptBufferStats();
        BufferStatsArray[i]->Read(str, version);
    }
}

// Serialization
void MovieInstructionStats::Write(File& str, UInt32 version) const
{
    str.WriteUInt32(static_cast<UInt32>(BufferStatsArray.GetSize()));
    for (UPInt i = 0; i < BufferStatsArray.GetSize(); ++i)
    {
        BufferStatsArray[i]->Write(str, version);
    }
}

///////////////////////////////////////////////////////////

// Serialization
void MovieInstructionStats::ScriptBufferStats::Read(File& str, UInt32 version)
{
    SF_UNUSED(version);
    SwdHandle = str.ReadUInt32();
    BufferOffset = str.ReadUInt32();
    BufferLength = str.ReadUInt32();
    UInt32 iSize = str.ReadUInt32();
    InstructionTimesArray.Resize(iSize);
    for (UPInt i = 0; i < InstructionTimesArray.GetSize(); ++i)
    {
        InstructionTimesArray[i].Offset = str.ReadUInt32();
        InstructionTimesArray[i].Time = str.ReadUInt64();
    }
}

// Serialization
void MovieInstructionStats::ScriptBufferStats::Write(File& str, UInt32 version) const
{
    SF_UNUSED(version);
    str.WriteUInt32(SwdHandle);
    str.WriteUInt32(BufferOffset);
    str.WriteUInt32(BufferLength);
    str.WriteUInt32(static_cast<UInt32>(InstructionTimesArray.GetSize()));
    for (UPInt i = 0; i < InstructionTimesArray.GetSize(); ++i)
    {
        str.WriteUInt32(InstructionTimesArray[i].Offset);
        str.WriteUInt64(InstructionTimesArray[i].Time);
    }
}

///////////////////////////////////////////////////////////////

MovieFunctionStats& MovieFunctionStats::operator/=(unsigned numFrames)
{
    for (UPInt i = 0; i < FunctionTimings.GetSize(); ++i)
    {
        FunctionTimings[i].TimesCalled /= numFrames;
        FunctionTimings[i].TotalTime /= numFrames;        
    }
    return *this;
}

MovieFunctionStats& MovieFunctionStats::operator*=(unsigned num)
{
    for (UPInt i = 0; i < FunctionTimings.GetSize(); ++i)
    {
        FunctionTimings[i].TimesCalled *= num;
        FunctionTimings[i].TotalTime *= num;        
    }
    return *this;
}

void MovieFunctionStats::Merge(const MovieFunctionStats& other)
{
    for (UPInt i = 0; i < other.FunctionTimings.GetSize(); ++i)
    {
        bool merged = false;
        for (UPInt j = 0; j < FunctionTimings.GetSize(); ++j)
        {
            if (FunctionTimings[j].FunctionId == other.FunctionTimings[i].FunctionId
                && FunctionTimings[j].CallerId == other.FunctionTimings[i].CallerId)
            {
                FunctionTimings[j].TimesCalled += other.FunctionTimings[i].TimesCalled;
                FunctionTimings[j].TotalTime += other.FunctionTimings[i].TotalTime;
                merged = true;
                break;
            }
        }
        if (!merged)
        {
            FunctionTimings.PushBack(other.FunctionTimings[i]);
        }
    }

    FunctionDescMap::ConstIterator it;
    for (it = other.FunctionInfo.Begin(); it != other.FunctionInfo.End(); ++it)
    {
        FunctionInfo.Set(it->First, it->Second);
    }
}


// Serialization
void MovieFunctionStats::Read(File& str, UInt32 version)
{
    SF_UNUSED(version);
    UInt32 size = str.ReadUInt32();
    FunctionTimings.Resize(size);
    for (UInt32 i = 0; i < size; i++)
    {
        FunctionTimings[i].FunctionId = str.ReadUInt64();
        FunctionTimings[i].CallerId = str.ReadUInt64();
        FunctionTimings[i].TimesCalled = str.ReadUInt32();
        FunctionTimings[i].TotalTime = str.ReadUInt64();
    }
    
    size = str.ReadUInt32();
    for (UInt32 i = 0; i < size; i++)
    {
        UInt64 iKey = str.ReadUInt64();
        FunctionDesc* pDesc = SF_HEAP_AUTO_NEW(this) FunctionDesc();
        readString(str, &pDesc->Name);
        pDesc->Length = str.ReadUInt32();
        if (version >= 9)
        {
            pDesc->FileId = str.ReadUInt64();
            pDesc->FileLine = str.ReadUInt32();
            if (version >= 13)
            {
                pDesc->ASVersion = str.ReadUInt32();
            }
        }
        FunctionInfo.Set(iKey, *pDesc);
    }
}

// Serialization
void MovieFunctionStats::Write(File& str, UInt32 version) const
{
    SF_UNUSED(version);
    str.WriteUInt32(static_cast<UInt32>(FunctionTimings.GetSize()));
    for (UPInt i = 0; i < FunctionTimings.GetSize(); ++i)
    {
        str.WriteUInt64(FunctionTimings[i].FunctionId);
        str.WriteUInt64(FunctionTimings[i].CallerId);
        str.WriteUInt32(FunctionTimings[i].TimesCalled);
        str.WriteUInt64(FunctionTimings[i].TotalTime);
    }

    str.WriteUInt32(static_cast<UInt32>(FunctionInfo.GetSize()));
    FunctionDescMap::ConstIterator descIt;
    for (descIt = FunctionInfo.Begin(); descIt != FunctionInfo.End(); ++descIt)
    {
        str.WriteUInt64(descIt->First);
        writeString(str, descIt->Second->Name);
        str.WriteUInt32(descIt->Second->Length);
        if (version >= 9)
        {
            str.WriteUInt64(descIt->Second->FileId);
            str.WriteUInt32(descIt->Second->FileLine);
            if (version >= 13)
            {
                str.WriteUInt32(descIt->Second->ASVersion);
            }
        }
    }
}

// Debug message for visualizing the call graph
void MovieFunctionStats::DebugPrint() const
{
    for (UPInt i = 0; i < FunctionTimings.GetSize(); ++i)
    {
        String entry;
        String name;
        FunctionDescMap::ConstIterator descIt;
        descIt = FunctionInfo.Find(FunctionTimings[i].FunctionId);
        if (descIt != FunctionInfo.End())
        {
            name = descIt->Second->Name;
        }
        String parentName;
        descIt = FunctionInfo.Find(FunctionTimings[i].CallerId);
        if (descIt != FunctionInfo.End())
        {
            parentName = descIt->Second->Name;
        }
        Format(entry, "{0} ({1}) from {2} ({3}): {4} times\n", name.ToCStr(), FunctionTimings[i].FunctionId, parentName.ToCStr(), FunctionTimings[i].CallerId, FunctionTimings[i].TimesCalled);
        SF_DEBUG_MESSAGE(1, entry);
    }
    SF_DEBUG_MESSAGE(1, "------------------------------");
}

///////////////////////////////////////////////////////////////

MovieSourceLineStats& MovieSourceLineStats::operator/=(unsigned numFrames)
{
    for (UPInt i = 0; i < SourceLineTimings.GetSize(); ++i)
    {
        SourceLineTimings[i].TotalTime /= numFrames;        
    }
    return *this;
}

MovieSourceLineStats& MovieSourceLineStats::operator*=(unsigned num)
{
    for (UPInt i = 0; i < SourceLineTimings.GetSize(); ++i)
    {
        SourceLineTimings[i].TotalTime *= num;        
    }
    return *this;
}

void MovieSourceLineStats::Merge(const MovieSourceLineStats& other)
{
    for (UPInt i = 0; i < other.SourceLineTimings.GetSize(); ++i)
    {
        bool merged = false;
        for (UPInt j = 0; j < SourceLineTimings.GetSize(); ++j)
        {
            if (SourceLineTimings[j].FileId == other.SourceLineTimings[i].FileId
                && SourceLineTimings[j].LineNumber == other.SourceLineTimings[i].LineNumber)
            {
                SourceLineTimings[j].TotalTime += other.SourceLineTimings[i].TotalTime;
                merged = true;
                break;
            }
        }
        if (!merged)
        {
            SourceLineTimings.PushBack(other.SourceLineTimings[i]);
        }
    }

    SourceFileDescMap::ConstIterator it;
    for (it = other.SourceFileInfo.Begin(); it != other.SourceFileInfo.End(); ++it)
    {
        SourceFileInfo.Set(it->First, it->Second);
    }
}


// Serialization
void MovieSourceLineStats::Read(File& str, UInt32 version)
{
    if (version >= 9)
    {
        UInt32 size = str.ReadUInt32();
        SourceLineTimings.Resize(size);
        for (UInt32 i = 0; i < size; i++)
        {
            SourceLineTimings[i].FileId = str.ReadUInt64();
            SourceLineTimings[i].LineNumber = str.ReadUInt32();
            SourceLineTimings[i].TotalTime = str.ReadUInt64();
        }

        size = str.ReadUInt32();
        for (UInt32 i = 0; i < size; i++)
        {
            UInt64 iKey = str.ReadUInt64();
            String desc;
            readString(str, &desc);
            SourceFileInfo.Set(iKey, desc);
        }
    }
}

// Serialization
void MovieSourceLineStats::Write(File& str, UInt32 version) const
{
    if (version >= 9)
    {
        str.WriteUInt32(static_cast<UInt32>(SourceLineTimings.GetSize()));
        for (UPInt i = 0; i < SourceLineTimings.GetSize(); ++i)
        {
            str.WriteUInt64(SourceLineTimings[i].FileId);
            str.WriteUInt32(SourceLineTimings[i].LineNumber);
            str.WriteUInt64(SourceLineTimings[i].TotalTime);
        }

        str.WriteUInt32(static_cast<UInt32>(SourceFileInfo.GetSize()));
        SourceFileDescMap::ConstIterator descIt;
        for (descIt = SourceFileInfo.Begin(); descIt != SourceFileInfo.End(); ++descIt)
        {
            str.WriteUInt64(descIt->First);
            writeString(str, descIt->Second);
        }
    }
}

///////////////////////////////////////

ServerState::ServerState() :
    StateFlags(0),
    CurveTolerance(0.0f), 
    CurveToleranceMin(0.0f),
    CurveToleranceMax(0.0f),
    CurveToleranceStep(0.0f),
    CurrentFileId(0),
    CurrentLineNumber(0)
{
}

ServerState& ServerState::operator=(const ServerState& rhs)
{
    StateFlags = rhs.StateFlags;
    ConnectedApp = rhs.ConnectedApp;
    ConnectedFile = rhs.ConnectedFile;
    AaMode = rhs.AaMode;
    StrokeType = rhs.StrokeType;
    CurrentLocale = rhs.CurrentLocale;
    Locales.Resize(rhs.Locales.GetSize());
    for (UPInt i = 0; i < Locales.GetSize(); ++i)
    {
        Locales[i] = rhs.Locales[i];
    }
    CurveTolerance = rhs.CurveTolerance;
    CurveToleranceMin = rhs.CurveToleranceMin;
    CurveToleranceMax = rhs.CurveToleranceMax;
    CurveToleranceStep = rhs.CurveToleranceStep;
    CurrentFileId = rhs.CurrentFileId;
    CurrentLineNumber = rhs.CurrentLineNumber;
    return *this;
}

bool ServerState::operator!=(const ServerState& rhs) const
{
    if (StateFlags != rhs.StateFlags) return true;
    if (ConnectedApp != rhs.ConnectedApp) return true;
    if (ConnectedFile != rhs.ConnectedFile) return true;
    if (AaMode != rhs.AaMode) return true;
    if (StrokeType != rhs.StrokeType) return true;
    if (CurrentLocale != rhs.CurrentLocale) return true;
    if (Locales.GetSize() != rhs.Locales.GetSize()) return true;
    for (UPInt i = 0; i < Locales.GetSize(); ++i)
    {
        if (Locales[i] != rhs.Locales[i]) return true;
    }
    if (Alg::Abs(CurveTolerance - rhs.CurveTolerance) > 0.0001) return true;
    if (Alg::Abs(CurveToleranceMin - rhs.CurveToleranceMin) > 0.0001) return true;
    if (Alg::Abs(CurveToleranceMax - rhs.CurveToleranceMax) > 0.0001) return true;
    if (Alg::Abs(CurveToleranceStep - rhs.CurveToleranceStep) > 0.0001) return true;
    if (CurrentFileId != rhs.CurrentFileId) return true;
    if (CurrentLineNumber != rhs.CurrentLineNumber) return true;

    return false;
}


// Serialization
void ServerState::Read(File& str, UInt32 version)
{
    StateFlags = str.ReadUInt32();
    readString(str, &ConnectedApp);
    if (version >= 5)
    {
        readString(str, &ConnectedFile);
    }
    readString(str, &AaMode);
    readString(str, &StrokeType);
    readString(str, &CurrentLocale);
    UInt32 size = str.ReadUInt32();
    Locales.Resize(size);
    for (UInt32 i = 0; i < size; ++i)
    {
        readString(str, &Locales[i]);
    }
    CurveTolerance = str.ReadFloat();
    CurveToleranceMin = str.ReadFloat();
    CurveToleranceMax = str.ReadFloat();
    CurveToleranceStep = str.ReadFloat();
    if (version >= 10)
    {
        CurrentFileId = str.ReadUInt64();
        CurrentLineNumber = str.ReadUInt32();
    }
}

// Serialization
void ServerState::Write(File& str, UInt32 version) const
{
    str.WriteUInt32(StateFlags);
    writeString(str, ConnectedApp);
    if (version >= 5)
    {
        writeString(str, ConnectedFile);
    }
    writeString(str, AaMode);
    writeString(str, StrokeType);
    writeString(str, CurrentLocale);
    str.WriteUInt32(static_cast<UInt32>(Locales.GetSize()));
    for (UPInt i = 0; i < Locales.GetSize(); ++i)
    {
        writeString(str, Locales[i]);
    }
    str.WriteFloat(CurveTolerance);
    str.WriteFloat(CurveToleranceMin);
    str.WriteFloat(CurveToleranceMax);
    str.WriteFloat(CurveToleranceStep);
    if (version >= 10)
    {
        str.WriteUInt64(CurrentFileId);
        str.WriteUInt32(CurrentLineNumber);
    }
}

} // namespace AMP
} // namespace GFx
} // namespace Scaleform
