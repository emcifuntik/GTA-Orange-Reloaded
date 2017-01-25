/**********************************************************************

Filename    :   LoaderImpl.cpp
Content     :   GFxPlayer loader implementation
Created     :   June 30, 2005
Authors     :   

Copyright   :   (c) 2001-2006 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"
#include "Render/Render_Image.h"

#include "GFx/GFx_LoaderImpl.h"

#include "GFx/GFx_LoadProcess.h"
#include "GFx/GFx_ImageResource.h"

#include "GFx/GFx_Log.h"
#include "GFx/GFx_TaskManager.h"

#include <string.h> // for memset
#include <float.h>

namespace Scaleform { namespace GFx {

int StateAccessCount = 0;

/*********** KNOWN ISSUE WITH LOADING ********************************************************
There is a known issue with our loading process: reloading a swf file does not detect 
changes made in imported files. The only workaround around this problem is to release 
all references to the main movie and the reload it again completely with all it 
dependencies instead of reloading only modified files. This is not a very efficient way. 
The solution for this problem could be to hold a list of {MovieDefImpl,timestamp} objects
for each MovieDataDef in the Resource lib. When this list is retrieved from Resource Lib
we would take its last item and then go over all imports in a MovieDataDef find theirs
MovieDefImpl in that item�s ImportedMovieSource vector and check if any of imported movies
is changed. If no changes are found we return this last MovieDefImpl to the user. If we 
find a change we will need to create a new MovieDefImpl for a given MovieDataDef add it
to the list that we got from the Resource Lib and return this new object to the user. 
(Keep in mind that the search should be done recursively because there could be a chain 
of imports-exports).  
The solution is not completely thought through because there could be a loading inconsistence. 
Let say Movie1 imports MovieA and MovieB and MovieB also imports MovieA. Then Movie1 loads 
its imports first it loads MovieA and then MovieB, but while MovieB is being load MovieA could
be changed. When MovieB starts loading MovieA it will detect the change and load a new version 
of MovieA (MovieA.1). After loading is complete Movie1 will use MovieA but MovieB will use MovieA.1. 
We decided not to invest more time in fixing this issue for now, because it can take a lot time
but will benefit a couple of our user.
**************************************************************************************************/


// ***** ExporterInfoImpl Loading

// Assigns data
void    ExporterInfoImpl::SetData(UInt16 version, Loader::FileFormatType format,
                                     const char* pname, const char* pprefix, unsigned flags, const Array<UInt32>* codeOffsets)
{
    SI.Version  = version;
    SI.Format   = format;
    Prefix      = (pprefix) ? pprefix : "";
    SWFName     = (pname) ? pname : "";
    SI.pSWFName = SWFName.ToCStr(); // Update SI pointers.
    SI.pPrefix  = Prefix.ToCStr();
    SI.ExportFlags = flags;
    if (codeOffsets != NULL)
    {
        CodeOffsets = *codeOffsets;
    }
    else
    {
        CodeOffsets.Clear();
    }
}

void    ExporterInfoImpl::ReadExporterInfoTag(Stream *pin, TagType tagType)
{
    SF_UNUSED(tagType);
    SF_ASSERT(tagType == Tag_ExporterInfo);

    // Utilizes the tag 1000 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1000
    // Version          UI16            Version (1.10 will be encoded as 0x10A)
    // Flags            UI32            Version 1.10 (0x10A) and above - flags
    //      Bit0 - Contains glyphs' textures info (tags 1002)
    //      Bit1 - Glyphs are stripped from DefineFont tags
    //      Bit2 - Indicates gradients' images were exported
    // BitmapsFormat    UI16            1 - TGA
    //                                  2 - DDS
    // PrefixLen        UI8
    // Prefix           UI8[PrefixLen]
    // SwfNameLen       UI8
    // SwfName          UI8[SwfNameLen]

    UInt32  flags = 0;
    UInt16  version  = pin->ReadU16(); // read version
    if (version >= 0x10A)
        flags = pin->ReadU32();
    UInt16  bitmapFormat = pin->ReadU16();

    String fxstr, swfstr;
    pin->ReadStringWithLength(&fxstr);
    pin->ReadStringWithLength(&swfstr);

    Array<UInt32> codeOffsets;
    if (version >= 0x401)
    {
        UInt16 numCodeOffsets = pin->ReadU16();
        for (UInt16 i = 0; i < numCodeOffsets; ++i)
        {
            codeOffsets.PushBack(pin->ReadU32());
        }
    }

    pin->LogParse("  ExportInfo: tagType = %d, tool ver = %d.%d, imgfmt = %d, prefix = '%s', swfname = '%s', flags = 0x%X\n",
        tagType,
        (version>>8), (version&0xFF),
        bitmapFormat,
        fxstr.ToCStr(),
        swfstr.ToCStr(),
        (int)flags);

    SetData(version, (Loader::FileFormatType)bitmapFormat, swfstr.ToCStr(), fxstr.ToCStr(), flags, &codeOffsets);
}


// Processes and reads in a SWF file header and opens the Stream
// If 0 is returned, there was an error and error message is already displayed
bool    SWFProcessInfo::Initialize(File *pin, LogState *plog, ZlibSupportBase* zlib,
                                   ParseControl* pparseControl, bool parseMsg)
{
    UInt32  header;
    bool    compressed;

    FileStartPos        = pin->Tell();
    header              = pin->ReadUInt32();
    Header.FileLength   = pin->ReadUInt32();
    FileEndPos          = FileStartPos + Header.FileLength;
    NextActionBlock     = 0;
    Header.Version      = (header >> 24) & 255;
    Header.SWFFlags     = 0;
    compressed          = (header & 255) == 'C';
    FileAttributes      = 0;

    // Verify header
    if ( ((header & 0x0FFFFFF) != 0x00535746) && // FWS
        ((header & 0x0FFFFFF) != 0x00535743) && // CWS
        ((header & 0x0FFFFFF) != 0x00584647) && // GFX
        ((header & 0x0FFFFFF) != 0x00584643) )  // CFX
    {
        // ERROR
        if (plog)
            plog->LogError("Loader read failed - file does not start with a SWF header");
        return 0;
    }
    if (((header >> 16) & 0xFF) == 'X')
        Header.SWFFlags |= MovieInfo::SWF_Stripped;
    if (compressed)
        Header.SWFFlags |= MovieInfo::SWF_Compressed;

    // Parse messages will not be generated if they are disabled by ParseControl.
    if (!plog || !pparseControl || !pparseControl->IsVerboseParse())
        parseMsg = false;

    if (parseMsg)
        plog->LogMessageByType(Log_Parse,
        "SWF File version = %d, File length = %d\n",
        Header.Version, Header.FileLength);

    // AddRef to file
    Ptr<File> pfileIn = pin;  
    if (compressed)
    {
#ifndef SF_ENABLE_ZLIB
        SF_UNUSED(zlib);
        if (plog)
            plog->LogError("Loader - unable to read compressed SWF data; SF_ENABLE_ZLIB not defined");
        return 0;
#else
        if (!zlib)
        {
            if (plog)
                plog->LogError("Loader - unable to read compressed SWF data; GFxZlibState is not set.");
            return 0;
        }
        if (parseMsg)
            plog->LogMessageByType(Log_Parse, "SWF file is compressed.\n");

        // Uncompress the input as we read it.
        pfileIn = *zlib->CreateZlibFile(pfileIn);

        // Subtract the size of the 8-byte header, since
        // it's not included pin the compressed
        // Stream length.
        FileEndPos = Header.FileLength - 8;
#endif
    }

    // Initialize stream, this AddRefs to file
    Stream.Initialize(pfileIn, plog->GetLog(), pparseControl);

    // Read final data
    Stream.ReadRect(& Header.FrameRect);
    Header.FPS         = Stream.ReadU16() / 256.0f;
    Header.FrameCount  = Stream.ReadU16();

    // Read the exporter tag, which must be the first tag in the GFX file.
    // We require this tag to be included in the very beginning of file because:
    //  1. Some of its content is reported by GFxMovieInfo.
    //  2. Reporting it from cached MovieDataDef would require a
    //     'wait-for-load' in cases when most of the loading is done
    //     on another thread.

    if (Header.SWFFlags & MovieInfo::SWF_Stripped)
    {
        if ((UInt32) Stream.Tell() < FileEndPos)
        {
            if (Stream.OpenTag() == Tag_ExporterInfo)
            {
                Header.mExporterInfo.ReadExporterInfoTag(&Stream, Tag_ExporterInfo);
//                 if ((Header.mExporterInfo.GetExporterInfo()->Version & (~0xFF)) != 0x400)
//                 {
//                     // Only gfx from exporter 3.0 are supported
//                     if (plog)
//                         plog->LogError(
//                         "Loader read failed - incompatible GFX file, version 4.x expected");
//                     return 0;
//                 }
            }
            else
            {
                if (plog)
                    plog->LogError(
                    "Loader read failed - no mExporterInfo tag in GFX file header");
                return 0;
            }
            Stream.CloseTag();
        }
        // Do not seek back; we advance the tags by one, as appropriate.
    }
    // need to read file attributes tag right here for SWF9+ in order
    // to determine is it AS2 or AS3 file.
    if (Header.Version >= 9 && (UInt32) Stream.Tell() < FileEndPos)
    {
        TagInfo tagInfo;
        if (Stream.OpenTag(&tagInfo) == Tag_FileAttributes)
        {
            FileAttributes = Stream.ReadU16();
        }
        Stream.CloseTag();
        Stream.SetPosition(tagInfo.TagOffset); // revert pos back
    }
    
    return 1;
}

// ***** LoaderTask - implementation

LoaderTask::LoaderTask(LoadStates* pls, TaskId id)
: Task(id), pLoadStates(pls)
{
    //printf("LoaderTask::LoaderTask : %x, thread : %d\n", this, GetCurrentThreadId());
    pLoadStates->pLoaderImpl->RegisterLoadProcess(this);
}
LoaderTask::~LoaderTask()
{
    //printf("LoaderTask::~LoaderTask : %x, thread : %d\n", this, GetCurrentThreadId());
    pLoadStates->pLoaderImpl->UnRegisterLoadProcess(this);
}

// ***** LoaderImpl - loader implementation


LoaderImpl::LoaderImpl(ResourceLib* plib, bool debugHeap)
: DebugHeap(debugHeap)
{    
    if (plib)
        pWeakResourceLib = plib->GetWeakLib();

    if (pStateBag = *new StateBagImpl(0))
    {
        pStateBag->SetLog(Ptr<Log>(*new Log));
        pStateBag->SetImageCreator(Ptr<ImageCreator>(*new ImageCreator));

        // By default there should be no glyph packer
        //pStateBag->SetFontPackParams(0);
        //pStateBag->SetFontPackParams(Ptr<FontPackParams>(*new FontPackParams));

        // It's mandatory to have the cache manager for text rendering to work,
        // even if the dynamic cache isn't used. 
//@FONT
//         pStateBag->SetFontCacheManager(
//             Ptr<FontCacheManager>(*new FontCacheManager(true, DebugHeap)));

        pStateBag->SetTextClipboard(Ptr<TextClipboard>(*new TextClipboard));
        pStateBag->SetTextKeyMap(Ptr<TextKeyMap>(*(new TextKeyMap)->InitWindowsKeyMap()));
    }
}

LoaderImpl::LoaderImpl(LoaderImpl* psource)
: pWeakResourceLib(psource->pWeakResourceLib), DebugHeap(psource->DebugHeap)
{   
    if (pStateBag = *new StateBagImpl(0))
    {
        if (psource->pStateBag)
            pStateBag->CopyStatesFrom(psource->pStateBag);
        else
        {
            pStateBag->SetLog(Ptr<Log>(*new Log));

            // By default there should be no glyph packer
            //pStateBag->SetFontPackParams(0);
            //pStateBag->SetFontPackParams(Ptr<FontPackParams>(*new FontPackParams));

            // It's mandatory to have the cache manager for text rendering to work,
            // even if the dynamic cache isn't used. 
//@FONT
//             pStateBag->SetFontCacheManager(
//                 Ptr<FontCacheManager>(*new FontCacheManager(true, DebugHeap)));
        }      
    }
}

LoaderImpl::~LoaderImpl()
{
    CancelLoading();
}



// Obtains information about SWF file and checks for its availability.
// Return 1 if the info was obtained successfully (or was null, but SWF file existed),
// or 0 if it did not exist. Pass LoadCheckLibrary if the library should be checked before loading the file.
// Specifying LoadFromLibraryOnly can be used to check for presence of the file in the library.
bool    LoaderImpl::GetMovieInfo(const char *pfilename, MovieInfo *pinfo,
                                 bool getTagCount, unsigned loadConstants)
{    
    if (!pinfo)
    {
        SF_DEBUG_WARNING(1, "Loader::GetMovieInfo failed, pinfo argument is null");
        return 0;
    }
    pinfo->Clear();

    // LOCK
    // Capture loading states/variables used during loading.
    Ptr<LoadStates>  pls = *new LoadStates(this);
    // UNLOCK

    if (!pls->GetLib())
    {
        SF_DEBUG_WARNING(1, "Loader::GetMovieInfo failed, ResourceLibrary does not exist");
        return 0;
    }


    // Translate the filename.
    URLBuilder::LocationInfo loc(URLBuilder::File_Regular, pfilename);
    String                   fileName;
    pls->BuildURL(&fileName, loc);

    // Use the MovieDataDef version already in the library if necessary.
    Ptr<Resource>    pmovieDataResource;

    //    if (loadConstants & Loader::LoadCheckLibrary)
    {
        // Image creator is only used as a key if it is bound to, based on flags.    
        ImageCreator* pkeyImageCreator = pls->GetLoadTimeImageCreator(loadConstants);

        FileOpener *pfileOpener = pls->GetFileOpener();
        SInt64         modifyTime  = pfileOpener ?
            pfileOpener->GetFileModifyTime(fileName.ToCStr()) : 0;

        ResourceKey fileDataKey = 
            MovieDataDef::CreateMovieFileKey(fileName.ToCStr(), modifyTime,
            pfileOpener, pkeyImageCreator,
            pls->GetPreprocessParams());
        pmovieDataResource = *pls->GetLib()->GetResource(fileDataKey);
    }

    if (pmovieDataResource)
    {
        // Fetch the data from MovieDataDef.
        MovieDataDef* pmd = (MovieDataDef*)pmovieDataResource.GetPtr();
        pmd->GetMovieInfo(pinfo);

        if (getTagCount)
        {
            // TBD: This may have to block for MovieDef to load.
            pinfo->TagCount = pmd->GetTagCount();
        }
    }
    else
    {
        // Open the file; this will automatically do the logging on error.
        Ptr<File> pin = *pls->OpenFile(fileName.ToCStr());
        if (!pin)
            return 0;

        // Open and real file header, failing if it doesn't match.
        SWFProcessInfo pi(Memory::GetGlobalHeap());
        if (!pi.Initialize(pin, pls->GetLogState(), pls->GetZlibSupport(), pls->pParseControl))
            return 0;

        // Store header data.
        pi.Header.GetMovieInfo(pinfo);

        if (getTagCount)
        {
            // Count tags.
            // pinfo->TagCount starts out at 0 after Clear
            while ((UInt32) pi.Stream.Tell() < pi.FileEndPos)
            {
                pi.Stream.OpenTag();
                pi.Stream.CloseTag();
                pinfo->TagCount++;
            }
        }

        // Done; file will be closed by destructor.
    }   

    return 1;
}



MovieDef* LoaderImpl::CreateMovie(const char* pfilename, unsigned loadConstants, UPInt memoryArena)
{
    // LOCK

    // Capture loading states/variables used during loading.
    Ptr<LoadStates>  pls = *new LoadStates(this);
    // If CreateMovie is started on a thread (not main thread) we need to set
    // threaded loading flag regardless of whether task manage is set or not
    if (loadConstants & Loader::LoadOnThread)
        pls->ThreadedLoading = true;

    // UNLOCK

    if (!pls->GetLib())
    {
        SF_DEBUG_WARNING(1, "Loader::CreateMovie failed, ResourceLibrary does not exist");
        return 0;
    }

    URLBuilder::LocationInfo loc(URLBuilder::File_Regular, pfilename);
    return CreateMovie_LoadState(pls, loc, loadConstants, NULL, memoryArena);
}


void LoaderImpl::RegisterLoadProcess(LoaderTask* ptask)
{

    //SF_ASSERT(pStateBag->GetTaskManager());
    Lock::Locker guard(&LoadProcessesLock);
    LoadProcesses.PushBack(new LoadProcessNode(ptask));
}

void LoaderImpl::UnRegisterLoadProcess(LoaderTask* ptask)
{
    //SF_ASSERT(pStateBag->GetTaskManager());
    Lock::Locker guard(&LoadProcessesLock);
    LoadProcessNode* pnode = LoadProcesses.GetFirst();
    while (!LoadProcesses.IsNull(pnode))
    {
        if (pnode->pTask == ptask) 
        {
            LoadProcesses.Remove(pnode);
            //pStateBag->GetTaskManager()->AbandonTask(ptask);
            delete pnode;
            break;
        }
        pnode = pnode->pNext;
    }
}
void LoaderImpl::CancelLoading()
{
    //printf("LoaderTask::~CancelLoading ---  : %x, thread : %d\n", this, GetCurrentThreadId());
    Ptr<TaskManager> ptm = pStateBag->GetTaskManager();
    if (!ptm)
        return;
    Lock::Locker guard(&LoadProcessesLock);
    LoadProcessNode* pnode = LoadProcesses.GetFirst();
    while (!LoadProcesses.IsNull(pnode))
    {
        LoadProcesses.Remove(pnode);
        ptm->AbandonTask(pnode->pTask);
        delete pnode;
        pnode = LoadProcesses.GetFirst();
    }
}

// *** Loading tasks.

// Loading of image into MovieDataDef.
class MovieImageLoadTask : public LoaderTask
{
    // TODO: Replace this with equivalent of a weak pointer,
    // same as in GFxMovieDataLoadTask above.
    Ptr<MovieDataDef>       pDef;
    Ptr<MovieDefImpl>       pDefImpl;

    // Data required for image loading.    
    Ptr<File>               pImageFile;
    Loader::FileFormatType  ImageFormat;

    //Ptr<LoadStates>         pLoadStates;
    //Ptr<ImageCreator>       pImageCreator;
    //Ptr<GFxRenderConfig>       pRenderConfig;

    // This stores the result of loading: Image Resource.
    Ptr<ImageResource>      pImageRes;

public:
    MovieImageLoadTask(
        MovieDataDef *pdef, MovieDefImpl *pdefImpl,
        File *pin, Loader::FileFormatType format, LoadStates* pls)
        : LoaderTask(pls, Id_MovieImageLoad),
        pDef(pdef), pDefImpl(pdefImpl), pImageFile(pin), ImageFormat(format)//, pLoadStates(pls)
    { }

    virtual void    Execute()
    {
        // Image data loading: could be separated into a different task later on.
        // For now, read the data.
        Ptr<Render::ImageSource>           pimage;

        // MA: Using resource lib's heap seems ok here, although the image is not
        // shared directly through ResourceLib. Instead, the image is shared indirectly
        // based in its contained MovieDataDef. This will work; however, is not ideal
        // because it means that if you both call 'loadMovie' on an image from ActionScript
        // AND have it loaded based on 'gxexport' extraction, you will get two copies in
        // memory. TBD: Perhaps we could use a MovideDef heap here... or improve image
        // sharing so that it is done for the *image itself* in the resource lib.
        MemoryHeap*           pimageHeap = pLoadStates->pWeakResourceLib->GetImageHeap();

        pimage = *LoaderImpl::LoadBuiltinImage(pImageFile, ImageFormat,
                        Resource::Use_Bitmap, 
                        pLoadStates,
                        pLoadStates->GetLog(), 
                        pimageHeap);

        if (pimage)
        {
            // Use creator for image
//             ImageCreateInfo ico(pimage, Resource::Use_None, pLoadStates->GetRenderConfig());
//             ico.ThreadedLoading = pLoadStates->IsThreadedLoading();
//             ico.pHeap           = pimageHeap;
//@IMG            pimageBase = *pLoadStates->GetBindStates()->pImageCreator->CreateImage(ico);

//             if (pimageBase)            
            pImageRes = 
                *SF_HEAP_NEW(pimageHeap) ImageResource(pimage, Resource::Use_Bitmap);
        }

        if (pImageRes)
        {
            pDef->InitImageFileMovieDef(pImageFile->GetLength(), pImageRes, pDefImpl->GetImageCreator());

            // Notify MovieDefImpl that binding is finished,
            // so that any threaded waiters can be released.
            unsigned fileBytes = pDef->GetFileBytes();
            pDefImpl->pBindData->UpdateBindingFrame(pDef->GetLoadingFrame(), fileBytes);
            pDefImpl->pBindData->SetBindState(
                MovieDefImpl::BS_Finished |
                MovieDefImpl::BSF_Frame1Loaded | MovieDefImpl::BSF_LastFrameLoaded);
        }
        else
        {   // Error            
            pDefImpl->pBindData->SetBindState(MovieDefImpl::BS_Error);
        }
    }

    virtual void    OnAbandon(bool)
    {        
        // TODO: Mark movie as canceled, so that it knows that
        // it will not proceed here.
    }

    bool    LoadingSucceeded() const   { return pImageRes.GetPtr() != 0; }
};  





// Static: The actual creation function; called from CreateMovie.
MovieDefImpl* LoaderImpl::CreateMovie_LoadState(LoadStates* pls,
                                                const URLBuilder::LocationInfo& loc,
                                                unsigned loadConstants, LoadStackItem* ploadStack,
                                                UPInt memoryArena)
{
    // Translate the filename.
    String fileName;
    pls->BuildURL(&fileName, loc);


    // *** Check Library and Initiate Loading

    ResourceLib::BindHandle     bh;    
    Ptr<MovieDataDef>           pmd;
    MovieDefImpl*               pm = 0;
    LogState*                   plog  = pls->pLog;

    bool                        movieNeedsLoading = 0;    
    MovieDataDef::MovieDataType mtype  = MovieDataDef::MT_Empty;
    Ptr<MovieBindProcess>       pbp;
    Ptr<LoadProcess>            plp;
    Ptr<File>                   pin;
    FileFormatType              format = Loader::File_Unopened;

    ImagePackParamsBase* pimagePacker = pls->GetBindStates()->pImagePackParams;
    if (pimagePacker)
        loadConstants |= Loader::LoadOrdered|Loader::LoadWaitCompletion;

    // Ordered loading means that all binding-dependent files (imports and images) will be
    // loaded after the main file. Technically, this disagrees with progressive loading,
    // although we could devise a better scheme in the future.
    // If 'Ordered' is not specified, loading is interleaved, meaning that imports
    // and dependencies get resolved while parent file hasn't yet finished loading.
    // ThreadedBinding implies interleaved loading, since the binding thread can
    // issue a dependency load request at any time (Removed for simplicity).
    bool    interleavedLoading = !(loadConstants & Loader::LoadOrdered);

    // We integrate optional ImageCreator for loading, with hash matching
    // dependent on ImageLoader::IsKeepingImageData and LoadKeepBindData flag.
    // Image creator is only used as a key if it is bound to, based on flags.    
    ImageCreator*   pkeyImageCreator = pls->GetLoadTimeImageCreator(loadConstants);
    FileOpener*     pfileOpener = pls->GetFileOpener();
    UInt64          modifyTime  = pfileOpener ?
                                  pfileOpener->GetFileModifyTime(fileName.ToCStr()) : 0;

    ResourceKey fileDataKey = 
        MovieDataDef::CreateMovieFileKey(fileName.ToCStr(), modifyTime,
                                         pfileOpener, pkeyImageCreator, pls->GetPreprocessParams());

    ResourceLib::ResolveState rs;

    if ((rs = pls->GetLib()->BindResourceKey(&bh, fileDataKey)) ==
        ResourceLib::RS_NeedsResolve)
    {        
        // Open the file; this will automatically do the logging on error.
        pin = *pls->OpenFile(fileName.ToCStr(), loadConstants);
        if (!pin) 
        {
            // TBD: Shouldn't we transfer this OpenFile's error message to our 
            // waiters, if any? That way both threads would report the same message.
            // For now, just create a string.
            String s("Loader failed to open \"", fileName.ToCStr(), "\"\n");
            bh.CancelResolve(s.ToCStr());
            return 0;
        }


        // Detect file format so that we can determine whether we can
        // and/or allowed to support it. Images can be loaded directly
        // into MovieDef files, but their loading logic is custom.        
        format = DetectFileFormat(pin);

        switch(format)
        {      
        case Loader::File_SWF:
            if (loadConstants & Loader::LoadDisableSWF)
            {                
                String s("Failed loading SWF file \"", fileName.ToCStr(),
                          "\" - GFX file format expected");
                if (plog)
                    plog->LogError("%s", s.ToCStr());
                bh.CancelResolve(s.ToCStr());
                return 0;
            }
            // Fall through to Flash file loading
        case Loader::File_GFX:
            mtype = MovieDataDef::MT_Flash;
            break;

            // Image file formats support.
        case Loader::File_JPEG:
        case Loader::File_DDS:
        case Loader::File_TGA:
        case Loader::File_PNG:
            // If image file format loading is enabled proceed to do so.
            if (loadConstants & Loader::LoadImageFiles)
            {
                mtype = MovieDataDef::MT_Image;
                break;
            }

        case Loader::File_Unopened:
            // Unopened should not occur due to the check above.
        case Loader::File_Unknown:
        default:
            {            
                String s("Unknown file format at URL \"",
                          fileName.ToCStr(), "\"");
                if (plog)
                    plog->LogError("%s", s.ToCStr());
                bh.CancelResolve(s.ToCStr());
            }            
            return 0;
        }


        // Create MovieDataDef of appropriate type (Image or Flash)
        pmd = *MovieDataDef::Create(fileDataKey, mtype, fileName.ToCStr(), 0,
                                       (loadConstants & Loader::LoadDebugHeap) ? true : false, 
                                       memoryArena);

        //printf("Thr %4d, %8x : CreateMovie - constructed MovieDataDef for '%s'\n", 
        //       GetCurrentThreadId(), pmd.GetPtr(), fileName.ToCStr());

        if (pmd)
        {
            // Assign movieDef's file path to LoadStates.
            pls->SetRelativePathForDataDef(pmd);

            // Create a loading process for Flash files and verify header.
            // For images, this is done later on.
            if (mtype == MovieDataDef::MT_Flash)
            {  
                plp = *SF_NEW LoadProcess(pmd, pls, loadConstants);

                // Read in and verify header, initializing loading.
                // Note that this also reads the export tags,
                // so no extra pre-loading will be necessary in MovieDef.
                if (!plp || !plp->BeginSWFLoading(pin))
                {
                    // Clear pmd, causing an error message and CancelResolve below.
                    plp = 0;
                    pmd = 0;
                }
            }
        }

        if (pmd)
        {    
            // For images we always create DefImpl before ResolveResource, so that
            // other threads don't try to bind us (no separate binding from images now).

            if  ((mtype != MovieDataDef::MT_Flash) || interleavedLoading )
            {
                // If we are doing interleaved loading, create the bound movie entry immediately,
                // to ensure that we don't have another thread start binding before us.     
                pm = CreateMovieDefImpl(pls, pmd, loadConstants,
                    (mtype == MovieDataDef::MT_Flash) ? &pbp.GetRawRef() : 0, true, ploadStack, memoryArena);
            }

            bh.ResolveResource(pmd.GetPtr());
        }
        else
        {
            String s("Failed to load SWF file \"", fileName.ToCStr(), "\"\n");
            bh.CancelResolve(s.ToCStr());
            return 0;
        }

        movieNeedsLoading = 1;
    }
    else
    {
        // If Available and Waiting resources will be resolved here.        
        /*
        if (rs == ResourceLib::RS_Available)        
        printf("Thr %4d, ________ : CreateMovie - '%s' is in library\n", GetCurrentThreadId(), fileName.ToCStr());        
        else        
        printf("Thr %4d, ________ : CreateMovie - waiting on '%s'\n", GetCurrentThreadId(), fileName.ToCStr());        
        */
        SF_UNUSED(rs);

        if ((pmd = *(MovieDataDef*)bh.WaitForResolve()).GetPtr() == 0)
        {
            // Error occurred during loading.
            if (plog)
                plog->LogError("%s", bh.GetResolveError());
            return 0;
        }

        mtype = pmd->MovieType;

        // SetDataDef to load states so that MovieDefImpl::Bind can proceed.
        pls->SetRelativePathForDataDef(pmd);
        // May need to wait for movieDefData to become available.
    }


    // *** Check the library for MovieDefImpl and Initiate Binding    

    // Do a check because for Ordered loading this might have been
    // done above to avoid data race.
    if (!movieNeedsLoading || !interleavedLoading)
    {
        if (!pm)
        {            
            // For images this can grab an existing MovieDefImpl, but it will never
            // create one since it's taken care of before DataDef ResolveResource.

            pm = CreateMovieDefImpl(pls, pmd, loadConstants,
                (mtype == MovieDataDef::MT_Flash) ? &pbp.GetRawRef() : 0, false, ploadStack, memoryArena);
        }
    }
    if (!pm)
        return 0;


    // *** Do Loading

    if (movieNeedsLoading)
    {
        if (mtype == MovieDataDef::MT_Flash)
        { 
            // Set 'ploadBind' if we are going to do interleaved binding
            // simultaneously with loading. LoadOrdered means that binding
            // will be done separately - in that case Read is don with no binding.
            MovieBindProcess* ploadBind =
                (loadConstants & Loader::LoadOrdered) ? 0 : pbp.GetPtr();
            if (ploadBind)
                plp->SetBindProcess(ploadBind);

            // bind process will allocate the temporary data if necessary
            if (pbp) // used for packer; incompatible with packer if binding is occurring on diff thread (AR)
                plp->SetTempBindData(pbp->GetTempBindData());

            // If we have task manager, queue up loading task for execution,
            // otherwise just run it immediately.
            if (loadConstants & Loader::LoadWaitCompletion || !pls->SubmitBackgroundTask(plp))
                plp->Execute();

            if (ploadBind)
            {
                // If bind process was performed as part of the load task,
                // we no longer need it.
                pbp = 0;
            }

            plp = 0;
            pin = 0;
        }
        else
        {
            Ptr<MovieImageLoadTask> ptask = 
                *SF_NEW MovieImageLoadTask(pmd, pm, pin, format, pls);

            if ((loadConstants & (Loader::LoadWaitCompletion|Loader::LoadOrdered)) 
                || !pls->SubmitBackgroundTask(ptask) )
            {
                ptask->Execute();

                if (!ptask->LoadingSucceeded())
                {
                    if (pm) pm->Release();
                    return 0;
                }

                // NOTE: A similar check is done by the use of 'waitSuceeded'
                // flag below for threaded tasks.
            }
        }
    }


    // Run bind task on a MovieDefImpl and waits for completion, based on flags. 
    return BindMovieAndWait(pm, pbp, pls, loadConstants, ploadStack);
}



MovieDefImpl* LoaderImpl::BindMovieAndWait(MovieDefImpl* pm, MovieBindProcess* pbp,
                                           LoadStates* pls, unsigned loadConstants, LoadStackItem* ploadStack)
{
    // It we still need binding, perform it.
    if (pbp)
    {   
        if (loadConstants & Loader::LoadWaitCompletion || !pls->SubmitBackgroundTask(pbp))
            pbp->Execute();
    }

    // Note that if loading failed in the middle we may return a partially loaded object.
    // This is normal because (a) loading can technically take place in a different thread
    // so it is not yet known if it will finish successfully and (2) Flash can actually
    // play unfinished files, even if the error-ed in a middle.

    // The exception to above are wait flags, however.
    bool waitSuceeded = true;
    bool needWait = true;

    // Checking for recursion in the loading process.
    LoadStackItem* pstack = ploadStack;
    while(pstack)
    {
        if (pstack->pDefImpl == pm)
        {
            // Recursion is detected. 
            // Check if this is a self recursion 
            if(pstack->pNext)
            {
                // This is not a self recursion. We don't support this recursion type yet.
                // Stop loading and return error.
                waitSuceeded = false;
                if (pls->GetLogState())
                {
                    StringBuffer buffer;

                    while(ploadStack)
                    {
                        buffer += ploadStack->pDefImpl->GetFileURL();
                        buffer += '\n';
                        ploadStack = ploadStack->pNext;
                    }

                    buffer += pm->GetFileURL();                    
                    pls->GetLogState()->LogError("Recursive import detected. Import stack:\n%s", buffer.ToCStr());
                }
            }
            // We must not wait on a waitcondition which will never be set.
            needWait = false;
            break;
        }
        pstack = pstack->pNext;
    }
    if (needWait && (loadConstants & Loader::LoadWaitCompletion))
    {
        // TBD: Under threaded situation the semantic of LoadWaitCompletion might
        // actually be to do loading on 'this' thread without actually queuing
        // a task.

        // We might also want to have a flag that would control whether WaitCompletion
        // fails the load on partial load, or returns an object partially loaded
        // similar to the standard behavior above.
        waitSuceeded = pm->WaitForBindStateFlags(MovieDefImpl::BSF_LastFrameLoaded);
    }
    else if (needWait && (loadConstants & Loader::LoadWaitFrame1))
    {
        waitSuceeded = pm->WaitForBindStateFlags(MovieDefImpl::BSF_Frame1Loaded);
    }

    // waitSuceeded would only be 'false' in case of error.
    if (!waitSuceeded)
    {
        pm->Release();
        pm = 0;
    }
    return pm;
}


// Looks up or registers MovieDefImpl, separated so that both versions 
// of loading can share implementation. Fills is pbindProcess pointer if
// the later is provided (not necessary for image file MovieDefImpl objects).
MovieDefImpl* LoaderImpl::CreateMovieDefImpl(LoadStates* pls,
                                             MovieDataDef* pmd,
                                             unsigned loadConstants,
                                             MovieBindProcess** ppbindProcess,
                                             bool checkCreate, 
                                             LoadStackItem* ploadStack,
                                             UPInt memoryArena)
{        
    ResourceLib::BindHandle  bh;    
    MovieDefImpl*            pm = 0;

    // Create an Impl key and see if it can be resolved.
    MovieDefBindStates*      pbindStates   = pls->GetBindStates();
    ResourceKey              movieImplKey  = MovieDefImpl::CreateMovieKey(pmd, pbindStates);
    Ptr<MovieBindProcess>    pbp;
    ResourceLib::ResolveState rs;

    if ((rs = pls->GetLib()->BindResourceKey(&bh, movieImplKey)) ==
        ResourceLib::RS_NeedsResolve)
    {
        // Create a new MovieDefImpl
        // We pass GetStateBagImpl() from loader so that it is used for delegation 
        // when accessing non-binding states such as log and renderer.
        pm = SF_NEW MovieDefImpl(pmd, pbindStates, pls->pLoaderImpl, loadConstants,
                                 pls->pLoaderImpl->pStateBag, Memory::pGlobalHeap, 0, memoryArena);

        //printf("Thr %4d, %8x :  CreateMovieDefImpl - MovieDefImpl constructed for %8x\n",
        //       GetCurrentThreadId(), pm, pmd);

        if (ppbindProcess)
        {
            // Only create bind process for Flash movies, not images.
            *ppbindProcess = SF_NEW MovieBindProcess(pls, pm, ploadStack);
            if (!*ppbindProcess && pm)
            {
                pm->Release();
                pm = 0;
            }
        }

        // Need to read header first.
        if (pm)
            bh.ResolveResource(pm);
        else
        {
            String s("Failed to bind SWF file \"", pmd->GetFileURL(), "\"\n");
            bh.CancelResolve(s.ToCStr());
            return 0;
        }

    }
    else
    {
        SF_ASSERT(!checkCreate);
        SF_UNUSED(checkCreate);

        /*
        if (rs == ResourceLib::RS_Available)
        {
        printf("Thr %4d, ________ :  CreateMovieDefImpl - Impl for %8x is in library\n",
        GetCurrentThreadId(), pmd);
        }
        else
        {
        printf("Thr %4d, ________ :  CreateMovieDefImpl - waiting MovieDefImpl for %8x\n",
        GetCurrentThreadId(), pmd);
        }
        */
        SF_UNUSED(rs);


        // If Available and Waiting resources will be resolved here.
        // Note: Returned value is AddRefed for us, so we don't need to do so.
        if ((pm = (MovieDefImpl*)bh.WaitForResolve()) == 0)
        {
            // Error occurred during loading.
            if (pls->pLog)
                pls->pLog->LogError("%s", bh.GetResolveError());
            return 0;
        }
    }

    return pm;
}


// Loading version used for look up / bind MovieDataDef based on provided states.
// Used to look up movies serving fonts from GFxFontProviderSWF.
MovieDefImpl*  LoaderImpl::CreateMovie_LoadState(LoadStates* pls,
                                                 MovieDataDef* pmd,
                                                 unsigned loadConstants,
                                                 UPInt memoryArena)
{
    if (pmd)
        pls->SetRelativePathForDataDef(pmd);

    ResourceLib::BindHandle      bh;
    Ptr<MovieBindProcess>       pbp;
    MovieDefImpl*                pm = CreateMovieDefImpl(pls, pmd, loadConstants,
        &pbp.GetRawRef(), false, NULL, memoryArena);

    if (!pm) return 0;

    // It we need binding, perform it.
    return BindMovieAndWait(pm, pbp, pls, loadConstants);
}


// Loads a Render::Image from an open file, assuming the specified file format.
Render::ImageSource* LoaderImpl::LoadBuiltinImage(File* pfile,
                                                  FileTypeConstants::FileFormatType format,
                                                  Resource::ResourceUse,
                                                  LoadStates* pls,
                                                  Log* plog,
                                                  MemoryHeap* pimageHeap)
{
    // Open the file and create image.
    Render::ImageSource* pimageSrc = 0;
    const char*    pfilePath = pfile->GetFilePath();
    Render::ImageFileFormat imageFmt;

    switch(format)
    {
    case Loader::File_TGA:
        imageFmt = Render::ImageFile_TGA;
        break;

    case Loader::File_DDS:
        imageFmt = Render::ImageFile_DDS;
        break;

    case Loader::File_JPEG:
        imageFmt = Render::ImageFile_JPEG;
        break;

    case Loader::File_PNG:
        imageFmt = Render::ImageFile_PNG;
        break;

    default:
        // Unknown format!
        if (plog)
            plog->LogMessage("Default image loader failed to load '%s'", pfilePath);
        else 
        {
            SF_DEBUG_WARNING1(1, "Default image loader failed to load '%s'",
                pfilePath);
        }
        return NULL;
    }

    Ptr<ImageFileHandlerRegistry> imageFileReg = pls->GetImageFileHandlerRegistry();
    if (!imageFileReg)
    {
        if (plog)
            plog->LogError("Image file handler registry is not installed - can't load image data");
    }
    else
    {
        Render::ImageFileHandler* imgHandler = imageFileReg->GetReader(imageFmt);
        if (!imgHandler)
        {
            if (plog)
                plog->LogError("Can't load image %s - appropriate reader is not installed.", pfilePath);
        }
        else
        {
            Render::ImageFileReader* imgReader = static_cast<Render::ImageFileReader*>(imgHandler);
            Render::ImageCreateArgs args;
            args.pHeap = pimageHeap;
            //args.Use = info.Use;
            //args.pManager = pTextureManager; //?
            pimageSrc = imgReader->ReadImageSource(pfile, args);

        }
    }
    return pimageSrc;
}


// *** File format detection logic.

Loader::FileFormatType LoaderImpl::DetectFileFormat(File *pfile)
{   
    if (!pfile)
        return Loader::File_Unopened;

    int             pos     = pfile->Tell();
    FileFormatType  format  = Loader::File_Unknown;
    UByte           buffer[4] = {0,0,0,0};

    if (pfile->Read(buffer, 4) <= 0)
        return Loader::File_Unknown;

    switch(buffer[0])
    {
    case 0x43:
    case 0x46:    
        if ((buffer[1] == 0x57) && (buffer[2] == 0x53))
            format = Loader::File_SWF;
        else if ((buffer[1] == 0x46) && (buffer[2] == 0x58))
            format = Loader::File_GFX;
        break;    

    case 0xFF:
        if (buffer[1] == 0xD8)
            format = Loader::File_JPEG;
        break;

    case 0x89:
        if ((buffer[1] == 'P') && (buffer[2] == 'N') && (buffer[3] == 'G'))
            format = Loader::File_PNG;
        break;

    case 'G':
        if ((buffer[1] == 'I') && (buffer[2] == 'F') && (buffer[3] == '8'))
            format = Loader::File_GIF;
        // 'GFX' also starts with a G.
        if ((buffer[1] == 0x46) && (buffer[2] == 0x58))
            format = Loader::File_GFX;
        break;
    case 'D': // check is it DDS
        if ((buffer[1] == 'D') && (buffer[2] == 'S'))
            format = Loader::File_DDS;
        break;
    }

    pfile->Seek(pos);
    if (format == Loader::File_Unknown)
    {
        // check for extension. TGA format is hard to detect, that is why
        // we use extension test.
        const char* ppath = pfile->GetFilePath();
        if (ppath)
        {
            // look for the last '.'
            const char* pstr = strrchr(ppath, '.');
            if (pstr && String::CompareNoCase(pstr, ".tga") == 0)
                format = Loader::File_TGA;
        }
    }
    return format;
}




// ImageResource* LoaderImpl::LoadMovieImage(const char *purl,
//                                           ImageLoader *ploader, Log *plog,
//                                           MemoryHeap* pheap)
// {
//     // LoadMovieImage function is only used to load images with 'img://' prefix.
//     // This means that we don't cache it in the resourceLib and instead just
//     // pass it along to the user (allowing them to refresh the data if necessary).
// 
// //@IMG    Ptr<ImageInfoBase> pimage;
// //     if (ploader)
// //         pimage = *ploader->LoadImage(purl);
// // 
// //     if (!pimage)
// //     {
// //         if (plog)
// //             plog->LogScriptWarning(
// //             "Could not load user image \"%s\" - ImageLoader failed or not specified\n", purl);
// //         pimage = *CreateStaticUserImage();
// //     }
// // 
// //     // With respect to image keys, we just use a unique key here.
// //     return pimage? SF_HEAP_NEW(pheap) ImageResource(pimage) : 0;
//     return 0;
// }

// Create a filler image that will be displayed in place of loadMovie() user images.
//@IMG
// ImageInfoBase* LoaderImpl::CreateStaticUserImage()
// {
//     enum {
//         StaticImgWidth      = 19,
//         StaticImgHeight     = 12,
//         StaticImageScale    = 3,
//     };
// 
//     // Encodes 'img:' picture with color palette int the back.  
//     static char pstaticImage[StaticImgWidth * StaticImgHeight + 1] =
//         "aaaaaaaaaaaaaaaaaaa"
//         "aaaaaaaaaaaaaaaaaaa"
//         "rr#rrrrrrrrrrrrrrrr"
//         "rrrrrrrrrrrrrrrr##r" 
//         "g##gg##g#ggg###g##g"
//         "gg#gg#g#g#g#gg#gggg"
//         "bb#bb#b#b#b#bb#b##b"
//         "bb#bb#b#b#bb###b##b"
//         "y###y#y#y#yyyy#yyyy"
//         "yyyyyyyyyyy###yyyyy"
//         "ccccccccccccccccccc"
//         "ccccccccccccccccccc"
//         ;
// 
//     Ptr<Render::Image> pimage = *new Render::Image(Render::Image::Image_ARGB_8888,
//         StaticImgWidth * StaticImageScale,
//         StaticImgHeight * StaticImageScale);
//     if (pimage)
//     {
//         for (int y=0; y<StaticImgHeight; y++)
//             for (int x=0; x<StaticImgWidth; x++)
//             {
//                 UInt32 color = 0;
// 
//                 switch(pstaticImage[y * StaticImgWidth + x])
//                 {
//                 case '#': color = 0xFF000000; break;
//                 case 'r': color = 0x80FF2020; break;
//                 case 'g': color = 0x8020FF20; break;
//                 case 'b': color = 0x802020FF; break;
//                 case 'y': color = 0x80FFFF00; break;
//                 case 'a': color = 0x80FF00FF; break;
//                 case 'c': color = 0x8000FFFF; break;
//                 }
// 
//                 for (int iy = 0; iy < StaticImageScale; iy++)
//                     for (int ix = 0; ix < StaticImageScale; ix++)
//                     {
//                         pimage->SetPixelRGBA(x * StaticImageScale + ix,
//                             y * StaticImageScale + iy, color);
//                     }
//             }
//     }
// 
//     // Use ImageInfo directly without callback, since this is an alternative
//     // to pImageLoadFunc, which is not the same as pImageCreateFunc.    
//     return new ImageInfo(pimage);
// }

//////////////////////////////////////////////////////////////////////////
// Default implementation of Asian (Japanese, Korean, Chinese) word-wrapping
//
bool Translator::OnWordWrapping(LineFormatDesc* pdesc)
{
    if (WWMode == WWT_Default)
        return false;
    if ((WWMode & (WWT_Asian | WWT_NoHangulWrap | WWT_Prohibition)) && pdesc->NumCharsInLine > 0)
    {
        UPInt wordWrapPos = WordWrapHelper::FindWordWrapPos
            (WWMode, 
            pdesc->ProposedWordWrapPoint, 
            pdesc->pParaText, pdesc->ParaTextLen, 
            pdesc->LineStartPos, 
            pdesc->NumCharsInLine);
        if (wordWrapPos != SF_MAX_UPINT)
        {
        pdesc->ProposedWordWrapPoint = wordWrapPos;
        return true;
    }
        return false;
    }
    else if ((WWMode & WWT_Hyphenation))
    {
        if (pdesc->ProposedWordWrapPoint == 0)
            return false;
        const wchar_t* pstr = pdesc->pParaText + pdesc->LineStartPos;
        // determine if we need hyphenation or not. For simplicity,
        // we just will put dash only after vowels.
        UPInt hyphenPos = pdesc->NumCharsInLine;
        // check if the proposed word wrapping position is at the space.
        // if so, this will be the ending point in hyphenation position search.
        // Otherwise, will look for the position till the beginning of the line.
        // If we couldn't find appropriate position - just leave the proposed word
        // wrap point unmodified.
        UPInt endingHyphenPos = (SFiswspace(pstr[pdesc->ProposedWordWrapPoint - 1])) ? 
            pdesc->ProposedWordWrapPoint : 0;
        for (; hyphenPos > endingHyphenPos; --hyphenPos)
        {
            if (WordWrapHelper::IsVowel(pstr[hyphenPos - 1]))
            {
                // check if we have enough space for putting dash symbol
                // we need to summarize all widths up to hyphenPos + pdesc->DashSymbolWidth
                // and this should be less than view rect width
                float lineW = pdesc->pWidths[hyphenPos - 1];
                lineW += pdesc->DashSymbolWidth;
                if (lineW < pdesc->VisibleRectWidth)
                {
                    // ok, looks like we can do hyphenation
                    pdesc->ProposedWordWrapPoint = hyphenPos;
                    pdesc->UseHyphenation = true;
                    return true;
                }
                else
                {
                    // oops, we have no space for hyphenation mark
                    continue;
                }
                break;
            }
        }
    }
    return false;
}

void Translator::TranslateInfo::SetResult(const wchar_t* presultText, UPInt resultLen)
{
    SF_ASSERT(pResult);
    if (!presultText)
        return;

    if (resultLen == SF_MAX_UPINT)
        resultLen = SFwcslen(presultText);
    pResult->Resize(resultLen + 1);
    SFwcsncpy(pResult->GetBuffer(), resultLen + 1, presultText, resultLen);

    Flags |= TranslateInfo::Flag_Translated;
}

void Translator::TranslateInfo::SetResult(const char* presultTextUTF8, UPInt resultUTF8Len)
{
    SF_ASSERT(pResult);
    if (!presultTextUTF8)
        return;

    if (resultUTF8Len == SF_MAX_UPINT)
        resultUTF8Len = SFstrlen(presultTextUTF8);

    int nchars = (int)UTF8Util::GetLength(presultTextUTF8);
    pResult->Resize(nchars + 1);
    UTF8Util::DecodeString(pResult->GetBuffer(), presultTextUTF8, resultUTF8Len);

    Flags |= TranslateInfo::Flag_Translated;
}

}} // Scaleform::GFx

