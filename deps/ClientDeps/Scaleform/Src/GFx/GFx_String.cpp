/**********************************************************************

Filename    :   String.cpp
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

#include "GFx/GFx_String.h"

#include "Kernel/SF_UTF8Util.h"
#include "Kernel/SF_Debug.h"
//#include "Kernel/SF_Functions.h"

#include <stdlib.h>
#include <ctype.h>

#ifdef SF_OS_QNX
# include <strings.h>
#endif

namespace Scaleform { namespace GFx {

}} // Scaleform::GFx
