/**********************************************************************

PublicHeader:   Render
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

#ifndef INC_SF_Render_CxForm_H
#define INC_SF_Render_CxForm_H

#include "Render_Color.h"

namespace Scaleform { namespace Render {

class Cxform
{
public:
    // 0,1,2,3 -> R,G,B,A
    float   M[4][2];   // [RGBA][mult, add]

    enum NoInitType { NoInit };

    Cxform();
    inline Cxform(NoInitType) { }

    explicit Cxform(Color color)
    {
        M[0][0] = 0; M[1][0] = 0; M[2][0] = 0; M[3][0] = 0;
        M[0][1] = color.GetRed()/255.f; M[1][1] = color.GetGreen()/255.f;
        M[2][1] = color.GetBlue()/255.f; M[3][1] = color.GetAlpha()/255.f;
    }

    // Append applies our transformation before argument one.
    // Proper form: childCx.Append(parentCx).
    void    Append(const Cxform& c);
    void    Prepend(const Cxform& c);
    Color   Transform(const Color in) const;
    
    void    SetToAppend(const Cxform& c0, const Cxform& c1);

    // Normalize transform from range 0...255 to 0...1
    void    Normalize();

    inline void GetAsFloat2x4(float (*rows)[4]) const
    {
        rows[0][0] = M[0][0];
        rows[0][1] = M[1][0];
        rows[0][2] = M[2][0];
        rows[0][3] = M[3][0];

        rows[1][0] = M[0][1];
        rows[1][1] = M[1][1];
        rows[1][2] = M[2][1];
        rows[1][3] = M[3][1];
    }

    // TO DO: Move it out of here!
    // Formats matrix message to a buffer, but 512 bytes at least
    //void    Format(char *pbuffer) const;

    void    SetIdentity();
    bool    IsIdentity() const;

    bool    operator == (const Cxform& x) const
    {
        return M[0][0] == x.M[0][0] && M[1][0] == x.M[1][0] && M[2][0] == x.M[2][0] && M[3][0] == x.M[3][0] &&
               M[0][1] == x.M[0][1] && M[1][1] == x.M[1][1] && M[2][1] == x.M[2][1] && M[3][1] == x.M[3][1];
    }
    static Cxform   Identity;
};

}} // Scaleform::Render

#endif
