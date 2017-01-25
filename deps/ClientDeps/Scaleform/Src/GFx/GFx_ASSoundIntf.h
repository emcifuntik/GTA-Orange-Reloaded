#ifndef INC_SF_GFX__ASSOUNDINTF_H
#define INC_SF_GFX__ASSOUNDINTF_H

#include "Kernel/SF_Types.h"

namespace Scaleform { namespace GFx {

class ASSoundIntf : public NewOverrideBase<StatMV_ActionScript_Mem>
{
public:
    virtual ~ASSoundIntf() {}

    virtual void        ExecuteOnSoundComplete() =0;
    virtual void        ReleaseTarget() =0;

};

}} // Scaleform::GFx

#endif // INC_SF_GFX__ASSOUNDINTF_H

