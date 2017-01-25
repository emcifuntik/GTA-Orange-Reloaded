/**********************************************************************

Filename    :   Amp_Stream.cpp
Content     :   Stream used by AMP to serialize messages
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   (c) 2005-2009 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "Amp_Stream.h"
#include "Kernel/SF_Alg.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

// Default constructor (for writing a new message)
AmpStream::AmpStream() : readPosition(0)
{
}

// Buffer constructor (for parsing a received message)
// Assumes that the input buffer includes the header information (length)
AmpStream::AmpStream(const UByte* buffer, UPInt bufferSize) : readPosition(0)
{
    Append(buffer, bufferSize);
}

// File virtual method override
int AmpStream::SkipBytes(int numBytes)
{ 
    readPosition += numBytes; 
    return numBytes; 
}

// File virtual method override
int AmpStream::BytesAvailable()
{ 
    return (static_cast<int>(Data.GetSize()) - readPosition); 
}

// File virtual method override
// Not supported for AmpStream
int AmpStream::Seek(int offset, int origin)
{ 
    SF_UNUSED2(offset, origin); 
    SF_ASSERT(false); // Seek not supported
    return -1; 
}

// File virtual method override
// Not supported for AmpStream
SInt64 AmpStream::LSeek(SInt64 offset, int origin)
{ 
    SF_UNUSED2(offset, origin); 
    SF_ASSERT(false); // LSeek not supported
    return -1;  
}

// File virtual method override
// Not supported for AmpStream
bool AmpStream::ChangeSize(int newSize)
{ 
    SF_UNUSED(newSize); 
    SF_ASSERT(false); // ChangeSize not supported
    return false;
}

// File virtual method override
// Not supported for AmpStream
int AmpStream::CopyFromStream(File *pstream, int byteSize)
{ 
    SF_UNUSED2(pstream, byteSize);
    SF_ASSERT(false); // Copy from stream not supported
    return -1;
}

// Write a number of bytes to the stream
// File virtual method override
// Updates the message size stored in the header
int AmpStream::Write(const UByte *pbufer, int numBytes)
{
    IncreaseMessageSize(numBytes);
    memcpy(&Data[GetBufferSize() - numBytes], pbufer, numBytes);
    return numBytes;
}

// Read a number of bytes from the stream
// File virtual method override
int AmpStream::Read(UByte *pbufer, int numBytes)
{
    memcpy(pbufer, &Data[readPosition], numBytes);
    readPosition += numBytes;
    return numBytes;
}

// Read a string
void AmpStream::ReadString(String* str)
{
    str->Clear();
    UInt32 stringLength = ReadUInt32();
    for (UInt32 i = 0; i < stringLength; ++i)
    {
        str->AppendChar(ReadSByte());
    }
}

// Write a string
void AmpStream::WriteString(const String& str)
{
    WriteUInt32(static_cast<UInt32>(str.GetLength()));
    for (UPInt i = 0; i < str.GetLength(); ++i)
    {
        WriteSByte(str[i]);
    }
}

void AmpStream::ReadStream(AmpStream* str)
{
    UInt32 streamLength = ReadUInt32();
    str->Data.Resize(streamLength);
    for (UInt32 i = 0; i < streamLength; ++i)
    {
        str->Data[i] = ReadUByte();
    }
    str->Rewind();
}

void AmpStream::WriteStream(const AmpStream& str)
{
    WriteUInt32(str.GetBufferSize());
    Write(str.GetBuffer(), str.GetBufferSize());
}

// Append a buffer
// The buffer should already be in the proper format
// i.e. with message sizes stored before each message
void AmpStream::Append(const UByte* buffer, UPInt bufferSize)
{
    Data.Append(buffer, bufferSize);
    Rewind();
}

// Data accessor
const UByte* AmpStream::GetBuffer() const
{
    return Data.GetDataPtr();
}

// Buffer size
UPInt AmpStream::GetBufferSize() const
{
    return Data.GetSize();
}

// Update the message size
// Assumes that there is only one message currently in the stream
void AmpStream::IncreaseMessageSize(UInt32 newSize)
{
    // Make space for header
    UInt32 sizePlusHeader;
    if (Data.GetSize() == 0)
    {
        sizePlusHeader = sizeof(UInt32) + newSize;
    }
    else
    {
        sizePlusHeader = static_cast<UInt32>(Data.GetSize()) + newSize;
    }
    Data.Resize(sizePlusHeader);

    // Write the header
    UInt32 sizeLE = Alg::ByteUtil::SystemToLE(sizePlusHeader);
    memcpy(&Data[0], reinterpret_cast<UByte*>(&sizeLE), sizeof(UInt32));
}

// Clear the buffer
void AmpStream::Clear()
{
    Data.Clear();
    WriteUInt32(0);
    Rewind();
}

// Start reading from the beginning
void AmpStream::Rewind()
{
    readPosition = sizeof(UInt32);  // skip the header
}

// The size of the data without the header
UPInt AmpStream::FirstMessageSize()
{
    int oldIndex = readPosition;
    readPosition = 0;
    UInt32 msgSize = ReadUInt32();
    readPosition = oldIndex;
    return msgSize;
}

// Remove the first message from the stream
// Returns true if successful
bool AmpStream::PopFirstMessage()
{
    UPInt messageSize = FirstMessageSize();
    UPInt bufferLength = GetBufferSize();
    if (messageSize > bufferLength)
    {
        // incomplete message
        return false;
    }
    for (UPInt i = messageSize; i < bufferLength; ++i)
    {
        Data[i - messageSize] = Data[i];
    }
    Data.Resize(bufferLength - messageSize);
    Rewind();
    return true;
}

} // namespace AMP
} // namespace GFx
} // namespace Scaleform
