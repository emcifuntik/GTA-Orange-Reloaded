/**********************************************************************

Filename    :   GL_Texture.h
Content     :   GL Texture and TextureManager header
Created     :   
Authors     :   

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_GL_Texture_H
#define INC_SF_Render_GL_Texture_H

#include "Render/GL/GL_Common.h"

#include "Kernel/SF_List.h"
#include "Kernel/SF_Threads.h"
#include "Render/Render_Image.h"
#include "Render/Render_MemoryManager.h"
#include "Kernel/SF_HeapNew.h"


namespace Scaleform { namespace Render { namespace GL {


    // TextureFormat describes format of the texture and its caps.
    // Format includes allowed usage capabilities and ImageFormat
    // from which texture is supposed to be initialized.

    struct TextureFormat
    {
        ImageFormat              Format, ConvFormat;
        GLenum                   GLColors, GLFormat, GLData;
        UByte                    BytesPerPixel;
        bool                     Mappable;
        UByte                    DrawModes;
        const char*              Extension;
        Image::CopyScanlineFunc  CopyFunc;

        ImageFormat     GetImageFormat() const { return Format; }

        unsigned        GetPlaneCount() const
        { return ImageData::GetFormatPlaneCount(GetImageFormat()); }
    };

    class MappedTexture;
    class TextureManager;
    class HAL;


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


    // GL Texture class implementation; it man actually include several HW 
    // textures (one for each ImageFormat plane).

    class Texture : public Render::Texture, public ListNode<Texture>
    {
    public:
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
        ImageBase*               pImage;
        Ptr<RawImage>            pBackingImage;
        const TextureFormat*     pFormat;

        ImageSize               ImgSize;

        UByte                   MipLevels;
        UByte                   TextureCount;
        UInt16                  Use;
        UByte                   TextureFlags;
        CreateState             State;

        // If texture is currently mapped, it is here.
        MappedTexture*          pMap;

        struct HWTextureDesc
        {
            ImageSize           Size;
            GLuint              TexId;
        };

        // TextureDesc array is allocated if more then one is needed.
        HWTextureDesc*          pTextures;
        HWTextureDesc           Texture0;

        GLint                   LastMinFilter, LastAddress;

        Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat, unsigned mipLevels,
            const ImageSize& size, unsigned use, unsigned arena, ImageBase* pimage);
        ~Texture();

        ImageFormat             GetImageFormat() const { return pFormat->Format; }
        TextureManager*         GetManager() const     { return pManagerLocks->pManager; }
        inline  HAL*      GetHAL() const;
        bool                    IsValid() const        { return pTextures != 0; }

        void                    LoseManager();    
        bool                    Initialize();
        void                    ReleaseHWTextures();

        inline unsigned         GetTextureStageCount() const { return TextureCount; }

        // *** Interface implementation

        //    virtual HAL*      GetRenderHAL() const { SF_ASSERT(0); return 0; } // TBD

        virtual Render::TextureManager* GetTextureManager() const;
        virtual Image*          GetImage() const
            { SF_ASSERT(!pImage || (pImage->GetImageType() != Image::Type_ImageBase)); return (Image*)pImage; }
        virtual ImageFormat     GetFormat() const         { return GetImageFormat(); }
        virtual ImageSize       GetSize() const           { return ImgSize; }
        virtual unsigned        GetMipmapCount() const    { return (Use & ImageUse_GenMipmaps) ? 1 : MipLevels; }

        // Obtains lock & stop using the image
        virtual void            ImageLost();

        virtual bool            Map(ImageData* pdata, unsigned mipLevel, unsigned levelCount);
        virtual bool            Unmap();

        virtual bool            Update(const UpdateDesc* updates, unsigned count = 1, unsigned mipLevel = 0);    
        virtual bool            Update();

        virtual bool            Upload(unsigned itex, unsigned level, const ImagePlane& plane);
        virtual void            MakeMappable();
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



    // GL Texture Manger.
    // This class is responsible for creating textures and keeping track of them
    // in the list.
    // 

    class TextureManager : public Render::TextureManager
    {
        friend class Texture;

        typedef ArrayConstPolicy<8, 8, false>   KillListArrayPolicy;
        typedef ArrayLH<GLuint,
            StatRender_TextureManager_Mem,
            KillListArrayPolicy>    GLTextureArray;

        enum TextureCaps
        {
            TC_NonPower2Limited = 1,
            TC_NonPower2Full    = 2,
#if defined(GL_ES_VERSION_2_0)
            TC_UseBgra          = 4,
#endif
        };

        MappedTexture       MappedTexture0;

        Ptr<TextureManagerLocks> pLocks;
        HAL*                     pHal;
        unsigned                 Caps;
        // Lists protected by TextureManagerLocks::TextureMutex.
        List<Texture>            Textures;
        List<Texture>            TextureInitQueue;
        GLTextureArray           GLTextureKillList;
        ImageUpdateQueue         ImageUpdates;
        ThreadId                 RenderThreadId;

        // Texture format table, organized by supported HW features.
        ArrayLH<TextureFormat*>  TextureFormats;

        const TextureFormat* getTextureFormat(ImageFormat format) const;

        // Texture Memory-mapping support.
        MappedTexture*  mapTexture(Texture* p, unsigned mipLevel, unsigned levelCount);
        MappedTexture*  mapTexture(Texture* p) { return mapTexture(p, 0, p->MipLevels); }
        void            unmapTexture(Texture *ptexture);    

        void            processImageUpdates() { ImageUpdates.ProcessUpdates(this); }
        void            processInitTextures();
        void            processTextureKillList();

    public:
        TextureManager();
        ~TextureManager();

        // XXX - use Extensions instead if needed
        void            Initialize(HAL* phal);

        // Processes work queued up on other threads, including texture
        // creation and image updates.
        // This function should be called before each frame on the RenderThread.
        void            ProcessQueues();

        // *** TextureManager
        virtual Render::Texture* CreateTexture(ImageFormat format, unsigned mipLevels,
            const ImageSize& size, unsigned use, unsigned arena = 0, ImageBase* pimage = 0);

        bool            IsMultiThreaded() { return false; }
        unsigned        GetTextureUseCaps(ImageFormat format);
        bool            IsNonPow2Supported(ImageFormat format, UInt16 use);

        // *** ImageUpdateSync
        virtual void    UpdateImage(Image* pimage);
        virtual void    UpdateImage(ImageUpdate* pupdate);
    };

    GL::HAL* Texture::GetHAL() const
    {
        return pManagerLocks->pManager->pHal;
    }

}}};  // namespace Scaleform::Render::GL

#endif
