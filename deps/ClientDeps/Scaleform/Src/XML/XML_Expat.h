/**********************************************************************

PublicHeader:   XML
Filename    :   XML_Expat.h
Content     :   XML Parser implementation using expat
Created     :   March 6, 2008
Authors     :   Prasad Silva
Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_XMLExpat_H
#define INC_SF_GFX_XMLExpat_H

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "GFx/XML/XML_Support.h"
#include "GFx/XML/XML_Parser.h"

//
// This is an implementation of the SAX2 pluggable parser interface 
// using the expat XML parser library found at http://expat.sourceforge.net/.
// It has been verified to support Win32, Win64, Linux, OSX, PS2, PS3, 
// PSP, XBOX360 and Wii platforms. 
//
// The FxPlayer included in GFx distributions install an instance of 
// the expat XML parser delegate as the GFxLoader parser state. This 
// exposes functionality to parse XML files for the application and 
// loaded GFx movies.
//

namespace Scaleform {
namespace GFx {
namespace XML {

//
// Expat XML parser instance
//
//
class ParserExpat : public Parser
{
public:
    ParserExpat() {}

    //
    // Implementation of Parser
    //
    bool            ParseFile(const char* pfilename, FileOpenerBase* pfo, 
                              ParserHandler* pparseHandler);
    bool            ParseString(const char* pdata, UPInt len, 
                                ParserHandler* pparseHandler);
};

}}} //SF::GFx::XML

#endif //#ifdef GFX_ENABLE_XML

#endif  // #define INC_SF_GFX_XMLExpat_H
