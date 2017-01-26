/**********************************************************************

PublicHeader:   GFx
Filename    :   GFx/String.h
Content     :   String UTF8 string implementation with copy-on
                write semantics (thread-safe for assignment but not
                modification).
Created     :   April 27, 2007
Authors     :   Ankur Mohan, Michael Antonov

Notes       :
History     :

Copyright   :   (c) 1998-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFx_String_H
#define INC_SF_GFx_String_H

#include "Kernel/SF_String.h"

// GFxString... classes have been renamed to String.. classes and moved to GKernel
// typedefs are here only for backward compatibility

namespace Scaleform { namespace GFx {

// typedef String       String;
// typedef SF::StringLH     StringLH;
// typedef SF::StringDH     StringDH;
// typedef SF::StringBuffer StringBuffer;
// typedef StringDataPtr StringDataPtr;



}} // Scaleform::GFx

#endif // INC_SF_GFx_String_H
