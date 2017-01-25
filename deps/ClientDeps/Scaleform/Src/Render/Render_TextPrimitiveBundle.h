/**********************************************************************

Filename    :   Render_TextPrimitiveBundle.h
Content     :   PrimitiveBundle for Text
Created     :
Authors     :   Michael Antonov

Notes       :   
History     :   

Copyright   :   (c) 2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#ifndef INC_SF_Render_TextPrimitiveBundle_H
#define INC_SF_Render_TextPrimitiveBundle_H

#include "Render_TreeCacheText.h"
#include "Render_MeshCache.h" // for MeshUseStatus
#include "Render_TextMeshProvider.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {


class TextPrimitiveBundle;


// Each TextBatch layer turns into a primitive.
//  - initially not all TFs are in primitive, but they should be before they are drawn.
struct TextLayerPrimitive : public Primitive
{
    friend class TextPrepareBuffer;
    friend class TextEmitBuffer;

    // These two are searched as keys.
    TextLayerType         Type;
    // Track bundle entries so that they can be searched for on Removal.
    ArrayLH<BundleEntry*> Entries;

public:    

    TextLayerPrimitive(HAL* hal, TextLayerType type, PrimitiveFill* fill)
        : Primitive(hal, fill), Type(type)
    {
    }

    bool    AddEntry(BundleEntry* e, Mesh* pmesh, const HMatrix& m);
    bool    RemoveEntry(BundleEntry* e);

    void    PinEntryBatches();
    void    UnpinEntryBatches();

    void    ShuffleAndMergeVirtualBatches();
};


// ***** TextPrimitiveBundle

class TextPrepareBuffer;

class TextPrimitiveBundle : public Bundle, public RenderQueueItem::Interface
{
    friend class TextPrepareBuffer;
    friend class TextEmitBuffer;
    friend class TextMeshProvider;
public:
    TextPrimitiveBundle();
    ~TextPrimitiveBundle();

    void    Draw(HAL* hal)
    {
        hal->Draw(this);
    }

    virtual void    UpdateMesh(BundleEntry* entry);
    virtual void    InsertEntry(UPInt index, BundleEntry* shape);
    virtual void    RemoveEntries(UPInt index, UPInt count);
    
    // RenderQueueItem::Interface implementation
    virtual QIPrepareResult  Prepare(RenderQueueItem&, RenderQueueProcessor&, bool);
    virtual void             EmitToHAL(RenderQueueItem&, RenderQueueProcessor&);

    // TBD: Shapes are custom - they need HMatrix.
private:       

    void    startPrimitive(RenderQueueItem& item,
                           TextPrepareBuffer* prepareBuffer,
                           RenderQueueProcessor& qp);
    
    bool    addAndPinBatchLayers(TreeCacheText* textCache, TextMeshProvider* tm);
    void    clearBatchLayers();

    void    pinLayerBatches()
    {
        for (UPInt i = 0; i< Layers.GetSize(); i++)
            Layers[i]->PinEntryBatches();
    }
    void    unpinLayerBatches()
    {
        for (UPInt i = 0; i< Layers.GetSize(); i++)
            Layers[i]->UnpinEntryBatches();
    }

    void    removeEntryFromLayers(BundleEntry* e);

    ArrayReserveLH_Mov<Ptr<TextLayerPrimitive> > Layers;

    // Mask primitive, used if any one of the layers has a mask.
    Ptr<MaskPrimitive> pMaskPrimitive;
};




}} // Scaleform::Render

#endif

