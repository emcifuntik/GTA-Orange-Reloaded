/**********************************************************************

Filename    :   Render_MeshKey.cpp
Created     :   2005-2010
Authors     :   Maxim Shemanarev, Michael Antonov

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#include "Render_MeshKey.h"
//#include "Render_Math2D.h"
//#include "Render_BoundBox.h"
#include "Render_TessGen.h"
#include "Kernel/SF_HeapNew.h"
//#include "Renderer2D_Linear.h"

namespace Scaleform { namespace Render {


//------------------------------------------------------------------------
// ***** MeshKey

unsigned MeshKey::GetKeySize(unsigned flags)
{
    unsigned size = 3;
    if (flags & KF_Scale9Grid)
    {
        size = Scale9GridData::MeshKeySize;
    }
    size++; // Morph ratio
 
    //// TO DO: revise, consider XY-offset in the MeshKey
    //if ((flags & KF_KeyTypeMask) == KF_StrokeHinted)
    //{
    //    size += 2;
    //}

    SF_ASSERT(size <= MaxKeySize);
    return size;
}


//------------------------------------------------------------------------
bool MeshKey::Match(unsigned layer, unsigned flags,
                    const float* keyData, const ToleranceParams& cfg) const
{
    if (layer != pMesh->GetLayer() || flags != Flags) 
        return false;

    if (flags & KF_Degenerate)
        return true;

    unsigned i;

    if (flags & KF_Scale9Grid)
    {
        for(i = 0; i < unsigned(Size-1); ++i)
        {
            if (keyData[i] != Data[i])
                return false;
        }
    }
    else
    {
        unsigned s = 3; // TO DO: possibly revise
        float fillLowerScale = cfg.FillLowerScale;
        float fillUpperScale = cfg.FillUpperScale;
        if ((flags & KF_EdgeAA) == 0 || (flags & KF_Mask) != 0)
        {
            fillLowerScale = cfg.FillAliasedLowerScale;
            fillUpperScale = cfg.FillAliasedUpperScale;
        }

        switch(flags & KF_KeyTypeMask)
        {
        case KF_Fill:
            for(i = 0; i < s; ++i)
            {
                if (keyData[i] < Data[i] * fillLowerScale || 
                    keyData[i] > Data[i] * fillUpperScale)
                {
                    return false;
                }
            }
            break;

        case KF_Stroke:
            for(i = 0; i < s; ++i)
            {
                if (keyData[i] < Data[i] * cfg.StrokeLowerScale || 
                    keyData[i] > Data[i] * cfg.StrokeUpperScale)
                {
                    return false;
                }
            }
            break;

        case KF_StrokeHinted:
            // TO DO: revise, consider offset in the MeshKey
            for(i = 0; i < s; ++i)
            {
                if (keyData[i] < Data[i] * cfg.HintedStrokeLowerScale || 
                    keyData[i] > Data[i] * cfg.HintedStrokeUpperScale)
                {
                    return false;
                }
            }
            break;
        }
    }

    // Morph ratio
    if (keyData[Size-1] < Data[Size-1] * (1-cfg.MorphTolerance) ||
        keyData[Size-1] > Data[Size-1] * (1+cfg.MorphTolerance))
    {
        return false;
    }
    return true;
}


//------------------------------------------------------------------------
bool MeshKey::CalcMatrixKey(const Matrix2F& m, float* key, Matrix2F* m2)
{
    // DBG: GFx3.0 scaling key
    //key[0] = m.GetMaxScale();
    //key[1] = 1;
    //key[2] = 1;
    //if (m2) *m2 = GMatrix2D::Scaling(key[0]);
    //return;

    // Advanced, 3-component scaling key. It takes into account
    // ScaleX, ScaleY and Skewing
    //----------------------
    float x2 = m.Sx();
    float y2 = m.Shy();
    float x3 = m.Shx();
    float y3 = m.Sy();

    // The legend:
    //----------------------------------------------------
    //               x3,y3
    //                 +-----------------+
    //                /|                /|
    //               / |               / |
    //              /  |              /  |
    //             /   |             /   |
    //          sy/    |h           /    |
    //           /     |           /     |
    //          /      |          /      |
    //   (0,0) +---w---+---------+ x2,y2 |
    //         |--------sx-------|       |
    //         |-----------sx+w----------|
    //----------------------------------------------------
    float d2 = x2*x2 + y2*y2;
    float d3 = x3*x3 + y3*y3;
    if (d2 == 0 || d3 == 0) 
        return false;                       // Degenerate matrix (zero scale)
    float sx = sqrtf(d2);
    float h  = fabsf((y3-y2)*x2 - (x3-x2)*y2) / sx;
    if (h == 0)
        return false;                       // Degenerate matrix (infinite skew)
    float w  = sx * (x3*x2 + y3*y2) / d2;

    key[0] = sx;
    key[1] = sqrtf(d3);                     // sy
    key[2] = (w >= 0) ? 1+w/h : h/(h-w);    // sh (skew)

    if (m2)
    {
        // Normalize the matrix to remove the rotation. We just take
        // the parallelogram based on the OX axis and construct
        // the matrix to transform rectangle (0,0)(1,1) to (0,0)(sx,0)(sx+w,h).
        //---------------------
        float p[8] = { 0, 0, sx, 0, sx+w, h }; // p[8] is used later
        m2->SetRectToParl(0, 0, 1, 1, p);

        // Optimization. When we have a skewed matrix it's better to select
        // the longest base and put it vertically, so that, the tessellator
        // could potentially produce better triangles. The following code
        // can be commented out or removed:
        //-------------------- from here
        p[0] =  1;              // 0 degree
        p[1] =  0;
        p[2] =  0.707106781f;   // 45 degree
        p[3] =  0.707106781f;
        p[4] =  0;       
        p[5] =  1.05f;          // 90 degree, make it have higher priority
        p[6] = -0.707106781f;
        p[7] =  0.707106781f;   // 135 degree
        float    maxLen = 0;
        unsigned maxIdx = 4;
        for (unsigned i = 0; i < 8; i += 2)
        {
            m2->Transform2x2(&p[i+0], &p[i+1]);
            d2 = p[i+0]*p[i+0] + p[i+1]*p[i+1];
           if (d2 > maxLen)
            {
                maxLen = d2;
                maxIdx = i;
            }
        }
        m2->AppendRotation(3.141592653589793f/2 - atan2f(p[maxIdx+1], p[maxIdx+0]));
        //------------------- to here
    }
    return true;
}

void MeshKey::Release()
{
    if (--UseCount == 0)
    {
        // Remove us if Mesh is also evicted.
        if (!pMesh || pMesh->IsEvicted())
            pKeySet->DestroyKey(this);
    }
}


//------------------------------------------------------------------------
void MeshKeySetHandle::releaseCache()
{
    MeshKeyManager* manager = pManager.Exchange_NoSync(0);
    if (manager)
    {
        manager->providerLost(*this);
        manager->Release();
    }
}
void MeshKeySetHandle::releaseCache_NoNotify()
{
    MeshKeyManager* manager = pManager.Exchange_NoSync(0);
    if (manager)
        manager->Release();
}


//------------------------------------------------------------------------
// ***** MeshKeySet

MeshKey* MeshKeySet::CreateKey(const float* keyData, unsigned flags)
{
    unsigned size = MeshKey::GetKeySize(flags);
    void*    buf = SF_HEAP_ALLOC(pManager->pRenderHeap,
                                 sizeof(MeshKey) + sizeof(float) * (size - 1),
                                 StatRender_Mem);
    if (!buf) return 0;

    MeshKey* key = Construct<MeshKey>(buf);
    key->pKeySet = this;
    key->Size  = (UInt16)size;
    key->Flags = (UInt16)flags;

    SF_ASSERT((flags&0xFFFF) == flags);
    memcpy(key->Data, keyData, sizeof(float) * size);

    Meshes.PushBack(key);
    return key;
}

void MeshKeySet::DestroyKey(MeshKey* key)
{
    SF_ASSERT(key->pKeySet == this);

    // Key should no longer be in use when this is called.
    // If it is, it means MeshKey references in TreeCache weren't flushed,
    // which they should be before MeshKeyManager cleanup.
    SF_ASSERT(key->NotInUse());

    key->RemoveNode();
    if (key->pMesh)
    {
        key->pMesh->ClearProvider();
        key->pMesh.Clear();
    }    
    SF_FREE(key);

    // Free this keySet if it became empty.
    if (Meshes.IsEmpty())
        pManager->destroyKeySet(this);
}

void MeshKeySet::DestroyAllKeys()
{    
    MeshKey *key = Meshes.GetFirst();
    MeshKey *temp;

    // Clear out mesh handlers first, so that they don't call us recursively,
    // trying to remove following items.
    while(!Meshes.IsNull(key))
    {
        // Key should no longer be in use (see comment in DestroyKey).
        SF_ASSERT(key->NotInUse());
        if (key->pMesh)
            key->pMesh->ClearProvider();
        key = key->pNext;
    }

    key = Meshes.GetFirst();

    while(!Meshes.IsNull(key))
    {
        temp = key->pNext;
        if (key->pMesh)
            key->pMesh.Clear();
        SF_FREE(key);
        key = temp;
    }
    Meshes.Clear();
}


void MeshKeySet::OnEvict(MeshBase *pmesh)
{
    MeshKey* key = Meshes.GetFirst();
    while (!Meshes.IsNull(key))
    {
        if (pmesh == key->pMesh.GetPtr())
        {
            if (key->NotInUse())
                DestroyKey(key);
            break;
        }
        key = key->pNext;
    }
}

MeshKey* MeshKeySet::findMatchingKey(unsigned layer, unsigned flags,
                                     const float* keyData, const ToleranceParams& cfg)
{
    MeshKey* key = Meshes.GetFirst();
    while (!Meshes.IsNull(key))
    {
        if (key->Match(layer, flags, keyData, cfg))
            return key;
        key = key->pNext;
    }
    return 0;
}

void MeshKeySet::releaseDelegate_RenderThread()
{
    // Must be called on RenderThread only during Lock!
    if (pDelegate)
    {
        MeshKeySetHandle& handle = pDelegate->hKeySet;

        // pDelegate MeshProvider is guaranteed alive until releaseCache_NoNotify
        // executes, since it stores atomically exchanged pointer. Even if ~MeshProvider
        // executes then, it will be blocked on our Lock.
        SF_ASSERT(handle.pKeySet == this);
        handle.pKeySet = 0;
        handle.releaseCache_NoNotify();
        pDelegate = 0;
    }
}


//------------------------------------------------------------------------
// ***** MeshKeyManager

void MeshKeyManager::DestroyAllKeys()
{
    Lock::Locker scopeLock(getLock());
    destroyKeySetList_NTS(KeySet_KillList);
    destroyKeySetList_NTS(KeySet_LiveList);
}

void MeshKeyManager::ProcessKillList()
{
    Lock::Locker scopeLock(getLock());
    destroyKeySetList_NTS(KeySet_KillList);
}


MeshKey* MeshKeyManager::CreateMatchingKey(MeshProvider_KeySupport* provider,
                                           unsigned layer, unsigned flags,
                                           const float* keyData,
                                           const ToleranceParams& cfg)
{
    MeshKeySetHandle& handle = provider->hKeySet;
    SF_ASSERT(!handle.pManager || (handle.pManager == this));

    // When we are called here, we are assumed to own MeshKeySetHandle container
    // reference, such that it can't possibly be destroyed in the other thread,
    // This means that our access is safe.
    if (!handle.pManager)
    {
        AddRef();
        handle.pManager = this;
    }
    if (!handle.pKeySet)
    {
        Lock::Locker scope(getLock());

        handle.pKeySet = SF_HEAP_NEW(pRenderHeap) MeshKeySet(this, provider);
        if (!handle.pKeySet)
            return 0;
        KeySets[KeySet_LiveList].PushBack(handle.pKeySet);
    }

    return CreateMatchingKey(handle.pKeySet, layer, flags, keyData, cfg);
}

MeshKey* MeshKeyManager::CreateMatchingKey(MeshKeySet *keySet,
                                           unsigned layer, unsigned flags,
                                           const float* keyData,
                                           const ToleranceParams& cfg)
{
    SF_ASSERT(keySet != 0);

    MeshKey* key = keySet->findMatchingKey(layer, flags, keyData, cfg);
    if (key)
    {
        key->AddRef();
        return key;
    }
    return keySet->CreateKey(keyData, flags);
}


void MeshKeyManager::providerLost(MeshKeySetHandle& handle)
{
    Lock::Locker scopeLock(getLock());    

    // Since handle.pKeySet is always modified in lock, we can
    // check it directly to see if MeshKeySet was destroyed on us.
    // This is possible if RenderThread swapped out all related meshes
    // or called DestroyAllKeys().
    // Key set was already destroyed, nothing to do.
    MeshKeySet* keySet = handle.pKeySet;
    if (!keySet)
        return;

    // If key set wasn't destroyed yet, place it into KillList.
    // That list is freed on render thread.
    SF_ASSERT(keySet->pDelegate != 0);

    // Move to kill/evict list. 
    keySet->RemoveNode();
    KeySets[KeySet_KillList].PushBack(keySet);
    keySet->pDelegate = 0;
    handle.pKeySet = 0;
}


void MeshKeyManager::destroyKeySet(MeshKeySet* keySet)
{
    Lock::Locker scopeLock(getLock());

    keySet->releaseDelegate_RenderThread();
    keySet->RemoveNode();
    delete keySet;

    // Doesn't hurt to check KillList occasionally.
    if (!KeySets[KeySet_KillList].IsEmpty())
        destroyKeySetList_NTS(KeySet_KillList);
}

void MeshKeyManager::destroyKeySetList_NTS(KeySetListType type)
{    
    if (KeySets[type].IsEmpty())
        return;

    // Steal items from source list so that we short-circuit if (KillList.IsEmpty())
    // call in case we are called recursively. Recursive calls come in due to mesh
    // handlers in 'delete keySet'; to address this we remove one item at a time.
    List<MeshKeySet> list;
    list.PushListToFront(KeySets[type]);

    do {
        MeshKeySet* keySet = list.GetFirst();

        if (type != KeySet_KillList)
        {   // Update mesh providers if destroying main lust.
            keySet->releaseDelegate_RenderThread();
        }
        else
        {   // For kill list, pDelegate provider should be clear.
            SF_ASSERT(keySet->pDelegate == 0);
        }

        keySet->RemoveNode();
        delete keySet;
    } while(!list.IsEmpty());
}


}} // Scaleform::Render
