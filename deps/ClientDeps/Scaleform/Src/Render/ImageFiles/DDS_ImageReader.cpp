/**********************************************************************

Filename    :   DDS_ImageReader.cpp
Content     :   DDS Image file format reader implementation
Created     :   February 2010
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "DDS_ImageFile.h"
#include "Render_ImageFileUtil.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace DDS {

// DDSD (DirectDraw Surface Descriptor) constants.
enum DDSFlags
{
    DDSF_CAPS               =0x00000001l,
    DDSF_HEIGHT             =0x00000002l,
    DDSF_WIDTH              =0x00000004l,
    DDSF_PITCH              =0x00000008l,
    DDSF_BACKBUFFERCOUNT    =0x00000020l,
    DDSF_ZBUFFERBITDEPTH    =0x00000040l,
    DDSF_ALPHABITDEPTH      =0x00000080l,
    DDSF_LPSURFACE          =0x00000800l,
    DDSF_PIXELFORMAT        =0x00001000l,
    DDSF_CKDESTOVERLAY      =0x00002000l,
    DDSF_CKDESTBLT          =0x00004000l,
    DDSF_CKSRCOVERLAY       =0x00008000l,
    DDSF_CKSRCBLT           =0x00010000l,
    DDSF_MIPMAPCOUNT        =0x00020000l,
    DDSF_REFRESHRATE        =0x00040000l,
    DDSF_LINEARSIZE         =0x00080000l,
    DDSF_TEXTURESTAGE       =0x00100000l,
    DDSF_FVF                =0x00200000l,
    DDSF_SRCVBHANDLE        =0x00400000l,
    DDSF_DEPTH              =0x00800000l
};

//
enum DDSPixelFormat
{
    DDSPF_ALPHAPIXELS       =0x00000001l,
    DDSPF_ALPHA             =0x00000002l,
    DDSPF_FOURCC            =0x00000004l,
    DDSPF_PALETTEINDEXED4   =0x00000008l,
    DDSPF_PALETTEINDEXEDTO8 =0x00000010l,
    DDSPF_PALETTEINDEXED8   =0x00000020l,
    DDSPF_RGB               =0x00000040l,
    DDSPF_COMPRESSED        =0x00000080l
};

struct DDSDescr
{
    UInt32    RGBBitCount;
    UInt32    RBitMask;
    UInt32    GBitMask;
    UInt32    BBitMask;
    UInt32    ABitMask;
    bool      HasAlpha;
    UByte     ShiftR, ShiftG, ShiftB, ShiftA;
    inline DDSDescr()
    {
        RGBBitCount = RBitMask = GBitMask = BBitMask = ABitMask = 0;
        ShiftR = ShiftG = ShiftB = ShiftA;
        HasAlpha = false;                
    }

    static UByte CalcShiftByMask(UInt32 mask)
    {
        unsigned shifts = 0;

        if (mask == 0) return 0;

        if ((mask & 0xFFFFFFu) == 0)
        {
            mask >>= 24;
            shifts += 24;
        }
        else if ((mask & 0xFFFFu) == 0)
        {
            mask >>= 16;
            shifts += 16;
        }
        else if ((mask & 0xFFu) == 0)
        {
            mask >>= 8;
            shifts += 8;
        }
        while((mask & 1) == 0)
        {
            mask >>= 1;
            ++shifts;
        }
        return UByte(shifts);
    }

    void CalcShifts()
    {
        ShiftR = CalcShiftByMask(RBitMask);
        ShiftG = CalcShiftByMask(GBitMask);
        ShiftB = CalcShiftByMask(BBitMask);
        ShiftA = CalcShiftByMask(ABitMask);
    }
};

struct DDSHeaderInfo
{
    UInt32 Width;
    UInt32 Height;
    UInt32 Pitch;
    ImageFormat Format;
    UInt32 MipmapCount;
    DDSDescr DDSFmt;
    DDSHeaderInfo() : Width(0), Height(0), Pitch(0), Format(Image_None), MipmapCount(1), DDSFmt(){}

};

// Temporary DDS image class used as a data source, allowing
// direct initialization of RawImage or Texture with image data.

class DDSFileImageSource : public FileImageSource
{
    UByte           ImageDesc; 
    DDSHeaderInfo   HeaderInfo;
    ImageData       Data;
    
public:
    DDSFileImageSource(File* file, ImageFormat format)
        : FileImageSource(file,format), ImageDesc(0)
    {
    }
    virtual      ~DDSFileImageSource(){}
    bool ReadHeader();
    virtual bool Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
    virtual unsigned        GetMipmapCount() const { return HeaderInfo.MipmapCount; }

    SF_AMP_CODE(virtual UPInt GetBytes() const { return Data.GetBytesPerPixel() * Data.GetSize().Area(); })
};


// *** DDS Format loading

static const UByte* ParseUInt32(const UByte* buf, UInt32* pval)
{
    *pval = Alg::ByteUtil::LEToSystem(*(UInt32*)buf);    
    return buf + 4;
}



static bool Image_ParseDDSHeader(DDSHeaderInfo* pinfo, const UByte* buf, const UByte** pdata)
{

    ImagePlane p0;
    UInt32 flags;
    UInt32 v;
    
    buf = ParseUInt32(buf, &flags);

    buf = ParseUInt32(buf, &v);
    if (flags & DDSF_HEIGHT)
        pinfo->Height = v;

    buf = ParseUInt32(buf, &v);
    if (flags & DDSF_WIDTH)
        pinfo->Width = v;
    buf = ParseUInt32(buf, &v);
    if (flags & DDSF_PITCH)
        pinfo->Pitch = v;
    else if (flags & DDSF_LINEARSIZE)
        pinfo->Pitch = v/pinfo->Height*4; // Required by D3D10

    buf = ParseUInt32(buf, &v);
    //if (flags & DDSF_DEPTH)
    //    pimage->Depth = v;

    buf = ParseUInt32(buf, &v);
    if (flags & DDSF_MIPMAPCOUNT)
        pinfo->MipmapCount = v;

    //buf = ParseUInt32(buf, &v); // alpha bit count
    //if (flags & DDSF_ALPHABITDEPTH)
    //    pimage->AlphaBitDepth = v;

    buf += 11 * 4;

    if (flags & DDSF_PIXELFORMAT)
    {
        // pixel format (DDPIXELFORMAT)
        buf = ParseUInt32(buf, &v); // dwSize
        if (v != 32) // dwSize should be == 32 
        {
            SF_ASSERT(0);
            return false;
        }
      
        UInt32 pfflags;
        buf = ParseUInt32(buf, &pfflags);   // dwFlags
        buf = ParseUInt32(buf, &v);         // dwFourCC
        if (pfflags & DDSPF_FOURCC)
        {
            if (v == 0x35545844)        // DXT5
                pinfo->Format = Image_DXT5;
            else if (v == 0x33545844)   // DXT3
                pinfo->Format = Image_DXT3;
            else if (v == 0x31545844)   // DXT1
                pinfo->Format = Image_DXT1;
            buf += 20;  // skip remaining part of PixelFormat
        }
        else if ((pfflags & DDSPF_RGB) || (pfflags & DDSPF_ALPHA)) 
        {
            // uncompressed DDS. Only 32-bit/24-bit RGB formats and alpha only (A8) are supported
            UInt32 bitCount;
            buf = ParseUInt32(buf, &bitCount); // dwRGBBitCount
            pinfo->DDSFmt.RGBBitCount = bitCount;
            switch(bitCount)
            {
                case 32: pinfo->Format = Image_R8G8B8A8; break;
                case 24: pinfo->Format = Image_R8G8B8; break;
                case 8:
                    if (pfflags & DDSPF_ALPHA)
                    {
                        pinfo->Format = Image_A8;
                        break;
                    }
                default:
                    SF_ASSERT(0); // unsupported
                    return false;
            }
            if (!(flags & DDSF_PITCH))
                pinfo->Pitch = pinfo->Width*(bitCount/8);
                //AB: what is the Pitch in DDS for 24-bit RGB?
            
            buf = ParseUInt32(buf, &v); // dwRBitMask
            pinfo->DDSFmt.RBitMask = v;
            buf = ParseUInt32(buf, &v); // dwGBitMask
            pinfo->DDSFmt.GBitMask = v;
            buf = ParseUInt32(buf, &v); // dwBBitMask
            pinfo->DDSFmt.BBitMask = v;
            buf = ParseUInt32(buf, &v); // dwRGBAlphaBitMask
            if (pfflags & DDSPF_ALPHAPIXELS) 
            {
                pinfo->DDSFmt.ABitMask = v;
                pinfo->DDSFmt.HasAlpha = true;
            }

            // check for X8R8G8B8 - need to set alpha to 255
            if (v == 0 && bitCount == 32)
            {
                SF_ASSERT(0); // not supported for now.
                //@TODO - need to have one more Image_<> format for X8R8G8B8
            }
        }
        SF_ASSERT(pinfo->Format != Image_None); // Unsupported format
        if (pinfo->Format == Image_None)
            return false;
    }
    else
        buf += 32;
    buf += 16; // skip ddsCaps
    buf += 4; // skip reserved
    if (pdata) *pdata = buf;
    return true;
}

static bool ProcessUDDSData(UByte* buffer, UPInt size, ImageFormat format, const DDSDescr& ddsFmt)
{   
    if (format == Image_R8G8B8)
    {
        for (unsigned x = 0; x < size; x += 3)
        {
            UInt32 val = buffer[x + 0] | (UInt32(buffer[x + 1]) << 8) | (UInt32(buffer[x + 2]) << 16);
            buffer[x + 2] = UByte((val >> ddsFmt.ShiftB) & 0xFF); // B
            buffer[x + 1] = UByte((val >> ddsFmt.ShiftG) & 0xFF); // G
            buffer[x + 0] = UByte((val >> ddsFmt.ShiftR) & 0xFF); // R
        }
    }
    else if (format == Image_R8G8B8A8)
    {
        for (unsigned x = 0; x < size; x += 4)
        {
            UInt32 val = buffer[x + 0] | (UInt32(buffer[x + 1]) << 8) |
                (UInt32(buffer[x + 2]) << 16) | (UInt32(buffer[x + 3]) << 24);
            buffer[x + 2] = UByte((val >> ddsFmt.ShiftB) & 0xFF); // B
            buffer[x + 1] = UByte((val >> ddsFmt.ShiftG) & 0xFF); // G
            buffer[x + 0] = UByte((val >> ddsFmt.ShiftR) & 0xFF); // R
            if (ddsFmt.HasAlpha)
                buffer[x + 3] = UByte((val >> ddsFmt.ShiftA) & 0xFF); // A
            else
                buffer[x + 3] = 0xFF;
        }
    }

    return true;
}

bool DDSFileImageSource::Decode( ImageData* pdest, CopyScanlineFunc copyScanline, void* arg ) const
{
    if (!seekFileToDecodeStart())
        return false;

    unsigned mipWidth = Size.Width;
    unsigned mipHeight = Size.Height;
    for (unsigned m = 0; m < pdest->GetMipLevelCount(); m++)
    {
        ImagePlane mipPlane;
        if (pdest->HasSeparateMipmaps())
            pdest->GetMipLevelPlane(m, 0, &mipPlane);
        else
            pdest->GetPlaneRef().GetMipLevel(pdest->GetFormat(), m, &mipPlane);
        //For compressed formats copy mipmap data directly. Since DXT formats store data in 4x4 blocks 
        // reading/converting by scanline will not work at least with current implementation of ImageScanlineBuffer
        if (Format >= Image_DXT1 && Format <= Image_DXT5) 
        {
            SF_ASSERT(pdest->Format == Format);
            if (pFile->Read(mipPlane.pData, (int)mipPlane.DataSize) != (int)mipPlane.DataSize)
                return false;            
        }
        else
        {
            ImageScanlineBuffer<1024*4> scanline(HeaderInfo.Format, mipWidth, Format);
            int readSize = (int)scanline.GetReadSize();
            if (!scanline.IsValid())
                return false;
            for (unsigned y = 0; y < mipHeight; y++)
            {
                if (pFile->Read(scanline.GetReadBuffer(), readSize) != readSize)
                    return false;
                if (Format == Image_R8G8B8A8 || Format == Image_R8G8B8)
                    ProcessUDDSData(scanline.GetReadBuffer(), readSize, Format, HeaderInfo.DDSFmt);
                UByte* destScanline = mipPlane.GetScanline(y);
                scanline.ConvertReadBuffer(destScanline, 0, copyScanline, arg);
            }
        }
        mipWidth /= 2;
        mipHeight /= 2;
    }
    return true;

}

bool DDSFileImageSource::ReadHeader()
{
//    int     fileSize = pFile->GetLength();
    UInt32  fourcc   = pFile->ReadUInt32();
    if (fourcc != 0x20534444) // 'D','D','S',' '
        return false;

    UInt32 sz = pFile->ReadUInt32();
    if (sz != 124)
        return false;
    UByte buf[256];
    if (pFile->Read(buf, 120) != 120)
        return false;

    if (!Image_ParseDDSHeader(&HeaderInfo, buf, 0))
        return false;
    HeaderInfo.DDSFmt.CalcShifts();

    if (Format == Image_None)
        Format =  HeaderInfo.Format;
    Size = ImageSize(HeaderInfo.Width, HeaderInfo.Height);
    FilePos = pFile->LTell();
//    MemoryHeap* pheap = args.GetHeap();
//    if (!pheap)
//        pheap = Memory::GetGlobalHeap();
//    
//    int     dataSize = fileSize - pFile->Tell();
//    UByte*  pdata    = (UByte*)SF_HEAP_ALLOC(pheap, dataSize, Stat_Image_Mem);
//    if (!pdata)
//        return 0;
//    if (pFile->Read(pdata, dataSize) != dataSize)
//    {
//        SF_FREE(pdata);
//        return 0;
//    }
//    
//    // AB: do we need to do same for uncompressed DDS?
//#ifdef SF_OS_XBOX360
//    if (Data->IsCompressed())
//    {
//        // We need to convert byte order for XBox360. This does not apply
//        // to other big-endian systems such as PS3.
//        UInt16 *pidata = (UInt16*)pdata;
//        int     i;
//        for (i=0; i<dataSize/2; i++)
//        {
//            *pidata = Alg::ByteUtil::LEToSystem(*pidata);
//            pidata++;
//        }   
//    }
//#endif
    return true;
}
bool FileReader::MatchFormat(File* file, UByte* headerArg, UPInt headerArgSize) const
{
    FileHeaderReader<4> header(file, headerArg, headerArgSize);
    if (!header)
        return false;
    if ((header[0] != 'D') || (header[1] != 'D') || (header[2] != 'S') || (header[3] != ' '))
        return false;
    return true;
}

ImageSource* FileReader::ReadImageSource(File* file, const ImageCreateArgs& args) const
{
    if (!file || !file->IsValid())
        return 0;

    DDSFileImageSource* source = SF_NEW DDSFileImageSource(file, args.Format);
    if (source && !source->ReadHeader())
    {
        source->Release();
        return 0;
    }
    return source; 
   
}

// Instance singleton.
FileReader FileReader::Instance;

}}}

