/**********************************************************************

Filename    :   Render_ImageFile.cpp
Content     :   Image file handlers and registry for their support.
Created     :   August 2010
Authors     :   Michael Antonov

Copyright   :   (c) 2008-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"
#include "Render_ImageFile.h"

#include <stdarg.h>

namespace Scaleform { namespace Render {


//--------------------------------------------------------------------
// ***** ImageFileReader

// Creates an image by reading data from a file and applying its arguments.
Image* ImageFileReader::Read(File* file, const ImageCreateArgs& args) const
{
    Ptr<ImageSource> source = *ReadImageSource(file);
    return source ? source->CreateCompatibleImage(args) : 0;
}

    
// ***** ImageFileWriter

// static 
bool ImageFileWriter::writeImage(File* file, const ImageFileWriter* writer,
                                 Image* image, const ImageWriteArgs* args)
{
    SF_ASSERT(writer && image);

    Ptr<RawImage> tempImage;
    ImageData     idata;
    bool          needUnmap = false;

    if (image->GetImageType() == Image::Type_RawImage)
    {        
        ((RawImage*)image)->GetImageData(&idata);
    }

    else if (image->GetUse() & ImageUse_MapLocal)
    {
        if (!image->Map(&idata))
            return false;
        needUnmap = true;
    }

    else
    {
        tempImage = *RawImage::Create(image->GetFormat(), 1, image->GetSize(), 0);
        if (!tempImage) return false;

        tempImage->GetImageData(&idata);
        if (!image->Decode(&idata))
        {
            SF_DEBUG_WARNING(1, "ImageFileWriter failed - image not Mappable or Decodable");
            return false;
        }
    }

    bool success = writer->Write(file, idata, args);
    if (needUnmap)
        image->Unmap();
    return success;
}

//--------------------------------------------------------------------
// ***** ImageFileHandlerRegistry

ImageFileHandlerRegistry::ImageFileHandlerRegistry(unsigned handlerCount, ...)
{
    va_list vl;
    va_start(vl, handlerCount);

    for (unsigned i=0; i< handlerCount; i++)
    {
        ImageFileHandler* handler = va_arg(vl, ImageFileHandler*);
        if (handler)
            Handlers.PushBack(handler);
    }
    va_end(vl);
}

void ImageFileHandlerRegistry::AddHandler(ImageFileHandler* handler)
{
    SF_ASSERT(handler);
    Handlers.PushBack(handler);
}

ImageFileReader* ImageFileHandlerRegistry::GetReader(ImageFileFormat format)
{
    for (UPInt i = 0; i< Handlers.GetSize(); i++)    
        if (Handlers[i]->IsReader() && (Handlers[i]->GetFormat() == format))
            return (ImageFileReader*)Handlers[i];
    return 0;
}

ImageFileWriter* ImageFileHandlerRegistry::GetWriter(ImageFileFormat format)
{
    for (UPInt i = 0; i< Handlers.GetSize(); i++)    
        if (Handlers[i]->IsWriter() && (Handlers[i]->GetFormat() == format))
            return (ImageFileWriter*)Handlers[i];
    return 0;
}

ImageFileFormat ImageFileHandlerRegistry::DetectFormat(
    ImageFileReader** preader, File* file, UByte* header, UPInt headerSize)
{
    ImageFileReader* temp = 0;
    if (!preader)
        preader = &temp;

    for (UPInt i = 0; i< Handlers.GetSize(); i++)  
    {
        if (Handlers[i]->IsReader())
        {
            ImageFileReader* reader = (ImageFileReader*)Handlers[i];
            if (reader->MatchFormat(file, header, headerSize))
            {
                *preader = reader;
                return (*preader)->GetFormat();
            }
        }
    }
    *preader = 0;
    return ImageFile_Unknown;
}


Image* ImageFileHandlerRegistry::ReadImage(File* file, const ImageCreateArgs& args) const
{
    for (UPInt i = 0; i< Handlers.GetSize(); i++)
    {
        if (Handlers[i]->IsReader())
        {
            ImageFileReader* reader = (ImageFileReader*)Handlers[i];
            if (reader->MatchFormat(file, 0, 0))
                return reader->Read(file, args);
        }                
    }
    return 0;
}

}}; // namespace Scaleform::Render
