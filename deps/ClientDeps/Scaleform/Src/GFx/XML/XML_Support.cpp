/**********************************************************************

Filename    :   XML_Support.cpp
Content     :   GFx XML support
Created     :   March 7, 2008
Authors     :   Prasad Silva
Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "XML_Support.h"

namespace Scaleform {
namespace GFx {
namespace XML {

//
// Load file given the provided file opener and DOM builder.
//
bool    Support::ParseFile(const char* pfilename, 
                                 FileOpenerBase* pfo, 
                                 ParserHandler* pdb)
{
    SF_ASSERT(pParser.GetPtr());
    return pParser->ParseFile(pfilename, pfo, pdb);
}

//
// Load a string using the provided DOM builder
//
bool    Support::ParseString(const char* pdata, 
                                   UPInt len, 
                                   ParserHandler* pdb)
{
    SF_ASSERT(pParser.GetPtr());
    return pParser->ParseString(pdata, len, pdb);
}

}}} //SF::GFx::XML

#endif  // SF_NO_XML_SUPPORT
