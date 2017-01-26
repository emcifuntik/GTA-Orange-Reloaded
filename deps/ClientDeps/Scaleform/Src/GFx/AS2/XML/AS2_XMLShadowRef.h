/**********************************************************************

Filename    :   AS2_XMLShadowRef.h
Content     :   XML DOM support
Created     :   Februrary 5, 2009
Authors     :   Prasad Silva
Copyright   :   (c) 2005-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_XMLShadowRef_H
#define INC_SF_GFX_XMLShadowRef_H

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "GFx/XML/XML_Document.h"
#include "AS2/AS2_Object.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// forward declarations
class XmlNodeObject;


//
// The elements of the DOM tree are defined here. Each is managed
// from a memory pool and is ref counted, except for Attribute
// which belongs to one and only one ElementNode. 
//


// 
// A reference holder to an AS shadow object for a GFxXMLNode. This class
// contains a seperate reference to a Object that holds the attributes
// of the AS XMLNode object even if the node object is freed.
//
// This special structure is required because attributes are offloaded to
// actionscript when the shadow object is created for correct Flash-like 
// behavior. This still does not solve the problem with setting
// custom properties to the AS XMLNode object, which will be deleted when
// all references are dropped. This is the case when a file is loaded and 
// custom properties are set to a child in the DOM tree. If all references
// to the child are removed, and found again using a tree traversal, the
// custom properties won't exist.
//
struct XMLShadowRef : XML::ShadowRefBase
{
    XmlNodeObject*       pASNode;
    Ptr<Object>         pAttributes;

    XMLShadowRef() : pASNode(NULL) {}
};

}}} //namespace SF::GFx::AS2

#endif  // GFX_ENABLE_XML

#endif  // INC_SF_GFX_XMLShadowRef_H
