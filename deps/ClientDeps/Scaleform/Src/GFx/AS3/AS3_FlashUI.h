/**********************************************************************

Filename    :   AS3_FlashUI.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_AS3_FlashUI_H
#define INC_AS3_FlashUI_H


#include "Abc/AS3_Abc.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

class VM;

class FlashUI
{
public:
    enum StateType {sError, sBreakpoint, sPreStep, sStep};
    
public:
    FlashUI()
    : State(sError)
    , NeedToCheck(false)
    {
    }
    virtual ~FlashUI() {}
    
public:

    // Specifies the type of output that is taking place,
    // used for color-coding and selective output by GFx.
    enum OutputMessageType
    {
        Output_Message,     // Messages generated by trace().
        Output_Error,       // Exception error messages.
        Output_Warning,     // Not implemented and other warnings generated by VM.
        Output_Action       // Action trace and VM state log.
    };

    // Called to output ActionScript trace() statements.
    virtual void Output(OutputMessageType type, const char* msg) = 0;  
    
    // Called for each opcode when GFX_AS3_VERBOSE is defined.
    virtual bool OnOpCode(const Abc::TOpCode& code,
                          Abc::TCodeOffset offset, Abc::Code::OpCode opcode) = 0;

#ifdef USE_WORDCODE
    virtual bool OnOpCode(const ArrayLH_POD<UInt8>& code,
        Abc::TCodeOffset offset, Abc::Code::OpCode opcode) 
    { 
        SF_UNUSED3(code, offset, opcode);
        return false;
    }
#endif

public:
    void SetHandleBreakpoint()
    {
        SetState(sBreakpoint);
    }
    void SetHandleStep()
    {
        SetState(sPreStep);
    }
    void SetHandleError()
    {
        SetState(sError);
    }
    
public:
    StateType GetState()
    {
        return State;
    }
    bool NeedToCheckOpCode() const
    {
        return NeedToCheck;
    }

protected:
    void SetState(StateType _state)
    {
        State = _state;
    }
    void SetNeedToCheckOpCode(bool flag = true)
    {
        NeedToCheck = flag;
    }
    
protected:
    StateType State;
    bool NeedToCheck;
};

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif // INC_AS3_FlashUI_H
