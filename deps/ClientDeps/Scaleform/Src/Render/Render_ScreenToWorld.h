/*****************************************************************

Filename    :   Render_ScreenToWorld.h
Content     :   Screen to World reverse transform helper class 
Created     :   Jan 15, 2010
Authors     :   Mustafa Thamer

History     :   
Copyright   :   (c) 2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Render_SCREENTOWORLD_H
#define INC_SF_Render_SCREENTOWORLD_H

#include "Render/Render_Types2D.h"
#include "Render/Render_Matrix4x4.h"

namespace Scaleform { namespace Render {

//
// This class computes a screen to world transform.
// It requires the following inputs to be set:
//      normalized screen coords - these are from the mouse generally and should be -1 to 1
//      view, perspective and word matrices - these are what are set in the renderer and on the object
//
class ScreenToWorld
{
public:

    ScreenToWorld() : Sx(FLT_MAX), Sy(FLT_MAX), LastX(FLT_MAX), LastY(FLT_MAX),
        MatProj(NULL), MatView(NULL), MatWorld(NULL) { }

    // required inputs
    void SetNormalizedScreenCoords(float nsx, float nsy) { Sx=nsx; Sy=-nsy; }   // note: the Y is inverted
    void SetView(const Matrix3F &mView) 
    { 
        MatView = &mView; 
    }
    void SetProjection(const Matrix4F &mProj) 
    { 
        MatProj = &mProj;
        MatInvProj.SetInverse(mProj);       // cache        
    }
    void SetWorld(const Matrix3F &mWorld) 
    { 
        MatWorld = &mWorld;
    }

    // computes the answer
    void GetWorldPoint(PointF *ptOut);
    void GetWorldPoint(Point3F *ptOut);
    
    PointF GetLastWorldPoint() const { return PointF(LastX, LastY); }
private:
    void VectorMult(float *po, const float *pa, float x, float y, float z, float w);
    void VectorMult(float *po, const float *pa, const float *v);
    void VectorInvHomog(float *v);

    float Sx, Sy;
    float LastX, LastY;
    const Matrix4F* MatProj;
    const Matrix3F* MatView;
    const Matrix3F* MatWorld;
    Matrix4F MatInvProj;
};

}}

#endif  // INC_SF_Render_SCREENTOWORLD_H

