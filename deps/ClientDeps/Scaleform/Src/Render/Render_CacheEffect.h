/**********************************************************************

Filename    :   Render_CacheEffect.h
Content     :   Defines CacheEffect and CacheEffectChain classes
                attachable to TreeCacheNode for mask, blend mode and
                other effect support.
Created     :   July 8, 2010
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

#ifndef INC_SF_Render_CacheEffect_H
#define INC_SF_Render_CacheEffect_H

#include "Render_Bundle.h"
#include "Render_States.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {

class TreeCacheNode;


// CacheEffect represents a single effect typically created
// for a specific state, associated with a tree cache node.
// An effect typically contains a Start and End BundleEntry objects,
// that represent rendering commands to be executed before
// and after tree node is rendered, respectively.


class CacheEffect : public NewOverrideBase<StatRender_TreeCache_Mem>
{
    friend class CacheEffectChain;
protected:    
    CacheEffect* pNext;    
    UInt32       Length; // Cumulative number of entries in this chain
                         // and nested one.    
public:
    CacheEffect(CacheEffect* next)
       :  pNext(next), Length(0)
    { }

    virtual ~CacheEffect() { }

    virtual StateType      GetType() const = 0;
    virtual TreeCacheNode* GetSourceNode() const = 0;
    virtual bool           Update(const State*) = 0;

    // Chains next effect bundle to us; fills in result describing us.
    // Input:  *chain is the shape/child bundle list, or was obtained by a
    //          ChainNext call on the following effect.
    // Output: *chain is the resulting chain including us.
    virtual void            ChainNext(BundleEntryRange* chain,
                                      BundleEntryRange* maskChain = 0) = 0;
    // Obtains our bundle range.
    virtual void            GetRange(BundleEntryRange* result) = 0;

    typedef CacheEffect* (*CreateFunc)(TreeCacheNode*, const State*, CacheEffect* next);
};

class MaskEffect;


// A chain of cache effects stored within a TreeCacheNode.
// Effects are created based on StateBag entries attached to TreeNode.

class CacheEffectChain
{
    CacheEffect* pEffect;

public:
    CacheEffectChain() : pEffect(0) { }
    ~CacheEffectChain();

    bool IsEmpty() const { return pEffect == 0; }

    // Return 'true' if change took place that resulted in
    // parent bundle chain needing an update.
    // Takes TreeNode change flags.
    bool UpdateEffects(TreeCacheNode* node, unsigned changeFlags);
    
    // Updates the chain of bundles:
    //   - On input *chain should is list on Entrys nested within
    //    an effect set.
    //   - On output, it is the resulting entry set.
    void UpdateBundleChain(BundleEntryRange* chain,
                           BundleEntryRange* maskChain = 0)
    {
        if (pEffect)
            updateBundleChain(pEffect, chain, maskChain);
    }

    bool GetRange(BundleEntryRange* chain)
    {
        if (pEffect)
        {
            pEffect->GetRange(chain);
            return true;
        }
        return false;
    }

    inline MaskEffect* GetMaskEffect() const;

private:
    void updateBundleChain(CacheEffect* effect,
                           BundleEntryRange* chain,
                           BundleEntryRange* maskChain);
};


//--------------------------------------------------------------------

enum MaskEffectState
{
    MES_NoMask,
    MES_Culled,
    MES_Clipped,
    MES_Combinable,
    MES_Entry_Count
};


class MaskEffect : public CacheEffect
{
public:
    MaskEffect(TreeCacheNode* node,
               const MaskEffectState mes, const HMatrix& areaMatrix,
               CacheEffect* next);

    // Mask has three entrys
    BundleEntry     StartEntry, EndEntry, PopEntry;
    MaskEffectState MES;    
    HMatrix         BoundsMatrix;

    HMatrix               GetBoundsMatrix() const { return BoundsMatrix; }
    inline MaskEffectState GetEffectState() const { return MES; }

    // Uodates BoundsMatrix and returns 'true' if full update
    // needs to be queued up.
    bool                   UpdateMatrix(const MaskEffectState mes,
                                        const Matrix2F& areaMatrix);

    virtual StateType      GetType() const { return State_MaskNode; }
    virtual TreeCacheNode* GetSourceNode() const { return StartEntry.pSourceNode; }
    virtual bool           Update(const State*);
    virtual void           ChainNext(BundleEntryRange* chain,
                                     BundleEntryRange* maskChain);
    virtual void           GetRange(BundleEntryRange* result);

    static CacheEffect*    Create(TreeCacheNode*, const State*, CacheEffect* next);
};

inline MaskEffect* CacheEffectChain::GetMaskEffect() const
{
    CacheEffect* effect = pEffect;
    while (effect && effect->GetType() != State_MaskNode)
        effect = effect->pNext;
    return (MaskEffect*)effect;
}


class BlendModeEffect : public CacheEffect
{
public:
    BlendModeEffect(TreeCacheNode* node, const BlendState& state, CacheEffect* next);
    ~BlendModeEffect();

    // CacheEffect will typically have at least two entrys.
    BundleEntry  StartEntry, EndEntry;

    virtual StateType      GetType() const { return State_BlendMode; }
    virtual TreeCacheNode* GetSourceNode() const { return StartEntry.pSourceNode; }
    virtual bool           Update(const State*);
    virtual void           ChainNext(BundleEntryRange* chain,
                                     BundleEntryRange*);
    virtual void           GetRange(BundleEntryRange* result);

    static CacheEffect* Create(TreeCacheNode*, const State*, CacheEffect* next);
};


}} // Scaleform::Render

#endif
