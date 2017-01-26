/**********************************************************************

Filename    :   SF_MemReport.cpp
Content     :   Forming string buffer with memory report.

Created     :   July 15, 2009
Authors     :   Boris Rayskiy

Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "SF_Memory.h"
#include "SF_String.h"
#include "SF_MsgFormat.h"
#include "SF_RefCount.h"
#include "SF_Alg.h"
#include "SF_Stats.h"
#include "SF_MemItem.h"

namespace Scaleform {


#ifdef SF_ENABLE_STATS

// This class creates the memory report
// It lives here so we don't have to pollute the public MemoryHeap header with internals
//
class StatsUpdate
{
public:
    StatsUpdate() : NextHandle(0) { }
    // Public interface
    // Called by MemoryHeap::MemReport
    void MemReport(MemItem* rootItem, MemoryHeap::MemReportType reportType);

private:
    // Heap visitor that adds all the child heaps to an array
    struct HolderVisitor : public MemoryHeap::HeapVisitor
    {
        Array<MemoryHeap*> Heaps;

        virtual void Visit(MemoryHeap* pParent, MemoryHeap *heap)
        { 
            SF_UNUSED(pParent);
            Heaps.PushBack(heap); 
        }
    };

    // Custom heap visitor class that keeps track of memory per StatID
    class SummaryStatIdVisitor : public MemoryHeap::HeapVisitor
    {
    public:
        SummaryStatIdVisitor(bool debug) : Debug(debug) { }
        void ExcludeHeap(HeapId excludedHeap)
        {
            ExcludedHeaps.PushBack(excludedHeap);
        }
        virtual void Visit(MemoryHeap* parent, MemoryHeap *heap)
        {
            SF_UNUSED(parent);
            if (((heap->GetFlags() & MemoryHeap::Heap_UserDebug) != 0) == Debug)
            {
                bool exclude = false;
                for (unsigned i = 0; i < ExcludedHeaps.GetSize(); ++i)
                {
                    if (heap->GetId() == static_cast<UPInt>(ExcludedHeaps[i]))
                    {
                        exclude = true;
                        break;
                    }
                }

                if (!exclude)
                {
                    StatBag bag;
                    heap->GetStats(&bag);
                    StatIdBag.Add(bag);

                    heap->VisitChildHeaps(this);
                }
            }
        }
        UPInt GetStatIdMemory(StatDesc::Iterator it) const
        {
            StatInfo statInfo;
            UPInt totalMemory = 0;

            if (StatIdBag.GetStat(&statInfo, it->GetId()))
            {
                Stat::StatValue statValue;
                statInfo.GetStat(&statValue, 0);
                totalMemory = statValue.IValue;
            }

            for (StatDesc::Iterator itChild = it->GetChildIterator(); !itChild.IsEnd(); ++itChild)
            {
                totalMemory += GetStatIdMemory(itChild);
            }

            return totalMemory;
        }
    private:
        StatBag         StatIdBag;
        bool            Debug;
        Array<HeapId>   ExcludedHeaps;
    };


    // Custom heap visitor class that keeps track of Movie, Mesh, and Font memory
    class SummaryMemoryVisitor : public MemoryHeap::HeapVisitor
    {
    public:
        SummaryMemoryVisitor(bool debug) : 
          Debug(debug), 
          MovieViewMemory(0), MovieDataMemory(0),
          OtherMemory(0) {}

        virtual void Visit(MemoryHeap* parent, MemoryHeap *heap)
        {
            SF_UNUSED(parent);
            if (((heap->GetFlags() & MemoryHeap::Heap_UserDebug) != 0) == Debug)
            {
                switch (heap->GetId())
                {
                case HeapId_MovieView:
                    MovieViewMemory += heap->GetUsedSpace();
                    break;
                case HeapId_MovieData:
                    MovieDataMemory += heap->GetUsedSpace();
                    break;
                default:
                    OtherMemory += heap->GetUsedSpace();
                    break;
                }
            }
        }
        UPInt           GetMovieViewMemory() const { return MovieViewMemory; }
        UPInt           GetMovieDataMemory() const { return MovieDataMemory; }
        UPInt           GetOtherMemory() const { return OtherMemory; }
    private:
        UPInt           MovieViewMemory;
        UPInt           MovieDataMemory;
        UPInt           OtherMemory;
        bool            Debug;
    };

    // Custom heap visitor class that keeps track of Movie, Mesh, and Font memory
    class HeapTypeVisitor : public MemoryHeap::HeapVisitor
    {
    public:
        HeapTypeVisitor(bool debug) : Debug(debug), GlobalMemory(0), MovieViewMemory(0), MovieDataMemory(0), 
            OtherMemory(0) {}

          virtual void Visit(MemoryHeap* parent, MemoryHeap *heap)
          {
              SF_UNUSED(parent);
              if (((heap->GetFlags() & MemoryHeap::Heap_UserDebug) != 0) == Debug)
              {
                  switch (heap->GetId())
                  {
                  case HeapId_Global:
                      GlobalHeaps.PushBack(heap);
                      GlobalMemory += heap->GetUsedSpace();
                      break;
                  case HeapId_MovieView:
                      MovieViewHeaps.PushBack(heap);
                      MovieViewMemory += heap->GetUsedSpace();
                      break;
                  case HeapId_MovieData:
                      MovieDataHeaps.PushBack(heap);
                      MovieDataMemory += heap->GetUsedSpace();
                      break;
                  default:
                      OtherHeaps.PushBack(heap);
                      OtherMemory += heap->GetUsedSpace();
                      break;
                  }
              }
          }

          UPInt           GetGlobalMemory() const    { return MovieViewMemory; }
          UPInt           GetMovieViewMemory() const { return MovieViewMemory; }
          UPInt           GetMovieDataMemory() const { return MovieDataMemory; }
          UPInt           GetOtherMemory() const     { return OtherMemory; }

          const Array<MemoryHeap*>& GetMemHeaps(HeapId heapId) const
          {
              switch (heapId)
              {
              case HeapId_Global:
                  return GlobalHeaps;
              case HeapId_MovieView:
                  return MovieViewHeaps;
              case HeapId_MovieData:
                  return MovieDataHeaps;
              default:
                  return OtherHeaps;
              }
          }


    private:
        bool                                Debug;
        Array<MemoryHeap*>                  GlobalHeaps;
        Array<MemoryHeap*>                  MovieViewHeaps;
        Array<MemoryHeap*>                  MovieDataHeaps;
        Array<MemoryHeap*>                  OtherHeaps;
        UPInt                               GlobalMemory;
        UPInt                               MovieViewMemory;
        UPInt                               MovieDataMemory;
        UPInt                               OtherMemory;
    };


    // This struct holds the memory stats for a file
    // It is part of the heap visitor, FileVisitor, below
    struct FileStats
    {
        FileStats() : TotalMemory(0) { }
        StatBag         Bag;
        UPInt           TotalMemory;
    };

    // Heap visitor that traverses the tree and creates a map of 
    // file names to memory statistics
    // The total memory for each file is not updated by the visitor
    struct FileVisitor : public MemoryHeap::HeapVisitor
    {
        StringHash<FileStats> FileStatsMap;

        // recursively updates the file statistics by examining child heaps
        void UpdateMovieHeap(MemoryHeap* heap, StatBag* fileStatBag)
        {
            if ((heap->GetFlags() & MemoryHeap::Heap_UserDebug) == 0)
            {
                StatBag statBag;
                heap->GetStats(&statBag);
                *fileStatBag += statBag;

                HolderVisitor visitor;
                heap->VisitChildHeaps(&visitor);
                for (UPInt i = 0; i < visitor.Heaps.GetSize(); ++i)
                {
                    UpdateMovieHeap(visitor.Heaps[i], fileStatBag);
                }
            }
        }

        virtual void Visit(MemoryHeap* parent, MemoryHeap* heap)
        { 
            SF_UNUSED(parent);

            if ((heap->GetFlags() & MemoryHeap::Heap_UserDebug) != 0)
            {
                return;
            }

            switch (heap->GetId())
            {
            case HeapId_MovieDef:
            case HeapId_MovieData:
            case HeapId_MovieView:
                {
                    // Recover the file name
                    String heapName = heap->GetName();
                    UPInt heapNameLength = heapName.GetLength();
                    UPInt startIndex = 0;
                    UPInt endIndex = heapNameLength;
                    for (UPInt i = 0; i < heapNameLength; ++i)
                    {
                        UPInt iIndex = heapNameLength - i - 1;
                        if (heapName[iIndex] == '"')
                        {
                            if (endIndex == heapNameLength)
                            {
                                endIndex = iIndex;
                            }
                            else
                            {
                                startIndex = iIndex + 1;
                                break;
                            }
                        }
                    }
                    SF_ASSERT(startIndex < endIndex);
                    String filename = heapName.Substring(startIndex, endIndex);

                    // Add the file name to the map
                    StringHash<FileStats>::Iterator it = FileStatsMap.FindCaseInsensitive(filename);
                    if (it == FileStatsMap.End())
                    {
                        FileStatsMap.SetCaseInsensitive(filename, FileStats());
                        it = FileStatsMap.FindCaseInsensitive(filename);
                    }

                    // update the stats for this file
                    UpdateMovieHeap(heap, &(it->Second.Bag));
                }
                break;
            default:
                break;
            }
        }
    };

    UInt32 GetRoundUpKilobytes(UPInt bytes);
    UInt32 GetNearestKilobytes(UPInt bytes);

    void MemReportHeaps(MemoryHeap* pHeap, MemItem* rootItem, MemoryHeap::MemReportType reportType);
    void GetHeapMemory(StatDesc::Iterator it, StatBag* statBag, MemItem* rootItem);
    bool GetStatMemory(StatDesc::Iterator it, StatBag* statBag, MemItem* rootItem);
    void MemReportFile(MemItem* rootItem, MemoryHeap::MemReportType reportType);
    void MemReportHeapsDetailed(MemItem* rootItem, MemoryHeap* heap);
    UPInt GetFileMemory(StatDesc::Iterator it, FileStats& kFileStats, MemItem* rootItem, MemoryHeap::MemReportType reportType);

    UInt32 NextHandle;
};

void StatsUpdate::MemReport(MemItem* rootItem, MemoryHeap::MemReportType reportType)
{
    rootItem->ID = NextHandle++;
    rootItem->StartExpanded = true;

    MemoryHeap::RootStats rootStats;
    Memory::pGlobalHeap->GetRootStats(&rootStats);

    switch (reportType)
    {
    case MemoryHeap::MemReportHeapDetailed:
        MemReportHeapsDetailed(rootItem, Memory::pGlobalHeap);
        break;

    case MemoryHeap::MemReportFileSummary:
        MemReportFile(rootItem, reportType);
        break;

    default:

        if (reportType != MemoryHeap::MemReportSummary && reportType != MemoryHeap::MemReportHeapsOnly && reportType != MemoryHeap::MemReportBrief)
        {
            Format(rootItem->Name,
                "Memory {0:sep:,}K / {1:sep:,}K",
                GetNearestKilobytes(rootStats.SysMemUsedSpace - rootStats.UserDebugFootprint - 
                rootStats.DebugInfoFootprint),
                GetNearestKilobytes(rootStats.SysMemFootprint - rootStats.UserDebugFootprint - 
                rootStats.DebugInfoFootprint));

            MemItem* sysSummaryItem = rootItem->AddChild(NextHandle++, "System Summary");
            sysSummaryItem->AddChild(NextHandle++, "System Memory FootPrint", static_cast<UInt32>(rootStats.SysMemFootprint));
            sysSummaryItem->AddChild(NextHandle++, "System Memory Used Space", static_cast<UInt32>(rootStats.SysMemUsedSpace));
            sysSummaryItem->AddChild(NextHandle++, "Page Mapping Footprint", static_cast<UInt32>(rootStats.PageMapFootprint));
            sysSummaryItem->AddChild(NextHandle++, "Page Mapping UsedSpace", static_cast<UInt32>(rootStats.PageMapUsedSpace));
            sysSummaryItem->AddChild(NextHandle++, "Bookkeeping Footprint", static_cast<UInt32>(rootStats.BookkeepingFootprint));
            sysSummaryItem->AddChild(NextHandle++, "Bookkeeping Used Space", static_cast<UInt32>(rootStats.BookkeepingUsedSpace));
            sysSummaryItem->AddChild(NextHandle++, "Debug Info Footprint", static_cast<UInt32>(rootStats.DebugInfoFootprint));
            sysSummaryItem->AddChild(NextHandle++, "Debug Info Used Space", static_cast<UInt32>(rootStats.DebugInfoUsedSpace));
            sysSummaryItem->AddChild(NextHandle++, "Debug Heaps Footprint", static_cast<UInt32>(rootStats.UserDebugFootprint));
            sysSummaryItem->AddChild(NextHandle++, "Debug Heaps Used Space", static_cast<UInt32>(rootStats.UserDebugUsedSpace));
        }

        if (reportType != MemoryHeap::MemReportHeapsOnly)
        {
            MemItem* summaryItem;
            
            if (reportType == MemoryHeap::MemReportBrief)
            {
                summaryItem = rootItem;
            }
            else
            {
                summaryItem = rootItem->AddChild(NextHandle++, "Summary");
            }
            summaryItem->StartExpanded = true;

            SummaryStatIdVisitor statVisit(false);
            Memory::GetGlobalHeap()->VisitChildHeaps(&statVisit);

            SummaryMemoryVisitor heapVisit(false);
            Memory::GetGlobalHeap()->VisitChildHeaps(&heapVisit);

            summaryItem->AddChild(NextHandle++, "Image", 
                static_cast<UInt32>(statVisit.GetStatIdMemory(StatDesc::GetGroupIterator(Stat_Image_Mem))));

            summaryItem->AddChild(NextHandle++, "Renderer", 
                static_cast<UInt32>(statVisit.GetStatIdMemory(StatDesc::GetGroupIterator(StatGroup_Renderer + 1))));

#ifdef GFX_ENABLE_SOUND
            summaryItem->AddChild(NextHandle++, "Sound",
                static_cast<UInt32>(statVisit.GetStatIdMemory(StatDesc::GetGroupIterator(Stat_Sound_Mem)))); 
#endif

#ifdef GFX_ENABLE_VIDEO
            summaryItem->AddChild(NextHandle++, "Video",
                static_cast<UInt32>(statVisit.GetStatIdMemory(StatDesc::GetGroupIterator(Stat_Video_Mem))));
#endif
            summaryItem->AddChild(NextHandle++, "Movie View", static_cast<UInt32>(heapVisit.GetMovieViewMemory()));
            summaryItem->AddChild(NextHandle++, "Movie Data", static_cast<UInt32>(heapVisit.GetMovieDataMemory()));
        }

        if (reportType != MemoryHeap::MemReportSummary && reportType != MemoryHeap::MemReportBrief)
        {
            MemReportHeaps(Memory::pGlobalHeap, rootItem, reportType);
        }
        break;
    }
}



UInt32 StatsUpdate::GetRoundUpKilobytes(UPInt bytes)
{
    return (static_cast<UInt32>(bytes) + 1023) / 1024;
}

UInt32 StatsUpdate::GetNearestKilobytes(UPInt bytes)
{
    return (static_cast<UInt32>(bytes) + 512) / 1024;
}

void StatsUpdate::MemReportHeaps(MemoryHeap* pHeap, MemItem* rootItem, MemoryHeap::MemReportType reportType)
{
    if (((pHeap->GetFlags() & MemoryHeap::Heap_UserDebug) == 0) && pHeap->GetFootprint() > 0)
    {
        StatBag            statBag;
        StatInfo           statInfo;
        Stat::StatValue    statValue;

        pHeap->GetStats(&statBag);

        statBag.GetStat(&statInfo, StatHeap_TotalFootprint);
        statInfo.GetStat(&statValue, 0);

        String buffer;
        Format(buffer, "[Heap] {0}", pHeap->GetName());
        MemItem* heapItem = rootItem->AddChild(NextHandle++, buffer, static_cast<UInt32>(statValue.IValue));

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
        if (reportType == MemoryHeap::MemReportFull || reportType == MemoryHeap::MemReportMedium)
        {
            StatDesc::Iterator it = StatDesc::GetGroupIterator(StatGroup_Core);
//#ifdef SF_BUILD_DEBUG
            if (reportType == MemoryHeap::MemReportFull)
            {
                it = StatDesc::GetGroupIterator(StatGroup_Default);
            }
//#endif

            GetHeapMemory(it, &statBag, heapItem);

            statBag.UpdateGroups();
            StatInfo sumStat;
            if (statBag.GetStat(&sumStat, Stat_Mem))
            {
                heapItem->AddChild(NextHandle++, "Allocations Count", static_cast<UInt32>(sumStat.ToMemoryStat()->GetAllocCount()));
            }
        }
#endif
        if (reportType != MemoryHeap::MemReportMedium)
        {
            HolderVisitor hv;
            pHeap->VisitChildHeaps(&hv);

            for (UPInt i = 0; i < hv.Heaps.GetSize(); ++i)
            {
                MemReportHeaps(hv.Heaps[i], heapItem, reportType);
            }
        }
    }
}

// Invokes recursively memory statistic data belonging to the given heap.
void StatsUpdate::GetHeapMemory(StatDesc::Iterator it, StatBag* statBag, MemItem* rootItem)
{
    const StatDesc* pdesc = *it;

    if (pdesc)
    {
        Ptr<MemItem> childItem;
        if (pdesc->GetGroupId() == 0)
        {
            if (pdesc->GetId() != 0)
            {
                if ((pdesc->GetType() == (UByte)Stat::Stat_Memory) ||
                    (pdesc->GetType() == (UByte)Stat::Stat_LogicalGroup))
                {
                    childItem = *SF_HEAP_AUTO_NEW(rootItem) MemItem(NextHandle++);
                    childItem->Name = pdesc->GetName();
                    if (pdesc->GetId() != StatHeap_Summary)
                    {
                        childItem->StartExpanded = true;
                    }
                }
            }
        }

        if (!childItem)
        {
            childItem = *SF_HEAP_AUTO_NEW(rootItem) MemItem(NextHandle++);
            childItem->Name = pdesc->GetName();
        }

        StatDesc::Iterator ichild = pdesc->GetChildIterator();  
        while (!ichild.IsEnd())
        {
            if ((pdesc->GetGroupId() == 0) && (pdesc->GetId() == 0))
            {
                GetHeapMemory(ichild, statBag, rootItem);
            }
            else if (childItem)
            {
                GetHeapMemory(ichild, statBag, childItem);
            }

            if (childItem)
            {
                StatInfo           statInfo;
                Stat::StatValue    statValue;

                if (statBag->GetStat(&statInfo, ichild.GetId()))
                {
                    statInfo.GetStat(&statValue, 0);
                    if (statValue.IValue > 0)
                    {
                        childItem->AddChild(NextHandle++, statInfo.GetName(), static_cast<UInt32>(statValue.IValue));
                    }
                }
            }
            ++ichild;
        }

        if (childItem && childItem->Children.GetSize() > 0)
        {
            rootItem->Children.PushBack(childItem);
        }
    }
}

// Invokes recursively memory statistic data belonging to the given heap.
bool StatsUpdate::GetStatMemory(StatDesc::Iterator it, StatBag* statBag, MemItem* rootItem)
{
    bool foundNonZeroItem = false;
    const StatDesc* pdesc = *it;

    if (pdesc)
    {
        StatDesc::Iterator ichild = pdesc->GetChildIterator();  
        while (!ichild.IsEnd())
        {
            bool                nonZeroChild = false;
            StatInfo            statInfo;
            Stat::StatValue     statValue;
            Ptr<MemItem> childItem = *SF_HEAP_AUTO_NEW(rootItem) MemItem(NextHandle++);
            childItem->Name = ichild->GetName();

            if (statBag->GetStat(&statInfo, ichild.GetId()))
            {
                statInfo.GetStat(&statValue, 0);
                UInt32 childValue = static_cast<UInt32>(statValue.IValue);
                if (childValue > 0)
                {
                    childItem->SetValue(childValue);
                    rootItem->SetValue(rootItem->Value + childValue);
                    nonZeroChild = true;
                }
            }

            if (GetStatMemory(ichild, statBag, childItem))
            {
                nonZeroChild = true;
            }

            if (nonZeroChild)
            {
                rootItem->Children.PushBack(childItem);
                foundNonZeroItem = true;
            }

            ++ichild;
        }
    }
    return foundNonZeroItem;
}

void StatsUpdate::MemReportFile(MemItem* rootItem, MemoryHeap::MemReportType reportType)
{
    FileVisitor visitor;
    Memory::pGlobalHeap->VisitChildHeaps(&visitor);

    StringHash<FileStats>& statsMap = visitor.FileStatsMap;
    StringHash<FileStats>::Iterator itFile;
    for (itFile = statsMap.Begin(); itFile != statsMap.End(); ++itFile)
    {
        String buffer;
        Format(buffer, "Movie File {0}", itFile->First);
        MemItem* childItem = rootItem->AddChild(NextHandle++, buffer);

        GetFileMemory(StatDesc::GetGroupIterator(Stat_Mem), itFile->Second, childItem, reportType);
    }
}

void StatsUpdate::MemReportHeapsDetailed(MemItem* rootItem, MemoryHeap* heap)
{
    MemoryHeap::RootStats rootStats;
    heap->GetRootStats(&rootStats);

    MemItem* sysTotalItem = rootItem->AddChild(NextHandle++, "Total Footprint", static_cast<UInt32>(rootStats.SysMemFootprint));
    sysTotalItem->StartExpanded = true;
    MemItem* sysUsedItem = sysTotalItem->AddChild(NextHandle++, "Used Space", static_cast<UInt32>(rootStats.SysMemUsedSpace - rootStats.UserDebugFootprint));
    sysUsedItem->StartExpanded = true;
    sysTotalItem->AddChild(NextHandle++, "Debug Data", static_cast<UInt32>(rootStats.UserDebugFootprint));
    MemItem* sysUnusedItem = sysTotalItem->AddChild(NextHandle++, "Unused Space", static_cast<UInt32>(rootStats.SysMemFootprint - rootStats.SysMemUsedSpace));

    HeapTypeVisitor heapVisit(false);
    heapVisit.Visit(NULL, heap);
    heap->VisitChildHeaps(&heapVisit);
    MemItem* globalMemItem = sysUsedItem->AddChild(NextHandle++, "Global Heap", static_cast<UInt32>(heapVisit.GetGlobalMemory()));
    MemItem* viewMemItem = sysUsedItem->AddChild(NextHandle++, "Movie View Heaps", static_cast<UInt32>(heapVisit.GetMovieViewMemory()));
    MemItem* dataMemItem = sysUsedItem->AddChild(NextHandle++, "Movie Data Heaps", static_cast<UInt32>(heapVisit.GetMovieDataMemory()));
    MemItem* otherMemItem = sysUsedItem->AddChild(NextHandle++, "Other Heaps", static_cast<UInt32>(heapVisit.GetOtherMemory()));

    const Array<MemoryHeap*>* heaps = &heapVisit.GetMemHeaps(HeapId_MovieView);
    StatBag statBag;
    heap->GetStats(&statBag);
    GetStatMemory(StatDesc::GetGroupIterator(Stat_Mem), &statBag, globalMemItem);
    sysUnusedItem->AddChild(NextHandle++, heap->GetName(), heap->GetFootprint() - heap->GetUsedSpace());

    for (UPInt i = 0; i < heaps->GetSize(); ++i)
    {
        MemoryHeap& heapi = *((*heaps)[i]);
        MemItem* heapMemItem = viewMemItem->AddChild(NextHandle++, heapi.GetName(), 0);
        StatBag statBag;
        heapi.GetStats(&statBag);
        GetStatMemory(StatDesc::GetGroupIterator(Stat_Mem), &statBag, heapMemItem);
        sysUnusedItem->AddChild(NextHandle++, heapi.GetName(), heapi.GetFootprint() - heapi.GetUsedSpace());
    }
    heaps = &heapVisit.GetMemHeaps(HeapId_MovieData);
    for (UPInt i = 0; i < heaps->GetSize(); ++i)
    {
        MemoryHeap& heapi = *((*heaps)[i]);
        MemItem* heapMemItem = dataMemItem->AddChild(NextHandle++, heapi.GetName(), 0);
        StatBag statBag;
        heapi.GetStats(&statBag);
        GetStatMemory(StatDesc::GetGroupIterator(Stat_Mem), &statBag, heapMemItem);
        sysUnusedItem->AddChild(NextHandle++, heapi.GetName(), heapi.GetFootprint() - heapi.GetUsedSpace());
    }
    heaps = &heapVisit.GetMemHeaps(HeapId_OtherHeaps);
    for (UPInt i = 0; i < heaps->GetSize(); ++i)
    {
        MemoryHeap& heapi = *((*heaps)[i]);
        MemItem* heapMemItem = otherMemItem->AddChild(NextHandle++, heapi.GetName(), 0);
        StatBag statBag;
        heapi.GetStats(&statBag);
        GetStatMemory(StatDesc::GetGroupIterator(Stat_Mem), &statBag, heapMemItem);
        sysUnusedItem->AddChild(NextHandle++, heapi.GetName(), heapi.GetFootprint() - heapi.GetUsedSpace());
    }    
}



// Recursively adds the memory per StatId for the given file
UPInt StatsUpdate::GetFileMemory(StatDesc::Iterator it, FileStats& fileStats, MemItem* rootItem, MemoryHeap::MemReportType reportType)
{
    StatInfo           statInfo;
    Stat::StatValue    statValue;

    UPInt total = 0;
    if (fileStats.Bag.GetStat(&statInfo, it.GetId()))
    {
        statInfo.GetStat(&statValue, 0);

        fileStats.TotalMemory += statValue.IValue;
        total += statValue.IValue;
    }

    Ptr<MemItem> childItem = *SF_HEAP_AUTO_NEW(rootItem) MemItem(NextHandle++);
    for (StatDesc::Iterator itChild = it->GetChildIterator(); !itChild.IsEnd(); ++itChild)
    {

        UPInt childTotal = GetFileMemory(itChild, fileStats, childItem, reportType);
        if (childTotal > 0)
        {
            total += childTotal;
        }
    }

    if (total > 0)
    {
        childItem->Name = it->GetName();
        childItem->SetValue(static_cast<UInt32>(total));
        rootItem->Children.PushBack(childItem);
    }

    return total;
}

//------------------------------------------------------------------------
// Collects memory statistic data from heaps and puts obtained data in string
// buffer. 
void MemoryHeap::MemReport(StringBuffer& buffer, MemReportType reportType)
{   
    Ptr<MemItem> rootItem = *SF_NEW MemItem(0);
    StatsUpdate().MemReport(rootItem, reportType);
    rootItem->ToString(&buffer);
}

void MemoryHeap::MemReport(MemItem* rootItem, MemReportType reportType)
{
    StatsUpdate().MemReport(rootItem, reportType);
}

#else // SF_ENABLE_STATS

void MemoryHeap::MemReport(StringBuffer& buffer, MemReportType reportType)
{
    SF_UNUSED(buffer);
    SF_UNUSED(reportType);
}

void MemoryHeap::MemReport(MemItem* rootItem, MemReportType reportType)
{
    SF_UNUSED(rootItem);
    SF_UNUSED(reportType);
}


#endif // SF_ENABLE_STATS

} // Scaleform

