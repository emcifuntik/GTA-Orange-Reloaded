/**********************************************************************

Filename    :   D3D9_Texture.h
Content     :   D3D9 Texture and TextureManager header
Created     :   January 2010
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_D3D9_Texture_H
#define INC_SF_Render_D3D9_Texture_H

#include "Kernel/SF_List.h"
#include "Kernel/SF_Threads.h"
#include "Render/Render_Image.h"
#include "Kernel/SF_HeapNew.h"
#include <d3d9.h>

namespace Scaleform { namespace Render { namespace D3D9 {


// TextureFormat describes format of the texture and its caps.
// Format includes allowed usage capabilities and ImageFormat
// from which texture is supposed to be initialized.

struct TextureFormat
{
    struct Mapping
    {
        ImageFormat              Format;
        D3DFORMAT                D3DFormat;
        UByte                    BytesPerPixel;
        bool                     Mappable;
        Image::CopyScanlineFunc  CopyFunc;
        Image::CopyScanlineFunc  UncopyFunc;
    };

    const Mapping*  pMapping;
    DWORD           D3DUsage;

    ImageFormat     GetImageFormat() const { return pMapping->Format; }
    D3DFORMAT       GetD3DFormat() const   { return pMapping->D3DFormat; }

    unsigned        GetPlaneCount() const
    { return ImageData::GetFormatPlaneCount(GetImageFormat()); }
    
    bool            CanAutoGenMipmaps() const
    { return (D3DUsage & D3DUSAGE_AUTOGENMIPMAP) != 0; }
    bool            CanBeDynamic() const
    { return (D3DUsage & D3DUSAGE_DYNAMIC) != 0; }
};

class MappedTexture;
class TextureManager;


// TextureManagerLocks contains TextureMutex and other synchronization objects.
// These are separated into a separate object to allow proper synchronization
// when Texture and TextureManager objects die out of order on different threads.

class TextureManagerLocks : public RefCountBase<TextureManagerLocks, StatRender_TextureManager_Mem>
{
public:
    TextureManager* pManager;

    // Lock obtained when pImage within any texture is used/cleared.
    Lock            ImageLock;
    // Mutex obtained when TextureManager::Textures/InitTextureQueue is modified.
    Mutex           TextureMutex;
    // Notified when TextureInitQueue is processed.
    WaitCondition   TextureInitWC;  

    TextureManagerLocks(TextureManager* pmanager) : pManager(pmanager) { }
    ~TextureManagerLocks() { }
};


// D3D9 Texture class implementation; it man actually include several HW 
// textures (one for each ImageFormat plane).

class Texture : public Render::Texture, public ListNode<Texture>
{
public:
    enum AllocType
    {
        Type_Normal, // Not used yet.
        Type_Managed,
        Type_Dynamic,
    };
    // Describes the lifetime state that the texture is in.
    enum CreateState
    {
        State_InitPending,
        State_InitFailed,
        State_Valid,
        State_Lost,
        State_Dead  // Texture becomes Dead once Manager is destroyed.
    };

    // Bits stored in TextureFlags.
    enum TextureFlagBits
    {
        TF_Rescale    = 0x01,
        TF_SWMipGen   = 0x02,
    };

    Ptr<TextureManagerLocks> pManagerLocks;
    ImageBase*              pImage;
    
    const TextureFormat*    pFormat;
    ImageSize               ImgSize;
    
    UByte                   MipLevels;
    UByte                   TextureCount;
    UInt16                  Use;
    UByte                   TextureFlags;
    AllocType               Type;
    CreateState             State;

    // If texture is currently mapped, it is here.
    MappedTexture*          pMap;
    
    struct HWTextureDesc
    {        
        ImageSize           Size;
        IDirect3DTexture9*  pTexture;
    };

    // TextureDesc array is allocated if more then one is needed.
    HWTextureDesc*          pTextures;
    HWTextureDesc           Texture0;

    Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat, unsigned mipLevels,
            const ImageSize& size, unsigned use, unsigned arena, ImageBase* pimage);
    ~Texture();

    ImageFormat             GetImageFormat() const { return pFormat->pMapping->Format; }
    TextureManager*         GetManager() const     { return pManagerLocks->pManager; }
    bool                    IsValid() const        { return pTextures != 0; }

    void                    LoseManager();    
    bool                    Initialize();
    void                    ReleaseHWTextures();

    // Applies a texture to device starting at pstageIndex, advances index
    // TBD: Texture matrix may need to be adjusted if image scaling is done.
    void                    ApplyTexture(DWORD stageIndex, ImageFillMode fm);
    inline DWORD            GetTextureStageCount() const { return TextureCount; }

    // *** Interface implementation

//    virtual HAL*      GetRenderHAL() const { SF_ASSERT(0); return 0; } // TBD

    virtual Render::TextureManager* GetTextureManager() const;
    virtual Image*          GetImage() const
    { SF_ASSERT(!pImage || (pImage->GetImageType() != Image::Type_ImageBase)); return (Image*)pImage; }
    virtual ImageFormat     GetFormat() const         { return GetImageFormat(); }
    virtual ImageSize       GetSize() const           { return ImgSize; }
    virtual unsigned        GetMipmapCount() const    { return (Use & ImageUse_GenMipmaps) ? 1 : MipLevels; }

    virtual void            GetUVGenMatrix(Matrix2F* mat) const;
    
    // Obtains lock & stop using the image
    virtual void            ImageLost();
    
    virtual bool            Map(ImageData* pdata, unsigned mipLevel, unsigned levelCount);
    virtual bool            Unmap();

    virtual bool            Update(const UpdateDesc* updates, unsigned count = 1, unsigned mipLevel = 0);    
    virtual bool            Update();

    // Copies the image data from the hardware.
    SF_AMP_CODE( virtual bool Copy(ImageData* pdata); )
};


// MappedTexture object repents a Texture mapped into memory with Texture::Map() call;
// it is also used internally during updates.
// The key part of this class is the Data object, stored Locked texture level plains.

class MappedTexture : public NewOverrideBase<StatRender_TextureManager_Mem>
{
    friend class Texture;

    Texture*      pTexture;
    // We support mapping sub-range of levels, in which case
    // StartMipLevel may be non-zero.
    unsigned      StartMipLevel;
    unsigned      LevelCount;
    // Pointer data that can be copied to.
    ImageData     Data;

    enum { PlaneReserveSize = 4 };
    ImagePlane    Planes[PlaneReserveSize];

public:
    MappedTexture()
        : pTexture(0), StartMipLevel(false), LevelCount(0) { }
    ~MappedTexture()
    {
        SF_ASSERT(!IsMapped());
    }

    bool        IsMapped() { return (LevelCount != 0); }
    bool        Map(Texture* ptexture, unsigned mipLevel, unsigned levelCount);
    void        Unmap();
};


// D3D configuration flags, including both user configuration settings
// and available capabilities.

struct  D3DCapFlags
{
    UInt32 Flags;

    enum 
    {
        Cap_MultiThreaded    = 0x00000001,
        Cap_Managed          = 0x00000002,
        Cap_DynamicTex       = 0x00000004,
        Cap_AutoGenMipmaps   = 0x00000010,
        Cap_NonPow2Tex       = 0x00000040,
        Cap_NonPow2TexWrap   = 0x00000080,
    };

    bool IsMultiThreaded() const    { return (Flags & Cap_MultiThreaded) != 0; }
    bool IsManaged() const          { return (Flags & Cap_Managed) != 0; }
    bool DynamicTexAllowed() const  { return (Flags & Cap_DynamicTex) != 0; }

    bool NonPow2Supported(unsigned use)
    {
        UInt32 bit =(use & ImageUse_Wrap) ? Cap_NonPow2TexWrap : Cap_NonPow2Tex;
        return (Flags & bit) != 0;
    }

    // TBD: Add Multi-Threaded and other options in the future..
    //  We also need to combine HW capabilities with user-specified settings
    void InitFromHWCaps(const D3DCAPS9& caps)
    {
        Flags = 0;
        if (!(caps.TextureCaps & D3DPTEXTURECAPS_POW2))
        {
            Flags |= Cap_NonPow2Tex;
            if (!(caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL))
                Flags |= Cap_NonPow2TexWrap;
        }
        else  if (caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
        {
            Flags |= Cap_NonPow2Tex;
        }
    }
};


// ImageUpdateQueue is a simple array-based queue used to record image
// update requests and execute them all at once. Uses the lower pointer
// bit to determine whether this is a full or partial image update. 

struct ImageUpdateQueue
{
    typedef ArrayConstPolicy<4, 4, false> UpdateShrinkPolicy;
    typedef UPInt                         UpdateRef;

    ArrayLH<UpdateRef, StatRender_TextureManager_Mem, UpdateShrinkPolicy>  Queue;

    void Add(Image* pimage)
    {
        SF_ASSERT(!(((UPInt)pimage)&1));
        Queue.PushBack(((UpdateRef)pimage)|1);
        pimage->AddRef(); 
    }
    void    Add(ImageUpdate* pupdate)
    {
        Queue.PushBack((UpdateRef)pupdate);
        pupdate->AddRef();
    }

    ~ImageUpdateQueue();

    void   ProcessUpdates(Render::TextureManager* pmanager);
};



// D3D9 Texture Manger.
// This class is responsible for creating textures and keeping track of them
// in the list.
// 

class TextureManager : public Render::TextureManager
{
    friend class Texture;

    typedef ArrayConstPolicy<8, 8, false>   KillListArrayPolicy;
    typedef ArrayLH<IDirect3DTexture9*,
                    StatRender_TextureManager_Mem,
                    KillListArrayPolicy>    D3DTextureArray;

    IDirect3DDevice9*   pDevice;
    D3DCapFlags         Caps;
    ThreadId            RenderThreadId;
    MappedTexture       MappedTexture0;

    Ptr<TextureManagerLocks> pLocks;
    // Lists protected by TextureManagerLocks::TextureMutex.
    List<Texture>            Textures;
    List<Texture>            TextureInitQueue;
    D3DTextureArray          D3DTextureKillList;
    ImageUpdateQueue         ImageUpdates;
    
    // Texture format table, organized by supported HW features.
    ArrayLH<TextureFormat>   TextureFormats;

    // Detecting redundant sampler/address setting.
    static const int MaximumStages = 4;
    D3DTEXTUREADDRESS    AddressMode[MaximumStages];
    D3DTEXTUREFILTERTYPE FilterType[MaximumStages];

    // Detects supported D3DFormats and capabilities.
    void            initTextureFormats();
    const TextureFormat* getTextureFormat(ImageFormat format) const;    

    // Texture Memory-mapping support.
    MappedTexture*  mapTexture(Texture* p, unsigned mipLevel, unsigned levelCount);
    MappedTexture*  mapTexture(Texture* p) { return mapTexture(p, 0, p->MipLevels); }
    void            unmapTexture(Texture *ptexture);    
    
    void            processTextureKillList();
    void            processInitTextures();    
    void            processImageUpdates() { ImageUpdates.ProcessUpdates(this); }

public:
    TextureManager(IDirect3DDevice9* pdevice,
                   D3DCapFlags caps);
    ~TextureManager();

    // Processes work queued up on other threads, including texture
    // creation and image updates.
    // This function should be called before each frame on the RenderThread.
    void            ServiceQueues();

    void            SetSamplerState( unsigned stage, D3DTEXTUREFILTERTYPE filter, D3DTEXTUREADDRESS address );

    // *** TextureManager
    virtual Render::Texture* CreateTexture(ImageFormat format, unsigned mipLevels,
                                           const ImageSize& size,
                                           unsigned use, unsigned arena, ImageBase* pimage);

    virtual unsigned        GetTextureUseCaps(ImageFormat format);
 
    // *** ImageUpdateSync
    virtual void    UpdateImage(Image* pimage);
    virtual void    UpdateImage(ImageUpdate* pupdate);
};


}}};  // namespace Scaleform::Render::D3D9

#endif
