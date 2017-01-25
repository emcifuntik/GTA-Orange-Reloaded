/**********************************************************************

Filename    :   Render_CxForm.h
Content     :   Color transform for renderer.
Created     :   August 17, 2009
Authors     :   Michael Antonov

Notes       :   
History     :   

Copyright   :   (c) 2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#include "Render_CxForm.h"
#include "Kernel/SF_Alg.h"

namespace Scaleform { namespace Render {

Cxform   Cxform::Identity;

// Initialize to identity transform.
Cxform::Cxform()
{
    SetIdentity();
}

// Prepend c's transform onto ours.  When
// transforming colors, c's transform is applied
// first, then ours.
void Cxform::Prepend(const Cxform& c)
{
    M[0][1] += M[0][0] * c.M[0][1];
    M[1][1] += M[1][0] * c.M[1][1];
    M[2][1] += M[2][0] * c.M[2][1];
    M[3][1] += M[3][0] * c.M[3][1];

    M[0][0] *= c.M[0][0];
    M[1][0] *= c.M[1][0];
    M[2][0] *= c.M[2][0];
    M[3][0] *= c.M[3][0];
}

void Cxform::Append(const Cxform& c)
{
    M[0][1] = c.M[0][1] + c.M[0][0] * M[0][1];
    M[1][1] = c.M[1][1] + c.M[1][0] * M[1][1];
    M[2][1] = c.M[2][1] + c.M[2][0] * M[2][1];
    M[3][1] = c.M[3][1] + c.M[3][0] * M[3][1];

    M[0][0] *= c.M[0][0];
    M[1][0] *= c.M[1][0];
    M[2][0] *= c.M[2][0];
    M[3][0] *= c.M[3][0];
}

void Cxform::SetToAppend(const Cxform& c0, const Cxform& c1)
{
    M[0][1] = c1.M[0][1] + c1.M[0][0] * c0.M[0][1];
    M[1][1] = c1.M[1][1] + c1.M[1][0] * c0.M[1][1];
    M[2][1] = c1.M[2][1] + c1.M[2][0] * c0.M[2][1];
    M[3][1] = c1.M[3][1] + c1.M[3][0] * c0.M[3][1];

    M[0][0] = c0.M[0][0] * c1.M[0][0];
    M[1][0] = c0.M[1][0] * c1.M[1][0];
    M[2][0] = c0.M[2][0] * c1.M[2][0];
    M[3][0] = c0.M[3][0] * c1.M[3][0];
}


// Apply our transform to the given color; return the result.
Color  Cxform::Transform(const Color in) const
{
    Color  result(
        (UByte)Alg::Clamp<float>(in.GetRed()   * M[0][0] + 255.0f * M[0][1], 0, 255),
        (UByte)Alg::Clamp<float>(in.GetGreen() * M[1][0] + 255.0f * M[1][1], 0, 255),
        (UByte)Alg::Clamp<float>(in.GetBlue()  * M[2][0] + 255.0f * M[2][1], 0, 255),
        (UByte)Alg::Clamp<float>(in.GetAlpha() * M[3][0] + 255.0f * M[3][1], 0, 255) );

    return result;
}

void Cxform::Normalize()
{
    M[0][1] *= (1.f/255.f);
    M[1][1] *= (1.f/255.f);
    M[2][1] *= (1.f/255.f);
    M[3][1] *= (1.f/255.f);
}


/*
// Debug log.
void    Cxform::Format(char *pbuffer) const
{

    G_Format(GStringDataPtr(pbuffer, GFX_STREAM_BUFFER_SIZE),
        "    *         +\n"
        "| {0:4.4} {1:4.4}|\n"
        "| {2:4.4} {3:4.4}|\n"
        "| {4:4.4} {5:4.4}|\n"
        "| {6:4.4} {7:4.4}|\n",
        M[0][0], M[0][1],
        M[1][0], M[1][1],
        M[2][0], M[2][1],
        M[3][0], M[3][1]
        );
}
*/

void    Cxform::SetIdentity()
{
    M[0][0] = 1;
    M[1][0] = 1;
    M[2][0] = 1;
    M[3][0] = 1;
    M[0][1] = 0;
    M[1][1] = 0;
    M[2][1] = 0;
    M[3][1] = 0;
}

bool    Cxform::IsIdentity() const
{
    return (M[0][0] == 1 &&  M[1][0] == 1 && M[2][0] == 1 && M[3][0] == 1 &&
            M[0][1] == 0 &&  M[1][1] == 0 && M[2][1] == 0 && M[3][1] == 0);
}


}} // Scaleform::Render
