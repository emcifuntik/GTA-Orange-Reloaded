/**********************************************************************

Filename    :   Render_CacheEffect.cpp
Content     :   Implements CacheEffect and CacheEffectChain classes
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

#include "Render_CacheEffect.h"
#include "Render_TreeCacheNode.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {


//--------------------------------------------------------------------
// ***** CacheEffectChain


struct ChainOrderRecord
{
    StateType               Type;
    unsigned                ChangeFlag;
    CacheEffect::CreateFunc Factory;
};

ChainOrderRecord ChainOrderSequence[] = 
{
    { State_MaskNode,  Change_State_MaskNode, &MaskEffect::Create },
    { State_BlendMode, Change_State_BlendMode, &BlendModeEffect::Create }
};

enum { ChainOrderSequenzeSize = sizeof(ChainOrderSequence) / sizeof (ChainOrderRecord) };


CacheEffectChain::~CacheEffectChain()
{
    // Delete effects.
    CacheEffect *effect = pEffect, *temp;
    while (effect)
    {
        temp = effect;
        effect = effect->pNext;
        delete temp;
    }
}

bool CacheEffectChain::UpdateEffects(TreeCacheNode* node, unsigned changeFlags)
{
    bool updateParent = false;

    // Walk through sequence in order verifying presence/creating effects.
    if (!node->pNode)
        return false;
    const TreeNode::NodeData* treeNodeData = node->pNode->GetDisplayData();
    const StateBag&           states = treeNodeData->States;

    CacheEffect**prevEffectPointer = &pEffect;
    CacheEffect *effect = pEffect;
    UPInt        iseq;
    
    // Short-circuit: Don't do iteration if states are empty.
    if (!effect && states.IsEmpty())
        return false;

    for (iseq = 0; iseq < ChainOrderSequenzeSize; iseq++)
    {
        ChainOrderRecord& r = ChainOrderSequence[iseq];

        if (effect && (effect->GetType() == r.Type))
        {
            CacheEffect* nextEffect = effect->pNext;
            
            if (changeFlags & r.ChangeFlag)
            {
                const State* state = states.GetState(r.Type);
                if (!state)
                {
                    // Remove effect
                    // Kill List seems unnecessary since parent will not
                    // access us before chains are stripped.
                    delete effect;
                    *prevEffectPointer = nextEffect;
                    updateParent = true;
                    effect = nextEffect;
                    continue;
                }
                
                updateParent |= effect->Update(state);
            }
            prevEffectPointer = &effect->pNext;
            effect = nextEffect;
            continue;
        }
        else
        {
            // Effect of this index not present, which means we may need to create
            // a new effect.
            if (changeFlags & r.ChangeFlag)
            {
                const State* state = states.GetState(r.Type);
                if (state)
                {
                    CacheEffect* newEffect = r.Factory(node, state, effect);
                    if (newEffect)
                    {
                        *prevEffectPointer = newEffect;
                        prevEffectPointer = &newEffect->pNext;
                    }
                }
            }
        }
    }

    return updateParent;
}

void CacheEffectChain::updateBundleChain(CacheEffect* effect,
                                         BundleEntryRange* chain,
                                         BundleEntryRange* maskChain)
{    
    if (effect->pNext)
        updateBundleChain(effect->pNext, chain, maskChain);
    effect->ChainNext(chain, maskChain);
}



//--------------------------------------------------------------------
// ***** Mask Effect

MaskEffect::MaskEffect(TreeCacheNode* node,
                       const MaskEffectState mes, const HMatrix& areaMatrix,
                       CacheEffect* next)
: CacheEffect(next),
  StartEntry(node, SortKey((mes == MES_Clipped) ? SortKeyMask_PushClipped : SortKeyMask_Push)),
  EndEntry(node, SortKey(SortKeyMask_End)),
  PopEntry(node, SortKey(SortKeyMask_Pop)),
  MES(mes), BoundsMatrix(areaMatrix)
{
}

bool MaskEffect::UpdateMatrix(const MaskEffectState mes,
                              const Matrix2F& areaMatrix)
{
    BoundsMatrix.SetMatrix2D(areaMatrix);
    // If masking state changed, mark us for update, which can
    // actually change the state key.
    return (mes != MES);
}

bool MaskEffect::Update(const State* stateArg)
{    
    SF_ASSERT(stateArg->GetType() == State_MaskNode);
    SF_UNUSED(stateArg);
//    MaskNodeState* state = (MaskNodeState*)stateArg;
    
    bool updateParent = false;

    // Update re-calculates the node mostly to see if key-type has
    // changed, in which case it is re-updated. This update is slightly
    // redundant with UpdateMatrix, which marks us for update
    // when MaskEffectState changes (TBD: Could it do this directly?). 

    TreeCacheNode* node = StartEntry.pSourceNode;
    SF_ASSERT(node);
    
    RectF       maskBounds;
    Matrix2F    maskAreaMatrix;
    Matrix2F    viewMatrix;
    node->CalcViewMatrix(&viewMatrix);
    MaskEffectState mes = node->calcMaskBounds(&maskBounds, &maskAreaMatrix,
                                               viewMatrix, MES);

    SortKeyType     keyType;
    SortKeyMaskType keyConstructType;

    if (mes == MES_Clipped)
    {
        keyType          = SortKey_MaskStartClipped;
        keyConstructType = SortKeyMask_PushClipped;
    }
    else
    {
        keyType          = SortKey_MaskStart;
        keyConstructType = SortKeyMask_Push;
    }

    // If type changes, we need to re-flush the bundle.
    if (keyType != StartEntry.Key.GetType())
    {
        StartEntry.ClearBundle();
        StartEntry.Key = SortKey(keyConstructType);
        // Logically, we should re-update parent since our sorting changed.
        SF_ASSERT(node->pRoot);
        // However, Update/UpdateEffects is only called from GetPatternChain, which implies
        // that parent is being rebuilt anyway. This means that we don't need to add it to
        // update list; furthermore, if we did it we could corrupt nodes UpdateFlags after
        // the depth has already been processed (resulting in carryover and corruption
        // in future frames).
        // Do NOT do this: pnode->addParentToDepthPatternUpdate()
        updateParent = true; // Redundant? Need investigation...
    }

    MES = mes;
    BoundsMatrix.SetMatrix2D(maskAreaMatrix);
    return updateParent;
}

void MaskEffect::ChainNext(BundleEntryRange* chain, BundleEntryRange* maskChain)
{
    if ((MES == MES_NoMask) || !maskChain || (maskChain->GetLength() == 0))
    {
        // Mark empty chain with StartEntry.pNextPattern = 0;
        // record chain values in EndEntry/Pop.
        StartEntry.SetNextPattern(0);
        EndEntry.SetNextPattern(chain->GetFirst());
        PopEntry.SetNextPattern(chain->GetLast());
        Length = chain->GetLength();
        // Chain remains unchanged.
    }
    else
    {
        StartEntry.SetNextPattern(maskChain->GetFirst());
        StartEntry.pChain = 0;
        StartEntry.ChainHeight = 0;
        
        maskChain->GetLast()->SetNextPattern(&EndEntry);

        EndEntry.SetNextPattern(chain->GetFirst());
        EndEntry.pChain = 0;
        EndEntry.ChainHeight = 0;

        chain->GetLast()->SetNextPattern(&PopEntry);
        PopEntry.pChain = 0;
        PopEntry.ChainHeight = 0;

        Length = chain->GetLength() + maskChain->GetLength() + 3;
        chain->Init(&StartEntry, &PopEntry, Length);
    }
}

void  MaskEffect::GetRange(BundleEntryRange* result)
{
    if (StartEntry.HasNextPattern())
        result->Init(&StartEntry, &PopEntry, Length);
    else
        result->Init(EndEntry.GetNextPattern(), PopEntry.GetNextPattern(), Length); 
}


CacheEffect* MaskEffect::Create(TreeCacheNode* node, const State*, CacheEffect* next)
{
    // Determine what type of mask to create (clipped or not).
    RectF       maskBounds;
    Matrix2F    maskAreaMatrix;
    Matrix2F    viewMatrix;
    node->CalcViewMatrix(&viewMatrix);
    MaskEffectState mes = node->calcMaskBounds(&maskBounds, &maskAreaMatrix,
                                               viewMatrix, MES_NoMask);
    HMatrix     m = node->GetMatrixPool().CreateMatrix(maskAreaMatrix);

    return SF_HEAP_AUTO_NEW(node) MaskEffect(node, mes, m, next);
}



//--------------------------------------------------------------------
// ***** BlendMode Effect

BlendModeEffect::BlendModeEffect(TreeCacheNode* node, const BlendState& state, CacheEffect* next)
: CacheEffect(next),
  StartEntry(node, SortKey(SortKey_BlendModeStart, state.GetBlendMode())),
  EndEntry(node, SortKey(SortKey_BlendModeEnd, Blend_None))
{
}

BlendModeEffect::~BlendModeEffect()
{
}

bool BlendModeEffect::Update(const State* stateArg)
{
    SF_ASSERT(stateArg->GetType() == State_BlendMode);
    BlendState* state = (BlendState*)stateArg;
     
    if (StartEntry.Key.GetData() != (SortKeyData)state->GetBlendMode())
    {
        // BlendModeEffects don't have pBundle objects,
        // so it's ok to modify data without removing entry from bundle.
        StartEntry.Key.SetData((SortKeyData)state->GetBlendMode());
        return true;
    }
    return false;
}

void  BlendModeEffect::ChainNext(BundleEntryRange* chain, BundleEntryRange*)
{
    StartEntry.SetNextPattern(chain->GetFirst());
    StartEntry.pChain = 0;
    StartEntry.ChainHeight = 0;

    chain->GetLast()->SetNextPattern(&EndEntry);
    EndEntry.pChain = 0;
    EndEntry.ChainHeight = 0;

    Length = chain->GetLength() + 2;
    chain->Init(&StartEntry, &EndEntry, Length);
}

void  BlendModeEffect::GetRange(BundleEntryRange* result)
{
    result->Init(&StartEntry, &EndEntry, Length);
}

CacheEffect* BlendModeEffect::Create(TreeCacheNode* node, const State* stateArg, CacheEffect* next)
{
    SF_ASSERT(stateArg->GetType() == State_BlendMode);
    return SF_HEAP_AUTO_NEW(node) BlendModeEffect(node, *(BlendState*)stateArg, next);
}




}} // Scaleform::Render

