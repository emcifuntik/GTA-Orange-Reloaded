/**********************************************************************

Filename    :   PNG_ImageReader.cpp
Content     :   PNG Image file format reader PNG implementation
Created     :   February 2010
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "PNG_ImageFile.h"
#include "Render_ImageFileUtil.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

#ifdef SF_ENABLE_LIBPNG
#include "png.h"

#ifdef SF_CC_MSVC
// disable warning "warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable"
#pragma warning(disable:4611)
#endif

namespace Scaleform { namespace Render { namespace PNG {


struct PngContext
{
    png_structp         png_ptr;
    png_infop           info_ptr;
    png_uint_32         width, height; 
    int                 bitDepth;
    int                 colorType;
    int                 interlaceType;
    png_uint_32         ulRowBytes;
    char                errorMessage[100];
    char                filePath[256];
};

static void png_error_handler(png_structp png_ptr, png_const_charp msg)
{
    UPInt mlen = SFstrlen(msg);

    PngContext *pcontext = (PngContext*)png_get_error_ptr(png_ptr);
    if (mlen < sizeof(pcontext->errorMessage))
        SFstrcpy(pcontext->errorMessage, sizeof(pcontext->errorMessage), msg);
    else
    {
        SFstrncpy(pcontext->errorMessage, sizeof(pcontext->errorMessage), msg, sizeof(pcontext->errorMessage) - 1);
        pcontext->errorMessage[sizeof(pcontext->errorMessage) - 1] = 0;
    }
    longjmp(png_ptr->jmpbuf, 1);
}

static void png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    File* pfile = reinterpret_cast<File*>(png_ptr->io_ptr);
    int check = pfile->Read(data, (int)length);

    if (check < 0 || ((png_size_t)check) != length)
    {
        png_error(png_ptr, "Read Error.");
    }
}

static int GFxPngReadInfo(PngContext* context)
{
    double              dGamma;

    if (setjmp(png_jmpbuf(context->png_ptr)))
    {
        SF_DEBUG_WARNING2(1, "Image::ReadPng failed - Can't read info from file %s, error - %s\n", 
            context->filePath, context->errorMessage);
        return 0;
    }

    // initialize the png structure

    png_set_sig_bytes(context->png_ptr, 8);

    // read all PNG info up to image data

    png_read_info(context->png_ptr, context->info_ptr);

    // get width, height, bit-depth and color-type

    png_get_IHDR(context->png_ptr, context->info_ptr, &context->width, &context->height, &context->bitDepth,
        &context->colorType, &context->interlaceType, NULL, NULL);

    // expand images of all color-type and bit-depth to 3x8 bit RGB images
    // let the library process things like alpha, transparency, background

    // convert to 8-bit per color component
    if (context->bitDepth == 16)
        png_set_strip_16(context->png_ptr);

    // convert palette  (8-bit per pixel) to RGB
    if (context->colorType == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(context->png_ptr);

    if (context->bitDepth < 8)
#if (PNG_LIBPNG_VER_SONUM >= 14)
        png_set_expand_gray_1_2_4_to_8(context->png_ptr);
#else
        png_set_gray_1_2_4_to_8(context->png_ptr);
#endif

    if (png_get_valid(context->png_ptr, context->info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(context->png_ptr);

    if (context->colorType == PNG_COLOR_TYPE_GRAY ||
        context->colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(context->png_ptr);


    //!AB: we don't need to set background since this will replace transparent pixels
    // by the background colored ones.
    // set the background color to draw transparent and alpha images over.
    //png_color_16 *pBackground;
    //if (png_get_bKGD(context->png_ptr, context->info_ptr, &pBackground))
    //{
    //    png_set_background(context->png_ptr, pBackground, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
    //}

    // if required set gamma conversion
    if (png_get_gAMA(context->png_ptr, context->info_ptr, &dGamma))
        png_set_gamma(context->png_ptr, (double) 2.2, dGamma);

    // after the transformations have been registered update info_ptr data

    png_read_update_info(context->png_ptr, context->info_ptr);

    // get again width, height and the new bit-depth and color-type

    png_get_IHDR(context->png_ptr, context->info_ptr, &context->width, &context->height, &context->bitDepth,
        &context->colorType, &context->interlaceType, NULL, NULL);

    // row_bytes is the width x number of channels

    context->ulRowBytes = png_get_rowbytes(context->png_ptr, context->info_ptr);

    return 1;
}

class LibPNGInput : public Input
{

private:
    PngContext Context;
    LibPNGInput() { }
public:
    bool IsInitialized;

    virtual int ReadData(void** ppData) 
    {
        if (setjmp(png_jmpbuf(Context.png_ptr)))
        {
            SF_DEBUG_WARNING2(1, "GImage::ReadPng failed - Can't read data from file %s, error - %s\n", 
                Context.filePath, Context.errorMessage);
            return 0;
        }
        png_read_image(Context.png_ptr, (png_bytepp)ppData);
        return 1;
    }

    virtual int ReadScanline (UByte* prgbData)
    {
        SF_ASSERT(IsInitialized);
        png_read_row(Context.png_ptr, prgbData, NULL);
        return 1;
    }

    LibPNGInput(File* pin) : IsInitialized(false)
    {
        png_byte            pbSig[8];

        if (!pin || !pin->IsValid() )
            return;

        memset(&Context, 0, sizeof(Context));

        const char* ppath = pin->GetFilePath();
        SFstrcpy(Context.filePath, sizeof(Context.filePath), ppath);

        // first check the eight byte PNG signature

        if (pin->Read(pbSig, 8) != 8 || !png_check_sig(pbSig, 8))
        {
            SF_DEBUG_WARNING1(1, "Image::ReadPng failed - Can't read signature from file %s\n", 
                Context.filePath);
            return;
        }

        // create the two png(-info) structures

        Context.png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)&Context,  
            (png_error_ptr)png_error_handler, NULL);
        if (!Context.png_ptr)
        {
            SF_DEBUG_WARNING1(1, "Image::ReadPng failed - Can't create read struct for file %s\n", 
                Context.filePath);
            return;
        }

        Context.info_ptr = png_create_info_struct(Context.png_ptr);
        if (!Context.info_ptr)
        {
            SF_DEBUG_WARNING1(1, "Image::ReadPng failed - Can't create info struct for file %s\n", 
                Context.filePath);
            png_destroy_read_struct(&Context.png_ptr, NULL, NULL);
            return;
        }

        png_set_read_fn(Context.png_ptr, (png_voidp)pin, png_read_data);

        if (!GFxPngReadInfo(&Context))
        {
            // Context.errorMessage contains an error message
            png_destroy_read_struct(&Context.png_ptr, &Context.info_ptr, NULL);
            return;
        }
        IsInitialized = true;
    }
    

    ImageSize GetSize() const
    {
        SF_ASSERT(IsInitialized);
        return ImageSize(Context.width, Context.height);
    }

    bool Decode(ImageFormat format, ImageData* pdest, Image::CopyScanlineFunc copyScanline, void* arg)
    {
        unsigned                srcScanLineSize = 0;
        bool                success = true;
        ImageFormat         readFormat;

        switch(Context.colorType)
        {
        case PNG_COLOR_TYPE_RGB:        readFormat = Image_R8G8B8;   srcScanLineSize = Context.width*3; break;
        case PNG_COLOR_TYPE_RGB_ALPHA:  readFormat = Image_R8G8B8A8; srcScanLineSize = Context.width*4; break;
        default: readFormat = Image_None; break;
        }
        if (Context.ulRowBytes != 0) // can PngContext.ulRowBytes be zero? 
            srcScanLineSize = Context.ulRowBytes;

        // align srcScanLineSize on boundary of 4
        srcScanLineSize = (srcScanLineSize + 3) & (~3u);

        if (readFormat != Image_None)
        {

            //For non-interlaced PNG we can read by scanline, for interlaced we must read whole image
            if (!Context.interlaceType)
            {
                ImageScanlineBuffer<1024*4> scanline(readFormat, Context.width, format);

                for (unsigned y = 0; y < Context.height; y++)
                {
                    if (!ReadScanline(scanline.GetReadBuffer()))
                    {
                        success = false;
                        break; // read error!
                    }
                    UByte* destScanline = pdest->GetScanline(y);
                    scanline.ConvertReadBuffer(destScanline, 0, copyScanline, arg);
                }
            }
            else
            {
                png_bytepp ppbRowPointers =  (png_bytepp) SF_ALLOC(Context.height
                    * sizeof(png_bytep) + Context.height * srcScanLineSize * sizeof(png_byte), Stat_Default_Mem);
                 
                ppbRowPointers[0] = (png_bytep) (ppbRowPointers + Context.height);
          
                if (!ppbRowPointers)
                {
                    SF_DEBUG_WARNING1(1, "Image::ReadPng failed - Out of memory, file %s\n", 
                        Context.filePath);
                    png_destroy_read_struct(&Context.png_ptr, &Context.info_ptr, NULL);
                    return false;
                }
                
                // set the individual row-pointers to point at the correct offsets
                for (unsigned i = 1; i < Context.height; i++)
                {
                    ppbRowPointers[i] = ppbRowPointers[i-1] + srcScanLineSize;                    
                }

                if (!ReadData((void**)ppbRowPointers))
                {
                    SF_DEBUG_WARNING1(1, "Image::ReadPng failed, file %s\n", 
                        Context.filePath);
                    SF_FREE(ppbRowPointers);
                    png_destroy_read_struct(&Context.png_ptr, &Context.info_ptr, NULL);
                    return false;
                }

                ImageScanlineBuffer<1024*4> scanline(readFormat, Context.width, format);
                for (unsigned y = 0; y < Context.height; y++)
                {
                    memcpy(scanline.GetReadBuffer(), ppbRowPointers[y], scanline.GetReadSize());
                    UByte* destScanline = pdest->GetScanline(y);
                    scanline.ConvertReadBuffer(destScanline, 0, copyScanline, arg);
                }
                SF_FREE(ppbRowPointers);
            }

            png_read_end(Context.png_ptr, NULL);
            
        }
        png_destroy_read_struct(&Context.png_ptr, &Context.info_ptr, NULL);
        return success;
    }

    virtual ~LibPNGInput() { }
};


class MemoryBufferImage : public Render::MemoryBufferImage
{
public:
    MemoryBufferImage(ImageFormat format, const ImageSize& size, unsigned use,
        unsigned textureArena, ImageUpdateSync* sync, 
        File* file, SInt64 filePos, UPInt length = 0)
        : Render::MemoryBufferImage(format,size, use, textureArena, sync,
        file, filePos, length)
    { }

    virtual bool Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const
    {
        MemoryFile file(FilePath, &FileData[0], (int)FileData.GetSize());
        Input*     pin = FileReader::Instance.CreateInput(&file);
        if (!pin) return 0;
        
        bool success = pin->Decode(Format, pdest, copyScanline, arg);
        delete pin;
        return success;
    }
};


class PNGFileImageSource : public FileImageSource
{
    ImageFormat     SourceFormat;
    mutable Input* pOriginalInput;
public:
    PNGFileImageSource(File* file, ImageFormat format)
    : FileImageSource(file,format), pOriginalInput(0),
      SourceFormat(Image_None)
    {
    }
    virtual ~PNGFileImageSource() 
    {
        if (pOriginalInput)
        {
            delete pOriginalInput;
        }
    }
    virtual bool        Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
    virtual Image*      CreateCompatibleImage(const ImageCreateArgs& args);
    bool                ReadHeader();
    virtual unsigned    GetMipmapCount() const {return 1;}
};



bool PNGFileImageSource::Decode(ImageData* pdest,
                                CopyScanlineFunc copyScanline, void* arg) const
{
    return pOriginalInput ? pOriginalInput->Decode(Format, pdest, copyScanline, arg) : false;
}

bool PNGFileImageSource::ReadHeader()
{
    SF_ASSERT(pOriginalInput == 0);
    pOriginalInput = FileReader::Instance.CreateInput(pFile);
    if (!pOriginalInput)
        return false;
    Size = pOriginalInput->GetSize();
    if (Format == Image_None)
        Format = Image_R8G8B8A8;
    return true;
}

Image* PNGFileImageSource::CreateCompatibleImage( const ImageCreateArgs& args )
{
    if (IsDecodeOnlyImageCompatible(args))
    {
        return SF_HEAP_NEW(args.GetHeap())
            MemoryBufferImage(GetFormat(), GetSize(),
            args.Use, args.TextureArena, args.GetUpdateSync(),
            pFile, FilePos);
    }
    return Render::ImageSource::CreateCompatibleImage(args);
}
// ***** FileReader

bool FileReader::MatchFormat(File* file, UByte* headerArg, UPInt headerArgSize) const
{
    FileHeaderReader<8> header(file, headerArg, headerArgSize);
    if (!header)
        return false;
    if (!png_check_sig((png_byte*)header.GetPtr(), 8))
        return false;
    return true;

}

ImageSource* FileReader::ReadImageSource(File* file, const ImageCreateArgs& args) const
{
    if (!file || !file->IsValid())
        return 0;
    PNGFileImageSource* source = SF_NEW PNGFileImageSource(file, args.Format);

    if (source && !source->ReadHeader())
    {
        source->Release();
        source = 0;
    }

    return source;    
}

Input* FileReader::CreateInput(File* pin) const
{
    if (!pin || !pin->IsValid()) return 0;
    return SF_NEW LibPNGInput(pin);
}

// Instance singleton.
FileReader FileReader::Instance;


// ***** FileWriter

static void png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    File* pfile = reinterpret_cast<File*>(png_ptr->io_ptr);
    int check = pfile->Write(data, (int)length);

    if (check < 0 || ((png_size_t)check) != length)
    {
        png_error(png_ptr, "Write Error.");
    }
}

bool FileWriter::Write(File* file, const ImageData& imageData, const ImageWriteArgs* args) const
{
    SF_UNUSED(args);
    if (!file || !file->IsValid())
    {
       return false;
    }

    PngContext context;
    png_bytep* ppbRowPointers = 0;

    const char* ppath = file->GetFilePath();
    SFstrcpy(context.filePath, sizeof(context.filePath), ppath);
    context.width = imageData.GetWidth();
    context.height = imageData.GetHeight();

    switch (imageData.Format)
    {
    case Image_R8G8B8A8:
    case Image_B8G8R8A8:
        context.colorType = PNG_COLOR_TYPE_RGB_ALPHA;
        break;
    case Image_R8G8B8:
    case Image_B8G8R8:
        context.colorType = PNG_COLOR_TYPE_RGB;
        break;
    default:
        SF_DEBUG_WARNING(1, "FileWriter::Write failed.\n");
        return false;
    }
    context.bitDepth = 8; //Always 8 in GImage (?)
    context.png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)&context,  
        (png_error_ptr)png_error_handler, NULL);

    if (!context.png_ptr)
    {
        SF_DEBUG_WARNING1(1, "FileWriter::Write(%s): png_create_write_struct failed.\n", context.filePath);
        return false;
    }

    context.info_ptr = png_create_info_struct(context.png_ptr);

    if (!context.info_ptr)
    {
        SF_DEBUG_WARNING1(1, "FileWriter::Write(%s): png_create_info_struct failed.\n", context.filePath);
        return false;
    }

    png_set_write_fn(context.png_ptr, (png_voidp)file, png_write_data, NULL);

    if (setjmp(png_jmpbuf(context.png_ptr)))
    {
        SF_DEBUG_WARNING2(1, "FileWriter::Write(%s): Writing header failed. Error: %s\n",
            context.filePath, context.errorMessage);
        return false;
    }

    png_set_IHDR(context.png_ptr, context.info_ptr, imageData.GetWidth(), imageData.GetHeight(),
        context.bitDepth, context.colorType, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(context.png_ptr, context.info_ptr);


    if (setjmp(png_jmpbuf(context.png_ptr)))
    {
        SF_DEBUG_WARNING2(1, "FileWriter::Write(%s): Writing image data failed. Error: %s\n",
            context.filePath, context.errorMessage);
        if (ppbRowPointers)
            SF_FREE(ppbRowPointers);
        return false;
    }

    ppbRowPointers = (png_bytep*) SF_ALLOC(sizeof(png_bytep) * imageData.GetHeight(), Stat_Default_Mem);
    for (unsigned i = 0; i < context.height; i++)
        ppbRowPointers[i] = (png_bytep)imageData.GetScanline(i); //(png_byte*) malloc(info_ptr->rowbytes);

    png_write_image(context.png_ptr, ppbRowPointers);
    SF_FREE(ppbRowPointers);

    if (setjmp(png_jmpbuf(context.png_ptr)))
    {
        SF_DEBUG_WARNING(1, "FileWriter::Write: Error during end of write");
        return false;
    }

    png_write_end(context.png_ptr, NULL);

    return true;
}

// Instance singleton.
FileWriter FileWriter::Instance;


}}}; // namespace Scaleform::Render::PNG

#endif // SF_ENABLE_LIBPNG
